#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "WInner.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WCliques.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

WDistributions::WDistributions(TokenCover *pToken): m_pToken(pToken), m_bMRF(false)
{
    SpyTo(&Token());
    SpyTo(&Graph());
}

WDistributions::~WDistributions()
{
    if(IsMRF())
    {
	delete m_pCliques;
    }
}

void WDistributions::SetMRF(bool mrfFlag /* = true */)
{
    m_bMRF = mrfFlag;
    if(mrfFlag)
    {
	StopSpyTo(Token().Graph());
	m_pCliques = new WCliques(Token().Graph());
	SpyTo(m_pCliques);
    }
    else
    {
	ThrowInternalError("Not yet realized", "SetMRF(false)");
    }
}

void WDistributions::Setup(int iNode)
{
    TokArr ta(Tok(this->Token().Node(iNode)));
    int nodeClass = Token().NodesClassification(ta);
    if(iNode >= m_aDistribution.size())
    {
	m_aDistribution.resize(iNode + 1, 0);
	m_abDiscrete.resize(iNode + 1, nodeClass == eNodeClassDiscrete );
    }
    delete m_aDistribution[iNode];

    Vector<int> aParent;
    Graph().GetParents(&aParent, iNode);
    Vector<TokIdNode*> parTokId = Token().Nodes(aParent);

    if (nodeClass == eNodeClassDiscrete )
    {
	bool isSoftMax = false;
        int i;
	for (i=0; i< parTokId.size(); i++)
	{
	    TokIdNode *tok = parTokId[i];
	    if (!static_cast<pnl::CNodeType*>(tok->v_prev->data)->IsDiscrete() )
	    {
		isSoftMax = true;
		break;
	    }
	}
	bool isCondSoftMax = false;

	if (isSoftMax)
	{
	    for (i=0; i< parTokId.size(); i++)
	    {
		TokIdNode *tok = parTokId[i];
		if (static_cast<pnl::CNodeType*>(tok->v_prev->data)->IsDiscrete() )
		{
		    isCondSoftMax = true;
		    break;
		}
	    }
	}
	if (isCondSoftMax)
	    m_aDistribution[iNode] = new WCondSoftMaxDistribFun();
	else
	    if (isSoftMax)
		m_aDistribution[iNode] = new WSoftMaxDistribFun();
	    else
		m_aDistribution[iNode] = new WTabularDistribFun();            
    }

    else
    {
	if (nodeClass == eNodeClassContinuous)
	{
	    bool isCondGauss = false;
	    int size = parTokId.size();
	    for (int index = 0; (index < size)&&(!isCondGauss); index++)
	    {
		TokIdNode *tokIdNode = parTokId[index];
		isCondGauss = isCondGauss||(static_cast<pnl::CNodeType*>(tokIdNode->v_prev->data)->IsDiscrete());
	    };

	    if (isCondGauss)
	    {
		m_aDistribution[iNode] = new WCondGaussianDistribFun();
	    }
	    else
	    {
		m_aDistribution[iNode] = new WGaussianDistribFun();
	    };
	}
	else
	{
	    ThrowUsingError("Unknown type", "Setup");
	}
    }


    /*    Graph().GetParents(&aParent, iNode);
    Vector<TokIdNode*> parTokId = Token().Nodes(aParent);
    */
    m_aDistribution[iNode]->Setup(Token().Node(iNode), parTokId);
    TokIdNode *tok = Token().Node(iNode);
    if (nodeClass == eNodeClassDiscrete )
    {
	if (static_cast<pnl::CNodeType*>(tok->v_prev->data)->GetNodeState() != pnl::nsValue)
	{
	    m_aDistribution[iNode]->SetDefaultDistribution();
	}
	else
	{
	    static_cast<WTabularDistribFun*>(m_aDistribution[iNode])->SetDefaultUtilityFunction();
	}
    }
    else
    {
	m_aDistribution[iNode]->SetDefaultDistribution();
    }
}

void WDistributions::SetupNew(int iDistribution)
{
    pnl::CNodeType nt = DistributionType(iDistribution);

    if(iDistribution >= m_aDistribution.size())
    {
	m_aDistribution.resize(iDistribution + 1, 0);
	m_abDiscrete.resize(iDistribution + 1, true);
	m_abValid.resize(iDistribution + 1, false);
    }
    else
    {
	delete m_aDistribution[iDistribution];
    }

    Vector<int> domain;
    WDistribFun *pDistribution;

    GetDomain(&domain, iDistribution);

    Vector<TokIdNode*> parentTokIds = Token().Nodes(domain);
    TokIdNode *node = parentTokIds.back();

    parentTokIds.pop_back();
    m_abDiscrete[iDistribution] = nt.IsDiscrete();// will be deleted soon

    if(nt.IsDiscrete())
    {
	pDistribution = new WTabularDistribFun();
	pDistribution->Setup(node, parentTokIds);
    }
    else
    {
	pDistribution = new WGaussianDistribFun();
	pDistribution->Setup(node, parentTokIds);
    }

    m_aDistribution[iDistribution] = pDistribution;
    m_abValid[iDistribution] = false;
}

void WDistributions::DropDistribution(int iNode)
{
    if(iNode >= m_aDistribution.size())
    {
	m_aDistribution.resize(iNode + 1, 0);
	m_abValid.resize(iNode + 1, false);
	return;
    }
    delete m_aDistribution[iNode];
    m_aDistribution[iNode] = 0;
    if(m_abValid.size() > iNode)
    {
	m_abValid[iNode] = false;
    }
}

void WDistributions::Apply(int iNode)
{
}

void WDistributions::ApplyNew(int iDistribution)
{
    if(m_abValid.at(iDistribution))
    {
	return;
    }
    WDistribFun *pDistribution = m_aDistribution[iDistribution];

    if(m_abDiscrete[iDistribution])
    {
	pnl::CNodeType nt = DistributionType(iDistribution);

	if (nt.GetNodeState() != pnl::nsValue)
	{
	    pDistribution->SetDefaultDistribution();
	    if(IsMRF())
	    {
		pDistribution->Matrix(pnl::matTable)->Normalize();
	    }
	}
	else
	{
	    static_cast<WTabularDistribFun*>(pDistribution)->SetDefaultUtilityFunction();
	}
    }
    else
    {
	pDistribution->SetDefaultDistribution();
    }
    m_abValid[iDistribution] = true;
}

void
WDistributions::GetNodeTypeInfo(bool *pbDiscrete, int *pSize,
				pnl::EIDNodeState *nodeState, int iNode)
{
    if(iNode >= m_abDiscrete.size())
    {
	if(!Graph().IsValidINode(iNode))
	{
	    ThrowUsingError("Requested info for non-existant node", "GetNodeTypeInfo");
	}
	Setup(iNode);
    }
    *pbDiscrete = m_abDiscrete[iNode];
    *pSize = Token().nValue(iNode);

    TokIdNode *TokId = Token().Node(iNode);
    while(TokId && TokId->tag != eTagNodeType)
    {
	TokId = TokId->v_prev;
    }
    *nodeState = ((pnl::CNodeType*)(TokId->data))->GetNodeState();
}

pnl::CNodeType WDistributions::NodeType(int iDistribution) const
{
    TokIdNode *tokNode = Token().Node(iDistribution);
	
    for(; tokNode && tokNode->tag != eTagNodeType; tokNode = tokNode->v_prev);
    PNL_CHECK_IS_NULL_POINTER(tokNode);
    PNL_CHECK_IS_NULL_POINTER(tokNode->data);
    pnl::CNodeType &refType = *(pnl::CNodeType*)tokNode->data;
    
    return pnl::CNodeType(refType.IsDiscrete(), Token().nValue(iDistribution), refType.GetNodeState());
}

pnl::CNodeType WDistributions::DistributionType(int iDistribution) const
{
    if(!IsMRF())
    {
	return NodeType(iDistribution);
    }

    Vector<int> clique;
    int iNode;

    m_pCliques->GetClique(iDistribution, &clique);
    PNL_CHECK_LEFT_BORDER(clique.size(), 1);
    iNode = clique.back();

    TokIdNode *tokNode = Token().Node(iNode);

    for(; tokNode && tokNode->tag != eTagNodeType; tokNode = tokNode->v_prev);
    PNL_CHECK_IS_NULL_POINTER(tokNode);
    PNL_CHECK_IS_NULL_POINTER(tokNode->data);
    pnl::CNodeType &refType = *(pnl::CNodeType*)tokNode->data;
    
    return pnl::CNodeType(refType.IsDiscrete(), Token().nValue(iNode), refType.GetNodeState());
}

void WDistributions::ResetDistribution(int iNode, pnl::CFactor &ft)
{
    const float *pData;
    int nElement;

    DropDistribution(iNode);

    WDistribFun* pDistr = Distribution(iNode);
    DistribFunDesc *desc = pDistr->desc();
    int NumOfNodes = desc->nNode();
    int NumOfContinuous = desc->nContinuous();

    if(DistributionType(iNode).IsDiscrete())
    {
	bool isSoftMax = false, isCondSoftMax = false;
        bool noDiscrete = true, noCont = true;
        for (int i = 0; i < NumOfNodes-1; i++)
            if (desc->isTabular(i))
            {
                noDiscrete = false;
            }
            else
            {
                noCont = false;
            }

        if ((noCont == false)&&(noDiscrete == false))
            isCondSoftMax = true;
        else
            if (noCont == false)
                isSoftMax = true;

        if (isCondSoftMax)
        {
             WCondSoftMaxDistribFun* pCondSoftMaxDF = 
                    static_cast<WCondSoftMaxDistribFun*>(Distribution(iNode));  
             pCondSoftMaxDF->CreateDefaultDistribution();

             pnl::CCondSoftMaxDistribFun *pCondSoftMaxDFHigh = 
                 pCondSoftMaxDF->GetDistribution();
             PNL_CHECK_IS_NULL_POINTER(pCondSoftMaxDFHigh);
             
             pnl::CCondSoftMaxDistribFun *pCondSoftMaxDFPNL = 
                 dynamic_cast<pnl::CCondSoftMaxDistribFun*>(ft.GetDistribFun());
             PNL_CHECK_IS_NULL_POINTER(pCondSoftMaxDFPNL);
             
             pnl::CMatrixIterator<pnl::CSoftMaxDistribFun*>* iterHigh =
                 pCondSoftMaxDFHigh->GetMatrixWithDistribution()->InitIterator();
             
             pnl::CMatrixIterator<pnl::CSoftMaxDistribFun*>* iterPNL =
                 pCondSoftMaxDFPNL->GetMatrixWithDistribution()->InitIterator();
             
             for (iterHigh, iterPNL; 
             (pCondSoftMaxDFHigh->GetMatrixWithDistribution()->IsValueHere(iterHigh))&&
                 (pCondSoftMaxDFPNL->GetMatrixWithDistribution()->IsValueHere(iterPNL));
             pCondSoftMaxDFHigh->GetMatrixWithDistribution()->Next(iterHigh),
                 pCondSoftMaxDFPNL->GetMatrixWithDistribution()->Next(iterPNL) )
             {
                 pnl::CSoftMaxDistribFun* DataPNL =
                     *(pCondSoftMaxDFPNL->GetMatrixWithDistribution()->Value(iterPNL));
                 PNL_CHECK_IS_NULL_POINTER(DataPNL);
                 
                 pnl::CMatrix<float> *matr = DataPNL->GetMatrix(pnl::matWeights);
                 pnl::floatVector *ofVect = DataPNL->GetOffsetVector();
                 
                 pnl::CSoftMaxDistribFun* DataHigh =
                     *(pCondSoftMaxDFHigh->GetMatrixWithDistribution()->Value(iterHigh));
                 PNL_CHECK_IS_NULL_POINTER(DataHigh);
                 
                 DataHigh->AttachOffsetVector(ofVect);
                 DataHigh->AttachMatrix(matr, pnl::matWeights);
             }
        }
        else
            if (isSoftMax)
            {
                WSoftMaxDistribFun* pSoftMaxDistr = 
                    static_cast<WSoftMaxDistribFun*>(Distribution(iNode));

              	pSoftMaxDistr->CreateDefaultDistribution();
                
                static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matWeights))
                    ->GetRawData(&nElement, &pData);
                
                pSoftMaxDistr->SetData(pnl::matWeights, pData);

                pnl::floatVector *offVect = 
                    static_cast<pnl::CSoftMaxDistribFun*>(ft.GetDistribFun())->
                    GetOffsetVector();

                float *off = new float [offVect->size()];
                memcpy(off, &(offVect->front()), (offVect->size())*sizeof(float));

                pSoftMaxDistr->SetVector(off);

            }
            else
            {
                static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matTable))->
                    GetRawData(&nElement, &pData);
                Distribution(iNode)->Matrix(pnl::matTable)->SetData(pData);
            }
    }
    else
    {
	bool isCondGaussian = false;
	Vector<int> Parents;
	Graph().GetParents(&Parents, iNode);

	int size = Parents.size();
	for (int parent = 0; (parent < size)&&(!isCondGaussian); parent ++)
	{
	    isCondGaussian = isCondGaussian || DistributionType(Parents[parent]).IsDiscrete();
	};

	if (isCondGaussian)
	{
	    pnl::intVector parentIndexes;
	    WCondGaussianDistribFun* pCondGauDistr = static_cast<WCondGaussianDistribFun*>(Distribution(iNode));
	    pnl::CCondGaussianDistribFun* pCondGauFT = static_cast<pnl::CCondGaussianDistribFun *>(ft.GetDistribFun());
	    pnl::CMatrix<pnl::CGaussianDistribFun*>* pMatrixWithDistribution = pCondGauFT->
		GetMatrixWithDistribution();
	    pnl::CMatrixIterator<pnl::CGaussianDistribFun*>* iter = pMatrixWithDistribution->InitIterator();

	    for (iter; pMatrixWithDistribution->IsValueHere(iter); pMatrixWithDistribution->Next(iter))
	    {
		pMatrixWithDistribution->Index( iter, &parentIndexes );		
		pCondGauDistr->CreateDefaultDistribution();

		if (pCondGauFT->GetDistribution(&(parentIndexes.front()))->IsDistributionSpecific() == 1)
		{
		    pCondGauDistr->CreateUniformDistribution();
		}
		else
		{
		    static_cast<pnl::CDenseMatrix<float>*>(pCondGauFT->GetMatrix(pnl::matMean, -1, &(parentIndexes.front())))
			->GetRawData(&nElement, &pData);
		    pCondGauDistr->SetData(pnl::matMean, pData, -1, &(parentIndexes.front()));

		    if (pCondGauFT->GetDistribution(&(parentIndexes.front()))->IsDistributionSpecific() != 2)
		    {
			static_cast<pnl::CDenseMatrix<float>*>(pCondGauFT->GetMatrix(pnl::matCovariance, -1, &(parentIndexes.front())))
			    ->GetRawData(&nElement, &pData);
			pCondGauDistr->SetData(pnl::matCovariance, pData, -1, &(parentIndexes.front()));
		    

			int NumOfNodes = ft.GetDomainSize();
			for (int parent = 0; parent < (NumOfContinuous - 1); parent++)
			{
			    static_cast<pnl::CDenseMatrix<float>*>(pCondGauFT->GetMatrix(pnl::matWeights, parent, &(parentIndexes.front())))
				->GetRawData(&nElement, &pData);
			    pCondGauDistr->SetData(pnl::matWeights, pData, parent, &(parentIndexes.front()));
			}
		    }
		}
	    }

	}
	else
	{		    
	    WGaussianDistribFun* pGauDistr = static_cast<WGaussianDistribFun*>(Distribution(iNode));

	    pGauDistr->CreateDefaultDistribution();

	    if (ft.GetDistribFun()->IsDistributionSpecific() == 1)
	    {
		pGauDistr->CreateDistribution();
	    }
	    else
	    {
		static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matMean))
		    ->GetRawData(&nElement, &pData);
		pGauDistr->SetData(pnl::matMean, pData);

		if (ft.GetDistribFun()->IsDistributionSpecific() != 2)
		{
		    static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matCovariance))
			->GetRawData(&nElement, &pData);
		    pGauDistr->SetData(pnl::matCovariance, pData);

		    int NumOfNodes = ft.GetDomainSize();
		    for (int parent = 0; parent < (NumOfNodes - 1); parent++)
		    {
			static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matWeights, parent))
			    ->GetRawData(&nElement, &pData);
			pGauDistr->SetData(pnl::matWeights, pData, parent);
		    }
		}
	    }
	}
	
    }
}

void WDistributions::FillData(TokArr &value, TokArr &probability,
			      const TokArr &parentValue, pnl::EMatrixType matType)
{
    static const char fname[] = "FillData";

    PNL_CHECK_FOR_ZERO(value.size());
    Token().Resolve(value);

    int index = Token().iNode(value[0]);

    TokArr ta(Tok(this->Token().Node(index)));
    int nodeClass = Token().NodesClassification(ta);
    if(parentValue.size())
    {
	Token().Resolve(const_cast<TokArr&>(parentValue));
    }

    if (nodeClass == eNodeClassDiscrete)
    {
	Distribution(index)->FillData(matType, value, probability, parentValue);
    }
    else
    {
	if (nodeClass == eNodeClassContinuous)
	{
	    if (static_cast<WGaussianDistribFun*>(Distribution(index))->
		IsDistributionSpecific() == 1)
	    {

		static_cast<WGaussianDistribFun*>(Distribution(index))->CreateDefaultDistribution();
	    }

	    Distribution(index)->FillData(matType, value, probability, parentValue);
	}
	else
	{
	    ThrowUsingError("Unsupported type of node", fname);
	}
    }
    Notify(Message::eSetModelInvalid, index);// eNodeUpdated is more informative
}

int WDistributions::IDistribution(const Vector<TokIdNode *> &nodes) const
{
    if(IsMRF())
    {
	return m_pCliques->iClique(nodes);
    }
    else
    {
	return Graph().INode(nodes[nodes.size() - 1]->Name());
    }
}

void WDistributions::GetDomain(Vector<int> *domain, int iDistribution) const
{
    if(IsMRF())
    {
	m_pCliques->GetClique(iDistribution, domain);
    }
    else
    {
	Graph().GetChildren(domain, iDistribution);
	domain->push_back(iDistribution);
    }
}

void WDistributions::FillDataNew(pnl::EMatrixType matType, TokArr &matrix)
{
    static const char fname[] = "FillDataNew";

    int i;

    Vector<TokIdNode *> nodes = Token().ExtractNodes(matrix[0]);

    int iDistrib = IDistribution(nodes);

    // check that all elements from the same matrix
    for(i = 1; i < matrix.size(); i++)
    {
	nodes = Token().ExtractNodes(matrix[i]);

	if(iDistrib != IDistribution(nodes))
	{
	    ThrowUsingError("All elements of matrix must be for the same distribution", fname);
	}
    }

    // apply will be called in Distribution(iDistribution), if need
    Distribution(iDistrib)->FillDataNew(matType, matrix);
}

void WDistributions::ExtractData(pnl::EMatrixType matType, TokArr &matrix)
{
    static const char fname[] = "FillDataNew";

    int i;

    Vector<TokIdNode *> nodes = Token().ExtractNodes(matrix[0]);

    int iDistrib = IDistribution(nodes);

    // check that all elements from the same matrix
    for(i = 1; i < matrix.size(); i++)
    {
	nodes = Token().ExtractNodes(matrix[i]);

	if(iDistrib != IDistribution(nodes))
	{
	    ThrowUsingError("All elements of matrix must be for the same distribution", fname);
	}
    }

    // apply will be called in Distribution(iDistribution), if need
    Distribution(iDistrib)->ExtractData(matType, matrix);
}

void WDistributions::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eMSGDelNode:
	DropDistribution(msg.IntArg());
	break;
    case Message::eChangeParentNState:
    case Message::eChangeNState:
    case Message::eInit:
	if(IsMRF())
	{
	    SetupNew(msg.IntArg());
	    ApplyNew(msg.IntArg());
	}
	else
	{
	    Setup(msg.IntArg());
	    Apply(msg.IntArg());
	}
	break;
    default:
	ThrowInternalError("Unhandled message arrive" ,"DoNotify");
	return;
    }
}

WGraph &WDistributions::Graph() const
{
    return *Token().Graph();
}

PNLW_END
