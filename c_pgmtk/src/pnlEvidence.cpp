/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEvidence.cpp                                             //
//                                                                         //
//  Purpose:   CEvidence class member functions implementation             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// pgmEvidece.cpp - implementation of methods of CEvidence class

#include "pnlConfig.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "pnlEvidence.hpp"
#include "pnlException.hpp"

PNL_BEGIN

bool CEvidence::Save(const char *fname,  pConstEvidenceVector& evVec)
{
    int nEv = evVec.size();
    PNL_CHECK_FOR_ZERO( nEv );
    const CModelDomain* pMD = evVec[0]->GetModelDomain();

    int nnodes = pMD->GetNumberVariables();

    FILE * stream = fopen(fname, "w");

    if( !stream )
    {
	return false;
    }

    const CEvidence* pEv;
    int i;
    for( i = 0; i < nEv; i++ )
    {
	if( evVec[i]->GetModelDomain() != pMD )
	{
	    fclose(stream);
	    PNL_THROW(CBadArg, "al evidences must have similar model domain")
	}
	pEv = evVec[i];
	int j;
	for( j = 0; j < nnodes; j++ )
	{
	    if( pEv->IsNodeObserved( j ) )
	    {
		const Value *val = pEv->GetValue(j);
		if( val->IsDiscrete() )
		{
		    fprintf(stream, "%d", val->GetInt());
		}
		else
		{
		    int nVls = pMD->GetVariableType(j)->GetNodeSize();
		    int k;
		    for( k = 0; k < nVls-1; k++ )
		    {
			fprintf(stream, "%g ", val[k].GetFlt());
		    }
		    fprintf(stream, "%g", val[k].GetFlt());

		}
	    }
	    else
	    {
		fprintf(stream, "N/A");
	    }
	    if( j != nnodes - 1 )
	    {
		fprintf(stream, " ");

	    }
	}
	if( i != nEv - 1)
	{
	    fprintf(stream, "\n");
	}
    }
    fclose(stream);
    return true;
}

bool CEvidence::Save(const char *fname,  const pEvidencesVecVector& evVec)
{

    int nSeries = evVec.size();
    PNL_CHECK_FOR_ZERO( nSeries );
    const CModelDomain* pMD = (evVec[0])[0]->GetModelDomain();

    int nnodes = pMD->GetNumberVariables()/2;

    FILE * stream = fopen(fname, "w");

    if( !stream )
    {
	return false;
    }

    const CEvidence* pEv;
    int series;
    for( series = 0; series < nSeries; series++ )
    {
	int nEv  = (evVec[series]).size();
	int i;
	for( i = 0; i < nEv; i++ )
	{
	    if( evVec[series][i]->GetModelDomain() != pMD )
	    {
		fclose(stream);
		PNL_THROW(CBadArg, "al evidences must have similar model domain")
	    }
	    pEv = (evVec[series])[i];
	    int j;
	    for( j = 0; j < nnodes; j++ )
	    {
		if( pEv->IsNodeObserved( j ) )
		{
		    const Value *val = pEv->GetValue(j);
		    if( val->IsDiscrete() )
		    {
			fprintf(stream, "%d", val->GetInt());
		    }
		    else
		    {
			int nVls = pMD->GetVariableType(j)->GetNodeSize();
			int k;
			for( k = 0; k < nVls - 1; k++ )
			{
			    fprintf(stream, "%g ", val[k].GetFlt());
			}
			fprintf(stream, "%g", val[k].GetFlt());

		    }
		}
		else
		{
		    fprintf(stream, "N/A");
		}
		if( j != nnodes -1 )
		{
		    fprintf(stream, " ");
		}
	    }

	    if( i < nEv -1 )
	    {
		fprintf(stream, "\t");
	    }
	    else
	    {
		if( series != nSeries -1 )
		{
		    fprintf(stream, "\n");
		}
	    }
	}

    }
    fclose(stream);
    return true;
}


bool CEvidence::Load(const char *fname,  pEvidencesVector* evVec, const CModelDomain *pMD)
{


    FILE * stream = fopen(fname, "rt");


    if( !stream )
    {
	return false;
    }
    int next;
    valueVector vls;
    int valInt;
    float valFlt;
    int nnodes = pMD->GetNumberVariables();
    intVector obsNds;
    obsNds.reserve(nnodes);

    boolVector isFloat(nnodes, true);
    intVector nVls;
    nVls.resize(nnodes);
    const CNodeType *nt;

    int numAllVls = 0;
    int i = 0;
    for( i = 0; i < nnodes; i++)
    {
	nt= pMD->GetVariableType(i);
	if( nt->IsDiscrete() )
	{
	    isFloat[i] = false;
	    nVls[i] = 1;
	}
	else
	{
	    nVls[i] = nt->GetNodeSize();
	}
	numAllVls += nVls[i];
    }


    vls.resize(numAllVls);
    valueVector::iterator it;
    valueVector::iterator itEnd;
    it = vls.begin();
    itEnd = vls.end();
    i = 0;
    int ev = 0;

    while(1)
    {
	next = getc(stream);
	//fread(&next, 1, 1, stream);
	if( feof(stream) ||  i == nnodes )
	{
	    if( i )
	    {
		if( it != itEnd)
		{
		    vls.erase(it, itEnd);
		}
		ev++;
		CEvidence *pEv = CEvidence::Create(pMD, obsNds, vls);
		evVec->push_back(pEv);
	    }

	    if( i== nnodes && !feof(stream) )
	    {
		i = 0;
		vls.resize(numAllVls);
		it = vls.begin();
		itEnd = vls.end();
		obsNds.clear();
		obsNds.reserve(nnodes);
		//fread(&next, 1, 1, stream);

	    }
	    else
	    {
		break;
	    }
	}

	if( next != ' ' && next != '\n' )
	{
	    if( isalpha(next) )
	    {

		if( next == 'N' )
		{
		    char buf[2];
		    fread(buf, sizeof(buf)/sizeof(buf[0]), sizeof(buf[0]), stream);

		    if( buf[0] != '/' || buf[1] != 'A' /*|| !isspace(buf[2])*/)
		    {
			fclose(stream);
			PNL_THROW( CBadArg, "Bad file with cases");
		    }
		    i++;
		}
		else
		{
		    fclose(stream);
		    PNL_THROW( CBadArg, "Bad file with cases");
		}
	    }
	    else
	    {
		obsNds.push_back(i);
		ungetc(next, stream);
		if( isFloat[i] )
		{
		    int j;
		    for( j = 0; j < nVls[i]; j++ )
		    {

			fscanf(stream, "%e", &valFlt);
			it->SetFlt(valFlt);
			it++;
		    }
		}
		else
		{

		    fscanf(stream, "%d", &valInt);
		    it->SetInt(valInt);
		    it++;
		}
		i++;
	    }

	}
    }
    fclose(stream);
    return true;

}


bool
CEvidence::Load(const char *fname,  pEvidencesVecVector* evVec, const CModelDomain *pMD)
{


    FILE * stream = fopen(fname, "rt");


    if( !stream )
    {
	return false;
    }
    int next;
    valueVector vls;
    int valInt;
    float valFlt;
    int nnodes = pMD->GetNumberVariables()/2;
    intVector obsNds;
    obsNds.reserve(nnodes);

    boolVector isFloat(nnodes, true);
    intVector nVls;
    nVls.resize(nnodes);
    const CNodeType *nt;

    int numAllVls = 0;
    int i = 0;
    for( i = 0; i < nnodes; i++)
    {
	nt= pMD->GetVariableType(i);
	if( nt->IsDiscrete() )
	{
	    isFloat[i] = false;
	    nVls[i] = 1;
	}
	else
	{
	    nVls[i] = nt->GetNodeSize();
	}
	numAllVls += nVls[i];
    }

    intVector nSlices(1, 0);
    while( !feof(stream) )
    {
	next = getc(stream);
	//fread(&next, 1, 1, stream);
	switch( next )
	{
	case '\t':
	    {
		nSlices.back() += 1;
		break;
	    }
	case '\n':
	    {
		nSlices.back() += 1;
		nSlices.push_back(0);
	    }
	    break;
	default:;

	}
    }
    nSlices.back() += 1;

    fseek(stream, 0, SEEK_SET);

    evVec->resize(nSlices.size());
    for( i = 0; i < nSlices.size(); i++ )
    {
	((*evVec)[i]).resize( nSlices[i]);
    }

    vls.resize(numAllVls);
    valueVector::iterator it;
    valueVector::iterator itEnd;
    it = vls.begin();
    itEnd = vls.end();

    int series = 0;
    int slice = 0;
    i = 0;
    while(1)
    {
	next = getc(stream);
	//fread(&next, 1, 1, stream);
	if( feof(stream) ||  i == nnodes )
	{
	    if( it != itEnd)
	    {
		vls.erase(it, itEnd);
	    }

	    CEvidence *pEv = CEvidence::Create(pMD, obsNds, vls);
	    ((*evVec)[series])[slice] = pEv;

	    if( feof(stream) )
	    {
		break;
	    }

	    if( slice != nSlices[series] - 1)
	    {
		slice++;
	    }
	    else
	    {
		slice = 0;
		series++;
	    }

	    i = 0;
	    vls.resize(numAllVls);
	    it = vls.begin();
	    obsNds.clear();
	    obsNds.reserve(nnodes);
	    fread(&next, 1, 1, stream);

	}

	if( next != ' ' && next != '\n' )
	{
	    if( isalpha(next) )
	    {

		if( next == 'N' )
		{
		    char buf[2];
		    fread(buf, sizeof(buf)/sizeof(buf[0]), sizeof(buf[0]), stream);

		    if( buf[0] != '/' || buf[1] != 'A')
		    {
			fclose(stream);
			PNL_THROW( CBadArg, "Bad file with cases");
		    }
		    i++;
		}
		else
		{
		    fclose(stream);
		    PNL_THROW( CBadArg, "Bad file with cases");
		}
	    }
	    else
	    {
		obsNds.push_back(i);
		ungetc(next, stream);
		if( isFloat[i] )
		{
		    int j;
		    for( j = 0; j < nVls[i]; j++ )
		    {

			fscanf(stream, " %e", &valFlt);
			it->SetFlt(valFlt);
			it++;
		    }
		}
		else
		{

		    fscanf(stream, " %d", &valInt);
		    it->SetInt(valInt);
		    it++;
		}
		i++;
	    }

	}
    }
    fclose(stream);
    return true;
}

CEvidence* CEvidence::Create( const CModelDomain* pMD,
			     const intVector& obsNodes, const valueVector& obsValues )
{
    int nObsNodes = obsNodes.size();
    const int* pObsNodes = &obsNodes.front();
    return CEvidence::Create( pMD, nObsNodes, pObsNodes, obsValues );
}
CEvidence* CEvidence::Create( const CNodeValues* values,
			     const intVector& obsNodes, const CModelDomain* pMD,
			     int takeIntoObservationFlags )
{
    int nObsNodes = obsNodes.size();
    const int* pObsNodes = &obsNodes.front();
    return CEvidence::Create( values, nObsNodes, pObsNodes, pMD,
	takeIntoObservationFlags );
}

CEvidence* CEvidence::Create( const CGraphicalModel* pGrModel,
			     const intVector& obsNodes, const valueVector& obsValues )
{
    CModelDomain* pMD = pGrModel->GetModelDomain();
    return CEvidence::Create( pMD, obsNodes, obsValues );
}

CEvidence*
CEvidence::Create(const CGraphicalModel *pGrModel, int NObsNodes,
		  const int *obs_nodes, const valueVector& obsValues)
{
    CModelDomain* pMD = pGrModel->GetModelDomain();
    return CEvidence::Create( pMD, NObsNodes, obs_nodes, obsValues );
}

void CEvidence::ToggleNodeState( const intVector& numsOfNds )
{
    int numOfNodes = numsOfNds.size();
    const int* pNodes = &numsOfNds.front();
    ToggleNodeState( numOfNodes, pNodes );
}

CEvidence*
CEvidence::Create(const CModelDomain *pMD, int NObsNodes, const int *obs_nodes,
		  const valueVector& obsValues)
{
    PNL_CHECK_IS_NULL_POINTER( pMD );
    int numVars = pMD->GetNumberVariables();
    PNL_CHECK_RANGES( NObsNodes, 0, numVars );
    int i;
    if( NObsNodes > 0 )
    {
	PNL_CHECK_IS_NULL_POINTER( obs_nodes );

	//check there is only one value per node in ModelDomain
	intVector varsInMD;
	varsInMD.resize(numVars);
	for( i = 0; i < numVars; varsInMD[i] = i, i++ );

	pConstNodeTypeVector ObsNodeTypes(NObsNodes);
	int loc;
	for( i = 0; i < NObsNodes; i++ )
	{
	    int curNum = obs_nodes[i];
	    loc = std::find( varsInMD.begin(), varsInMD.end(), curNum )
		- varsInMD.begin();
	    if( loc < varsInMD.size() )
	    {
		varsInMD.erase( varsInMD.begin() + loc );
		ObsNodeTypes[i] = pMD->GetVariableType(curNum);
	    }
	    else
	    {
		PNL_THROW( CInvalidOperation,
		    "one node from Model Domain must takes only one value at once" )
	    }

	}
	CEvidence *pNewEvidence = new CEvidence( NObsNodes, obs_nodes,
	    obsValues,	&ObsNodeTypes.front(), pMD );
	return pNewEvidence;
    }
    else
    {
	CEvidence* pNewEvidence = new CEvidence( NObsNodes, NULL,
	    valueVector(), NULL, pMD );
	return pNewEvidence;
    }
}

CEvidence*
CEvidence::Create(const CNodeValues *values, int nObsNodes, const int *obsNodes,
		  const CModelDomain* pMD, int takeIntoObservationFlags )
{
    PNL_CHECK_IS_NULL_POINTER( values );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    int numNodesInVals = values->GetNumberObsNodes();
    int numNodesInMD = pMD->GetNumberVariables();
    PNL_CHECK_RANGES( nObsNodes, 0, numNodesInMD );

    if( numNodesInVals != nObsNodes )
    {
	PNL_THROW ( CInconsistentSize, "number of nodes in values differ from nObsNodes" );
    }
    valueVector RawData;
    values->GetRawData( &RawData );
    const CNodeType *const * nodeTypes = values->GetNodeTypes();
    CEvidence *ev = new CEvidence( nObsNodes, obsNodes, RawData,
	nodeTypes, pMD );
    //we've created evidence in it all nodes are really observed,
    //we need to toggle some of them
    if( takeIntoObservationFlags )
    {
	const int *flags = values->GetObsNodesFlags();
	intVector toggleNodes;
	for ( int i = 0; i < nObsNodes; i++ )
	{
	    if( !flags[i] )
	    {
		toggleNodes.push_back(obsNodes[i]);
	    }
	}
	ev->ToggleNodeState( toggleNodes.size(), &toggleNodes.front() );
    }
    return ev;
}

CEvidence::CEvidence(int nNodes, const int *obsNodes, const valueVector& pEvidence,
		     const CNodeType *const *ObsNodeTypes,const CModelDomain* pMD )
		     :CNodeValues( nNodes, ObsNodeTypes, pEvidence)
{
    if( nNodes )
    {
	m_obsNodes.assign( obsNodes, obsNodes + nNodes );
    }
    m_pMD = pMD;
}

CEvidence::~CEvidence()
{
}

void CEvidence::MakeNodeHidden( int nodeNumber )
{
    int number = std::find( m_obsNodes.begin(), m_obsNodes.end(),
	nodeNumber) - m_obsNodes.begin();
    if( number >= m_numberObsNodes )
    {
	PNL_THROW( COutOfRange, "query node is not in observed nodes" );
    }
    if( !m_isObsNow[number] )
    {
	PNL_THROW( CInvalidOperation, "node is hidden now!" );
    }
    MakeNodeHiddenBySerialNum( number );
}

void CEvidence::MakeNodeObserved( int nodeNumber )
{
    int number = std::find( m_obsNodes.begin(), m_obsNodes.end(),
	nodeNumber) - m_obsNodes.begin();
    if( number >= m_numberObsNodes )
    {
	PNL_THROW( COutOfRange, "query node is not in observed nodes" );
    }
    if( m_isObsNow[number] )
    {
	PNL_THROW( CInvalidOperation, "node is observed" )
    }
    MakeNodeObservedBySerialNum(number);
}

void CEvidence::ToggleNodeState(int nNodes, const int *nodeNumbers)
{
/*to change the status of node from really observed
    to potentially observed and backwards*/
    int i;
    if( ( nNodes<0 ) || ( nNodes > m_numberObsNodes ) )
    {
	PNL_THROW( CInconsistentSize	,
	    "number of nodes to toggle must be less than number of all potentially observed nodes" );
	/*number of nodes to toggle must be less than m_numberObsNodes*/
    }
    else
    {
	int flagAllRight = 1;
	// all nodes from nodeIndices are potentially
	// observed (if it is so - all numbers of this nodes
	// are in m_ObsNodes
	int loc;
	int *nodeIndices = new int[nNodes];
	PNL_CHECK_IF_MEMORY_ALLOCATED( nodeIndices );
	/*checking up all input data*/
	for( i = 0; i < nNodes; i++ )
	{
	    loc = std::find( m_obsNodes.begin(), m_obsNodes.end(),
		nodeNumbers[i]) - m_obsNodes.begin();
	    if( loc >= m_numberObsNodes )
	    {
		flagAllRight = 0;
		break;
	    }
	    nodeIndices[i] = loc;
	}
	if ( flagAllRight )
	{
	    ToggleNodeStateBySerialNumber( nNodes, nodeIndices );
	}
	else
	{
	    PNL_THROW( COutOfRange, "some node has number which is not in m_obsNode" );
	}
	delete []nodeIndices;
    }
}
/*to use really observed nodes we need to do :
	int *inOffset;
	int *reallyObservedOffset;
	int *reallyObservedNodes;
	int numReallyObserved = 0;
	for (int i=0;i<;i++)
	{
	inOffset = std::find (m_offset.begin(), m_offset.end(),1);
	if (inOffset!=m_offset.end())
	{
	reallyObservedOffset[numReallyObserved] = m_offset[inOffset];
	reallyObservedNodes = inOffset - m_offset.begin();
	numReallyObserved++;
	}
	else {assert(0);}
	};
	*/
	/*Other version: use:
	const int *mReallyObs = myEvid->GetObsNodesFlags();
	if(mReallyObs[i])
	{
	...
	}

*/

const Value* CEvidence::GetValue( int nodeNumber )const
{
    int number = std::find(m_obsNodes.begin(), m_obsNodes.end(),
	nodeNumber) - m_obsNodes.begin();
    if( number >= m_numberObsNodes)
    {
	PNL_THROW( COutOfRange, "query node is not in observed nodes" );
	return NULL;
    }
    if( !m_isObsNow[number] )
    {
	PNL_THROW( CInvalidOperation, "this node is hidden" )
    }
    return GetValueBySerialNumber(number);
}

Value* CEvidence::GetValue( int nodeNumber )
{
    int number = std::find(m_obsNodes.begin(), m_obsNodes.end(),
	nodeNumber) - m_obsNodes.begin();
    if( number >= m_numberObsNodes+1)
    {
	PNL_THROW( COutOfRange, "query node is not in observed nodes" );
	return NULL;
    }
    return GetValueBySerialNumber( number );
}

int CEvidence::IsNodeObserved( int nodeNumber ) const
{
    int number = std::find(m_obsNodes.begin(), m_obsNodes.end(),
	nodeNumber) - m_obsNodes.begin();
    if( number >= m_numberObsNodes )
    {
	return 0;
    }
    return IsObserved(number);
}

const int* CEvidence::GetAllObsNodes()const
{
	return &m_obsNodes.front();
}

void CEvidence::GetObsNodesWithValues( intVector* pObsNodes,
			               pConstValueVector* pObsValues,
			               pConstNodeTypeVector* pNodeTypes )const
{
    PNL_CHECK_IS_NULL_POINTER(pObsNodes);
    PNL_CHECK_IS_NULL_POINTER(pObsValues);

    pObsNodes->clear();
    pObsValues->clear();

    for( int i = 0; i < m_numberObsNodes; i++ )
    {
	if( m_isObsNow[i] )
	{
	    pObsNodes->push_back( m_obsNodes[i] );
	    pObsValues->push_back( &m_rawValues[m_offset[i]] );
	}
    }
    if( pNodeTypes )
    {
	pNodeTypes->resize( pObsNodes->size());
	int t = 0;
	for( int i = 0; i < m_numberObsNodes; i++ )
	{
	    if( m_isObsNow[i] )
	    {
		(*pNodeTypes)[t] = m_NodeTypes[i];
		t++;
	    }
	}

	PNL_CHECK_FOR_NON_ZERO(t - pObsNodes->size());
    }
}

void CEvidence::Dump() const
{
    intVector            obsNodes;
    pConstValueVector    obsValues;
    pConstNodeTypeVector nodeTypes;

    GetObsNodesWithValues( &obsNodes, &obsValues, &nodeTypes );

    for( int i = 0; i < obsNodes.size(); i++ )
    {
	std::cout<<"values for node"<<obsNodes[i]<<" are:";
	if( (nodeTypes[i])->IsDiscrete() )
	{
	    std::cout<<obsValues[i]->GetInt()<<" it is discrete node"<<std::endl;
	}
	else
	{
	    for( int j = 0; j < nodeTypes[i]->GetNodeSize(); j++ )
	    {
		float val = (obsValues[i])[j].GetFlt();
		std::cout<<val<<";  ";
	    }
	    std::cout<<" it is continues node"<<std::endl;
	}
    }
}

 void CEvidence::GetValues( int nodeNumber, valueVector* vlsOut ) const
 {
     const Value *pVal = GetValue(nodeNumber);
     vlsOut->assign( pVal, pVal + GetModelDomain()->GetNumVlsForNode(nodeNumber) );
 }

 void CEvidence::GetAllObsNodes( intVector* obsNdsOut ) const
 {
     obsNdsOut->assign( GetAllObsNodes(), GetAllObsNodes() + GetNumberObsNodes() );
 }

 void CEvidence::GetObsNodesWithValues( intVector* obsNodesOut,
     valueVecVector* obsValuesOut,
     pConstNodeTypeVector* pNodeTypesOut) const
 {
     pConstValueVector pObsVals;
     GetObsNodesWithValues( obsNodesOut, &pObsVals, pNodeTypesOut );
     obsValuesOut->resize(obsNodesOut->size());
     int i;
     for( i = 0; i < obsNodesOut->size(); i++ )
     {
	 (*obsValuesOut)[i].assign(pObsVals[i], 
	     pObsVals[i] + GetModelDomain()->GetNumVlsForNode((*obsNodesOut)[i]));
     }
     
 }
 
bool CEvidence::IsAllDiscreteNodesObs(const CStaticGraphicalModel *pGrModel) const
 {
     intVector         obsNdsNums;
     pConstValueVector obsNdsVals;
     GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );
     
     intVector discrObsNdsNums(0);
     int i;
     for ( i = 0; i < obsNdsNums.size(); i++)
     {
         if (pGrModel->GetNodeType(obsNdsNums[i])->IsDiscrete())
         {
             discrObsNdsNums.push_back(obsNdsNums[i]);
         }
     }

     bool isAllDiscrObs = true;

     int NumDiscr = 0;
     for ( i = 0; i < pGrModel->GetNumberOfNodes(); i++)
     {
         if (pGrModel->GetNodeType(i)->IsDiscrete())
         {
             NumDiscr++;
             intVector ::iterator loc = std::find(discrObsNdsNums.begin(),discrObsNdsNums.end(), i);
             if (loc ==  discrObsNdsNums.end())
             {
                 isAllDiscrObs = false;
                 break;
             }
         }
     }
     return (NumDiscr == 0)? false :isAllDiscrObs;
 }

bool CEvidence::IsAllCountinuesNodesObs( const CStaticGraphicalModel *pGrModel) const
{
    intVector         obsNdsNums;
    pConstValueVector obsNdsVals;
    GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );
    
    intVector contObsNdsNums(0);
    int i;
    for(i = 0; i < obsNdsNums.size(); i++)
    {
        if (!pGrModel->GetNodeType(obsNdsNums[i])->IsDiscrete())
        {
            contObsNdsNums.push_back(obsNdsNums[i]);
        }
    }

    bool isAllContObs = true;
    int NumCont = 0;
    for ( i = 0; i < pGrModel->GetNumberOfNodes(); i++)
    {
        if (!pGrModel->GetNodeType(i)->IsDiscrete())
        {
            NumCont++;
            intVector ::iterator loc = std::find(contObsNdsNums.begin(),contObsNdsNums.end(), i);
            if (loc ==  contObsNdsNums.end())
            {
                isAllContObs = false;
                break;
            }
        }
    }
    return (NumCont == 0)? false :isAllContObs;
}

#ifdef PNL_RTTI
const CPNLType CEvidence::m_TypeInfo = CPNLType("CEvidence", &(CNodeValues::m_TypeInfo));

#endif

PNL_END
