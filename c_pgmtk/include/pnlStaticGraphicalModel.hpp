/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticGraphicalModel.hpp                                 //
//                                                                         //
//  Purpose:   CStaticGraphicalModel class definition                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLSTATICGRAPHICALMODEL_HPP__
#define __PNLSTATICGRAPHICALMODEL_HPP__

#include "pnlGraphicalModel.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlFactors.hpp"
#include "pnlGraph.hpp"
#include "pnlNodeType.hpp"
#include "pnlException.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(AllocFactorByDomainNumber) CStaticGraphicalModel::AllocFactor(int);
#endif

class PNL_API CStaticGraphicalModel : public CGraphicalModel
{
public:
	
	/* returns the number of nodes in the model */
	inline virtual int GetNumberOfNodes() const;
	
	/* returns the graph structure of the model */
	inline CGraph* GetGraph() const;
	
	/* returns number of node types of the model */
	virtual inline int GetNumberOfNodeTypes() const;

    /* returns node type of the node with a number == nodeNumber */
	inline const CNodeType* GetNodeType(int nodeNumber) const;
	
	/* returns the pointer to the array of node associations ot the types */
#ifndef SWIG
	virtual inline const int* GetNodeAssociations() const;
#endif
	/* function returns the number of factors of the model */
	virtual inline int GetNumberOfFactors() const;
	
	/* attaches factors and returns the ones that were attached
	beforehand (will have to destroy it) */
	virtual CFactors* AttachFactors( CFactors *paramsIn );
	
	/* returns the factor if the number of domain is already known */
	virtual CFactor* GetFactor(int domainNum) const;
	
	/* the following are virtual functions which work differently in cases of
	Markov, Bayes Nets and chaingraphs: */
	
	/* allocates enough memory for all the factors storing */
	virtual void AllocFactors() = 0;
	
#ifndef SWIG
	/* the function to set the factor on the domain */
	virtual void AllocFactor( int numberOfNodesInDomain, const int *domainIn ) = 0;
#endif

    /* the function to set the factor on the domain, the number input 
	parameter is the number of the child node in BNet model and the number of
	the clique in the MNet model */
	virtual void AllocFactor(int number) = 0;
	
	/* functions to work with each factor separately to attach the input
	parameter to the model and store it in m_pParams*/
	virtual void AttachFactor(CFactor *paramIn) = 0;

#ifndef SWIG
	/* function returns all the factors which are set on the domains
	which contain the nodes as a subdomain */
	virtual void GetFactors( int numberOfNodes, const int* nodesIn,
		                     int *numberOfFactorsOut,
		                     CFactor ***paramsOut ) const = 0;
#endif
	
#ifndef SWIG
	/* function returns all the factors which are set on the domains
	which contain the nodes as a subdomain */
	virtual int GetFactors( int numberOfNodes, const int* nodesIn,
		                    pFactorVector *paramsOut ) const = 0;
#endif
	bool IsValid( std::string* descriptionOut = NULL ) const = 0;

        bool IsValidAsBaseForDynamicModel(std::string* descriptionOut = NULL) const;

	virtual ~CStaticGraphicalModel();
	
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CStaticGraphicalModel::m_TypeInfo;
  }
#endif
protected:
	
	CStaticGraphicalModel( CGraph *pGraph, CModelDomain* pMD );

    CStaticGraphicalModel( CGraph* pGraph, int numberOfNodes,
                int numberOfNodeTypes,
		        const CNodeType *nodeTypes, const int *nodeAssociation );

	/* need to have a default constructor so that JTree can be constructed */
	CStaticGraphicalModel(CModelDomain* pMD);
	
	int _AllocFactor( int domainNodes, int numberOfNodesInDomain,
		                 const int *domain, EFactorType paramType );

    //store the pointers to Factors of the Model
    //pFactorVector m_pFactors;

    int m_numberOfNodes;
    
	/* a pointer to a Graph class object, attached to the model */
	CGraph *m_pGraph;
	
	/* a pointer to a set of factors, attached to the model */
	CFactors *m_pParams;

	// stores the pointers to the factors which are set on the domains
	//which have the query nodes in them 
	mutable pFactorVector m_paramsForNodes;

	// domain (vector<int>) is a key and index (int) is the value 
	intVector m_paramInds;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:

};

#ifndef SWIG

inline int CStaticGraphicalModel::GetNumberOfNodes() const
{
	return m_pMD->GetNumberVariables();
}

inline CGraph* CStaticGraphicalModel::GetGraph() const
{
	return m_pGraph;
}

inline int CStaticGraphicalModel::GetNumberOfNodeTypes() const
{
	return m_pMD->GetNumberOfVariableTypes();
}


inline const CNodeType* 
CStaticGraphicalModel::GetNodeType(int nodeNumber) const
{	
	return m_pMD->GetVariableType(nodeNumber);
}

inline const int* CStaticGraphicalModel::GetNodeAssociations() const
{
	return m_pMD->GetVariableAssociations();
}

inline int CStaticGraphicalModel::GetNumberOfFactors() const
{
	return m_pParams->GetNumberOfFactors();
}

#endif

PNL_END

#endif //__PNLSTATICGRAPHICALMODEL_HPP__
