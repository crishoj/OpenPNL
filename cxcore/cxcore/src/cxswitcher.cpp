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


/****************************************************************************************/
/*                         Dynamic detection and loading of IPP modules                 */
/****************************************************************************************/

#include "_cxcore.h"

#if _MSC_VER >= 1200
#pragma warning( disable: 4115 )        /* type definition in () */
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <string.h>
#endif

#include <stdio.h>
#include <ctype.h>


typedef const char* CxProcessorType;

#define CX_PROC_GENERIC    ""
#define CX_PROC_IA32_PII   "m6"
#define CX_PROC_IA32_PIII  "a6"
#define CX_PROC_IA32_P4    "w7"

/*
   determine processor type
*/
static CxProcessorType
icxGetProcessorType( void )
{
    CxProcessorType proc_type = CX_PROC_GENERIC;

#ifdef  WIN32

    SYSTEM_INFO sys;
    GetSystemInfo( &sys );

    if( sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
        sys.dwProcessorType == PROCESSOR_INTEL_PENTIUM )
    {
        static const char cpuid_code[] =
            "\x53\x56\x57\xb8\x01\x00\x00\x00\x0f\xa2\x5f\x5e\x5b\xc3";
        typedef int64 (CX_CDECL * func_ptr)(void);
        func_ptr cpuid = (func_ptr)(void*)cpuid_code;

        int version = 0, features = 0, family = 0;
        int id = 0;

        int64 val = cpuid();
        version = (int)val;
        features = (int)(val >> 32);

        #define ICX_CPUID_M6     ((1<<15)|(1<<23)|6)  /* cmov + mmx */
        #define ICX_CPUID_A6     ((1<<25)|ICX_CPUID_M6) /* <all above> + xmm */
        #define ICX_CPUID_W7     ((1<<26)|ICX_CPUID_A6|(1<<3)|1) /* <all above> + emm */

        family = (version >> 8) & 15;
        if( family >= 6 && (features & (ICX_CPUID_M6 & ~6)) != 0 ) /* Pentium II or higher */
        {
            id = (features & ICX_CPUID_W7 & -256) | family;
        }

        switch( id )
        {
        case ICX_CPUID_W7:
            proc_type = CX_PROC_IA32_P4;
            break;
        case ICX_CPUID_A6:
            proc_type = CX_PROC_IA32_PIII;
            break;
        case ICX_CPUID_M6:
            proc_type = CX_PROC_IA32_PII;
            break;
        }
    }

#else
    char buffer[1000] = "";

    //reading /proc/cpuinfo file (proc file system must be supported)
    FILE *file = fopen( "/proc/cpuinfo", "r" );

    memset( buffer, 0, sizeof(buffer));

    if( file && fread( buffer, 1, 1000, file ))
    {
        if( strstr( buffer, "mmx" ) && strstr( buffer, "cmov" ))
        {
            proc_type = CX_PROC_IA32_PII;

            if( strstr( buffer, "xmm" ) || strstr( buffer, "sse" ))
            {
                proc_type = CX_PROC_IA32_PIII;

                if( strstr( buffer, "emm" ))
                    proc_type = CX_PROC_IA32_P4;
            }
        }
    }
#endif

    return proc_type;
}


static CxProcessorType
icxPreviousProcessor( const char* proc_type )
{
    char signature[100];
    int i;

    if( strlen( proc_type ) >= sizeof(signature))
        return CX_PROC_GENERIC;

    for( i = 0; proc_type[i]; i++ )
        signature[i] = (char)tolower( proc_type[i] );

    signature[i++] = '\0';

    if( !strcmp( signature, CX_PROC_IA32_P4 ))
        proc_type = CX_PROC_IA32_PIII;
    else if( !strcmp( signature, CX_PROC_IA32_PIII ))
        proc_type = CX_PROC_IA32_PII;
    else
        proc_type = CX_PROC_GENERIC;

    return proc_type;
}


/****************************************************************************************/
/*                               Make functions descriptions                            */
/****************************************************************************************/

#undef IPCXAPI_EX
#define IPCXAPI_EX(type,func_name,names,modules,arg) \
    { &(void*&)func_name, (void*)func_name##_f, names, modules, 0 },

static CxIPPFuncInfo cxcore_func_tab[] =
{
#undef _CXCORE_IPP_H_
#include "_cxipp.h"
#undef _CXCORE_IPP_H_
    {0, 0, 0, 0, 0}
};


/*
   determine processor type, load appropriate dll and
   initialize all function pointers
*/
#ifdef WIN32
#define DLL_PREFIX ""
#define DLL_SUFFIX ".dll"
#else
#define DLL_PREFIX "lib"
#define DLL_SUFFIX ".so"
#define LoadLibrary(name) dlopen(name, RTLD_LAZY)
#define FreeLibrary(name) dlclose(name)
#define GetProcAddress dlsym
typedef void* HMODULE;
#endif

#if 0 /*def _DEBUG*/
#define DLL_DEBUG_FLAG "d"
#else
#define DLL_DEBUG_FLAG ""
#endif

/*#define VERBOSE_LOADING*/

#ifdef VERBOSE_LOADING
#define ICX_PRINTF(args)  printf args
#else
#define ICX_PRINTF(args)
#endif

typedef struct CxModuleInfo
{
    const char* basename;
    HMODULE handle;
    char name[100];
}
CxModuleInfo;

static CxModuleInfo modules[CX_IPP_MAX];


typedef struct CxIPPTabList
{
    CxIPPFuncInfo* tab;
    struct CxIPPTabList* next;
}
CxIPPTabList;


static CxIPPTabList *icxFirstTab = 0, *icxLastTab = 0;


static int
icxUpdateIPPTab( CxIPPFuncInfo* ipp_func_tab )
{
    int i, loaded_functions = 0;
    
    // reset pointers
    for( i = 0; ipp_func_tab[i].default_func_addr != 0; i++ )
        *ipp_func_tab[i].func_addr = ipp_func_tab[i].default_func_addr;

    for( i = 0; ipp_func_tab[i].func_addr != 0; i++ )
    {
    #if _MSC_VER >= 1200
        #pragma warning( disable: 4054 4055 )   /* converting pointers to code<->data */
    #endif
        char name[100];
        int j = 0, idx = 0;

        assert( ipp_func_tab[i].loaded_from == 0 );

        if( ipp_func_tab[i].search_modules )
        {
            uchar* addr = 0;
            const char* name_ptr = ipp_func_tab[i].func_names;

            for( ; j < 10 && name_ptr; j++ )
            {
                char* name_start = strchr( name_ptr, 'i');
                char* name_end = name_start ? strchr( name_start, ',' ) : 0;
                idx = (ipp_func_tab[i].search_modules / (1<<j*3)) % CX_IPP_MAX;

                if( modules[idx].handle != 0 && name_start )
                {
                    if( name_end != 0 )
                    {
                        strncpy( name, name_start, name_end - name_start );
                        name[name_end - name_start] = '\0';
                    }
                    else
                        strcpy( name, name_start );

                    addr = (uchar*)GetProcAddress( modules[idx].handle, name );
                    if( addr )
                        break;
                }
                name_ptr = name_end;
            }

            if( addr )
            {
            /*#ifdef WIN32
                while( *addr == 0xE9 )
                    addr += 5 + *((int*)(addr + 1));
            #endif*/
                *ipp_func_tab[i].func_addr = addr;
                ipp_func_tab[i].loaded_from = idx; // store index of the module
                                                   // that contain the loaded function
                loaded_functions++;
                ICX_PRINTF(("%s: \t%s\n", name, modules[idx].name ));
            }

            #if _MSC_VER >= 1200
                #pragma warning( default: 4054 4055 )
            #endif
        }
    }
    
#ifdef VERBOSE_LOADING
    {
    int not_loaded = 0;
    ICX_PRINTF(("\nTotal loaded: %d\n\n", loaded_functions ));
    printf( "***************************************************\nNot loaded ...\n\n" );
    for( i = 0; ipp_func_tab[i].func_addr != 0; i++ )
        if( !ipp_func_tab[i].loaded_from )
        {
            ICX_PRINTF(( "%s\n", ipp_func_tab[i].func_names ));
            not_loaded++;
        }

    ICX_PRINTF(("\nTotal: %d\n", not_loaded ));
    }
#endif

    return loaded_functions;
}


CX_IMPL int
cxRegisterPrimitives( CxIPPFuncInfo* ipp_func_tab )
{
    int loaded_functions;
    CxIPPTabList* tab = (CxIPPTabList*)malloc(sizeof(*tab));

    tab->tab = ipp_func_tab;
    tab->next = 0;

    if( icxFirstTab == 0 )
        icxFirstTab = tab;
    else
        icxLastTab->next = tab;
    icxLastTab = tab;

    if( icxFirstTab == icxLastTab )
        loaded_functions = cxLoadPrimitives("");
    else
        loaded_functions = icxUpdateIPPTab( ipp_func_tab );

    return loaded_functions;
}


CX_IMPL int
cxLoadPrimitives( const char* proc_type )
{
    int i, loaded_modules = 0, loaded_functions = 0;
    CxIPPTabList* tab;

    if( !proc_type || strcmp( proc_type, CX_PROC_GENERIC ) == 0 )
        proc_type = icxGetProcessorType();

    modules[CX_IPP_NONE].basename = 0;
    modules[CX_IPP_NONE].name[0] = '\0';
    modules[CX_IPP_OPTCX].basename = "optcx";
    modules[CX_IPP_IPPCX].basename = "ippcx";
    modules[CX_IPP_IPPI].basename = "ippi";
    modules[CX_IPP_IPPS].basename = "ipps";
    modules[CX_IPP_IPPVM].basename = "ippvm";

    // try to load optimized dlls
    for( i = 1; i < CX_IPP_MAX; i++ )
    {
        CxProcessorType proc = proc_type;

        // unload previously loaded optimized modules
        if( modules[i].handle )
        {
            FreeLibrary( modules[i].handle );
            modules[i].handle = 0;
        }
        
        if( modules[i].basename )
        {
            while( strcmp( proc, CX_PROC_GENERIC ) != 0 )
            {
                sprintf( modules[i].name, DLL_PREFIX "%s%s" DLL_DEBUG_FLAG DLL_SUFFIX,
                         modules[i].basename, (const char*)proc );
            
                modules[i].handle = LoadLibrary( modules[i].name );
                if( modules[i].handle != 0 )
                {
                    ICX_PRINTF(("%s loaded\n", modules[i].name )); 
                    loaded_modules++;
                    break;
                }
            
                proc = icxPreviousProcessor( proc );
            }
            if( strcmp( proc, CX_PROC_GENERIC) == 0 )
                modules[i].name[0] = '\0';
        }
    }

    for( tab = icxFirstTab; tab != 0; tab = tab->next )
    {
        loaded_functions += icxUpdateIPPTab( tab->tab );
    }

    return loaded_functions;
}

static int loaded_functions = cxRegisterPrimitives(cxcore_func_tab);

CX_IMPL void
cxGetLibraryInfo_cxcore( const char **_version, const char **_dll_names )
{
    static const char* version = __DATE__;
    static char loaded_modules[1000] = "";

    if( _version )
        *_version = version;
    
    if( _dll_names )
    {
        int i;
        static int init_loaded_modules = 0;
        
        if( !init_loaded_modules )
        {
            init_loaded_modules = 1;
            for( i = 0; modules[i].basename; i++ )
                if( modules[i].handle != 0 )
                {
                    sprintf( loaded_modules + strlen(loaded_modules),
                             ", %s", modules[i].name );
                }
        }

        *_dll_names = strlen(loaded_modules) == 0 ? "none" :
                     (const char*)(loaded_modules + 2); // skip ", "
    }
}

/* End of file. */
