/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGraphicalModel.hpp                                       //
//                                                                         //
//  Purpose:   CGraphicalModel class definition                            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLGRAPHICALMODEL_HPP__
#define __PNLGRAPHICALMODEL_HPP__

#include "pnlObject.hpp"
#include "pnlModelTypes.hpp"
#include "pnlNodeType.hpp"
#include "pnlModelDomain.hpp"


#include "pnlTypeDefs.hpp"
#include "pnlFactors.hpp"
#include "pnlGraph.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#define AllocParameter AllocFactor
#define AllocParameters AllocFactors
#define AttachParameter AttachFactor
#define GetNumberOfParameters GetNumberOfFactors

#ifdef SWIG
%rename(AllocFactorByDomainNumber) CGraphicalModel::AllocFactor(int);
%rename(GetFactorsIntoVector) CGraphicalModel::GetFactors(const intVector& , pFactorVector *) const;
#endif

/* base class for all Tk objects */
class PNL_API CGraphicalModel : public CPNLBase 
{
public:

    virtual ~CGraphicalModel();
	
    // returns the number of nodes in the model
    // for Dynamic Graphical Model it is number of nodes per slice
    virtual int GetNumberOfNodes() const = 0;

	// returns node type of the node with a number == nodeNumber 
    virtual const CNodeType* GetNodeType( int nodeNumber ) const = 0;

    // returns number of node types of the model 
    virtual int GetNumberOfNodeTypes() const = 0;
    
#ifdef PNL_OBSOLETE
    virtual void GetNodeTypes(nodeTypeVector* nodeTypesOut);
#endif

	virtual void GetNodeTypes(pConstNodeTypeVector *nodeTypesOut);
    
    
	// returns the pointer to the array of node associations ot the types 
	
#ifdef PNL_OBSOLETE
	virtual const int *GetNodeAssociations() const = 0;
#endif
	
	// function returns the number of factors of the model 
    virtual int GetNumberOfFactors() const = 0;

	// allocates enough memory for all the factors storing 
	virtual CFactors* AttachFactors( CFactors *paramsinIn ) = 0;

	// returns the factor if the number of domain is already known 
	virtual CFactor* GetFactor(int domainNodes) const = 0;
	
	// allocates enough memory for all the factors storing
	virtual void AllocFactors() = 0;

#ifdef PNL_OBSOLETE
	// the function to set the factor on the domain 
	virtual void AllocFactor( int numberOfNodesInDomain, const int *domainIn ) = 0;
#endif

	virtual void AllocFactor( const intVector& domainIn);
    
	// the function to set the factor on the domain, the number input 
	// factor is the number of the child node in BNet model 
	virtual void AllocFactor( int number ) = 0;
	
	// functions to work with each factor separately to attach the input
    // factor to the model and store it in m_pParams
	virtual void AttachFactor(CFactor *paramIn) = 0;
	
#ifdef PNL_OBSOLETE
	// functions returns all the factors which are set on the domains
	// which contain the nodes as a subdomain 
	virtual void GetFactors( int numberOfNodes, const int* nodesIn,
							 int *numberOfFactorsOut, 
							 CFactor ***paramsOut ) const = 0;
#endif

#ifdef PNL_OBSOLETE	
	// function returns all the factors which are set on the domains
	// which contain the nodes as a subdomain 
	virtual int GetFactors( int numberOfNodes, const int* nodesIn,
							pFactorVector *paramsOut) const = 0;
#endif

	virtual int GetFactors( const intVector& subdomainIn,
		                    pFactorVector *paramsOut ) const;
    
	virtual CGraph* GetGraph() const = 0;
   
    inline EModelTypes GetModelType() const;

    inline CModelDomain* GetModelDomain()const;

    virtual bool IsValid( std::string* descriptionOut = NULL ) const = 0;

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CGraphicalModel::m_TypeInfo;
    }
#endif
protected:
	
    CGraphicalModel(CModelDomain* pMD);

    CGraphicalModel( int numberOfNodes, int numberOfNodeTypes,
		      const CNodeType *nodeTypes, const int *nodeAssociation );

	/* a constant, which specifies the type of the model */
	EModelTypes m_modelType;
    
    //keep the pointer to Model Domain with NodeTypes etc.
    CModelDomain *m_pMD;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    

};

#ifndef SWIG

inline EModelTypes CGraphicalModel::GetModelType() const
{
	return m_modelType;
}

inline CModelDomain* CGraphicalModel::GetModelDomain() const
{
    return m_pMD;
}

#endif

PNL_END

#endif // __PNLGRAPHICALMODEL_HPP__

