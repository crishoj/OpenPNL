/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicGraphicalModel.hpp                                //
//                                                                         //
//  Purpose:   Base class definition  for all Dynamic graphical            //
//             models                                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLDYNAMICGRAPHICALMODEL_HPP__
#define __PNLDYNAMICGRAPHICALMODEL_HPP__

#include "pnlGraphicalModel.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlFactors.hpp"
#include "pnlGraph.hpp"
#include "pnlNodeType.hpp"
#include "pnlException.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlGraphicalModel.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN


#ifdef SWIG
%rename(AllocFactorByDomainNumber) CStaticGraphicalModel::AllocFactor(int);
#endif
class PNL_API CDynamicGraphicalModel : public CGraphicalModel
{
public:
	
	virtual CStaticGraphicalModel *CreatePriorSliceGrModel()const = 0 ;
	// destructor of the graphical model 
	virtual ~CDynamicGraphicalModel();
	//return number of nodes per slice
	inline int GetNumberOfNodes() const;
	inline CGraph* GetGraph() const;
	virtual inline int GetNumberOfFactors() const;
	
	// unrolling dynamic model for number of slices = numberOfSlices
	virtual CStaticGraphicalModel* UnrollDynamicModel(int numberOfSlices) const = 0;

#ifdef PNL_OBSOLETE
    // return interface nodes
    inline void GetInterfaceNodes(int *numberOfInterfaceNodesOut,
        const int **interfaceNodesOut ) const;
#endif
    inline void GetInterfaceNodes(intVector* interfaceNodesOut) const;
    //Returns pointer to the static graphcal model used for creation of the DBN
    inline CStaticGraphicalModel *GetStaticModel() const;


    // returns number of node types of the model
    virtual inline int GetNumberOfNodeTypes() const;

    // returns node type of the node with a number == nodeNumber
    // <= number of nodes per slice
    inline const CNodeType* GetNodeType( int nodeNumber ) const;

    // returns the pointer to the array of node associations ot the types
#ifndef SWIG
    virtual inline const int *GetNodeAssociations() const;
#endif

    // attaches factors and returns the ones that were attached
    // beforehand (will have to destroy it)
    virtual CFactors* AttachFactors( CFactors *paramsIn );

    // returns the factor if the number of domain is already known
    virtual CFactor* GetFactor(int domainNodes) const;


    // allocates enough memory for all the factors storing
    virtual void AllocFactors();

    // the function to set the factor on the domain
#ifdef PNL_OBSOLETE
    virtual void AllocFactor( int numberOfNodesInDomain,const int *domainIn );
#endif
    // the function to set the factor on the domain, the number input
    // factor is the number of the child node in BNet model
    virtual void AllocFactor( int number );

    // functions to work with each factor separately to attach the input
    // factor to the model and store it in m_pParams
    virtual void AttachFactor(CFactor *paramIn);

    // functions returns all the factors which are set on the domains
    // which contain the nodes as a subdomain
#ifdef PNL_OBSOLETE
    virtual void GetFactors( int numberOfNodes, const int* nodesIn,
		int *numberOfFactorsOut,
                CFactor ***paramsOut ) const;
    virtual int GetFactors( int numberOfNodes, const int* nodesIn,
                pFactorVector *paramsOut) const;
#endif    
	
    bool IsValid(std::string* descriptionOut = NULL) const;

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDynamicGraphicalModel::m_TypeInfo;
    }
#endif
protected:

    // constructor of the Dynamic Graphical Model object
    CDynamicGraphicalModel( EModelTypes modelType,
	CStaticGraphicalModel *pGrModel);

    const CStaticGraphicalModel *GrModel() const { return m_pGrModel; }
    int m_nnodesPerSlice;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    void FindInterfaceNodes();
    CStaticGraphicalModel *m_pGrModel;
    intVector m_InterfaceNodes;
};

#ifndef SWIG
inline int CDynamicGraphicalModel::GetNumberOfNodes() const
{
    return m_nnodesPerSlice;
}

inline CGraph* CDynamicGraphicalModel::GetGraph() const
{
    return m_pGrModel->GetGraph();
}

 inline int CDynamicGraphicalModel::GetNumberOfFactors() const
{
    return m_pGrModel->GetNumberOfFactors();
}

inline void CDynamicGraphicalModel::
GetInterfaceNodes(int *numberOfInterfaceNodes, const int **interfaceNodes ) const
{
    // bad-args check
    if( !numberOfInterfaceNodes )
    {
	PNL_THROW( CNULLPointer,
	    " pointer to numberOfNodes cannot be NULL" );
    }

    if( !interfaceNodes )
    {
	PNL_THROW( CNULLPointer,
	    " pointer to nodeAssignment cannot be NULL" );
    }
    // bad-args check end

    *numberOfInterfaceNodes = m_InterfaceNodes.size();
    *interfaceNodes = &m_InterfaceNodes.front();
}

inline void CDynamicGraphicalModel::
GetInterfaceNodes(intVector* interfaceNodesOut) const
{
    PNL_CHECK_IS_NULL_POINTER(interfaceNodesOut);
    int numberOfInterfaceNodes;
    const int *interfaceNodes;
    GetInterfaceNodes(&numberOfInterfaceNodes, &interfaceNodes );
    (*interfaceNodesOut).assign(interfaceNodes,
        interfaceNodes + numberOfInterfaceNodes);
}

inline CStaticGraphicalModel *CDynamicGraphicalModel::GetStaticModel() const
{
    return m_pGrModel;
}


inline int CDynamicGraphicalModel::GetNumberOfNodeTypes() const
{
    return (m_pGrModel->GetNumberOfNodeTypes());
}

/*inline void CDynamicGraphicalModel::GetNodeTypes( int *numberOfNodeTypes,
const CNodeType **nodeTypes) const
{
    m_pGrModel->GetNodeTypes(numberOfNodeTypes, nodeTypes);
}
*/
inline const CNodeType*
CDynamicGraphicalModel::GetNodeType( int nodeNumber ) const
{
    /* bad-args check */
    if( ( nodeNumber < 0 ) || ( nodeNumber > 2*m_nnodesPerSlice - 1 ) )
    {
	PNL_THROW( COutOfRange,
	    " node uumber < 0 || node number >= number of nodes per slice" );
    }
    /* bad-args check end */
    return (m_pGrModel->GetNodeType(nodeNumber));
}

inline const int *CDynamicGraphicalModel::GetNodeAssociations() const
{
    return (m_pGrModel->GetNodeAssociations());
}


#endif

PNL_END

#endif //__PNLDINAMICGRAPHICALMODEL_HPP__
