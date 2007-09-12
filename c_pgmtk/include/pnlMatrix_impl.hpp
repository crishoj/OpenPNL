/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Copyright (c) 2007 The Massachusetts Institute of Technology            //
//                                                                         //
// Permission is hereby granted, free of charge, to any person obtaining   //
// a copy of this software and associated documentation files (the         //
// "Software"), to deal in the Software without restriction, including     //
// without limitation the rights to use, copy, modify, merge, publish,     //
// distribute, sublicense, and/or sell copies of the Software, and to      //
// permit persons to whom the Software is furnished to do so, subject to   //
// the following conditions:                                               //
//                                                                         //
// The above copyright notice and this permission notice shall be          //
// included in all copies or substantial portions of the Software.         //
//                                                                         //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,         //
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF      //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  //
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    //
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       //
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  //
//                                                                         //
//  File:      pnlMatrix_impl.hpp                                          //
//                                                                         //
//  Purpose:   Definition of some CMatrix member functions                 //
//                                                                         //
//  Author(s): Brian Milch                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//pnlMatrix_impl.hpp

#ifndef __PNLMATRIX_IMPL_HPP__
#define __PNLMATRIX_IMPL_HPP__

#include "pnlMatrix.hpp"
#include "pnlLog.hpp"

PNL_USING

template<class Type> 
void pnl::CMatrix<Type>::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
    dump << "Matrix of dimension ";

    int numDim;
    const int *ranges;
    GetRanges(&numDim, &ranges);
    for (int i = 0; i < numDim; ++i) {
	dump << ranges[i];
	if (i + 1 < numDim) {
	    dump << "x";
	}
    }
    dump << "\n";

    intVector indices;
    const Type *value;
    CMatrixIterator<Type> *matIter = InitIterator();
    for (; IsValueHere(matIter); Next(matIter)) 
    {
	dump << "    ";

	Index(matIter, &indices);
	for (intVector::iterator idxIter = indices.begin(); 
	     idxIter != indices.end(); ++idxIter) 
	{
	    dump << *idxIter << " ";
	}

	value = Value(matIter);
       	dump << ": " << *value << "\n";
    }
    delete matIter;
}

#endif //__PNLMATRIX_IMPL_HPP__
