/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistArray.hpp                                         //
//                                                                         //
//  Purpose:   Saving/Loading for arrays of simple type.                   //
//             Array must have operator<< and operator>> for std::stream   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPERSISTARRAY_HPP__
#define __PNLPERSISTARRAY_HPP__

#include "pnlConfig.hpp"
#include <ctype.h>
#include <sstream>
#include "pnlContextPersistence.hpp"

PNL_BEGIN

template<typename Type>
void SaveArray(pnlString &buf, const Type *pArray, int nElement, char delim = ' ')
{
    buf << '[';
    for(int i = 0; i < nElement; ++i)
    {
        buf << pArray[i] << delim;
    }
    buf << ']';
}

template<typename Type>
void SaveArray(CContextSave &rContext, const Type *pArray, int nElement, char delim = ' ')
{
    pnlString buf;
    
    SaveArray(buf, pArray, nElement, delim);
    rContext.AddText(buf.c_str());
}

template<typename Type>
void LoadArray(std::istringstream &buf, pnlVector<Type> &array, char delim = ' ')
{
    char ch;
    int ich;
    Type val;

    buf >> ch;
    ASSERT(ch == '[' || !buf.good());
    array.reserve(16);
    array.resize(0);
    for(;buf.good();)
    {
        do
        {
            ich = buf.peek();
            if(ich == ']')
            {
                buf >> ch;
                return;
            }
            else if(ich == delim || (delim == ' ' && isspace(ich)))
            {
                buf.get(ch);
            }
        } while(buf.good() && (ich == delim || (delim == ' ' && isspace(ich))));

	if(!buf.good())
	{
	    break;
	}

        buf >> val;
        array.push_back(val);
    }
}

template<typename Type> pnlVector<Type>*
LoadArray(CContextLoad &rContext, pnlVector<Type> *pArray = 0, char delim = ' ')
{
    if(!pArray)
    {
        pArray = new pnlVector<Type>();
    }

    pnlString str;

    rContext.GetText(str);

    std::istringstream buf(str.c_str());

    LoadArray(buf, *pArray, delim);

    return pArray;
}

PNL_END

#endif // __PNLPERSISTARRAY_HPP__
