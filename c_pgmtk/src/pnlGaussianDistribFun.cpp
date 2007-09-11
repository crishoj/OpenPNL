/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianDistribFun.cpp                                   //
//                                                                         //
//  Purpose:   CGaussianDistribFun class member functions implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlEvidence.hpp"
#include "pnlException.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnl2DNumericDenseMatrix.hpp"
#include "pnlLog.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <sstream>
#include "pnlRng.hpp"

PNL_USING

//using namespace std;

CGaussianDistribFun* CGaussianDistribFun::
CreateInMomentForm( int isPotential, int NumberOfNodes,
		   const CNodeType *const* NodeTypes, const float *dataMean,
		   const float *dataCov, const float **dataWeight )
{
    PNL_CHECK_IS_NULL_POINTER( NodeTypes );
    
    int allNodesContinious = 1;
    for( int i=0; i<NumberOfNodes; i++ )
    {
        if( (NodeTypes[i]->IsDiscrete()) && ( NodeTypes[i]->GetNodeSize() != 1 ) )
        {
            allNodesContinious = 0;
            break;
        }
    }
    if( !allNodesContinious )
    {
        PNL_THROW( CInconsistentType,
	    "dtGaussian - not all nodes continious" )
	    //some nodes are tabular and not observed - it cannot be Gaussian distrib fun
    }
    else
    {
        CGaussianDistribFun *newData = new CGaussianDistribFun( isPotential,
	    NumberOfNodes, NodeTypes, dataMean, dataCov, dataWeight );
        return newData;
    }
}

CGaussianDistribFun* CGaussianDistribFun::CreateInCanonicalForm(
								int NumberOfNodes, const CNodeType *const* NodeTypes,
								const float *dataH, const float *dataK, float g )
{
    PNL_CHECK_IS_NULL_POINTER( NodeTypes );
    
    int allNodesContinious = 1;
    for( int i=0; i<NumberOfNodes; i++ )
    {
        if(  NodeTypes[i]->IsDiscrete() && ( NodeTypes[i]->GetNodeSize() != 1 )   )
        {
            allNodesContinious = 0;
            break;
        }
    }
    if( !allNodesContinious )
    {
        PNL_THROW( CInconsistentType,
	    "dtGaussian - not all nodes continious" )
	    //some nodes are tabular and not observed - it cannot be Gaussian distrib fun
    }
    else
    {
        CGaussianDistribFun * newData = new CGaussianDistribFun( NumberOfNodes, 
            NodeTypes, dataH, dataK, g);
        return newData;
    }
}

CGaussianDistribFun* CGaussianDistribFun::CreateDeltaDistribution( 

                int NumberOfNodes, const CNodeType *const* nodeTypes,
                const float *dataMean, int isMoment, int isPotential )
{
    PNL_CHECK_IS_NULL_POINTER( nodeTypes );
    
    int allNodesContinious = 1;
    for( int i=0; i<NumberOfNodes; i++ )
    {
        if(  nodeTypes[i]->IsDiscrete() && ( nodeTypes[i]->GetNodeSize() != 1 )  )
        {
            allNodesContinious = 0;
            break;
        }
    }
    if( !allNodesContinious )
    {
        PNL_THROW( CInconsistentType,
	    "dtGaussian - not all nodes continious" )
	    //some nodes are tabular and not observed- it cannot be Gaussian distrib fun
    }
    else
    {
        CGaussianDistribFun *resData = new CGaussianDistribFun( NumberOfNodes, nodeTypes,
            dataMean, isMoment, isPotential );
        return resData;
    }
}

CGaussianDistribFun* CGaussianDistribFun::CreateUnitFunctionDistribution( 
									 int NumberOfNodes, const CNodeType *const*nodeTypes,
									 int isPotential, int isCanonical )
{
    PNL_CHECK_IS_NULL_POINTER( nodeTypes );
    
    int allNodesContinious = 1;
    for( int i=0; i<NumberOfNodes; i++ )
    {
        if(  nodeTypes[i]->IsDiscrete() && ( nodeTypes[i]->GetNodeSize() != 1 )  )
        {
            allNodesContinious = 0;
            break;
        }
    }
    if( !allNodesContinious )
    {
        PNL_THROW( CInconsistentType,"dtGaussian - not all nodes continious" );
        //some nodes are tabular and not observed - it cannot be Gaussian distrib fun
    }
    else
    {
        if( !isPotential && isCanonical )
        {
            PNL_THROW( CInconsistentType, 
		"unit function CPD must be in moment form only as every potential" );
        }
        CGaussianDistribFun *resData = new CGaussianDistribFun( NumberOfNodes, 
            nodeTypes, isPotential, isCanonical);
        return resData;
    }
}

CGaussianDistribFun* CGaussianDistribFun::Copy( const CGaussianDistribFun* pInpDistr )
{
    PNL_CHECK_IS_NULL_POINTER( pInpDistr );
    
    CGaussianDistribFun *retDistr = new CGaussianDistribFun( *pInpDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retDistr );
    
    return retDistr;
}

CDistribFun* CGaussianDistribFun::Clone() const
{
    if( !m_bCanonical && !m_bMoment )
    {
        PNL_THROW( CInvalidOperation, "clone invalid data" );
    }
    CGaussianDistribFun *resData = CGaussianDistribFun::CreateInMomentForm( m_bPotential, m_NumberOfNodes,
	&m_NodeTypes.front(), NULL, NULL, NULL);
    resData->m_bCanonical = m_bCanonical;
    resData->m_bMoment = m_bMoment;    
    resData->m_bUnitFunctionDistribution = m_bUnitFunctionDistribution;
    resData->m_bDeltaFunction = m_bDeltaFunction;
    resData->m_g = m_g;
    resData->m_normCoeff = m_normCoeff;
    resData->m_numberOfDims = m_numberOfDims;
    bool isMultByDelta = (IsDistributionSpecific() == 3 )? 1:0;
    //if the distribution specific - multiplied by delta,
    //it can have ill-conditioned or with zero determinant covariance matrix
    if( m_bUnitFunctionDistribution )
    {
        return resData;
    }
    //we can check every matrix
    if( m_pMatrixMean )
    {
        resData->AttachMatrix(m_pMatrixMean->Clone(), matMean);
    }
    if( m_bDeltaFunction )
    {
        return resData;
    }
    if( !m_bPotential )
    {
        if( m_NumberOfNodes > 1 )
        {
            for( int i = 0; i < m_NumberOfNodes - 1; i++ )
            {
                if( m_pMatricesWeight[i] )
                {
                    resData->AttachMatrix(m_pMatricesWeight[i]->Clone(),
			matWeights, i);
                }
            }
        }
    }
    else
    {
        if( m_posOfDeltaMultiply.size() > 0 )
        {
            //we need to copy multiplied data
            resData->m_posOfDeltaMultiply = intVector( 
                m_posOfDeltaMultiply.begin(), m_posOfDeltaMultiply.end() );
            resData->m_meanValuesOfMult = floatVector( m_meanValuesOfMult.begin(),
                m_meanValuesOfMult.end() );
            resData->m_offsetToNextMean = intVector( m_offsetToNextMean.begin(),
                m_offsetToNextMean.end() );
        }
    }
    if( m_pMatrixCov )
    {
        resData->AttachMatrix(m_pMatrixCov->Clone(),matCovariance,-1,NULL, isMultByDelta);
    }
    if( m_pMatrixH )
    {
        resData->AttachMatrix(m_pMatrixH->Clone(), matH);
    }
    if( m_pMatrixK )
    {
        resData->AttachMatrix(m_pMatrixK->Clone(), matK,-1,NULL, isMultByDelta);
    }
    
    return resData;
}

CDistribFun* CGaussianDistribFun::CloneWithSharedMatrices()
{
    if( !m_bCanonical && !m_bMoment )
    {
	PNL_THROW( CInvalidOperation, "clone invalid data" );
    }
    CGaussianDistribFun *resData = CGaussianDistribFun::CreateInMomentForm( m_bPotential, m_NumberOfNodes,
	&m_NodeTypes.front(), NULL, NULL, NULL );
    resData->m_bCanonical = m_bCanonical;
    resData->m_bMoment = m_bMoment;    
    resData->m_bUnitFunctionDistribution = m_bUnitFunctionDistribution;
    resData->m_bDeltaFunction = m_bDeltaFunction;
    resData->m_g = m_g;
    resData->m_normCoeff = m_normCoeff;
    resData->m_numberOfDims = m_numberOfDims;
    bool isMultByDelta = (IsDistributionSpecific() == 3 )? 1:0;
    //if the distribution specific - multiplied by delta,
    //it can have ill-conditioned or with zero determinant covariance matrix
    if( m_bUnitFunctionDistribution )
    {
        return resData;
    }
    //we can check every matrix
    if( m_pMatrixMean )
    {
        resData->AttachMatrix(m_pMatrixMean,matMean);
    }
    if( m_bDeltaFunction )
    {
        return resData;
    }
    if( !m_bPotential )
    {
        if( m_NumberOfNodes > 1 )
        {
            for( int i = 0; i < m_NumberOfNodes - 1; i++ )
            {
                if( m_pMatricesWeight[i] )
                {
                    resData->AttachMatrix(m_pMatricesWeight[i],matWeights, i);
                }
            }
        }
    }
    else
    {
	if( m_posOfDeltaMultiply.size() > 0 )
	{
	    //we need to copy multiplied data
	    resData->m_posOfDeltaMultiply = intVector( 
		m_posOfDeltaMultiply.begin(), m_posOfDeltaMultiply.end() );
	    resData->m_meanValuesOfMult = floatVector( m_meanValuesOfMult.begin(),
		m_meanValuesOfMult.end() );
	    resData->m_offsetToNextMean = intVector( m_offsetToNextMean.begin(),
		m_offsetToNextMean.end() );
	}
    }
    if( m_pMatrixCov )
    {
        resData->AttachMatrix(m_pMatrixCov,matCovariance,-1,NULL,isMultByDelta);
    }
    if( m_pMatrixH )
    {
        resData->AttachMatrix(m_pMatrixH, matH);
    }
    if( m_pMatrixK )
    {
        resData->AttachMatrix(m_pMatrixK, matK,-1,NULL, isMultByDelta);
    }
    return resData;
}

//typeOfMatrices == 1 - randomly created matrices
void CGaussianDistribFun::CreateDefaultMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    if( m_bMoment || m_bCanonical )
    {
        //the distribution already have valid form, it doesn't need matrices
        PNL_THROW( CInvalidOperation,
            "can't create default matrices - distribution function already has valid form" );
    }
    //create in moment form
    //find sizes
    floatVector mean;
    mean.assign( m_numberOfDims, 0.0f );
    pnlRand( m_numberOfDims, &mean.front(), -5.0f, 5.0f );
    void* pObj = this;
    if( m_pMatrixMean )
    {
        PNL_THROW( CInvalidOperation,
            "can't create default matrix, the distribution already has it" );
    }
    intVector dims;
    dims.assign( 2, 1 );
    dims[0] = m_numberOfDims;
    m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( &dims.front(),
        &mean.front() );
    m_pMatrixMean->AddRef(pObj);
    if( m_pMatrixCov )
    {
        PNL_THROW( CInvalidOperation,
            "can't create default matrix, the distribution already has it" );
    }
    m_pMatrixCov = C2DNumericDenseMatrix<float>::CreateIdentityMatrix( 
        m_numberOfDims );
    m_pMatrixCov->AddRef( pObj );
    if( !m_bPotential )
    {
        //need to create weight matrices of corresponding sizes
        int i;
        for( i = 0; i < m_NumberOfNodes - 1; i++ )
        {
            if( m_pMatricesWeight[i] )
            {
                PNL_THROW( CInvalidOperation,
		    "can't create default matrix, the distribution already has it" );                
            }
            dims[0] = m_numberOfDims;
            dims[1] = m_NodeTypes[i]->GetNodeSize();
            floatVector weight;
            weight.resize(dims[0]*dims[1]);
            pnlRand( weight.size(), &weight.front(), -3.0f, 3.0f );
            m_pMatricesWeight[i] = C2DNumericDenseMatrix<float>::Create( 
                &dims.front(), &weight.front() );
            m_pMatricesWeight[i]->AddRef(pObj);
        }
    }
    CheckMomentFormValidity();    
}



//constructor for gaussian data in moment form (both CPD & factor)
//number of nodes - number of nodes in domain,
//number of dimensions - is the same for CPD, sum of all dims for factor
CGaussianDistribFun::CGaussianDistribFun( int isPotential, int NumberOfNodes,
					 const CNodeType *const* nodeTypes,
					 const float *dataMean,
					 const float *dataCov, 
					 const float **dataWeight):
CDistribFun(dtGaussian, NumberOfNodes, nodeTypes, 0),
m_bMoment(0), m_bCanonical(0), m_bDeltaFunction(0)
{
    if( NumberOfNodes <= 0 )
    {
        PNL_THROW( COutOfRange, "number of nodes" );
    }
    int i;
    int NumberOfDimensions = 0;
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_freedomDegreeCov = -1;
    m_freedomDegreeMean = -1;
    m_pPseudoCountsMean = NULL;
    m_pPseudoCountsCov = NULL;
    m_pMatrixMean = NULL;
    m_pMatrixCov = NULL;
    m_pMatricesWeight = NULL;
    m_pMatrixH = NULL;
    m_pMatrixK = NULL;
    m_normCoeff = 0.0f;
    m_g = FLT_MAX;
    m_offsetToNextMean.push_back(0);
    if( isPotential )
    {
        m_bPotential = 1;
        for( i = 0; i < NumberOfNodes; i++ )
        {
            if (!nodeTypes[i]->IsDiscrete())
                NumberOfDimensions += nodeTypes[i]->GetNodeSize();
        }
    }
    else
    {
        m_bPotential = 0;
        NumberOfDimensions = nodeTypes[NumberOfNodes - 1]->GetNodeSize();
        if( NumberOfNodes - 1 > 0 )
        {
            m_pMatricesWeight = new C2DNumericDenseMatrix<float>*[NumberOfNodes - 1];
            PNL_CHECK_IF_MEMORY_ALLOCATED( m_pMatricesWeight );
            for( i = 0; i < NumberOfNodes - 1; i++ )
            {
                m_pMatricesWeight[i] = NULL;
            }
        }
    }
    m_NumberOfNodes = NumberOfNodes;
    m_numberOfDims = NumberOfDimensions;
    void *pObj = this;
    //we keep it to have multinomial Gaussian Distribution in such space
    int *ranges = new int [2];
    PNL_CHECK_IF_MEMORY_ALLOCATED( ranges );
    if( dataMean )
    {
        ranges[0] = NumberOfDimensions;
        ranges[1] = 1;
        m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( ranges, dataMean );
        static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
    }
    if( dataCov )
    {
        ranges[1] = NumberOfDimensions;
        m_pMatrixCov = C2DNumericDenseMatrix<float>::Create(  ranges, dataCov );
        if( !m_pMatrixCov->IsSymmetric(1e-3f) )
        {
            PNL_THROW( CInvalidOperation, "covariance isn't symmetric" )
        }
        static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
    }
    if( !nodeTypes )
    {
        PNL_THROW( CNULLPointer, "nodeTypes" );
    }
    if( dataMean && dataCov )
    {
        int isIllMatrix = m_pMatrixCov->IsIllConditioned();
        if( isIllMatrix )
        {
            PNL_THROW( CInvalidOperation,
                "delta function is a special case - we create it by special function" );
        }
        float detCov = m_pMatrixCov->Determinant();
        if( detCov <= 0 ) 
        {
            PNL_THROW( CInvalidOperation,
                "delta function is a special case - we create it by special function" );
        }
        else
        {
            float p1 = (float)pow((double)( 2 * PNL_PI ),
                ( NumberOfDimensions / 2.0 ));
            /*float detCov = m_pMatrixCov->Determinant();
            if( detCov < 0 )
            {
                PNL_THROW( CInternalError,
                    "covariance matrix must be positive semidifinite" );
            }*/
            float p2 = (float)pow((double)detCov,0.5);
            m_normCoeff = 1/(p1*p2);
        }
    }
    switch( isPotential )
    {
    case 1:
        {
            m_bPotential = 1;
            m_pMatricesWeight = NULL;
            break;
        }
    case 0:
        {
            m_bPotential = 0;
            if( dataWeight )
            {
                int numParents = NumberOfNodes - 1;
                if( numParents>0 )
                {
                    ranges[0] = NumberOfDimensions;
                    for( int i = 0; i <numParents; i++ )              
                    {
                        if( dataWeight[i] )
                        {
                            ranges[1] = nodeTypes[i]->GetNodeSize();
                            m_pMatricesWeight[i] = 
                                C2DNumericDenseMatrix<float>::Create( ranges,
                                dataWeight[i] );
                            static_cast<CMatrix<float>*>(
                                m_pMatricesWeight[i])->AddRef(pObj);
                        }
                        else
                        {
                            PNL_THROW( CNULLPointer, "dataWeight[i]" );
                        }
                    }
                }
                else
                {
                    m_pMatricesWeight = NULL;
                }
            }
            break;
        }
    default:
        {
            PNL_THROW( CBadConst, "isFactor should be 0 or 1" );
        }
    }
    delete []ranges;
    
}
//constructor in canonical form
CGaussianDistribFun::CGaussianDistribFun( int NumberOfNodes, 
					 const CNodeType *const* nodeTypes,
					 const float *dataH,
					 const float *dataK,
					 float g):
CDistribFun( dtGaussian, NumberOfNodes, nodeTypes, 0 ), 
m_bMoment(0), m_bCanonical(0),m_bDeltaFunction(0)
{
    m_pMatrixMean = NULL;
    m_pMatrixCov = NULL;
    m_pMatricesWeight = NULL;
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_freedomDegreeCov = -1;
    m_freedomDegreeMean = -1;
    m_pPseudoCountsMean = NULL;
    m_pPseudoCountsCov = NULL;
    m_pMatrixH = NULL;
    m_pMatrixK = NULL;
    m_bPotential = 1;
    if( NumberOfNodes <= 0 )
    {
        PNL_THROW( COutOfRange, "number of nodes" );
    }
    int i;
    int NumberOfDimensions = 0;
    for( i = 0; i < NumberOfNodes; i++ )
    {
        if (!nodeTypes[i]->IsDiscrete())
            NumberOfDimensions += nodeTypes[i]->GetNodeSize();
    }
    m_NumberOfNodes = NumberOfNodes;
    m_numberOfDims = NumberOfDimensions;
    void *pObj = this;
    //we keep it to have multinomial Gaussian Distribution in such space
    //we set this flag so because of creating in canonical form
    if( dataH && dataK )
    {
        int *ranges = new int[2];
        PNL_CHECK_IF_MEMORY_ALLOCATED( ranges );
        ranges[1] = 1;
        ranges[0] = NumberOfDimensions;
        m_pMatrixH = C2DNumericDenseMatrix<float>::Create( ranges, dataH );
        static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
        float Hsum = m_pMatrixH->SumAll(1);
        ranges[1] = NumberOfDimensions;
        m_pMatrixK = C2DNumericDenseMatrix<float>::Create( ranges, dataK );
        if( !m_pMatrixK->IsSymmetric(1e-3f) )
        {
            PNL_THROW( CInvalidOperation, "K should be symmetric" )
        }
        static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
        float Ksum = m_pMatrixK->SumAll(1);
        if( g >= FLT_MAX )
        {
            PNL_THROW( COutOfRange, "g" );
        }
        m_g = g;
        if( (Hsum < 0.0001f) && (Ksum < 0.0001f) && ( fabs(m_g) < 0.001f ) )
        {
            PNL_THROW( CInvalidOperation, 
                "uniform distribution is a special case" );
        }
        delete []ranges;
        m_bCanonical = 1;
        //UpdateMomentForm();
    }
    
}
//constructor for Delta function
CGaussianDistribFun::CGaussianDistribFun( int NumberOfNodes,
                                   const CNodeType *const* nodeTypes,
                                   const float *dataMean, int isMoment, int isPotential ):
CDistribFun(dtGaussian, NumberOfNodes, nodeTypes, 0),
m_bMoment(0), m_bCanonical(0)
{
    if( NumberOfNodes <= 0 )
    {
        PNL_THROW( COutOfRange, "number of nodes" );
    }
    if(!((isMoment == 1)||(isMoment == 0)))
    {
        PNL_THROW( COutOfRange, "flag is only 0 or 1 " )
    }
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_freedomDegreeCov = -1;
    m_freedomDegreeMean = -1;
    m_pPseudoCountsMean = NULL;
    m_pPseudoCountsCov = NULL;
    int i;
    void *pObj = this;
    m_bPotential = isPotential;
    m_bDeltaFunction = 1;
    m_bUnitFunctionDistribution = 0;
    m_bMoment = isMoment ? 1 : 0 ;
    m_bCanonical = 1 - m_bMoment;
    int NumberOfDimensions = 0;
    for( i = 0; i < NumberOfNodes; i++ )
    {
        if (!nodeTypes[i]->IsDiscrete())
            NumberOfDimensions += (nodeTypes[i])->GetNodeSize();
    }
    m_numberOfDims = NumberOfDimensions;
    m_NumberOfNodes = NumberOfNodes;
    intVector ranges(2,1);
    ranges[0] = NumberOfDimensions;
    if( dataMean )
    {
        m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( &ranges.front(), dataMean );
        static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef( pObj );
    }
    else
    {
        PNL_THROW( CInvalidOperation, "delta function must have a mean" )
    }
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_pMatrixCov = NULL;
    m_pMatricesWeight = NULL;
    m_pMatrixH = NULL;
    m_pMatrixK = NULL;
    m_g = FLT_MAX;
    m_normCoeff = 0.0f;
}

//constructor for unit function distribution
CGaussianDistribFun::CGaussianDistribFun( int NumberOfNodes,
					 const CNodeType *const*nodeTypes, int isPotential,
					 int isCanonical ):
CDistribFun(dtGaussian, NumberOfNodes, nodeTypes, 1),
m_bMoment(0), m_bCanonical(0)
{
    if( NumberOfNodes <= 0 )
    {
        PNL_THROW( COutOfRange, "number of nodes" );
    }
    if( (isCanonical != 1) && (isCanonical != 0) )
    {
        PNL_THROW( COutOfRange, "flag is only 0 or 1" )
    }
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_freedomDegreeCov = -1;
    m_freedomDegreeMean = -1;
    m_pPseudoCountsMean = NULL;
    m_pPseudoCountsCov = NULL;
    int NumberOfDimensions = 0;
    if( isPotential )
    {
        for( int i = 0; i < NumberOfNodes; i++ )
        {
            if (!nodeTypes[i]->IsDiscrete())
                NumberOfDimensions += (nodeTypes[i])->GetNodeSize();
        }
    }
    else
    {
        NumberOfDimensions = m_NumberOfNodes;
    }
    m_numberOfDims = NumberOfDimensions;
    m_bPotential = isPotential? 1:0;
    m_bDeltaFunction = 0;
    m_bCanonical = isCanonical? 1 : 0 ;
    m_bMoment = 1 - m_bCanonical;
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_pMatrixMean = NULL;
    m_pMatrixCov = NULL;
    m_pMatricesWeight = NULL;
    m_pMatrixH = NULL;
    m_pMatrixK = NULL;
    m_g = 0.0f;
}

//copy constructor
CGaussianDistribFun::CGaussianDistribFun( const CGaussianDistribFun & inpDistr )
:CDistribFun( dtGaussian )
{
    m_NumberOfNodes = inpDistr.m_NumberOfNodes;
    m_NodeTypes.assign( inpDistr.m_NodeTypes.begin(),
        inpDistr.m_NodeTypes.end());
    m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;
    
    m_numberOfDims = inpDistr.m_numberOfDims;
    
    m_bPotential = inpDistr.m_bPotential;
    m_bCanonical = inpDistr.m_bCanonical;
    m_bMoment = inpDistr.m_bMoment;
    m_bDeltaFunction = inpDistr.m_bDeltaFunction;
    
    m_g = inpDistr.m_g;
    m_normCoeff = inpDistr.m_normCoeff;
    
    m_meanValuesOfMult.assign( inpDistr.m_meanValuesOfMult.begin(), 
        inpDistr.m_meanValuesOfMult.end() );
    m_posOfDeltaMultiply.assign( inpDistr.m_posOfDeltaMultiply.begin(),
        inpDistr.m_posOfDeltaMultiply.end() );
    m_offsetToNextMean.assign( inpDistr.m_offsetToNextMean.begin(), 
        inpDistr.m_offsetToNextMean.end() );
    m_pMatrixK = NULL;
    m_pMatrixH = NULL;
    m_pMatrixMean = NULL;
    m_pMatrixCov = NULL;
    m_pMatricesWeight = NULL;
    m_pLearnMatrixMean = NULL;
    m_pLearnMatrixCov = NULL;
    m_freedomDegreeCov = -1;
    m_freedomDegreeMean = -1;
    m_pPseudoCountsMean = NULL;
    m_pPseudoCountsCov = NULL;
    if( m_bUnitFunctionDistribution )
    {
        return;
    }
    void *pObj = this;
    //we can check every matrix
    if( inpDistr.m_pMatrixMean )
    {
        m_pMatrixMean = C2DNumericDenseMatrix<float>::Copy(inpDistr.m_pMatrixMean);
        static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
    }
    if( m_bDeltaFunction )
    {
        return;
    }
    if( inpDistr.m_pMatrixCov )
    {
        m_pMatrixCov = C2DNumericDenseMatrix<float>::Copy(inpDistr.m_pMatrixCov);
        static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
    }
    if( inpDistr.m_pMatrixH )
    {
        m_pMatrixH = C2DNumericDenseMatrix<float>::Copy(    inpDistr.m_pMatrixH );
        static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
    }
    if( inpDistr.m_pMatrixK )
    {
        m_pMatrixK = C2DNumericDenseMatrix<float>::Copy( inpDistr.m_pMatrixK );
        static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
    }
    if( inpDistr.m_pMatricesWeight )
    {
        int numParents = inpDistr.m_NumberOfNodes -1 ;
        m_pMatricesWeight = new C2DNumericDenseMatrix<float>*[numParents];
        for( int i = 0; i < numParents; i++ )
        {
            if( inpDistr.m_pMatricesWeight[i] )
            {
                m_pMatricesWeight[i] = C2DNumericDenseMatrix<float>::Copy( 
                    inpDistr.m_pMatricesWeight[i] );
                static_cast<CMatrix<float>*>(m_pMatricesWeight[i])->AddRef(pObj);
            }
        }
    }
}

void CGaussianDistribFun::ReleaseAllMatrices()
{
    void *pObj = this;
    if( m_pMatrixMean )
    {
        static_cast<CMatrix<float>*>(m_pMatrixMean)->Release(pObj);
        m_pMatrixMean = NULL;
    }
    if( m_pMatrixCov )
    {
        static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
        m_pMatrixCov = NULL;
    }
    if( m_pPseudoCountsMean )
    {
        static_cast<CMatrix<float>*>(m_pPseudoCountsMean)->Release(pObj);
        m_pPseudoCountsMean = NULL;
    }
    if( m_pPseudoCountsCov )
    {
        static_cast<CMatrix<float>*>(m_pPseudoCountsCov)->Release(pObj);
        m_pPseudoCountsCov = NULL;
    }
    if( m_bPotential )
    {
        if( m_pMatrixH )
        {
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
        }
        if( m_pMatrixK )
        {
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
        }
    }
    else
    {
        if( m_pMatricesWeight )
        {
            for( int i = 0; i < m_NumberOfNodes - 1; i++ )
            {
                if( m_pMatricesWeight[i] )
                {
                    static_cast<CMatrix<float>*>(
                        m_pMatricesWeight[i])->Release(pObj);
                    m_pMatricesWeight[i] = NULL;
                }
            }
            delete []m_pMatricesWeight;
            m_pMatricesWeight = NULL;
        }
    }
}

CGaussianDistribFun::~CGaussianDistribFun()
{
    if( m_pLearnMatrixMean )
    {
        delete m_pLearnMatrixMean;
        m_pLearnMatrixMean = NULL;
    }
    if( m_pLearnMatrixCov )
    {
        delete m_pLearnMatrixCov;
        m_pLearnMatrixCov = NULL;
    }
    ReleaseAllMatrices();
}

CDistribFun& CGaussianDistribFun::operator =( const CDistribFun& pInputDistr )
{
    if( this == &pInputDistr  )
    {
        return *this;
    }
    if( pInputDistr.GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInvalidOperation, 
            "input distribution must be Gaussian " );
    }
    //both distributions must be on the same node types
    const CGaussianDistribFun &pGInputDistr = 
        static_cast<const CGaussianDistribFun&>(pInputDistr);
    int isTheSame = 1;
    if( pGInputDistr.m_numberOfDims != m_numberOfDims )
    {
        isTheSame = 0;
    }
    if( m_NumberOfNodes != pGInputDistr.m_NumberOfNodes )
    {
        isTheSame = 0;
    }
    int i;
    const pConstNodeTypeVector* ntInput = pInputDistr.GetNodeTypesVector();
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
        if( (*ntInput)[i] != m_NodeTypes[i] )
        {
            isTheSame = 0;
            break;
        }
    }
    if( m_bPotential != pGInputDistr.m_bPotential )
    {
        isTheSame = 0;
    }
    if( !isTheSame )
    {
        PNL_THROW(CInvalidOperation,
            "both distributions must be on the same nodes");
    }
    //if both distributions are in the same form - all is easy
    int sameMoment = ( m_bMoment == pGInputDistr.m_bMoment )? 1:0;
    int sameCanonical = (m_bCanonical == pGInputDistr.m_bCanonical)? 1:0;
    int sameUnit = (m_bUnitFunctionDistribution == pGInputDistr.m_bUnitFunctionDistribution )? 1:0;
    int sameDelta = ( m_bDeltaFunction == pGInputDistr.m_bDeltaFunction )? 1:0;
    if( sameUnit && sameDelta )
    {
        if( m_bUnitFunctionDistribution )
        {
            m_bCanonical = pGInputDistr.m_bCanonical;
            m_bMoment = pGInputDistr.m_bMoment;
            return *this;
        }
        if( m_bDeltaFunction )
        {
            m_bCanonical = pGInputDistr.m_bCanonical;
            m_bMoment = pGInputDistr.m_bMoment;
            (*m_pMatrixMean) = *(pGInputDistr.m_pMatrixMean);
            m_normCoeff = pGInputDistr.m_normCoeff;
            return *this;
        }
        if( sameCanonical && sameMoment )
        {
            if( m_bPotential )
            {
                
                if( m_bMoment )
                {
                    m_normCoeff = pGInputDistr.m_normCoeff;
                    (*m_pMatrixMean) = *(pGInputDistr.m_pMatrixMean);
                    (*m_pMatrixCov) = *( pGInputDistr.m_pMatrixCov );
                }
                if( m_bCanonical )
                {
                    m_g = pGInputDistr.m_g;
                    (*m_pMatrixH) = *(pGInputDistr.m_pMatrixH);
                    (*m_pMatrixK) = *(pGInputDistr.m_pMatrixK);
                }
            }
            else
            {
                m_normCoeff = pGInputDistr.m_normCoeff;
                (*m_pMatrixMean) = *(pGInputDistr.m_pMatrixMean);
                (*m_pMatrixCov) = *(pGInputDistr.m_pMatrixCov);
                for( i = 0; i < m_NumberOfNodes-1; i++ )
                {
                    (*m_pMatricesWeight[i]) = 
                        *(pGInputDistr.m_pMatricesWeight[i]);
                }
            }
        }
        else
        {
            PNL_THROW( CInvalidOperation,
                "non special types of distributions (non delta, non unit) must be in the same form" );
        }
    }
    void* pObj = this;
    //the delta distributions and unit function distributions haven't weight matrices
    if( !sameDelta )
    {
        //if only one of them is delta distribution 
        //we need change self without any supporting their forms (moment, canonical) 
        if( m_bDeltaFunction )//pGInputDistr.m_bDeltaFunction == 0
        {
            //copy data from the mean matrix and clone all other matrices
            m_bDeltaFunction = 0;
            if( pGInputDistr.m_bMoment )
            {
                (*m_pMatrixMean) = (*pGInputDistr.m_pMatrixMean);
                m_normCoeff = pGInputDistr.m_normCoeff;
                m_pMatrixCov = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixCov->Clone());
                m_pMatrixCov->AddRef( pObj );
            }
            if( pGInputDistr.m_bCanonical )
            {
                m_pMatrixMean->Release(pObj);
                m_pMatrixMean = NULL;
                m_g = pGInputDistr.m_g;
                m_pMatrixH = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixH->Clone());
                m_pMatrixH->AddRef( pObj );
                m_pMatrixK = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixK->Clone());
                m_pMatrixK->AddRef( pObj );
            }
        }
        else //this isn't delta, other - it is
        {
            //this distribution can't be CPD - only potentials can be delta and its the same here
            m_bDeltaFunction = 1;
            if( m_bMoment )
            {
                (*m_pMatrixMean) = (*pGInputDistr.m_pMatrixMean);
                m_normCoeff = pGInputDistr.m_normCoeff;
                m_pMatrixCov->Release(pObj);
                m_pMatrixCov = NULL;
            }
            else
            {
                m_pMatrixH->Release(pObj);
                m_pMatrixH = NULL;
                m_pMatrixK->Release( pObj );
                m_pMatrixK = NULL;
                m_g = FLT_MAX;
                m_normCoeff = pGInputDistr.m_normCoeff;
                m_pMatrixMean = static_cast<C2DNumericDenseMatrix<float>*>(
                    pGInputDistr.m_pMatrixMean->Clone());
                m_pMatrixMean->AddRef(pObj);
            }
            
        }
        m_bMoment = pGInputDistr.m_bMoment;
        m_bCanonical = pGInputDistr.m_bCanonical;
        //return *this;
    }
    if( !sameUnit )
    {
        if( m_bUnitFunctionDistribution )
        {
            m_bUnitFunctionDistribution = 0;
            m_bCanonical = pGInputDistr.m_bCanonical;
            m_bMoment = pGInputDistr.m_bMoment;
            if( pGInputDistr.m_bDeltaFunction )
            {
                m_pMatrixMean = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixMean->Clone());
                m_pMatrixMean->AddRef( pObj );
                return *this;
            }
            if( m_bMoment )
            {
                m_normCoeff = pGInputDistr.m_normCoeff;
                m_pMatrixMean = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixMean->Clone());
                m_pMatrixMean->AddRef( pObj );
                m_pMatrixCov =  static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixCov->Clone());
                m_pMatrixCov->AddRef( pObj );
                if( pGInputDistr.m_pMatricesWeight )
                {
                    int numParents = m_NumberOfNodes - 1;
                    m_pMatricesWeight = new C2DNumericDenseMatrix<float> *[numParents];
                    PNL_CHECK_IF_MEMORY_ALLOCATED( m_pMatricesWeight );
                    for( i = 0; i < numParents; i++ )
                    {
                        m_pMatricesWeight[i] = static_cast<
                            C2DNumericDenseMatrix<float>*>(
                            pGInputDistr.m_pMatricesWeight[i]->Clone());
                        m_pMatricesWeight[i]->AddRef(pObj);
                    }
                }
            }
            if( m_bCanonical )
            {
                m_g = pGInputDistr.m_g;
                m_pMatrixH = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixH->Clone());
                m_pMatrixH->AddRef( pObj );
                m_pMatrixK = static_cast<C2DNumericDenseMatrix<
                    float>*>(pGInputDistr.m_pMatrixK->Clone());
                m_pMatrixK->AddRef( pObj );
            }
        }
        else //the other distribution is unit, this - not
        {
            m_bUnitFunctionDistribution = 1;
            m_g = FLT_MAX;
            m_normCoeff = 0.0f;
            if( m_pMatrixMean )  
            {
                m_pMatrixMean->Release(pObj);
                m_pMatrixMean = NULL;
            }
            if( m_pMatrixCov )
            {
                m_pMatrixCov->Release(pObj);
                m_pMatrixCov = NULL;
            }
            if( m_pMatrixH )
            {
                m_pMatrixH->Release(pObj);
                m_pMatrixH = NULL;
            }
            if( m_pMatrixK )
            {
                m_pMatrixK->Release(pObj);
                m_pMatrixK = NULL;
            }
            if( m_pMatricesWeight )
            {
                for( i = 0; i < m_NumberOfNodes-1; i++ )
                {
                    m_pMatricesWeight[i]->Release( pObj );
                }
                m_pMatricesWeight = NULL;
            }
        }
    }
    //we also need to set positions multiplied by delta if there are such
    if( !m_posOfDeltaMultiply.empty() )
    {
	//need to release it
	m_posOfDeltaMultiply.clear();
	m_offsetToNextMean.clear();
	m_meanValuesOfMult.clear();
    }
    if( !pGInputDistr.m_posOfDeltaMultiply.empty() )
    {
	m_posOfDeltaMultiply.assign( 
	    pGInputDistr.m_posOfDeltaMultiply.begin(),
	    pGInputDistr.m_posOfDeltaMultiply.end() );
	m_offsetToNextMean.assign( pGInputDistr.m_offsetToNextMean.begin(),
	    pGInputDistr.m_offsetToNextMean.end() );
	m_meanValuesOfMult.assign( pGInputDistr.m_meanValuesOfMult.begin(), 
	    pGInputDistr.m_meanValuesOfMult.end());
    }
    return *this;
}

void CGaussianDistribFun::SetCoefficient( float  coeff, int isG )
{
    if( !isG )
    {
        m_normCoeff = coeff;
        CheckMomentFormValidity();
    }
    else
    {
        m_g = coeff;
        CheckCanonialFormValidity();
    }
}

void CGaussianDistribFun::UpdateCanonicalCoefficient()
{
    if(m_bCanonical && !IsDistributionSpecific())
    {
        if( m_pMatrixK && m_pMatrixH )
        {
			//float det = m_pMatrixK->Determinant();
			//if(det < 1e-6f)
			//{
				//PNL_THROW( CInconsistentType,
				//	"K matrix must be positive semidifinite" );
			//}
			int oldMoment = m_bMoment;
			UpdateMomentForm();
			C2DNumericDenseMatrix<float>* matTr = m_pMatrixMean->Transpose();
			C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply(matTr,
				m_pMatrixK, 0);
			delete matTr;
			C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply(matTemp1,
				m_pMatrixMean, 0);
			delete matTemp1;
			int size;
			const float *val;
			matTemp2->GetRawData( &size, &val);
			if( size != 1 )
			{
				PNL_THROW( CInternalError, "it should be a single value" );
			}
			m_g = (float)log(m_normCoeff) - 0.5f * val[0];
			delete matTemp2;
			if( !oldMoment )
			{
				m_bMoment = 0;
				void *pObj = this;
				m_pMatrixMean->Release(pObj);
				m_pMatrixMean = NULL;
				m_pMatrixCov->Release(pObj);
				m_pMatrixCov = NULL;
				m_normCoeff = 0.0f;
			}
		}
    }
	else
	{
		m_g = FLT_MAX;
	}
}

int CGaussianDistribFun::GetMultipliedDelta( const int **positions, 
					    const float **values, const int **offsets ) const
{
    if( !m_bPotential )
    {
        PNL_THROW( CInvalidOperation, "we can multiply only factors" );
    }
    if( m_posOfDeltaMultiply.size() == 0 )
    {
        (*positions) = NULL;
        (*values) = NULL;
        (*offsets) = NULL;
        return 0;
    }
    else
    {
        (*positions) = &m_posOfDeltaMultiply.front();
        (*values) = &m_meanValuesOfMult.front();
        (*offsets) = &m_offsetToNextMean.front();
        return m_posOfDeltaMultiply.size();
    }
}


void CGaussianDistribFun::AllocMatrix( const float *data, EMatrixType mType, 
				      int numberOfWeightMatrix,
				      const int *parentIndices) 
{
    PNL_CHECK_IS_NULL_POINTER( data );
    
    if(( m_bDeltaFunction )&&( mType != matMean ))
    {
        PNL_THROW( CInvalidOperation, 
            "delta function have only matrix mean" )
    }
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, 
            "uniform distribution have no matrices" )
    }
    //check node sizes with node types
    int i;
    void *pObj = this;
    
    switch( m_bPotential )
    {
    case 1:
        {
            if( mType == matWeights )
            {
                PNL_THROW( CInconsistentType, "matrix type" );
            }
            //check node sizes and sum all them
            int dimSizes = 0;
            for( i = 0; i < m_NumberOfNodes; i++ )
            {
               if (!m_NodeTypes[i]->IsDiscrete())
                   dimSizes += m_NodeTypes[i]->GetNodeSize();
            }
            intVector dims;
            dims.assign( 2, 1 );
            dims[0] = dimSizes;
            if( mType == matMean )
            {
                if( m_pMatrixMean )
                {
                    static_cast<CMatrix<float>*>(
                        m_pMatrixMean)->Release(pObj);
                    m_pMatrixMean = NULL;
                }
                m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( &dims.front(), data );
                static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
            }
            if( mType == matCovariance )
            {
                if( m_pMatrixCov )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                    m_pMatrixCov = NULL;
                }
                dims[1] = dimSizes;
                m_pMatrixCov = C2DNumericDenseMatrix<float>::Create( &dims.front(), data );
                float det = m_pMatrixCov->Determinant();
                if( det < 0 )
                {
                    delete m_pMatrixCov;
                    PNL_THROW( CInconsistentType,
                        "covariance matrix must be positive semidifinite" );
                }
                if( !m_pMatrixCov->IsSymmetric(1e-3f) )
                {
                    delete m_pMatrixCov;
                    PNL_THROW( CInvalidOperation, 
                        "covariance should be symmetric" )
                }
                int isIllMatrix = m_pMatrixCov->IsIllConditioned();
                if( isIllMatrix )
                {
                    PNL_THROW( CInvalidOperation,
                        "delta function is a special case - we create it by special function" );
                }
                static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
                break;
            }
            if( mType == matH )
            {
                if( m_pMatrixH )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
                    m_pMatrixH = NULL;
                }
                m_pMatrixH = C2DNumericDenseMatrix<float>::Create( &dims.front(), data );
                static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                break;
            }
            if ( mType == matK )
            {
                if( m_pMatrixK )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                    m_pMatrixK = NULL;
                }
                dims[1] = dimSizes;
                m_pMatrixK = C2DNumericDenseMatrix<float>::Create( &dims.front(), data );
                float det = m_pMatrixK->Determinant();
                if( det < 0 )
                {
                    PNL_THROW( CInconsistentType,
                        "K matrix must be positive semidifinite" );
                }
                if( !m_pMatrixK->IsSymmetric(1e-3f))
                {
                    static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                    PNL_THROW( CInvalidOperation, 
                        "K should be symmetric and well conditioned" )
                }
                static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
            }
            break;
        }
    case 0:
        {
            if(( mType == matH )||( mType == matK ))
            {
                PNL_THROW( CInconsistentType, 
                    "matrix type" );
            }
            int dim = m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize();
            int* dims = new int[2];
            PNL_CHECK_IF_MEMORY_ALLOCATED( dims );
            dims[0] = dim;
            dims[1] = 1;
            if( mType == matMean )
            {
                if( m_pMatrixMean )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixMean)->Release(pObj);
                    m_pMatrixMean = NULL;
                }
                m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( dims, data );
                static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
                delete []dims;
                break;
            }
            if( mType == matWishartMean )
            {
                if (m_pPseudoCountsMean)
                {
                    static_cast<CMatrix<float>*>(
                        m_pPseudoCountsMean)->Release(pObj);
                    m_pPseudoCountsMean = NULL;
                }
                m_pPseudoCountsMean = C2DNumericDenseMatrix<float>::Create(  dims, data );
                static_cast<CMatrix<float>*>(m_pPseudoCountsMean)->AddRef(pObj);
                break;
            }                                  
            if( mType == matCovariance )
            {
                if( m_pMatrixCov )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                    m_pMatrixCov = NULL;
                }
                dims[1] = dim;
                m_pMatrixCov = C2DNumericDenseMatrix<float>::Create( dims, data );
                if( !m_pMatrixCov->IsSymmetric(1e-3f) )
                {
                    PNL_THROW(CInvalidOperation, 
                        "Covariance is only symmetric")
                }
                static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
                delete []dims;
                break;
            }
            if (mType == matWishartCov)
            {
                if (m_pPseudoCountsCov)
                {
                    static_cast<CMatrix<float>*>(
                        m_pPseudoCountsCov)->Release(pObj);
                    m_pPseudoCountsCov = NULL;
                }
                dims[1] = dim;
                m_pPseudoCountsCov = C2DNumericDenseMatrix<float>::Create( dims, data );
                float det = m_pPseudoCountsCov->Determinant();
                if( det < 0 )
                {
                    delete m_pPseudoCountsCov;
                    PNL_THROW( CInconsistentType,
                        "parametric matrix must be positive semidifinite" );
                }
                int isIllMatrix = m_pPseudoCountsCov->IsIllConditioned();
                if( isIllMatrix )
                {
                    PNL_THROW( CInvalidOperation,
                        "delta function is a special case - we create it by special function" );
                }
                static_cast<CMatrix<float>*>(m_pPseudoCountsCov)->AddRef(pObj);
                break;
            }
            if( mType == matWeights )
            {
                PNL_CHECK_RANGES( numberOfWeightMatrix, 0, m_NumberOfNodes -1 );
		
                if( m_pMatricesWeight[numberOfWeightMatrix] )
                {
                    static_cast<CMatrix<float>*>(
                        m_pMatricesWeight[numberOfWeightMatrix])->Release(pObj);
                    m_pMatricesWeight[numberOfWeightMatrix] = NULL;
                }
                dims[0] = dim;
                dims[1] = m_NodeTypes[numberOfWeightMatrix]->GetNodeSize();
                m_pMatricesWeight[numberOfWeightMatrix] = C2DNumericDenseMatrix<float>::Create( 
                    dims, data );
                static_cast<CMatrix<float>*>(
                    m_pMatricesWeight[numberOfWeightMatrix])->AddRef(pObj);
            }
            delete []dims;    
            break;
        }
    default:
        {
            PNL_THROW( CBadConst, "isFactor should be 0 or 1" );
        }
    }
    CheckMomentFormValidity();
    CheckCanonialFormValidity();
}
void CGaussianDistribFun::AttachMatrix( CMatrix<float>* pMatrix,
				       EMatrixType mType,
				       int numberOfWeightMatrix,
				       const int *parentIndices,
                                       bool isMultipliedByDelta )
{    
    if( !pMatrix )
    {
        PNL_THROW( CNULLPointer, "matrix" );
    }
    if( pMatrix->GetNumberDims() != 2 )
    {
        PNL_THROW( CInvalidOperation, 
            "Gaussiandata works with plane matrices" )
    }
    if(( m_bDeltaFunction )&&( mType != matMean ))
    {
        PNL_THROW( CInvalidOperation, 
            "delta function have only matrix mean" )
    }
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, 
            "uniform distribution have no matrices" )
    }
    C2DNumericDenseMatrix<float>* p2Matrix = static_cast<C2DNumericDenseMatrix<float>*>(pMatrix);
    void *pObj = this;
    if(( m_bDeltaFunction )&&( mType == matMean ))
    {
        if( m_pMatrixMean )
        {
            static_cast<CMatrix<float>*>(m_pMatrixMean)->Release(pObj);
        }
        m_pMatrixMean = p2Matrix;
        static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
        //we need to Release all other matrices
        if(m_pMatrixH)
        {
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
        }
        if(m_pMatrixK)
        {
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
        }
        if(m_pMatrixCov)
        {
            static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
            m_pMatrixCov = NULL;
        }
        m_normCoeff = 0;
        return;
    }
    int i;
    int numDims;
    const int *ranges;
    p2Matrix->GetRanges( &numDims, &ranges );
    switch( m_bPotential )
    {
    case 1:
        {
            if( mType == matWeights )
            {
                PNL_THROW( CInconsistentType,
                    "matrix of type weights is inconsistent for factor" );
            }
            int dimSize = 0;
            for( i = 0; i < m_NumberOfNodes; i++)
            {
                if (!m_NodeTypes[i]->IsDiscrete())
                    dimSize += m_NodeTypes[i]->GetNodeSize();
            }
            
            /*//we delete all matrices from another form
            if(( mType == matMean )||( mType == matCovariance ))
            { 
                if( m_bCanonical )
                {
                    m_bCanonical = 0;
                    if(m_pMatrixH)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixH)->Release(pObj);
                        m_pMatrixH = NULL;
                    }
                    if(m_pMatrixK)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixK)->Release(pObj);
                        m_pMatrixK = NULL;
                    }
                    m_g = FLT_MAX;
                }
            }
            if(( mType == matH )||( mType == matK ))
            {
                if( m_bMoment )
                {
                    m_bMoment = 0;
                    if(m_pMatrixMean)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixMean)->Release(pObj);
                        m_pMatrixMean = NULL;
                    }
                    if(m_pMatrixCov)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixCov)->Release(pObj);
                        m_pMatrixCov = NULL;
                    }
                    m_normCoeff = 0;
                }
            }*/
            // change matrix - checking data
            if(( mType == matMean )||( mType == matH ))
            {
                if(( numDims != 2 )||( ranges[0] != dimSize )||
                    (ranges[1] != 1))
                {
                    PNL_THROW( CInconsistentSize, "matrix sizes" );
                }
            }
            if(( mType == matCovariance )||( mType == matK ))
            {
                if(( numDims != 2 )||( ranges[0] != dimSize )||
                    ( ranges[1] != dimSize ))
                {
                    PNL_THROW( CInconsistentSize, "matrix sizes" );
                }
                if (!p2Matrix->IsSymmetric(1e-3f))
                {
                    PNL_THROW( CInvalidOperation, 
                        "K & Covariance should be symmetric" )
                }
            }
            switch( mType )
            {
            case matMean:
                {
                    if(m_pMatrixMean)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixMean)->Release(pObj); 
                        m_pMatrixMean = NULL;
                    }
                    m_pMatrixMean = p2Matrix;
                    static_cast<CMatrix<float>*>(
                        m_pMatrixMean)->AddRef(pObj);
                    break;
                }
            case matCovariance:
                {
                    if(m_pMatrixCov)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixCov)->Release(pObj); 
                        m_pMatrixCov = NULL;
                    }
                    m_pMatrixCov = p2Matrix;
                    if( !isMultipliedByDelta )
                    {
                        float det = m_pMatrixCov->Determinant();
                        if( det < 0 )
                        {
                            PNL_THROW( CInconsistentType,
                            "covariance matrix must be positive semidifinite" );
                        }
                        int isIllMatrix = m_pMatrixCov->IsIllConditioned();
                        if( isIllMatrix )
                        {
                            PNL_THROW( CInconsistentType,
                                "covariance matrix is is ill-ocnditioned, it should be delta distribution");
                            //m_bDeltaFunction = 1;
                            //static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                            //m_pMatrixCov = NULL;
                        }
                    }
                    static_cast<CMatrix<float>*>(
                        m_pMatrixCov)->AddRef(pObj);
                    
                    break;
                }
            case matH:
                {
                    if(m_pMatrixH)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixH)->Release(pObj); 
                        m_pMatrixH = NULL;
                    }
                    m_pMatrixH = p2Matrix;
                    static_cast<CMatrix<float>*>(
                        m_pMatrixH)->AddRef(pObj);
                    break;
                }
            case matK:
                {
                    if(m_pMatrixK)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pMatrixK)->Release(pObj);
                        m_pMatrixK = NULL;
                    }
                    m_pMatrixK = p2Matrix;
                    if( !isMultipliedByDelta )
                    {
                        float det = m_pMatrixK->Determinant();
                        if( det < 0 )
                        {
                            PNL_THROW( CInconsistentType,
                                "K matrix must be positive semidifinite" );
                        }
                    }
                    static_cast<CMatrix<float>*>(
                        m_pMatrixK)->AddRef(pObj);
                    break;
                }
            }
            break;
        }
        case 0:
	    {
		int nodeSize = m_NodeTypes[ m_NumberOfNodes - 1 ]->GetNodeSize();
		switch (mType)
		{
		case matH: case matK:
		    {
			PNL_THROW( CInconsistentType, 
			    "matrix type for CPD cannot be  matH or MatK" );
		    }
		case matMean:
		    {
			if(( numDims != 2 )||(ranges[1] != 1)||
			    ( ranges[0] != nodeSize))
			{
			    PNL_THROW( CInconsistentSize, "matrix size" );
			}
			else
			{
			    if(m_pMatrixMean)
			    {
				static_cast<CMatrix<float>*>(
				    m_pMatrixMean)->Release(pObj); 
				m_pMatrixMean = NULL;
			    }
			    m_pMatrixMean = p2Matrix;
			    static_cast<CMatrix<float>*>(
				m_pMatrixMean)->AddRef(pObj);    
			}
			break;
		    }
            case matWishartMean:
                {
                    if(m_pPseudoCountsMean)
                    {
                        static_cast<CMatrix<float>*>(
                            m_pPseudoCountsMean)->Release(pObj); 
                        m_pPseudoCountsMean = NULL;
                    }
                    m_pPseudoCountsMean = p2Matrix;
                    static_cast<CMatrix<float>*>(
                        m_pPseudoCountsMean)->AddRef(pObj);
                    break;
                }
                case matCovariance:
		    {
			if(( numDims !=2 )||( ranges[0] != nodeSize )||
			    ( ranges[1] != nodeSize ))
			{
			    PNL_THROW( CInconsistentSize, "matrix size" );
			}
			else
			{
			    if(m_pMatrixCov)
			    {
				static_cast<CMatrix<float>*>(
				    m_pMatrixCov)->Release(pObj); 
				m_pMatrixCov = NULL;
			    }
			    m_pMatrixCov = p2Matrix;
                            if( !isMultipliedByDelta )
                            {
                                float det = m_pMatrixCov->Determinant();
                                if( det < 0 )
                                {
                                    PNL_THROW( CInconsistentType,
                                    "covariance matrix must be positive semidifinite" );
                                }
                                int isIllMatrix = m_pMatrixCov->IsIllConditioned();
                                if( isIllMatrix )
                                {
                                    PNL_THROW( CInconsistentType,
                                        "covariance matrix is is ill-ocnditioned, it should be delta distribution");
                                    //m_bDeltaFunction = 1;
                                    //static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                                    //m_pMatrixCov = NULL;
                                }
                            }
			    static_cast<CMatrix<float>*>(
				m_pMatrixCov)->AddRef(pObj);
			}
			break;
		    }
        case matWishartCov:
            {
                if(m_pPseudoCountsCov)
                {
                    static_cast<CMatrix<float>*>(
                        m_pPseudoCountsCov)->Release(pObj); 
                    m_pPseudoCountsCov = NULL;
                }
                if(( numDims != 2 )||( ranges[0] != nodeSize )||
                  ( ranges[1] != nodeSize ))
                {
                    PNL_THROW( CInconsistentSize, "matrix sizes" );
                }
                m_pPseudoCountsCov = p2Matrix;
                float det = m_pPseudoCountsCov->Determinant();
                if( det < 0 )
                {
                    PNL_THROW( CInconsistentType,
                    "parametric matrix must be positive semidifinite" );
                }
                int isIllMatrix = m_pPseudoCountsCov->IsIllConditioned();
                if( isIllMatrix )
                {
                    PNL_THROW( CInconsistentType,
                        "parametric matrix is is ill-ocnditioned");
                }
    
                static_cast<CMatrix<float>*>(
                    m_pPseudoCountsCov)->AddRef(pObj);
                break;
            }
		case matWeights:
		    {
			if(( numberOfWeightMatrix <0 )
			    ||( numberOfWeightMatrix > m_NumberOfNodes-1 ))
			{
			    PNL_THROW( COutOfRange, "number of matrix weight" )
			}
			else
			{
			    if(( ranges[0] != nodeSize )||
				( ranges[1] != 
				m_NodeTypes[numberOfWeightMatrix]->GetNodeSize()))
			    {
				PNL_THROW( CInconsistentSize, "matrix size" );
			    }
			    else
			    {
				if(m_pMatricesWeight[numberOfWeightMatrix])
				{
				    static_cast<CMatrix<float>*>(
					m_pMatricesWeight[numberOfWeightMatrix])->Release(pObj); 
				    m_pMatricesWeight[numberOfWeightMatrix] = NULL;
				}
				m_pMatricesWeight[numberOfWeightMatrix] = p2Matrix;
				static_cast<CMatrix<float>*>(
				    m_pMatricesWeight[numberOfWeightMatrix])->AddRef(pObj);
			    }
			}
			break;
		    }
		}
	    }        
    }
    CheckMomentFormValidity();
    CheckCanonialFormValidity();
}

bool CGaussianDistribFun::IsValid(std::string* description) const
{
    if( m_bMoment || m_bCanonical )
    {
        return 1;
    }
    else
    {
        if( description )
        {
            std::stringstream st;
            st<<"Gaussian distribution function haven't any valid form - ";
            st<<"neither Moment no Canonical."<<std::endl;
            st<<"For valid Moment form should be matrix Mean, symmetric ";
            st<<"nonsingular matrix Covarianceand in case of CPD  with ";
            st<<"Gaussian parents - matrices Weights."<<std::endl;
            st<<"For valid Canonical form (appropriate only for potentials)";
            st<<"should be H matrix and symmetric nonsingular K matrix."<<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
        }
        return 0;
    }
}

int CGaussianDistribFun::CheckMomentFormValidity()
{
    
    if( m_bUnitFunctionDistribution )
    {
        return m_bMoment;
    }
    if( m_bDeltaFunction )
    {
        if( m_pMatrixMean )
        {
            return m_bMoment;
        }
        else
        {
            return 0;
        }
    }
    if( m_pMatrixMean && m_pMatrixCov )
    {
        bool isMultipliedByDelta = (IsDistributionSpecific()==3)? 1:0;
        if( !isMultipliedByDelta )
        {
            float det = m_pMatrixCov->Determinant();
            if( det < 0 )
            {
                PNL_THROW( CInconsistentType,
                    "covariance matrix must be positive semidefinite" );
            }
            int isIllMatrix = m_pMatrixCov->IsIllConditioned();
            if( isIllMatrix )
            {
                PNL_THROW( CInconsistentType,
                "covariance matrix is is ill-conditioned, it should be delta distribution");
            }
	    if( !m_pMatrixCov->IsSymmetric(1e-3f) )
	    {
		PNL_THROW( CInvalidOperation, "covariance isn't symmetric" );
	    }
        }
        if(( m_bPotential )||( !m_bPotential && (m_NumberOfNodes == 1) ))
        {
            if(!((fabs( m_normCoeff)<0.001f )||( fabs(m_normCoeff) >= FLT_MAX )))
            {
                m_bMoment = 1;
                return 1;
            }
            else
            {
                float p1 = (float)pow((double)( 2 * PNL_PI ),
		    ( m_numberOfDims / 2.0 ));
                float detCov = m_pMatrixCov->Determinant();
                if( detCov < 0  )
                {
                    PNL_THROW( CInternalError,
                        "covariance matrix must be positive semidifinite" );
                }
                float p2 = (float)pow((double)detCov,0.5);
                m_normCoeff = 1/(p1*p2);
                m_bMoment = 1;
                return 1;
            }
        }
        else
        {
            int i;
            for( i = 0; i < m_NumberOfNodes - 1; i++ )
            {
                if( !m_pMatricesWeight[i] )
                {
                    return 0;
                }
            }
            if( m_normCoeff    )
            {
                m_bMoment = 1;
                return 1;
            }
            else
            {
                float p1 = (float)pow((double)( 2 * PNL_PI ),
		    ( m_numberOfDims / 2.0 ));
                float detCov = m_pMatrixCov->Determinant();
                if( detCov < 0 )
                {
                    PNL_THROW( CInternalError,
                        "covariance matrix must be positive semidifinite" );                
                }
                float p2 = (float)pow((double)detCov,0.5);
                m_normCoeff = 1/(p1*p2);
                m_bMoment = 1;
                return 1;
            }
        }
    }
    else
    {
        return 0;
    }
    
}

int CGaussianDistribFun::CheckCanonialFormValidity()
{
    int multWithMax = 0;
    if( m_bUnitFunctionDistribution )
    {
        return m_bCanonical;
    }
    if( m_bDeltaFunction )
    {
        if( m_pMatrixMean )
        {
            return m_bCanonical;
        }
        else
        {
            return 0;
        }
    }
    if( m_bPotential )
    {
        if( m_pMatrixK && m_pMatrixH )
        {
            bool isMultipliedByDelta = (IsDistributionSpecific()==3)? 1:0;
            if( !isMultipliedByDelta )
            {
                float det = m_pMatrixK->Determinant();
                if( det < 0 )
                {
                    PNL_THROW( CInconsistentType,
		        "K matrix must be positive semidifinite" );
                }
                //float kSum = m_pMatrixK->SumAll(1);
                //float hSum = m_pMatrixH->SumAll(1);
                //if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                //{
                    //PNL_THROW( CInvalidOperation, 
		        //"created model is UnitFunction but it created by non special case" );
                //}
            }
            if( fabs(m_g) < FLT_MAX)
            {
                m_bCanonical = 1;
                return 1;
            }
            else
            {
                //we can recompute m_g by converting in other form
                m_bCanonical = 1;
                int oldMoment = m_bMoment;
                UpdateMomentForm();
                C2DNumericDenseMatrix<float>* matTr = m_pMatrixMean->Transpose();
                C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply( matTr,
		    m_pMatrixK, multWithMax );
                delete matTr;
                C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply( matTemp1,
		    m_pMatrixMean, multWithMax );
                delete matTemp1;
                int size;
                const float *val;
                matTemp2->GetRawData( &size, &val);
                if( size != 1 )
                {
                    PNL_THROW( CInternalError, "it should be a single value" );
                }
                //m_g = m_normCoeff + 0.5f*(float)(log(m_pMatrixK->Determinant()))
		    //- m_numberOfDims*(float)log(2*PNL_PI) - val[0];
                m_g = (float)log(m_normCoeff) - 0.5f * val[0];
                delete matTemp2;
                if( !oldMoment )
                {
                    m_bMoment = 0;
                    void *pObj = this;
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                    m_normCoeff = 0.0f;
                }
                return 1;
                //return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void CGaussianDistribFun::UpdateMomentForm()
{
    int multWithMaximize = 0;
    if( m_bPotential )
    {
        if(( !m_bMoment )&&( !m_bCanonical ))
        {
            PNL_THROW( CInvalidOperation, "no valid form" );
        }
        if(( m_bUnitFunctionDistribution )||(m_bDeltaFunction))
        {
            m_bMoment = 1;
            return;
        }
        if(( m_bCanonical )&&( !m_bMoment ))
        {
            void *pObj = this;
            //cov = inv(K) ;
            if( m_pMatrixCov )
            {
                static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                m_pMatrixCov = NULL;
            }
            m_pMatrixCov = m_pMatrixK->Inverse();
            static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
            //mean = cov*H;
            if( m_pMatrixMean )
            {
                static_cast<CMatrix<float>*>(m_pMatrixMean)->Release(pObj);
                m_pMatrixMean = NULL;
            }
            m_pMatrixMean = pnlMultiply( m_pMatrixCov, m_pMatrixH,
                multWithMaximize );
            static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
            //we can compute norm coeff without base of g
            //n = length(mean);
            int n;
            const int *ranges;
            m_pMatrixMean->GetRanges(&n, &ranges);
            n = ranges[0];
            //if(( fabs(m_g)<0.0001f )||( fabs(m_g)>=FLT_MAX ))
            //{
                //we recompute it if m_g takes the specified value
                //but its all right if it takes this value accidentally - no problem
                float p1 = (float)pow((double)(2*PNL_PI), (n/2.0) );
                float detCov = m_pMatrixCov->Determinant();
                if( detCov < 0 )
                {
                    PNL_THROW( CInternalError,
                        "covariance matrix must be positive semidifinite" );                
                }
                float p2 = (float)pow((double)detCov, 0.5);
                m_normCoeff = 1/(p1*p2);
            /*}
            else
            {
                //m_normCoeff = g - 0.5*(log(det(K)) - n*log(2*pi) - mean'*K*mean)
                //float p1 = (float)log(m_pMatrixK->Determinant());
                //float p2 = (float)log( 2*PNL_PI );
                C2DNumericDenseMatrix<float>* matTr = m_pMatrixMean->Transpose();
                C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply(
		    matTr, m_pMatrixK, multWithMaximize );
                delete matTr;
                C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply( matTemp1,
                    m_pMatrixMean, multWithMaximize );
                int size;
                const float *val;
                matTemp2->GetRawData( &size, &val);
                if( size != 1 )
                {
                    PNL_THROW( CInternalError, "it should be a single value" );
                }
                m_normCoeff = (float)exp(m_g + 0.5f * val[0]);
                delete matTemp1;
                delete matTemp2;
            }*/
            m_bMoment = 1;
        }
    }
}

void CGaussianDistribFun::UpdateCanonicalForm()
{
    if( m_bPotential )
    {
        int multWithMaximize = 0;
        if(( !m_bMoment )&&( !m_bCanonical ))
        {
            PNL_THROW( CInvalidOperation, "no valid form" );
        }
        if(( m_bDeltaFunction )||( m_bUnitFunctionDistribution ))
        {
            m_bCanonical = 1;
            return;
        }
        if(( !m_bCanonical )&&( m_bMoment ))
        {
            void *pObj = this;
            //K = inv(cov);
            if( m_pMatrixK )
            {
                static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                m_pMatrixK = NULL;
            }
            m_pMatrixK = m_pMatrixCov->Inverse();
            static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
            //H = K*mean;
            if( m_pMatrixH )
            {
                static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
                m_pMatrixH = NULL;
            }
            m_pMatrixH = pnlMultiply( m_pMatrixK, m_pMatrixMean, multWithMaximize );
            static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
            //n = length(K);
            int n; const int *ranges;
            m_pMatrixK->GetRanges(&n, &ranges);
            n = ranges[0];
            //    g = norm_coeff + 0.5*(log(det(K)) - n*log(2*pi) - mu'*K*mu);
            C2DNumericDenseMatrix<float>* matTr = m_pMatrixMean->Transpose();
            C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply( matTr,
		m_pMatrixK, multWithMaximize );
            delete matTr;
            C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply( matTemp1,
                m_pMatrixMean, multWithMaximize);
            int size;const float *val;
            matTemp2->GetRawData( &size, &val);
            if( size != 1 )
            {
                PNL_THROW( CInternalError, "it should be a single value" );
            }
            if( (fabs( m_normCoeff)<0.001f )||( fabs(m_normCoeff) >= FLT_MAX ) )
            {
                float p1 = (float)pow((double)(2*PNL_PI),(m_numberOfDims/2.0));
                float detCov = m_pMatrixCov->Determinant();
                if( detCov < 0 )
                {
                    PNL_THROW( CInternalError,
                        "covariance matrix must be positive semidifinite" );
                }
                float p2 = (float)pow((double)detCov, 0.5);
                m_normCoeff = 1/(p1*p2);
            }
            //m_g = m_normCoeff + 0.5f*(float)(log(m_pMatrixK->Determinant()))
                //- m_numberOfDims*(float)log(2*PNL_PI) - val[0];
            m_g = (float)log(m_normCoeff) - 0.5f * val[0];
            delete matTemp1;
            delete matTemp2;
            m_bCanonical = 1;
        }
    }
}

CNodeValues *CGaussianDistribFun::GetMPE()
{
    CNodeValues *mpe = NULL;
    if( m_bPotential )
    {
        UpdateMomentForm();
        int dataLength;
        const float* data;
        m_pMatrixMean->GetRawData( &dataLength, &data );
        valueVector dataVec;
        dataVec.resize( dataLength );
        for( int i = 0; i < dataLength; i++ )
        {
            dataVec[i].SetFlt(data[i]);
        }
        mpe = CNodeValues::Create( m_NumberOfNodes, &m_NodeTypes.front(),
            dataVec);
    }
    return mpe;
    
}


int CGaussianDistribFun::IsEqual( const CDistribFun *dataToCompare, 
				 float epsilon, int withCoeff,
				 float* maxDifference ) const
{
    if( !dataToCompare )
    {
        PNL_THROW( CNULLPointer, "dataToCompare" );
    }
    if( maxDifference )
    {
        *maxDifference = 0.0f;
    }
    if( dataToCompare->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInconsistentType, 
            "we can compare datas of the same type" );
    }
    int i;
    float ZeroEpsilon = epsilon;
    const CGaussianDistribFun *data = (const CGaussianDistribFun*)dataToCompare;
    if( (!IsValid())||( !data->IsValid() ) )
    {
        PNL_THROW( CInvalidOperation, "we can't compare invalid data" )
    }
    //we need to check node types
    if( m_numberOfDims != data->m_numberOfDims )
    {
        return 0;
    }
    if( m_NumberOfNodes != data->m_NumberOfNodes )
    {
        return 0;
    }
    //int ret = 1;
    int ret = 0;
    //we need to check special cases but it can be in both special & common forms (fixme)
    if( m_bUnitFunctionDistribution )
    {
        if(( !data->m_bUnitFunctionDistribution )&&
            ( !data->IsDistributionSpecific() )&&(data->m_bCanonical))
        {
            float hSum = data->m_pMatrixH->SumAll(1);
            float kSum = data->m_pMatrixK->SumAll(1);
            ret = (( hSum < ZeroEpsilon )&&( kSum < ZeroEpsilon )&&
                ( fabs( data->m_g ) < ZeroEpsilon ) ) ? 1 : 0;
            if( maxDifference && !ret )
            {
                *maxDifference = (hSum > kSum) ? hSum: kSum;
                *maxDifference = (*maxDifference > m_g)? *maxDifference:m_g;
            }
        }
        else if( data->m_bUnitFunctionDistribution)
        {
            ret = 1;
        }
        return ret;
	
    }
    //now we can check if data is in unitFunction form & "this" usn't in this form
    if( data->m_bUnitFunctionDistribution )
    {
        if(( !IsDistributionSpecific() )&&( m_bCanonical ))
        {
            float hSum = m_pMatrixH->SumAll(1);
            float kSum = m_pMatrixK->SumAll(1);
            ret = (( hSum < ZeroEpsilon )&&( kSum < ZeroEpsilon )&&
                ( fabs(m_g) < ZeroEpsilon ) )? 1:0;
            if( maxDifference && !ret )
            {
                *maxDifference = (hSum > kSum) ? hSum: kSum;
                *maxDifference = (*maxDifference > m_g)? *maxDifference:m_g;
            }
        }
        else
        {
            ret = 0;
        }
        return ret;
    }
    if( m_bDeltaFunction )
    {
        if( data->m_bDeltaFunction )
        {
            //need to compare means
            float difNormCoeff = (float)fabs(
                data->m_normCoeff-(double)m_normCoeff);
            C2DNumericDenseMatrix<float> *matTemp = static_cast
                <C2DNumericDenseMatrix<float>*>( pnlCombineNumericMatrices( 
                data->m_pMatrixMean, m_pMatrixMean, 0 ));
            float difMean = matTemp->SumAll(1);
            delete matTemp;
            if(( m_bMoment && data->m_bMoment )||
                ( m_bCanonical && data->m_bCanonical ))
            {
                if( !withCoeff )
                {
                    ret = ( difMean < epsilon ) ? 1:0;
                    if( maxDifference && !ret )
                    {
                        *maxDifference = difMean;
                    }
                }
                else
                {
                    ret = (( difNormCoeff < epsilon )&&( difMean <epsilon ))
                        ? 1 : 0;
                    if( maxDifference && !ret )
                    {
                        *maxDifference = ( difMean > difNormCoeff )
                            ? difMean: difNormCoeff;
                    }
                }
            }
        }
        else
        {
            if(( data->m_bMoment )&&( !data->IsDistributionSpecific()))
            {
                float difNormCoeff = (float)fabs(
                    data->m_normCoeff-(double)m_normCoeff);
                C2DNumericDenseMatrix<float> *matTemp = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( data->m_pMatrixMean, m_pMatrixMean, 0));
                float difMean = matTemp->SumAll(1);
                float covSum = data->m_pMatrixCov->SumAll(1);
                if( !withCoeff )
                {
                    ret = (( difMean < epsilon ) &&( covSum < ZeroEpsilon) ) 
                        ? 1:0;
                    if( maxDifference && !ret )
                    {
                        *maxDifference = ( difMean > covSum )? difMean:covSum;
                    }
                }
                else
                {
                    ret = (( difNormCoeff < epsilon )&&( difMean <epsilon )&&
                        ( covSum < ZeroEpsilon )) ? 1:0; 
                    if( maxDifference && !ret )
                    {
                        *maxDifference = ( difMean > covSum )? difMean:covSum;
                        *maxDifference = ( *maxDifference > difNormCoeff ) ? 
                            *maxDifference:difNormCoeff;
                    }
                }
                delete matTemp;
            }
        }
        return ret;
    }
    //the case of "this" in non-special form & dataToCompare in special form
    if(( data->m_bDeltaFunction )&&( !IsDistributionSpecific() ))
    {
        if( m_bMoment )
        {
            float difNormCoeff = (float)fabs(
                data->m_normCoeff-(double)m_normCoeff);
            C2DNumericDenseMatrix<float> *matTemp = 
                static_cast<C2DNumericDenseMatrix<float>*>(
                pnlCombineNumericMatrices( data->m_pMatrixMean, m_pMatrixMean, 0 ));
            float difMean = matTemp->SumAll(1);
            float covSum = m_pMatrixCov->SumAll(1);
            if( !withCoeff )
            {
                ret = (( difMean < epsilon ) &&( covSum < ZeroEpsilon) )? 1 :0;
                if( maxDifference && !ret )
                {
                    *maxDifference = ( difMean > covSum )? difMean:covSum;
                }
            }
            else
            {
                ret = (( difNormCoeff < epsilon )&&( difMean <epsilon )&&
                    ( covSum < ZeroEpsilon )) ? 1 : 0;
                if( maxDifference && !ret )
                {
                    *maxDifference = ( difMean > covSum )? difMean:covSum;
                    *maxDifference = ( *maxDifference > difNormCoeff ) ? 
                        *maxDifference:difNormCoeff;
                }
            }
            delete matTemp;
        }
        return ret;
    }
    if( m_posOfDeltaMultiply.size() != data->m_posOfDeltaMultiply.size() )
    {
        return 0;
    }
    else if( m_posOfDeltaMultiply.size() > 0 )
    {
        //we need to compare pos of delta multiply 
        int numPos = m_posOfDeltaMultiply.size();
        intVector posOfOtherInThis = intVector( numPos );
        int location;
        for( i = 0; i < numPos; i++ )
        {
            location = std::find( m_posOfDeltaMultiply.begin(),
                m_posOfDeltaMultiply.end(), data->m_posOfDeltaMultiply[i] ) 
                - m_posOfDeltaMultiply.begin();
            if( location < numPos )
            {
                posOfOtherInThis[i] = location;
            }
            else
            {
                return 0;
            }
        }
        const float *mean1Data = &m_meanValuesOfMult.front();
        const float *mean2Data = &data->m_meanValuesOfMult.front();
        int countAllRight = 0;
        int size = 0;
	int j = 0;
        for( i = 0; i < numPos; i++ )
        {
            size = m_NodeTypes[m_posOfDeltaMultiply[
                posOfOtherInThis[i]]]->GetNodeSize();
            countAllRight = 0;
            for( j = 0; j <size; j++ )
            {
                if( fabs( mean1Data[m_offsetToNextMean[
                    posOfOtherInThis[i]]] -
                    mean2Data[data->m_offsetToNextMean[i]]) > epsilon )
                {
                    return 0;
                }
            }
        }
    }
    const CGaussianDistribFun* dataInMomentForm;
    const CGaussianDistribFun* thisInMomentForm;
    switch( m_bPotential )
    {
    case 0:
        {
            if( data->m_bPotential )
            {
                return 0;
            }
            else
            {
                dataInMomentForm = data;
                thisInMomentForm = this;
                C2DNumericDenseMatrix<float> *matDiff;
                float trace;
		
                matDiff= static_cast<C2DNumericDenseMatrix<float>*>
                    (pnlCombineNumericMatrices( 
                    dataInMomentForm->m_pMatrixMean,
                    thisInMomentForm->m_pMatrixMean, 0 ));
                trace = matDiff->SumAll(1);
                delete matDiff;
                if( trace > epsilon )
                {
                    if( maxDifference )
                    {
                        *maxDifference = trace;
                    }
                    return 0;
                }
                matDiff = static_cast<C2DNumericDenseMatrix<float>*>
                    (pnlCombineNumericMatrices( 
                    dataInMomentForm->m_pMatrixCov,
                    thisInMomentForm->m_pMatrixCov, 0 ));
                C2DNumericDenseMatrix<float> *prodDiff;
                prodDiff = pnlMultiply(matDiff, matDiff, 0);
                delete matDiff;
                trace = prodDiff->Trace();
                delete prodDiff;
                if( trace > epsilon )
                {
                    if( maxDifference )
                    {
                        *maxDifference = trace;
                    }
                    return 0;
                }
                int numParents = dataInMomentForm->GetNumberOfNodes()-1;
                int i;
                for( i = 0; i < numParents; i++ )
                {
                    matDiff= static_cast<C2DNumericDenseMatrix<float>*>
                        (pnlCombineNumericMatrices( 
                        dataInMomentForm->m_pMatricesWeight[i], 
                        thisInMomentForm->m_pMatricesWeight[i], 0 ));
		    
                    int nDims;
                    const int *ranges;
                    matDiff->GetRanges(&nDims, &ranges);
                    trace = matDiff->SumAll(1)/(ranges[0]*ranges[1]);
                    if( trace > epsilon )
                    {
                        if( maxDifference )
                        {
                            *maxDifference = trace;
                        }
                        return 0;
                    }
                    delete matDiff;
                }
            }
            ret = 1;
            break;
        }
    case 1:
        {
            if( !data->m_bPotential )
            {
                return 0;
            }
            else 
            {
                if(( data->m_bCanonical )&&( m_bCanonical ))
                {
                    float difG = (float)fabs( data->m_g - m_g );
                    CMatrix<float> *matTemp = pnlCombineNumericMatrices( 
                        data->m_pMatrixH, m_pMatrixH, 0 );
                    float difH = ( matTemp->SumAll(1) )/m_numberOfDims;
                    delete matTemp;
                    matTemp = pnlCombineNumericMatrices( data->m_pMatrixK,
                        m_pMatrixK, 0 );
                    float difK = ( matTemp->SumAll(1) )/(
                        m_numberOfDims * m_numberOfDims);
                    delete matTemp;
                    if( !withCoeff )
                    {
                        ret = (( difH < epsilon )&&( difK < epsilon)) ? 1:0;
                        if( maxDifference && !ret )
                        {
                            *maxDifference = (difH > difK)? difH : difK;
                        }
                    }
                    else
                    {
                        ret = (( difG < epsilon )&&( difH < epsilon )&&
                            ( difK < epsilon)) ? 1:0;
                        if( maxDifference && !ret )
                        {
                            *maxDifference = (difH > difK)? difH : difK;
                            *maxDifference = (*maxDifference > difG)?
                                (*maxDifference):m_g;
                        }
                    }
                }
                else
                {
                    if(( data->m_bMoment )&&( m_bMoment ))
                    {
                        float difNormCoeff = (float)fabs(
                            data->m_normCoeff-m_normCoeff);
                        CNumericDenseMatrix<float> *matTemp = 
                            pnlCombineNumericMatrices(
                            data->m_pMatrixMean, m_pMatrixMean, 0 );
                        float difMean = (matTemp->SumAll(1))/m_numberOfDims;
                        delete matTemp;
                        matTemp = pnlCombineNumericMatrices( data->m_pMatrixCov,
                            m_pMatrixCov, 0 );
                        float difCov = ( matTemp->SumAll(1) )/(
                            m_numberOfDims * m_numberOfDims );
                        delete matTemp;
                        if( !withCoeff )
                        {
                            ret = (( difMean < epsilon )&&( difCov < epsilon))
                                ? 1:0;
                            if( maxDifference && !ret )
                            {
                                *maxDifference = ( difMean > difCov )
                                    ? difMean:difCov;
                            }
                        }
                        else
                        {
                            ret = (( difNormCoeff < epsilon )&&
				( difMean < epsilon )&&( difCov < epsilon))? 1:0;
                            if( maxDifference && !ret )
                            {
                                *maxDifference = ( difMean > difNormCoeff )
                                    ? difMean:difNormCoeff;
                                *maxDifference = 
                                    ( *maxDifference > difNormCoeff )
                                    ?(*maxDifference):difNormCoeff;
                            }
                        }
                    }
                }
            }
            break;
        }
    default:
        {
            PNL_THROW(CInconsistentType, "distribution form");
        }
    }
    
    return ret;
}

void CGaussianDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CGaussianDistribFun now ");
}

void CGaussianDistribFun::MultiplyInSelfData( const int *pBigDomain, 
					     const int *pSmallDomain, const CDistribFun *pOtherData )
{
    EDistributionType dtOther = pOtherData->GetDistributionType();
    if(( dtOther != dtGaussian )&&( dtOther != dtScalar ))
    {
        PNL_THROW( CInconsistentType, "only for Gaussian and scalars" );
    }
    if(( !pBigDomain )||( !pSmallDomain )||( !pOtherData ))
    {
        PNL_THROW( CNULLPointer, "one of inputs" )
    }
    int size1 = m_NumberOfNodes;
    int size2 = pOtherData->GetNumberOfNodes();
    if( size1 < size2 )
    {
        PNL_THROW( CInconsistentSize, 
            "small data is large than big data" );
    }
    if( dtOther == dtScalar )
    {
        return;
    }
    const CGaussianDistribFun *pGSmallData = (const CGaussianDistribFun *)pOtherData;
    if(!( m_bPotential )&&((pGSmallData)->GetFactorFlag()))
    {
        PNL_THROW( CInconsistentType, 
            "we can multiply only data for factors");
    }
    void *pObj = this;
    //getting information to determine type of multiplying
    //delta function(s) - special case
    int fDelta1 = m_bDeltaFunction;
    int fDelta2 = pGSmallData->m_bDeltaFunction;
    int fUni1 = m_bUnitFunctionDistribution;
    int fUni2 = pGSmallData->m_bUnitFunctionDistribution;
    if( fUni2 )
    {
        return;
        //we needn't do anything - the Big Data doesn't change
    }
    int fc1 = m_bCanonical;
    int fc2 = pGSmallData->m_bCanonical;
    int fm1 = m_bMoment;
    int fm2 = pGSmallData->m_bMoment;
    int i,j;
    if(!((fc1&&fc2)||(fc1&&fm2)||(fm1&&fc2)||(fm1&&fm2)))
    {
        PNL_THROW( CInvalidOperation, "one of data is invalid" )
    }
    if( pGSmallData->m_posOfDeltaMultiply.size() )
    {
        PNL_THROW( CNotImplemented, 
            "we can multiply now only by non composite functions" );
    }
    //we need to check all cases of input data
    //type of result is the type of largest data!
    
    //we need to find equiv positions and check data in the same positions
    //if both data of the same size they may have different order in domain
    int location;
    intVector posOfSmallDomInBig = intVector(size2);
    for( i = 0; i < size2; i++ )
    {
        location = std::find( pBigDomain, pBigDomain + size1, 
            pSmallDomain[i] ) - pBigDomain;
        if( location >= size1 )
        {
            PNL_THROW( CInternalError, "small domain isn't subset of big" );
        }
        posOfSmallDomInBig[i] = location;
    }
    //if small data is delta function - 
    //we need to keep it in the list of datas to multiply
    if( fDelta2 )
    {
        if(( !fDelta1 )&&( !m_posOfDeltaMultiply.size() ))
        {
            if( size2 == size1 )
            {
                //if they bot of the same size - we create new delta function
                m_bDeltaFunction = 1;
                m_bUnitFunctionDistribution = 0;
                if( !pGSmallData->m_pMatrixMean )
                {
                    PNL_THROW( CInconsistentState, 
                        "we need to multiply full data only!" )
                }
                if( m_pMatrixMean )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixMean)->Release(pObj);
                }
                m_pMatrixMean = pGSmallData->m_pMatrixMean;
                static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
                if( m_pMatrixCov )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixCov)->Release(pObj);
                    m_pMatrixCov = NULL;
                }
                if( m_pMatrixH )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
                    m_pMatrixH = NULL;
                }
                if( m_pMatrixK )
                {
                    static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                    m_pMatrixK = NULL;
                }
                m_bMoment = fm2;
                m_bCanonical = fc2;
                return;
            }
            //we keep mean values in linear array
            //to get mean for next position in domain
            //(i.e. in numbers from 0 to m_numberOfNodes-1)
            //we need to ask about offset to it
            //this offset can be computed by using nodeTypes, but we have it here
            m_posOfDeltaMultiply.insert( m_posOfDeltaMultiply.end(), 
                posOfSmallDomInBig.begin(), posOfSmallDomInBig.end() );
            const floatVector* mean = pGSmallData->m_pMatrixMean->GetVector();
            m_meanValuesOfMult.insert( m_meanValuesOfMult.end(), mean->begin(),
                mean->end() );
            if( m_offsetToNextMean.empty() )
            {
                m_offsetToNextMean.push_back(0);
            }
            int nodeSize;
            for( i = 0; i < size2; i++ )
            {
                nodeSize = m_NodeTypes[posOfSmallDomInBig[i]]->IsDiscrete()? 
                    0:m_NodeTypes[posOfSmallDomInBig[i]]->GetNodeSize();
                m_offsetToNextMean.push_back(m_offsetToNextMean[
                    m_offsetToNextMean.size()-1]+nodeSize );
            }
            return;
        }
        else
        {
            if (fDelta1)
            {
                int mean1l; const float *mean1Data;
                m_pMatrixMean->GetRawData( &mean1l, &mean1Data );
                int mean2l; const float *mean2Data;
                pGSmallData->m_pMatrixMean->GetRawData( &mean2l, &mean2Data );
                int countAllRight = 0;
                for( i = 0; i <size2; i++ )
                {
                    //get node sizes for compare
                    int nodeSize = m_NodeTypes[posOfSmallDomInBig[i]]->IsDiscrete() ?
                        0:m_NodeTypes[posOfSmallDomInBig[i]]->GetNodeSize();
                    for( j = 0; j < nodeSize; j++ )
                    {
                        if( fabs( mean1Data[posOfSmallDomInBig[i]+j] 
                            - mean2Data[i])>0.0001f)
                        {
                            PNL_THROW( CInvalidOperation,
                                "means in delta functions are differ" )
                        }
                    }
                    countAllRight++;
                }
                if( countAllRight==size2)
                {
                    //result - the same delta function as pGBigData
                    return;
                }
                else
                {
                    PNL_THROW( CInvalidOperation,
                        "multiply of two different Delta functions" )
                }
            }
            else //if ( m_posOfDeltaMultiply.size() )
            {
                int isAllright = 1;
                if( m_bMoment )
                {
                    for( i = 0; i < m_posOfDeltaMultiply.size(); i++ )
                    {
                        location = std::find( posOfSmallDomInBig.begin(),
                            posOfSmallDomInBig.end(), m_posOfDeltaMultiply[i] )
                            - posOfSmallDomInBig.begin();
                        if( location <= size2 )
                        {
                            //we need to check all values of mean:
                            int mean2l; const float *mean2Data;
                            pGSmallData->m_pMatrixMean->GetRawData( &mean2l, &mean2Data );
                            int thisPosSize = m_offsetToNextMean[i+1] - 
                                m_offsetToNextMean[i];
                            int offset = 0;
                            for( j = 0; j < location; j++ )
                            {
                                offset += pGSmallData->m_NodeTypes[j]->IsDiscrete()? 
                                    0:pGSmallData->m_NodeTypes[j]->GetNodeSize();
                            }
                            for( j = 0; j < thisPosSize; j++ )
                            {
                                if( fabs( mean2Data[offset + j] - 
                                    m_meanValuesOfMult[m_offsetToNextMean[i] + j]
                                    )>0.001f )
                                {
                                    isAllright = 0;
                                    PNL_THROW( CInvalidOperation,
					"multiply by other data on the same position" );
                                }
                            }
                        }
                    }
                }
                if( isAllright )
                {
                    m_posOfDeltaMultiply.insert( m_posOfDeltaMultiply.end(), 
                        posOfSmallDomInBig.begin(), posOfSmallDomInBig.end() );
                    const floatVector* mean = 
                        pGSmallData->m_pMatrixMean->GetVector();
                    m_meanValuesOfMult.insert( m_meanValuesOfMult.end(), 
			mean->begin(), mean->end() );
                    if( m_offsetToNextMean.empty() )
                    {
                        m_offsetToNextMean.push_back(0);
                    }
                    int nodeSize;
                    for( i = 0; i < size2; i++ )
                    {
/*                        nodeSize = 
                            m_NodeTypes[posOfSmallDomInBig[i]]->GetNodeSize();
*/
                        nodeSize = m_NodeTypes[posOfSmallDomInBig[i]]->IsDiscrete() ? 
                            0:m_NodeTypes[posOfSmallDomInBig[i]]->GetNodeSize();
                        m_offsetToNextMean.push_back(m_offsetToNextMean[
                            m_offsetToNextMean.size() - 1] + nodeSize );
                    }
                }
                return;
            }
        }
    }
    //we have only case of non special smallest data and any largest data
    if(!(fDelta1|| fUni1))
    {    
        // we can multiply by simply sum of canonical characteristics
        //first - two factors of the same size in canoniacal form
        if( fc1&&fc2 )
        {
            m_bCanonical = 1;
            if( size1 == size2 )
            {
                C2DNumericDenseMatrix<float> *resH = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixH,
                    pGSmallData->m_pMatrixH,1 )) ;
                static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
                m_pMatrixH = resH;
                static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef( pObj );
                C2DNumericDenseMatrix<float> *resK = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixK,
                    pGSmallData->m_pMatrixK, 1));
                static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                m_pMatrixK = resK;
                static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                m_g += pGSmallData->m_g;
                float kSum = m_pMatrixK->SumAll(1);
                float hSum = m_pMatrixH->SumAll(1);
                if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                {
                    //need to release matrices and make it unit function
                    m_bUnitFunctionDistribution = 1;
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
                if( m_bMoment )
                {
                    m_bMoment = 0;
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                    UpdateMomentForm();
                }
                return;
            }
            else
            {
                //get sizes from big data and get information from small data
                int num; const float *data;
                m_pMatrixH->GetRawData( &num, &data );
                //num is the number of rows and number of columns in big K
                const floatVector *vecBigK = m_pMatrixK->GetVector();
                //we need to create extended matrix from small to sum them
                C2DNumericDenseMatrix<float> *matSmallH = pGSmallData->m_pMatrixH;
                C2DNumericDenseMatrix<float> *matSmallK = pGSmallData->m_pMatrixK;
                float gSmall = pGSmallData->m_g;
                const floatVector*vecH = matSmallH->GetVector();
                const floatVector*vecK = matSmallK->GetVector();
                floatVector newSmallH = floatVector( num, 0 );
                intVector offsets = intVector(size1, 0);
                intVector nodeSizesInBig = intVector(size1, 0);

                nodeSizesInBig[0] = m_NodeTypes[0]->IsDiscrete() ? 
                    0:m_NodeTypes[0]->GetNodeSize();
//                nodeSizesInBig[0] = m_NodeTypes[0]->GetNodeSize();
                offsets[0] = 0;
                for( i = 1; i < size1; i++ )
                {
//                    nodeSizesInBig[i] = m_NodeTypes[i]->GetNodeSize();
                    nodeSizesInBig[i] = m_NodeTypes[i]->IsDiscrete() ? 
                            0:m_NodeTypes[i]->GetNodeSize();
                    offsets[i] = nodeSizesInBig[i-1] + offsets[i-1];
                }
                intVector offSmall = intVector( size2,0 );
                offSmall[0] = 0;
                for( i = 1; i < size2; i++ )
                {
                    offSmall[i] = offSmall[i-1] +
                        nodeSizesInBig[posOfSmallDomInBig[i-1]];
                }
                //copy data for H - add information 
                for( i = 0; i < size2; i++)
                {
                    if( nodeSizesInBig[posOfSmallDomInBig[i]]>0 )
                    {
                        memcpy( &newSmallH[offsets[posOfSmallDomInBig[i]]],
			    &(*vecH)[offSmall[i]],
			    sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[i]] );
                    }
                }
                //add zeros to smallK
                int sizeBigK = vecBigK->size();
                int rowSizeSmall = vecH->size();
                floatVector newSmallK = floatVector( sizeBigK, 0 );
                int j,k;
                for( i = 0; i < size2; i++ )
                {
                    for( k = 0; k < size2; k++ )
                    {
                        //we need to copy data about intersections of smallK
                        for( j = 0;
                        j < nodeSizesInBig[posOfSmallDomInBig[i]]; j++ )
                        {
                            memcpy( &newSmallK[offsets[posOfSmallDomInBig[k]] + (offsets[posOfSmallDomInBig[i]] + j) * num],
				&(*vecK)[offSmall[k] + (offSmall[i] + j) * rowSizeSmall],
				sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[k]] );
                        }
                    }
                }
                int nDims; const int *ranges;
                m_pMatrixH->GetRanges( &nDims, &ranges );
                C2DNumericDenseMatrix<float> *matNewSmallH = 
                    C2DNumericDenseMatrix<float>::Create( ranges, &newSmallH.front() );
                m_pMatrixK->GetRanges( &nDims, &ranges );
                C2DNumericDenseMatrix<float> *matNewSmallK
                    = C2DNumericDenseMatrix<float>::Create( ranges, &newSmallK.front() );
                C2DNumericDenseMatrix<float> *resH = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixH, matNewSmallH, 1 ));
                static_cast<CMatrix<float>*>(m_pMatrixH)->Release( pObj );
                m_pMatrixH = resH;
                static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                C2DNumericDenseMatrix<float>* resK = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixK, matNewSmallK, 1 ));
                static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
                m_pMatrixK = resK;
                static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                m_g += gSmall;
                delete matNewSmallH;
                delete matNewSmallK;
                float kSum = m_pMatrixK->SumAll(1);
                float hSum = m_pMatrixH->SumAll(1);
                if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                {
                    //need to release matrices and make it unit function
                    m_bUnitFunctionDistribution = 1;
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
                if( m_bMoment )
                {
                    m_bMoment = 0;
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                    UpdateMomentForm();
                }
                return;
            }
	    
        }
        if( fm1&&fm2 )
        {
            //we need to convert both to canonical and multiply it - 
            //it isn't the special case of size1 == size2
            //we need to create new data, 
            //update canonical form for it and multiply
            CGaussianDistribFun *newSmallData = static_cast<CGaussianDistribFun*>
                (pGSmallData->Clone());
            newSmallData->UpdateCanonicalForm();
            UpdateCanonicalForm();
            MultiplyInSelfData( pBigDomain, pSmallDomain, 
                newSmallData );
		/*m_pMatrixMean->Release(pObj);
		m_pMatrixMean = NULL;
		m_pMatrixCov->Release(pObj);
		m_pMatrixCov = NULL;
		m_bMoment = 0;
            UpdateMomentForm();*/
            delete newSmallData;
            m_bCanonical = 0;
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
            m_g = FLT_MAX;
            return;
        }
        if( fm1 && fc2 )
        {
            //we can multiply it in the same way -
            //by converting big data in canonical form, multiply in canonical,
            //convert result to the moment
            UpdateCanonicalForm();
            MultiplyInSelfData( pBigDomain, pSmallDomain,
                pGSmallData );
            m_pMatrixMean->Release(pObj);
            m_pMatrixMean = NULL;
            m_pMatrixCov->Release(pObj);
            m_pMatrixCov = NULL;
            m_bMoment = 0;
            UpdateMomentForm();
            m_bCanonical = 0;
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
            m_g = FLT_MAX;
            return;
        }
        if( fc1 && fm2 )
        {
            //we can multiply it in the same way -
            //by converting small data in canonical form, multiply in canonical,
            CGaussianDistribFun *newSmallData = static_cast<CGaussianDistribFun*>
                (pGSmallData->Clone());
            newSmallData->UpdateCanonicalForm();
            MultiplyInSelfData( pBigDomain, pSmallDomain, newSmallData );
            delete newSmallData;
            return;
        }
    }
    else
    {
        if( fDelta1 )
        {
            return;
        }
        if( fUni1 )
        {
            if( size1 == size2 )
            {
                //we have the same data as small - be careful!!!
                m_bMoment = pGSmallData->m_bMoment;
                m_bCanonical = pGSmallData->m_bCanonical;
                m_bUnitFunctionDistribution = 
                    pGSmallData->m_bUnitFunctionDistribution;
                m_bDeltaFunction = pGSmallData->m_bDeltaFunction;
                //we needn't to Release matrices - 
                //UnitFunctionDistribution doesn't have matrices
                if( pGSmallData->m_pMatrixMean )
                {
                    m_pMatrixMean = pGSmallData->m_pMatrixMean;
                    static_cast<CMatrix<float>*>(m_pMatrixMean)->AddRef(pObj);
                }
                if( pGSmallData->m_pMatrixCov )
                {
                    m_pMatrixCov = pGSmallData->m_pMatrixCov;
                    static_cast<CMatrix<float>*>(m_pMatrixCov)->AddRef(pObj);
                }
                if( pGSmallData->m_pMatrixH )
                {
                    m_pMatrixH = pGSmallData->m_pMatrixH;
                    static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                }
                if( pGSmallData->m_pMatrixK )
                {
                    m_pMatrixK = pGSmallData->m_pMatrixK;
                    static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                }
                m_g = pGSmallData->m_g;
                m_normCoeff = pGSmallData->m_normCoeff;
                return;
            }
            else
            {
                //we need to copy some part of data from uniform distribution
                //to new data as canonical form
                m_bUnitFunctionDistribution = 0;
                m_bCanonical = 1;
                m_bMoment = 0;
                intVector dims(2, 1);
                dims[0] = m_numberOfDims;
                floatVector hVec = floatVector( m_numberOfDims, 0.0f);
                C2DNumericDenseMatrix<float> *matHOfZeros
                    = C2DNumericDenseMatrix<float>::Create( &dims.front(), &hVec.front() );
                floatVector kVec = floatVector( m_numberOfDims*m_numberOfDims, 0.f );
                dims[1] = m_numberOfDims;
                C2DNumericDenseMatrix<float> *matKofZeros
                    = C2DNumericDenseMatrix<float>::Create( &dims.front(), &kVec.front() );
                if( fc2 )
                {
                    //we can work directly with matrices
                    const floatVector*vecH = 
                        pGSmallData->m_pMatrixH->GetVector();
                    const floatVector*vecK = 
                        pGSmallData->m_pMatrixK->GetVector();
                    floatVector newSmallH( m_numberOfDims, 0 );
                    intVector offsets = intVector(size1, 0);
                    intVector nodeSizesInBig = intVector(size1, 0);
    
                    nodeSizesInBig[0] = m_NodeTypes[0]->IsDiscrete() ? 
                        0:m_NodeTypes[0]->GetNodeSize();
//                    nodeSizesInBig[0] = m_NodeTypes[0]->GetNodeSize();
                    offsets[0] = 0;
                    for( i = 1; i < size1; i++ )
                    {
//                        nodeSizesInBig[i] = m_NodeTypes[i]->GetNodeSize();
                        nodeSizesInBig[i] = m_NodeTypes[i]->IsDiscrete() ? 
                            0:m_NodeTypes[i]->GetNodeSize();
                        offsets[i] = nodeSizesInBig[i-1] + offsets[i-1];
                    }
                    intVector offSmall = intVector( size2,0 );
                    offSmall[0] = 0;
                    for( i = 1; i < size2; i++ )
                    {
                        offSmall[i] = offSmall[i - 1] + 
                            nodeSizesInBig[posOfSmallDomInBig[i - 1]];
                    }
                    //copy data for H - add information 
                    for( i = 0; i < size2; i++ )
                    {
                        if( nodeSizesInBig[posOfSmallDomInBig[i]] > 0 )
                        {
                            memcpy( &newSmallH[offsets[ posOfSmallDomInBig[i]]],
				&(*vecH)[offSmall[i]],
				sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[i]] );
                        }
                    }
                    //add zeros to smallK
                    int rowSizeSmall = vecH->size();
                    floatVector newSmallK(m_numberOfDims * m_numberOfDims, 0 );
                    int j,k;
                    for( i = 0; i < size2; i++ )
                    {
                        for( k = 0; k < size2; k++ )
                        {
                            //we need to copy data about intersections of smallK
                            for( j = 0; 
                            j < nodeSizesInBig[ posOfSmallDomInBig[i] ]; j++ )
                            {
                                memcpy( &newSmallK[offsets[posOfSmallDomInBig[k]] + (offsets[posOfSmallDomInBig[i]] + j) * m_numberOfDims],
				    &(*vecK)[offSmall[k] + (offSmall[i] + j) * rowSizeSmall],
				    sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[k]] );
                            }
                        }
                    }
                    C2DNumericDenseMatrix<float> *newMatSmallK
                        = C2DNumericDenseMatrix<float>::Create( &dims.front(), &newSmallK.front() );
                    dims[1] = 1;
                    C2DNumericDenseMatrix<float> *newMatSmallH
                        = C2DNumericDenseMatrix<float>::Create( &dims.front(), &newSmallH.front() );
                    m_pMatrixH = static_cast<C2DNumericDenseMatrix<float>*>(
                        pnlCombineNumericMatrices( matHOfZeros, newMatSmallH, 1 ));
                    static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                    m_pMatrixK = static_cast<C2DNumericDenseMatrix<float>*>(
                        pnlCombineNumericMatrices( matKofZeros, newMatSmallK, 1 ));
                    static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                    m_g = pGSmallData->m_g;
                    float kSum = m_pMatrixK->SumAll(1);
                    float hSum = m_pMatrixH->SumAll(1);
                    if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                    {
                        //need to release matrices and make it unit function
                        m_bUnitFunctionDistribution = 1;
                        m_pMatrixH->Release(pObj);
                        m_pMatrixK->Release(pObj);
                        m_g = FLT_MAX;
                    }
                    delete newMatSmallK;
                    delete newMatSmallH;
                    delete matHOfZeros;
                    delete matKofZeros;
                    return;                    
                }
                else //if(fm1)
                {
                    CGaussianDistribFun *newSmallData = 
                        static_cast<CGaussianDistribFun*>(
                        pGSmallData->Clone());
                    newSmallData->UpdateCanonicalForm();
                    const floatVector*vecH = 
                        newSmallData->m_pMatrixH->GetVector();
                    const floatVector*vecK = 
                        newSmallData->m_pMatrixK->GetVector();
                    floatVector newSmallH( m_numberOfDims, 0 );
                    intVector offsets(size1, 0);
                    intVector nodeSizesInBig(size1, 0);

                    nodeSizesInBig[0] = m_NodeTypes[0]->IsDiscrete() ? 
                        0:m_NodeTypes[0]->GetNodeSize();

//                    nodeSizesInBig[0] = m_NodeTypes[0]->GetNodeSize();
                    offsets[0] = 0;
                    for( i = 1; i < size1; i++ )
                    {
//                        nodeSizesInBig[i] = m_NodeTypes[i]->GetNodeSize();
                        nodeSizesInBig[i] = m_NodeTypes[i]->IsDiscrete() ? 
                          0:m_NodeTypes[i]->GetNodeSize();
                        offsets[i] = nodeSizesInBig[i-1] + offsets[i-1];
                    }
                    intVector offSmall( size2,0 );
                    offSmall[0] = 0;
                    for( i = 1; i < size2; i++ )
                    {
                        offSmall[i] = offSmall[i - 1] + 
                            nodeSizesInBig[posOfSmallDomInBig[i - 1]];
                    }
                    //copy data for H - add information 
                    for( i = 0; i < size2; i++)
                    {
                        if( nodeSizesInBig[posOfSmallDomInBig[i]] > 0 )
                        {
                            memcpy( &newSmallH[offsets[posOfSmallDomInBig[i]]],
				&(*vecH)[offSmall[i]],
				sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[i]] );
                        }
                    }
                    //add zeros to smallK
                    int rowSizeSmall = vecH->size();
                    floatVector newSmallK(m_numberOfDims*m_numberOfDims, 0 );
                    int j,k;
                    for( i = 0; i < size2; i++ )
                    {
                        for( k = 0; k < size2; k++ )
                        {
                            //we need to copy data about intersections of smallK
                            for( j = 0; 
                            j < nodeSizesInBig[posOfSmallDomInBig[i]]; j++ )
                            {
                                memcpy( &newSmallK[offsets[posOfSmallDomInBig[k]] + (offsets[posOfSmallDomInBig[i]] + j) * m_numberOfDims],
				    &(*vecK)[offSmall[k] + (offSmall[i] + j) * rowSizeSmall],
				    sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[k]] );
                            }
                        }
                    }
                    C2DNumericDenseMatrix<float> *newMatSmallK = 
                        C2DNumericDenseMatrix<float>::Create( 
                        &dims.front(), &newSmallK.front() );
                    dims[1] = 1;
                    C2DNumericDenseMatrix<float> *newMatSmallH = 
                        C2DNumericDenseMatrix<float>::Create(  
                        &dims.front(), &newSmallH.front() );
                    m_pMatrixH = static_cast<C2DNumericDenseMatrix<float>*>(
                        pnlCombineNumericMatrices( matHOfZeros, newMatSmallH, 1 ));
                    static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                    m_pMatrixK = static_cast<C2DNumericDenseMatrix<float>*>(
                        pnlCombineNumericMatrices( matKofZeros, newMatSmallK, 1 ));
                    static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                    m_g = pGSmallData->m_g;
                    float kSum = m_pMatrixK->SumAll(1);
                    float hSum = m_pMatrixH->SumAll(1);
                    if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                    {
                        //need to release matrices and make it unit function
                        m_bUnitFunctionDistribution = 1;
                        m_pMatrixH->Release(pObj);
                        m_pMatrixK->Release(pObj);
                        m_g = FLT_MAX;
                    }
                    delete newMatSmallK;
                    delete newMatSmallH;
                    delete matHOfZeros;
                    delete matKofZeros;
                    delete newSmallData;
                    return;                                
                }
                
            }
        }
    }
}


void CGaussianDistribFun::DivideInSelfData( const int *pBigDomain, 
					   const int *pSmallDomain, const CDistribFun *pOtherData ) 
{
    EDistributionType dtOther = pOtherData->GetDistributionType();
    if(( dtOther != dtGaussian )&&( dtOther != dtScalar ))
    {
        PNL_THROW( CInconsistentType, "only for Gaussian" );
    }
    if(( !pBigDomain )||( !pSmallDomain )||( !pOtherData ))
    {
        PNL_THROW( CNULLPointer, "one of inputs" )
    }
    if(!(( m_bPotential )&&(((CGaussianDistribFun*)pOtherData)->GetFactorFlag())))
    {
        PNL_THROW( CInconsistentType, 
            "we can divide only data for factors");
    }
    const CGaussianDistribFun *pGSmallData = (const CGaussianDistribFun *)pOtherData;
    void *pObj = this;
    //getting information to determine type of multiplying
    //delta function(s) - special case
    int fDelta1 = m_bDeltaFunction;
    int fDelta2 = pGSmallData->m_bDeltaFunction;
    if( fDelta2 )
    {
        PNL_THROW( CInvalidOperation, "we can't divide by Delta function" )
    }
    int fUni1 = m_bUnitFunctionDistribution;
    int fUni2 = pGSmallData->m_bUnitFunctionDistribution;
    int size1 = m_NumberOfNodes;
    int size2 = pGSmallData->m_NumberOfNodes;
    if( size1 < size2 )
    {
        PNL_THROW( CInconsistentSize, 
            "small data is large than big data" );
    }
    int fc1 = m_bCanonical;
    int fc2 = pGSmallData->m_bCanonical;
    int fm1 = m_bMoment;
    int fm2 = pGSmallData->m_bMoment;
    int i,j;
    if(!((fc1&&fc2)||(fc1&&fm2)||(fm1&&fc2)||(fm1&&fm2)))
    {
        PNL_THROW( CInvalidOperation, "one of data is invalid" )
    }
    if( pGSmallData->m_posOfDeltaMultiply.size() || m_posOfDeltaMultiply.size() )
    {
        PNL_THROW( CNotImplemented, 
            "we can divide only by non composite functions" );
    }
    if(( fUni2 )||( fDelta1 )||( dtOther == dtScalar ))
    {
        //we needn't to do something - the same result
        return;
    }
    int location;
    intVector posOfSmallDomInBig = intVector(size2);
    for( i = 0; i < size2; i++ )
    {
        location = std::find( pBigDomain, pBigDomain + size1, 
	    pSmallDomain[i] ) - pBigDomain;
        if( location >= size1 )
        {
            PNL_THROW( CInternalError, "small domain isn't subset of big" );
        }
        posOfSmallDomInBig[i] = location;
    }
    if( !fUni1 )
    {
	
        if( fc1 &&fc2 )
        {
            //m_bMoment = 0;
            //m_bCanonical = 1;
            if( size1 == size2 )
            {
                C2DNumericDenseMatrix<float> *resH = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixH,
                    pGSmallData->m_pMatrixH, 0 )) ;
                m_pMatrixH->Release(pObj);
                m_pMatrixH = resH;
                m_pMatrixH->AddRef( pObj );
                C2DNumericDenseMatrix<float> *resK = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixK, 
                    pGSmallData->m_pMatrixK, 0 ));
                m_pMatrixK->Release(pObj);
                m_pMatrixK = resK;
                m_pMatrixK->AddRef(pObj);
                m_g -= pGSmallData->m_g;
                float kSum = m_pMatrixK->SumAll(1);
                float hSum = m_pMatrixH->SumAll(1);
                if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                {
                    //need to release matrices and make it unit function
                    m_bUnitFunctionDistribution = 1;
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
                if( m_bMoment )
                {
                    m_bMoment = 0;
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                    UpdateMomentForm();
                }
                return;
            }
            else
            {
                //get sizes from big data and get information from small data
                int num;
                const float *data;
                m_pMatrixH->GetRawData( &num, &data );
                //num is the number of rows and number of columns in big K
                const floatVector *vecBigK = m_pMatrixK->GetVector();
                //we need to create extended matrix from small to sum them
                C2DNumericDenseMatrix<float> *matSmallH = pGSmallData->m_pMatrixH;
                C2DNumericDenseMatrix<float> *matSmallK = pGSmallData->m_pMatrixK;
                float gSmall = pGSmallData->m_g;
                const floatVector*vecH = matSmallH->GetVector();
                const floatVector*vecK = matSmallK->GetVector();
                floatVector newSmallH = floatVector( num, 0 );
                intVector offsets = intVector(size1, 0);
                intVector nodeSizesInBig = intVector(size1, 0);
                nodeSizesInBig[0] = m_NodeTypes[0]->GetNodeSize();
                offsets[0] = 0;
                for( i = 1; i < size1; i++ )
                {
                    nodeSizesInBig[i] = m_NodeTypes[i]->GetNodeSize();
                    offsets[i] = nodeSizesInBig[i-1] + offsets[i-1];
                }
                intVector offSmall = intVector( size2,0 );
                offSmall[0] = 0;
                for( i = 1; i < size2; i++ )
                {
                    offSmall[i] = offSmall[i-1] +
                        nodeSizesInBig[posOfSmallDomInBig[i-1]];
                }
                //copy data for H - add information 
                for( i = 0; i < size2; i++)
                {
                    if( nodeSizesInBig[posOfSmallDomInBig[i]]>0 )
                    {
                        memcpy( &newSmallH[offsets[ posOfSmallDomInBig[i]]],
			    &(*vecH)[offSmall[i]],
			    sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[i]] );
                    }
                }
                //add zeros to smallK
                int sizeBigK = vecBigK->size();
                int rowSizeSmall = vecH->size();
                floatVector newSmallK = floatVector( sizeBigK, 0 );
                int k;
                for( i = 0; i < size2; i++ )
                {
                    for( k = 0; k < size2; k++ )
                    {
                        //we need to copy data about intersections of smallK
                        for( j = 0;
                        j < nodeSizesInBig[posOfSmallDomInBig[i]]; j++ )
                        {
                            memcpy( &newSmallK[offsets[posOfSmallDomInBig[k]] + (offsets[posOfSmallDomInBig[i]] + j) * num],
				&(*vecK)[offSmall[k] + (offSmall[i] + j) * rowSizeSmall],
				sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[k]] );
                        }
                    }
                }
                int nDims;
                const int *ranges;
                m_pMatrixH->GetRanges( &nDims, &ranges );
                C2DNumericDenseMatrix<float> *matNewSmallH = 
                    C2DNumericDenseMatrix<float>::Create( ranges,
                    &newSmallH.front());
                m_pMatrixK->GetRanges( &nDims, &ranges );
                C2DNumericDenseMatrix<float> *matNewSmallK = 
                    C2DNumericDenseMatrix<float>::Create( ranges,
                    &newSmallK.front());
                C2DNumericDenseMatrix<float> *resH = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixH, matNewSmallH, 0));
                m_pMatrixH->Release( pObj );
                m_pMatrixH = resH;
                m_pMatrixH->AddRef(pObj);
                C2DNumericDenseMatrix<float>* resK = 
                    static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( m_pMatrixK, matNewSmallK, 0));
                m_pMatrixK->Release(pObj);
                m_pMatrixK = resK;
                m_pMatrixK->AddRef(pObj);
                m_g -= gSmall;
                delete matNewSmallH;
                delete matNewSmallK;
                float kSum = m_pMatrixK->SumAll(1);
                float hSum = m_pMatrixH->SumAll(1);
                if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                {
                    //need to release matrices and make it unit function
                    m_bUnitFunctionDistribution = 1;
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
                if( m_bMoment )
                {
                    m_bMoment = 0;
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                    UpdateMomentForm();
                }
                return;
            }
        }
        if( fm1 && fm2 )
        {
            CGaussianDistribFun *newSmallData = static_cast<CGaussianDistribFun*>
                (pGSmallData->Clone());
            newSmallData->UpdateCanonicalForm();
            UpdateCanonicalForm();
            DivideInSelfData( pBigDomain, pSmallDomain, newSmallData );
            m_bCanonical = 0;
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
            m_g = FLT_MAX;
            delete newSmallData;
            return;
        }
        
        if( fc1 && fm2 )
        {
            CGaussianDistribFun *newSmallData = static_cast<CGaussianDistribFun*>
                (pGSmallData->Clone());
            newSmallData->UpdateCanonicalForm();
            DivideInSelfData( pBigDomain, pSmallDomain, newSmallData );
            delete newSmallData;
            return;
        }
        if( fm1 && fc2 )
        {
            UpdateCanonicalForm();
            DivideInSelfData( pBigDomain, pSmallDomain, pGSmallData );
            m_bCanonical = 0;
            static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
            m_pMatrixH = NULL;
            static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
            m_pMatrixK = NULL;
            m_g = FLT_MAX;
            return;
        }
	
    }
    else //if( fUni1) 
    {
        //result - partially uniform, partially second distribution with minus
        if( size1 == size2 )
        {
            //we have the same data as small - be careful!!!
            m_bUnitFunctionDistribution = 0;
            //we needn't to Release matrices - 
            //UnitFunctionDistribution doesn't have matrices
            if( pGSmallData->m_bDeltaFunction )
            {
                PNL_THROW( CInvalidOperation, "can't divide by delta distribution" )
            }
            if( pGSmallData->m_bCanonical )
            {
                intVector dims(2, 1);
                dims[0] = m_numberOfDims;
                floatVector hVec = floatVector( m_numberOfDims, 0.0f);
                m_pMatrixH = C2DNumericDenseMatrix<float>::Create( &dims.front(),
                    &hVec.front() );
                m_pMatrixH->AddRef(pObj);
                m_pMatrixH->CombineInSelf( pGSmallData->m_pMatrixH, 0 );
                floatVector kVec = floatVector( m_numberOfDims*m_numberOfDims, 0.f );
                dims[1] = m_numberOfDims;
                m_pMatrixK = C2DNumericDenseMatrix<float>::Create( &dims.front(),
                    &kVec.front());
                m_pMatrixK->AddRef(pObj);
                m_pMatrixK->CombineInSelf( pGSmallData->m_pMatrixK, 0 );
                m_g = -(pGSmallData->m_g);
                if( m_bMoment )
                {
                    //need to update moment form
                    UpdateMomentForm();
                }
                if( !m_bCanonical )
                {
                    //need to release canonical
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
            }
            else
            {
                //need to get canonical form
                CGaussianDistribFun* pCanonicalSmall = 
                    static_cast<CGaussianDistribFun*>(pGSmallData->Clone());
                pCanonicalSmall->UpdateCanonicalForm();
                DivideInSelfData( pBigDomain, pSmallDomain, pCanonicalSmall );
                delete pCanonicalSmall;
            }
            
            return;
        }
        else
        {
            if( fc2 )
            {
                //we need to copy some part of data from uniform distribution
                //to new data as canonical form
                m_bUnitFunctionDistribution = 0;
                m_bCanonical = 1;
                m_bMoment = 0;
                intVector dims(2, 1);
                dims[0] = m_numberOfDims;
                floatVector hVec = floatVector( m_numberOfDims, 0.0f);
                C2DNumericDenseMatrix<float> *matHOfZeros = 
                    C2DNumericDenseMatrix<float>::Create( &dims.front(),
                    &hVec.front() );
                floatVector kVec = floatVector( m_numberOfDims*m_numberOfDims, 0.f );
                dims[1] = m_numberOfDims;
                C2DNumericDenseMatrix<float> *matKofZeros = 
                    C2DNumericDenseMatrix<float>::Create( &dims.front(),
                    &kVec.front());
                //we can work directly with matrices
                const floatVector*vecH = 
                    pGSmallData->m_pMatrixH->GetVector();
                const floatVector*vecK = 
                    pGSmallData->m_pMatrixK->GetVector();
                floatVector newSmallH( m_numberOfDims, 0 );
                intVector offsets = intVector(size1, 0);
                intVector nodeSizesInBig = intVector(size1, 0);
                nodeSizesInBig[0] = m_NodeTypes[0]->GetNodeSize();
                offsets[0] = 0;
                for( i = 1; i < size1; i++ )
                {
                    nodeSizesInBig[i] = m_NodeTypes[i]->GetNodeSize();
                    offsets[i] = nodeSizesInBig[i-1] + offsets[i-1];
                }
                intVector offSmall = intVector( size2,0 );
                offSmall[0] = 0;
                for( i = 1; i < size2; i++ )
                {
                    offSmall[i] = offSmall[i - 1] + 
                        nodeSizesInBig[posOfSmallDomInBig[i - 1]];
                }
                //copy data for H - add information 
                for( i = 0; i < size2; i++ )
                {
                    if( nodeSizesInBig[posOfSmallDomInBig[i]] > 0 )
                    {
                        memcpy( &newSmallH[offsets[ posOfSmallDomInBig[i]]],
			    &(*vecH)[offSmall[i]],
			    sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[i]] );
                    }
                }
                //add zeros to smallK
                int rowSizeSmall = vecH->size();
                floatVector newSmallK(m_numberOfDims * m_numberOfDims, 0 );
                int j,k;
                for( i = 0; i < size2; i++ )
                {
                    for( k = 0; k < size2; k++ )
                    {
                        //we need to copy data about intersections of smallK
                        for( j = 0; 
                        j < nodeSizesInBig[ posOfSmallDomInBig[i] ]; j++ )
                        {
                            memcpy( &newSmallK[offsets[posOfSmallDomInBig[k]] + (offsets[posOfSmallDomInBig[i]] + j) * m_numberOfDims],
				&(*vecK)[offSmall[k] + (offSmall[i] + j) * rowSizeSmall],
				sizeof(float) * nodeSizesInBig[posOfSmallDomInBig[k]] );
                        }
                    }
                }
                C2DNumericDenseMatrix<float> *newMatSmallK = 
                    C2DNumericDenseMatrix<float>::Create( 
                    &dims.front(), &newSmallK.front() );
                dims[1] = 1;
                C2DNumericDenseMatrix<float> *newMatSmallH = 
                    C2DNumericDenseMatrix<float>::Create(
                    &dims.front(), &newSmallH.front() );
                m_pMatrixH = static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( matHOfZeros, newMatSmallH, 0 ));
                static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);
                m_pMatrixK = static_cast<C2DNumericDenseMatrix<float>*>(
                    pnlCombineNumericMatrices( matKofZeros, newMatSmallK, 0 ));
                static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
                m_g = 0 - pGSmallData->m_g;
                float kSum = m_pMatrixK->SumAll(1);
                float hSum = m_pMatrixH->SumAll(1);
                if( (kSum < 0.001f)&&( hSum < 0.001f )&&( fabs(m_g)<0.001 ) )
                {
                    //need to release matrices and make it unit function
                    m_bUnitFunctionDistribution = 1;
                    m_pMatrixH->Release(pObj);
                    m_pMatrixH = NULL;
                    m_pMatrixK->Release(pObj);
                    m_pMatrixK = NULL;
                    m_g = FLT_MAX;
                }
                delete newMatSmallK;
                delete newMatSmallH;
                delete matHOfZeros;
                delete matKofZeros;
                return;                    
            }
            else //if(fm1)
            {
                CGaussianDistribFun *newSmallData = 
                    static_cast<CGaussianDistribFun*>(
                    pGSmallData->Clone());
                newSmallData->UpdateCanonicalForm();
                DivideInSelfData( pBigDomain, pSmallDomain, newSmallData );
                delete newSmallData;
                return;
            }
        }
    }
}


CDistribFun* CGaussianDistribFun::GetNormalized() const 
{
    //gaussian data is normalized - always!
    //int multWithMaximize = 0;
    CGaussianDistribFun *gauData = static_cast<CGaussianDistribFun*>(Clone());
    /*if( m_bMoment && !m_bDeltaFunction && 
    ( (fabs( m_normCoeff)<0.001f ))||( fabs(m_normCoeff) >= FLT_MAX ) )
    {
    float p1 = (float)pow((double)(2*PNL_PI),(m_numberOfDims/2.0));
    float detCov = m_pMatrixCov->Determinant();
    if( detCov < 0 )
    {
    PNL_THROW( CInternalError,
    "covariance matrix must be positive semidifinite" );
    }
    float p2 = (float)pow((double)detCov,0.5);
    gauData->m_normCoeff = 1/(p1*p2);
    }
    if( m_bCanonical && !m_bUnitFunctionDistribution &&( fabs(m_g)>=FLT_MAX ) )
    {
    //we need to recompute it according normalization
    gauData->UpdateMomentForm();
    //we have valid moment form - can recompute m_g
    C2DNumericDenseMatrix<float> *matMeanTransp = gauData->m_pMatrixMean->Transpose();
    C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply( matMeanTransp, 
    m_pMatrixK, multWithMaximize);
    C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply(matTemp1,
    gauData->m_pMatrixMean, multWithMaximize);
    delete matMeanTransp;
    int size;
    const float *val;
    matTemp2->GetRawData( &size, &val);
    if( size != 1 )
    {
    PNL_THROW( CInternalError, "it should be a single value" );
    }
    gauData->m_g = m_normCoeff + 
    0.5f*(float)(log(m_pMatrixK->Determinant())) - 
    m_numberOfDims*(float)log(2*PNL_PI) - val[0];
    delete matTemp1;
    delete matTemp2;
    if( !m_bMoment )
    {
    //need to release all matrices
    gauData->m_pMatrixMean->Release(gauData);
    gauData->m_pMatrixMean = NULL;
    gauData->m_pMatrixCov->Release(gauData);
    gauData->m_pMatrixCov = NULL;
    gauData->m_normCoeff = 0.0f;
    
      }
}*/
    return gauData;
}

void CGaussianDistribFun::Normalize()
{
    //gaussian data is normalized - always!
    /*int multWithMaximize = 0;
    if( m_bMoment && !m_bDeltaFunction && 
    ( (fabs( m_normCoeff)<0.001f ))||( fabs(m_normCoeff) >= FLT_MAX ) )
    {
    float p1 = (float)pow((double)(2*PNL_PI),(m_numberOfDims/2.0));
    float detCov = m_pMatrixCov->Determinant();
    if( detCov < 0 )
    {
    PNL_THROW( CInternalError,
    "covariance matrix must be positive semidifinite" );
    }
    float p2 = (float)pow((double)detCov,0.5);
    m_normCoeff = 1/(p1*p2);
    }
    if( m_bCanonical && !m_bUnitFunctionDistribution &&( fabs(m_g) >= FLT_MAX ) )
    {
    //we need to recompute it according normalization
    int oldMoment = m_bMoment;
    UpdateMomentForm();
    //we have valid moment form - can recompute m_g
    C2DNumericDenseMatrix<float> *matMeanTransp = m_pMatrixMean->Transpose();
    C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply( matMeanTransp, 
    m_pMatrixK, multWithMaximize);
    C2DNumericDenseMatrix<float> *matTemp2 = pnlMultiply(matTemp1,
    m_pMatrixMean, multWithMaximize);
    delete matMeanTransp;
    int size;
    const float *val;
    matTemp2->GetRawData( &size, &val);
    if( size != 1 )
    {
    PNL_THROW( CInternalError, "it should be a single value" );
    }
    m_g = m_normCoeff + 0.5f*(float)(log(m_pMatrixK->Determinant())) - 
    m_numberOfDims*(float)log(2*PNL_PI) - val[0];
    delete matTemp1;
    delete matTemp2;
    if( !oldMoment )
    {
    void* pObj = this;
    //need to release all matrices
    m_pMatrixMean->Release(pObj);
    m_pMatrixMean = NULL;
    m_pMatrixCov->Release(pObj);
    m_pMatrixCov = NULL;
    m_normCoeff = 0.0f;
    }
}*/
}

CMatrix<float>* CGaussianDistribFun::GetMatrix( EMatrixType mType, 
                                               int numWeightMat,
                                               const int *parentIndices) const
{
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, 
	    "we haven't any matrices for uniform distribution - it is special case " );
    }
    if( m_bDeltaFunction &&(mType != matMean) )
    {
        PNL_THROW( CInvalidOperation,
            "delta function have only matrix mean" );
    }
    switch( mType )
    {
    case matMean:
        {
            if(( m_pMatrixMean )&&( m_bMoment || m_bDeltaFunction ))
            {
                return m_pMatrixMean;
            }
            else 
            {
                PNL_THROW( CInvalidOperation, 
                    "we haven't now valid mean matrix" );
            }
        }
    case matWishartMean:
        {
            if(( m_pPseudoCountsMean )&&( m_bMoment || m_bDeltaFunction ))
            {
                return m_pPseudoCountsMean;
            }
            else 
            {
                PNL_THROW( CInvalidOperation, 
                    "we haven't now valid mean matrix" );
            }
        }
    case matCovariance:
        {
            if (( m_bMoment )&&( m_pMatrixCov ))
            {
                return m_pMatrixCov;
            }
            else 
            {
                PNL_THROW( CInvalidOperation,
                    "we haven't now valid Covariance matrix" );
            }
        }
    case matWishartCov:
        {
            if (( m_bMoment )&&( m_pPseudoCountsCov ))
            {
                return m_pPseudoCountsCov;
            }
            else 
            {
                PNL_THROW( CInvalidOperation,
                    "we haven't now valid Covariance matrix" );
            }
        }
    case matWeights:
        {
            if((numWeightMat < 0 )||( numWeightMat >m_NumberOfNodes - 1    ))
            {
                PNL_THROW( COutOfRange, 
                    "index of weight matrix is invalid" )
            }
            if(( !m_bPotential )&&( m_bMoment ))
            {
                return m_pMatricesWeight[numWeightMat];
            }
            else
            {    
                PNL_THROW( CInvalidOperation, 
                    "we have no valid matrix weights" );
            }
        }
    case matH:
        {
            if( m_pMatrixH && m_bCanonical )
            {
                return m_pMatrixH;
            }
            else
            {
                PNL_THROW( CInvalidOperation,
                    "we haven't now valid H matrix" );
            }
        }
    case matK:
        {
            if( m_pMatrixK && m_bCanonical )
            {
                return m_pMatrixK;
            }
            else
            {
                PNL_THROW( CInvalidOperation,
                    "we haven't now valid K matrix" );
            }
        }
    case matTable:
        {
            PNL_THROW( CInvalidOperation, 
                "Gaussian data have no matrix for tabular" );
        }
    default:
        {
            PNL_THROW( CBadConst, "no competent type" );
        }
    }
}

//compute probability of taking input value
double CGaussianDistribFun::ComputeProbability( const C2DNumericDenseMatrix<float>* pMatVariable,
					       int asLog, int numObsParents, const int* obsParentsIndices,
					       C2DNumericDenseMatrix<float>*const * pObsParentsMats ) const
{
    PNL_CHECK_IS_NULL_POINTER( pMatVariable );
    PNL_CHECK_RANGES( asLog, 0, 2 );
    
    if( !m_bPotential && (m_NumberOfNodes > 1) &&
        ( ( !(numObsParents == m_NumberOfNodes - 1 )||
	(!obsParentsIndices)||(!pObsParentsMats ) ) ) )
    {
        PNL_THROW( CInvalidOperation,
            "can compute probability only for fully conditioned CPD" )
    }
    if( !m_bPotential && !m_bMoment )
    {
        PNL_THROW( CInvalidOperation, "CPD must have valid moment form" );
    }
    
    //determine limits
    const double maxForExp = 200;
    const double minForExp = -36; 

    //need to check matrices sizes
    int numRanges;
    const int* pRanges;
    pMatVariable->GetRanges( &numRanges, &pRanges);
    int numRangesHere;
    const int* pRangesHere;
    
    int i;
    if( m_bMoment )
    {
        m_pMatrixMean->GetRanges( &numRangesHere, &pRangesHere );
        if( numRanges != numRangesHere )
        {
            PNL_THROW( CInconsistentSize,
                "input matrix of variables must be of the same sizes" );
        }
        else
        {
            for( i = 0; i < numRanges; i++ )
            {
                if( pRanges[i] != pRangesHere[i] )
                {
                    PNL_THROW( CInconsistentSize,
			"input matrix of variables must be of the same sizes");
                }
            }
        }
        C2DNumericDenseMatrix<float>* matCentered = 
            static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( pMatVariable, m_pMatrixMean, 0 ));
        for( i = 0; i < numObsParents; i++ )
        {
            C2DNumericDenseMatrix<float>* multRes = pnlMultiply( 
		m_pMatricesWeight[obsParentsIndices[i]],
		pObsParentsMats[i], 0 );
            matCentered->CombineInSelf( multRes, 0 );
            delete multRes;
        }
        if( m_bDeltaFunction )
        {
            //we can only compare mean values
            //and in case of equal values - set probability to 1
            //and to 0 - otherwise
            float sumAllAbs = matCentered->SumAll(1);
            delete matCentered;
            float epsilon = 1e-5f;
            if( sumAllAbs < epsilon)
            {
                if(asLog)
                {
                    return 0.0;
                }
                else
                {
                    return 1.0;
                }
            }
            else
            {
                if( asLog )
                {
                    return -36;
                }
                else
                {
                    return 0.0;
                }
            }
        }
        C2DNumericDenseMatrix<float>* matCenteredTr = matCentered->Transpose();
        C2DNumericDenseMatrix<float>* matCentMult = pnlMultiply( matCentered,
	    matCenteredTr, 0 );
        C2DNumericDenseMatrix<float>* invCov = m_pMatrixCov->Inverse();
        C2DNumericDenseMatrix<float>* resExp = pnlMultiply( matCentMult, invCov, 0 );
        delete invCov;
        delete matCentMult;
        const int* dims;
        int numDims;
        resExp->GetRanges(&numDims, &dims);
        assert( dims[0] == dims[1] );
        int dim = dims[0];
        float resExpOrder = 0.0f;
        float valAdd = 0.0f;
        int indices[2] = {0,0};
        for( i = 0; i < dim; i++ )
        {
            indices[0] = i;
            indices[1] = i;
            valAdd = resExp->GetElementByIndexes(indices);
            resExpOrder += valAdd;
        }
        delete resExp;
        if( asLog == 2 )
        {
            delete matCentered;
            delete matCenteredTr;
            return resExpOrder;
        }
        resExpOrder = 0.5f*resExpOrder;
        /*C2DNumericDenseMatrix<float>* multRes = pnlMultiply(matCenteredTr, invCov, 0);
        C2DNumericDenseMatrix<float>* matResExpOrder = pnlMultiply( multRes, matCentered, 0 );
        int length;
        const float* resVal;
        matResExpOrder->GetRawData( &length, &resVal );
        assert( length == 1 );
        resExpOrder = 0.5f*resVal[0];
        delete multRes;*/
        delete matCentered;
        delete matCenteredTr;
        double p1 = pow((double)(2*PNL_PI),(((double)m_numberOfDims)/2));
        double p2 = pow(((double)(m_pMatrixCov->Determinant())),0.5);
        double normCoeff = 1/(p1*p2);
        if( asLog  )
        {
            double ret = log(normCoeff)-resExpOrder;
            if( ret > maxForExp ) 
            {
                return maxForExp;
            }
            if( ret < minForExp ) 
            {
                return minForExp;
            }
            return ret;
        }
        else
        {
            double ret = normCoeff*exp(-resExpOrder);
            return ret;
        }
    }
    else 
    {
        //as we're working in terms child-parent, canonical form doesn't use here
        PNL_THROW( CInconsistentType,
            "can work with moment Gaussian distribution only" );
	    /*if( m_bCanonical )
	    {
            m_pMatrixH->GetRanges( &numRangesHere, &pRangesHere );
            if( numRanges != numRangesHere )
            {
	    PNL_THROW( CInconsistentSize,
	    "input matrix of variables must be of the same sizes" );
            }
            else
            {
	    for( i = 0; i < numRanges; i++ )
	    {
	    if( pRanges[i] != pRangesHere[i] )
	    {
	    PNL_THROW( CInconsistentSize,
	    "input matrix of variables must be of the same sizes");
	    }
	    }
            }
            resExpOrder = m_g;
            C2DNumericDenseMatrix<float>* pMatVarTr = pMatVariable->Transpose();
            C2DNumericDenseMatrix<float>* multRes = pnlMultiply( m_pMatrixH,
	    pMatVarTr, 0 );
            int length;
            const float* resVal;
            multRes->GetRawData( &length, &resVal );
            assert( length == 1 );
            resExpOrder += resVal[0];
            delete multRes;
            multRes = pnlMultiply( pMatVariable, m_pMatrixK, 0 );
            C2DNumericDenseMatrix<float>* multRes1 = pnlMultiply( multRes,
	    pMatVarTr, 0 );
            multRes1->GetRawData( &length, &resVal );
            assert( length == 1 );
            resExpOrder += resVal[0];
            delete multRes;
            delete multRes1;
            delete pMatVarTr;
            if( asLog )
            {
	    return resExpOrder;
            }
            else
            {
	    return (float)exp( resExpOrder );
            }
	    }
	    else
	    {
            PNL_THROW( CInvalidOperation,
	    "can't compute probability for invalid distribution" )
    }*/
    }
}


CMatrix<float> *CGaussianDistribFun::GetStatisticalMatrix( EStatisticalMatrix mType,
							  int *parentIndices ) const
{
    switch( mType )
    {
    case stMatMu:
        {
            if( m_pLearnMatrixMean )
            {
                return m_pLearnMatrixMean;
            }
            else 
            {
                PNL_THROW( CInvalidOperation, 
                    "we haven't now valid mean matrix" );
            }
        }
    case stMatSigma:
        {
            if(m_pLearnMatrixCov )
            {
                return m_pLearnMatrixCov;
            }
            else 
            {
                PNL_THROW( CInvalidOperation,
                    "we haven't now valid Covariance matrix" );
            }
        }
        
    default:
        {
            PNL_THROW( CBadConst, "no competent type" );
        }
    }
}

CDistribFun* CGaussianDistribFun::ConvertToSparse() const
{
    const CGaussianDistribFun* self = this;
    CGaussianDistribFun* resDistrib = Copy( self );
    return resDistrib;
}

CDistribFun* CGaussianDistribFun::ConvertToDense() const
{
    const CGaussianDistribFun* self = this;
    CGaussianDistribFun* resDistrib = Copy( self );
    return resDistrib;
}

void CGaussianDistribFun::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
    
    dump<<"I'm a Gaussian distribution function of "<< m_NumberOfNodes <<" nodes.\n";
    int isSpecific = IsDistributionSpecific();
    if( isSpecific == 1 )
    {
        dump<<"I'm a Uniform distribution, haven't any matrices.\n";
        return;
    }
    int i;
    if( isSpecific == 2 )
    {
        dump<<"I'm a delta distribution - have only mean matrix, covariance is zero matrix\n";
        dump<<"My matrix Mean is:\n";
        const floatVector *myVector = static_cast<CNumericDenseMatrix<float>*>(
            m_pMatrixMean)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
        return;
    }
    if( isSpecific == 3 ) 
    {
        dump<<"I'm a mixed distribution, result of multiplication by Delta distributions in some dimensions.";
        dump<<" To see me in valid form you need to marginalize.\n";
        return;
    }
    int isFactor = GetFactorFlag();
    int canonicalFormFlag = GetCanonicalFormFlag();
    int momentFormFlag = GetMomentFormFlag();
    if( momentFormFlag )
    {
        dump<<"I have valid moment form\n";
        dump<<"My normal coefficient is " << m_normCoeff<<"\n";
        dump<<"My matrix Mean is:\n";
        const floatVector *myVector = static_cast<CNumericDenseMatrix<float>*>(
            m_pMatrixMean)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
        dump<<"My matrix Covariance is:\n";
        myVector = static_cast<CNumericDenseMatrix<float>*>(m_pMatrixCov)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
        if( !isFactor )
        {
            int numParents = m_NumberOfNodes - 1;
            for( int j = 0; j < numParents; j++ )
            {
                dump<<"My matrix Weights from %d parent is:"<<j<<"\n";
                myVector = static_cast<CNumericDenseMatrix<float>*>(
                    m_pMatricesWeight[j])->GetVector();
                for( i = 0; i < myVector->size(); i++ )
                {
                    dump<<(*myVector)[i]<<" ";
                }
                dump<<"\n";
            }
        }
    }
    if( canonicalFormFlag )
    {
        dump<<"I have valid canonical form\n";
        dump<<"My coefficient g is "<<m_g<<"\n";
        dump<<"My matrix H is:\n";
        const floatVector *myVector = static_cast<
            CNumericDenseMatrix<float>*>(m_pMatrixH)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
        dump<<"My matrix K is:\n";
        myVector = static_cast<CNumericDenseMatrix<float>*>(m_pMatrixK)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
    }
    if(!(momentFormFlag||canonicalFormFlag) )        
    {
        dump<<"I haven't any valid form\n";
    }
    dump<<"\n";
}

int CGaussianDistribFun::IsSparse() const 
{
    return 0;
}
int CGaussianDistribFun::IsDense() const 
{
    return 1;
}

void CGaussianDistribFun::MarginalizeData( const CDistribFun *pOldData,
					  const int *VarsOfKeep, int NumVarsOfKeep,
					  int maximize )
{
    PNL_CHECK_IS_NULL_POINTER( pOldData );
    PNL_CHECK_IS_NULL_POINTER( VarsOfKeep );
    PNL_CHECK_LEFT_BORDER( NumVarsOfKeep, 1 );
    if( pOldData->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInvalidOperation, "we can marginalize only Gaussian" )
    }
    const CGaussianDistribFun *pData = (const CGaussianDistribFun*)pOldData;
    if( !pData->m_bPotential )
    {
        PNL_THROW( CInvalidOperation, "marginalize a CPD" );
    }
    ReleaseAllMatrices();
    //if we needn't to marginalize - if numNodes == numVarsOfKeep
    //also we needn't to marginalize really if vars of keep - 
    //all nodes except observed (it has zero size in matrix)
    m_bPotential = 1;
    int i,j;
    int numNodes = pData->m_NumberOfNodes;
    m_bDeltaFunction = pData->m_bDeltaFunction;
    m_bUnitFunctionDistribution = pData->m_bUnitFunctionDistribution;
    m_bMoment = pData->m_bMoment;
    m_bCanonical = pData->m_bCanonical;
    if(( NumVarsOfKeep == numNodes ))
    {
        m_NodeTypes = pConstNodeTypeVector(// may be optimized
	    pData->m_NodeTypes.begin(), pData->m_NodeTypes.end());
        m_NumberOfNodes = pData->m_NumberOfNodes;
        m_numberOfDims = pData->m_numberOfDims;
	//make self unit function and use operator =()
	m_bUnitFunctionDistribution = 1;
        CGaussianDistribFun* self = this;
	(*self) = (*pOldData);
        return;
    }
    //check the subset of keep positions and exact corresponding node types
    intVector varAll = intVector( numNodes );
    for( i = 0; i < numNodes; i++ )
    {
        varAll[i] = i;
    }
    intVector varAllFull = intVector( varAll.begin(), varAll.end() );
    int locSmall;
    intVector positionsKeep = intVector( NumVarsOfKeep );
    m_NodeTypes.resize(NumVarsOfKeep);
    m_NumberOfNodes = NumVarsOfKeep;
    //find positionsKeep of dims of keep in domain and check subset
    for( i = 0; i < NumVarsOfKeep; i++ )
    {
        //we find positions and erase it from vector - to to refrain from repetition
        locSmall = std::find( varAll.begin(), varAll.end(), VarsOfKeep[i] )
            - varAll.begin();
        if( locSmall > varAll.size() )
        {
            PNL_THROW( COutOfRange, "varsOfKeep" );
        }
        varAll.erase( varAll.begin() + locSmall );
        //we find positions of vars of keep in all variables
        positionsKeep[i] = VarsOfKeep[i];
        m_NodeTypes[i] = pData->m_NodeTypes[VarsOfKeep[i]];
    }
    if( int(varAll.size()) != numNodes-NumVarsOfKeep )
    {
        PNL_THROW( CInvalidOperation,  "dims of keep not in domain" );
    }
    
    //compute offsets in vector of mean/h values
    intVector ranges = intVector(2);
    intVector nodeSizesOld = intVector( numNodes );
    intVector offsetsInOld = intVector( numNodes );
    offsetsInOld[0] = 0;
//    nodeSizesOld[0] = pData->m_NodeTypes[0]->GetNodeSize();
    nodeSizesOld[0] = pData->m_NodeTypes[0]->IsDiscrete()? 0:pData->m_NodeTypes[0]->GetNodeSize();

    for( i = 1; i < numNodes; i++ )
    {
//        nodeSizesOld[i] = pData->m_NodeTypes[i]->GetNodeSize();
        nodeSizesOld[i] =  pData->m_NodeTypes[i]->IsDiscrete() ? 0:pData->m_NodeTypes[i]->GetNodeSize();
        offsetsInOld[i] = offsetsInOld[i-1]+nodeSizesOld[i-1];
    }
    m_numberOfDims = 0;
    for( i = 0; i < NumVarsOfKeep; i++ )
    {
        m_numberOfDims += nodeSizesOld[positionsKeep[i]];
    }
    if( pData->m_bUnitFunctionDistribution )
    {
        return;
    }
    void * pObj = this;
    if( pData->m_bDeltaFunction )
    {
        C2DNumericDenseMatrix<float>* tempMat = NULL;
        pData->m_pMatrixMean->GetLinearBlocks(&positionsKeep.front(),
            positionsKeep.size(), &nodeSizesOld.front(), nodeSizesOld.size(),
            &m_pMatrixMean, &tempMat );
        m_pMatrixMean->AddRef(pObj);
        delete tempMat;
        return;
    }
    //create array with marks for every type of elements
    //0 - the position should be marginalized (its not delta position)
    //1 - the position should be marginalized but it is the delta position
    //2 - the position should be keep (and its not delta position)
    //3 - the position should be keep and its delta position
    intVector marks;
    marks.assign( numNodes, 0 );
    for( i = 0; i < NumVarsOfKeep; i++ )
    {
        marks[VarsOfKeep[i]] = 2;
    }
    //we need to find positions which are in posOfDeltaMultiply
    int numDeltaVars = pData->m_posOfDeltaMultiply.size();
    for( i = 0; i < numDeltaVars; i++ )
    {
        int deltaPos = pData->m_posOfDeltaMultiply[i];
        int curMark = marks[deltaPos];
        if( curMark == 0 )
        {
            //the position should be marginalized
            marks[deltaPos] = 1;
        }
        else
        {
            //the position 
            marks[deltaPos] = 3;
        }
    }
    
    //need to find intersection of arrays and indices in intersection
    intVector keptPosNumsInDeltaMult;//index of m_posOfDeltaMult in additive inversion to intersection
    keptPosNumsInDeltaMult.reserve(numDeltaVars);
    intVector numsOfDeltaPosInNewData;//index of positionsKeep in additive inversion of  to intersection
    numsOfDeltaPosInNewData.reserve(numDeltaVars);
    intVector posToMargInDeltaVars;//index of positionsKeep in intersection
    posToMargInDeltaVars.reserve(numDeltaVars);
    intVector posToMargOnDelta;//index of additive inversion to intersection in all dimensions
    posToMargOnDelta.reserve(NumVarsOfKeep);
    //we need to have vectors of positions Delta in new (small) data
    int curPos = 0;
    
    if( numDeltaVars > 0 )
    {
        for( i = 0; i < numDeltaVars; i++ )
        {
            curPos = pData->m_posOfDeltaMultiply[i] ;
            locSmall = std::find( positionsKeep.begin(), positionsKeep.end(),
		curPos) - positionsKeep.begin();
            if( locSmall < NumVarsOfKeep )
            {
                keptPosNumsInDeltaMult.push_back( i );
                numsOfDeltaPosInNewData.push_back( locSmall );
            }
            else
            {
                posToMargInDeltaVars.push_back( i );
                posToMargOnDelta.push_back(curPos);
            }
        }
        //need to find the positionsKeep in new matrices - the result of shrinking
        int allNumNodes = pData->m_NumberOfNodes;
        intVector posExceptMargDelta;
        posExceptMargDelta.reserve( allNumNodes );
        int numMargVarsOnDelta = posToMargOnDelta.size();
        for( i = 0; i < allNumNodes; i++ )
        {
            locSmall = std::find( posToMargOnDelta.begin(),
                posToMargOnDelta.end(), i ) - posToMargOnDelta.begin();
            if( locSmall >= numMargVarsOnDelta )
            {
                posExceptMargDelta.push_back(i);
            }
        }
        int numPosExceptMargDelta = posExceptMargDelta.size();
        //we need take into account all delta dims were multiplied
        //this dims affect on all other dims by changing their mean value.
        int numberOfKeptDelta = keptPosNumsInDeltaMult.size();
        int numOfMargDelta = numDeltaVars - numberOfKeptDelta;
        if( numberOfKeptDelta == NumVarsOfKeep )
        {
            //we just put all the delta values to new mean matrix and set flag
            //create mean matrix
            floatVector mean;
            mean.reserve(m_numberOfDims);
            for( i = 0; i < numberOfKeptDelta; i++ )
            {
                int beginCopy = pData->m_offsetToNextMean[keptPosNumsInDeltaMult[i]];
                int endCopy = pData->m_offsetToNextMean[keptPosNumsInDeltaMult[i]+1];
                int copySize = endCopy - beginCopy;
                for( j = 0; j < copySize; j++ )
                {
                    mean.push_back(pData->m_meanValuesOfMult[beginCopy+j]);
                }
            }
            intVector dims;
            dims.assign(2,0);
            dims[0] = m_numberOfDims;
            dims[1] = 1;
            m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( &dims.front(),
                &mean.front() );
            m_pMatrixMean->AddRef(pObj);
            m_bDeltaFunction = 1;
            m_bUnitFunctionDistribution = 0;
            return;
        }
        if( numOfMargDelta > 0 )
        {
            //need to provide changes at mean vector according these positions
            //find the indices
	    
            //create the matrix of observed values - values of Delta distribution
            int numMargPos = posToMargInDeltaVars.size();
            floatVector margMean;
            margMean.reserve( m_meanValuesOfMult.size() );
            for( i = 0; i < numMargPos; i++ )
            {
                int beginCopy = pData->m_offsetToNextMean[posToMargInDeltaVars[i]];
                int endCopy = pData->m_offsetToNextMean[posToMargInDeltaVars[i]+1];
                int sizeCopy = endCopy - beginCopy;
                for( j = 0; j < sizeCopy; j++ )
                {
                    margMean.push_back(pData->m_meanValuesOfMult[beginCopy+j]);
                }
            }
            intVector ranges;
            ranges.assign(2,1);
            ranges[0] = margMean.size();
            C2DNumericDenseMatrix<float>* matObs = 
                C2DNumericDenseMatrix<float>::Create( &ranges.front(), &margMean.front() );
            //create array of node sizes after first shrinking 
            //these are all marks 0,2,3
            for( i = numNodes - 1; i >=0; i-- )
            {
                if( marks[i] == 1 )
                {
                    nodeSizesOld.erase(nodeSizesOld.begin()+i);
                }
            }
            //create array of new positions of keep 
            //(in distribution after shrinking it by delta positions)
            int curIndex = 0;
            for( i = 0; i < NumVarsOfKeep; i++ )
            {
                curPos = positionsKeep[i];
                for( j = 0; j < curPos; j++ )
                {
                    if( marks[j] == 1 )
                    {
                        curIndex++;
                    }
                }
                positionsKeep[i] -= curIndex;
                curIndex = 0;
            }
            if( pData->m_bMoment )
            {
                C2DNumericDenseMatrix<float>* tempMean = NULL;
                C2DNumericDenseMatrix<float>* tempCov = NULL;
                float tempCoeff = 0.0f;
                pData->ShrinkToMatrices( &posExceptMargDelta.front(),
                    numPosExceptMargDelta, matObs, &tempMean,
                    &tempCov, &tempCoeff ); 
                //marginalize these matrices
                MarginalizeOnMatrices( &positionsKeep.front(), positionsKeep.size(),
                    &nodeSizesOld.front(), nodeSizesOld.size(), tempMean, 
                    tempCov, tempCoeff, 1, &m_pMatrixMean,
                    &m_pMatrixCov, &m_normCoeff, maximize );
                m_pMatrixMean->AddRef(pObj);
                if( m_pMatrixCov )
                {
                    m_pMatrixCov->AddRef( pObj );
                }
                else
                {
                    m_bDeltaFunction = 1;
                }
                delete tempMean;
                delete tempCov;
                if( m_bCanonical )
                {
                    m_bCanonical = 0;
                    UpdateCanonicalForm();
                }
            }
            else if( pData->m_bCanonical )
            {
                C2DNumericDenseMatrix<float>* tempH = NULL;
                C2DNumericDenseMatrix<float>* tempK = NULL;
                float tempG = 0.0f;
                pData->ShrinkToMatrices( &posExceptMargDelta.front(),
                    numPosExceptMargDelta, matObs, &tempH,
                    &tempK, &tempG ); 
                //marginalize these matrices
                MarginalizeOnMatrices( &positionsKeep.front(), positionsKeep.size(),
                    &nodeSizesOld.front(), nodeSizesOld.size(), tempH, 
                    tempK, tempG, 1, &m_pMatrixH,
                    &m_pMatrixK, &m_g, maximize );
                m_pMatrixH->AddRef(pObj);
                m_pMatrixK->AddRef(pObj);
                delete tempH;
                delete tempK;
            }
            delete matObs;
        }
        else
        {
            //provide marginalization as usual and set positions of delta multiply as usual
            if( pData->m_bMoment )
            {
                MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
                    &nodeSizesOld.front(), nodeSizesOld.size(), pData->m_pMatrixMean, 
                    pData->m_pMatrixCov, pData->m_normCoeff, 1, &m_pMatrixMean,
                    &m_pMatrixCov, &m_normCoeff, maximize );
                m_pMatrixMean->AddRef(pObj);
                if( m_pMatrixCov )
                {
                    m_pMatrixCov->AddRef( pObj );
                }
                else
                {
                    m_bDeltaFunction = 1;
                }
                if( m_bCanonical )
                {
                    m_bCanonical = 0;
                    UpdateCanonicalForm();
                }
            }
            else
            {
                if( pData->m_bCanonical )
                {
                    CGaussianDistribFun* pDistr = static_cast<CGaussianDistribFun*>(
                        pData->Clone());
                    pDistr->UpdateMomentForm();
                    MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
                        &nodeSizesOld.front(), nodeSizesOld.size(), pDistr->m_pMatrixMean, 
                        pDistr->m_pMatrixCov, pDistr->m_normCoeff, 1, &m_pMatrixMean,
                        &m_pMatrixCov, &m_normCoeff, maximize );
                    m_bCanonical = 0;
                    m_bMoment = 1;
                    m_pMatrixMean->AddRef(pObj);
                    if( m_pMatrixCov )
                    {
                        m_pMatrixCov->AddRef( pObj );
                    }
                    else
                    {
                        m_bDeltaFunction = 1;
                    }
                    UpdateCanonicalForm();
                    if( !m_bDeltaFunction)
                    {
                        m_pMatrixMean->Release(pObj);
                        m_pMatrixMean = NULL;
                        m_pMatrixCov->Release(pObj);
                        m_pMatrixCov = NULL;
                    }
                    delete pDistr;
                    m_normCoeff = 0.0f;
                    m_bMoment = 0;
                    /*MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
		    &nodeSizesOld.front(), nodeSizesOld.size(),
		    pData->m_pMatrixH, pData->m_pMatrixK, pData->m_g, 0,
		    &m_pMatrixH, &m_pMatrixK, &m_g, maximize );
                    m_pMatrixH->AddRef(pObj);
                    m_pMatrixK->AddRef(pObj);*/
                }
            }
        }
        //need to set all positions marginalized by delta and kept
        m_posOfDeltaMultiply.assign(numsOfDeltaPosInNewData.begin(),
            numsOfDeltaPosInNewData.end());
        m_meanValuesOfMult.reserve( pData->m_meanValuesOfMult.size() );
        m_offsetToNextMean.reserve(pData->m_offsetToNextMean.size());
        m_offsetToNextMean.push_back(0);
        int numKeptDelta = m_posOfDeltaMultiply.size();
        for( i = 0; i < numKeptDelta; i++ )
        {
            int beginCopy = pData->m_offsetToNextMean[keptPosNumsInDeltaMult[i]];
            int endCopy = pData->m_offsetToNextMean[keptPosNumsInDeltaMult[i]+1];
            int sizeCopy = endCopy - beginCopy;
            m_offsetToNextMean.push_back(sizeCopy);
            for( j = 0; j < sizeCopy; j++ )
            {                    
                m_meanValuesOfMult.push_back(pData->m_meanValuesOfMult[beginCopy+j]);
            }    
        }
        return;
    }
    //we need to check number of keeped positions in new delta multiply
    else
    {
        if( pData->m_bMoment )
        {
            MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
                &nodeSizesOld.front(), nodeSizesOld.size(), pData->m_pMatrixMean, 
                pData->m_pMatrixCov, pData->m_normCoeff, 1, &m_pMatrixMean,
                &m_pMatrixCov, &m_normCoeff, maximize );
            m_pMatrixMean->AddRef(pObj);
            if( m_pMatrixCov )
            {
                m_pMatrixCov->AddRef( pObj );
            }
            else
            {
                m_bDeltaFunction = 1;
            }
            if( m_bCanonical )
            {
                m_bCanonical = 0;
                UpdateCanonicalForm();
            }
        }
        else
        {
            if( pData->m_bCanonical )
            {
				MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
					&nodeSizesOld.front(), nodeSizesOld.size(), pData->m_pMatrixH, 
					pData->m_pMatrixK, pData->m_g, 0, &m_pMatrixH,
					&m_pMatrixK, &m_g, maximize );
				m_pMatrixH->AddRef(pObj);
				m_pMatrixK->AddRef(pObj);
                /*CGaussianDistribFun* pDistr = static_cast<CGaussianDistribFun*>(
                    pData->Clone());
                pDistr->UpdateMomentForm();
                MarginalizeOnMatrices( VarsOfKeep, NumVarsOfKeep,
                    &nodeSizesOld.front(), nodeSizesOld.size(), pDistr->m_pMatrixMean, 
                    pDistr->m_pMatrixCov, pDistr->m_normCoeff, 1, &m_pMatrixMean,
                    &m_pMatrixCov, &m_normCoeff, maximize );
                m_bCanonical = 0;
                m_bMoment = 1;
                m_pMatrixMean->AddRef(pObj);
                if( m_pMatrixCov )
                {
                    m_pMatrixCov->AddRef( pObj );
                }
                else
                {
                    m_bDeltaFunction = 1;
                }
                UpdateCanonicalForm();
                //need to make moment form invalid since the pData haven't valid moment form
                if( !m_bDeltaFunction )
                {
                    m_pMatrixMean->Release(pObj);
                    m_pMatrixMean = NULL;
                    m_pMatrixCov->Release(pObj);
                    m_pMatrixCov = NULL;
                }
                //delta distribution have matrix mean and canonical form falg only
                m_normCoeff = 0.0f;
                m_bMoment = 0;
                delete pDistr;*/
            }
            else
            {
                PNL_THROW( CInvalidOperation, "marginalization invalid data" );
            }
        }
    }
}
void CGaussianDistribFun::ShrinkObservedNodes( const CDistribFun* pOldData, 
					      const int *pVarsObserved, 
					      const Value* const* pObsValues,
					      int numObsVars,
					      const CNodeType* pObsTabNT,
					      const CNodeType* pObsGauNT )
{
    PNL_CHECK_IS_NULL_POINTER( pOldData );
    if( pOldData->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInconsistentType, "we need to shrink Gaussian" );
    }
    const CGaussianDistribFun *pData = (const CGaussianDistribFun*)pOldData;
    if( !pData->m_bPotential )
    {
        PNL_THROW( CInvalidOperation, "we can shrink only factor" );
    }
    void *pObj = this;
    m_NodeTypes = pConstNodeTypeVector(pData->m_NodeTypes.begin(), 
	pData->m_NodeTypes.end());// may be optimized    
    if( numObsVars == 0 )
    {
        ReleaseAllMatrices();
        //add referencies for new matrices
        if( pData->m_pMatrixH )
        {
            m_pMatrixH = static_cast<C2DNumericDenseMatrix<float>*>(
		pData->m_pMatrixH->Clone());
            m_pMatrixH->AddRef(pObj);
        }
        if( pData->m_pMatrixK )
        {
            m_pMatrixK = static_cast<C2DNumericDenseMatrix<float>*>(
		pData->m_pMatrixK->Clone());
            m_pMatrixK->AddRef(pObj);
        }
        m_g = pData->m_g;
        m_bCanonical = pData->m_bCanonical;
        if( pData->m_pMatrixMean )
        {
            m_pMatrixMean = static_cast<C2DNumericDenseMatrix<float>*>(
		pData->m_pMatrixMean->Clone());
            m_pMatrixMean->AddRef(pObj);
        }
        if( pData->m_pMatrixCov )
        {
            m_pMatrixCov = static_cast<C2DNumericDenseMatrix<float>*>(
		pData->m_pMatrixCov->Clone());
            m_pMatrixCov->AddRef(pObj);
        }
        m_normCoeff = pData->m_normCoeff;
        m_bMoment = pData->m_bMoment;
        m_bPotential = pData->m_bPotential;
        m_numberOfDims = pData->m_numberOfDims;
        m_NumberOfNodes = pData->m_NumberOfNodes;
        m_DistributionType = pData->m_DistributionType;
        m_bUnitFunctionDistribution = pData->m_bUnitFunctionDistribution;
        m_bDeltaFunction = pData->m_bDeltaFunction;
        return;
    }
    if( !pVarsObserved )
    {
        PNL_THROW( CNULLPointer, "pointer to observed dims" )
    }
    if( !pObsValues )
    {
        PNL_THROW( CNULLPointer, "pointer to observed values" );
    }
    PNL_CHECK_IS_NULL_POINTER( pObsGauNT );
    int i, j;
    int numNodes = pData->GetNumberOfNodes();
    if( numNodes < numObsVars )
    {
        PNL_THROW( COutOfRange,
            "number of observed nodes should be less than number of all nodes" );
    }
    intVector variables = intVector(numNodes);
    intVector nodeSizes = intVector( numNodes );//to partition into blocks
    intVector offsetsAll = intVector( numNodes );
    offsetsAll[0] = 0;
    nodeSizes[0] = pData->m_NodeTypes[0]->GetNodeSize();
    for( i = 1; i <numNodes; i++ )
    {
        variables[i] = i;
        nodeSizes[i] = pData->m_NodeTypes[i]->GetNodeSize();
        offsetsAll[i] = offsetsAll[i-1]+nodeSizes[i-1];
    }
    //to check: is this positions the subset of domain
    intVector varsToCheck = intVector( variables.begin(), variables.end());
    //positions of observed nodes in data
    intVector posOfObs = intVector( numObsVars );
    //positions of kept(unobserved)
    intVector keepedPos = intVector( numNodes - numObsVars );
    //length of vectors
    int lengthObserved = 0;
    int keepedLength = 0;
    //temporary pointers 
    int locAll, locSmall;
    //need to create new vector of node types
    for( i = 0; i < numObsVars; i++ )
    {
        locSmall = std::find( varsToCheck.begin(), varsToCheck.end(), 
            pVarsObserved[i]) - varsToCheck.begin();
        if( locSmall >= varsToCheck.size() )
        {
            PNL_THROW( COutOfRange, 
                "numbers of observed nodes - should be subset" )
        }
        varsToCheck.erase( varsToCheck.begin() + locSmall );
        locAll = std::find( variables.begin(), variables.end(), 
            pVarsObserved[i]) - variables.begin();
        posOfObs[i] = locAll;    
        lengthObserved += nodeSizes[posOfObs[i]];
    }
    if( int(varsToCheck.size()) != numNodes - numObsVars )
    {
        PNL_THROW( COutOfRange, "some of observed variables" );
    }
    //change node types to observed
    //all node types here only observed Gaussian!!!
    for( i = 0; i < numObsVars; i++ )
    {
        m_NodeTypes[posOfObs[i]] = pObsGauNT;
    }
    //need to work on fully observed potential separetly - it should be scalar disribution
    /*if( numNodes == numObsVars )
    {
    //PNL_THROW( CNotImplemented, "fully observed Gaussian potential" );
    m_numberOfDims = keepedLength;
    m_bCanonical = pData->m_bCanonical;
    m_bMoment = pData->m_bMoment;
    m_bDeltaFunction = 0;
    m_bUnitFunctionDistribution = 1;//we have unit function!!!
    ReleaseAllMatrices();
    m_meanValuesOfMult.clear();
    m_posOfDeltaMultiply.clear();
    m_offsetToNextMean.clear();
    return;
    }*/
    for( i = 0; i < numNodes - numObsVars; i++ )
    {
        locAll = std::find( variables.begin(), variables.end(), 
            varsToCheck[i] ) - variables.begin();
        keepedPos[i] = locAll;
        keepedLength += nodeSizes[keepedPos[i]];
    }
    int offset = 0;
    int sizeToCopy;
    floatVector observedValues = floatVector( lengthObserved );
    for( i = 0; i < numObsVars; i++ )
    {
        sizeToCopy = nodeSizes[posOfObs[i]];
        for( j = 0; j < sizeToCopy; j++ )
        {
            observedValues[offset] = ((pObsValues[i])[j]).GetFlt();
            offset++;
        }
    }
    intVector ranges = intVector(2);
    ranges[0] = lengthObserved;
    ranges[1] = 1;
    C2DNumericDenseMatrix<float> *matObserved = 
        C2DNumericDenseMatrix<float>::Create( &ranges.front(), &observedValues.front());
    //we partition all matrices into blocks & multiply it to get new distribution
    ReleaseAllMatrices();
    //check if the model is delta function
    if( pData->m_bDeltaFunction)
    {
        C2DNumericDenseMatrix<float> *matMeanObserved = NULL;
        C2DNumericDenseMatrix<float> *matMeanUnObserved = NULL;
        pData->m_pMatrixMean->GetLinearBlocks( &keepedPos.front(), keepedPos.size(),
	    &nodeSizes.front(), pData->m_NumberOfNodes, &matMeanUnObserved,
	    &matMeanObserved);
        CNumericDenseMatrix<float>* matDiff = pnlCombineNumericMatrices(matMeanObserved,
            matObserved, 0);
        float sumAll = matDiff->SumAll(1);
        delete matDiff;
        delete matMeanObserved;
        float epsilon = 1e-5f;
        if( sumAll > epsilon )
        {
            PNL_THROW(CInvalidOperation,
                "can shrink Gaussian Delta distribution only for the same evidence");
        }
        else
        {
            m_pMatrixMean = matMeanUnObserved;
            m_pMatrixMean->AddRef(pObj);
            m_bUnitFunctionDistribution = pData->m_bUnitFunctionDistribution;
            m_bDeltaFunction = pData->m_bDeltaFunction;
            m_bMoment = pData->m_bMoment;
            m_bCanonical = pData->m_bCanonical;
        }
        delete matObserved;
        return;
    }
    if( pData->m_bCanonical )
    {
        m_bMoment = 0;
        m_bCanonical = 1;
        pData->ShrinkToMatrices( &keepedPos.front(), keepedPos.size(),
	    matObserved, &m_pMatrixH, &m_pMatrixK, &m_g );
        m_pMatrixH->AddRef(pObj);
        m_pMatrixK->AddRef(pObj);
        if( pData->m_bMoment )
        {
            UpdateMomentForm();
        }
    }
    else
    {
        if( pData->m_bMoment )
        {
            m_bCanonical = 0;
            //we can do it directly - by matrix operations
            m_bMoment = 1;
            pData->ShrinkToMatrices( &keepedPos.front(), keepedPos.size(),
		matObserved, &m_pMatrixMean, &m_pMatrixCov, &m_normCoeff );
            m_pMatrixMean->AddRef(pObj);
            m_pMatrixCov->AddRef(pObj);
        }
        else
        {
            PNL_THROW( CInvalidOperation, "no valid form to shrink it" );
        }
    }
    delete matObserved;
}

void CGaussianDistribFun::MarginalizeOnMatrices( const int* keepVariables,
						int numKeepVariables,
						const int* allVariableSizes, 
						int numVariables,
						C2DNumericDenseMatrix<float>* vecMat,
						C2DNumericDenseMatrix<float>* squareMat,
						float coeff, int isInMoment,
						C2DNumericDenseMatrix<float>** resVecMat,
						C2DNumericDenseMatrix<float>** resSquareMat,
						float *resCoeff,
						int maximize )const
{
    PNL_CHECK_LEFT_BORDER( numKeepVariables, 1 );
    PNL_CHECK_IS_NULL_POINTER( keepVariables );
    PNL_CHECK_IS_NULL_POINTER( allVariableSizes );
    
    if( isInMoment == 1 )
    {
        C2DNumericDenseMatrix<float>* temp = NULL;
        C2DNumericDenseMatrix<float>* temp1 = NULL;
        C2DNumericDenseMatrix<float>* temp2 = NULL;
        squareMat->GetBlocks( keepVariables, numKeepVariables,
            allVariableSizes, numVariables, resSquareMat, &temp, &temp1,
            &temp2 );
        delete temp;
        vecMat->GetLinearBlocks( keepVariables, numKeepVariables,
            allVariableSizes, numVariables, resVecMat, &temp );
        const int* dimSizes;
        int dimSize;
        (*resSquareMat)->GetRanges( &dimSize, &dimSizes );
        dimSize = dimSizes[0];
        float p1 = (float)pow((double)(2*PNL_PI),(dimSize/2.0));
        float detCov = (*resSquareMat)->Determinant();
        if( detCov <= 0 )
        {
            //PNL_THROW( CInternalError,
	    //	"covariance matrix must be positive semidifinite" );
            delete (*resSquareMat);
            (*resSquareMat) = NULL;
            *resCoeff = 1;
        }
        else
        {
            float p2 = (float)pow((double)detCov,0.5);
	    *resCoeff = 1/(p1*p2);
        }
        delete temp;
        delete temp1;
        delete temp2;
        return;
    }
    else
    {
		C2DNumericDenseMatrix<float>* matHKeep = NULL;
		C2DNumericDenseMatrix<float>* matHSum = NULL;
		vecMat->GetLinearBlocks( keepVariables, numKeepVariables,
			allVariableSizes, numVariables, &matHKeep, &matHSum );

		C2DNumericDenseMatrix<float> *matXK = NULL;
		C2DNumericDenseMatrix<float> *matYK = NULL;
		C2DNumericDenseMatrix<float> *matXYK = NULL;
		C2DNumericDenseMatrix<float> *matYXK = NULL;
		squareMat->GetBlocks(  keepVariables, numKeepVariables,
			allVariableSizes, numVariables, &matXK, &matYK, &matXYK, &matYXK );
		if (!matHSum)
		{
			*resVecMat = matHKeep;
			*resSquareMat = matXK;
			*resCoeff = coeff;
			return;
		}
		C2DNumericDenseMatrix<float> *matHSUMTr = matHSum->Transpose();
		C2DNumericDenseMatrix<float> *matYKinverse = matYK->Inverse();
		C2DNumericDenseMatrix<float> *matMult1 = 
			pnlMultiply(matYKinverse, matHSum, maximize);
		C2DNumericDenseMatrix<float> *matMult2 = pnlMultiply(matHSUMTr,matMult1, maximize );
		int multSize;const float*dataMult;
		matMult2->GetRawData( &multSize, &dataMult );
		if( multSize != 1 )
		{
			PNL_THROW( CInvalidOperation,
				"result should be one float number" );
		}
		*resCoeff = (float)coeff + 0.5f * (m_numberOfDims * (float)log(2 * PNL_PI) - 
			(float)log(matYK->Determinant()) + dataMult[0]);
		delete matMult1;
		delete matMult2;
		matMult1 = pnlMultiply( matXYK, matYKinverse, maximize );
		matMult2 = pnlMultiply( matMult1,matHSum, maximize);
		//substract in combining matrices
		*resVecMat = static_cast<C2DNumericDenseMatrix<float>*>(
			pnlCombineNumericMatrices( matHKeep, matMult2, 0 ));
		delete matMult2;
		//substract in combining matrices
		matMult2 = pnlMultiply( matMult1, matYXK, maximize);

		//make matrix simmetric to correct miscalculation
		float *matVec = (float*)&(matMult2->GetVector()->front());
		const int *ranges;
		int numDims;
		matMult2->GetRanges(&numDims, &ranges);
		int i,j;
		int size = ranges[0];
		for( i = 0; i < size ; i++)
		{
			for( j = 0; j < i; j++ )
			{
				matVec[i*size+j] = matVec[j*size+i];
			}
		}
		
		*resSquareMat = static_cast<C2DNumericDenseMatrix<float>*>(
			pnlCombineNumericMatrices( matXK, matMult2, 0 ));
		delete matMult1;
		delete matMult2;
		delete matXK;
		delete matYK;
		delete matYKinverse;
		delete matXYK;
		delete matYXK;
		delete matHSum;
		delete matHSUMTr;
		delete matHKeep;
		return;
    }
}
						
void CGaussianDistribFun::ShrinkToMatrices( const int* keepPositions, 
    int numKeepPositions, 
    C2DNumericDenseMatrix<float>* obsMatrix, 
    C2DNumericDenseMatrix<float>** resVecMat,
    C2DNumericDenseMatrix<float>** resSquareMat,
    float *coeff )const
{
    PNL_CHECK_LEFT_BORDER( numKeepPositions, 1 );
    PNL_CHECK_IS_NULL_POINTER( keepPositions );
    PNL_CHECK_IS_NULL_POINTER( obsMatrix );
    
    //find the node sizes
    intVector nodeSizes;
    nodeSizes.resize( m_NumberOfNodes );
    int i;
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
	nodeSizes[i] = m_NodeTypes[i]->GetNodeSize();
    }
    int flagOfMaximizeWithMult = 0;
    if( m_bCanonical )
    {
	C2DNumericDenseMatrix<float> *matHObserved = NULL;
	C2DNumericDenseMatrix<float> *matHUnObserved = NULL;
	m_pMatrixH->GetLinearBlocks( keepPositions, numKeepPositions,
	    &nodeSizes.front(), m_NumberOfNodes, &matHUnObserved,
	    &matHObserved);
	C2DNumericDenseMatrix<float> *matY = NULL;
	C2DNumericDenseMatrix<float> *matXY = NULL;
	C2DNumericDenseMatrix<float> *matYX = NULL;
	m_pMatrixK->GetBlocks( keepPositions, numKeepPositions,
	    &nodeSizes.front(), nodeSizes.size(),
	    resSquareMat, &matY, &matXY, &matYX );
	int flagOfMaximizeWithMult = 0;
	C2DNumericDenseMatrix<float> *tempMat = pnlMultiply( matXY, 
	    obsMatrix, flagOfMaximizeWithMult );
	(*resVecMat) = static_cast<C2DNumericDenseMatrix<float>*>(
	    pnlCombineNumericMatrices( matHUnObserved, tempMat, 0 ));
	delete tempMat;
	C2DNumericDenseMatrix<float> *tempMat2 = obsMatrix->Transpose();
	tempMat = pnlMultiply( matY, obsMatrix, flagOfMaximizeWithMult );
	C2DNumericDenseMatrix<float>* tempMat3 = pnlMultiply( tempMat2,
	    tempMat, flagOfMaximizeWithMult );
	const floatVector *vecData = tempMat3->GetVector();
	if( vecData->size()!=1 )
	{
	    PNL_THROW( CInvalidOperation, "multiplication" );
	}
	delete tempMat;
	*coeff = (m_g)-0.5f*(*vecData)[0] ;
	delete tempMat3;
	delete tempMat2;
	tempMat = matHObserved->Transpose();
	delete matHObserved;
	delete matHUnObserved;
	tempMat2 = pnlMultiply( tempMat, obsMatrix, flagOfMaximizeWithMult );
	vecData = tempMat2->GetVector();
	if( vecData->size()!=1 )
	{
	    PNL_THROW( CInvalidOperation, "multiplication" );
	}
	delete tempMat;
	delete matY;
	delete matXY;
	delete matYX;
	*coeff = (*coeff)+(*vecData)[0];
	delete tempMat2;
	return;
    }
    if( m_bMoment )
    {
	C2DNumericDenseMatrix<float> *matMeanObserved = NULL;
	C2DNumericDenseMatrix<float> *matMeanUnObserved = NULL;
	m_pMatrixMean->GetLinearBlocks( keepPositions, numKeepPositions,
	    &nodeSizes.front(), m_NumberOfNodes, &matMeanUnObserved,
	    &matMeanObserved);
	C2DNumericDenseMatrix<float>* matrixK = m_pMatrixCov->Inverse();
	C2DNumericDenseMatrix<float> *matX = NULL;
	C2DNumericDenseMatrix<float> *matY = NULL;
	C2DNumericDenseMatrix<float> *matXY = NULL;
	C2DNumericDenseMatrix<float> *matYX = NULL;
	matrixK->GetBlocks( keepPositions, numKeepPositions,
	    &nodeSizes.front(), nodeSizes.size(),
	    &matX, &matY, &matXY, &matYX);
	delete matrixK;
	*resSquareMat = matX->Transpose();
	C2DNumericDenseMatrix<float>* meanComp = pnlMultiply( matX,
	    matMeanUnObserved, flagOfMaximizeWithMult );
	C2DNumericDenseMatrix<float>* meanMult = 
	    static_cast<C2DNumericDenseMatrix<float>*>(pnlCombineNumericMatrices( 
	    obsMatrix, matMeanObserved, 0 ));
	C2DNumericDenseMatrix<float>* meanCompSubstr = pnlMultiply( matXY,
	    meanMult, flagOfMaximizeWithMult );
	C2DNumericDenseMatrix<float>* substrRes = 
	    static_cast<C2DNumericDenseMatrix<float>*>(pnlCombineNumericMatrices( 
	    meanComp, meanCompSubstr, 0 ));
	*resVecMat = pnlMultiply( *resSquareMat, substrRes,
	    flagOfMaximizeWithMult );
	delete matMeanUnObserved;
	delete matMeanObserved;
	delete matX;
	delete matY;
	delete matXY;
	delete matYX;
	delete meanComp;
	delete meanMult;
	delete meanCompSubstr;
	delete substrRes;
    }
}


void CGaussianDistribFun::ExpandData( const int* pDimsToExtend, 
				     int numDimsToExpand,
				     const Value* const* valuesArray, 
				     const CNodeType* const *allFullNodeTypes,
				     int UpdateCanonical)
{
    if( !m_bPotential )
    {
        PNL_THROW( CInvalidOperation, "we can expand only factors" )
    }
    if((!pDimsToExtend)||(!valuesArray)||(!allFullNodeTypes))
    {
        PNL_THROW( CNULLPointer, "one of inputs" )
    }
    //we have some dims of size 0 - need to expand it to the size from allFullNodeTypes
    //check if we try to expand distribution without matrices (fully observed)
    //if(  )
    if(( numDimsToExpand>0 ))
    {
        //we need to add float values from valuesArray to mean and zeros&ones to Cov
        //so we can do it in moment form;
        int i,j,k;
        int newMeanSize = 0;
        int oldMeanSize = 0;
        intVector exisitngDims = intVector( m_NumberOfNodes );
        int loc, sizeHere, discreteIn, discreteHere;
        intVector NodeSizesNew = intVector( m_NumberOfNodes );
        intVector offsetsOld = intVector( m_NumberOfNodes );
        intVector nodeSizesOld = intVector( m_NumberOfNodes );
        intVector offsNew = intVector( m_NumberOfNodes );
        int offNew = 0;
        int sizeNew = 0;
        int offOld = 0;
        int sizeOld = 0;
        for( i = 0; i < m_NumberOfNodes; i++ )
        {
            //old offsets and node sizes
            offsetsOld[i] = offOld + sizeOld;
            offOld = offsetsOld[i];
            sizeOld = m_NodeTypes[i]->GetNodeSize();
            nodeSizesOld[i] = sizeOld;
            oldMeanSize +=sizeOld;
            //new offsets and node sizes
            offsNew[i] = offNew + sizeNew;
            offNew = offsNew[i];
            sizeNew = allFullNodeTypes[i]->GetNodeSize();
            NodeSizesNew[i] = sizeNew;
            newMeanSize += NodeSizesNew[i];
            //create array of existing dims
            exisitngDims[i] = i;
            loc = std::find( pDimsToExtend, pDimsToExtend + numDimsToExpand,i)
                - pDimsToExtend;
            if( loc >= numDimsToExpand )//this node needn't to be extend
            {
                sizeHere = m_NodeTypes[i]->GetNodeSize();
                discreteHere = m_NodeTypes[i]->IsDiscrete();
                discreteIn = allFullNodeTypes[i]->IsDiscrete();
                if(( sizeHere!=NodeSizesNew[i] )||(discreteIn != discreteHere)
                    ||( discreteIn!=0 ))
                {
                    PNL_THROW( CInvalidOperation, 
			"nonexpanded dims should have same sizes all node types - gaussian" )
                }
            }
        }
        //we need to check subset in indices
        for( i = 0; i < numDimsToExpand; i++ )
        {
            loc = std::find( exisitngDims.begin(), exisitngDims.end(), 
                pDimsToExtend[i] ) - exisitngDims.begin();
            if( loc < exisitngDims.size() )
            {
                exisitngDims.erase(exisitngDims.begin() + loc);
            }
            else
            {
                PNL_THROW( COutOfRange, "one of dimsToExpand" )
            }
        }
        //we need to update moment form as we have 
        UpdateMomentForm();
        //we need to find blocks in existing data and put it in bigData for mean and covariance
        const floatVector *meanOld = m_pMatrixMean->GetVector();
        const floatVector *covOld = m_pMatrixCov->GetVector();
        //add new means for extended dims
        floatVector meanNew = floatVector( meanOld->begin(), meanOld->end());
        int offsetAdd = 0;
        for( i = 0; i < numDimsToExpand; i++ )    
        {
            sizeHere = NodeSizesNew[pDimsToExtend[i]];
            floatVector insVec = floatVector(sizeHere);
            for( j = 0; j < sizeHere; j++ )
            {
                insVec[j] = valuesArray[i][j].GetFlt();
            }
            meanNew.insert( meanNew.begin() + offsetAdd + 
                offsetsOld[pDimsToExtend[i]], insVec.begin(), insVec.end());
            //insert mean for 
            m_meanValuesOfMult.insert( m_meanValuesOfMult.end(),
                insVec.begin(), insVec.end() );
            offsetAdd +=sizeHere;
        }
        //create new covariance matrix of zeros and add information from old covarience matrix
        floatVector covNew = floatVector( newMeanSize*newMeanSize, 0);
        //copy data from old covarience matrix
        int numVarsToCopy = exisitngDims.size();
        for( i = 0; i < numVarsToCopy; i++ )
        { 
            int horNum = exisitngDims[i];
            int offsHorizSize = offsNew[horNum];
            int oldOffHorSize = offsetsOld[i];
            //copy the part of covariance self-to-self and self-to-other
            for( j = 0; j < numVarsToCopy; j++ )
            {
                int vertNum = exisitngDims[j];
                int offsVertSize = offsNew[vertNum];
                int oldOffVertSize = offsetsOld[j];
                int copySize = NodeSizesNew[horNum];
                int numLinesToCopy = NodeSizesNew[vertNum];
                for( k = 0; k < numLinesToCopy; k++ )
                {
                    memcpy( &covNew.front() + offsHorizSize + (offsVertSize+k)*newMeanSize,
			&covOld->front() + oldOffHorSize + (oldOffVertSize+k)*oldMeanSize ,
			copySize * sizeof(float)/sizeof(char));
                }
            }
        }
        void *pObj = this;
        m_pMatrixMean->Release(pObj);
        m_pMatrixCov->Release(pObj);
        m_numberOfDims = newMeanSize;
        intVector ranges = intVector(2);
        ranges[1] = 1;
        ranges[0] = newMeanSize;
        m_pMatrixMean = C2DNumericDenseMatrix<float>::Create( &ranges.front(), &meanNew.front() );
        m_pMatrixMean->AddRef(pObj);
        ranges[1] = newMeanSize;
        m_pMatrixCov = C2DNumericDenseMatrix<float>::Create( &ranges.front(), &covNew.front());
        m_pMatrixCov->AddRef(pObj);
        m_bCanonical = 0;
        m_normCoeff = 1.0f;//fixme - we have singular matrix - no real norm Coeff
        //we need to set new node types:
        m_NodeTypes = pConstNodeTypeVector(allFullNodeTypes, 
            allFullNodeTypes + m_NumberOfNodes);
        //set evidence as result of multiplying by Delta distribution
        m_posOfDeltaMultiply.reserve( m_posOfDeltaMultiply.size() + numDimsToExpand );
        if( m_offsetToNextMean.empty() )
        {
            m_offsetToNextMean.push_back(0);
        }
        m_offsetToNextMean.reserve(numDimsToExpand + m_offsetToNextMean.size());
        int nodeSize;
        for( i = 0; i < numDimsToExpand; i++ )
        {
            m_posOfDeltaMultiply.push_back( pDimsToExtend[i] );
            nodeSize = m_NodeTypes[pDimsToExtend[i]]->GetNodeSize();
            m_offsetToNextMean.push_back(m_offsetToNextMean[
                    m_offsetToNextMean.size()-1]+nodeSize );
        }
        // may be optimized
        m_pMatrixH->Release(pObj);
        m_pMatrixH = NULL;
        m_pMatrixK->Release(pObj);
        m_pMatrixK = NULL;
        m_g = FLT_MAX;
        if(  UpdateCanonical  )
        {
            UpdateCanonicalForm();
        }
    }            
}
void CGaussianDistribFun::ClearStatisticalData()
{
    if (m_pLearnMatrixMean)
    {
        m_pLearnMatrixMean->ClearData();
    }
    
    if (m_pLearnMatrixCov)
    {
        m_pLearnMatrixCov->ClearData();
    }
}



void CGaussianDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
					     const CEvidence *pEvidence, 
					     float weightingCoeff,
					     const int* domain ) 
{
    
    if( !infData )
    {
        PNL_THROW( CNULLPointer, "resalts of inference" )
    }
    const CGaussianDistribFun* myinfData; 
    
    CGaussianDistribFun* pGaussDistr = NULL;
    
    if(!static_cast<const CGaussianDistribFun* >(infData)->GetMomentFormFlag())
    {
	pGaussDistr = static_cast<CGaussianDistribFun*>(infData->Clone());
	pGaussDistr->UpdateMomentForm();
	myinfData = pGaussDistr;
    }
    else
    {
        myinfData = static_cast<const CGaussianDistribFun* >(infData);
    }
    
    
    if( !m_pLearnMatrixMean || !m_pLearnMatrixCov)
    {
        int length = 0;    
        for (int node = 0; node < m_NumberOfNodes; node++)
        {
            length += ( m_NodeTypes[node]->GetNodeSize() );
        }
        float *data = new float [length];
        int *ranges = new int [2];
        ranges[0] = length;
        ranges[1] = 1;
        m_pLearnMatrixMean = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixMean -> ClearData();
        delete []data;
        ranges[1] = length;
        data = new float [length*length];
        m_pLearnMatrixCov = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixCov -> ClearData();
        delete []data;
        delete []ranges;
    }
    const float *meanFromInf = NULL;
    floatVector mean;
    int meanSize = 0;
    
    const float *covFromInf = NULL;
    floatVector cov;
    
    int covSize;
    CNumericDenseMatrix<float>* matMT = NULL;
    CNumericDenseMatrix<float>* matCT = NULL;
    switch(myinfData->IsDistributionSpecific())
    {
    case 0:
        matMT = static_cast<CNumericDenseMatrix<float>*>(
            myinfData->GetMatrix(matMean));
        matMT->GetRawData(&meanSize, &meanFromInf);
        mean.assign( meanFromInf, meanFromInf + meanSize);
        matCT = static_cast<CNumericDenseMatrix<float>*>(
            myinfData->GetMatrix(matCovariance));
        matCT->GetRawData(&covSize, &covFromInf);
        cov.assign(covFromInf, covFromInf+covSize);
        break;
    case 1:
        PNL_THROW(CAlgorithmicException, "uniform distribution");
        break;
    case 2: 
        matMT = static_cast<CNumericDenseMatrix<float>*>(
            myinfData->GetMatrix(matMean));
        matMT->GetRawData(&meanSize, &meanFromInf);
        mean.assign( meanFromInf, meanFromInf + meanSize);
        cov.assign(meanSize*meanSize, 0.0f);
        break;
    case 3:
        matMT = static_cast<CNumericDenseMatrix<float>*>(
            myinfData->GetMatrix(matMean));
        matMT->GetRawData(&meanSize, &meanFromInf);
        mean.assign( meanFromInf, meanFromInf + meanSize);
        matCT = static_cast<CNumericDenseMatrix<float>*>(
            myinfData->GetMatrix(matCovariance));
        matCT->GetRawData(&covSize, &covFromInf);
        cov.assign(covFromInf, covFromInf+covSize);
        int nPositions;
        const int* positions;
        const float* values;
        const int* offsets;
        
        nPositions = myinfData->GetMultipliedDelta(&positions, &values, &offsets);
        for(int i =  0; i < nPositions; i++)
        {
            memcpy(&mean[positions[i]], &values[offsets[i]], offsets[i+1]-offsets[i]);///???
            for(int j = 0; j < nPositions; j++)
            {
                fill(&cov[positions[i]*meanSize], &cov[positions[i]*meanSize + meanSize-1], 0.0f);
                for(int k = 0; k < meanSize; k++)
                {
                    cov[positions[i] +k*meanSize] = 0.0f;
                }
            }
        }
	
	
        break;
	
    }
    if(pGaussDistr)
    {
        delete pGaussDistr;
    }
    
    int offset = 0;
    float value = 0; 
    for( int coordinate_i = 0; coordinate_i < meanSize; coordinate_i++)
    {
        value = ( m_pLearnMatrixMean->GetElementByOffset( coordinate_i ) ) 
            + mean[coordinate_i]*weightingCoeff;
        m_pLearnMatrixMean -> SetElementByOffset(value, coordinate_i);
        
        for ( int coordinate_j = 0; coordinate_j < meanSize; coordinate_j++)
        {
            value = ( m_pLearnMatrixCov -> 
                GetElementByOffset( offset ) )
                + (mean[coordinate_i]*mean[coordinate_j] + cov[offset])*weightingCoeff;
            m_pLearnMatrixCov -> 
                SetElementByOffset( value, offset );
            offset++;
        }
    }
}

void CGaussianDistribFun::SetStatistics( const CMatrix<float> *pMat, 
					EStatisticalMatrix matrix, const int* parentsComb )
{
    PNL_CHECK_IS_NULL_POINTER( pMat );
    
    EMatrixClass cl = pMat->GetMatrixClass();
    if(  cl != mc2DNumericDense )
    {
	PNL_THROW(CInconsistentType, "matrix should be 2D numeric dense" );
    }
    switch(matrix) 
    {
    case stMatMu:
	{
	    if( !m_pLearnMatrixMean )
	    {
		int length = 0;    
		for (int node = 0; node < m_NumberOfNodes; node++)
		{
		    length += ( m_NodeTypes[node]->GetNodeSize() );
		}
		int ranges[2] = { length, 1 };
		int numEl;
		const float* data;
		static_cast<const C2DNumericDenseMatrix<float >*>(pMat)->GetRawData(
		    &numEl, &data );
		PNL_CHECK_FOR_NON_ZERO(numEl - length);
		m_pLearnMatrixMean = C2DNumericDenseMatrix<float>::Create( ranges,
		    data );
	    }
	    else
	    {
		m_pLearnMatrixMean->SetDataFromOtherMatrix(pMat);
	    }
	    break;
	}
    case stMatSigma:
	{
	    if(!m_pLearnMatrixCov)
	    {
		int length = 0;    
		for (int node = 0; node < m_NumberOfNodes; node++)
		{
		    length += ( m_NodeTypes[node]->GetNodeSize() );
		}
		int ranges[2] = {length, length};
		int numEl;
		const float* data;
		static_cast<const C2DNumericDenseMatrix<float>*>(pMat)->GetRawData(
		    &numEl, &data );
		PNL_CHECK_FOR_NON_ZERO(numEl - length * length);
		m_pLearnMatrixCov= C2DNumericDenseMatrix<float>::Create( ranges,
		    data );
	    }
	    m_pLearnMatrixCov->SetDataFromOtherMatrix( pMat );
	    break;
	}
    default: PNL_THROW(CBadArg, "number of matrix");
    }
}


void CGaussianDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences,
					     int EvidenceNumber,
					     const int *domain, 
					     float weightingCoeff)
{
    if( !pEvidences )
    {
        PNL_THROW( CNULLPointer, "evidences" )//no Evidences - NULL pointer
    }
    
    if( !m_pLearnMatrixMean || !m_pLearnMatrixCov)
    {
        int length = 0;    
        for (int node = 0; node < m_NumberOfNodes; node++)
        {
            length += ( m_NodeTypes[node]->GetNodeSize() );
        }
        float *data = new float [length];
        int *ranges = new int [2];
        ranges[0] = length;
        ranges[1] = 1;
        m_pLearnMatrixMean = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixMean->ClearData();
        delete []data;
        ranges[1] = length;
        data = new float [length*length];
        m_pLearnMatrixCov = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixCov->ClearData();
        delete []data;
        delete []ranges;
    }
    floatVector ev;
    for( int evNumber = 0; evNumber < EvidenceNumber; evNumber++ )
    {
        if( !pEvidences[evNumber] )
        {
            PNL_THROW( CNULLPointer, "evidence" )//no Evidence - NULL pointer
        }
        ev.clear();
        for( int i = 0; i < m_NumberOfNodes; i++ )
        {
            int node = domain[i];
            const Value *ev_i = pEvidences[evNumber]->GetValue(node);
            for (int j = 0; j < (m_NodeTypes[i]->GetNodeSize()); j++)
            {
                ev.push_back(ev_i[j].GetFlt());
            }
        }
        
        int offset = 0;
        float value = 0; 
        for( int coordinate_i = 0; coordinate_i < ev.size(); coordinate_i++)
        {
            value = ( m_pLearnMatrixMean->GetElementByOffset( coordinate_i ) ) 
                + ev[coordinate_i]*weightingCoeff;
            m_pLearnMatrixMean -> SetElementByOffset(value, coordinate_i);
            
            for ( int coordinate_j = 0; coordinate_j < ev.size(); coordinate_j++)
            {
                value = ( m_pLearnMatrixCov -> 
                    GetElementByOffset( offset ) )
                    + ev[coordinate_i] * ev[coordinate_j]*weightingCoeff;
                m_pLearnMatrixCov -> 
                    SetElementByOffset( value, offset );
                offset++;
            }
        }
        
    }
    
}

#if 0
float CGaussianDistribFun::ProcessingStatisticalData( float numEvidences ) 
{
    if( numEvidences <= 0 )
    {
        PNL_THROW( CInconsistentSize, "num evidences must be more than 0" )
    }
    if( m_bPotential )
    {
        PNL_THROW( CInvalidOperation,
            "this version of learning is only for CPD" );
    }
    
    if( !m_pLearnMatrixCov || !m_pLearnMatrixMean )
    {
        PNL_THROW( CInvalidOperation, "can't work without learn matrices" );
    }
    
    if( !m_pMatrixCov )
    {
        //fixme - create new covariance matrix
        m_pMatrixCov = C2DNumericDenseMatrix<float>::CreateIdentityMatrix(
            m_NodeTypes.back()->GetNodeSize());
        void* pObj = this;
        m_pMatrixCov->AddRef(pObj);
    }
    int flagMaximize = 0;
    float m_critVal = 0.0f;
    int chldSize = m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize();
    int meanIsNotClamp = m_pMatrixMean->GetClampValue()==0 ? 1 : 0;
    int i,j;
    float value;
    int indexes[2];
    floatVector data;
    int sizes[2];
    int length = 0;
    
    
    if(m_NumberOfNodes > 1)
    {
        //(y*x2'-mu*x2'-Bi*x2')
        int node;
        
        int notclLinesize = 0;
        int clLinesize = 0;
        
        floatVector notClPrtsData;
        floatVector clPrtsData;
        intVector notClWeightsNumbers;
        intVector ClWeightsNumbers;
        intVector blockSizes = intVector(m_NumberOfNodes);
        int offset = 0;
        
        for( node = 0; node < m_NumberOfNodes; node++ )
        {
            int sz = m_NodeTypes[node]->GetNodeSize();
            blockSizes[node] = sz;
            if( node < m_NumberOfNodes - 1)
            {
                if( !m_pMatricesWeight[node]->GetClampValue() )
                {
                    notClWeightsNumbers.push_back(node);
                    notclLinesize += sz;
                    for(int i = 0; i < sz; i++, offset++)
                    {
                        notClPrtsData.push_back(m_pLearnMatrixMean->
                            GetElementByOffset(offset));
                    }
                    
                }
                else
                {
                    ClWeightsNumbers.push_back(node);
                    clLinesize += sz;
                    for(int i = 0; i < sz; i++, offset++)
                    {
                        clPrtsData.push_back(m_pLearnMatrixMean->GetElementByOffset(offset));
                    }
                }
            }
        }
        
        C2DNumericDenseMatrix<float> *matXX;
        C2DNumericDenseMatrix<float> *matYY;
        C2DNumericDenseMatrix<float> *matXY;
        C2DNumericDenseMatrix<float> *matYX;
        
        if( meanIsNotClamp || notClWeightsNumbers.size() )
        {
            
            m_pLearnMatrixCov->GetBlocks( &notClWeightsNumbers.front(), 
                notClWeightsNumbers.size(), &blockSizes.front(),m_NumberOfNodes, &matXX,
                &matYY, &matXY, &matYX );
            
            C2DNumericDenseMatrix<float> *notClExpPrts;
            
            sizes[0] = notclLinesize + meanIsNotClamp;
            sizes[1] = notclLinesize + meanIsNotClamp;
            length = sizes[0]*sizes[1];
            data.assign(length, 0.0f);                 
            notClExpPrts = C2DNumericDenseMatrix<float>::Create(sizes, &data.front());
            
            
            
            for( i = 0; i < notclLinesize; i++ )
            {
                indexes[0] = i;
                for( j = 0; j < notclLinesize; j++ )
                {
                    indexes[1] = j;
                    value = matXX->GetElementByIndexes( indexes );
                    notClExpPrts->SetElementByIndexes( value, indexes );
                }
                if( meanIsNotClamp )
                {
                    indexes[1] = j;
                    notClExpPrts->SetElementByIndexes( notClPrtsData[i], indexes );
		    indexes[0] = j;
		    indexes[1] = i;
		    notClExpPrts->SetElementByIndexes( notClPrtsData[i], indexes );
                }
                
            }
            if( meanIsNotClamp )
            {       
                notClExpPrts->SetElementByOffset( float(numEvidences), length - 1);
            }
            
            data.clear();
            
            for( i = clLinesize; i < clLinesize + chldSize; i++ )
            {
                indexes[0] = i;
                for( j = 0; j < notclLinesize ; j++ )
                {
                    indexes[1] = j;
                    value = matYX->GetElementByIndexes(indexes);
                    data.push_back(value);
                }
                
                if( meanIsNotClamp )
                {
                    value = m_pLearnMatrixMean->
                        GetElementByOffset(notclLinesize + i );
                    data.push_back(value);
                }   
            }
            /*
            if( meanIsNotClamp )
            {
            value = m_pLearnMatrixMean->
            GetElementByOffset(notclLinesize + i );
            data.push_back(value);
        }  */
            
            C2DNumericDenseMatrix<float>* combMeanPrts;
            int thisSizes[] = {chldSize, notclLinesize + meanIsNotClamp};
            combMeanPrts = C2DNumericDenseMatrix<float>::Create( thisSizes, &data.front());
            
            int bigSz = 0;
            for( node = 0; node < ClWeightsNumbers.size(); node++ )
            {
                
                sizes[0] = m_NodeTypes[ClWeightsNumbers[node]]->GetNodeSize();
                
                data.clear();
                for( i = 0; i < sizes[0]; i++ )
                {
                    for( j = 0; j < sizes[1] - meanIsNotClamp; j++ )
                    {
                        int indexes[] = {i+bigSz, j+bigSz};
                        value = matYX->GetElementByIndexes(indexes);
                        data.push_back(value);
                    }
                    if(meanIsNotClamp)
                    {
                        data.push_back(clPrtsData[bigSz+i]);
                    }
                }
                
                bigSz += sizes[1];
                C2DNumericDenseMatrix<float> *combClChdsChds;
                combClChdsChds = C2DNumericDenseMatrix<float>::Create( sizes, &data.front());
                C2DNumericDenseMatrix<float> *prodBlock = 
                    pnlMultiply(m_pMatricesWeight[ClWeightsNumbers[node]], combClChdsChds, flagMaximize);
                delete combClChdsChds;
                combMeanPrts->CombineInSelf(prodBlock, 0);
            }
            if(!meanIsNotClamp)
            {
                sizes[1] = notClPrtsData.size();
                sizes[0] = 1;
                C2DNumericDenseMatrix<float>* expPrts = C2DNumericDenseMatrix<float>::Create(sizes, &notClPrtsData.front());
                C2DNumericDenseMatrix<float>* clMeanExpPrts = pnlMultiply(m_pMatrixMean, expPrts, flagMaximize);
                delete expPrts;
                combMeanPrts->CombineInSelf(clMeanExpPrts, 0);
                delete clMeanExpPrts;
            }
            C2DNumericDenseMatrix<float>* invNotClExpPrts = notClExpPrts->Inverse();
            delete notClExpPrts;
            notClExpPrts = NULL;
            C2DNumericDenseMatrix<float>* combWeights = pnlMultiply(combMeanPrts,invNotClExpPrts, flagMaximize);
            delete invNotClExpPrts;
            delete combMeanPrts;
            offset = 0;
            for ( j = 0; j < chldSize; j++)
            {
                indexes[0] = j;
                for( node = 0; node < notClWeightsNumbers.size(); node++ )
                {
                    for( i = 0; i < blockSizes[notClWeightsNumbers[node]]; i++)
                    {
                        indexes[1] = i;
                        value = combWeights->GetElementByOffset(offset);
                        m_pMatricesWeight[notClWeightsNumbers[node]]
                            ->SetElementByIndexes(value, indexes);
                        ++offset;
                    }
                }
                if(meanIsNotClamp)
                {
                    value = combWeights->GetElementByOffset(offset);
                    m_pMatrixMean->SetElementByOffset(value, j);
                    ++offset;
                }
            }
            delete combWeights;
            
            delete matXX;
            delete matYY;
            delete matXY;
            delete matYX;
        }
        /////////////////////////////////////////////////////////
        
        int childe[] = {m_NumberOfNodes - 1};
        m_pLearnMatrixCov-> GetBlocks( childe, 1, 
            &blockSizes.front(),m_NumberOfNodes,
            &matYY, &matXX, &matYX, &matXY );
        
        delete matXX;
        C2DNumericDenseMatrix<float> *combAllWeights;
        int numOfDims;
        const int* ranges;
        matYX->GetRanges(&numOfDims, &ranges);
	
        data.resize(ranges[0]*ranges[1]);
        offset = 0;
        for ( j = 0; j < chldSize; j++)
        {
            indexes[0] = j;
            for( node = 0; node < m_NumberOfNodes-1; node++ )
            {
                for( i = 0; i < blockSizes[node]; i++)
                {
                    indexes[1] = i;
                    data[offset++] = m_pMatricesWeight[node]
                        ->GetElementByIndexes(indexes);
                    
                }
            }
            
        }
        combAllWeights = C2DNumericDenseMatrix<float>::Create(ranges, &data.front());
        C2DNumericDenseMatrix<float>* productBXY = pnlMultiply(combAllWeights, matXY, flagMaximize);
        delete matXY;
        matXY = NULL;
	
        delete combAllWeights;
        
        const float* learnData;
        m_pLearnMatrixMean->GetRawData(&length, &learnData);
        
        sizes[0] = 1;
        sizes[1] = chldSize;
        C2DNumericDenseMatrix<float>* stForMean = 
            C2DNumericDenseMatrix<float>::Create(sizes, &learnData[clLinesize + notclLinesize]);
        
        C2DNumericDenseMatrix<float>* muX = pnlMultiply(m_pMatrixMean, stForMean, flagMaximize);
        delete stForMean;
        
        productBXY->CombineInSelf(muX);
        delete muX;
        
        matYY->CombineInSelf(productBXY, 0);
        delete productBXY;
        delete matYX;
        
        float trace = 0.0f;
        if( m_pMatrixCov->GetClampValue() == 0)
        {
            for( i = 0 ; i < chldSize; i++ )
            {
                for(j = i; j < chldSize; j++)
                {
                    indexes[0] = i;
                    indexes[1] = j;
                    value = matYY->GetElementByIndexes(indexes)/numEvidences;
                    m_pMatrixCov->SetElementByIndexes(value, indexes);

		    indexes[0] = j;
		    indexes[1] = i;
		    m_pMatrixCov->SetElementByIndexes(value, indexes);
                }
            }
	    int isIllMatrix = m_pMatrixCov->IsIllConditioned();
            if( isIllMatrix )
            {
                m_bDeltaFunction = 1;
		static_cast<CMatrix<float>*>(
		    m_pMatrixCov)->Release(this);
		m_pMatrixCov = NULL;                
            }
            float det = m_pMatrixCov->Determinant();
            if(det < 0)
	    {
		m_bDeltaFunction = 1;
		static_cast<CMatrix<float>*>(
		    m_pMatrixCov)->Release(this);
		m_pMatrixCov = NULL;
	    }
	    
            trace = float(numEvidences*chldSize);
        }
        else
        {  
            C2DNumericDenseMatrix<float>* invCov = m_pMatrixCov->Inverse();
            C2DNumericDenseMatrix<float>* exp = pnlMultiply(matYY, invCov, flagMaximize);
            delete invCov;
            
            for( i = 0; i < chldSize*chldSize; i += chldSize+1)
            {
                trace += exp->GetElementByOffset(i);
            }
            delete exp;
	    
        }
        delete matYY;
	
        if( !m_bDeltaFunction)
	{
	    float abs_det =float(fabs( m_pMatrixCov->Determinant() ));
	    float log_abs_Cov = 
		float( ( abs_det < FLT_MIN ) ? -FLT_MAX : log(abs_det) );
	    log_abs_Cov += chldSize*float(log(2*PNL_PI));
	    
	    m_critVal = -(numEvidences*log_abs_Cov + trace)/2.0f;
	}
        
        
    }// end if m_NumberOfNodes
    else
    {
        
        if(meanIsNotClamp)
        {
            indexes[1] = 0;
            for( indexes[0] = 0; indexes[0] < chldSize; indexes[0]++)
            {
                value = m_pLearnMatrixMean->GetElementByOffset(indexes[0])/numEvidences;
                m_pMatrixMean->SetElementByIndexes(value, indexes);
                
            }
        }
        
        C2DNumericDenseMatrix<float>* pVirtMatrixCov;
        if(m_pMatrixCov && m_pMatrixCov->GetClampValue() == 0  )
        {
            pVirtMatrixCov = m_pMatrixCov;
            
        }
        else
        {
            pVirtMatrixCov = C2DNumericDenseMatrix<float>::Copy(m_pLearnMatrixCov);
        }
        C2DNumericDenseMatrix<float>* matrixMeanTr = m_pMatrixMean->Transpose();
        C2DNumericDenseMatrix<float>* meanSquare = pnlMultiply( m_pMatrixMean, matrixMeanTr, flagMaximize );
	
        if(meanIsNotClamp)
        {
            for( i = 0 ; i < chldSize; i++ )
            {
                
                for(j = i; j < chldSize; j++)
                {
                    indexes[0] = i;
                    indexes[1] = j;
                    value = m_pLearnMatrixCov->GetElementByIndexes(indexes)/numEvidences;
                    value -= meanSquare->GetElementByIndexes(indexes);
                    pVirtMatrixCov->SetElementByIndexes(value, indexes);
                    
		    indexes[0] = j;
		    indexes[1] = i;
                    pVirtMatrixCov->SetElementByIndexes(value, indexes);
                    
                }
            }
            
        }
        else
        {
            C2DNumericDenseMatrix<float> *prod = pnlMultiply(m_pLearnMatrixMean, matrixMeanTr, flagMaximize);
            
            for( i = 0 ; i < chldSize; i++ )
            {
		
                for(j = i; j < chldSize; j++)
                { 
                    indexes[0] = i;
                    indexes[1] = j;
                    value = m_pLearnMatrixCov->GetElementByIndexes(indexes);
                    value -= prod->GetElementByIndexes(indexes);
 
		    indexes[0] = j;
		    indexes[1] = i;
                    value -= prod->GetElementByIndexes(indexes);
                    value /= numEvidences;
                    value += meanSquare->GetElementByIndexes(indexes);
                    pVirtMatrixCov->SetElementByIndexes(value, indexes);

		    indexes[0] = i;
		    indexes[1] = j;
                    pVirtMatrixCov->SetElementByIndexes(value, indexes);
                    
                }
            }
            delete prod;
            
        }
        delete matrixMeanTr;
        delete meanSquare;
	
        float trace = 0.0f;
        if(m_pMatrixCov->GetClampValue() == 0)
        {
	    int isIllMatrix = m_pMatrixCov->IsIllConditioned();
            if( isIllMatrix )
            {
                m_bDeltaFunction = 1;
		static_cast<CMatrix<float>*>(
		    m_pMatrixCov)->Release(this);
		m_pMatrixCov = NULL;
            }
            float det = m_pMatrixCov->Determinant();
            if( det < 0 )
	    {
		m_bDeltaFunction = 1;
		static_cast<CMatrix<float>*>(
		    m_pMatrixCov)->Release(this);
		m_pMatrixCov = NULL;
	    }
	    trace = float(numEvidences*chldSize);
            
        }
        else
        {
            C2DNumericDenseMatrix<float>* invMatrixCov = m_pMatrixCov->Inverse();
            C2DNumericDenseMatrix<float>* exp = pnlMultiply(pVirtMatrixCov, invMatrixCov, flagMaximize);
            delete pVirtMatrixCov;
            delete invMatrixCov;
            for( i = 0; i < chldSize*chldSize; i += chldSize+1)
            {
                trace += exp->GetElementByOffset(i);
            }
            delete exp;
        }
        
        if( ! m_bDeltaFunction)
	{
	    float abs_det =float(fabs( m_pMatrixCov->Determinant() ));
	    float log_abs_Cov = 
		float( ( abs_det < FLT_MIN ) ? -FLT_MAX : log(abs_det) );
	    log_abs_Cov += chldSize*float(log(2*PNL_PI));
	    
	    m_critVal = -(numEvidences*log_abs_Cov + trace)/2.0f;
	}
        
        
    }
    
    
    
    return m_critVal;  
    
}

#endif
CDistribFun *CGaussianDistribFun::ConvertCPDDistribFunToPot()const
{
    if( m_bPotential )
    {
        return CGaussianDistribFun::Copy( this );
    }
    if( m_bUnitFunctionDistribution )
    {
	return new CGaussianDistribFun(m_NumberOfNodes, &m_NodeTypes.front(), 1, 1);
    }
    if(!m_bMoment)
    {
	PNL_THROW( CInvalidOperation, "we have no valid form of CPD" )
    }
    
    CGaussianDistribFun* factData = NULL;
    int NumAllInweights = 0;
    int i,j;
    int childSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
    
    for( i = 0; i < m_NumberOfNodes - 1; i++ )
    {
	NumAllInweights += m_NodeTypes[i]->GetNodeSize();
    }
    // if the node have no parents - we just convert the data to the canonical form & use it
    if( NumAllInweights == 0 )
    {
	factData = CGaussianDistribFun::Copy( this );
	factData->m_bPotential = 1;
	factData->UpdateCanonicalForm();
	return factData;
    }
    floatVector bigWeightsData = floatVector( NumAllInweights*childSize);
    floatVector nullData = floatVector( NumAllInweights*childSize, 0 );
    int offset = 0;
    int copySize = 0;
    int dims; const int *rangs;
    const floatVector *weitsVec = NULL;
    for( i = 0; i < m_NumberOfNodes -1; i++ )
    {
	weitsVec = m_pMatricesWeight[i]->GetVector();
	m_pMatricesWeight[i]->GetRanges( &dims, &rangs );
	copySize = rangs[1];
	//we assume every matrix has her own node size at the second dimension
	for( j = 0; j < childSize; j++ )
	{
	    memcpy( &bigWeightsData[offset+j*NumAllInweights], &(*weitsVec)[j*copySize],
		copySize*sizeof(float) );
	}
	offset +=copySize;
    }
    intVector ranges = intVector(2);
    ranges[0] = childSize;
    ranges[1] = NumAllInweights;
    C2DNumericDenseMatrix<float> *matBigWeights = 
        C2DNumericDenseMatrix<float>::Create( &ranges.front(), &bigWeightsData.front() );
    C2DNumericDenseMatrix<float> *matBigZeros = 
        C2DNumericDenseMatrix<float>::Create( &ranges.front(), &nullData.front() );
    C2DNumericDenseMatrix<float> *matZerosTransp = matBigZeros->Transpose();
    C2DNumericDenseMatrix<float> *matMinusWeights = 
        static_cast<C2DNumericDenseMatrix<float>*>(
	pnlCombineNumericMatrices( matBigZeros, matBigWeights, 0 ));
    C2DNumericDenseMatrix<float> *matWeightsTransp = matBigWeights->Transpose();
    C2DNumericDenseMatrix<float> *matMinusWeightsTransp = 
        static_cast<C2DNumericDenseMatrix<float>*>(
	pnlCombineNumericMatrices( matZerosTransp, matWeightsTransp, 0 ));
    C2DNumericDenseMatrix<float> *matInvCov = m_pMatrixCov->Inverse();
    C2DNumericDenseMatrix<float> *matInvCovTransp = matInvCov->Transpose();
    int flagMaximize = 0;
    //create matrices to combine it in all H
    C2DNumericDenseMatrix<float> *hSelf = pnlMultiply( matInvCov,
	m_pMatrixMean, flagMaximize );
    C2DNumericDenseMatrix<float> *hAdd = pnlMultiply( 
	matMinusWeightsTransp, hSelf, flagMaximize );
    //create matrices to combine it in all K
    C2DNumericDenseMatrix<float> *KAdd1 = pnlMultiply( matInvCov,
	matMinusWeights, flagMaximize );
    C2DNumericDenseMatrix<float> *Kadd2 = pnlMultiply( 
	matMinusWeightsTransp, matInvCovTransp, flagMaximize );
    C2DNumericDenseMatrix<float> *Kadd3 = pnlMultiply(
	matMinusWeightsTransp, KAdd1, flagMaximize );
    //combine all in H in order - child after parents
    floatVector allHVec = floatVector( NumAllInweights+childSize );
    const floatVector *hvecSelf = hSelf->GetVector();
    const floatVector *hvecAdd = hAdd->GetVector();
    offset = 0;
    copySize = hvecAdd->size();
    memcpy( &allHVec.front(), &hvecAdd->front(), copySize*sizeof(float) );
    offset +=copySize;
    copySize = hvecSelf->size();
    memcpy( &allHVec[offset], &hvecSelf->front(), copySize*sizeof(float) );
    ranges[0] = NumAllInweights+childSize;
    ranges[1] = 1;
    floatVector kvecAll = floatVector((NumAllInweights+childSize)*(NumAllInweights +childSize ));
    const floatVector *kvecAdd1 = KAdd1->GetVector();
    const floatVector *KvecAdd2 = Kadd2->GetVector();
    const floatVector *kParents = Kadd3->GetVector();
    const floatVector *kSelf = matInvCov->GetVector();
    //copy data for parents - add matrix to the left top corner
    offset = 0;
    for( i = 0; i < NumAllInweights; i++ )
    {
	memcpy( &kvecAll[i*(NumAllInweights+childSize)],
	    &(*kParents)[i*NumAllInweights],
	    NumAllInweights*sizeof(float) );
    }
    //add matrix in right top corner
    offset = NumAllInweights;
    for( i = 0; i < NumAllInweights; i++ )
    {
	memcpy( &kvecAll[i*(NumAllInweights+childSize)+offset],
	    &(*KvecAdd2)[i*childSize],
	    childSize*sizeof(float) );
    }
    //add matrix to the left bottom corner
    offset = (NumAllInweights+childSize)*NumAllInweights;
    for( i = 0; i < childSize; i++ )
    {
	memcpy( &kvecAll[offset+i*(NumAllInweights+childSize)],
	    &(*kvecAdd1)[i*NumAllInweights],
	    NumAllInweights*sizeof(float) );
    }
    //add matrix to the right bottom corner
    offset += NumAllInweights;
    for( i = 0; i < childSize; i++ )
    {
	memcpy( &kvecAll[offset+i*(NumAllInweights+childSize)],
	    &(*kSelf)[i*childSize],
	    childSize*sizeof(float) );
    }
    ranges[1] = NumAllInweights+childSize;
    //now we can compute g;
    float g = -m_numberOfDims*0.5f*(float)log(2*PNL_PI) 
	-0.5f*(float)log( m_pMatrixCov->Determinant());
    C2DNumericDenseMatrix<float> *matMeantr = m_pMatrixMean->Transpose();
    C2DNumericDenseMatrix<float> *matTemp1 = pnlMultiply( matMeantr,
	hSelf, flagMaximize );
    const floatVector *vecTemp = matTemp1->GetVector();
    if( vecTemp->size() != 1 )
    {
	PNL_THROW( CInvalidOperation, "the result should be scalar" );
    }
    g += -0.5f*(*vecTemp)[0];
    factData = CGaussianDistribFun::CreateInCanonicalForm( m_NumberOfNodes, &m_NodeTypes.front(),
	&allHVec.front(), &kvecAll.front(), g );
    delete matTemp1;
    delete matMeantr;
    delete Kadd3;
    delete Kadd2;
    delete KAdd1;
    delete hAdd;
    delete hSelf;
    delete matInvCov;
    delete matInvCovTransp;
    delete matMinusWeightsTransp;
    delete matWeightsTransp;
    delete matMinusWeights;
    delete matZerosTransp;
    delete matBigZeros;
    delete matBigWeights;
    
    return factData;
}

CDistribFun* CGaussianDistribFun::CPD_to_pi(CDistribFun *const*allPiMessages, 
					    int *multParentIndices, int numMultNodes, int posOfExceptParent, 
					    int maximizeFlag )const 
{
    if( m_bPotential )
    {
        PNL_THROW( CInvalidOperation, 
            "only CPD can be converted to lambda message" );
    }
    int i;
    //some parents are in nodeIndices
    if( numMultNodes > m_NumberOfNodes - 1 )
    {
        PNL_THROW( CInconsistentSize, "num nodes must be less than number of parents" )
    }
    if(( posOfExceptParent >=0 )&&( numMultNodes+2 != m_NumberOfNodes ))
    {
        PNL_THROW( CInconsistentSize, "numNodes should connect" )
    }
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, 
	    "unit function haven't valid moment form and so can't convert to pi which is in moment form" );
    }
    intVector nonUnitFunction;
    for( i = 0; i < numMultNodes; i++ )
    {
        if(allPiMessages[multParentIndices[i]]->GetDistributionType() != dtGaussian )
        {
            PNL_THROW( CInvalidOperation, 
                "discrete pi messages can't produce Gaussian message to parent" )
        }
        else 
        {
            if( !static_cast<CGaussianDistribFun*>(allPiMessages[
                multParentIndices[i]])->m_bUnitFunctionDistribution )
            {
                nonUnitFunction.push_back(multParentIndices[i]);
            }
        }
        //fixme - we need to check Moment form for parent messages
    }
    int numNonInif = nonUnitFunction.size();
    CGaussianDistribFun *resData = CGaussianDistribFun::CreateInMomentForm( 1,
        1, &m_NodeTypes[m_NumberOfNodes-1], NULL, NULL, NULL );
    
#ifndef PAR_OMP
    C2DNumericDenseMatrix<float> *matrixMean = m_pMatrixMean;
    C2DNumericDenseMatrix<float> *matrixCov = m_pMatrixCov;
#else
    C2DNumericDenseMatrix<float> *matrixMean = 
        static_cast<C2DNumericDenseMatrix<float>*>(m_pMatrixMean->Clone());
    C2DNumericDenseMatrix<float> *matrixCov = 
        static_cast<C2DNumericDenseMatrix<float>*>(m_pMatrixCov->Clone());
#endif
    
    matrixMean->AddRef(resData);
    matrixCov->AddRef(resData);
    C2DNumericDenseMatrix<float> *prodMat1 = NULL;
    C2DNumericDenseMatrix<float> *prodMat2 = NULL;
    int flagNumComb = 1;
    int flagMaximize = 0;
    for( i = 0; i < numNonInif; i++ )
    {
        //compute covariance matrix
        if( !((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->m_bDeltaFunction )
        {
            //we needn't to multiply by matrix of zeros!
            prodMat1 = m_pMatricesWeight[nonUnitFunction[i]]->Transpose();
            prodMat2 = pnlMultiply(
                ((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->m_pMatrixCov,
                prodMat1, flagMaximize );
            delete prodMat1;
            prodMat1 = pnlMultiply( m_pMatricesWeight[nonUnitFunction[i]],
		prodMat2, flagMaximize );
            delete prodMat2;
            prodMat2 = static_cast<C2DNumericDenseMatrix<float>*>(
                pnlCombineNumericMatrices( matrixCov, prodMat1, flagNumComb ));
            delete prodMat1;
            matrixCov->Release(resData);
            matrixCov = prodMat2;
            matrixCov->AddRef(resData);
        }
        //compute mean matrix
        prodMat1 = pnlMultiply( m_pMatricesWeight[nonUnitFunction[i]], 
            ((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->
            m_pMatrixMean, flagMaximize );
        prodMat2 = static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( matrixMean, prodMat1, flagNumComb ));
        delete prodMat1;
        matrixMean->Release(resData);
        matrixMean = prodMat2;
        matrixMean->AddRef(resData);
    }
    //both matricae are in reference list now
    resData->m_pMatrixMean = matrixMean;
    resData->m_pMatrixCov = matrixCov;
    int numDims1; const int *ranges1;
    int numDims2; const int *ranges2;
    matrixMean->GetRanges( &numDims1, &ranges1 );
    matrixCov->GetRanges( &numDims2, &ranges2 );
    if(( ranges1[0] != ranges2[0] )||( ranges2[0] != ranges2[1] ))
    {
        PNL_THROW( CInconsistentSize, "mean and covarience" );
    }
    float sumMatCov = matrixCov->SumAll(1);
    if( sumMatCov < 0.000001f )
    {
        resData->m_bDeltaFunction = 1;
        resData->m_normCoeff = 0.0f;
    }
    else
    {
        float p1 = (float)pow((double)(2*PNL_PI),(m_numberOfDims/2.0));
        float p2 = (float)pow((double)matrixCov->Determinant(),0.5);
        resData->m_normCoeff = 1/(p1*p2);
    }
    resData->m_bMoment = 1;
    
    
    return resData;    
}

CDistribFun* CGaussianDistribFun::CPD_to_lambda(const CDistribFun *lambda, 
						CDistribFun *const* allPiMessages, int *multParentIndices, 
						int numNodes, int posOfExceptNode, int maximizeFlag )const
{
    if( m_bPotential )
    {
        PNL_THROW( CInvalidOperation, 
            "only CPD can be converted to lambda message" );
    }
    int i;
    //some parents are in nodeNumbers
    if( numNodes > m_NumberOfNodes - 2 )
    {
        PNL_THROW( CInconsistentSize, 
            "numNodes is number of parents except one" )
    }
    if( lambda->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInvalidOperation,
            "discrete lambda can't produce Gaussian message to parent" )
    }
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, 
	    "unit function haven't valid moment form and so can't convert to lambda which is in moment form" );
    }
    const CGaussianDistribFun *lam = static_cast<const CGaussianDistribFun*>(lambda);
    intVector nonUnitFunction;
    for( i = 0; i < numNodes; i++ )
    {
        if(allPiMessages[multParentIndices[i]]->GetDistributionType() != dtGaussian )
        {
            PNL_THROW( CInvalidOperation, 
                "discrete pi messages can't produce Gaussian message to parent" )
        }
        else 
        {
            if( !static_cast<CGaussianDistribFun*>(allPiMessages[
                multParentIndices[i]])->m_bUnitFunctionDistribution )
            {
                nonUnitFunction.push_back(multParentIndices[i]);
            }
        }
        //fixme - we need to check Moment form for parent messages
    }
    if( lam->m_bUnitFunctionDistribution )
    {
        CGaussianDistribFun *resData = 
            CGaussianDistribFun::CreateUnitFunctionDistribution( 1, 
            &m_NodeTypes[posOfExceptNode], 1 );
        return resData;
    }
    float g = 0.0f;
    CGaussianDistribFun *resData = CGaussianDistribFun::CreateInCanonicalForm(
        1, &(m_NodeTypes[posOfExceptNode]), NULL, NULL, g );
    int numNonInif = nonUnitFunction.size();
    C2DNumericDenseMatrix<float> *matrixK = NULL;
    C2DNumericDenseMatrix<float> *matrixH = NULL;
    //we need to compute the same product as in CPD_to_pi - we can do the same without creation new data

#ifndef PAR_OMP
    C2DNumericDenseMatrix<float> *matMeanPi = m_pMatrixMean;
    C2DNumericDenseMatrix<float> *matCovPi = m_pMatrixCov;
#else
    C2DNumericDenseMatrix<float> *matMeanPi = 
        static_cast<C2DNumericDenseMatrix<float>*>(m_pMatrixMean->Clone());
    C2DNumericDenseMatrix<float> *matCovPi = 
        static_cast<C2DNumericDenseMatrix<float>*>(m_pMatrixCov->Clone());
#endif

    matMeanPi->AddRef(resData);
    matCovPi->AddRef(resData);
    C2DNumericDenseMatrix<float> *prodMat1 = NULL;
    C2DNumericDenseMatrix<float> *prodMat2 = NULL;
    int flagNumComb = 1;
    int flagMaximize = 0;
    for( i = 0; i < numNonInif; i++ )
    {
        //compute covariance matrix
        if( !((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->m_bDeltaFunction )
        {
            //we needn't to multiply by matrix of zeros!
            prodMat1 = m_pMatricesWeight[nonUnitFunction[i]]->Transpose();
            prodMat2 = pnlMultiply(
                ((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->
                m_pMatrixCov, prodMat1, flagMaximize);
            delete prodMat1;
            prodMat1 = pnlMultiply( 
                m_pMatricesWeight[nonUnitFunction[i]], prodMat2, flagMaximize );
            delete prodMat2;
            prodMat2 = static_cast<C2DNumericDenseMatrix<float>*>(
                pnlCombineNumericMatrices( matCovPi, prodMat1, flagNumComb ));
            delete prodMat1;
            matCovPi->Release(resData);
            matCovPi = prodMat2;
            matCovPi->AddRef(resData);
        }
        //compute mean matrix
        prodMat1 = pnlMultiply( m_pMatricesWeight[nonUnitFunction[i]], 
            ((CGaussianDistribFun*)allPiMessages[nonUnitFunction[i]])->
            m_pMatrixMean, flagMaximize );
        prodMat2 = static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( matMeanPi, prodMat1, flagNumComb ));
        delete prodMat1;
        matMeanPi->Release(resData);
        matMeanPi = prodMat2;
        matMeanPi->AddRef(resData);
    }
    //we use different forms for singular lambda covariance or not
    //we can't use it without specifying of lambda - delta function
    C2DNumericDenseMatrix<float> *matC = NULL;
    C2DNumericDenseMatrix<float> *CovLam = NULL;
    // we need to manipulate with special cases
    if( lam->m_bDeltaFunction )
    {
        int dim = lam->m_numberOfDims;
        floatVector zerosCov = floatVector( dim*dim,  0.0f );
        intVector dims = intVector(2, dim);
        CovLam = C2DNumericDenseMatrix<float>::Create( &dims.front(), &zerosCov.front() );
    }
    else
    {
        CovLam = lam->m_pMatrixK->Inverse();
    }
    if( !CovLam )
    {
        C2DNumericDenseMatrix<float> *matA = NULL;
        prodMat2 = matCovPi->Inverse();
        prodMat1 = static_cast<C2DNumericDenseMatrix<float>*>(pnlCombineNumericMatrices( 
            lam->m_pMatrixK, prodMat2, 1 ));
        delete prodMat2;
        matA = prodMat1->Inverse();
        delete prodMat1;
        prodMat1 = pnlMultiply( lam->m_pMatrixK , matA, flagMaximize );
        prodMat2 = pnlMultiply( prodMat1, lam->m_pMatrixK, flagMaximize );
        matC = static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( lam->m_pMatrixK,    prodMat2, 0 ));
        C2DNumericDenseMatrix<float> *identity = C2DNumericDenseMatrix<float>
            ::CreateIdentityMatrix( m_NodeTypes[m_NumberOfNodes - 1]->
            GetNodeSize() );
        delete prodMat2;
        C2DNumericDenseMatrix<float> *matD = 
            static_cast<C2DNumericDenseMatrix<float>*>(pnlCombineNumericMatrices(
            identity, prodMat1, flagNumComb ));
        delete prodMat1;
        prodMat1 = pnlMultiply( lam->m_pMatrixK, matMeanPi, flagMaximize );
        prodMat2 = pnlMultiply( matD, prodMat1, flagMaximize );
        delete prodMat1;
        prodMat1 = pnlMultiply( matD, lam->m_pMatrixH, flagMaximize );
        delete matD;
        matD = static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( prodMat2, prodMat1, 0 ));
        delete prodMat1;
        delete prodMat2;
        prodMat1 = m_pMatricesWeight[posOfExceptNode]->Transpose();
        matrixH = pnlMultiply( prodMat1, matD, flagMaximize );
        delete prodMat1;
        delete matD;
    }
    else
    {
        prodMat1 = static_cast<C2DNumericDenseMatrix<float>*>(
            pnlCombineNumericMatrices( matCovPi, CovLam, 1 ));
        matC = prodMat1->Inverse();
        delete prodMat1;
        if( lam->m_bDeltaFunction )
        {
            prodMat1 = static_cast<C2DNumericDenseMatrix<float>*>(
                pnlCombineNumericMatrices( lam->m_pMatrixMean, matMeanPi, 0 ));
            prodMat2 = pnlMultiply( matC, prodMat1, flagMaximize );
            delete prodMat1;
            prodMat1 = m_pMatricesWeight[posOfExceptNode]->Transpose();
            matrixH = pnlMultiply( prodMat1, prodMat2, flagMaximize );
            delete prodMat2;
            delete prodMat1;
        }
        else
        {
            prodMat1 = pnlMultiply( CovLam, lam->m_pMatrixH, flagMaximize );
            prodMat2 = static_cast<C2DNumericDenseMatrix<float>*>(
                pnlCombineNumericMatrices( prodMat1, matMeanPi,0 ));
            delete prodMat1;
            prodMat1 = pnlMultiply( matC, prodMat2, flagMaximize );
            delete prodMat2;
            prodMat2 = m_pMatricesWeight[posOfExceptNode]->Transpose();
            matrixH = pnlMultiply( prodMat2, prodMat1, flagMaximize );
            delete prodMat2;
            delete prodMat1;
        }
    }
    prodMat1 = pnlMultiply( matC, m_pMatricesWeight[posOfExceptNode], flagMaximize );
    prodMat2 = m_pMatricesWeight[posOfExceptNode]->Transpose();
    matrixK = pnlMultiply( prodMat2, prodMat1, flagMaximize );
    delete matC;
    delete CovLam;
    delete prodMat1;
    delete prodMat2;
    matMeanPi->Release(resData);
    matCovPi->Release(resData);
    resData->AttachMatrix(matrixK, matK);
    resData->AttachMatrix(matrixH, matH);
    resData->SetCoefficient( g, 1 );
    return resData;
}

int CGaussianDistribFun::GetNumberOfFreeParameters() const
{
  	 CMatrix<float>* pMatrix;
    int i, nDims, nMean, nCov, nWeights;
    int dimOfGaussian, nparents;
    const int* Dims; 
    dimOfGaussian = 0;
    nCov = 0;

    if (!m_bDeltaFunction)
    {
        pMatrix = this->GetMatrix(matCovariance);
        pMatrix->GetRanges(&nDims, &Dims);
        dimOfGaussian = Dims[0];
        if( pMatrix->GetClampValue() )
            nCov = 0;
        else
        {
            nCov = dimOfGaussian * (dimOfGaussian-1) / 2;   //symmetric (and positive definite)
        }
    }


    pMatrix = this->GetMatrix(matMean);
    if( pMatrix->GetClampValue() )
        nMean = 0;
    else
    {
        nMean = dimOfGaussian;
    }
    nparents = GetNumberOfNodes()-1;

    nWeights = 0;
    if (!m_bDeltaFunction)
    {
        if(m_pMatricesWeight)
        {
            for(i=0; i</*nDims-1*/nparents; i++)
            {
                pMatrix = GetMatrix(matWeights, i);
                if( !pMatrix->GetClampValue() )
                {
                    pMatrix->GetRanges(&nDims, &Dims);
                    nWeights += Dims[0] * Dims[1];
                }
            }
        }
    }
    return nMean + nCov + nWeights;
}


/*

  void CGaussianDistribFun::GenerateSample(pValueVector &obsVls, intVector &vlsSz ) const 
  {
  int nDims;
  const int* ranges;
  m_pMatricesWeight[0]->GetRanges(&nDims, &ranges);
  
    floatVector resData(ranges[0], 0.0f);
    
      int length;
      const float* weightData;
      m_pMatricesWeight[0]->GetRawData(&length, &weightData);
      int i;
      for( i = 0; i < length; i++)
      {
      resData[j] += weightData[i]*obsVls[i][j].GetFlt();
      }
      
	
	  
	    }
*/
#if 1
float CGaussianDistribFun::ProcessingStatisticalData( float nEv ) 
{
    
    PNL_CHECK_LEFT_BORDER(nEv, 1);
    int nnodes = GetNumberOfNodes();
    int nParents = nnodes - 1;
    
    const pConstNodeTypeVector *nt = GetNodeTypesVector();
    
    const C2DNumericDenseMatrix<float> * pMatB;
    const C2DNumericDenseMatrix<float> *pMatSigma;
    const C2DNumericDenseMatrix<float> *pMatMu;
    C2DNumericDenseMatrix<float> *pMatMean;
    pMatSigma = static_cast<C2DNumericDenseMatrix<float> *>(GetStatisticalMatrix(stMatSigma));
    pMatMu = static_cast<C2DNumericDenseMatrix<float> *>(GetStatisticalMatrix(stMatMu));
    pMatMean = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matMean));
    
    bool bClMean = pMatMean->GetClampValue() ? true : false;
    
    
    intVector unclumpedDims;
    intVector clumpedDims;
    
    intVector nsVec( nnodes, 0);
    
    int i, j, ns;
    if( nParents )
    {
	intVector clPos;
	intVector unclPos;
	int nUnclEl = 0;
	for( i = 0; i < nParents; i++ )
	{
	    pMatB = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, i));
	    ns = (*nt)[i]->GetNodeSize();
	    nsVec[i] = ns;
	    if( !pMatB || !pMatB->GetClampValue() )
	    { 
		nUnclEl += ns;
		unclPos.push_back(i);
	    } 
	    else
	    {
		clPos.push_back(i);
	    }
	}
	nsVec.back() = nt->back()->GetNodeSize();
	//////////////////////////////////////////////////////////////////////////
	C2DNumericDenseMatrix<float> *pMatSum;
	int sizes[2];
	sizes[0] = nsVec.back();
	sizes[1] = nUnclEl;
	if(!bClMean)
	{
	    sizes[1] +=1;
	}
	if(!sizes[1])
	{
	    sizes[1] = 1;
	}
	pMatSum = static_cast<C2DNumericDenseMatrix<float>*>
	    (pMatMu->CreateEmptyMatrix( 2, sizes, 0, float(0.0f) ));
	
	intVector tmp(1, 0);
	
	C2DNumericDenseMatrix<float> *pMat1;
	C2DNumericDenseMatrix<float> *pMat2;
	if( clPos.size())
	{
	    
	    int i = 0;
	    pMatB = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, clumpedDims[i]));
	    
	    for( i = 0; i < clPos.size(); i++ )
	    {
		tmp[0] = clPos[i];
		if( !bClMean )
		{
		    pMat1 = GetBlock(tmp, unclPos, nsVec, pMatSigma, pMatMu);
		}
		else
		{
		    pMat1 = GetBlock(tmp, unclPos, nsVec, pMatSigma, NULL );
		    
		}
		pMat2 = pnlMultiply( pMatB, pMat1, 0);
		delete pMat1;
		
		pMatSum->CombineInSelf(pMat2, 0);
		delete pMat2;	  
	    } 
	    if( bClMean )
	    {
		pMat1 = FormMuX(  unclPos, nsVec );
		pMatSum->CombineInSelf(pMat1, 0);
		delete pMat1;
	    }
	    
	    
	} 
	tmp[0] = nnodes-1;
	
	
	if(!bClMean)
	{
		pMat1 = GetBlock(tmp, unclPos, nsVec, pMatSigma, pMatMu);
	}
	else
	{
		pMat1 = GetBlock(tmp, unclPos, nsVec, pMatSigma, NULL);

	}
	pMatSum->CombineInSelf(pMat1, 1);
	delete pMat1;
	
	//////////////////////////////////////////////////////////////////////////
	
	pMat1 =  FormXX( unclPos, nsVec, bClMean, nEv);
	pMat2 = pMat1->Inverse();
	delete pMat1;
	
	pMat1 = pnlMultiply( pMatSum, pMat2, 0);
	delete pMat2;
	delete pMatSum;
	//////////////////////////////////////////////////////////////////////////
	
	int mulind[2];
	int offsetB = 0;
	const floatVector *res = pMat1->GetVector();
	floatVector resB;
	for( i = 0; i < unclPos.size(); i++ )
	{
	    
	    pMat2 = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, unclPos[i]));
	    int nEl = pMat2->GetRawDataLength();
	    resB.resize(nEl);
	    
	    mulind[1] = offsetB;
	    for( j = 0; j < nsVec.back(); j++ )
	    {
		mulind[0] = j;
		memcpy( &resB[j*nsVec[unclPos[i]]], &(*res)[pMat1->ConvertMultiDimIndex(mulind)], 
		    nsVec[unclPos[i]]*sizeof(float));
	    }
	    offsetB += nsVec[unclPos[i]]; 
	    pMat2->SetData(&resB.front());
	}
	if( !bClMean )
	{  	    
	    mulind[1] = offsetB;
	    float val;
	    for( i = 0; i < nsVec.back(); i++ )
	    {
		mulind[0] = i;
		val = pMat1->GetElementByIndexes(mulind);		
		pMatMean->SetElementByOffset(val, i);
	    }
	}
	delete pMat1;
    }
    else
    {
        nsVec.back() =  nt->back()->GetNodeSize();
        if( !bClMean )
        {
	    float val;
	    for( i = 0; i < nsVec.back(); i++ )
	    {
	        val= pMatMu->GetElementByOffset(i)/nEv;
	        pMatMean->SetElementByOffset( val, i);
	    }
	    
	    
        }
    }
    //////////////////////////////////////////////////////////////////////////
    C2DNumericDenseMatrix<float> *matCovVirt = FormCov( nsVec,  nEv );
    C2DNumericDenseMatrix<float> *pMatCov = NULL;
    if( !m_bDeltaFunction )
    {
        pMatCov = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matCovariance));
    }
    int isIllCond = matCovVirt->IsIllConditioned();
    if( matCovVirt->Determinant() <= FLT_MIN )
    {
        isIllCond = 1;
    }
    float val1;
    if( pMatCov )
    {
        if( !pMatCov->GetClampValue() )
        {
	    if( !isIllCond )
	    {
	        pMatCov->SetDataFromOtherMatrix(matCovVirt);
                m_normCoeff = 0.0f;
                CheckMomentFormValidity();
	    }
	    else
	    {
	        m_bDeltaFunction = 1;
	        static_cast<CMatrix<float>*>(
		    pMatCov)->Release(this);
	        m_pMatrixCov = NULL;
                m_normCoeff = 0.0f;
	    }
            delete matCovVirt;	    
            val1 = float(nsVec.back()*nEv);
        }
        else
        {
	    C2DNumericDenseMatrix<float> *invCov = pMatCov->Inverse();
	    C2DNumericDenseMatrix<float> *tmp = pnlMultiply(matCovVirt, invCov, 0);
	    delete invCov;
	    delete matCovVirt;
	    val1 = tmp->Trace()*nEv;  
	    delete tmp;
        }
    }
    else
    {
        if( !isIllCond )
        {
	    m_bDeltaFunction = 0;
            m_normCoeff = 0;
            AttachMatrix(matCovVirt, matCovariance );
	    CheckMomentFormValidity();
        }
       else
       {
           delete matCovVirt;
       }
       val1 = float(nsVec.back()*nEv);
    }
    if( !m_bDeltaFunction )
    {
        pMatCov = static_cast<C2DNumericDenseMatrix<float> *>(
            GetMatrix(matCovariance));
        double ll = -0.5f*(nEv*(nsVec.back()*log(2.f*PNL_PI) + 
            log( fabs( pMatCov->Determinant() ) ) ) + val1);
        return (float)ll;
    }
    else
    {
        return -FLT_MAX;
    }

}

void CGaussianDistribFun::SetFreedomDegrees( int forMean, int forCov )
{
    if( forMean <= 0 )
    {
        PNL_THROW( CBadConst,
        "The degree of freedom for mean dustribution must be positive" );
    }
    if ( forCov <= m_numberOfDims+1 )
    {
        PNL_THROW( CBadConst,
        "The degree of freedom for Wishart dustribution are incorrect" );
    }
    m_freedomDegreeMean = forMean;
    m_freedomDegreeCov = forCov;
}

void CGaussianDistribFun::BayesUpdateFactor(const CEvidence* const* pEvidences,
        int EvidenceNumber, const int *domain)
{
    if( !pEvidences )
    {
        PNL_THROW( CNULLPointer, "evidences" )//no Evidences - NULL pointer
    }
    if( (!m_pPseudoCountsMean) || (!m_pPseudoCountsCov) ||
        (m_freedomDegreeCov < 0) || (m_freedomDegreeMean < 0))
    {
        PNL_THROW( CNULLPointer, "Prior distribution was not specified" )
    }
    floatVector tempMean;
    tempMean.assign( m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize(), 0 );
    int j;
    // calculating sample sum
    int k;
    for ( k = 0; k < m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize(); ++k)
    {
        float sum = 0;
        for( j = 0; j < EvidenceNumber; ++j)
        {
            if( !pEvidences[j] )
            {
                PNL_THROW( CNULLPointer, "evidence" );//no Evidence - NULL pointer
            }
            sum += ((pEvidences[j]->GetValue(m_NumberOfNodes - 1))[k]).GetFlt();
        };
        tempMean[k] = sum;
    }
    // update precesion matrix
    int index[2];
    index[0] = 0; // first index
    index[1] = 0; // second index
    for ( j = 0; j < m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize(); ++j)
    {
        for( k = 0; k < m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize(); ++k)
        {
            index[0] = k;
            index[1] = j;
            float elem = m_pPseudoCountsCov->GetElementByIndexes(index);
            int p;
            for ( p = 0; p < EvidenceNumber; ++p)
            {
                elem += (((pEvidences[p]->GetValue(m_NumberOfNodes-1))[k]).GetFlt()-
                    tempMean[j]/EvidenceNumber)*(((pEvidences[p]->
                    GetValue(m_NumberOfNodes-1))[j]).GetFlt()-tempMean[k]/EvidenceNumber);
            }
            int ind1[2] = { index[0], 0 };
            int ind2[2] = { index[1], 0 };
            elem += m_freedomDegreeMean*EvidenceNumber*
                (m_pPseudoCountsMean->GetElementByIndexes(ind1)-tempMean[index[0]]/EvidenceNumber)*
                (m_pPseudoCountsMean->GetElementByIndexes(ind2)-tempMean[index[1]]/EvidenceNumber)/
                (m_freedomDegreeMean + EvidenceNumber);
            m_pPseudoCountsCov->SetElementByIndexes(elem, index);
        }
    }
    // update mean vector
    int indexes[2];
    indexes[0]=0;
    indexes[1]=0;
        for ( k = 0; k < m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize(); ++k)
        {
            indexes[0]=k;
            float elem = m_pPseudoCountsMean->GetElementByIndexes( indexes );
            elem = (elem*m_freedomDegreeMean+tempMean[k])/(m_freedomDegreeMean + EvidenceNumber);
            m_pPseudoCountsMean->SetElementByIndexes( elem, indexes );
        }
}


void CGaussianDistribFun::PriorToCPD( floatVecVector &parentPrior )
{
    if( (!m_pPseudoCountsMean) || (!m_pPseudoCountsCov) ||
        (m_freedomDegreeCov < 0) || (m_freedomDegreeMean < 0))
    {
        PNL_THROW( CNULLPointer, "Prior distribution was not specified" )
    }
    int i;
    int index[2];
    index[0] = 0;
    index[1] = 0;
    // update mean matrix
    for( i = 0; i < m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize(); i++, index[0]++)
    {
        // float elem = m_pMatrixMean->GetElementByIndexes(index); ???
        float elem = m_pPseudoCountsMean->GetElementByIndexes(index);
        int j;
        for( j = 0; j < m_NumberOfNodes - 1; ++j)
        {
            C2DNumericDenseMatrix<float> *matW = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, j));
            int length = 0;
            const float *output; 
            matW->GetRawData(&length, &output);
            if( length != m_NodeTypes[m_NumberOfNodes-1]->GetNodeSize()*parentPrior[j].size())
            {
                PNL_THROW(COutOfRange, "incorrect sizes");
            }
            int k;
            float weigth = 0;
            for( k = 0; k < parentPrior[j].size(); ++k)
            {
                weigth += output[i*parentPrior[j].size()+k]*parentPrior[j][k];
            }
            elem -= weigth;
        }
        m_pMatrixMean->SetElementByIndexes(elem, index);
    }
    // update covariance matrix
//    m_pMatrixCov->SetDataFromOtherMatrix((CMatrix<float>*)(m_pPseudoCountsCov));
    int indexes[2] = { 0, 0 };
    float elem = m_pPseudoCountsCov->GetElementByIndexes(indexes);
    elem = 1/(2*(m_freedomDegreeCov - 2)*elem);
    m_pMatrixCov->SetElementByIndexes(elem, indexes);
}


#endif
C2DNumericDenseMatrix<float> * CGaussianDistribFun::GetBlock( intVector &ind1, intVector &ind2,
							     intVector &ns,
							     const C2DNumericDenseMatrix<float> *pMat,
							     const C2DNumericDenseMatrix<float> *pMat1)
{
    
    
    intVector offsets(ns.size(), 0);
    int offset = 0;
    int i;
    for( i = 1; i < ns.size(); i++ )
    {	
	offset += ns[i-1];
	offsets[i] = offset;
    }
    
    
    int sz1 = 0;
    int sz2 = 0;
    for( i = 0; i < ind1.size(); i++ )
    {
	sz1 += ns[ind1[i]];
    }
    for( i = 0; i < ind2.size(); i++ )
    {
	sz2 += ns[ind2[i]];
    }
    
    if( pMat1 )
    {
	++sz2;
    }
    
    floatVector data( sz1*sz2 );
    
    int length;
    const float * raw;
    pMat->GetRawData(&length, &raw);
    
    int indexes[2];
    offset = 0;
    int j, k;
    for( i = 0; i < ind1.size(); i ++ )
    { 
	int start = offsets[ind1[i]];
	int end = start + ns[ind1[i]];
	for( k = start; k < end ; k++ )
	{
	    indexes[0] = k;
	    for( j = 0; j < ind2.size(); j ++ )
	    {	
		indexes[1] = offsets[ind2[j]];
		int step = pMat->ConvertMultiDimIndex( indexes );
		int nEl = ns[ind2[j]];
		memcpy( &data[offset], &raw[step], nEl*sizeof(float) );
		offset += nEl;
		
	    }
	    if( pMat1 )
	    {
		int x[] = {k, 0};
		data[offset] = pMat1->GetElementByIndexes(x);
		offset++;
	    }
	}
    }	
    indexes[0] = sz1;
    indexes[1] = sz2;
    
    return C2DNumericDenseMatrix<float>::Create( indexes, &data.front() );  
    
}

C2DNumericDenseMatrix<float> * CGaussianDistribFun::FormMuX(  intVector& unclumpedDims, intVector& nsVec )
{
    const C2DNumericDenseMatrix<float> *pMean = 
	static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matMean));
    const C2DNumericDenseMatrix<float> *pMu = 
	static_cast<C2DNumericDenseMatrix<float> *>(GetStatisticalMatrix(stMatMu));
    C2DNumericDenseMatrix<float> *matX;
    C2DNumericDenseMatrix<float> *matY;
    
    pMu->GetLinearBlocks(&unclumpedDims.front(), unclumpedDims.size(), 
	&nsVec.front(), nsVec.size(), 
	&matX, &matY);
    delete matY;
    matY = matX->Transpose();
    delete matX;
    matX = pnlMultiply( pMean, matY, 0);
    delete matY;
    return matX; 
}

C2DNumericDenseMatrix<float> * CGaussianDistribFun::FormXX( intVector& unclumpedDims, intVector& nsVec, 
							   bool bMeanCl, float nEv)
{
    if( bMeanCl)
    {
	return GetBlock(unclumpedDims, unclumpedDims, nsVec, NULL );
    }
    
    intVector blocks( unclumpedDims.begin(), unclumpedDims.end() );
    blocks.push_back( nsVec.size() - 1 );
    
    intVector nsTmp( nsVec.begin(), nsVec.end() );
    nsTmp.back() = 1;
    C2DNumericDenseMatrix<float> *matXX = GetBlock(blocks, blocks, nsTmp, 
	(static_cast<C2DNumericDenseMatrix<float>*>(GetStatisticalMatrix(stMatSigma))), NULL );
    C2DNumericDenseMatrix<float> *matX;
    C2DNumericDenseMatrix<float> *matY;
    
    (static_cast<C2DNumericDenseMatrix<float>*>(GetStatisticalMatrix(stMatMu)))->
	GetLinearBlocks(&unclumpedDims.front(), unclumpedDims.size(), 
	&nsVec.front(), nsVec.size(), 
	&matX, &matY);
    delete matY;
    
    int nDims;
    const int * ranges;
    matXX->GetRanges(&nDims, &ranges);
    int ind[2];
    int i;
    for( i = 0; i < ranges[0] - 1; i++ )
    {
	float val = matX->GetElementByOffset(i);
	
	ind[0] = ranges[0] - 1;
	ind[1] = i;
	
	matXX->SetElementByIndexes( val, ind );
        
	ind[0] = i;
	ind[1] = ranges[0] - 1;
	matXX->SetElementByIndexes( val, ind );
    }
    
    ind[0] = ranges[0] - 1;
    ind[1] = ranges[0] - 1;
    matXX->SetElementByIndexes( (float)nEv, ind );
    delete matX;
    
    return matXX;
    
}

C2DNumericDenseMatrix<float> * CGaussianDistribFun::FormCov(intVector& nsVec, float nEv )
{
    
    int nnodes = this->GetNumberOfNodes();
    C2DNumericDenseMatrix<float>* matMu = 
	static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matMean));
    C2DNumericDenseMatrix<float> *matMuT = matMu->Transpose();
    C2DNumericDenseMatrix<float> *matS = 
	static_cast<C2DNumericDenseMatrix<float> *>(GetStatisticalMatrix(stMatSigma));
    C2DNumericDenseMatrix<float> *matStMu = 
	static_cast<C2DNumericDenseMatrix<float> *>(GetStatisticalMatrix(stMatMu));
    C2DNumericDenseMatrix<float> *matMuMu = pnlMultiply( matMu, matMuT, 0);
    C2DNumericDenseMatrix<float> *tmp;
    C2DNumericDenseMatrix<float> *tmp1;
    C2DNumericDenseMatrix<float> *tmp2;
    int i;

    if( nnodes == 1 && !GetMatrix(matMean)->GetClampValue())
    {
	C2DNumericDenseMatrix<float> *matCov = 
	    static_cast<C2DNumericDenseMatrix<float> *>(matS->Clone()); 

	float val;
	const pnlVector<float>*  dat = matCov->GetVector();
	for( i = 0; i < dat->size(); i++ )
	{
	    val = (*dat)[i]/nEv;
	    matCov->SetElementByOffset(val, i);
	}
	
	matCov->CombineInSelf(matMuMu, 0);
	delete matMuT;
	delete matMuMu;
	return matCov;
	
    }

    intVector ind1( 1, nnodes - 1);
    intVector ind2( 1, nnodes - 1 );
    
    //y*y^t
    C2DNumericDenseMatrix<float> *matYY = GetBlock(ind1, ind2, nsVec, matS );
    C2DNumericDenseMatrix<float> *matY;
    matStMu->GetLinearBlocks( &ind1.front(), ind1.size(), &nsVec.front(), nsVec.size(), &matY, &tmp );
    delete tmp;
    
    
    tmp = pnlMultiply(matY, matMuT, 0);
    delete matY;
    
    //y*y^ - y*mu^t
    matYY->CombineInSelf(tmp, 0);
    
    tmp1 = tmp->Transpose();
    
    //y*y^T - y*mu^t - mu*y^t
    matYY->CombineInSelf(tmp1, 0);
    delete tmp1;
    delete tmp;
    
    
    
    //y*y^T - y*mu^t - mu*y^t - y*xt*Bt - B*x*yt
    C2DNumericDenseMatrix<float> *matW;
    ind1[0] = nnodes - 1;
    for( i = 0; i < nnodes - 1; i++ )
    {
	ind2[0] = i;
	matW = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, i));
	tmp = GetBlock(ind2, ind1, nsVec, matS, NULL );
	tmp1 = pnlMultiply(matW, tmp, 0);
	delete tmp;
	
	matYY->CombineInSelf(tmp1, 0);
	tmp = tmp1->Transpose();
	delete tmp1;
	
	matYY->CombineInSelf(tmp, 0);
	delete tmp;
	
	C2DNumericDenseMatrix<float> *vecXi;
	matStMu->GetLinearBlocks( &ind2.front(), 1, &nsVec.front(), nsVec.size(), &vecXi, &tmp );
	delete tmp;
	
	
	tmp = pnlMultiply(matW, vecXi, 0);
	tmp1 = pnlMultiply(tmp, matMuT, 0);
	delete tmp;
	delete vecXi;
	
	matYY->CombineInSelf(tmp1, 1);
	
	tmp = tmp1->Transpose();
	delete tmp1;
	
	matYY->CombineInSelf(tmp, 1);	
	delete tmp;
	
    }
    
    int j;
    for( i = 0; i < nnodes - 1; i++ )
    {
	for( j = i; j < nnodes - 1; j++ )
	{
	    ind1[0] = i;
	    ind2[0] = j;
	    
	    tmp = GetBlock(ind1, ind2, nsVec, matS );
	    
	    matW = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, i));
	    
	    tmp1 = pnlMultiply(matW, tmp, 0 );
	    delete tmp;
	    
	    matW = static_cast<C2DNumericDenseMatrix<float> *>(GetMatrix(matWeights, j));
	    tmp = matW->Transpose();
	    
	    tmp2 = pnlMultiply( tmp1, tmp, 0 );
	    delete tmp;
	    delete tmp1;
	    
	    matYY->CombineInSelf(tmp2, 1);
	    
	    if( i != j )
	    {
		tmp = tmp2->Transpose();
		matYY->CombineInSelf(tmp, 1);
		delete tmp;
	    }
	    delete tmp2;
	    
	}
    }
    //1/nEv(y*y^T - y*mu^t - mu*y^t+...)
     
   /*
   const pnlVector<float>* valVec = matYY->GetVector();
   for( i = 0; i < valVec->size(); i++ )
   {
   matYY->SetElementByOffset((*valVec)[i]/nEv, i);
   }
   //y*y^T - y*mu^t - mu*y^t+... + mu*mu^t
   matYY->CombineInSelf(matMuMu, 1);
    */
    int sz = nsVec.back();
    int ind[2];
    float val = 0.0f;
    float num = 0.5f/nEv;
    for( i = 0; i < sz; i++ )
    {
	for( j = i; j < sz; j++ )
	{
	    ind[0] = i;
	    ind[1] = j;
	    val = matYY->GetElementByIndexes(ind);

	    ind[0] = j;
	    ind[1] = i;
	    val += matYY->GetElementByIndexes(ind);
	    val *= num;
	    
	    val += matMuMu->GetElementByIndexes(ind);
	    
	    matYY->SetElementByIndexes(val, ind);
	    ind[0] = i;
	    ind[1] = j;
	    matYY->SetElementByIndexes(val, ind);
	}
	
    }
   
    delete matMuT;
    delete matMuMu;
    
    return matYY;
    
}

#ifdef PAR_PNL
void CGaussianDistribFun::UpdateStatisticsML(CDistribFun *pDF)
{
    if (pDF->GetDistributionType() != dtGaussian)
        PNL_THROW(CInconsistentType, 
        "Can not use function CGaussianDistribFun::UpdateStatisticsML with wrong distribution type");

    if( !m_pLearnMatrixMean || !m_pLearnMatrixCov)
    {
        int length = 0;    
        for (int node = 0; node < m_NumberOfNodes; node++)
        {
            length += ( m_NodeTypes[node]->GetNodeSize() );
        }
        float *data = new float [length];
        int *ranges = new int [2];
        ranges[0] = length;
        ranges[1] = 1;
        m_pLearnMatrixMean = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixMean->ClearData();
        delete []data;
        ranges[1] = length;
        data = new float [length*length];
        m_pLearnMatrixCov = C2DNumericDenseMatrix<float>::Create( ranges, data );
        m_pLearnMatrixCov->ClearData();
        delete []data;
        delete []ranges;
    }

    CGaussianDistribFun *pGDF = dynamic_cast <CGaussianDistribFun *>(pDF);
    C2DNumericDenseMatrix<float> *pLearnMatrixMean = 
        dynamic_cast<C2DNumericDenseMatrix<float>*>(pGDF->GetStatisticalMatrix(stMatMu));
    C2DNumericDenseMatrix<float> *pLearnMatrixCov = 
        dynamic_cast<C2DNumericDenseMatrix<float>*>(pGDF->GetStatisticalMatrix(stMatSigma));

    int NumDims;
    const int *ranges;

    m_pLearnMatrixCov->GetRanges(&NumDims, &ranges);

    int coordinate_i_size = ranges[0];
    int coordinate_j_size = ranges[1];

    int offset = 0;
    float value;

    for( int coordinate_i = 0; coordinate_i < coordinate_i_size; coordinate_i++)
    {
        value = ( m_pLearnMatrixMean->GetElementByOffset( coordinate_i ) ) 
            + pLearnMatrixMean->GetElementByOffset ( coordinate_i ) /** NSamples*/;
        m_pLearnMatrixMean -> SetElementByOffset(value, coordinate_i);

        for ( int coordinate_j = 0; coordinate_j < coordinate_j_size; coordinate_j++)
        {
            value = ( m_pLearnMatrixCov -> GetElementByOffset( offset ) )
                + pLearnMatrixCov -> GetElementByOffset( offset ) /** NSamples*/;
            m_pLearnMatrixCov -> SetElementByOffset( value, offset );
            offset++;
        }
    }
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CGaussianDistribFun::m_TypeInfo = CPNLType("CGaussianDistribFun", &(CDistribFun::m_TypeInfo));

#endif
