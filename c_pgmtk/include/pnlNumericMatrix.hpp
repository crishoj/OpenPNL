/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNumericMatrix.hpp                                        //
//                                                                         //
//  Purpose:   CNumericMatrix template class definition & implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLNUMERICMATRIX_HPP__
#define __PNLNUMERICMATRIX_HPP__

#include "pnlMatrix.hpp"
/*
PNL_BEGIN

template <class Type> class PNL_API CNumericMatrix : public virtual CMatrix<Type>
{
public:
    virtual CMatrix<Type>* Clone() const = 0;
    virtual void MultiplyInSelf( const CNumericMatrix<Type>* matToMult,
        int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
        const Type uniVal = Type(0) ) = 0;
    virtual void DivideInSelf( const CNumericMatrix<Type>* matToDiv,
        int numDimsToDiv, const int* indicesToDivInSelf ) = 0;
    virtual void GetIndicesOfMaxValue( intVector* indices )const = 0;
    virtual CNumericMatrix<Type>* NormalizeAll() const = 0;
    virtual Type SumAll(int byAbsValue) const = 0;
    ~CNumericMatrix(){};
protected:
    CNumericMatrix( int Clamp );
};
PNL_END

PNL_USING
template <class Type>
CNumericMatrix<Type>::CNumericMatrix( int Clamp )
                        :CMatrix<Type>(Clamp)
{
}
*/
#endif //__PNLNUMERICMATRIX_HPP__
