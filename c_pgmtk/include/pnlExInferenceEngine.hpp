/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlExInferenceEngine.hpp                                    //
//                                                                         //
//  Purpose:   CExInferenceEngine class definition                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLEXINFERENCEENGINE_HPP__
#define __PNLEXINFERENCEENGINE_HPP__

#include "pnlInferenceEngine.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlImpDefs.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

enum EExInfEngineFlavour
{
    PNL_EXINFENGINEFLAVOUR_DISCONNECTED = 1 << 0,
    PNL_EXINFENGINEFLAVOUR_UNSORTED = 1 << 1,
    PNL_EXINFENGINEFLAVOUR_JTREEKLUDGE = 1 << 2,

    PNL_EXINFENGINEFLAVOUR_DISCONNECTED_UNSORTED = PNL_EXINFENGINEFLAVOUR_DISCONNECTED | PNL_EXINFENGINEFLAVOUR_UNSORTED,
    PNL_EXINFENGINEFLAVOUR_ALL = PNL_EXINFENGINEFLAVOUR_DISCONNECTED_UNSORTED | PNL_EXINFENGINEFLAVOUR_JTREEKLUDGE
};

#define PNL_IS_EXINFENGINEFLAVOUR_DISCONNECTED( flav ) ((bool)((flav) & PNL_EXINFENGINEFLAVOUR_DISCONNECTED))
#define PNL_IS_EXINFENGINEFLAVOUR_UNSORTED( flav ) ((bool)((flav) & PNL_EXINFENGINEFLAVOUR_UNSORTED))
#define PNL_IS_EXINFENGINEFLAVOUR_JTREEKLUDGE( flav ) ((bool)((flav) & PNL_EXINFENGINEFLAVOUR_JTREEKLUDGE))

template< class INF_ENGINE, class MODEL = CBNet,
          EExInfEngineFlavour FLAV = PNL_EXINFENGINEFLAVOUR_DISCONNECTED_UNSORTED,
          class FALLBACK_ENGINE1 = CNaiveInfEngine, class FALLBACK_ENGINE2 = INF_ENGINE >
class CExInfEngine: public CInfEngine
{
public:
    static CExInfEngine *Create( CStaticGraphicalModel const *model );

    inline CStaticGraphicalModel const *GetModel() const;

    inline static CNodeType const *GetObsGauNodeType();

    inline static CNodeType const *GetObsTabNodeType();

    void EnterEvidence( CEvidence const *evidence,
                        int maximize = 0,
                        int sumOnMixtureNode = 1 );
	
#ifdef PNL_OBSOLETE
    void MarginalNodes( int const *query, int querySize,
                        int notExpandJPD = 0 );

#endif

    void MarginalNodes( intVector const &queryNdsIn,
                        int notExpandJPD = 0 );

    inline CPotential const *GetQueryJPD() const;

    CEvidence const *GetMPE() const;

    ~CExInfEngine();

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CExInfEngine< CJtreeInfEngine >::GetStaticTypeInfo();
    }
#endif
protected:
    CExInfEngine( CStaticGraphicalModel const *pGM );

    void PurgeEvidences();

    EInfTypes inf_type;
    CStaticGraphicalModel const *graphical_model;

    pnlVector< CInfEngine * > engines;
    pnlVector< MODEL * > models;
    pnlVector< CGraph * > graphs;
    pnlVector< CEvidence * > evs;
    intVector orig2comp, orig2idx;
    intVecVector decomposition;
    intVector active_components;
    intVector saved_query;
    intVecVector query_dispenser;

    CEvidence const *evidence;
    bool evidence_mine;

    pnlVector< pnlVector< CNodeType > > node_types;
    intVecVector node_assoc;

    int maximize;
    mutable CPotential *query_JPD;
    mutable CEvidence *MPE_ev;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifdef PNL_RTTI
template< class INF_ENGINE, class MODEL ,
          EExInfEngineFlavour FLAV,
          class FALLBACK_ENGINE1 , class FALLBACK_ENGINE2>
const CPNLType CExInfEngine<INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2>::m_TypeInfo = CPNLType("CExInfEngine", &(CInfEngine::m_TypeInfo));

#endif

#ifndef SWIG
template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
inline CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 > *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::Create( CStaticGraphicalModel const *gm )
{
    return new( CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 > )( gm );
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
inline CStaticGraphicalModel const *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::GetModel() const
{
    return graphical_model;
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
inline CNodeType const *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::GetObsTabNodeType()
{
    return INF_ENGINE::GetObsTabNodeType();
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
inline CNodeType const *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::GetObsGauNodeType()
{
    return INF_ENGINE::GetObsGauNodeType();
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
void CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::PurgeEvidences()
{
    int i;

    if ( evidence_mine )
    {
        delete( evidence );
        evidence = 0;
        evidence_mine = false;
    }
    for ( i = evs.size(); i--; )
    {
        delete( evs[i] );
        evs[i] = 0;
    }
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::CExInfEngine( CStaticGraphicalModel const *gm )
    : CInfEngine( itEx, gm ), evidence_mine( false ),
      maximize( 0 ), MPE_ev( 0 ), query_JPD( 0 ), graphical_model( gm )
{
    int i, j, k;
    intVector dom;
    intVector conv;
    CFactor *fac;

    PNL_MAKE_LOCAL( CGraph *, gr, gm, GetGraph() );
    PNL_MAKE_LOCAL( int, sz, gr, GetNumberOfNodes() );

    gr->GetConnectivityComponents( &decomposition );

    for ( i = decomposition.size(); i--; )
    {
        std::sort( decomposition[i].begin(), decomposition[i].end() );
    }
    if ( PNL_IS_EXINFENGINEFLAVOUR_UNSORTED( FLAV ) )
    {
        gr->GetTopologicalOrder( &conv );
    }

    orig2comp.resize( sz );
    orig2idx.resize( sz );

    for ( k = 2; k--; )
    {
        for ( i = decomposition.size(); i--; )
        {
            for ( j = decomposition[i].size(); j--; )
            {
                orig2comp[decomposition[i][j]] = i;
                orig2idx[decomposition[i][j]] = j;
            }
        }

        if ( PNL_IS_EXINFENGINEFLAVOUR_UNSORTED( FLAV ) && k )
        {
            for ( i = sz; i--; )
            {
                decomposition[orig2comp[conv[i]]][orig2idx[conv[i]]] = i;
            }
        }
        else
        {
            break;
        }
    }

    graphs.resize( decomposition.size() );
    models.resize( decomposition.size() );
    engines.resize( decomposition.size() );

    for ( i = decomposition.size(); i--; )
    {
        graphs[i] = gr->ExtractSubgraph( decomposition[i] );
#if 0
        std::cout << "graph " << i << std::endl;
        graphs[i]->Dump();
#endif
    }
    node_types.resize( decomposition.size() );
    node_assoc.resize( decomposition.size() );
    for ( i = 0, k = 0; i < decomposition.size(); ++i )
    {
        node_types[i].resize( decomposition[i].size() );
        node_assoc[i].resize( decomposition[i].size() );
        for ( j = 0; j < decomposition[i].size(); ++j )
        {
            node_types[i][j] = *gm->GetNodeType( decomposition[i][j] );
            node_assoc[i][j] = j;
        }
    }
    for ( i = decomposition.size(); i--; )
    {
        models[i] = MODEL::Create( decomposition[i].size(), node_types[i], node_assoc[i], graphs[i] );
    }
    for ( i = 0; i < gm->GetNumberOfFactors(); ++i )
    {
        fac = gm->GetFactor( i );
        fac->GetDomain( &dom );
#if 0
        std::cout << "Ex received orig factor" << std::endl;
        fac->GetDistribFun()->Dump();
#endif
        k = orig2comp[dom[0]];
        for ( j = dom.size(); j--; )
        {
            dom[j] = orig2idx[dom[j]];
        }
        fac = CFactor::CopyWithNewDomain( fac, dom, models[k]->GetModelDomain() );
#if 0
        std::cout << "Ex mangled it to" << std::endl;
        fac->GetDistribFun()->Dump();
#endif
        models[k]->AttachFactor( fac );
    }
    for ( i = decomposition.size(); i--; )
    {
        switch ( decomposition[i].size() )
        {
        case 1:
            engines[i] = FALLBACK_ENGINE1::Create( models[i] );
            continue;
        case 2:
            engines[i] = FALLBACK_ENGINE2::Create( models[i] );
            continue;
        default:
            engines[i] = INF_ENGINE::Create( models[i] );
        }
    }
}


template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
void CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::EnterEvidence( CEvidence const *evidence,
                                                                                                 int maximize, int sumOnMixtureNode )
{
    int i, j, k, m;
    intVector nodes;
    pConstValueVector values;
    pConstNodeTypeVector node_types;
    intVecVector dispenser;
    valueVecVector dispenser_values;

    PurgeEvidences();
    this->evidence = evidence;
    this->maximize = maximize;

    evidence->GetObsNodesWithValues( &nodes, &values, &node_types );
    dispenser.resize( decomposition.size() );
    dispenser_values.resize( decomposition.size() );
    for ( i = 0, m = 0; i < nodes.size(); ++i )
    {
        dispenser[orig2comp[nodes[i]]].push_back( orig2idx[nodes[i]] );
        k = node_types[i]->IsDiscrete() ? 1 : node_types[i]->GetNodeSize();
        for ( j = 0; j < k; ++j )
        {
            dispenser_values[orig2comp[nodes[i]]].push_back( *values[m++] );
        }
    }
    evs.resize( decomposition.size() );
    for ( i = decomposition.size(); i--; )
    {
        evs[i] = 0;
        evs[i] = CEvidence::Create( models[i], dispenser[i], dispenser_values[i] );
        engines[i]->EnterEvidence( evs[i], maximize, sumOnMixtureNode );
    }
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::~CExInfEngine()
{
    int i;

    delete( MPE_ev );
    delete( query_JPD );

    PurgeEvidences();
    for ( i = engines.size(); i--; )
    {
        delete( engines[i] );
    }
    for ( i = models.size(); i--; )
    {
        delete( models[i] );
    }
}

// current implementation works not very inefficiently when evidence resides
// only on small fraction of connectivity components of the model

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
void CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::MarginalNodes( int const *query, int querySize, int notExpandJPD )
{
    int i, j;

    if( querySize== 0 )
    {
        PNL_THROW( CInconsistentSize, "query nodes vector should not be empty" );
    }

    saved_query.assign( query, query + querySize );

    if ( evidence == 0 )
    {
        evidence_mine = true;
        EnterEvidence( evidence = CEvidence::Create( graphical_model, intVector(), valueVector() ) );
    }

    active_components.resize( 0 );

    query_dispenser.resize( 0 );
    query_dispenser.resize( decomposition.size() );

    for ( i = 0; i < querySize; ++i )
    {
        query_dispenser[orig2comp[query[i]]].push_back( orig2idx[query[i]] );
    }

    for ( i = decomposition.size(); i--; )
    {
        if ( query_dispenser[i].size() )
        {
            active_components.push_back( i );
        }
    }

	EDistributionType dt=dtTabular;
	bool determined = false;
	
	for ( i = active_components.size(); i--; )
    {
        j = active_components[i];
		engines[j] -> MarginalNodes( &query_dispenser[j].front(), query_dispenser[j].size(), 1 );
		if(!determined)
		{
			dt = engines[j]->GetQueryJPD()->GetDistributionType();
			if(dt != dtScalar)
				determined = true;
		}
    }
	if(!(dt == dtTabular || dt == dtGaussian || dt == dtScalar))
		PNL_THROW(CNotImplemented, "we can not support this type of potentials");

    CPotential *pot1;
    CPotential const *pot2=NULL;
    intVector dom;
	intVector obsIndices;
	for(i=0; i<querySize; i++)
	{
		if(evidence->IsNodeObserved(query[i]))
		{
			obsIndices.push_back(i);
		}
	}

	if(query_JPD)
		delete query_JPD;
	if ( active_components.size() > 1 )
    {
        if((dt == dtTabular) || (dt == dtScalar))
		{
			query_JPD = CTabularPotential::CreateUnitFunctionDistribution( saved_query, graphical_model->GetModelDomain(), 1, obsIndices );
		}
		else if(dt == dtGaussian)
        {
			query_JPD = CGaussianPotential::CreateUnitFunctionDistribution( saved_query, graphical_model->GetModelDomain(), 1, obsIndices );
		}

//        query_JPD = pot2->ShrinkObservedNodes( evidence );
//        delete( pot2 );

        for ( i = active_components.size(); i--; )
        {
            pot2 = engines[active_components[i]]->GetQueryJPD();
            pot2->GetDomain( &dom );
            for ( j = dom.size(); j--; )
            {
                dom[j] = decomposition[active_components[i]][dom[j]];
            }
            pot1 = (CPotential *)CPotential::CopyWithNewDomain( pot2, dom, graphical_model->GetModelDomain() );
            *query_JPD *= *pot1;
            delete( pot1 );
        }
    }
    else
    {
        pot2 = engines[active_components[0]]->GetQueryJPD();
        pot2->GetDomain( &dom );

        for ( j = dom.size(); j--; )
        {
           dom[j] = decomposition[active_components[0]][dom[j]];
        }
        query_JPD = (CPotential *)CPotential::CopyWithNewDomain( pot2, dom, graphical_model->GetModelDomain() );
    }
	query_JPD->Normalize();
}

// current implementation works inefficiently when evidence resides
// only on small fraction of connectivity components of the model
template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
void CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::MarginalNodes( intVector const &queryNds, int notExpandJPD )
{
    MarginalNodes(&queryNds.front(), queryNds.size(), notExpandJPD );
}

// current implementation works inefficiently when evidence resides
// only on small fraction of connectivity components of the model
template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
CEvidence const *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::GetMPE() const
{
    int i, j, k, s;
    intVector conv_nodes;
    valueVector conv_values;

    intVector nodes;
    pConstValueVector values;
    pConstNodeTypeVector node_types;

    CEvidence const *ev;

    if ( PNL_IS_EXINFENGINEFLAVOUR_JTREEKLUDGE( FLAV ) )
    {
        intVecVector clqs;
        intVector dummy;
        intVector mask;
        intVector tmpdom;
        int clqs_lim, coun;
        int best_clq, best_val;
        intVecVector partition;
        intVector partition_clq;

        dummy.resize( 1 );

        for ( i = active_components.size(); i--; )
        {
            intVector tmpclq;
            intVector tmpmask;

            k = active_components[i];
            if ( engines[k]->m_InfType != itJtree )
            {
                ev = engines[k]->GetMPE();
                ev->GetObsNodesWithValues( &nodes, &values, &node_types );
                for ( j = nodes.size(); j--; )
                {
                    conv_nodes.push_back( decomposition[k][nodes[j]] );
                    conv_values.push_back( Value( *values[j] ) );
                }
                continue;
            }
            clqs.resize( 0 );
            clqs.resize( query_dispenser[k].size() );

            tmpmask.resize( 0 );
            tmpmask.assign( decomposition[k].size(), 0 );

            for ( j = query_dispenser[k].size(), clqs_lim = 0; j--; )
            {
                tmpmask[query_dispenser[k][j]] = j + 1;
                dummy[0] = query_dispenser[k][j];
                ((CJtreeInfEngine *)engines[k])->GetClqNumsContainingSubset( dummy, &clqs[j] );

                for ( s = clqs[j].size(); s--; )
                {
                    if ( clqs[j][s] > clqs_lim )
                    {
                        clqs_lim = clqs[j][s];
                    }
                }
            }

            mask.resize( 0 );
            mask.assign( ++clqs_lim, 0 );

            for ( j = query_dispenser[k].size(); j--; )
            {
                for ( s = clqs[j].size(); s--; )
                {
                    ++mask[clqs[j][s]];
                }
            }

            partition.resize( 0 );
            partition_clq.resize( 0 );

            for ( coun = query_dispenser[k].size(); coun; )
            {
                best_clq = clqs_lim;
                best_val = 0;
                for ( j = clqs_lim; j--; )
                {
                    if ( mask[j] > best_val )
                    {
                        best_val = mask[best_clq = j];
                    }
                }

                if ( best_val == query_dispenser[k].size() )
                {
                    // query fits into single clique.  No workaround needed for this component.
                    engines[k]->MarginalNodes( &query_dispenser[k].front(), query_dispenser[k].size() );
                    j = 0; goto brk;
                }

                coun -= best_val;

                partition_clq.push_back( best_clq );
                partition.push_back( intVector() );
                tmpclq.resize( 0 );
                ((CJtreeInfEngine *)engines[k])->GetJTreeNodeContent( best_clq, &tmpclq );

                for ( j = tmpclq.size(); j--; )
                {
                    if ( tmpmask[tmpclq[j]] > 0 )
                    {
                        partition.back().push_back( tmpclq[j] );
                        for ( s = clqs[tmpmask[tmpclq[j]] - 1].size(); s--; )
                        {
                            --mask[clqs[tmpmask[tmpclq[j]] - 1][s]];
                        }
                        tmpmask[tmpclq[j]] = - tmpmask[tmpclq[j]];
                    }
                }
            }

            for ( j = partition.size(); j--; )
            {
                engines[k]->MarginalNodes( &partition[j].front(), partition[j].size() );
brk:
                ev = engines[k]->GetMPE();
                ev->GetObsNodesWithValues( &nodes, &values, &node_types );
                for ( s = nodes.size(); s--; )
                {
                    conv_nodes.push_back( decomposition[k][nodes[s]] );
                    conv_values.push_back( Value( *values[s] ) );
                }
            }
        }
    }
    else
    {
        for ( i = active_components.size(); i--; )
        {
            ev = engines[active_components[i]]->GetMPE();
            ev->GetObsNodesWithValues( &nodes, &values, &node_types );
            for ( j = nodes.size(); j--; )
            {
                conv_nodes.push_back( decomposition[active_components[i]][nodes[j]] );
                conv_values.push_back( Value( *values[j] ) );
            }
        }
    }

    return MPE_ev = CEvidence::Create( graphical_model, conv_nodes, conv_values );
}

template< class INF_ENGINE, class MODEL, EExInfEngineFlavour FLAV, class FALLBACK_ENGINE1, class FALLBACK_ENGINE2 >
CPotential const *CExInfEngine< INF_ENGINE, MODEL, FLAV, FALLBACK_ENGINE1, FALLBACK_ENGINE2 >::GetQueryJPD() const
{
    return query_JPD;
}

#endif

PNL_END

#endif // __PNLINFERENCEENGINE_HPP__
