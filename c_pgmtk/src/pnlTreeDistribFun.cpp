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
//  Purpose:   CTreeDistribFun class member functions implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlEvidence.hpp"
#include "pnlException.hpp"
#include "pnlTreeDistribFun.hpp"

PNL_USING

using namespace std;

CTreeDistribFun::CTreeDistribFun(int NodeNumber, 
				 const CNodeType *const* NodeTypes,
				 const SCARTParams* params) : 
CDistribFun(dtTree, NodeNumber, NodeTypes)
{
    if (params)
    {
		m_Params = *params;
    }
    else
    {
		m_Params.is_cross_val = true;
		m_Params.cross_val_folds = 5;
		m_Params.learn_sample_part = 0.6f;
		m_Params.priors = NULL;
    }
    m_pCART = NULL;
    m_pSample = NULL;
    m_pOptimalData = NULL;
    m_response_mat = NULL;
    m_terms_mat	   = NULL;
    m_missed_mask  = NULL;
    m_domain = NULL;
}

CTreeDistribFun::~CTreeDistribFun()
{
    Clear();
}

void CTreeDistribFun::Clear()
{
    if (m_pCART)
    {
		cxReleaseCART(&m_pCART);
    }
    if (m_pSample)
    {
		CxClassifierSampleChunk* chunk = m_pSample->chunk[0];
		cxReleaseMat( &m_response_mat );
		if (m_terms_mat)
			cxReleaseMat( &m_terms_mat );
		if (m_missed_mask)
			cxReleaseMat( &m_missed_mask );
		
		delete[] chunk->indices_of_interest;
		cxReleaseClassifierSample( &m_pSample ); 
    }
}

CTreeDistribFun* CTreeDistribFun::Create( int NodeNumber, const CNodeType *const* NodeTypes, const SCARTParams* params )
{
    CTreeDistribFun* retDistr = new CTreeDistribFun(NodeNumber, NodeTypes, params);
    PNL_CHECK_IF_MEMORY_ALLOCATED( retDistr );
    return retDistr;
}

CTreeDistribFun* CTreeDistribFun::Copy( const CTreeDistribFun* pInpDistr )
{
    PNL_CHECK_IS_NULL_POINTER( pInpDistr );
    
    CTreeDistribFun *retDistr = new CTreeDistribFun( *pInpDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retDistr );
    
    return retDistr;
}

CDistribFun* CTreeDistribFun::Clone() const
{
    //	CTreeDistribFun* resData = CTreeDistribFun::Create(m_NumberOfNodes, m_NodeTypes.begin(), NULL, NULL, NULL);
    //	memcpy(resData, this, sizeof(CTreeDistribFun));
    return Copy(this);
    //	return resData;
}

CDistribFun* CTreeDistribFun::CloneWithSharedMatrices()
{
/* 	CTreeDistribFun* resData = CTreeDistribFun::Create(m_NumberOfNodes, m_NodeTypes.begin(), NULL, NULL, NULL);
memcpy(resData, this, sizeof(CTreeDistribFun));
    return resData;*/
    return Copy(this);
}

void CTreeDistribFun::CreateDefaultMatrices( int /*typeOfMatrices*/  )
{
    //do nothing
}


//copy constructor
CTreeDistribFun::CTreeDistribFun( const CTreeDistribFun & inpDistr )
:CDistribFun( dtTree )
{
    memcpy(this, &inpDistr, sizeof(*this));
    m_pCART = cxCopyCART(m_pCART);
}


CDistribFun& CTreeDistribFun::operator=( const CDistribFun& pInputDistr )
{
    if( this == &pInputDistr  )
    {
		return *this;
    }
    if( pInputDistr.GetDistributionType() != dtTree )
    {
		PNL_THROW( CInvalidOperation, 
			"input distribution must be Tree " );
    }
    //both distributions must be on the same node types
    const CTreeDistribFun &pGInputDistr = 
		static_cast<const CTreeDistribFun&>(pInputDistr);
    int isTheSame = 1;
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
	if( !isTheSame )
	{
		PNL_THROW(CInvalidOperation,
		"both distributions must be on the same nodes");
	}
	return *this;
}

static int* rand_perm(int n, int* ar = 0)
{
	if (!ar)
	{
		ar = new int[n];
		for (int i = 0; i < n ; i++)
			ar[i] = i;
	}
	srand((unsigned int)GetTicks());
	int* _ar = ar;
	for (int i = n; i ; _ar++, i--)
	{
		int pos = rand() % i;
		int tmp = _ar[pos];
		_ar[pos] = *_ar;
		*_ar = tmp;
	}
	return ar;
}


void CTreeDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences,
                                   int nEvidences,
				   const int *domain, 
                                   float weightingCoeff)
{
    UNREFERENCED_PARAMETER( weightingCoeff );
    if( !pEvidences )
    {
	PNL_THROW( CNULLPointer, "evidences" )//no Evidences - NULL pointer
    }
    m_vecAllEvidences.insert(m_vecAllEvidences.end(), pEvidences , pEvidences + nEvidences);
    m_domain = domain;
}

void CTreeDistribFun::_CreateCART()
{
    Clear();
    assert(m_domain);
    
    int nEvidences = m_vecAllEvidences.size();
    CxMat* priors_mat = m_Params.priors;
    int num_samples_learn = m_Params.is_cross_val ? nEvidences :
    (int)(nEvidences * m_Params.learn_sample_part);
    int num_samples_test = nEvidences - num_samples_learn;
    int* samples_of_interest = rand_perm( nEvidences );
    int total_vars = m_NodeTypes.size() - 1;
    
    m_response_mat  = cxCreateMat( 1, nEvidences, CX_32FC1 );
    CxMat* type_mask     = cxCreateMat( total_vars + 1, 1, CX_32SC1 );
    CxMat* features_of_interest = 0;
    if ( total_vars > 0)
    {
		m_terms_mat     = cxCreateMat( total_vars, nEvidences, CX_32FC1 );
		m_missed_mask   = cxCreateMat( total_vars, nEvidences, CX_8UC1 );
		features_of_interest = cxCreateMat( total_vars  , 1, CX_32SC1 );
		for (int  i = 0; i < total_vars; i++ )
			((int *)features_of_interest->data.ptr)[i] = i;
    }
    for( int evNumber = 0; evNumber < nEvidences; evNumber++ )
    {
		float* _terms = m_terms_mat ? (m_terms_mat->data.fl + evNumber) : NULL;
		char* _missed = m_missed_mask ? (((char*)m_missed_mask->data.ptr) + evNumber) : NULL;
		const CEvidence* pEvidence = m_vecAllEvidences[evNumber];
		for( int i = 0 ; i < total_vars ; i++, _terms += nEvidences, _missed += nEvidences)
		{
			assert(m_vecAllEvidences[evNumber]);
			*_terms = GetNodeValue(pEvidence, i);//m_NodeTypes[i]->IsDiscrete() ? (float)val->GetInt() : val->GetFlt();
			int node =  m_domain[i];
#ifdef _DEBUG
			assert(node >= 0);
			float f = m_NodeTypes[i]->IsDiscrete() ? 
				pEvidence->GetValue(node)->GetInt() : pEvidence->GetValue(node)->GetFlt();
			assert ( fabs (f - *_terms) < EPS);
#endif
			*_missed = !pEvidence->IsNodeObserved(node);
		}
		m_response_mat->data.fl[evNumber] = GetNodeValue(pEvidence, total_vars);
		//m_NodeTypes[total_vars]->IsDiscrete() ? (float)val->GetInt() : val->GetFlt();
#ifdef _DEBUG	
		int node = m_domain[total_vars];
		assert(node >= 0);
	float f = m_NodeTypes[total_vars]->IsDiscrete() ? 
	    pEvidence->GetValue(node)->GetInt() : pEvidence->GetValue(node)->GetFlt();
	assert ( fabs (f - m_response_mat->data.fl[evNumber]) < EPS);
#endif
    }
    int* tmask = (int *)type_mask->data.ptr;
    for (int i1 = 0; i1 < total_vars + 1; i1++)
    {
	//		 int node = m_domain[i1];
	//	 	 assert(node >= 0);
		
		const CNodeType* pNodeType = m_NodeTypes[i1];
		
		tmask[i1] = pNodeType->IsDiscrete() ?  
			CX_CLASSIFIER_VAR_TYPE_CATEGORIC_32F( pNodeType->GetNodeSize() ) :
		CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F;
    }
    
    m_pSample =	cxCreateClassifierSampleHeader( m_terms_mat, 0, m_response_mat, 
		type_mask, m_missed_mask,
		samples_of_interest, num_samples_learn );
    
    
    CxCARTTrainParams*  train_params = 
		cxCARTTrainParams( features_of_interest,   // features_of_interest 
		priors_mat,             // priors 
		CxCARTEntropyCriterion, //splitting rule
		0,    // num_competitors 
		0,    // competitor_threshold 
		5,    // num_surrogates 
		.2f,  // surrogate_threshold 
		12,   // tree_max_depth 
		5,    // split_max_points, 
		0,    // split_min_points, 
		10,0);// number of clusters
    
    
    m_pCART = cxBuildCART( m_pSample, train_params , NULL);
    assert(m_pCART && (m_pCART->error.code == 0));
    cxPruneCART(m_pCART);
    if ( total_vars > 0 )
    {
		if (m_Params.is_cross_val)
		{
			m_pOptimalData = cxFindBestSubtreeCrossValidation(m_pCART , m_pSample, m_Params.cross_val_folds);
		}
		else
		{
			m_pSample->chunk[0]->indices_of_interest = samples_of_interest + num_samples_learn; 
			m_pSample->chunk[0]->num_indices_of_interest = num_samples_test;
			m_pOptimalData = cxFindBestSubtree(m_pCART , m_pSample, TRUE);
			m_pSample->chunk[0]->indices_of_interest = samples_of_interest; 
			m_pSample->chunk[0]->num_indices_of_interest = nEvidences;
		}
    }
    else
		m_pOptimalData = icxGetPruningData(m_pCART, 0);
    
#ifdef _DEBUG
    FILE* file = fopen("d:\\tree.log", "wt");
    cxDumpCART(file,m_pCART);
    fprintf(file, "Optimal pruning step : %d\n" , m_pOptimalData->step);
    fclose(file);
#endif	
    if (type_mask)
	cxReleaseMat( &type_mask );
}

float CTreeDistribFun::ProcessingStatisticalData( float numEvidences ) 
{
    int nEvidences = int(numEvidences);
    assert( numEvidences == m_vecAllEvidences.size() );
    
    _CreateCART();
    
    CxClassifierSampleChunk* chunk = m_pSample->chunk[0];
    
    assert(nEvidences > 0);
    int fstp0 = chunk->features_step[0];	
    int fstp1 = chunk->features_step[1];	
    int mstp0 = chunk->missed_step[0];	
    int mstp1 = chunk->missed_step[1];	
    char* _features = ((char*)chunk->features_data);
    char* _missed_mask = chunk->missed_data ;
    int step = m_pOptimalData->step;
    double error = 0.0;
    if (IsRegression())
    {
		for (int j = 0 ; j < nEvidences ; j++, _features += fstp0, _missed_mask += mstp0)
		{
			CxCARTNode* node = m_pCART->root;
			assert(node);
			while (icxIsNodeSplit(node) && !icxIsNodePruned(node , step))
			{
				node = cxIsStringSplitLeft(m_pCART, node ,
					_features , fstp1 ,
					_missed_mask , mstp1) ? node->child_left : node->child_right;
			}
			float response = icxGetChunkResponse(chunk,j).fl;
			float diff = response - node->response.fl ;
			float dev = icxGetNodeVariance(m_pCART, node);
			error += 0.5 * diff * diff / (dev * dev);
			error += log(dev);	
		}
		m_fLogLik = (float)(- error - nEvidences * log(sqrt(2.0 * PNL_PI)));
    }
    else
    {
		for (int j = 0 ; j < nEvidences ; j++, _features += fstp0, _missed_mask += mstp0)
		{
			CxCARTNode* node = m_pCART->root;
			assert(node);
			while (icxIsNodeSplit(node) && !icxIsNodePruned(node , step))
			{
				node = cxIsStringSplitLeft(m_pCART, node ,
					_features , fstp1 ,
					_missed_mask , mstp1) ? node->child_left : node->child_right;
			}
			int response = icxVarToInt(icxGetChunkResponse(chunk,j));
			float prob = node->fallen_stats[response]/(float)node->num_fallens;
			error += log(prob);	
		}
		m_fLogLik = (float)error;
    }
    
    return m_fLogLik;  
}

float CTreeDistribFun::GetLogLik( const CEvidence* pEv ) const
{
    CxClassifierSampleChunk* chunk = m_pSample->chunk[0];
    const int* flags = pEv->GetObsNodesFlags();
    int nVars = m_NodeTypes.size()-1;
    
    if( std::find( flags, flags + nVars + 1, 0 ) != flags + nVars + 1 )
    {
        PNL_THROW( CNotImplemented, "all nodes must be observed" )
    }
    int fstp1 = chunk->features_step[1];	
    int mstp1 = chunk->missed_step[1];	
    
    float* features = new float[nVars];
    int step = m_pOptimalData->step;
    CxCARTNode* node = m_pCART->root;
    assert(node);
    
    float response = GetNodeValue(pEv, nVars);//pEv->GetValueBySerialNumber(nVars)->GetFlt();
    for (int i = 0; i < nVars ; i++)
    {
		features[i] = GetNodeValue(pEv, i);//pEv->GetValueBySerialNumber(i)->GetFlt();
    }
    
    while (icxIsNodeSplit(node) && !icxIsNodePruned(node , step))
    {
		node = cxIsStringSplitLeft(m_pCART, node ,
			features , fstp1 ,
			NULL , mstp1) ? node->child_left : node->child_right;
    }
    
    if (IsRegression())
    {
		float diff = response - node->response.fl ;
		float dev = icxGetNodeVariance(m_pCART, node);
		float error = 0.5f * diff * diff / (dev * dev) + (float)log(dev);
		delete[] features;
		return (float)(-error - log(sqrt(2.0 * PNL_PI)));
    }
    else
    {
		int resp = cxRound(response);
		float prob = node->fallen_stats[resp]/(float)node->num_fallens;
		delete[] features;
		return (float)log(prob);
    }
}

void  CTreeDistribFun::GenerateSample( CEvidence*, int ) const
{
    PNL_THROW( CNotImplemented, "this operation" );
}

int CTreeDistribFun::GetMultipliedDelta(const int **, const float **,
					 const int **) const
{
    PNL_THROW( CNotImplemented, "this operation" );
    return 0;
}


void CTreeDistribFun::AllocMatrix(const float *, EMatrixType, int,
                                   const int *) 
{
    PNL_THROW( CNotImplemented, "this operation" );
}

void CTreeDistribFun::AttachMatrix( CMatrix<float>* /* pMatrix */,
				    EMatrixType /* mType */,
				    int /* numberOfWeightMatrix */,
			            const int * /* parentIndices */,
                                    bool)
{	
    PNL_THROW( CNotImplemented, "this operation" );
}

bool CTreeDistribFun::IsValid(std::string* /*description*/) const
{
    return TRUE;
}

CNodeValues *CTreeDistribFun::GetMPE()
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

int CTreeDistribFun::IsEqual( const CDistribFun * dataToCompare,
        float /* epsilon */, int /* withCoeff */, float* maxDifference) const
{
    if( !dataToCompare )
    {
        PNL_THROW( CNULLPointer, "dataToCompare" );
        return 0;
    }
    if( dataToCompare->GetDistributionType() != dtTree )
    {
        PNL_THROW( CInconsistentType, 
            "we can compare datas of the same type" );
        return 0;
    }
    if( maxDifference )
    {
        *maxDifference = 0.0f;
    }
    const CTreeDistribFun *data = (const CTreeDistribFun*)dataToCompare;
    if( (!IsValid())||( !data->IsValid() ) )
    {
        PNL_THROW( CInvalidOperation, "we can't compare invalid data" )
    }
    //we need to check node types
    if( m_NumberOfNodes != data->m_NumberOfNodes )
    {
        return 0;
    }
    int ret = (m_pCART == data->m_pCART);
    
    return ret;
}


void CTreeDistribFun::MultiplyInSelfData(const int * /* pBigDomain */, 
					  const int * /* pSmallDomain */,
					  const CDistribFun * /* pOtherData */)
{
    PNL_THROW( CNotImplemented, "this opeartion" );
}

void CTreeDistribFun::DivideInSelfData(const int * /* pBigDomain */, 
					const int * /* pSmallDomain */,
					const CDistribFun * /* pOtherData */) 
{
   	PNL_THROW( CNotImplemented, "this opeartion" );
}


CDistribFun* CTreeDistribFun::GetNormalized() const 
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

void CTreeDistribFun::Normalize()
{
    PNL_THROW( CNotImplemented, "normalization opeartion" );
}

CMatrix<float>*
CTreeDistribFun::GetMatrix(EMatrixType /* mType */, 
			    int /* numWeightMat */,
			    const int * /* parentIndices */) const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}


CMatrix<float>*
CTreeDistribFun::GetStatisticalMatrix(EStatisticalMatrix /* mType */,
				 int * /* parentIndices */) const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

CDistribFun* CTreeDistribFun::ConvertToSparse() const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

CDistribFun* CTreeDistribFun::ConvertToDense() const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

void CTreeDistribFun::Dump() const
{
    
}

int CTreeDistribFun::IsSparse() const 
{
    return 0;
}
int CTreeDistribFun::IsDense() const 
{
    return 1;
}

void CTreeDistribFun::MarginalizeData(const CDistribFun * /* pOldData */,
				       const int * /* VarsOfKeep */,
				       int /* NumVarsOfKeep */, int /* maximize */)
{
    PNL_THROW( CNotImplemented, "this opeartion" );
}

void
CTreeDistribFun::ShrinkObservedNodes(const CDistribFun* /* pOldData */, 
				      const int * /* pVarsObserved */, 
				      const Value* const* /* pObsValues */,
				      int /* numObsVars */,
				      const CNodeType* /* pObsTabNT */,
				      const CNodeType* /* pObsGauNT */) 
{
    PNL_THROW( CNotImplemented, "this opeartion" );
}

void CTreeDistribFun::ExpandData(const int* /* pDimsToExtend */, 
				  int   /* numDimsToExpand */,
				  const Value* const* /* valuesArray */, 
				  const CNodeType* const * /* allFullNodeTypes */,
				  int   /* UpdateCanonical */)
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    
}
void CTreeDistribFun::ClearStatisticalData()
{
    m_vecAllEvidences.clear();
}



void CTreeDistribFun::UpdateStatisticsEM(const CDistribFun* /* infData */,
                                   const CEvidence * /* pEvidence */, 
                                   float /* weightingCoeff */,
                                   const int* /* domain */) 
{
    PNL_THROW( CNotImplemented, "this opeartion" );
}

CDistribFun *CTreeDistribFun::ConvertCPDDistribFunToPot() const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

CDistribFun*
CTreeDistribFun::CPD_to_pi(CDistribFun *const* /* allPiMessages */, 
							int * /* multParentIndices */,
							int /* numMultNodes */,
							int /* posOfExceptParent */, 
							int /* maximizeFlag */) const 
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;	
}

CDistribFun*
CTreeDistribFun::CPD_to_lambda(const CDistribFun * /* lambda */,
								CDistribFun *const* /* allPiMessages */,
								int * /* multParentIndices */, 
								int /* numNodes */,
								int /* posOfExceptNode */,
								int /* maximizeFlag */) const
{
    PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

void CTreeDistribFun::SetStatistics( const CMatrix<float> * /*pMat*/, 
	EStatisticalMatrix /*matrix*/, const int* /*parentsComb*/ )
{
    PNL_THROW( CNotImplemented, "this opeartion" );

}

int CTreeDistribFun::GetNumberOfFreeParameters() const
{
    return m_pCART->num_nodes;	
}
