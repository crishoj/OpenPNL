// pnlDAG.cpp: implementation of the CDAG class.
//
//////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlDAG.hpp"
#include "pnlException.hpp"
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"

#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
PNL_USING
int CDAG::m_iNumberOfRandomUsed = -1;
CDAG::CDAG( int numOfNds, const int* numOfNbrs, const int *const *nbrsList,
	   const ENeighborType *const *nbrsTypes ) : CGraph(numOfNds, numOfNbrs, nbrsList, nbrsTypes)
{
    m_pAncesstorMatrix = NULL;
}
CDAG::CDAG( int numOfNds, const int *const *adjMat ) : CGraph(numOfNds, adjMat)
{
    m_pAncesstorMatrix = NULL;
}
CDAG::	CDAG(const CMatrix<int>* pAdjMat) : CGraph(pAdjMat)
{
    m_pAncesstorMatrix = NULL;
}
CDAG::CDAG(const CGraph& rGraph) :CGraph(rGraph)
{
    m_pAncesstorMatrix = NULL;
}

CDAG::~CDAG()
{
    ClearContent();
}

void CDAG::ClearContent()
{
    delete m_pAncesstorMatrix;
    m_pAncesstorMatrix = NULL;
}

void CDAG::CreateAncestorMatrix()
{
    delete m_pAncesstorMatrix;
    m_pAncesstorMatrix = C2DBitwiseMatrix::Create(GetNumberOfNodes(), GetNumberOfNodes());

    for(int i=0; i<GetNumberOfNodes(); i++)
    {
	intQueue qAncesstor;
	qAncesstor.push(i);
	while(!qAncesstor.empty())
	{
	    int nNode = qAncesstor.front();
	    qAncesstor.pop();
	    intVector vParrents;
	    GetParents(nNode, &vParrents);
	    for(int j=0; j<vParrents.size(); j++)
	    {
		qAncesstor.push(vParrents[j]);
		m_pAncesstorMatrix->SetValue(i, vParrents[j], 1);
	    }
	}
    }
}

CDAG* CDAG::Create( int numOfNds, const int *numOfNbrsIn,
		   const int *const *nbrsListIn,
		   const ENeighborType *const *nbrsTypesIn  )
{
    CDAG* pRet = new CDAG(numOfNds, numOfNbrsIn, nbrsListIn, nbrsTypesIn);
    if(!pRet->IsDAG())
    {
	delete pRet;
	return NULL;
    }
    pRet->CreateAncestorMatrix();
    return pRet;
}
CDAG* CDAG::Create( int numOfNds, const int *const *adjMatIn )
{
    CDAG* pRet = new CDAG(numOfNds, adjMatIn);
    if(!pRet->IsDAG())
    {
	delete pRet;
	return NULL;
    }
    pRet->CreateAncestorMatrix();
    return pRet;
}
CDAG* CDAG::Create(const CMatrix<int>* pAdjMat)
{
    CDAG* pRet = new CDAG(pAdjMat);
    if(!pRet->IsDAG())
    {
	delete pRet;
	return NULL;
    }
    pRet->CreateAncestorMatrix();
    return pRet;
}
CDAG* CDAG::Create(const CGraph& pGraph)
{
    CDAG* pRet = new CDAG(pGraph);
    if(!pRet->IsDAG())
    {
	delete pRet;
	return NULL;
    }
    pRet->CreateAncestorMatrix();
    return pRet;

}
//// /////////////
///
////////////////////

CDAG* CDAG::Change(int iStartNode, int iEndNode, EDAGChangeType iChangeType)
{
    CDAG* pRet;
    switch(iChangeType)
    {
    case DAG_ADD:
	if(m_pAncesstorMatrix->GetValue(iStartNode, iEndNode))
	{
	    return NULL; // invalid add
	}
	pRet = Clone();
	pRet->AddEdge(iStartNode, iEndNode, 1);
	pRet->UpdateAncestorMatrix(iStartNode, iEndNode, iChangeType);
	return pRet;
	break;
    case DAG_DEL:
	if(!IsExistingEdge(iStartNode, iEndNode))
	    return NULL;
	pRet = Clone();
	pRet->RemoveEdge(iStartNode, iEndNode); // always legel
	pRet->UpdateAncestorMatrix(iStartNode, iEndNode, iChangeType);
	return pRet;
	break;
    case DAG_REV:
	//		int nDirect = -1 * GetEndgeDirect(iStartNode, iEndNode);
	if(!IsExistingEdge(iStartNode, iEndNode))
	    return NULL;
	pRet = Clone();
	pRet->RemoveEdge(iStartNode, iEndNode); // always legel
	pRet->UpdateAncestorMatrix(iStartNode, iEndNode, DAG_DEL);
	if(pRet->m_pAncesstorMatrix->GetValue(iEndNode, iStartNode))
	{
	    delete pRet;
	    return NULL;
	}
	else
	{
	    pRet->AddEdge(iEndNode, iStartNode, 1);
	    pRet->UpdateAncestorMatrix(iEndNode, iStartNode, DAG_ADD);

	}
	return pRet;
	break;
    };
    return NULL;
}
void CDAG::UpdateAncestorMatrix(int iStartNode, int iEndNode, EDAGChangeType iChangeType)
{
    int i;
    switch(iChangeType)
    {
    case DAG_ADD:
	m_pAncesstorMatrix->SetValue(iEndNode, iStartNode, 1);
	m_pAncesstorMatrix->AddOneRowWithAnother(iStartNode, iEndNode);
	for(i=0; i<m_pAncesstorMatrix->GetHeight(); i++)
	    if(m_pAncesstorMatrix->GetValue(i, iEndNode))
		m_pAncesstorMatrix->AddOneRowWithAnother(iEndNode, i);
	    break;
    case DAG_DEL:
	intQueue descendantsQueue;
	descendantsQueue.push(iEndNode);
	while(!descendantsQueue.empty())
	{
	    int nCurrentNode = descendantsQueue.front();
	    descendantsQueue.pop();
	    UpdateRemoveNodes(nCurrentNode);
	    intVector vChildren;
	    GetChildren(nCurrentNode, &vChildren);
	    for(i=0; i<vChildren.size(); i++)
		descendantsQueue.push(vChildren[i]);
	}

	break;
    };

}

void CDAG::UpdateRemoveNodes(int iNodeNumber)
{
    m_pAncesstorMatrix->ZeroOneRow(iNodeNumber);
    intVector nParrent;
    GetParents(iNodeNumber, &nParrent);
    int i;
    for(i=0; i<nParrent.size(); i++)
	m_pAncesstorMatrix->SetValue(iNodeNumber, nParrent[i], 1);
    for(i=0; i<nParrent.size(); i++)
	m_pAncesstorMatrix->AddOneRowWithAnother(nParrent[i], iNodeNumber);
}

#ifdef _PNL_DEBUG_
void CDAG::DumpMatrix()
{
    Log dump("AncesstorMatrix: ", eLOG_RESULT, eLOGSRV_PNL);
    for(int i=0; i<m_pAncesstorMatrix->GetHeight(); i++)
    {
	for(int j=0; j<m_pAncesstorMatrix->GetWidth(); j++)
	    if(m_pAncesstorMatrix->GetValue(i, j))
		dump<<" 1";
	    else
		dump<<" 0";
	    dump<<"\n";
    }

}
#endif
CDAG* CDAG::Clone()
{
    CGraph* p = (CGraph*)this;
    CDAG* pRet = new CDAG(*p);
    pRet->SetAncesstorMatrix(this->m_pAncesstorMatrix);
    return pRet;
}

void CDAG::GetAllNeighbours(POINTVECTOR &vNeighbours, EDGEOPVECTOR &vOPs)
{
    int i;

    for(i = 0; i<vNeighbours.size(); i++)
    {
	CDAG* p = (CDAG*)vNeighbours[i];
	delete p;
    }

    vNeighbours.clear();
    vOPs.clear();
    for(i = 0; i<GetNumberOfNodes(); i++)
	for(int j = 0; j<GetNumberOfNodes(); j++)
	{
	    if(i == j)
	    {
		continue;
	    }
	    int nDirect = GetEdgeDirect(i, j);
	    if(nDirect == 1) // try remove, change
	    {
		CDAG* pNeighbour = Change(i, j, DAG_DEL);
		if(pNeighbour!=NULL)
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_DEL;
		    vOPs.push_back(eOP);
		}

		pNeighbour = Change(i, j, DAG_REV);
		if(pNeighbour!=NULL)
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_REV;
		    vOPs.push_back(eOP);
		}
	    }
	    else if (nDirect == 0)//no edge, try to add it.
	    {
		CDAG* pNeighbour = Change(i, j, DAG_ADD);
		if(pNeighbour!=NULL)
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_ADD;
		    vOPs.push_back(eOP);
		}

	    }
	}
}
void CDAG::GetAllNeighbours(POINTVECTOR &vNeighbours, EDGEOPVECTOR &vOPs, bool (*IsValid)(CDAG* pDAG))
{
    int i;

    for(i=0; i<vNeighbours.size(); i++)
    {
	CDAG* p = (CDAG*)vNeighbours[i];
	delete p;
    }
    vNeighbours.clear();
    vOPs.clear();
    for(i=0; i<GetNumberOfNodes(); i++)
	for(int j=0; j<GetNumberOfNodes(); j++)
	{
	    if(i == j)
	    {
		continue;
	    }
	    int nDirect = GetEdgeDirect(i, j);
	    if(nDirect == 1) // try remove, change
	    {
		CDAG* pNeighbour = Change(i, j, DAG_DEL);
		if(pNeighbour!=NULL && (*IsValid)(pNeighbour))
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_DEL;
		    vOPs.push_back(eOP);
		}
		else
		    delete pNeighbour;

		pNeighbour = Change(i, j, DAG_REV);
		if(pNeighbour!=NULL && (*IsValid)(pNeighbour))
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_REV;
		    vOPs.push_back(eOP);
		}
		else
		    delete pNeighbour;
	    }
	    else if (nDirect == 0)//no edge, try to add it.
	    {
		CDAG* pNeighbour = Change(i, j, DAG_ADD);
		if(pNeighbour!=NULL&& (*IsValid)(pNeighbour))
		{
		    vNeighbours.push_back((void*)pNeighbour);
		    EDGEOP eOP ;
		    eOP.originalEdge.startNode = i;
		    eOP.originalEdge.endNode = j;
		    eOP.DAGChangeType = DAG_ADD;
		    vOPs.push_back(eOP);
		}
		else
		    delete pNeighbour;

	    }
	}

}

C2DBitwiseMatrix* CDAG::GetAncesstorMatrix()
{
    return m_pAncesstorMatrix;
}
void CDAG::SetAncesstorMatrix(C2DBitwiseMatrix *pMatrix)
{
    if(m_pAncesstorMatrix == NULL)
	m_pAncesstorMatrix = C2DBitwiseMatrix::Create(1, 1);
    (*m_pAncesstorMatrix) = (*pMatrix);
}
int CDAG::GetMaxFanIn()
{
    int nMax =0;
    for(int i=0; i<GetNumberOfNodes(); i++)
    {
	int nParrents = GetNumberOfParents(i);
	if(nParrents > nMax)
	    nMax = nParrents;
    }
    return nMax;
}

bool CDAG::TopologicalSort(intVector* pvOutput)
{
    pvOutput->clear();
    int* pOutNumber = new int[GetNumberOfNodes()];
    intQueue node_stack;
    for(int i=0; i<GetNumberOfNodes(); i++)
    {
	pOutNumber[i] =GetNumberOfParents(i);
	if(pOutNumber[i] ==0)
	    node_stack.push(i);
    }
    while(!node_stack.empty())
    {
	int nTop = node_stack.front();
	node_stack.pop();
	pvOutput->push_back(nTop);
	if(pvOutput->size() > GetNumberOfNodes())
	{
	    delete []pOutNumber;
	    return false;
	}
	intVector vChildren;
	GetChildren(nTop, &vChildren);
	for(int j=0; j<vChildren.size(); j++)
	{
	    int nChildNode = vChildren[j];
	    pOutNumber[nChildNode]= pOutNumber[nChildNode] -1;
	    if(pOutNumber[nChildNode] == 0)
		node_stack.push(nChildNode);
	}


    }
    delete []pOutNumber;
    return true;
}
void CDAG::MarkovBlanket(int nNodeNumber, intVector *pvOutPut)
{
    pvOutPut->clear();
    int* pIn = new int[GetNumberOfNodes()];
    int i;

    for(i=0; i<GetNumberOfNodes(); i++)
	pIn[i] =0;

    intVector vParrent;
    GetParents(nNodeNumber, &vParrent);
    for(i=0; i<vParrent.size(); i++)
	pIn[vParrent[i]] =1;

    intVector vChildren;
    GetChildren(nNodeNumber, &vChildren);
    for(i=0; i<vChildren.size(); i++)
	pIn[vChildren[i]] =1;

    for(i=0; i<vChildren.size(); i++)
    {
	intVector vP;
	GetParents(vChildren[i], &vP);
	for(int j=0; j<vP.size(); j++)
	    pIn[vP[j]] =1;
    }
    pIn[nNodeNumber] =0;
    for(i=0; i<GetNumberOfNodes(); i++)
	if(pIn[i] == 1)
	    pvOutPut->push_back(i);
	delete []pIn;
}

bool CDAG::IsEquivalent(CDAG *pDAG1)
{
    PNL_CHECK_IS_NULL_POINTER(pDAG1);

    if(pDAG1->GetNumberOfNodes() != GetNumberOfNodes())
	return false;
    bool bEqual= true;
    int i;

    for(i=0; i<pDAG1->GetNumberOfNodes(); i++)
    {
    /*		if(pDAG1->m_nbrsList[i]->size() != m_nbrsList[i]->size())
    return false;
    for(int j=0; j<pDAG1->m_nbrsList[i]->size(); j++)
    {
    if((*(pDAG1->m_nbrsList[i]))[j] != (*(m_nbrsList[i]))[j])
				return false;
				if((*(pDAG1->m_nbrsTypes[i]))[j] != (*(m_nbrsTypes[i]))[j])
				bEqual = false;
    }*/
	intVector vNeighbours1;
	intVector vNeighbours;
	neighborTypeVector vNeighbourType1;
	neighborTypeVector vNeighbourType;
	pDAG1->GetNeighbors(i, &vNeighbours1, &vNeighbourType1);
	GetNeighbors(i, &vNeighbours, &vNeighbourType);
	if(vNeighbours.size() != vNeighbours1.size())
	    return false;
	for(int j=0; j<vNeighbours.size(); j++)
	{
	    if(vNeighbours[j]!=vNeighbours1[j])
		return false;
	    if(vNeighbourType[j]!=vNeighbourType1[j])
		bEqual = false;
	}

    }
    if(bEqual)
	return true;

    for(i=0; i<pDAG1->GetNumberOfNodes(); i++)
    {
	intVector vDAG1Parents;
	pDAG1->GetParents(i, &vDAG1Parents);
	intVector vDAG2Parents;
	GetParents(i, &vDAG2Parents);
	if(vDAG1Parents.size() >1)
	{
	    for(int j=1; j<vDAG1Parents.size(); j++)
		for(int k=0; k<j; k++)
		{
		    if(!pDAG1->IsExistingEdge(vDAG1Parents[j], vDAG1Parents[k])) // a v structure
		    {
			bool b = (IsExistingEdge(vDAG1Parents[j], vDAG1Parents[k]) == 0);
			b = b & (std::find(vDAG2Parents.begin(), vDAG2Parents.end(), vDAG1Parents[j]) != vDAG2Parents.end());
			b = b & (std::find(vDAG2Parents.begin(), vDAG2Parents.end(), vDAG1Parents[k]) != vDAG2Parents.end());
			if(!b)
			    return false;
		    }
		}
	}

	if(vDAG2Parents.size() >1)
	{
	    for(int j=1; j<vDAG2Parents.size(); j++)
		for(int k=0; k<j; k++)
		{
		    if(!IsExistingEdge(vDAG2Parents[j], vDAG2Parents[k])) // a v structure
		    {
			bool b = (pDAG1->IsExistingEdge(vDAG2Parents[j], vDAG2Parents[k]) == 0);
			b = b & (std::find(vDAG1Parents.begin(), vDAG1Parents.end(), vDAG2Parents[j]) != vDAG1Parents.end());
			b = b & (std::find(vDAG1Parents.begin(), vDAG1Parents.end(), vDAG2Parents[k]) != vDAG1Parents.end());
			if(!b)
			    return false;
		    }
		}
	}

    }
    return true;
}

void CDAG::GetAllEdges(EDGEVECTOR* pvOutput)
{
    PNL_CHECK_IS_NULL_POINTER(pvOutput);
    pvOutput->clear();
    for(int i=0; i<GetNumberOfNodes(); i++)
    {
	intVector vNeighbours;
	neighborTypeVector vNeighboursType;
	GetNeighbors(i, &vNeighbours, &vNeighboursType);
	for(int j=0; j<vNeighbours.size(); j++)
	{
	    int iEndNode = vNeighbours[j];
	    ENeighborType nType = vNeighboursType[j];
	    if(nType == ntChild)
	    {
		EDGE edge;
		edge.endNode = iEndNode;
		edge.startNode =i;
		pvOutput->push_back(edge);
	    }
	}
    }
}
int CDAG::GetEdgeDirect(int startNode, int endNode)
{
    intVector vNeighbours;
    neighborTypeVector vNeighboursType;
    GetNeighbors(startNode, &vNeighbours, &vNeighboursType);
    int numberOfNodes = vNeighbours.size();

    /* bad-args check */
    //	PNL_CHECK_RANGES( startNode, 0, numberOfNodes - 1 );
    //	PNL_CHECK_RANGES( endNode,   0, numberOfNodes - 1 );
    /* bad-args check end */
    int i=0;
    for(i=0; i<numberOfNodes; i++)
    {
	if(vNeighbours[i] == endNode)
	    break;
    }
    if(i == numberOfNodes)
	return 0;
    ENeighborType ntype = vNeighboursType[i];

    if(ntype == ntParent)
	return -1;
    return 1;
}

bool CDAG::DoMove(int iStartNode, int iEndNode, EDAGChangeType iChangeType)
{
    if(!IsValidMove(iStartNode, iEndNode, iChangeType))
	return false;

    switch(iChangeType)
    {
    case DAG_ADD:
	AddEdge(iStartNode, iEndNode, 1);
	UpdateAncestorMatrix(iStartNode, iEndNode, iChangeType);
	break;
    case DAG_DEL:
	RemoveEdge(iStartNode, iEndNode); // always legel
	UpdateAncestorMatrix(iStartNode, iEndNode, iChangeType);
	break;
    case DAG_REV:
	RemoveEdge(iStartNode, iEndNode);
	UpdateAncestorMatrix(iStartNode, iEndNode, DAG_DEL);
	AddEdge(iEndNode, iStartNode, 1);
	UpdateAncestorMatrix(iEndNode, iStartNode, DAG_ADD);
	break;
    };
    return true;

}

void CDAG::GetAllValidMove(EDGEOPVECTOR *pvOutput, intVector* pvAncesstorVector, intVector* pvDescendantsVector, intVector* pvNotParents, intVector* pvNotChild)
{
 pvOutput->clear();
    bool bCheck = false;
    if(pvAncesstorVector != NULL && pvDescendantsVector!= NULL) // if one  set is empty all are valid
	if(!pvAncesstorVector->empty() && !pvDescendantsVector->empty())
	    bCheck = true;

	bool bCheck2 = false;
	if(pvNotParents != NULL && pvNotChild!= NULL) // if one  set is empty all are valid
	    if(!pvNotParents->empty() && !pvNotChild->empty())
		bCheck2 = true;

	    for(int i=0; i<GetNumberOfNodes(); i++)
		for(int j=0; j<GetNumberOfNodes(); j++)
		{
		    if(i == j)
		    {
			continue;
		    }
		    int nDirect = GetEdgeDirect(i, j);
		    if(nDirect == 1) // try remove, change
		    {
			if(IsValidMove(i, j, DAG_DEL))
			{
			    EDGEOP eOP;
			    eOP.originalEdge.startNode = i;
			    eOP.originalEdge.endNode = j;
			    eOP.DAGChangeType = DAG_DEL;
			    if(/*(!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) &&*/ (!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
				pvOutput->push_back(eOP);
			    //					if(!bCheck)
			    //						pvOutput->push_back(eOP);


			}
			if(IsValidMove(i, j, DAG_REV))
			{
			    EDGEOP eOP;
			    eOP.originalEdge.startNode = i;
			    eOP.originalEdge.endNode = j;
			    eOP.DAGChangeType = DAG_REV;
			    if(/*(!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) && */(!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
				pvOutput->push_back(eOP);
			    //					if(!bCheck)
			    //						pvOutput->push_back(eOP);
			}
		    }
		    else if (nDirect == 0)//no edge, try to add it.
		    {
			if(IsValidMove(i, j, DAG_ADD))
			{
			    EDGEOP eOP;
			    eOP.originalEdge.startNode = i;
			    eOP.originalEdge.endNode = j;
			    eOP.DAGChangeType = DAG_ADD;
			    if(/*(!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) &&*/ (!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
				pvOutput->push_back(eOP);
			    //					if(!bCheck)
			    //						pvOutput->push_back(eOP);
			}
		    }
		}
}

bool CDAG::IsValidMove(int iStartNode, int iEndNode, EDAGChangeType iChangeType)
{
    switch(iChangeType)
    {
    case DAG_ADD:
	if(m_pAncesstorMatrix->GetValue(iStartNode, iEndNode))
	{
	    return false; // invalid add
	}
	//		pRet = Clone();
	break;
    case DAG_DEL:
	if(!IsExistingEdge(iStartNode, iEndNode))
	    return false;
	//		pRet = Clone();
	break;
    case DAG_REV:
	//		int nDirect = -1 * GetEndgeDirect(iStartNode, iEndNode);
	if(!IsExistingEdge(iStartNode, iEndNode))
	    return false;
	intVector nParrent;
    GetParents(iEndNode, &nParrent);
    for(int i=0; i<nParrent.size(); i++)
        if ((nParrent[i] != iStartNode) && m_pAncesstorMatrix->GetValue(nParrent[i], iStartNode))
            return false;
	break;
    };
    return true;
}

CDAG* CDAG::RootTree()
{
    CDAG* pRet = Clone();
    int * pDone = new int[pRet->GetNumberOfNodes()];
    int i;

    for(i=0; i<pRet->GetNumberOfNodes(); i++)
	pDone[i] =0;
    intVector vertexStack;
    intVector parentsStack;
    for(i=0; i<pRet->GetNumberOfNodes(); i++)
    {
	int iCurrentNode = -1;
	int iCurrentParent = -1;
	if(vertexStack.empty())
	{
	    for(int j=0; j<pRet->GetNumberOfNodes(); j++)
		if(pDone[j] == 0)
		{
		    iCurrentNode = j;
		    break;
		}
	}
	else
	{
	    iCurrentNode = vertexStack.back();
	    vertexStack.pop_back();
	}
	if(!parentsStack.empty())
	{
	    iCurrentParent = parentsStack.back();
	    parentsStack.pop_back();
	}

	intVector vNeighbours;
	neighborTypeVector vNeighboursType;
	GetNeighbors(iCurrentNode, &vNeighbours, &vNeighboursType);
	//		for(int j=0; j<pRet->m_nbrsList[iCurrentNode]->size(); j++)
	//			vNeighbours.push_back((*pRet->m_nbrsList[iCurrentNode])[j]);
	intVector v;
	v.push_back(iCurrentNode);
	if(iCurrentParent != -1)
	    v.push_back(iCurrentParent);
	SetDiff(vNeighbours, v);
	if(!vNeighbours.empty())
	{
	    for(int j=0; j<vNeighbours.size(); j++)
	    {
		//				if(pRet->GetEdgeDirect(vNeighbours[j], iCurrentNode) == 1)
		pRet->RemoveEdge(vNeighbours[j], iCurrentNode);
		pRet->AddEdge(iCurrentNode, vNeighbours[j], 1);
		vertexStack.push_back(vNeighbours[j]);
		parentsStack.push_back(iCurrentNode);
	    }
	}
	pDone[iCurrentNode] =1;
    }
    delete []pDone;
    return pRet;
}

void CDAG::SetDiff(intVector& vA, const intVector &vB)
{
    intVector vNew;
    int i;

    for(i=0; i<vA.size(); i++)
	if(std::find(vB.begin(), vB.end(), vA[i]) == vB.end())
	    vNew.push_back(vA[i]);
	vA.clear();
	for(i=0; i<vNew.size(); i++)
	    vA.push_back(vNew[i]);
}
CDAG* CDAG::CreateMinimalSpanningTree(const CMatrix<float>* pWeightMatrix, const CMatrix<float>* pWeightMatrix2)
{
    assert(pWeightMatrix != NULL);
    int       numOfDims;
    const int *ranges;
    pWeightMatrix->GetRanges( &numOfDims, &ranges );

    if( ( numOfDims != 2 ) || ( ranges[0] != ranges[1] ) )
    {
        PNL_THROW( CInvalidOperation,
            " weight. mat. should be a square 2D matrix " );
    }
    if(pWeightMatrix2 != NULL)
    {
	const int* ranges2;
	int dim2;
	pWeightMatrix2->GetRanges(&dim2, &ranges2);
	assert(dim2 == numOfDims);
	assert(ranges2[0] == ranges[0]);
	assert(ranges2[1] == ranges[1]);
    }
    int iNumberOfNode = ranges[0];
    CDAG* pRet = CDAG::Create(iNumberOfNode, NULL, NULL, NULL);
    int* pIndex = new int[2];
    const float dInf = (float)1e37; //the largest number;
    EDGE minWeightEdge; // remember this is a undirected edge
    float minfWeight = dInf;
    float minfWeight2 =dInf;
    //	minWeightEdge.dWeight = dInf;
    minWeightEdge.startNode = -1;
    minWeightEdge.endNode = -1;


    EDGEVECTOR* pVertexSet = new EDGEVECTOR;
    int* pNodeAdded = new int[iNumberOfNode];
    int i;

    for(i=0; i<iNumberOfNode; i++)
	pNodeAdded[i] =0;

    for(i=0; i<iNumberOfNode; i++)
	for(int j=0; j<iNumberOfNode; j++)
	{
	    if(i == j)
		continue;
	    pIndex[0] =i;
	    pIndex[1] =j;
	    float dWeight = pWeightMatrix->GetElementByIndexes(pIndex);
	    float dWeight2 = minfWeight2;
	    if(pWeightMatrix2!=NULL)
		dWeight2 = pWeightMatrix2->GetElementByIndexes(pIndex);
	    if(dWeight != 0 && (dWeight < minfWeight || (dWeight ==  minfWeight && dWeight2 < minfWeight2 && dWeight2 != 0)))
	    {
		minfWeight = dWeight;
		minWeightEdge.startNode = i;
		minWeightEdge.endNode = j;
		minfWeight2 = dWeight2;
	    }
	}
	pNodeAdded[minWeightEdge.startNode] = 1;
	pNodeAdded[minWeightEdge.endNode] = 1;
	pRet->AddEdge(minWeightEdge.startNode, minWeightEdge.endNode, 0);
	SpanningTreeAddEdge(minWeightEdge.startNode, pNodeAdded, pVertexSet, iNumberOfNode, pWeightMatrix, pWeightMatrix2);
	SpanningTreeAddEdge(minWeightEdge.endNode, pNodeAdded, pVertexSet, iNumberOfNode, pWeightMatrix, pWeightMatrix2);
	for(i=2; i<iNumberOfNode; i++)
	{
	    minfWeight = dInf;
	    for(int j=0; j<pVertexSet->size(); j++)
	    {
		EDGE wedge;
		wedge = (*pVertexSet)[j];
		pIndex[0] = wedge.startNode;
		pIndex[1] = wedge.endNode;
		float f = pWeightMatrix->GetElementByIndexes(pIndex);
		float f2 = minfWeight2;
		if(pWeightMatrix2!= NULL)
		    f2 = pWeightMatrix2->GetElementByIndexes(pIndex);

		if(f < minfWeight || (f == minfWeight && f2 < minfWeight2 && f2 != 0))
		{
		    minWeightEdge = wedge;
		    minfWeight = f;
		    minfWeight2 = f2;
		}
	    }
	    SpanningTreeAddEdge(minWeightEdge.endNode, pNodeAdded, pVertexSet, iNumberOfNode, pWeightMatrix, pWeightMatrix2);
	    pRet->AddEdge(minWeightEdge.startNode, minWeightEdge.endNode, 0);
	}
	CDAG* pTree = pRet->RootTree();
	delete pRet;
	delete []pNodeAdded;
	delete pVertexSet;
	delete []pIndex;
	pTree->CreateAncestorMatrix();
	return pTree;
}
void CDAG::SpanningTreeAddEdge(int NodeToAdd, int* pNodeAdded, EDGEVECTOR* & pWeightEdgeSet, int iNumberOfNode, const CMatrix<float>* pWeightMatrix, const CMatrix<float>* pWeightMatrix2)
{
    int* pIndex = new int[2];
    int i;

    EDGEVECTOR* pNew = new EDGEVECTOR;
    pNodeAdded[NodeToAdd] =1;

    for(i=0; i<pWeightEdgeSet->size(); i++)
    {
	if(pNodeAdded[(*pWeightEdgeSet)[i].endNode] == 0)
	    pNew->push_back((*pWeightEdgeSet)[i]);
    }
    pWeightEdgeSet->clear();
    delete pWeightEdgeSet;
    pWeightEdgeSet = pNew;
    for(i=0; i<iNumberOfNode; i++)
    {
	if(pNodeAdded[i] == 1)
	    continue;   // startnode -> i edge
	pIndex[0] =NodeToAdd;
	pIndex[1] =i;
	float dWeight = pWeightMatrix->GetElementByIndexes(pIndex);
	if(dWeight == 0)
	    continue;
	EDGE wedge;
	wedge.startNode = NodeToAdd;
	wedge.endNode = i;
	pWeightEdgeSet->push_back(wedge);
    }
    delete []pIndex;
}

int CDAG::GetARandomNum(unsigned int nMax)
{
    if(nMax ==0)
	return 0;
    const int iNumberReseed = 100; // reseed the srand with 100 rand();
    m_iNumberOfRandomUsed ++;
    if(m_iNumberOfRandomUsed == -1) //first used
    {
	srand(time(NULL));
	m_iNumberOfRandomUsed = 0;
    }

    if(m_iNumberOfRandomUsed == iNumberReseed)
    {
	srand(time(NULL));
	m_iNumberOfRandomUsed = 0;
    }
    double fRate = rand();
    fRate = fRate/ RAND_MAX;
    int nRet = (int)(fRate * (nMax-1) + 0.5);
    return nRet;
}

CDAG* CDAG::RandomCreateADAG(int iNodeNumber, int nMaxFanIn, const intVector &vAncesstor, const intVector &vDescendants, intVector* pvNotParent, intVector* pvNotChild)
{
	if(vAncesstor.empty() || vDescendants.empty())// if one  set is empty all are valid
    {
		CDAG* pRet = CDAG::Create(iNodeNumber, NULL, NULL, NULL);
		intVector vAllNodes;
		for(int k=0; k<iNodeNumber; k++)
			vAllNodes.push_back(k);
		pRet->RandomCreateASubDAG(vAllNodes, nMaxFanIn);
		return pRet;
    }
    bool bCheck = false;
    if(pvNotParent != NULL && pvNotChild!= NULL) // if one  set is empty all are valid
		if(!pvNotParent->empty() && !pvNotChild->empty())
			bCheck = true;
		
		
		CDAG* pRet = CDAG::Create(iNodeNumber, NULL, NULL, NULL);
		pRet->CreateAncestorMatrix(); // must be all zeros
		pRet->RandomCreateASubDAG(vAncesstor, nMaxFanIn-1);
		pRet->RandomCreateASubDAG(vDescendants, nMaxFanIn-1);
		int iRandomAddLink = GetARandomNum(vAncesstor.size() * vDescendants.size());
		int i;
		
		if(iRandomAddLink == 0)
			iRandomAddLink =1;
		for(i=0; i<iRandomAddLink; i++)
		{
			int iNode1 = GetARandomNum(vAncesstor.size());
			iNode1 = vAncesstor[iNode1];
			int iNode2 = GetARandomNum(vDescendants.size());
			iNode2 = vDescendants[iNode2];
			EDGEOP eOP;
			eOP.DAGChangeType = DAG_ADD;
			int Orientation = GetARandomNum(2); 
			if(Orientation == 0)
			{
				eOP.originalEdge.startNode = iNode1;
				eOP.originalEdge.endNode = iNode2;
				if(!bCheck || pRet->CheckValid2(*pvNotParent, *pvNotChild, eOP))
				{
					pRet->DoMove(iNode1, iNode2, DAG_ADD);
					if(pRet->GetMaxFanIn() > nMaxFanIn)
					{
						pRet->DoMove(iNode1, iNode2, DAG_DEL);
					}
				}
			}
			else
			{
				eOP.originalEdge.startNode = iNode2;
				eOP.originalEdge.endNode = iNode1;
				if(!bCheck || pRet->CheckValid2(*pvNotParent, *pvNotChild, eOP))
				{
					pRet->DoMove(iNode2, iNode1, DAG_ADD);
					//	pRet->Dump();
					if(pRet->GetMaxFanIn() > nMaxFanIn)
					{
						pRet->DoMove(iNode2, iNode1, DAG_DEL);
					}
				}
			};
			
		}
		if(iNodeNumber == int(vAncesstor.size() + vDescendants.size()))
			return pRet; // all nodes linked;
		
		//link nodes that are not in vAncesstor or vDescendants;
		int * pLinked = new int[iNodeNumber];
		for(i=0; i<iNodeNumber; i++)
			pLinked[i] = 0;
		for(i=0; i<vAncesstor.size(); i++)
			pLinked[vAncesstor[i]] =1;
		for(i=0; i<vDescendants.size(); i++)
			pLinked[vDescendants[i]] =1;
		for(i=0; i<iNodeNumber; i++)
		{
			if(pLinked[i] == 0)
			{
				int j;
				do
				{
					j = GetARandomNum(iNodeNumber);
				}while(j == i && pLinked[j] == 0);
				EDGEOP eOP;
				eOP.DAGChangeType = DAG_ADD;
				int Orientation = GetARandomNum(2); 
				if(Orientation == 0)
				{
					eOP.originalEdge.startNode = j;
					eOP.originalEdge.endNode = i;
					if(!bCheck || pRet->CheckValid2(*pvNotParent, *pvNotChild, eOP))
					{
						pRet->DoMove(i, j, DAG_ADD);
						if(pRet->GetMaxFanIn() > nMaxFanIn)
						{
							pRet->DoMove(i, j, DAG_DEL);
						}
					}
				}
				else
				{
					eOP.originalEdge.startNode = i;
					eOP.originalEdge.endNode = j;
					if(!bCheck || pRet->CheckValid2(*pvNotParent, *pvNotChild, eOP))
					{
						pRet->DoMove(j, i, DAG_ADD);
						if(pRet->GetMaxFanIn() > nMaxFanIn)
						{
							pRet->DoMove(j,i,DAG_DEL);
						}
					}
				};
				/*	eOP.originalEdge.startNode = i;
				eOP.originalEdge.endNode = j;*/
				
			}
		}
		
		delete []pLinked;
		return pRet;
}

void CDAG::RandomCreateASubDAG(const intVector &vSubNodes, int nMaxFanIn, intVector* pvNotParent, intVector* pvNotChild)
{
    if(vSubNodes.empty())
	return;
    bool bCheck = false;
    if(pvNotParent != NULL && pvNotChild!= NULL) // if one  set is empty all are valid
	if(!pvNotParent->empty() && !pvNotChild->empty())
	    bCheck = true;

	int* pLinked = new int[vSubNodes.size()];
	for(int i=0; i<vSubNodes.size(); i++)
	    pLinked[i] = 0;

	int nLinkednum =0;
	while(nLinkednum < vSubNodes.size())
	{
	    int iNodeSub1 = GetARandomNum(vSubNodes.size());
	    int iNodeSub2 = GetARandomNum(vSubNodes.size());
	    if(iNodeSub1 == iNodeSub2)
		continue;
	    int iNode1 = vSubNodes[iNodeSub1];
	    int iNode2 = vSubNodes[iNodeSub2];
	    EDGEOP eOP;
	    eOP.DAGChangeType = DAG_ADD;
	    eOP.originalEdge.startNode = iNode1;
	    eOP.originalEdge.endNode = iNode2;
	    if(bCheck && !CheckValid2(*pvNotParent, *pvNotChild, eOP))
		continue;
	    if(!DoMove(iNode1, iNode2, DAG_ADD))
		continue; // not added;
	    else
	    {
		if(this->GetMaxFanIn() > nMaxFanIn)
		{
		    this->DoMove(iNode1, iNode2, DAG_DEL);
		    continue;
		}
	    }

	    if(pLinked[iNodeSub1] == 0)
	    {
		pLinked[iNodeSub1] =1;
		nLinkednum++;
	    }
	    if(pLinked[iNodeSub2] == 0)
	    {
		pLinked[iNodeSub2] =1;
		nLinkednum++;
	    }

	}
	delete []pLinked;

}

bool CDAG::CheckValid(intVector &vAncesstorVector, intVector &vDescendantsVector, EDGEOP &op)
{
    int i;
    CDAG* pDAG;
    bool b1, b2;
    b1= false;
    b2 = false;
    switch(op.DAGChangeType)
    {
    case DAG_ADD: // i->j, if j's descendants are not in AncesstorVector and i's ancesstors not in pDescendantsVector
	for(i=0; i<vAncesstorVector.size(); i++)
	    if(m_pAncesstorMatrix->GetValue(vAncesstorVector[i], op.originalEdge.endNode) || vAncesstorVector[i] == op.originalEdge.endNode)
		b1= true;
	    for(i=0; i<vDescendantsVector.size(); i++)
		if(m_pAncesstorMatrix->GetValue(op.originalEdge.startNode, vDescendantsVector[i]) ||op.originalEdge.startNode==vDescendantsVector[i] )
		    b2 = true;
		if(b1 && b2)
		    return false;
		break;
    case DAG_DEL:
	break;
    case DAG_REV:
	pDAG = Clone();
	pDAG->DoMove(op.originalEdge.startNode, op.originalEdge.endNode, DAG_DEL);
	C2DBitwiseMatrix* pMatrix = pDAG->GetAncesstorMatrix();
	for(i=0; i<vAncesstorVector.size(); i++)
	    if(pMatrix->GetValue(vAncesstorVector[i], op.originalEdge.startNode) || vAncesstorVector[i] == op.originalEdge.startNode)
	    {
		b1= true;
	    }
	    for(i=0; i<vDescendantsVector.size(); i++)
		if(pMatrix->GetValue(op.originalEdge.endNode, vDescendantsVector[i]) || op.originalEdge.endNode== vDescendantsVector[i])
		{
		    b2= true;
		}
		delete pDAG;
		if(b1 && b2)
		    return false;
		break;
    }
    return true;
}


CDAG* CDAG::GetSubDAG(intVector &vSubNodesSet)
{
    if(vSubNodesSet.size() == 0)
	return NULL;

    CDAG* pRet = CDAG::Create(vSubNodesSet.size(), NULL, NULL, NULL);
    int i;

    for(i=0; i<vSubNodesSet.size(); i++)
    {
	intVector vChildren;
	GetChildren(vSubNodesSet[i], &vChildren);
	for(int j=0; j<vChildren.size(); j++)
	{
	    int k;
	    for(k=0; k<vSubNodesSet.size(); k++)
		if(vSubNodesSet[k] == vChildren[j])
		    break;
		if(k == int(vSubNodesSet.size()))
		    continue;
		pRet->AddEdge(i, k, 1);
	}
    }
    pRet->CreateAncestorMatrix();
    return pRet;

}

bool CDAG::SetSubDag(intVector &vSubNodeSet, CDAG *pSubDAG)
{
    EDGEOPVECTOR vBakUpOPs;
    if(pSubDAG->GetNumberOfNodes() != vSubNodeSet.size())
    {
	PNL_THROW( CInvalidOperation, " factor error " );
	return false;
    }

    //remove all old edges
    int i;

    for(i=0; i<vSubNodeSet.size(); i++)
	for(int j=0; j<vSubNodeSet.size(); j++)
	{
	    if(i == j)
		continue;
	    if(GetEdgeDirect(vSubNodeSet[i], vSubNodeSet[j]) == 1)
	    {
		EDGEOP op;
		op.originalEdge.startNode =vSubNodeSet[i];
		op.originalEdge.endNode = vSubNodeSet[j];
		op.DAGChangeType = DAG_ADD;
		vBakUpOPs.push_back(op);
		DoMove(vSubNodeSet[i], vSubNodeSet[j], DAG_DEL);
	    }
	}
	for(i=0; i<vSubNodeSet.size(); i++) // add new edges
	    for(int j=0; j<vSubNodeSet.size(); j++)
	    {
		if(i == j)
		    continue;
		if(pSubDAG->GetEdgeDirect(i, j) == 1)
		{
		    EDGEOP op;
		    op.originalEdge.startNode =vSubNodeSet[i];
		    op.originalEdge.endNode = vSubNodeSet[j];
		    op.DAGChangeType = DAG_DEL;
		    if(DoMove(vSubNodeSet[i], vSubNodeSet[j], DAG_ADD))
			vBakUpOPs.push_back(op);
		    else //bak off
		    {
			for(int k=vBakUpOPs.size()-1; k<=0; k--)
			{
			    EDGEOP eop = vBakUpOPs[k];
			    DoMove(eop.originalEdge.startNode, eop.originalEdge.endNode, eop.DAGChangeType);
			}
			return false;
		    }

		}
	    }
	    return true;
}

CDAG* CDAG::TopologicalCreateDAG(intVector& vNodesMap)
{
    //	intVector vNodesMap;
    vNodesMap.clear();
    TopologicalSort(&vNodesMap);
    int * pOriginalNodeMap = new int[vNodesMap.size()];
    int i;

    for(i=0; i<vNodesMap.size(); i++)
	pOriginalNodeMap[vNodesMap[i]] = i;
    CDAG* pRet = CDAG::Create(vNodesMap.size(), NULL, NULL, NULL);
    EDGEVECTOR vEdges;
    GetAllEdges(&vEdges);
    for(i=0; i<vEdges.size(); i++)
    {
	EDGE edge = vEdges[i];
	pRet->AddEdge(pOriginalNodeMap[edge.startNode], pOriginalNodeMap[edge.endNode], 1);
    }

    delete []pOriginalNodeMap;
    return pRet;
}

bool CDAG::CheckValid2(intVector &vNotParents, intVector &vNotChild, EDGEOP op)
{
    if(op.DAGChangeType == DAG_DEL)
	return true;
    if(op.DAGChangeType == DAG_ADD)
    {
	if(IsNodeIn(op.originalEdge.startNode, vNotParents) && IsNodeIn(op.originalEdge.endNode, vNotChild))
	    return false;
	else
	    return true;
    }

    if(op.DAGChangeType == DAG_REV)
    {
	if(IsNodeIn(op.originalEdge.endNode, vNotParents) && IsNodeIn(op.originalEdge.startNode, vNotChild))
	    return false;
	else
	    return true;
    }
    return true;

}

bool CDAG::IsNodeIn(int nNode, intVector &vNodeSet)
{
    for(int i=0; i<vNodeSet.size(); i++)
	if(vNodeSet[i] == nNode)
	    return true;
	return false;
}

int CDAG::SymmetricDifference(const CDAG* pDAG) const
{
    int nNodes1 = this->GetNumberOfNodes();
    int nNodes2 = pDAG->GetNumberOfNodes();
    int NP1, NP2, samecount=0, diffcount=0;
    PNL_CHECK_FOR_NON_ZERO(nNodes1-nNodes2);
    intVector Parents1, Parents2;
    for(int i=0; i<nNodes1; i++)
    {
	this->GetParents(i, &Parents1);
	pDAG->GetParents(i, &Parents2);
	NP1 = Parents1.size();
	NP2 = Parents2.size();
	for(int j=0; j<NP1; j++)
	{
	    if(!(std::find(Parents2.begin(), Parents2.end(), Parents1[j]) == Parents2.end()))
		samecount++;
	}
	diffcount += NP1 - samecount;
	diffcount += NP2 - samecount;
	samecount = 0;
    }
    return diffcount;
}

void CDAG::GetAllValidMove(EDGEOPVECTOR *pvOutput, const int* pNotChangeNodes, int numOfNot, int nMaxFanIn, 
	intVector* pvAncesstorVector, intVector* pvDescendantsVector, intVector* pvNotParents, intVector* pvNotChild)
{
	pvOutput->clear();
	bool bCheck = false;
	if(pvAncesstorVector != NULL && pvDescendantsVector!= NULL) // if one  set is empty all are valid
		if(!pvAncesstorVector->empty() && !pvDescendantsVector->empty())
			bCheck = true;
		
	bool bCheck2 = false;
	if(pvNotParents != NULL && pvNotChild!= NULL) // if one  set is empty all are valid
		if(!pvNotParents->empty() && !pvNotChild->empty())
			bCheck2 = true;
	
	int i, j;
	for(i=0; i<GetNumberOfNodes(); i++)
	{
		if(numOfNot)
		{
			if( std::find(pNotChangeNodes, pNotChangeNodes+numOfNot, i) != pNotChangeNodes+numOfNot )
				continue;
		}
		for(j=0; j<GetNumberOfNodes(); j++)
		{
			if(numOfNot)
			{
				if( std::find(pNotChangeNodes, pNotChangeNodes+numOfNot, j) != pNotChangeNodes+numOfNot )
					continue;
			}
			if(i == j)
			{
				continue;
			}
			int nDirect = GetEdgeDirect(i, j);
			if(nDirect == 1) // try remove, change
			{
				if(IsValidMove(i, j, DAG_DEL))
				{
					EDGEOP eOP;
					eOP.originalEdge.startNode = i;
					eOP.originalEdge.endNode = j;
					eOP.DAGChangeType = DAG_DEL;
					if((!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) && (!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
						pvOutput->push_back(eOP);

				}
				if(IsValidMove(i, j, DAG_REV))
				{
					EDGEOP eOP;
					eOP.originalEdge.startNode = i;
					eOP.originalEdge.endNode = j;
					eOP.DAGChangeType = DAG_REV;
					if((!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) 
						&& (!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
					{
						if( GetNumberOfParents(i)<nMaxFanIn )
							pvOutput->push_back(eOP);
					}
				}
			}
			else if (nDirect == 0)//no edge, try to add it.
			{
				if(IsValidMove(i, j, DAG_ADD))
				{
					EDGEOP eOP;
					eOP.originalEdge.startNode = i;
					eOP.originalEdge.endNode = j;
					eOP.DAGChangeType = DAG_ADD;
					if((!bCheck || CheckValid(*pvAncesstorVector, *pvDescendantsVector, eOP)) 
						&& (!bCheck2 || CheckValid2(*pvNotParents, *pvNotChild, eOP)))
					{
						if( GetNumberOfParents(j)<nMaxFanIn )
							pvOutput->push_back(eOP);
					}
				}
			}
		}
	}
}

#ifdef PNL_RTTI
const CPNLType CDAG::m_TypeInfo = CPNLType("CDAG", &(CGraph::m_TypeInfo));

#endif

