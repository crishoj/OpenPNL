/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AReferenceCounter.cpp                                       //
//                                                                         //
//  Purpose:   Definition of new class for testing reference counter       //
//               object functionality                                      //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"

static int CHECK_DESTR = 0;

class testRefCounter1 : public pnl::CNumericDenseMatrix<float>
{
public:
    
    testRefCounter1( int dim, int *range, float *data )
                     : pnl::CNumericDenseMatrix<float>(dim, range, data, 0)
    {}
    
    virtual ~testRefCounter1()
    {	
        //m_Table.clear();
        CHECK_DESTR=1;
    }

    int GetListSize() const
    {
        return GetNumOfReferences();
    }
};
