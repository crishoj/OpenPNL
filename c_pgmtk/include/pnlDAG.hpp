// pnlDAG.h: interface for the CDAG class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PNLDAG_H__
#define __PNLDAG_H__

#include "pnlGraph.hpp"
#include "pnl2DBitwiseMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
#define _PNL_DEBUG_
typedef pnlVector< void* > POINTVECTOR;
typedef PNL_API enum
{
	DAG_ADD,
	DAG_DEL,
	DAG_REV
} EDAGChangeType;
typedef struct EDGE
{
	int startNode;
	int endNode;
#ifdef PNL_VC7
	bool operator<(const EDGE &nt) const
	{
	    return startNode < nt.startNode;
	}
	bool operator==(const EDGE &nt) const
	{
	    return startNode == nt.startNode;
	}
#endif
}EDGE;

typedef struct EDGEOP
{
	EDGE originalEdge;
	EDAGChangeType DAGChangeType;
#ifdef PNL_VC7
	bool operator<(const EDGEOP &nt) const
	{
	    return DAGChangeType < nt.DAGChangeType;
	}
	bool operator==(const EDGEOP &nt) const
	{
	    return DAGChangeType == nt.DAGChangeType;
	}
#endif
}EDGEOP;

typedef pnlVector< EDGE > EDGEVECTOR;
typedef pnlVector< EDGEOP > EDGEOPVECTOR;

class PNL_API CDAG : public CGraph 
{
private:
	bool IsNodeIn(int nNode, intVector & vNodeSet);
	void RandomCreateASubDAG(const intVector& vSubNodes, int nMaxFanIn, intVector* pvNotParent= NULL, intVector* pvNotChild=NULL);
	CDAG* RootTree();
	static void SpanningTreeAddEdge(int NodeToAdd, int* pNodeAdded, EDGEVECTOR* &pWeightEdgeSet, int iNumberOfNode, const CMatrix<float>* pWeightMatrix, const CMatrix<float>* pWeightMatrix2);
	void UpdateAncestorMatrix(int iStartNode, int iEndNode, EDAGChangeType iChangeType); //update ancestor matrix
	void SetAncesstorMatrix(C2DBitwiseMatrix* pMatrix);
	void UpdateRemoveNodes(int iNodeNumber);
	C2DBitwiseMatrix* m_pAncesstorMatrix;
	void SetDiff(intVector& vA, const intVector &vB); // A = A/B;
	static int m_iNumberOfRandomUsed;
	bool CheckValid2(intVector &vNotParents, intVector &vNotChild, EDGEOP op);
	bool CheckValid(intVector &vAncesstorVector, intVector &vDescendantsVector, EDGEOP &op);
	static int GetARandomNum(unsigned int nMax);
	
public:
#ifdef _PNL_DEBUG_
	void DumpMatrix();
#endif
	//Construct Functions

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CDAG::m_TypeInfo;
  }
#endif
protected:
	CDAG( int numOfNds, const int* numOfNbrs, const int *const *nbrsList,
		    const ENeighborType *const *nbrsTypes );
	CDAG( int numOfNds, const int *const *adjMat );
	CDAG(const CMatrix<int>* pAdjMat);
	CDAG(const CGraph& rGraph);
//	CDAG(const CGraph& rDag);
	
#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif
public:
	int GetEdgeDirect(int startNode, int endNode);
	CDAG* TopologicalCreateDAG(intVector& vNodesMap);// vNodesMap[i] = original node number in original dag
	bool SetSubDag(intVector &vSubNodeSet, CDAG* pSubDAG);
	CDAG* GetSubDAG(intVector &vSubNodesSet); // get a sub dag of vSubNodesSet, node order is the same as vSubNodesSet;
	void CreateAncestorMatrix();
	static CDAG* RandomCreateADAG(int iNodeNumber, int nMaxFanIn, const intVector& vAncesstor, const intVector& vDescendants, intVector* pvNotParent= NULL, intVector* pvNotChild=NULL);
	static CDAG* CreateMinimalSpanningTree(const CMatrix<float>* pWeightMatrix, const CMatrix<float>* pWeightMatrix2);
	bool IsValidMove(int iStartNode, int iEndNode, EDAGChangeType iChangeType);
	void GetAllValidMove(EDGEOPVECTOR* pvOutput, intVector* pvAncesstorVector=NULL, intVector* pvDescendantsVector=NULL, intVector* pvNotParents=NULL, intVector* pvNotChild=NULL);
	void GetAllValidMove(EDGEOPVECTOR* pvOutput, const int* pNotChangeNodes, int numOfNot, int nMaxFanIn, 
		intVector* pvAncesstorVector=NULL, intVector* pvDescendantsVector=NULL, intVector* pvNotParents=NULL, intVector* pvNotChild=NULL);

	bool DoMove(int iStartNode, int iEndNode, EDAGChangeType iChangeType); // return false if a invalid move applyed
	void GetAllEdges(EDGEVECTOR* pvOutput);
	bool IsEquivalent(CDAG* pDAG1);  
	void MarkovBlanket(int iNodeNumber, intVector* pvOutPut);
	bool TopologicalSort(intVector* pvOutput);
	int GetMaxFanIn();
	C2DBitwiseMatrix* GetAncesstorMatrix();
	CDAG* Clone();
	int SymmetricDifference(const CDAG* pDAG) const;

#ifdef PNL_OBSOLETE
	static CDAG* Create( int numOfNds, const int *numOfNbrsIn,
		                   const int *const *nbrsListIn,
						   const ENeighborType *const *nbrsTypesIn  );
#endif
#ifdef PNL_OBSOLETE
	static CDAG* Create( int numOfNds, const int *const *adjMatIn );
#endif
	static CDAG* Create(const CMatrix<int>* pAdjMat);
	static CDAG* Create(const CGraph& pGraph);

	void ClearContent();
	virtual ~CDAG();

	CDAG* Change(int iStartNode, int iEndNode, EDAGChangeType iChangeType); //change to a neighbour dag
	void GetAllNeighbours(POINTVECTOR &vNeighbours, EDGEOPVECTOR &vOPs);
	void GetAllNeighbours(POINTVECTOR &vNeighbours, EDGEOPVECTOR &vOPs, bool (*IsValid)(CDAG* pDAG)); //get all neighbour dags that verify function IsValid
};
//static int CDAG::m_iNumberOfRandomUsed = 0;
PNL_END
#endif // __PNLDAG_H__
