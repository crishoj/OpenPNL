/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/* data structures I/O to XML */

#include "_cxcore.h"
#include <ctype.h>

#define ICX_FORMAT_TEXT 0
#define ICX_FORMAT_BINARY 1
#define ICX_IS_BINARY(storage) (((storage)->flags & 3) == CX_STORAGE_WRITE_BINARY)
#define ICX_IS_WRITE_MODE(storage) (((storage)->flags & 3) != 0)

#define ICX_DT_SHIFT(dt_buf)   (dt_buf[0] == '1' && !isdigit(dt_buf[1]))

static const char icxTypeSymbol[] = "ucsifd";
static const char* icxStandardTags[] = { "struct", "elem", 0 };

/* these are hacked versions of strtod & strtol that insert NUL character
   at the possible end-of-number position, which speeds up strtod & strtol execution in
   case of huge files ('cause they may call strlen() internally) */
static double icx_strtod( const char* ptr, char** endptr )
{
    double val;
    char* eonum = (char*)ptr;
    char char_copy;
    while( isspace(*eonum))
        eonum++;

    while( isdigit(*eonum) || *eonum == '.' || *eonum == '-' ||
           *eonum == 'e' || *eonum == 'E' || *eonum == '+' )
        eonum++;
    char_copy = *eonum;
    *eonum = (char)'\0';
    val = strtod( ptr, endptr );
    *eonum = char_copy;

    return val;
}


static long icx_strtol( const char* ptr, char** endptr, int base )
{
    long val;
    char* eonum = (char*)ptr;
    char char_copy;
    while( isspace(*eonum))
        eonum++;

    while( isdigit(*eonum) || *eonum == '-' || *eonum == '+' )
        eonum++;
    char_copy = *eonum;
    *eonum = (char)'\0';
    val = strtol( ptr, endptr, base );
    *eonum = char_copy;

    return val;
}


/* "black box" file storage */
typedef struct CxFileStorage
{
    int flags;
    int width;
    int max_width;
    FILE* file;
    CxMemStorage* dststorage;
    CxMemStorage* memstorage;
    CxSet* nodes;
    CxFileNode* root;
    CxFileNode* parent;
    int buffer_size;
    int line;
    char* filename;

    char* base64_buffer;
    char* base64_buffer_read;
    char* base64_buffer_start;
    char* base64_buffer_end;

    char* buffer_start;
    char* buffer_end;
    char* buffer;
    char* line_start;

    CxFileNode** hashtable;
    int   hashsize;
}
CxFileStorage;

/*********************************** Adding own types ***********************************/

static int
icxCheckString( const char* str, char termchar CX_DEFAULT(0))
{
    int l = 0;

    CX_FUNCNAME("icxCheckString");

    __BEGIN__;

    if( !str )
        CX_ERROR( CX_StsNullPtr, "NULL string" );

    if( (termchar & ~0x7f) != 0 || termchar == '\\' ||
        termchar == '\"' || (termchar != '\0' && !isprint(termchar)))
        CX_ERROR( CX_StsBadArg, "Invalid termination character" );

    for( ; l < CX_MAX_STRLEN && str[l] != '\0'; l++ )
    {
        int c = str[l];
        /*if( c == '\\' )
        {
            c = str[++l];
            if( c != '\\')// && c != '\"')
                CX_ERROR( CX_StsError,
                "Only \'\\\\\' esc-sequences is supported" );
            continue;
        }*/

        if( c == termchar )
            break;

        if( c > 0x7f || !isprint(c) || c == '\"' )
        {
            char msg[32];
            sprintf( msg, "Invalid character: %c (=\'\\x%02x\')", c, c );
            CX_ERROR( CX_StsBadArg, msg );
        }
    }

    if( l == CX_MAX_STRLEN )
        CX_ERROR( CX_StsOutOfRange, "Too long string" );

    __END__;

    if( cxGetErrStatus() < 0 )
        l = -1;
    return l;
}


CX_IMPL const char*
cxAttrValue( const CxAttrList* attr, const char* attr_name )
{
    while( attr && attr->attr )
    {
        int i;
        for( i = 0; attr->attr[i*2] != 0; i++ )
        {
            if( strcmp( attr_name, attr->attr[i*2] ) == 0 )
                return attr->attr[i*2+1];
        }
        attr = attr->next;
    }

    return 0;
}


static CxFileNode*
icxQueryName( CxFileStorage* storage, const char* name,
              CxFileNode* new_node CX_DEFAULT(0))
{
    CxFileNode* result_node = 0;

    CX_FUNCNAME( "icxQueryName" );

    __BEGIN__;

    unsigned hash_val = 0;
    int i;
    CxFileNode* node;

    if( !storage || !name )
        CX_ERROR( CX_StsNullPtr, "" );

    for( i = 0; name[i] != 0; i++ )
        hash_val = hash_val*33 + (uchar)name[i];

    i = hash_val % storage->hashsize;
    for( node = storage->hashtable[i]; node != 0; node = node->hash_next )
    {
        assert( node->name );
        if( node->hash_val == hash_val && strcmp( node->name, name ) == 0 )
            break;
    }

    if( node )
        result_node = node;
    else if( new_node )
    {
        new_node->hash_val = hash_val;
        assert( strcmp(new_node->name, name) == 0 );
        new_node->hash_next = storage->hashtable[i];
        storage->hashtable[i] = new_node;
        result_node = new_node;
    }

    __END__;

    return result_node;
}


#define ICX_EMPTY_TAG       1
#define ICX_INCOMPLETE_TAG  2

static void
icxPushXMLTag( CxFileStorage* storage, const char* tagname,
               CxAttrList _attr, int flags CX_DEFAULT(0))
{
    CX_FUNCNAME( "icxPushXMLTag" );

    __BEGIN__;

    int i, dl;
    CxAttrList* attr = &_attr;

    assert( storage && storage->file && tagname );

    CX_CALL( icxCheckString( tagname ));

    assert( storage->width == 0 );
    dl = 0;
    fprintf( storage->file, "<%s%n", tagname, &dl );
    storage->width += dl;

    while( attr && attr->attr )
    {
        for( i = 0; attr->attr[i] != 0; i += 2 )
        {
            CX_CALL( icxCheckString( attr->attr[i] ));
            if( attr->attr[i] == 0 )
                CX_ERROR( CX_StsNullPtr, "One of attribute values is NULL" );
            CX_CALL( icxCheckString( attr->attr[i+1] ));
            dl = 0;
            fprintf( storage->file, " %s=\"%s\"%n", attr->attr[i], attr->attr[i+1], &dl );
            storage->width += dl;
            if( storage->width >= storage->max_width )
            {
                fprintf( storage->file, "\n" );
                storage->width = 0;
            }
        }
        attr = attr->next;
    }

    if( !(flags & ICX_INCOMPLETE_TAG) )
    {
        fprintf( storage->file, (flags & ICX_EMPTY_TAG) ? "/>\n" : ">\n" );
        storage->width = 0;
    }

    for( i = 0; icxStandardTags[i] != 0; i++ )
    {
        if( strcmp( icxStandardTags[i], tagname ) == 0 )
        {
            tagname = icxStandardTags[i];
            break;
        }
    }

    if( icxStandardTags[i] == 0 )
    {
        char* ptr;
        CX_CALL( ptr = (char*)cxMemStorageAlloc( storage->memstorage, strlen(tagname) + 1 ));
        strcpy( ptr, tagname );
        tagname = ptr;
    }

    {
        CxFileNode* node = (CxFileNode*)cxSetNew( storage->nodes );
        memset( node, 0, sizeof(*node));
        node->tagname = tagname;

        if( storage->root )
        {
            if( !storage->parent )
                CX_ERROR( CX_StsError, "<opencx_storage> is already closed" );
            cxInsertNodeIntoTree( node, storage->parent, 0 );
        }
        else
            storage->root = storage->parent = node;
        if( !(flags & ICX_EMPTY_TAG) )
            storage->parent = node;
    }

    __END__;
}


static const char icxBase64Tab[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void
icxWriteBase64Block( CxFileStorage* storage, int flush_flag )
{
    /*CX_FUNCNAME( "icxWriteBase64Block" );*/

    __BEGIN__;

    uchar* buffer = (uchar*)storage->base64_buffer_start;
    int i, len = storage->base64_buffer - storage->base64_buffer_start;
    int val;

    for( i = 0; i <= len - 3; i += 3 )
    {
        int dl = 0;
        val = (buffer[i]<<16) + (buffer[i+1]<<8) + buffer[i+2];
        fprintf( storage->file, "%c%c%c%c%n",
                 icxBase64Tab[val >> 18], icxBase64Tab[(val >> 12) & 63],
                 icxBase64Tab[(val >> 6) & 63], icxBase64Tab[val & 63], &dl );
        storage->width += dl;
        if( storage->width > 72 )
        {
            fprintf( storage->file, "\n" );
            storage->width = 0;
        }
    }

    if( flush_flag )
    {
        buffer[len] = buffer[len+1] = '\0';
        val = (buffer[i]<<16) + (buffer[i+1]<<8) + buffer[i+2];

        switch( len - i )
        {
        case 0:
            if( storage->width )
                fprintf( storage->file, "\n" );
            break;
        case 1:
            fprintf( storage->file, "%c%c==\n",
                     icxBase64Tab[(val >> 18)], icxBase64Tab[(val >> 12) & 63] );
            break;
        case 2:
            fprintf( storage->file, "%c%c%c=\n",
                     icxBase64Tab[(val >> 18)], icxBase64Tab[(val >> 12) & 63],
                     icxBase64Tab[(val >> 6) & 63]);
            break;
        default:
            assert(0);
        }
        storage->width = 0;
        storage->base64_buffer = storage->base64_buffer_start;
    }
    else
    {
        len -= i;
        if( len )
            memmove( storage->base64_buffer_start,
                     storage->base64_buffer_start + i, len );
        storage->base64_buffer = storage->base64_buffer_start + len;
    }

    __END__;
}


static const uchar icxInvBase64Tab[] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 62, 255, 255, 255, 63, 52, 53,
54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
255, 64, 255, 255, 255, 0, 1, 2, 3, 4,
5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
25, 255, 255, 255, 255, 255, 255, 26, 27, 28,
29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255 };

static int
icxReadBase64Block( CxFileStorage* storage )
{
    int ok = 0;

    /*CX_FUNCNAME( "icxReadBase64Block" );*/

    __BEGIN__;

    uchar* dst = (uchar*)storage->base64_buffer_start;
    uchar* limit = (uchar*)storage->base64_buffer_end;
    uchar* src = (uchar*)storage->buffer;
    int c[4], k = 0;
    int len = storage->base64_buffer_read - storage->base64_buffer;

    if( len > 0  )
    {
        // copy remaining bytes to the beginning of the buffer
        memmove( storage->base64_buffer_start, storage->base64_buffer, len );
        dst += len;
    }

    while( dst < limit )
    {
        for( k = 0; k < 4; k++ )
        {
            c[k] = icxInvBase64Tab[src[0]];
            if( c[k] > 63 )
            {
                while( isspace((char)src[0]))
                    src++;
                c[k] = icxInvBase64Tab[src[0]];
                if( c[k] > 63 )
                    goto loop_end;
            }
            src++;
        }
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst[1] = (uchar)((c[1] << 4)|(c[2] >> 2));
        dst[2] = (uchar)((c[2] << 6)|c[3]);
        dst += 3;
    }

loop_end:

    switch( k & 3 )
    {
    case 0:
        break;
    case 1:
        EXIT; // error
    case 2:
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst++;
        break;
    case 3:
        dst[0] = (uchar)((c[0] << 2)|(c[1] >> 4));
        dst[1] = (uchar)((c[1] << 4)|(c[2] >> 2));
        dst += 2;
    }

    storage->base64_buffer = storage->base64_buffer_start;
    storage->base64_buffer_read = (char*)dst;
    storage->buffer = (char*)src;
    ok = 1;

    __END__;

    return ok;
}


static void
icxPopXMLTag( CxFileStorage* storage )
{
    /*CX_FUNCNAME( "icxPopXMLTag" );*/

    __BEGIN__;

    assert( storage && storage->file && storage->root && storage->parent &&
            storage->parent->tagname );

    icxWriteBase64Block( storage, 1 );

    fprintf( storage->file, "%s</%s>\n",
             storage->width == 0 ? "" : "\n",
             storage->parent->tagname );
    storage->width = 0;
    storage->parent = storage->parent->v_prev;

    __END__;
}


static void
icxWriteRawData( CxFileStorage* storage, const char* dt, int format,
                 const void* _data, int count )
{
    const char* data = (const char*)_data;

    CX_FUNCNAME( "icxWriteRawData" );

    __BEGIN__;

    int i = 0, k = 0;
    int pl = strlen( dt );

    if( pl == 0 )
        CX_ERROR( CX_StsBadArg, "Empty format specification" );

    if( format == ICX_FORMAT_TEXT )
    {
        if( pl == 1 || (pl == 2 && dt[0] == '1') )
        {
            switch( dt[pl-1] )
            {
            case 'u':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %3d%n", ((uchar*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 'c':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %4d%n", ((char*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 's':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %6d%n", ((short*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'i':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file, " %6d%n", ((int*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'f':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file," %14.6e%n", ((float*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;

            case 'd':
                for( i = 0; i < count; i++ )
                {
                    int dl = 0;
                    fprintf( storage->file," %23.15e%n", ((double*)data)[i], &dl );
                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            case 'a':
                if( count != 1 )
                    CX_ERROR( CX_StsBadArg, "Only a single string can be written at once" );
                {
                    int dl = 0;
                    fprintf( storage->file,"%s%n", (char*)data, &dl );

                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
                break;
            default:
                CX_ERROR( CX_StsBadArg, "Unknown format specifier" );
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    int dl = 0;

                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CX_ERROR(CX_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                                dl = 0;
                            }
                            if( n == 0 )
                                CX_ERROR(CX_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                        fprintf( storage->file, " %3d%n", *(uchar*)(data+k), &dl );
                        k++;
                        break;
                    case 'c':
                        fprintf( storage->file, " %4d%n", *(char*)(data+k), &dl );
                        k++;
                        break;
                    case 's':
                        k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                        fprintf( storage->file, " %6d%n", *(short*)(data+k), &dl );
                        k += sizeof(short);
                        break;
                    case 'i':
                        k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                        fprintf( storage->file, " %6d%n", *(int*)(data+k), &dl );
                        k += sizeof(int);
                        break;
                    case 'f':
                        k = (k + sizeof(float) - 1) & -(int)sizeof(float);
                        fprintf( storage->file, " %14.6e%n", *(float*)(data+k), &dl );
                        k += sizeof(float);
                        break;
                    case 'd':
                        k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                        fprintf( storage->file, " %23.15e%n", *(double*)(data+k), &dl );
                        k += sizeof(double);
                        break;
                    case 'p':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        k += sizeof(void*);
                        break;
                    case 'r':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        fprintf( storage->file, " %6u%n",
                                 (unsigned)(unsigned long)*(void**)(data+k), &dl );
                        k += sizeof(void*);
                        break;
                    default:
                        CX_ERROR( CX_StsBadArg, "Unknown format specifier" );
                    }

                    storage->width += dl;
                    if( storage->width >= storage->max_width )
                    {
                        fprintf( storage->file, "\n" );
                        storage->width = 0;
                    }
                }
            }
        }
    }
    else if( format == ICX_FORMAT_BINARY )
    {
        char* buffer = storage->base64_buffer;
        char* buffer_end = storage->base64_buffer_end;

        assert( sizeof(float) == sizeof(int) &&
                sizeof(float) == 4 &&
                sizeof(double) == 8 );

        if( pl == 1 )
        {
            i = 0;

            for( ;; )
            {
                switch( dt[0] )
                {
                case 'u':
                case 'c':
                    while( i < count )
                    {
                        *buffer++ = ((char*)data)[i++];
                        if( buffer >= buffer_end )
                            break;
                    }
                    break;
                case 's':
                    while( i < count )
                    {
                        int val = ((short*)data)[i++];
                        buffer[0] = (char)(val >> 8);
                        buffer[1] = (char)val;
                        if( (buffer += 2) >= buffer_end )
                            break;
                    }
                    break;
                case 'i':
                case 'f':
                    while( i < count )
                    {
                        int val = ((int*)data)[i++];
                        buffer[0] = (char)(val >> 24);
                        buffer[1] = (char)(val >> 16);
                        buffer[2] = (char)(val >> 8);
                        buffer[3] = (char)val;
                        if( (buffer += 4) >= buffer_end )
                            break;
                    }
                    break;
                case 'd':
                    while( i < count )
                    {
                        int hi = (int)(((uint64*)data)[i] >> 32);
                        int lo = (int)(((uint64*)data)[i++]);
                        buffer[0] = (char)(hi >> 24);
                        buffer[1] = (char)(hi >> 16);
                        buffer[2] = (char)(hi >> 8);
                        buffer[3] = (char)hi;
                        buffer[4] = (char)(lo >> 24);
                        buffer[5] = (char)(lo >> 16);
                        buffer[6] = (char)(lo >> 8);
                        buffer[7] = (char)lo;
                        if( (buffer += 8) >= buffer_end )
                            break;
                    }
                    break;
                default:
                    CX_ERROR( CX_StsBadArg, "Unknown format specifier" );
                }

                if( buffer >= buffer_end )
                {
                    storage->base64_buffer = buffer;
                    icxWriteBase64Block( storage, 0 );
                    buffer = storage->base64_buffer;
                }

                if( i == count )
                    break;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                int dl = 0;
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CX_ERROR(CX_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                            }
                            if( n == 0 )
                                CX_ERROR(CX_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                    case 'c':
                        *buffer++ = *(char*)(data + k);
                        k++;
                        break;
                    case 's':
                        {
                            int val;
                            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                            val = *(short*)(data + k);
                            k += sizeof(short);
                            buffer[0] = (char)(val >> 8);
                            buffer[1] = (char)val;
                            buffer += 2;
                        }
                        break;
                    case 'i':
                    case 'f':
                        {
                            int val;
                            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                            val = *(int*)(data + k);
                            k += sizeof(int);
                            buffer[0] = (char)(val >> 24);
                            buffer[1] = (char)(val >> 16);
                            buffer[2] = (char)(val >> 8);
                            buffer[3] = (char)val;
                            buffer += 4;
                        }
                        break;
                    case 'd':
                        {
                            int hi, lo;
                            k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                            hi = (int)(*(uint64*)(data+k) >> 32);
                            lo = (int)(*(uint64*)(data+k));
                            k += sizeof(double);
                            buffer[0] = (char)(hi >> 24);
                            buffer[1] = (char)(hi >> 16);
                            buffer[2] = (char)(hi >> 8);
                            buffer[3] = (char)hi;
                            buffer[4] = (char)(lo >> 24);
                            buffer[5] = (char)(lo >> 16);
                            buffer[6] = (char)(lo >> 8);
                            buffer[7] = (char)lo;
                            buffer += 8;
                        }
                        break;
                    case 'p':
                        {
                            /* just skip it */
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            k += sizeof(void*);
                        }
                        break;
                    case 'r':
                        {
                            unsigned val;
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            val = (unsigned)(unsigned long)(*(void**)(data+k));
                            buffer[0] = (char)(val >> 24);
                            buffer[1] = (char)(val >> 16);
                            buffer[2] = (char)(val >> 8);
                            buffer[3] = (char)val;
                            k += sizeof(void*);
                            buffer += 4;
                        }
                        break;
                    default:
                        CX_ERROR( CX_StsBadArg, "Unknown format specifier" );
                    }

                    if( buffer >= buffer_end )
                    {
                        storage->base64_buffer = buffer;
                        icxWriteBase64Block( storage, 0 );
                        buffer = storage->base64_buffer;
                    }
                }
            }
        }

        storage->base64_buffer = buffer;
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "Unsupported encoding format" );
    }

    __END__;
}


static int
icxReadRawData( CxFileStorage* storage, const char* dt,
                int format, void* _data, int count )
{
    int result = 0;
    const char* data = (const char*)_data;
    const char* buffer = storage->buffer;

    CX_FUNCNAME( "icxReadRawData" );

    __BEGIN__;

    int i = 0, k = 0;
    int pl;
    char* endptr = 0;

    if( !dt || (pl = strlen(dt)) == 0 )
        EXIT; // invalid format specification

    if( format == ICX_FORMAT_TEXT )
    {
        if( pl == 1 || (pl == 2 && isdigit(dt[0])) )
        {
            if( pl == 2 )
                count *= dt[0] - '0';

            switch( dt[pl-1] )
            {
            case 'u':
                for( i = 0; i < count; i++ )
                {
                    ((uchar*)data)[i] = (uchar)icx_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'c':
                for( i = 0; i < count; i++ )
                {
                    ((char*)data)[i] = (char)icx_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 's':
                for( i = 0; i < count; i++ )
                {
                    ((short*)data)[i] = (short)icx_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'i':
                for( i = 0; i < count; i++ )
                {
                    ((int*)data)[i] = (int)icx_strtol( buffer, &endptr, 10 );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'f':
                for( i = 0; i < count; i++ )
                {
                    ((float*)data)[i] = (float)icx_strtod( buffer, &endptr );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'd':
                for( i = 0; i < count; i++ )
                {
                    ((double*)data)[i] = (double)icx_strtod( buffer, &endptr );
                    if( endptr == buffer )
                        EXIT;
                    buffer = endptr;
                }
                break;

            case 'a':
                assert( count == 1 );
                *((const char**)data) = buffer;
                break;

            default:
                EXIT;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;

                for( ;; )
                {
                    int dl = 0;

                    if( --n <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CX_ERROR(CX_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                                dl = 0;
                            }
                            if( n == 0 )
                                CX_ERROR(CX_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    switch( c )
                    {
                    case 'u':
                        *(uchar*)(data+k) = (uchar)icx_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k++;
                        break;
                    case 'c':
                        *(char*)(data+k) = (char)icx_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k++;
                        break;
                    case 's':
                        k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                        *(short*)(data+k) = (short)icx_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(short);
                        break;
                    case 'i':
                        k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                        *(int*)(data+k) = (int)icx_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(int);
                        break;
                    case 'f':
                        k = (k + sizeof(float) - 1) & -(int)sizeof(float);
                        *(float*)(data+k) = (float)icx_strtod( buffer, &endptr );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(float);
                        break;
                    case 'd':
                        k = (k + sizeof(double) - 1) & -(int)sizeof(double);
                        *(double*)(data+k) = (double)icx_strtod( buffer, &endptr );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(double);
                        break;
                    case 'p':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        *(void**)(data+k) = 0;
                        k += sizeof(void*);
                        break;
                    case 'r':
                        k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                        *(unsigned*)(data+k) = (unsigned)icx_strtol( buffer, &endptr, 10 );
                        if( endptr == buffer )
                            EXIT;
                        buffer = endptr;
                        k += sizeof(void*);
                        break;
                    default:
                        EXIT;
                    }
                }
            }
        }
    }
    else if( format == ICX_FORMAT_BINARY )
    {
        uchar* buffer = (uchar*)storage->base64_buffer;
        uchar* buffer_end = (uchar*)storage->base64_buffer_read;

        assert( sizeof(float) == sizeof(int) &&
                sizeof(float) == 4 &&
                sizeof(double) == 8 );

        if( pl == 1 )
        {
            i = 0;

            for( ;; )
            {
                switch( dt[0] )
                {
                case 'u':
                case 'c':
                    while( i < count )
                    {
                        if( buffer >= buffer_end )
                            break;
                        ((uchar*)data)[i++] = *buffer++;
                    }
                    break;
                case 's':
                    while( i < count )
                    {
                        if( buffer + 1 >= buffer_end )
                            break;
                        ((short*)data)[i++] = (short)((buffer[0] << 8) + buffer[1]);
                        buffer += 2;
                    }
                    break;
                case 'i':
                case 'f':
                    while( i < count )
                    {
                        if( buffer + 3 >= buffer_end )
                            break;
                        ((int*)data)[i++] = (buffer[0] << 24) + (buffer[1] << 16) +
                                            (buffer[2] << 8) + buffer[3];
                        buffer += 4;
                    }
                    break;
                case 'd':
                    while( i < count )
                    {
                        unsigned lo, hi;
                        if( buffer + 7 >= buffer_end )
                            break;
                        hi = (buffer[0] << 24) + (buffer[1] << 16) +
                             (buffer[2] << 8) + buffer[3];
                        lo = (buffer[4] << 24) + (buffer[5] << 16) +
                             (buffer[6] << 8) + buffer[7];
                        ((uint64*)data)[i++] = ((uint64)hi << 32) + lo;
                        buffer += 8;
                    }
                    break;
                default:
                    EXIT;
                }

                if( i < count )
                {
                    int len = buffer_end - buffer;
                    storage->base64_buffer = (char*)buffer;
                    if( !icxReadBase64Block( storage ))
                        EXIT;
                    buffer = (uchar*)storage->base64_buffer;
                    buffer_end = (uchar*)storage->base64_buffer_read;
                    if( len >= buffer_end - buffer )
                        EXIT;
                }
                else
                    break;
            }
        }
        else
        {
            for( i = 0; i < count; i++ )
            {
                int n = 0;
                char c = '\0';
                const char* dtp = dt;
                int k0 = k;

                for( ;; )
                {
                    if( (n -= (k != k0)) <= 0 )
                    {
                        if( !dtp[0] )
                            break;
                        n = 1;
                        if( isdigit(dtp[0]))
                        {
                            if( !isdigit(dtp[1]))
                            {
                                n = dtp[0] - '0';
                                dtp++;
                            }
                            else
                            {
                                int dl;
                                if( sscanf( dtp, "%d%n", &n, &dl ) <= 0 )
                                    CX_ERROR(CX_StsBadArg,
                                            "Invalid data type specification");
                                dtp += dl;
                            }
                            if( n == 0 )
                                CX_ERROR(CX_StsBadArg,
                                        "Invalid data type specification");
                        }
                        c = *dtp++;
                    }

                    k0 = k;
                    switch( c )
                    {
                    case 'u':
                    case 'c':
                        if( buffer < buffer_end )
                        {
                            *(uchar*)(data + k) = *buffer++;
                            k++;
                        }
                        break;
                    case 's':
                        if( buffer + 1 < buffer_end )
                        {
                            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
                            *(short*)(data + k) = (short)((buffer[0] << 8) + buffer[1]);
                            k += sizeof(short);
                            buffer += 2;
                        }
                        break;
                    case 'i':
                    case 'f':
                        if( buffer + 3 < buffer_end )
                        {
                            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
                            *(int*)(data + k) = (buffer[0] << 24) + (buffer[1] << 16) +
                                                (buffer[2] << 8) + buffer[3];
                            k += sizeof(int);
                            buffer += 4;
                        }
                        break;
                    case 'd':
                        if( buffer + 7 < buffer_end )
                        {
                            unsigned lo, hi;
                            hi = (buffer[0] << 24) + (buffer[1] << 16) +
                                 (buffer[2] << 8) + buffer[3];
                            lo = (buffer[4] << 24) + (buffer[5] << 16) +
                                 (buffer[6] << 8) + buffer[7];
                            k = (k + sizeof(uint64) - 1) & -(int)sizeof(uint64);
                            *(uint64*)(data + k) = ((uint64)hi << 32) + lo;
                            k += sizeof(uint64);
                            buffer += 8;
                        }
                        break;
                    case 'p':
                        {
                            /* just skip it */
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            *(void**)(data+k) = 0;
                            k += sizeof(void*);
                        }
                        break;
                    case 'r':
                        if( buffer + 3 < buffer_end )
                        {
                            unsigned val;
                            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
                            val = (buffer[0] << 24) + (buffer[1] << 16) +
                                 (buffer[2] << 8) + buffer[3];
                            *(void**)(data+k) = (void*)(size_t)val;
                            k += sizeof(void*);
                            buffer += 4;
                        }
                        break;
                    default:
                        EXIT;
                    }

                    if( k == k0 )
                    {
                        int len = buffer_end - buffer;
                        storage->base64_buffer = (char*)buffer;
                        if( !icxReadBase64Block( storage ))
                            EXIT;
                        buffer = (uchar*)storage->base64_buffer;
                        buffer_end = (uchar*)storage->base64_buffer_read;
                        if( len >= buffer_end - buffer )
                            EXIT;
                    }
                }
            }
        }

        storage->base64_buffer = (char*)buffer;
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "Unsupported encoding format" );
    }

    result = 1;

    __END__;

    if( format == ICX_FORMAT_TEXT )
        storage->buffer = (char*)buffer;

    return result;
}


static int
icxCalcSize( const char* dt )
{
    int size = 0;

    CX_FUNCNAME( "icxCalcSize" );

    __BEGIN__;

    int k = 0;

    if( !dt )
        EXIT;

    for( ;; )
    {
        char c = '\0';
        int n = 1;

        if( !*dt )
            break;
        if( isdigit(*dt))
        {
            int dl = 0;
            sscanf( dt, "%d%n", &n, &dl );
            if( n == 0 )
                CX_ERROR( CX_StsBadArg, "Invalid data type specification" );
            dt += dl;
        }
        c = *dt++;

        switch( c )
        {
        case 'u':
        case 'c':
            k += n;
            break;
        case 's':
            k = (k + sizeof(short) - 1) & -(int)sizeof(short);
            k += sizeof(short)*n;
            break;
        case 'i':
        case 'f':
            k = (k + sizeof(int) - 1) & -(int)sizeof(int);
            k += sizeof(int)*n;
            break;
        case 'd':
            k = (k + sizeof(double) - 1) & -(int)sizeof(double);
            k += sizeof(double)*n;
            break;
        case 'p':
        case 'r':
        case 'a':
            k = (k + sizeof(void*) - 1) & -(int)sizeof(void*);
            k += sizeof(void*)*n;
            break;
        default:
            CX_ERROR( CX_StsBadArg, "Unknown format specifier" );
        }
    }

    size = k;

    __END__;

    return size;
}


#define ICX_OPENING_TAG 0
#define ICX_CLOSING_TAG 1

static int
icxFindTag( CxFileStorage* storage )
{
    char* buffer = storage->buffer;
    int is_comment = 0, is_control = 0, is_quot = 0, is_directive = 0;
    int result = -1;

    for( ; buffer < storage->buffer_end; buffer++ )
    {
        int c = buffer[0];
        if( c & ~0x7f )
            goto exit_func;
        if( isspace(c))
        {
            if( c == '\n' )
            {
                storage->line++;
                storage->line_start = buffer;
            }
            continue;
        }

        switch( buffer[0] )
        {
        case '-':
            if( !is_quot )
            {
                if( is_comment )
                {
                    if( buffer[1] == '-' && buffer[2] == '>' )
                        buffer += 2, is_comment = 0;
                }
            }
            break;
        case '<':
            if( !is_quot && !is_comment )
            {
                if( buffer[1] == '!' )
                {
                    if( buffer[2] == '-' )
                    {
                        if( buffer[3] == '-' )
                            buffer += 2, is_comment = 1;
                        else
                            goto exit_func;
                    }
                    else
                        is_control++;
                }
                else if( buffer[1] == '?' )
                {
                    buffer++;
                    is_directive = 1;
                }
                else if( buffer[1] == '/' )
                {
                    result = ICX_CLOSING_TAG;
                    buffer += 2;
                    goto exit_func;
                }
                else
                {
                    result = ICX_OPENING_TAG;
                    buffer++;
                    goto exit_func;
                }
            }
            break;
        case '\"':
        case '\'':
            if( is_quot )
            {
                if( c == is_quot )
                    is_quot = 0;
            }
            else if( !is_comment )
            {
                if( is_control == 0 && is_directive == 0 )
                    goto exit_func;
                is_quot = c;
            }
            break;
        case '>':
            if( !is_quot && !is_comment )
            {
                if( --is_control < 0 )
                    goto exit_func;
            }
            break;
        case '?':
            if( !is_quot && !is_comment )
            {
                if( buffer[1] == '>' )
                    if( is_directive )
                        buffer++, is_directive = 0;
                    else
                        goto exit_func;
            }
            break;
        /*default:
            if( !is_quot && !is_comment && !is_control && !is_directive )
                goto exit_func;*/
        }
    }

exit_func:

    storage->buffer = buffer;
    return result;
}


#define ICX_NEED_TAGNAME 0
#define ICX_NEED_ATTRNAME 1
#define ICX_NEED_EQSIGN 2
#define ICX_NEED_ATTRVAL 3

static int
icxParseXML( CxFileStorage* storage )
{
    const int chunk_size = 10;
    int result = 0;

    CX_FUNCNAME( "icxParseXML" );

    __BEGIN__;

    char errbuf[100];
    char* buffer;

    buffer = storage->line_start = storage->buffer = storage->buffer_start;
    storage->line = 1;

    for(;;)
    {
        int need = ICX_NEED_TAGNAME;
        CxFileNode* node = 0;
        CxFileNode stub_node;
        int free_attr = 0;
        char** attr_arr = 0;
        CxAttrList* last_attr = 0;
        int is_id = 0, is_empty = 0;
        int c, tag;

        storage->buffer = buffer;
        tag = icxFindTag( storage );

        if( tag < 0 )
        {
            sprintf( errbuf, "Syntax error at line %d, column %d",
                     storage->line, (int)(buffer - storage->line_start) );
            CX_ERROR( CX_StsError, errbuf );
        }

        if( tag == ICX_OPENING_TAG )
        {
            CX_CALL( node = (CxFileNode*)cxSetNew( storage->nodes ));
        }
        else
            node = &stub_node;

        memset( node, 0, sizeof(*node));

        buffer = storage->buffer;

        #define process_space()                                 \
            if( c != '\n' )                                     \
                ;                                               \
            else                                                \
                storage->line++, storage->line_start = buffer

        for( ; buffer < storage->buffer_end; buffer++ )
        {
            c = buffer[0];
skip_read:
            if( isspace(c) )
            {
                process_space();
                continue;
            }

            if( isalpha(c) || c == '_' || c == ':' )
            {
                char* name_start = buffer;

                if( need != ICX_NEED_TAGNAME && need != ICX_NEED_ATTRNAME )
                    EXIT;
                do
                {
                    c = *++buffer;
                }
                while( isalnum(c) || c == '_' || c == ':' || c == '.' || c == '-' );

                if( need == ICX_NEED_TAGNAME )
                {
                    buffer[0] = '\0';
                    node->tagname = name_start;

                    if( !isspace(c) && c != '/' && c != '>' )
                        EXIT; // a space or closing angle bracket
                              // should go after tag name
                    need = ICX_NEED_ATTRNAME;
                    goto skip_read;
                }
                else if( need == ICX_NEED_ATTRNAME )
                {
                    buffer[0] = '\0';
                    is_id = strcmp( name_start, "id" ) == 0;

                    if( free_attr == 0 )
                    {
                        // allocate another chunk of attributes
                        CxAttrList* new_attr;
                        CX_CALL( new_attr = (CxAttrList*)cxMemStorageAlloc(
                                storage->memstorage,
                                sizeof(*new_attr) + (chunk_size+1)*2*sizeof(char*)));
                        new_attr->attr = (char**)(new_attr+1);
                        new_attr->next = 0;
                        if( !last_attr )
                            node->attr = new_attr;
                        else
                        {
                            *attr_arr = 0;
                            last_attr->next = new_attr;
                        }
                        last_attr = new_attr;
                        attr_arr = new_attr->attr;
                        free_attr = chunk_size;
                    }

                    *attr_arr++ = name_start;
                    free_attr--;

                    if( c == '=' )
                        need = ICX_NEED_ATTRVAL;
                    else
                    {
                        if( !isspace(c))
                            EXIT;
                        need = ICX_NEED_EQSIGN;
                        goto skip_read;
                    }
                }
            }
            else if( c == '=' )
            {
                if( need != ICX_NEED_EQSIGN )
                    EXIT; // unexpected character
                else
                    need = ICX_NEED_ATTRVAL;
            }
            else if( c == '\"' || c == '\'' )
            {
                if( need != ICX_NEED_ATTRVAL )
                    EXIT; // unexpected character
                else
                {
                    int quot_char = c;
                    char* val_start = ++buffer;
                    char* val_end = 0;

                    for( ; buffer < storage->buffer_end; buffer++ )
                    {
                        c = buffer[0];
                        if( isspace(c))
                        {
                            process_space();
                            continue;
                        }

                        if( c == quot_char )
                        {
                            val_end = buffer;
                            *val_end = '\0';
                            break;
                        }
                    }

                    if( !val_end )
                        EXIT; // the attribute value is not closed

                    *attr_arr++ = val_start;
                    if( is_id )
                    {
                        if( node->name )
                            EXIT; // repeated 'id = "..."'
                        else
                            node->name = val_start;
                    }
                    need = ICX_NEED_ATTRNAME;
                }
            }
            else if( c == '/' )
            {
                if( need != ICX_NEED_ATTRNAME )
                    EXIT; // unexpected character
                else if( buffer[1] != '>' )
                    EXIT; // '>' is expected after '/'
                else
                {
                    is_empty = 1;
                    buffer += 2;
                    break;
                }
            }
            else if( c == '>' )
            {
                if( need != ICX_NEED_ATTRNAME )
                    EXIT; // unexpected character
                else
                {
                    buffer++;
                    break;
                }
            }
            else
                EXIT; // unexpected character;
        }

        if( attr_arr )
            *attr_arr++ = 0;

        if( !node->tagname )
            EXIT; // dummy empty tag (e.g. <>)

        if( tag == ICX_CLOSING_TAG )
        {
            if( attr_arr )
                EXIT; // closing tag has attributes
            if( !storage->parent || !storage->parent->tagname ||
                strcmp( storage->parent->tagname, node->tagname ) != 0 )
                EXIT; // unmatched names in the opening and closing tags
            storage->parent = storage->parent->v_prev;
            if( storage->parent == 0 )
            {
                result = 1; // XML has been read completely and
                            // its structure seems to be correct
                break;
            }
        }
        else
        {
            char* type_name;
            if( node->name )
                if( icxQueryName( storage, node->name, node ) != node )
                    EXIT; // duplicated id
            type_name = (char*)cxAttrValue( node->attr, "type" );
            if( type_name )
                CX_CALL( node->typeinfo = cxFindType( type_name ));
            if( !storage->root )
            {
                storage->root = node;
                if( is_empty )
                    EXIT; // the first tag (<opencx_storage>) must be non-empty
            }
            else
                cxInsertNodeIntoTree( node, storage->parent, 0 );
            if( !is_empty )
                storage->parent = node;
            node->body = buffer;
        }
    }

    __END__;

    return result;
}



/************************ standard types *************************/

/*********************** CxMat or CxMatND ************************/

static int
icxIsMat( const void* ptr )
{
    return CX_IS_MAT(ptr);
}


static int
icxIsMatND( const void* ptr )
{
    return CX_IS_MATND(ptr);
}


static void
icxWriteArray( CxFileStorage* storage, const char* name,
               const void* struct_ptr, CxAttrList attr, int /*flags*/ )
{
    CX_FUNCNAME( "icxWriteArray" );

    __BEGIN__;

    char dt_buf[16];
    char size_buf[10*CX_MAX_DIM];
    int format;

    char* new_attr[10];
    int idx = 0;
    CxMat* mat = (CxMat*)struct_ptr;
    CxMatND header;
    CxNArrayIterator iterator;

    if( !icxIsMat(mat) && !icxIsMatND(mat))
        CX_ERROR( CX_StsBadArg, "The input structure is not an array" );

    new_attr[idx++] = "dt";

    sprintf( dt_buf, "%d%c", CX_MAT_CN(mat->type),
             icxTypeSymbol[CX_MAT_DEPTH(mat->type)] );
    new_attr[idx++] = dt_buf + ICX_DT_SHIFT(dt_buf);

    new_attr[idx++] = "size";
    if( CX_IS_MAT(mat))
        sprintf( size_buf, "%d %d", mat->rows, mat->cols );
    else
    {
        CxMatND* matnd = (CxMatND*)struct_ptr;
        int i, size_buf_len = 0;
        for( i = 0; i < matnd->dims; i++ )
        {
            int dl = 0;
            sprintf( size_buf + size_buf_len, "%s%d%n", i == 0 ? "" : " ",
                     matnd->dim[i].size, &dl );
            size_buf_len += dl;
        }
    }
    new_attr[idx++] = size_buf;

    new_attr[idx++] = "format";
    if( ICX_IS_BINARY(storage) )
    {
        format = ICX_FORMAT_BINARY;
        new_attr[idx++] = "xdr.base64";
    }
    else
    {
        format = ICX_FORMAT_TEXT;
        new_attr[idx++] = "text";
    }
    new_attr[idx++] = 0;

    CX_CALL( cxStartWriteStruct( storage, name, CX_IS_MAT(mat) ? "CxMat" : "CxMatND",
                                 struct_ptr, cxAttrList(new_attr, &attr)));
    CX_CALL( cxInitNArrayIterator( 1, (void**)&struct_ptr, 0, &header, &iterator ));

    iterator.size.width *= CX_MAT_CN(mat->type);

    do
    {
        CX_CALL( icxWriteRawData( storage, dt_buf + 1, format,
                                  iterator.ptr[0], iterator.size.width ));
    }
    while( cxNextNArraySlice( &iterator ));

    CX_CALL( cxEndWriteStruct( storage ));

    __END__;
}


static void*
icxReadArray( CxFileStorage* storage, CxFileNode* node )
{
    void* ptr = 0;

    CX_FUNCNAME( "icxReadArray" );

    __BEGIN__;

    int size[CX_MAX_DIM+1], dims;
    int pl, type = 0, format;
    CxMatND header;
    CxNArrayIterator iterator;
    const char* size_buf;
    const char* dt;
    const char* format_buf;
    const char* pos;
    int is_matnd;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 && node->typeinfo &&
            (strcmp( node->typeinfo->type_name, "CxMat") == 0 ||
            strcmp( node->typeinfo->type_name, "CxMatND") == 0));

    is_matnd = strcmp( node->typeinfo->type_name, "CxMatND") == 0;

    size_buf = cxAttrValue( node->attr, "size" );
    format_buf = cxAttrValue( node->attr, "format" );
    dt = cxAttrValue( node->attr, "dt" );

    if( !size_buf || !format_buf || !dt )
        EXIT;

    pl = strlen(dt);
    if( pl != 1 && (pl != 2 || !isdigit(dt[0])))
        EXIT;

    pos = strchr( icxTypeSymbol, dt[pl-1] );
    if( !pos )
        EXIT;

    type = pos - icxTypeSymbol;
    if( pl == 2 )
    {
        if( dt[0] - '0' < 1 || dt[0] - '0' > 4 )
            EXIT;
        type += (dt[0] - '0')*8;
    }

    memset( size, 0, sizeof(size));
    storage->buffer = (char*)size_buf;
    icxReadRawData( storage, "i", ICX_FORMAT_TEXT, size, CX_MAX_DIM );
    for( dims = 0; size[dims] != 0; dims++ )
        if( size[dims] < 0 )
            EXIT;

    if( dims < 1 )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICX_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICX_FORMAT_BINARY;
    else
        EXIT;

    if( is_matnd )
    {
        CX_CALL( node->content = cxCreateMatND( dims, size, type ));
    }
    else
    {
        CX_CALL( node->content = cxCreateMat( size[0], dims > 1 ? size[1] : 1, type ));
    }

    CX_CALL( cxInitNArrayIterator( 1, (void**)&node->content, 0, &header, &iterator ));

    iterator.size.width *= CX_MAT_CN(type);
    storage->buffer = (char*)node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    do
    {
        if( !icxReadRawData( storage, dt + pl - 1, format,
                             iterator.ptr[0], iterator.size.width ))
            EXIT;
    }
    while( cxNextNArraySlice( &iterator ));

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cxRelease( (void**)&node->content );

    return ptr;
}


/******************************* IplImage ******************************/

static int
icxIsImage( const void* ptr )
{
    return CX_IS_IMAGE(ptr);
}


static void
icxWriteImage( CxFileStorage* storage, const char* name,
               const void* struct_ptr, CxAttrList attr, int /*flags*/ )
{
    CX_FUNCNAME( "icxWriteImage" );

    __BEGIN__;

    char dt_buf[16];
    char width_buf[16];
    char height_buf[16];
    char roi_buf[32];
    int format, depth;
    int y, width_n;

    const char* new_attr[20];
    int idx = 0;
    IplImage* img = (IplImage*)struct_ptr;

    if( !icxIsImage(img) )
        CX_ERROR( CX_StsBadArg, "The input structure is not an IplImage" );

    if( img->nChannels > 1 && img->dataOrder == IPL_DATA_ORDER_PLANE )
        CX_ERROR( CX_StsBadArg, "Planar IplImage data layout is not supported" );

    if( img->nChannels > 4 )
        CX_ERROR( CX_BadDepth, "Unsupported number of channels" );

    depth = icxIplToCxDepth(img->depth);
    if( depth < 0 )
        CX_ERROR( CX_BadDepth, "Unsupported image depth" );

    new_attr[idx++] = "dt";
    sprintf( dt_buf, "%d%c", img->nChannels, icxTypeSymbol[depth] );

    new_attr[idx++] = dt_buf + ICX_DT_SHIFT(dt_buf);

    new_attr[idx++] = "width";
    sprintf( width_buf, "%d", img->width );
    new_attr[idx++] = width_buf;

    new_attr[idx++] = "height";
    sprintf( height_buf, "%d", img->height );
    new_attr[idx++] = height_buf;

    if( img->roi )
    {
        new_attr[idx++] = "roi";
        sprintf( roi_buf, "%d %d %d %d %d", img->roi->coi,
                 img->roi->xOffset, img->roi->yOffset,
                 img->roi->width, img->roi->height );
        new_attr[idx++] = roi_buf;
    }

    new_attr[idx++] = "origin";
    new_attr[idx++] = img->origin == IPL_ORIGIN_TL ? "tl" : "bl";

    new_attr[idx++] = "format";
    if( ICX_IS_BINARY(storage) )
    {
        format = ICX_FORMAT_BINARY;
        new_attr[idx++] = "xdr.base64";
    }
    else
    {
        format = ICX_FORMAT_TEXT;
        new_attr[idx++] = "text";
    }
    new_attr[idx++] = 0;

    CX_CALL( cxStartWriteStruct( storage, name, "IplImage",
                                 struct_ptr, cxAttrList((char**)new_attr, &attr)));

    width_n = img->width * img->nChannels;

    for( y = 0; y < img->height; y++ )
    {
        CX_CALL( icxWriteRawData( storage, dt_buf + 1, format,
                                  img->imageData + y*img->widthStep, width_n ));
    }

    CX_CALL( cxEndWriteStruct( storage ));

    __END__;
}


static void*
icxReadImage( CxFileStorage* storage, CxFileNode* node )
{
    void* ptr = 0;

    CX_FUNCNAME( "icxReadImage" );

    __BEGIN__;

    CxSize size;
    int pl, depth = 0, channels = 1, format, origin;
    CxMatND header;
    CxNArrayIterator iterator;
    const char* width_buf;
    const char* height_buf;
    const char* dt;
    const char* format_buf;
    const char* roi_buf;
    const char* origin_buf;
    const char* pos;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 &&
            node->typeinfo && strcmp( node->typeinfo->type_name, "IplImage") == 0 );

    width_buf = cxAttrValue( node->attr, "width" );
    height_buf = cxAttrValue( node->attr, "height" );
    format_buf = cxAttrValue( node->attr, "format" );
    origin_buf = cxAttrValue( node->attr, "origin" );
    dt = cxAttrValue( node->attr, "dt" );
    roi_buf = cxAttrValue( node->attr, "roi" );

    if( !width_buf || !height_buf || !format_buf || !dt || !origin_buf )
        EXIT;

    pl = strlen(dt);
    if( pl != 1 && (pl != 2 || !isdigit(dt[0])))
        EXIT;

    pos = strchr( icxTypeSymbol, dt[pl-1] );
    if( !pos )
        EXIT;

    depth = icxCxToIplDepth( pos - icxTypeSymbol );

    if( pl == 2 )
    {
        channels = dt[0] - '0';
        if( channels < 1 || channels > 4 )
            EXIT;
    }

    size.width = size.height = 0;
    storage->buffer = (char*)width_buf;
    icxReadRawData( storage, "i", ICX_FORMAT_TEXT, &size.width, 1 );
    storage->buffer = (char*)height_buf;
    icxReadRawData( storage, "i", ICX_FORMAT_TEXT, &size.height, 1 );
    if( size.width <= 0 || size.height <= 0 )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICX_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICX_FORMAT_BINARY;
    else
        EXIT;

    if( strcmp( origin_buf, "tl" ) == 0 )
        origin = IPL_ORIGIN_TL;
    else if( strcmp( origin_buf, "bl" ) == 0 )
        origin = IPL_ORIGIN_BL;
    else
        EXIT;

    CX_CALL( node->content = cxCreateImage( size, depth, channels ));
    ((IplImage*)node->content)->origin = origin;

    CX_CALL( cxInitNArrayIterator( 1, (void**)&node->content, 0, &header, &iterator ));

    iterator.size.width *= channels;
    storage->buffer = (char*)node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    do
    {
        if( !icxReadRawData( storage, dt + pl - 1, format,
                             iterator.ptr[0], iterator.size.width ))
            EXIT;
    }
    while( cxNextNArraySlice( &iterator ));

    if( roi_buf )
    {
        IplROI roi;
        storage->buffer = (char*)roi_buf;
        if( !icxReadRawData( storage, "5i", ICX_FORMAT_TEXT, &roi, 1 ))
            EXIT;
        CX_CALL( cxSetImageROI( (IplImage*)node->content,
                 cxRect( roi.xOffset, roi.yOffset, roi.width, roi.height )));
        CX_CALL( cxSetImageCOI( (IplImage*)node->content, roi.coi ));
    }

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cxRelease( (void**)&node->content );

    return ptr;
}


/******************************* CxSeq ******************************/

static int
icxIsSeq( const void* ptr )
{
    return CX_IS_SEQ(ptr) || CX_IS_SET(ptr);
}


static void
icxReleaseSeq( void** ptr )
{
    CX_FUNCNAME( "icxReleaseSeq" );

    __BEGIN__;

    if( !ptr )
        CX_ERROR( CX_StsNullPtr, "NULL double pointer" );

    *ptr = 0; // it's impossible now to release seq, so just clear the pointer

    __END__;
}


static void*
icxCloneSeq( const void* ptr )
{
    return cxSeqSlice( (CxSeq*)ptr, CX_WHOLE_SEQ,
                       0 /* use the same storage as for the original sequence */, 1 );
}


static void
icxStartWriteDynStruct( CxFileStorage* storage, const char* name,
                        const char* type_name, const void* struct_ptr,
                        const char* dt, const char* header_dt0,
                        CxAttrList attr, int header_offset )
{
    CX_FUNCNAME( "icxStartWriteDynSeq" );

    __BEGIN__;

    char header_dt_buf[64];
    char flags_buf[16];
    char name_buf[16];
    char ptr_buf[4][16];
    const char* header_dt = 0;
    int dl = 0;

    const char* new_attr[30];
    int idx = 0;
    CxSeq* seq = (CxSeq*)struct_ptr;

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", struct_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    new_attr[idx++] = "type";
    new_attr[idx++] = (char*)type_name;

    new_attr[idx++] = "flags";
    sprintf( flags_buf, "%08x", seq->flags );
    new_attr[idx++] = flags_buf;

    if( seq->h_prev )
    {
        new_attr[idx++] = "h_prev";
        sprintf( ptr_buf[0], "%p", seq->h_prev );
        new_attr[idx++] = ptr_buf[0];
    }

    if( seq->h_next )
    {
        new_attr[idx++] = "h_next";
        sprintf( ptr_buf[1], "%p", seq->h_next );
        new_attr[idx++] = ptr_buf[1];
    }

    if( seq->v_prev )
    {
        new_attr[idx++] = "v_prev";
        sprintf( ptr_buf[2], "%p", seq->v_prev );
        new_attr[idx++] = ptr_buf[2];
    }

    if( seq->v_next )
    {
        new_attr[idx++] = "v_next";
        sprintf( ptr_buf[3], "%p", seq->v_next );
        new_attr[idx++] = ptr_buf[3];
    }

    if( !cxAttrValue( &attr, "dt" ))
    {
        new_attr[idx++] = "dt";
        new_attr[idx++] = (char*)dt;
    }

    new_attr[idx++] = "format";
    new_attr[idx++] = ICX_IS_BINARY(storage) ? "xdr.base64" : "text";

    if( header_offset == 0 )
        header_offset = CX_IS_GRAPH(seq) ? sizeof(CxGraph) :
                        CX_IS_SET(seq) ? sizeof(CxSet) : sizeof(CxSeq);

    if( seq->header_size > header_offset )
    {
        header_dt = cxAttrValue( &attr, "header_dt" );
        if( !header_dt )
        {
            if( header_dt0 )
                header_dt = header_dt0;
            else
            {
                sprintf( header_dt_buf, "%du", seq->header_size - header_offset );
                header_dt = header_dt_buf;
            }
            new_attr[idx++] = "header_dt";
            new_attr[idx++] = (char*)header_dt;
        }
        else
        {
            int size;
            CX_CALL( size = icxCalcSize( header_dt ));
            if( size != seq->header_size - header_offset )
                CX_ERROR( CX_StsBadArg,
                "Sequence header data spec doesn't match "
                "to the total size of user-defined header fields" );
        }
    }

    new_attr[idx++] = 0;
    CX_CALL( icxPushXMLTag( storage, "struct", cxAttrList( (char**)new_attr, &attr ),
                            ICX_INCOMPLETE_TAG ));

    if( header_dt )
    {
        fprintf( storage->file, " header=\"%n", &dl );
        storage->width += dl;

        CX_CALL( icxWriteRawData( storage, header_dt, ICX_FORMAT_TEXT,
                                  (char*)seq + header_offset, 1 ));
        fprintf( storage->file, "\"" );
    }
    fprintf( storage->file, ">\n" );
    storage->width = 0;

    __END__;
}


static void
icxWriteSeq( CxFileStorage* storage, const char* name,
             const void* struct_ptr, CxAttrList attr, int flags )
{
    CX_FUNCNAME( "icxWriteSeq" );

    __BEGIN__;

    CxSeq* seq = (CxSeq*)struct_ptr;
    CxTreeNodeIterator iterator;

    char dt_buf[64];
    const char* dt = cxAttrValue( &attr, "dt" );
    const char* header_dt = cxAttrValue( &attr, "header_dt" );
    char header_dt_buf[64];
    int format = ICX_IS_BINARY(storage);

    if( CX_IS_SEQ(seq) && CX_IS_SEQ_POINT_SET(seq) && !header_dt )
    {
        if( seq->header_size == sizeof(CxContour) )
            header_dt = "8i";
        else
        {
            sprintf( header_dt_buf, "8i%du", (int)(seq->header_size - sizeof(CxContour)) );
            header_dt = header_dt_buf;
        }
    }

    if( !dt )
    {
        if( CX_IS_SEQ( seq ) && icxPixSize[CX_SEQ_ELTYPE(seq)] == seq->elem_size )
            sprintf( dt_buf, "%d%c", CX_MAT_CN(seq->flags),
                     icxTypeSymbol[CX_MAT_DEPTH(seq->flags)] );
        else
            sprintf( dt_buf, "%du", seq->elem_size );
        dt = dt_buf + ICX_DT_SHIFT(dt_buf);
    }
    else
    {
        int size;
        CX_CALL( size = icxCalcSize( dt ));
        if( size != seq->elem_size )
            CX_ERROR( CX_StsBadArg,
            "Sequence element data spec doesn't match to the element size" );
    }

    if( flags & CX_WRITE_TREE )
    {
        CX_CALL( cxStartWriteStruct( storage, name, "CxSeq.tree", 0, attr ));
        name = "<auto>";
        attr = cxAttrList();
        cxInitTreeNodeIterator( &iterator, seq, INT_MAX );
    }
    else
    {
        iterator.node = struct_ptr;
    }

    while( iterator.node )
    {
        CxSeq* seq1 = (CxSeq*)iterator.node;
        CX_CALL( icxStartWriteDynStruct( storage, name, "CxSeq", seq1,
                                         dt, header_dt, attr, 0 ));
        if( CX_IS_SEQ( seq1 ))
        {
            CxSeqBlock* block = seq1->first;
            do
            {
                CX_CALL( icxWriteRawData( storage, dt, format,
                                          block->data, block->count ));
                block = block->next;
            }
            while( block != seq1->first );
        }
        else
        {
            int i;
            CxSeqReader reader;
            cxStartReadSeq( seq, &reader );
            for( i = 0; i < seq->total; i++ )
            {
                if( CX_IS_SET_ELEM( reader.ptr ))
                {
                    CX_CALL( icxWriteRawData( storage, dt, format,
                                              reader.ptr, 1 ));
                }
                CX_NEXT_SEQ_ELEM( seq->elem_size, reader );
            }
        }
        CX_CALL( icxPopXMLTag( storage ));
        if( !(flags & CX_WRITE_TREE) )
            EXIT;
        cxNextTreeNode( &iterator );
    }

    CX_CALL( cxEndWriteStruct( storage ));

    __END__;
}


static void*
icxReadDynStruct( CxFileStorage* storage, CxFileNode* node,
                  int header_size0 )
{
    void* ptr = 0;

    CX_FUNCNAME( "icxReadDynStruct" );

    __BEGIN__;

    int format, flags, elem_size;
    int header_size;
    const char* dt;
    const char* format_buf;
    const char* flags_buf;
    const char* header_dt;
    const char* header_buf = 0;
    void* elem_buf = 0;

    assert( node && node->tagname && strcmp( node->tagname, "struct" ) == 0 &&
            node->typeinfo && strcmp( node->typeinfo->type_name, "CxSeq") == 0 );

    format_buf = cxAttrValue( node->attr, "format" );
    flags_buf = cxAttrValue( node->attr, "flags" );
    dt = cxAttrValue( node->attr, "dt" );
    header_dt = cxAttrValue( node->attr, "header_dt" );

    if( !format_buf || !flags_buf || !dt )
        EXIT;

    elem_size = icxCalcSize( dt );
    if( !elem_size )
        EXIT;

    if( strcmp( format_buf, "text" ) == 0 )
        format = ICX_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICX_FORMAT_BINARY;
    else
        EXIT;

    if( sscanf( flags_buf, "%x", &flags ) <= 0 )
        EXIT;

    if( header_size0 == 0 )
        header_size0 = (flags & CX_MAGIC_MASK) == CX_SEQ_MAGIC_VAL ? sizeof(CxSeq) :
                       (flags & CX_MAGIC_MASK) == CX_SET_MAGIC_VAL ? sizeof(CxSet) : 0;

    header_size = header_size0;

    if( header_dt )
    {
        header_buf = cxAttrValue( node->attr, "header" );
        int size = icxCalcSize( header_dt );
        if( !header_buf || !size )
            EXIT;
        header_size += size;
    }

    header_size = (header_size + sizeof(void*) - 1) & -(int)sizeof(void*);

    elem_buf = alloca( elem_size );
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    switch( flags & CX_MAGIC_MASK )
    {
    case CX_SEQ_MAGIC_VAL:
        {
            CxSeqWriter writer;
            CxSeq* seq;

            if( header_size0 < (int)sizeof(CxSeq))
                EXIT;

            CX_CALL( node->content = seq = cxCreateSeq( flags, header_size, elem_size,
                                                        storage->dststorage ));
            if( header_dt )
            {
                storage->buffer = (char*)header_buf;
                if( !icxReadRawData( storage, header_dt, ICX_FORMAT_TEXT,
                                     (char*)seq + header_size0, 1 ))
                    EXIT;
            }

            storage->buffer = (char*)node->body;

            cxStartAppendToSeq( seq, &writer );

            for(;;)
            {
                if( !icxReadRawData( storage, dt, format, elem_buf, 1 ))
                    break;
                CX_WRITE_SEQ_ELEM_VAR( elem_buf, writer );
            }
            cxEndWriteSeq( &writer );
        }
        break;
    case CX_SET_MAGIC_VAL:
        {
            CxSet* set;

            if( header_size0 < (int)sizeof(CxSet))
                EXIT;

            CX_CALL( node->content = set = cxCreateSet( flags, header_size, elem_size,
                                                        storage->dststorage ));

            if( header_dt )
            {
                storage->buffer = (char*)header_buf;
                if( !icxReadRawData( storage, header_dt, ICX_FORMAT_TEXT,
                                     (char*)set + header_size0, 1 ))
                    EXIT;
            }

            storage->buffer = (char*)node->body;

            for(;;)
            {
                CxSetElem* set_elem = cxSetNew( set );
                if( !icxReadRawData( storage, dt, format, set_elem, 1 ))
                {
                    cxSetRemoveByPtr( set, set_elem );
                    break;
                }

                set_elem->flags &= INT_MAX;
            }
        }
        break;
    default:
        EXIT;
    }

    ptr = (void*)node->content;

    __END__;

    if( !ptr && node )
        cxRelease( (void**)&node->content );

    return ptr;
}


static void*
icxReadSeq( CxFileStorage* storage, CxFileNode* node )
{
    void* ptr = 0;

    CX_FUNCNAME( "icxReadSeq" );

    __BEGIN__;

    if( strcmp( node->typeinfo->type_name, "CxSeq" ) == 0 )
    {
        CX_CALL( ptr = icxReadDynStruct( storage, node, 0 ));
    }
    else if( strcmp( node->typeinfo->type_name, "CxSeq.tree" ) == 0 )
    {
        CxFileNode* root = node->v_next;
        CxTreeNodeIterator iterator;

        // pass 1. make sure all the underlying contours have been read
        cxInitTreeNodeIterator( &iterator, root, INT_MAX );
        for( ; iterator.node; cxNextTreeNode( &iterator ))
        {
            CxFileNode* contour_node = (CxFileNode*)iterator.node;
            if( !contour_node->content )
            {
                assert( contour_node->typeinfo && contour_node->typeinfo->read );
                contour_node->typeinfo->read( storage, contour_node );
            }
            assert( CX_IS_SEQ( contour_node->content ));
        }

        // pass 2. restore the links
        cxInitTreeNodeIterator( &iterator, root, INT_MAX );
        for( ; iterator.node; cxNextTreeNode( &iterator ))
        {
            CxFileNode* contour_node = (CxFileNode*)iterator.node;
            CxSeq* contour = (CxSeq*)contour_node->content;
            const char* v_next = cxAttrValue( contour_node->attr, "v_next" );
            const char* v_prev = cxAttrValue( contour_node->attr, "v_prev" );
            const char* h_next = cxAttrValue( contour_node->attr, "h_next" );
            const char* h_prev = cxAttrValue( contour_node->attr, "h_prev" );

            if( h_next && !contour->h_next )
            {
                CxFileNode* t = (CxFileNode*)cxGetFileNode( storage, h_next );
                CxSeq* t_content;
                if( !t || !CX_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                t_content = (CxSeq*)t->content;
                contour->h_next = t_content;
                t_content->h_prev = contour;
                if( contour->v_prev )
                    t_content->v_prev = contour->v_prev;
                else if( t_content->v_prev )
                    contour->v_prev = t_content->v_prev;
            }

            if( h_prev && !contour->h_prev )
            {
                CxFileNode* t = (CxFileNode*)cxGetFileNode( storage, h_prev );
                CxSeq* t_content;
                if( !t || !CX_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                t_content = (CxSeq*)t->content;
                contour->h_prev = t_content;
                t_content->h_next = contour;
                if( contour->v_prev )
                    t_content->v_prev = contour->v_prev;
                else if( t_content->v_prev )
                    contour->v_prev = t_content->v_prev;
            }

            if( v_next && !contour->v_next )
            {
                CxFileNode* t = (CxFileNode*)cxGetFileNode( storage, v_next );
                if( !t || !CX_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                contour->v_next = (CxSeq*)t->content;
                ((CxSeq*)t->content)->v_prev = contour;
            }

            if( v_prev && !contour->v_prev )
            {
                CxFileNode* t = (CxFileNode*)cxGetFileNode( storage, v_prev );
                if( !t || !CX_IS_SEQ(t->content) || t->v_prev != node )
                    EXIT;
                contour->v_prev = (CxSeq*)t->content;
            }
        }

        // find the tree root
        {
        CxSeq* root_contour = (CxSeq*)root->content;
        while( root_contour->v_prev )
            root_contour = root_contour->v_prev;
        while( root_contour->h_prev )
            root_contour = root_contour->h_prev;

        node->content = ptr = root_contour;
        }
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "Unknown type name" );
    }

    __END__;

    return ptr;
}



/******************************* CxGraph ******************************/

static int
icxIsGraph( const void* ptr )
{
    return CX_IS_GRAPH(ptr);
}


static void
icxReleaseGraph( void** ptr )
{
    CX_FUNCNAME( "icxReleaseGraph" );

    __BEGIN__;

    if( !ptr )
        CX_ERROR( CX_StsNullPtr, "NULL double pointer" );

    *ptr = 0; // it's impossible now to release graph, so just clear the pointer

    __END__;
}


static void*
icxCloneGraph( const void* ptr )
{
    return cxCloneGraph( (const CxGraph*)ptr, 0 );
}


static void
icxWriteGraph( CxFileStorage* storage, const char* name,
               const void* struct_ptr, CxAttrList attr, int /*flags*/ )
{
    int* flag_buffer = 0;

    CX_FUNCNAME( "icxWriteGraph" );

    __BEGIN__;

    const char* header_dt = cxAttrValue( &attr, "header_dt" );
    const char* vtx_dt = cxAttrValue( &attr, "vtx_dt" );
    const char* edge_dt = cxAttrValue( &attr, "edge_dt" );
    char vtx_dt_buf[64];
    char edge_dt_buf[64];
    int i, k;
    int format = ICX_IS_BINARY(storage);
    CxGraph* graph = (CxGraph*)struct_ptr;
    CxSeqReader reader;

    assert( CX_IS_GRAPH(graph) );

    CX_CALL( flag_buffer = (int*)cxAlloc( graph->total*sizeof(flag_buffer[0])));

    if( !vtx_dt )
    {
        if( graph->elem_size > (int)sizeof(CxGraphVtx))
        {
            sprintf( vtx_dt_buf, "ip%du", (int)(graph->elem_size - sizeof(CxGraphVtx)));
            vtx_dt = vtx_dt_buf;
        }
        else
            vtx_dt = "ip";
    }
    else if( strncmp( vtx_dt, "ip", 2 ) != 0 )
        CX_ERROR( CX_StsBadArg, "Graph vertex format spec should "
                                "start with CxGraphVtx (ip)" );

    if( !edge_dt )
    {
        if( graph->edges->elem_size > (int)sizeof(CxGraphEdge))
        {
            sprintf( edge_dt_buf, "if2p2r%du", (int)(graph->elem_size - sizeof(CxGraphVtx)));
            edge_dt = edge_dt_buf;
        }
        else
            edge_dt = "if2p2r";
    }
    else if( strncmp( edge_dt, "if2p2r", 6 ) != 0 )
        CX_ERROR( CX_StsBadArg, "Graph vertex format spec should "
                                "start with CxGraphEdge (\"if2p2r\")" );

    CX_CALL( cxStartWriteStruct( storage, name, "CxGraph", graph, attr ));

    // pass 1. Save flags and write vertices
    CX_CALL( icxStartWriteDynStruct( storage, 0, "CxSeq", graph, vtx_dt,
                                     header_dt, cxAttrList(), 0 ));

    cxStartReadSeq( (CxSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphVtx* vtx = (CxGraphVtx*)reader.ptr;
            CX_CALL( icxWriteRawData( storage, vtx_dt, format, vtx, 1 ));
            flag_buffer[k] = vtx->flags;
            vtx->flags = k++;
        }
        CX_NEXT_SEQ_ELEM( graph->elem_size, reader );
    }

    CX_CALL( icxPopXMLTag( storage ));

    // pass 2. Write edges
    CX_CALL( icxStartWriteDynStruct( storage, 0, "CxSeq", graph->edges, edge_dt,
                                     0, cxAttrList(), 0 ));
    cxStartReadSeq( (CxSeq*)graph->edges, &reader );
    for( i = 0, k = 0; i < graph->edges->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphEdge* edge = (CxGraphEdge*)reader.ptr;
            CxGraphVtx* vtx0 = edge->vtx[0];
            CxGraphVtx* vtx1 = edge->vtx[1];
            edge->vtx[0] = (CxGraphVtx*)(size_t)(vtx0->flags);
            edge->vtx[1] = (CxGraphVtx*)(size_t)(vtx1->flags);
            CX_CALL( icxWriteRawData( storage, edge_dt, format, edge, 1 ));
            edge->vtx[0] = vtx0;
            edge->vtx[1] = vtx1;
        }
        CX_NEXT_SEQ_ELEM( graph->edges->elem_size, reader );
    }

    CX_CALL( icxPopXMLTag( storage ));

    // pass 3. Restore flags
    cxStartReadSeq( (CxSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphVtx* vtx = (CxGraphVtx*)reader.ptr;
            vtx->flags = flag_buffer[k++];
        }
        CX_NEXT_SEQ_ELEM( graph->elem_size, reader );
    }

    CX_CALL( cxEndWriteStruct( storage ));

    __END__;

    cxFree( (void**)&flag_buffer );
}


static void*
icxReadGraph( CxFileStorage* storage, CxFileNode* node )
{
    void* ptr = 0;
    CxGraphVtx** ptr_buffer = 0;

    CX_FUNCNAME( "icxReadGraph" );

    __BEGIN__;

    CxGraph* graph;
    CxSeqReader reader;
    CxFileNode *vtx_node, *edge_node;
    int i, k;
    int flags = 0;
    const int graph_mask = CX_MAGIC_MASK|CX_SEQ_KIND_MASK;
    const int graph_sign = CX_SET_MAGIC_VAL|CX_SEQ_KIND_GRAPH;

    int format, vtx_size, edge_size;
    int header_size = sizeof(CxGraph);
    const char* vtx_dt;
    const char* edge_dt;
    const char* format_buf;
    const char* flags_buf;
    const char* header_dt;
    const char* header_buf = 0;
    int count = 0;
    CxGraphVtx* vtx_buffer;
    CxGraphEdge* edge_buffer;

    vtx_node = node->v_next;
    edge_node = node->v_next->h_next;

    flags_buf = cxAttrValue( vtx_node->attr, "flags" );
    if( !flags_buf || sscanf( flags_buf, "%x", &flags ) <= 0 )
        EXIT;

    if( (flags & graph_mask) != graph_sign )
    {
        CxFileNode* t;
        CX_SWAP( vtx_node, edge_node, t );
        flags_buf = cxAttrValue( vtx_node->attr, "flags" );
        if( !flags_buf || sscanf( flags_buf, "%x", &flags ) <= 0 )
            EXIT;
        if( (flags & graph_mask) != graph_sign )
            EXIT;
    }

    vtx_dt = cxAttrValue( vtx_node->attr, "dt" );
    edge_dt = cxAttrValue( edge_node->attr, "dt" );
    header_dt = cxAttrValue( vtx_node->attr, "header_dt" );

    if( !vtx_dt || !edge_dt )
        EXIT;

    vtx_size = icxCalcSize( vtx_dt );
    edge_size = icxCalcSize( edge_dt );
    vtx_buffer = (CxGraphVtx*)alloca( vtx_size );
    edge_buffer = (CxGraphEdge*)alloca( edge_size );

    if( vtx_size < (int)sizeof(CxGraphVtx) || edge_size < (int)sizeof(CxGraphEdge))
        EXIT;

    if( header_dt )
    {
        header_buf = cxAttrValue( node->attr, "header" );
        int size = icxCalcSize( header_dt );
        if( !header_buf || !size )
            EXIT;
        header_size += size;
    }

    header_size = (header_size + sizeof(void*) - 1) & -(int)sizeof(void*);

    CX_CALL( graph = cxCreateGraph( flags, header_size, vtx_size,
                                    edge_size, storage->dststorage ));

    if( header_dt )
    {
        storage->buffer = (char*)header_buf;
        if( !icxReadRawData( storage, header_dt, ICX_FORMAT_TEXT,
                             (char*)(graph + 1), 1 ))
            EXIT;
    }

    format_buf = cxAttrValue( vtx_node->attr, "format" );
    if( strcmp( format_buf, "text" ) == 0 )
        format = ICX_FORMAT_TEXT;
    else if( strcmp( format_buf, "xdr.base64" ) == 0 )
        format = ICX_FORMAT_BINARY;
    else
        EXIT;

    storage->buffer = (char*)vtx_node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    // pass 1. read vertices
    for(;;)
    {
        if( !icxReadRawData( storage, vtx_dt, format, vtx_buffer, 1 ))
            break;
        cxGraphAddVtx( graph, vtx_buffer, 0 );
        count++;
    }

    CX_CALL( ptr_buffer = (CxGraphVtx**)cxAlloc( count*sizeof(ptr_buffer[0])));

    // pass 2: init vertices pointers
    cxStartReadSeq( (CxSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            ptr_buffer[k++] = (CxGraphVtx*)reader.ptr;
        }
        CX_NEXT_SEQ_ELEM( vtx_size, reader );
    }

    storage->buffer = (char*)edge_node->body;
    storage->base64_buffer_read = storage->base64_buffer = storage->base64_buffer_start;

    // pass 3. read edges
    for(;;)
    {
        CxGraphVtx *org, *dst;
        if( !icxReadRawData( storage, edge_dt, format, edge_buffer, 1 ))
            break;
        org = ptr_buffer[(unsigned)(size_t)edge_buffer->vtx[0]];
        dst = ptr_buffer[(unsigned)(size_t)edge_buffer->vtx[1]];
        cxGraphAddEdgeByPtr( graph, org, dst, edge_buffer, 0 );
    }

    node->content = ptr = graph;

    __END__;

    cxFree( (void**)&ptr_buffer );

    if( !ptr && node )
        cxRelease( (void**)&node->content );

    return ptr;
}

/**************************** standard type initialization *****************************/

static CxTypeInfo*
icxInitTypeInfo( CxTypeInfo* info, const char* type_name,
                 CxIsInstanceFunc is_instance, CxReleaseFunc release,
                 CxReadFunc read, CxWriteFunc write,
                 CxCloneFunc clone )
{
    info->flags = 0;
    info->header_size = sizeof(CxTypeInfo);
    info->type_name = type_name;
    info->prev = info->next = 0;
    info->is_instance = is_instance;
    info->release = release;
    info->clone = clone;
    info->read = read;
    info->write = write;

    return info;
}


static CxTypeInfo *icxFirstType = 0, *icxLastType = 0;

static void
icxCreateStandardTypes(void)
{
    CxTypeInfo info;
    CxTypeInfo dummy_first;

    icxFirstType = &dummy_first;

    cxRegisterType( icxInitTypeInfo( &info, "CxSeq", icxIsSeq, icxReleaseSeq,
                                    icxReadSeq, icxWriteSeq, icxCloneSeq ));
    cxRegisterType( icxInitTypeInfo( &info, "CxSeq.tree", icxIsSeq, icxReleaseSeq,
                                    icxReadSeq, icxWriteSeq, icxCloneSeq ));
    cxRegisterType( icxInitTypeInfo( &info, "CxGraph", icxIsGraph, icxReleaseGraph,
                                    icxReadGraph, icxWriteGraph, icxCloneGraph ));
    cxRegisterType( icxInitTypeInfo( &info, "IplImage", icxIsImage,
                                    (CxReleaseFunc)cxReleaseImage,
                                    icxReadImage, icxWriteImage,
                                    (CxCloneFunc)cxCloneImage ));
    cxRegisterType( icxInitTypeInfo( &info, "CxMat", icxIsMat, (CxReleaseFunc)cxReleaseMat,
                                     icxReadArray, icxWriteArray, (CxCloneFunc)cxCloneMat ));
    cxRegisterType( icxInitTypeInfo( &info, "CxMatND", icxIsMatND, (CxReleaseFunc)cxReleaseMatND,
                                     icxReadArray, icxWriteArray, (CxCloneFunc)cxCloneMatND ));

    /*icxLastType = icxLastType->prev;
    icxLastType->next = 0;*/
}


CX_IMPL  CxTypeInfo*
cxRegisterType( CxTypeInfo* _info )
{
    CxTypeInfo* info = 0;

    CX_FUNCNAME("cxRegisterType" );

    __BEGIN__;

    int len;

    if( !icxFirstType )
        icxCreateStandardTypes();

    if( !_info || _info->header_size != sizeof(CxTypeInfo) )
        CX_ERROR( CX_StsBadSize, "Invalid type info" );

    if( !_info->is_instance || !_info->release ||
        !_info->read || !_info->write )
        CX_ERROR( CX_StsNullPtr,
        "Some of required function pointers "
        "(is_instance, release, read or write) are NULL");

    CX_CALL( len = icxCheckString(_info->type_name) );
    CX_CALL( info = (CxTypeInfo*)cxAlloc( sizeof(*info) + len + 1 ));

    *info = *_info;
    info->type_name = (char*)(info + 1);

    strcpy( (char*)info->type_name, _info->type_name );

    info->flags = 0;
    info->next = 0;

    if( !icxLastType )
        icxFirstType = icxLastType = info;
    else
    {
        info->next = icxFirstType;
        icxFirstType = icxFirstType->prev = info;
    }

    __END__;

    return info;
}


CX_IMPL void
cxUnregisterType( const char* type_name )
{
    CX_FUNCNAME("cxUnregisterType" );

    __BEGIN__;

    CxTypeInfo* info;

    CX_CALL( info = cxFindType( type_name ));
    if( info )
    {
        if( info->prev )
            info->prev->next = info->next;
        else
            icxFirstType = info->next;

        if( info->next )
            info->next->prev = info->prev;
        else
            icxLastType = info->prev;

        if( !icxFirstType || !icxLastType )
            icxFirstType = icxLastType = 0;

        cxFree( (void**)&info );
    }

    __END__;
}


CX_IMPL CxTypeInfo*
cxFirstType( void )
{
    CxTypeInfo* info = 0;

    /*CX_FUNCNAME("cxFirstType" );*/

    __BEGIN__;

    if( !icxFirstType )
        icxCreateStandardTypes();

    info = icxFirstType;

    __END__;

    return info;
}


CX_IMPL CxTypeInfo*
cxFindType( const char* type_name )
{
    CxTypeInfo* info = 0;

    CX_FUNCNAME("cxFindType" );

    __BEGIN__;

    CX_CALL( icxCheckString( type_name ));

    if( !icxFirstType )
        icxCreateStandardTypes();

    for( info = icxFirstType; info != 0; info = info->next )
        if( strcmp( info->type_name, type_name ) == 0 )
            break;

    __END__;

    return info;
}


CX_IMPL CxTypeInfo*
cxTypeOf( const void* struct_ptr )
{
    CxTypeInfo* info = 0;

    /*CX_FUNCNAME("cxFindType" );*/

    __BEGIN__;

    if( !icxFirstType )
        icxCreateStandardTypes();

    for( info = icxFirstType; info != 0; info = info->next )
        if( info->is_instance( struct_ptr ))
            break;

    __END__;

    return info;
}


/* universal functions */
CX_IMPL void
cxRelease( void** struct_ptr )
{
    CX_FUNCNAME("cxRelease" );

    __BEGIN__;

    CxTypeInfo* info;

    if( !struct_ptr )
        CX_ERROR( CX_StsNullPtr, "NULL double pointer" );

    if( *struct_ptr )
    {
        CX_CALL( info = cxTypeOf( *struct_ptr ));
        if( !info->release )
            CX_ERROR( CX_StsError, "release function pointer is NULL" );

        CX_CALL( info->release( struct_ptr ));
        *struct_ptr = 0;
    }

    __END__;
}


void* cxClone( const void* struct_ptr )
{
    void* struct_copy = 0;

    CX_FUNCNAME("cxClone" );

    __BEGIN__;

    CxTypeInfo* info;

    if( !struct_ptr )
        CX_ERROR( CX_StsNullPtr, "NULL structure pointer" );

    CX_CALL( info = cxTypeOf( struct_ptr ));
    if( !info->clone )
        CX_ERROR( CX_StsError, "clone function pointer is NULL" );

    CX_CALL( struct_copy = info->clone( struct_ptr ));

    __END__;

    return struct_copy;
}


/********************************** High-level functions ********************************/


/* "black box" file storage */
typedef struct CxFileStorage CxFileStorage;

#define ICX_FILE_STORAGE_INPUT_FLAG_MASK (0xffff)

/* opens existing or creates new file storage */
CX_IMPL  CxFileStorage*
cxOpenFileStorage( const char* filename, CxMemStorage* memstorage, int flags )
{
    CxFileStorage* storage = 0;
    int ok = 0;

    CX_FUNCNAME("cxOpenFileStorage" );

    __BEGIN__;

    const char* mode = "rb";
    int base64_buffer_size = 1 << 10;

    CX_CALL( storage = (CxFileStorage*)cxAlloc( sizeof(*storage) ));
    memset( storage, 0, sizeof(*storage));
    storage->flags = flags & ICX_FILE_STORAGE_INPUT_FLAG_MASK;

    if( memstorage )
    {
        if( memstorage->block_size < base64_buffer_size )
            CX_ERROR( CX_StsBadSize, "destination storage has too small size" );
        storage->dststorage = memstorage;
        CX_CALL( storage->memstorage = cxCreateChildMemStorage( storage->dststorage ));
    }
    else
        CX_CALL( storage->dststorage = storage->memstorage =
                 cxCreateMemStorage( base64_buffer_size*16 ));

    CX_CALL( storage->filename = (char*)cxMemStorageAlloc( storage->memstorage,
                                                           strlen(filename) + 1 ));

    if( (flags & 3) != CX_STORAGE_READ )
        mode = "wt";

    strcpy( storage->filename, filename );
    storage->file = fopen( storage->filename, mode );
    if( !storage->file )
        EXIT;

    CX_CALL( storage->nodes = cxCreateSet( 0, sizeof(CxSet),
                   sizeof(CxFileNode), storage->memstorage));
    storage->root = storage->parent = 0;

    storage->hashsize = base64_buffer_size / sizeof(storage->hashtable[0]);
    assert( (storage->hashsize & (storage->hashsize - 1)) == 0 );
    CX_CALL( storage->hashtable = (CxFileNode**)cxMemStorageAlloc( storage->memstorage,
                                storage->hashsize*sizeof(storage->hashtable[0])));
    memset( storage->hashtable, 0, storage->hashsize*sizeof(storage->hashtable[0]));

    storage->max_width = 71;

    CX_CALL( storage->base64_buffer = storage->base64_buffer_start =
                 (char*)cxAlloc( base64_buffer_size+256 ));
    storage->base64_buffer_end = storage->base64_buffer_start + base64_buffer_size;

    if( ICX_IS_WRITE_MODE(storage) )
    {
        fprintf( storage->file, "<?xml version=\"1.0\"?>\n" );
        CX_CALL( icxPushXMLTag( storage, "opencx_storage", cxAttrList() ));
        ok = 1;
    }
    else
    {
        fseek( storage->file, 0, SEEK_END );
        int buffer_size = ftell( storage->file );
        fseek( storage->file, 0, SEEK_SET );

        CX_CALL( storage->buffer = storage->buffer_start =
                 (char*)cxAlloc( buffer_size+256 ));

        buffer_size = fread( storage->buffer_start, 1, buffer_size, storage->file );
        fclose( storage->file );
        storage->file = 0;
        storage->buffer_start[buffer_size] = '\0';
        storage->buffer_end = storage->buffer_start + buffer_size;

        CX_CALL( ok = icxParseXML( storage ));
    }

    __END__;

    if( (cxGetErrStatus() < 0 || !ok) && storage )
        cxReleaseFileStorage( &storage );

    return storage;
}


/* closes file storage and deallocates buffers */
CX_IMPL  void
cxReleaseFileStorage( CxFileStorage** pstorage )
{
    CX_FUNCNAME("cxReleaseFileStorage" );

    __BEGIN__;

    if( !pstorage )
        CX_ERROR( CX_StsNullPtr, "NULL double pointer to file storage" );

    if( *pstorage )
    {
        CxFileStorage* storage = *pstorage;
        CxTreeNodeIterator iterator;

        *pstorage = 0;

        if( storage->file )
        {
            if( ICX_IS_WRITE_MODE(storage) )
                icxPopXMLTag( storage );
            fclose( storage->file );
            storage->file = 0;
        }

        if( storage->root )
        {
            cxInitTreeNodeIterator( &iterator, storage->root, INT_MAX );
            while( iterator.node )
            {
                CxFileNode* node = (CxFileNode*)iterator.node;
                if( node->content && node->typeinfo && node->typeinfo->release )
                    node->typeinfo->release( (void**)&node->content );

                cxNextTreeNode( &iterator );
            }
        }

        cxFree( (void**)&storage->buffer_start );
        cxFree( (void**)&storage->base64_buffer_start );

        cxReleaseMemStorage( &storage->memstorage );

        memset( storage, 0, sizeof(*storage) );
        cxFree( (void**)&storage );
    }

    __END__;
}


/* writes matrix, image, sequence, graph etc. */
CX_IMPL  void
cxWrite( CxFileStorage* storage, const char* name,
         const void* struct_ptr, CxAttrList attr, int flags )
{
    CX_FUNCNAME( "cxWrite" );

    __BEGIN__;

    CxTypeInfo* info;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "NULL filestorage pointer" );

    if( !struct_ptr )
        CX_ERROR( CX_StsNullPtr, "NULL structure pointer" );

    CX_CALL( info = cxTypeOf( struct_ptr ));
    if( !info || !info->write )
        CX_ERROR( CX_StsUnsupportedFormat, "The structure type is unknown "
                  "or writing operation is not implemented" );

    CX_CALL( info->write( storage, name, struct_ptr, attr, flags ));

    __END__;
}

/* writes opening tag of a compound structure */
CX_IMPL  void
cxStartWriteStruct( CxFileStorage* storage, const char* name,
              const char* type_name, const void* struct_ptr,
              CxAttrList attr )
{
    CX_FUNCNAME( "cxStartWriteStruct" );

    __BEGIN__;

    char name_buf[16];
    char* new_attr[10];
    int idx = 0;

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", struct_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    if( type_name && type_name[0] )
    {
        new_attr[idx++] = "type";
        new_attr[idx++] = (char*)type_name;
    }

    new_attr[idx++] = 0;
    CX_CALL( icxPushXMLTag( storage, "struct", cxAttrList( new_attr, &attr ) ));

    __END__;
}


/* writes closing tag of a compound structure */
CX_IMPL  void
cxEndWriteStruct( CxFileStorage* storage )
{
    CX_FUNCNAME( "cxEndWriteStruct" );

    __BEGIN__;

    CX_CALL( icxPopXMLTag( storage ));

    __END__;
}

/* writes a scalar */
CX_IMPL  void
cxWriteElem( CxFileStorage* storage, const char* name,
             const char* elem_dt, const void* data_ptr )
{
    CX_FUNCNAME( "cxWriteElem" );

    __BEGIN__;

    char name_buf[16];
    char* new_attr[10];
    int idx = 0, dl;

    if( !data_ptr )
        CX_ERROR( CX_StsNullPtr, "NULL data pointer" );

    if( !elem_dt || !elem_dt[0] )
        CX_ERROR( CX_StsBadArg, "NULL or empty data type specification" );

    if( name && name[0] )
    {
        new_attr[idx++] = "id";
        if( strcmp(name,"<auto>") == 0 )
        {
            sprintf( name_buf, "%p", data_ptr );
            name = name_buf;
        }
        new_attr[idx++] = (char*)name;
    }

    new_attr[idx++] = "dt";
    new_attr[idx++] = (char*)elem_dt;
    /*new_attr[idx++] = "format";
    new_attr[idx++] = "text";*/
    new_attr[idx++] = 0;

    CX_CALL( icxPushXMLTag( storage, "elem", cxAttrList( new_attr, 0 ),
                            ICX_INCOMPLETE_TAG + ICX_EMPTY_TAG ));

    dl = 0;
    fprintf( storage->file, " value=\"%n", &dl );
    storage->width += dl;

    CX_CALL( icxWriteRawData( storage, elem_dt, ICX_FORMAT_TEXT, data_ptr, 1 ));
    fprintf( storage->file, "\"/>\n" );
    storage->width = 0;

    __END__;
}


/* returns index of storage item or negative number if element is not found */
CX_IMPL  CxFileNode*
cxGetFileNode( CxFileStorage* storage, const char* name )
{
    CxFileNode* node = 0;

    CX_FUNCNAME( "cxGetFileNode" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !name )
        node = storage->root;
    else
    {
        CX_CALL( node = icxQueryName( storage, name ));
    }

    __END__;

    return node;
}


/* reads matrix, image, sequence, graph etc. */
CX_IMPL  const void*
cxReadFileNode( CxFileStorage* storage, CxFileNode* node, CxAttrList** list )
{
    const void* ptr = 0;
    CxAttrList* attr_list = 0;

    CX_FUNCNAME( "cxReadFileNode" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "NULL file storage pointer" );

    if( !node )
        EXIT;

    if( !node->content )
    {
        if( strcmp( node->tagname, "elem" ) == 0 )
        {
            int size, ok;
            const char* dt = cxAttrValue( node->attr, "dt" );
            CX_CALL( size = icxCalcSize(dt));

            if( size == 0 )
                EXIT;

            node->elem_size = size;

            CX_CALL( node->content = (void*)cxMemStorageAlloc(storage->memstorage, size));

            storage->buffer = (char*)cxAttrValue( node->attr, "value" );
            CX_CALL( ok = icxReadRawData( storage, dt, ICX_FORMAT_TEXT,
                                          (void*)node->content, 1 ));
            if( !ok )
                node->content = 0;
        }
        else if( strcmp( node->tagname, "struct" ) == 0 )
        {
            if( node->typeinfo && node->typeinfo->read )
            {
                CX_CALL( node->typeinfo->read( storage, node ));
            }
            else
            {
                CxFileNode* child = node->v_next;
                // decode recursively the child nodes
                for( ; child != 0; child = child->h_next )
                    CX_CALL( cxReadFileNode( storage, child ));
            }
        }
        else
            CX_ERROR( CX_StsBadArg, "The node is neither an element nor structure" );
    }

    ptr = node->content;
    attr_list = node->attr;

    __END__;

    if( list )
        *list = attr_list;

    return ptr;
}


/* reads a scalar */
CX_IMPL  int
cxReadElem( CxFileStorage* storage, const char* name, void* dst )
{
    int result = 0;

    CX_FUNCNAME( "cxReadElem" );

    __BEGIN__;

    CxFileNode* node;

    CX_CALL( node = cxGetFileNode( storage, name ));
    if( node )
    {
        const void* ptr = node->content;

        if( strcmp( node->tagname, "elem" ) != 0 )
            EXIT;

        if( !ptr )
            CX_CALL( ptr = cxReadFileNode( storage, node ));

        if( ptr )
        {
            if( dst )
                memcpy( dst, ptr, node->elem_size );
            result = 1;
        }
    }

    __END__;

    return result;
}

/* End of file. */
