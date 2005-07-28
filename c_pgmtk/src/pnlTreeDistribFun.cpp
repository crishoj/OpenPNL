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
//  Purpose:   CTreeDistribFun class member functions implementation       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlEvidence.hpp"
#include "pnlException.hpp"
#include "pnlTreeDistribFun.hpp"
#include "pnlRng.hpp" 

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
    FILE* file = fopen("c:\\tree.log", "wt");
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

void  CTreeDistribFun::GenerateSample( CEvidence* evidence, int maximize) const
{
     CxCARTNode* node = m_pCART->root;
    assert(node);
    CxClassifierVar var;
    while (icxIsNodeSplit(node))
    {
        if (evidence->GetValue(node->split->feature_idx)->IsDiscrete())
        {
            var.i = evidence->GetValue(node->split->feature_idx)->GetInt();
            var.fl = (float)var.i;
        }
        else 
        {
            var.fl = evidence->GetValue(node->split->feature_idx)->GetFlt();
        }
        node = icxIsVarSplitLeft(m_pCART, node->split , var)
                ? node->child_left : node->child_right;
    }
    if (IsRegression())
    {
    float response = node->response.fl;
    float dev = node->error;
    doubleVector doubleMean;
    doubleMean.push_back(response);
    doubleVector doubleCov;
    doubleCov.push_back(dev);
    doubleVector rndVls;
    pnlRandNormal( &rndVls, doubleMean, doubleCov );
    evidence->GetValue(m_domain[m_NodeTypes.size() - 1])->SetFlt((float)rndVls[0]);
    }
    else
    {
    int rnd = pnlRand( 0, 10000000);
    int i=0;
    int sum=node->fallen_stats[0];
    while (rnd > sum)
    {
        ++i;
        sum+=node->fallen_stats[i];
    }
    evidence->GetValue(m_domain[m_NodeTypes.size() - 1])->SetInt(i);
    }
    int arr[]={ m_domain[m_NodeTypes.size() - 1] };
    evidence->ToggleNodeState(1, arr);
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


void CTreeDistribFun::SumInSelfData(const int * /*pBigDomain*/,
  const int * /*pSmallDomain*/, const CDistribFun * /*pOtherData*/)
{
  PNL_THROW(CNotImplemented, "haven't for CTreeDistribFun now");
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

#ifdef PAR_PNL
void CTreeDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CTreeDistribFun not implemented yet");
}
#endif // PAR_OMP

void CTreeDistribFun::UpdateTree(const CGraph* pGraphTree, TreeNodeFields *fields )
{
    
    if( pGraphTree->IsBinaryTree())
    {
        //creating tree that contains only one node  
        int type_step = 0;
        int total_vars = m_NodeTypes.size() - 1;
        int num_features = 0;
        int response_type;
        int j;
        int i;
        for( i = 0; i< pGraphTree->GetNumberOfNodes(); i++)
        {
            if( fields[i].isTerminal == false) num_features++;
        }
            
        int s = sizeof( CxClassifierVarType ) * num_features;
        CxClassifierVarType *feature_type;
        feature_type = ( CxClassifierVarType*)malloc( s );
        memset(feature_type, 0, s);
        CxMat* type_mask     = cxCreateMat(total_vars+1, 1, CX_32SC1 );
        type_step = (type_mask->rows == 1) ? sizeof(int) : type_mask->step;
        response_type = *(CxClassifierVarType *)(type_mask->data.ptr + type_step * num_features);
        
        CxMat* features_of_interest = cxCreateMat(1, num_features, CX_32SC1 );
        
        int tmp = 0; 
        for( i = 0; i< pGraphTree->GetNumberOfNodes(); i++)
        {
            if( fields[i].isTerminal == false)
            {   
                if ( fields[i].Question == 1)
                    ((int *)features_of_interest->data.ptr)[tmp] = 1;
                else
                    if(fields[i].Question == 0)
                        ((int *)features_of_interest->data.ptr)[tmp] = 0;
                    else
                        PNL_THROW( CInvalidOperation, 
                        "we can compute only two types of questions now " );
                    tmp++;
            };
        };
        
        CxMat* priors_mat = m_Params.priors;
        //creation params
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
        //end of creation params
        response_type = 1; 
        m_pCART = cxCreateCART(num_features,feature_type,
        response_type, train_params, NULL);
        //end of creation tree contains one root node
        
        // finding root node number in the tree
        
        int rootNode;
        for(i = 0; i < pGraphTree->GetNumberOfNodes(); i++)
        {
            intVector NodeParents;
            pGraphTree->GetParents(i,&NodeParents);
            if( NodeParents.size() == 0 )
            {
                rootNode = i;  
                break;
            };
        };
        //end of finding root node number in the tree
                
        //filling feature type array
        m_pCART->feature_type = new int[pGraphTree->GetNumberOfNodes()];
        for( i = 0; i< pGraphTree->GetNumberOfNodes(); i++)
        {
            m_pCART->feature_type[i] = 1;
        };
        for(i = 0; i < pGraphTree->GetNumberOfNodes(); i++ )
        {
            if( !(fields[i].isTerminal) )
                if ( fields[i].Question == 1)
                    m_pCART->feature_type[fields[i].node_index] = 24576;
                else
                    if(fields[i].Question == 0)
                        m_pCART->feature_type[fields[i].node_index] = 2;
                    else
                        PNL_THROW( CInvalidOperation, 
                        "we can compute only two types of questions now " );
        };
        //end of filling feature type array
        cxInitNodeStorage(m_pCART);
        cxSetNodeStorageDepth(m_pCART, total_vars + 1, m_pCART->params->tree_max_depth);
        int currClass = 0;
        //filling number of classes array
        m_pCART->num_classes = new int[num_features]; 
        for( i = 0; i < pGraphTree->GetNumberOfNodes(); i++)
        {
            if( fields[i].isTerminal == false)
            {
                for(j = 0; j < total_vars; j++)
                {
                    if(m_domain[j] == fields[i].node_index) 
                    {
                        if(m_NodeTypes[j]->IsDiscrete())  
                            m_pCART->num_classes[currClass] = m_NodeTypes[j]->GetNodeSize();
                        else   m_pCART->num_classes[currClass]  = 1;
                        
                    currClass++;
                    }
                }
                
            }
        };
        int *arr = new int[pGraphTree->GetNumberOfNodes()];
        for( i = 0; i < pGraphTree->GetNumberOfNodes(); i++)
        {
            if( fields[i].isTerminal == false)
            {
                arr[i] = fields[i].node_index;
            }
            else 
            {
                arr[i] = 0;
            }
        };
        int max = arr[0];
        for( i = 1; i < pGraphTree->GetNumberOfNodes(); i++)
            if (arr[i] > max)
                max = arr[i];
        m_pCART->features_corr = new int[max+1];
        for( i = 0; i < max+1; i++)
            m_pCART->features_corr[i] = 0;

        //end of filling number of classes array
        
        // growing tree
        CxCARTNode* root = cxAllocNode(m_pCART, 1 , 0);
        m_pCART->root = root;
        m_pCART->num_nodes = 1;
        ConvertGraphToTree(m_pCART,m_pCART->root,pGraphTree,rootNode);
        //end of growing tree
        
        //filling tree
        FillTree(m_pCART,m_pCART->root,pGraphTree, fields,rootNode);
        // end of filling tree
		m_pCART->num_response_classes = m_NodeTypes[m_NodeTypes.size()-1]->GetNodeSize();
        m_pCART->root->num_fallens = 0;
    }
    else
    {
        PNL_THROW( CInvalidOperation, 
            "input graph must be Tree " );
    };
    
}

void CTreeDistribFun::ConvertGraphToTree(CxCART *pCart,CxCARTNode *node,
    const CGraph *pGraph, int nodeNum) const
{
    intVector CurrentNodeChildren;
    pGraph->GetChildren(nodeNum,&CurrentNodeChildren);
    if( CurrentNodeChildren.size() !=0)
    {
        //creating two child nodes
        CxCARTNode *left, *right;
        int id_left = node->id * 2;
        int id_right = id_left + 1 ;
        int new_depth = node->depth + 1 ;
        
        left = cxAllocNode(pCart , id_left , new_depth);
        right = cxAllocNode(pCart , id_right , new_depth);
        
        pCart->num_nodes += 2;
        node->child_left = left;
        node->child_right = right;
        left->parent = right->parent = node;
        
        ConvertGraphToTree(pCart,left,pGraph,CurrentNodeChildren[0]);
        ConvertGraphToTree(pCart,right,pGraph,CurrentNodeChildren[1]);  
        return;
    }
    else
    {
        return;
    }    
}

void CTreeDistribFun::FillTree(CxCART *pCart,CxCARTNode *node,
       const CGraph *pGraph, TreeNodeFields *fields, int nodeNum) const
{
       
    if( fields[nodeNum].isTerminal)
    {
        if(m_NodeTypes[m_NodeTypes.size() - 1]->IsDiscrete())
        {   
            int fallen_size;
            int i;
            fallen_size = m_NodeTypes[m_NodeTypes.size()-1]->GetNodeSize();
            int tmp_sum = 0;
            node->fallen_stats = new int [fallen_size];
            for (i = 0; i < fallen_size-1; i++)
            {
                node->fallen_stats[i] = (int)(10000000*fields[nodeNum].probVect[i]);
                tmp_sum += (int) (node->fallen_stats[i]);
            }
            node->fallen_stats[fallen_size-1] = 10000000 - tmp_sum;
            node->num_fallens = 10000000;
            
        }
        else
        {
            // terminal node is continuous
            node->response.fl = fields[nodeNum].expectation;   
            node->error = fields[nodeNum].variance;
                
        }
    }
    else
    {
		int s = sizeof( CxCARTSplit );
        CxCARTSplit *split;
        split = (CxCARTSplit *) malloc( s );
        split->feature_idx = fields[nodeNum].node_index;
        if( fields[nodeNum].Question == 1)
        {
            split->value.fl = fields[nodeNum].questionValue;
        }
        else
        {
            int tmpQuestionVal = (int)fields[nodeNum].questionValue;
            char *strQuestion;
            int j;
            int nodeSize = -1;
            for(j = 0; j < m_NodeTypes.size()-1; j++)
                if(m_domain[j] == fields[nodeNum].node_index ) 
                {
                 nodeSize = m_NodeTypes[j]->GetNodeSize();
                 if( m_NodeTypes[j]->IsDiscrete() == false)
                         PNL_THROW( CInvalidOperation, 
                        "we can compute 0 type questions only for discrete nodes " );
                 break;
                };
            if (nodeSize == -1)
                PNL_THROW( CInvalidOperation, 
                        "we can compute questions only by parents of tree node " );

            strQuestion = new char[nodeSize];
            int i;
            for( i= 0; i < nodeSize; i++ )
                strQuestion[i] = 0;
            strQuestion[tmpQuestionVal] = 1;
            split->value.ptr = (void*)strQuestion;
        }
        node->split= split;
        split->revert = 0;
    }
    intVector CurrentNodeChildren;
    pGraph->GetChildren(nodeNum,&CurrentNodeChildren);
    if( CurrentNodeChildren.size() !=0)
    {
        FillTree(pCart,node->child_left,pGraph,fields, CurrentNodeChildren[0]);
        FillTree(pCart,node->child_right,pGraph,fields, CurrentNodeChildren[1]);
        return;
    }
    else return;
    
}

void CTreeDistribFun::SetDomain(intVector domain )
{ 
 int i;
 int *tmp_domain;
 tmp_domain = new int[domain.size()];

 for ( i = 0; i < domain.size(); i++)
     tmp_domain[i] = domain[i];
 m_domain = tmp_domain;
 
}

floatVector CTreeDistribFun::GetProbability( const CEvidence* pEv ) const
{
    floatVector res;
    if (m_NodeTypes[m_NodeTypes.size() - 1]->IsDiscrete())
    {
        int DTNodeSize = m_NodeTypes[m_NodeTypes.size() - 1]->GetNodeSize();
        int i;
        CxCARTNode* node = m_pCART->root;
        CEvidence copyEv = *pEv;
        intVector indObsNow;
        copyEv.GetObsNodesFlags( &indObsNow );
        assert(node);
        CxClassifierVar var;
        intVector noObsNodes;
        while (icxIsNodeSplit(node))
        {
            if ( !(indObsNow[node->split->feature_idx]) )
            {
                noObsNodes.push_back(node->split->feature_idx);
                copyEv.ToggleNodeStateBySerialNumber(1, &(node->split->feature_idx));
            }
                if (copyEv.GetValue(node->split->feature_idx)->IsDiscrete())
                {
                    var.i = copyEv.GetValue(node->split->feature_idx)->GetInt();
                    var.fl = (float)var.i;
                }
                else 
                {
                    var.fl = copyEv.GetValue(node->split->feature_idx)->GetFlt();
                }
                node = icxIsVarSplitLeft(m_pCART, node->split , var)
                    ? node->child_left : node->child_right;
        }
        while ( noObsNodes.size() > 0 )
        {
            copyEv.ToggleNodeStateBySerialNumber(1, &(noObsNodes[noObsNodes.size() - 1]));
            noObsNodes.pop_back();
        }
        for( i = 0; i < DTNodeSize; ++i)
        {
            res.push_back(( (float)(node->fallen_stats[i])/(float)(node->num_fallens)));
        }
    }
    return res;
}

int CTreeDistribFun::GetAdjectives( const CEvidence* pEv, float &expect,
                                    float &varian) const
{
    int result = 0;
    if ( ! (m_NodeTypes[m_NodeTypes.size() - 1]->IsDiscrete()) )
    {
        CxCARTNode* node = m_pCART->root;
        assert(node);
        assert(pEv);
        CEvidence copyEv = *pEv;
        intVector indObsNow;
        copyEv.GetObsNodesFlags( &indObsNow );
        CxClassifierVar var;
        intVector noObsNodes;
        while (icxIsNodeSplit(node))
        {
            if ( !(indObsNow[node->split->feature_idx]) )
            {
                noObsNodes.push_back(node->split->feature_idx);
                copyEv.ToggleNodeStateBySerialNumber(1, &(node->split->feature_idx));
            }
                if (copyEv.GetValue(node->split->feature_idx)->IsDiscrete())
                {
                    var.i = copyEv.GetValue(node->split->feature_idx)->GetInt();
                    var.fl = (float)var.i;
                }
                else 
                {
                    var.fl = copyEv.GetValue(node->split->feature_idx)->GetFlt();
                }
                node = icxIsVarSplitLeft(m_pCART, node->split , var)
                    ? node->child_left : node->child_right;
        }
        while ( noObsNodes.size() > 0 )
        {
            copyEv.ToggleNodeStateBySerialNumber(1, &(noObsNodes[noObsNodes.size() - 1]));
            noObsNodes.pop_back();
        }
        expect = node->response.fl;
        varian = node->error;
        result = 1;
    }
    return result;
}

#ifdef PNL_RTTI
const CPNLType CTreeDistribFun::m_TypeInfo = CPNLType("CTreeDistribFun", &(CDistribFun::m_TypeInfo));

#endif