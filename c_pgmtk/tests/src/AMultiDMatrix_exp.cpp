/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMultiDMatrix_exp.cpp                                       //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <math.h>

PNL_USING

static char func_name[] = "testMatrix_expanded";

static char* test_desc = "Initialize a matrix_expanded";

static char* test_class = "Algorithm";

int testMatrix_expanded()
{
    
    int ret = TRS_OK;
    int dim = 0;
    int i;
    float eps = 1e-4f;
    while(dim <= 0)
    {
        trsiRead( &dim, "12", "matrix dimension" );
    }
    int data_size = 1 << dim;
    int* range = (int*)trsGuardcAlloc( dim, sizeof(int) );
    
    for (i = 0; i < dim; range[i] = 2,  i++);
    float* data = (float*)trsGuardcAlloc( data_size, sizeof(float) );
    for (i = 0; i < data_size; data[i] = i * 1.0f, i++);
    
    CNumericDenseMatrix<float>* m1 = CNumericDenseMatrix<float>::Create(dim, range, data);
    int length;
    const float *m1data;
    m1->GetRawData(&length, &m1data);
    
    for(i = 0; i < length; i++)
    {	
        // Test the values...
        if(m1data[i] != data[i])
        {
            ret = TRS_FAIL;
            break;
        }
    }
    int nline = 1;
    int* pconvIndexes = new int[dim];
    for (i = dim - 1; i >=0;  i--)
    {
        pconvIndexes[i] = nline;
        nline = nline * range [i];
    }
    int *multidimindexes = new int[dim];
    int k;
    div_t result;
    float val; 
    for (i = 0; i < length; i++)
    {
        val = m1->GetElementByOffset(i);
        if ( val != data[i])
        {
            ret = TRS_FAIL;
            break;
        }
        m1->SetElementByOffset(val+10, i);
        float val1 = m1->GetElementByOffset(i);
        if ( val1 != val + 10)
        {
            ret = TRS_FAIL;
            break;
        }
        m1->SetElementByOffset(val, i);
        
        //counting multi indexes from i
        int hres=i;
        for( k = 0; k < dim; k++)
        {
            result=div(hres, pconvIndexes[k]);
            multidimindexes[k]=result.quot;
            hres=result.rem;
        }
        val = m1->GetElementByIndexes(multidimindexes);
        if ( val != data[i] )
        {
            ret = TRS_FAIL;
            break;
        }
        m1->SetElementByIndexes(val+10, multidimindexes);
        val1 = m1->GetElementByIndexes(multidimindexes);
        if ( val1 != val + 10)
        {
            ret = TRS_FAIL;
            break;
        }
        m1->SetElementByIndexes(val, multidimindexes);
    }
    
    delete m1;
    
    float data2[16]={1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 0.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 0.0f, 2.0f};
    
    int  range2[2]={4, 4};
    
    C2DNumericDenseMatrix<float>* m2 = C2DNumericDenseMatrix<float>::Create( range2, data2);
    
    int NumOfDims;	// =m2->GetNumberDims();
    const int *range_get;
    m2->GetRanges(&NumOfDims, &range_get);
    
    if(NumOfDims != 2) 
    {
        ret = TRS_FAIL; 
    }
    
    int k1;
    for (k1=0; k1 < NumOfDims; k1++)
    {
        if (range_get[k1] != range2[k1])
        {
            ret = TRS_FAIL;
            break;
        }
    }
    
    const pnl::floatVector * vec=m2->GetVector();
    
    if (vec->size() != 16) { ret = TRS_FAIL; }
    
    for (int k2=0; k2<16; k2++)
    {
        if((*vec)[k2] != data2[k2])
        {
            ret = TRS_FAIL;
            break;
        }
    }
    
    float det = m2->Determinant();
    if( fabs(det-240.0f)>0.001f )
    {
        ret = TRS_FAIL;
    }
    float dataInv[16] = { -0.58333333333333f, -0.25f, 0.5f,-0.33333333333333f,0.64166666666667f, 0.375f,-0.65f, 0.66666666666667f, -0.13333333333333f,0.0f,0.2f,-0.33333333333333f, 0.175f,-0.125f, 0.05f, 0.0f};
    
    CNumericDenseMatrix<float> *invMat2 = m2->Inverse();
    const pnl::floatVector *invec = invMat2->GetVector();
    for( k1 = 0; k1 < 16; k1++ )
    {
        if( fabs((*invec)[k1]- dataInv[k1])>0.001f )
        {
            ret = TRS_FAIL;
            break;
        }
    }
    float transdata[16] = {	1.0f,5.0f,9.0f,5.0f,2.0f,6.0f,8.0f,4.0f,3.0f,7.0f,7.0f,0.0f,4.0f,0.0f,6.0f,2.0f};
    CNumericDenseMatrix<float>* matTransp = m2->Transpose();
    const pnl::floatVector *vecTransp = matTransp->GetVector();
    for( k1 = 0; k1 < 16; k1++ )
    {
        if( fabs((*vecTransp)[k1]- transdata[k1])>0.001f )
        {
            ret = TRS_FAIL;
            break;
        }
    }
    //partition matrix into blocks
    intVector blockSizes = intVector(3);
    blockSizes[0] = 1;
    blockSizes[1] = 2;
    blockSizes[2] = 1;
    intVector X = intVector(2);
    X[0] = 1;
    X[1] = 2;
    C2DNumericDenseMatrix<float> *matX = NULL;
    C2DNumericDenseMatrix<float> *matY = NULL;
    C2DNumericDenseMatrix<float> *matXY = NULL;
    C2DNumericDenseMatrix<float> *matYX = NULL;
    m2->GetBlocks( &X.front(), X.size(), &blockSizes.front(), blockSizes.size(),
        &matX, &matY, &matXY, &matYX);
    const pnl::floatVector *vecX = matX->GetVector();
    const pnl::floatVector *vecY = matY->GetVector();
    const pnl::floatVector *vecXY = matXY->GetVector();
    const pnl::floatVector *vecYX = matYX->GetVector();
    float datX[9] = {6.0f, 7.0f, 0.0f, 8.0f, 7.0f, 6.0f, 4.0f, 0.0f, 2.0f};
    float datY = 1.0f;
    float datXY[3] = {5.0f, 9.0f, 5.0f};
    float datYX[3] = {2.0f, 3.0f, 4.0f};
    if( vecX->size() != 9 )
    {
        ret = TRS_FAIL;
    }
    else
    {
        for( k1 = 0; k1 < 9; k1++ )
        {
            if( fabs( (*vecX)[k1] - datX[k1] )>0.001f )
            {
                ret = TRS_FAIL;
                break;
            }
        }
    }
    if( vecY->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    else
    {
        if( fabs( (*vecY)[0] - datY )>0.001f )
        {
            ret  = TRS_FAIL;
        }
    }
    if( vecXY->size() != 3 )
    {
        ret = TRS_FAIL;
    }
    else
    {
        for( k1 = 0; k1 < 3; k1++ )
        {
            if( fabs( (*vecXY)[k1] - datXY[k1] )>0.001f )
            {
                ret = TRS_FAIL;
                break;
            }
        }
    }
    if( vecYX->size() != 3)
    {
        ret = TRS_FAIL;
    }
    else
    {
        for( k1 = 0; k1 < 3; k1++ )
        {
            if( fabs( (*vecYX)[k1] - datYX[k1] )>0.001f )
            {
                ret = TRS_FAIL;
                break;
            }
        }
    }
    //check symmetry
    int isSym = m2->IsSymmetric(eps);
    if( isSym )
    {
        //the matrix doesn't symmetric
        ret = TRS_FAIL;
    }
    C2DNumericDenseMatrix<float>* m3 = C2DNumericDenseMatrix<float>::
                                                    CreateIdentityMatrix(4);
    isSym = m3->IsSymmetric(eps);
    if( !isSym )
    {
        //identity matrix is symmetric
        ret = TRS_FAIL;
    }

    delete m3;
    delete invMat2;
    delete matTransp;
    delete matX;
    delete matY;
    delete matXY;
    delete matYX;
    
    
    int range_memory_flag = trsGuardCheck( range );
    int data_memory_flag = trsGuardCheck( data );
    
    trsGuardFree( range );
    trsGuardFree( data );
    
    if(range_memory_flag || data_memory_flag)
    {
        return trsResult( TRS_FAIL, "Dirty memory");
    }
    delete [] pconvIndexes;
    delete [] multidimindexes;
    delete m2;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Bad matrix data");
}

void initAMultiDMatrix_expanded()
{
    trsReg(func_name, test_desc, test_class, testMatrix_expanded);
}
