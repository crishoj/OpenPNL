#include "WDistribFun.hpp"
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#include "pnlTok.hpp"
#include "BNet.hpp"
#include "TokenCover.hpp"

using namespace pnl;

static int nSubNodes(TokIdNode *node)
{
    register int result = 0;
    
    PNL_CHECK_IS_NULL_POINTER(node);

    for(node = node->v_next; node; node = node->h_next, ++result);

    return result;
}

DistribFunDesc::DistribFunDesc(TokIdNode *node, Vector<TokIdNode*> &aParent)
: m_aNode(aParent)
{
    m_aNode.push_back(node);
    int i = m_aNode.size();

    m_aNodeSize.resize(i);
    for(; --i >= 0;)
    {
	m_aNodeSize[i] = nSubNodes(m_aNode[i]);
    }
}

// encapsulates the fetching of index and value from Bayes node
bool DistribFunDesc::getIndexAndValue(int *index, int *value, Tok &tok)
{
    TokIdNode *node = tok.Node()->v_prev;

    for(*index = nNode(); *index >= 0; --*index)
    {
	if(node == m_aNode[*index])
	{
	    *value = TokenCover::Index(tok.Node());
	    return true;
	}
    }

    *value = -1;
    return false;
}

WTabularDistribFun::WTabularDistribFun(): m_pMatrix(0) {}

WDistribFun::WDistribFun(): m_pDesc(0) {}

void WDistribFun::Setup(TokIdNode *node, Vector<TokIdNode*> &aParent)
{
    if(m_pDesc != 0)
    {
	delete m_pDesc;
    }

    m_pDesc = new DistribFunDesc(node, aParent);
    DoSetup();
}

// assumptions:
//   all parents were connected to node before this point
void WDistribFun::FillData(int matrixId, TokArr value, TokArr probability, TokArr parentValue)
{
    static const char fname[] = "FillData";

    if(m_pDesc == 0)
    {
	ThrowInternalError("desc is null", fname);
    }

    // argument checking
    if((value.size() != probability.size() || value.size() == 0)&&(matrixId == matTable))
    {
	ThrowUsingError("The number of values must be equals to number of probabilities", fname);
    }

    if((parentValue.size() != (desc()->nNode() - 1))&&(matrixId == matTable))
    {
	ThrowUsingError("Wrong number of parent's values", fname);
    }

    Vector<int> aIndex(desc()->nNode(), -1);
    int i, mIndex, mValue;
    TokIdNode *valueBayesNode = value[0].Node();

    for(i = value.size(); --i >= 1;)
    {
	if(value[i].Node() != valueBayesNode)
	{
	    ThrowUsingError("Node must be same for all values", fname);
	}
    }

    //This "for" works only for tabular
    for(i = parentValue.size(); --i >= 0;)
    {
	if(!desc()->getIndexAndValue(&mIndex, &mValue, parentValue[i]))
	{
	    pnl::pnlString str;

	    str << "node '" << parentValue[i].Node()->v_prev->Name()
		<< "' is not parent for node '" << value[0].Node()->v_prev->Name() << "'";
		
	    ThrowUsingError(str.c_str(), fname);
	}
	if(aIndex[mIndex] != -1)
	{
	    pnl::pnlString str;

	    str << "duplicate value for node '"
		<< parentValue[i].Node()->v_prev->Name() << "'";
	    ThrowUsingError(str.c_str(), fname);
	}
	if(mIndex == aIndex.size() - 1)
	{
	    ThrowUsingError("Node is not parent for itself", fname);
	}
	aIndex[mIndex] = mValue;
    }

    for(i = value.size(); --i >= 0;)
    {
        if (matrixId == matTable)  
        {
	    if(!desc()->getIndexAndValue(&mIndex, &mValue, value[i]))
	    {
	        pnl::pnlString str;

	        str << "Unknown value for node '" << valueBayesNode->Name() << "'";
	        ThrowUsingError(str.c_str(), fname);
	    }
	    if(mIndex != aIndex.size() - 1)
	    {
	        ThrowUsingError("mixed parent and node itself", fname);
	    }
            aIndex[mIndex] = mValue;
        }
	if(!probability[i].FltValue(0).IsUndef())
	{
	    SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
	}
    }
}

void WTabularDistribFun::SetDefaultDistribution()
{
    if(desc() == 0)
    {
	ThrowInternalError("desc is null", "SetDefaultDistribution");
    }

    if(m_pMatrix == 0)
    {
	CreateMatrix();
    }

    int nNode = desc()->nNode();
    Vector<int> aIndex(nNode, 0);
    float value = 1.0f/desc()->nodeSize(nNode - 1);
    int i, nValue;

    for(nValue = 1, i = 0; i < nNode; ++i)
    {
	nValue *= desc()->nodeSize(i);
    }

    for(i = 0; i < nValue; ++i)
    {
	m_pMatrix->SetElementByOffset(value, i);
    }
}

void WTabularDistribFun::CreateMatrix()
{
    const Vector<int> &aSize = desc()->nodeSizes();

    int size, i;

    for(i = aSize.size(), size = 1; --i >= 0; size*=aSize[i]);
    Vector<float> aValue(size, float(0));
    m_pMatrix = pnl::CNumericDenseMatrix<float>::Create(aSize.size(),
	&aSize.front(), &aValue.front());
}

// matrix must be created
void WTabularDistribFun::SetAValue(int matrixId, Vector<int> &aIndex, float probability)
{
    if(!m_pMatrix)
    {
	CreateMatrix();
    }
    m_pMatrix->SetElementByIndexes(probability, &aIndex.front());
}

Vector<int> WTabularDistribFun::Dimensions(int matrixType)
{
    if(matrixType != pnl::matTable)
    {
	ThrowUsingError("Tabular distribution have only tabular matrix", "Dimensions");
    }

    if(!m_pMatrix)
    {
	CreateMatrix();
    }

    return desc()->nodeSizes();
}

WTabularDistribFun::~WTabularDistribFun()
{
    //delete m_pMatrix;
    delete desc();
}

void WTabularDistribFun::DoSetup()
{
    CreateMatrix();
}


WGaussianDistribFun::WGaussianDistribFun():WDistribFun(), m_pDistrib(0) {}

WGaussianDistribFun::~WGaussianDistribFun()
{
    if (m_pDistrib != 0) 
    {   
	
        /*for (node = 0; node < NumberOfNodes; node++)
    {
	delete nodeTypes[node];
    }*/

        delete m_pDistrib;
	m_pDistrib = 0;
    };
}

void WGaussianDistribFun::SetDefaultDistribution()
{
    CreateDistribution();
}

void WGaussianDistribFun::CreateDistribution()
{
    if (m_pDistrib != 0)
    {
	delete m_pDistrib;
	m_pDistrib = 0;
    }

    int NumberOfNodes = desc()->nNode();
    const CNodeType **nodeTypes = new const CNodeType *[NumberOfNodes];

    int node;
    for (node = 0; node < NumberOfNodes; node++)
    {
	nodeTypes[node] = new CNodeType(false, desc()->nodeSize(node), nsChance);
    }

    m_pDistrib = CGaussianDistribFun::CreateUnitFunctionDistribution(NumberOfNodes, nodeTypes);

    for (node = 0; node < NumberOfNodes; node++)
    {
	delete nodeTypes[node];
    }

    delete nodeTypes;
}


Vector<int> WGaussianDistribFun::Dimensions(int matrixType)
{
    if(!m_pDistrib)
    {
	CreateDistribution();
    }

    return desc()->nodeSizes();
}


void WGaussianDistribFun::DoSetup()
{
//    CreateDefaultDistribution();
    CreateDistribution();
}


pnl::CDenseMatrix<float> *WGaussianDistribFun::Matrix(int matrixType) const
{
    static const char fname[] = "Matrix";

    if (!m_pDistrib)
    {
	ThrowUsingError("Distribution function is not set", fname);
    }

    pnl::CMatrix<float> *pMatrix = 0;

    switch (matrixType)
    {
    case matMean:
	pMatrix = m_pDistrib->GetMatrix(matMean);
	break;
    case matCovariance:
	pMatrix = m_pDistrib->GetMatrix(matCovariance);
	break;
    case matWeights:
	pMatrix = m_pDistrib->GetMatrix(matWeights, 0);
	break;
    default:
	ThrowUsingError("Unsupported matrix type", fname);
	break;
    }

    if (dynamic_cast<pnl::CDenseMatrix<float> *>(pMatrix) != 0)
    {
	return dynamic_cast<pnl::CDenseMatrix<float> *>(pMatrix);
    }
    else
    {
	return pMatrix->ConvertToDense();
    }
}

void WGaussianDistribFun::SetAValue(int matrixId, Vector<int> &aIndex, float probability)
{
    static const char fname[] = "SetAValue";

    if(!m_pDistrib)
    {
//	CreateDistribution();
        CreateDefaultDistribution();
    }

    EMatrixType matType;
    switch (matrixId)
    {
    case matMean:
    case matCovariance:
    case matWeights:
    case matH:
    case matK:
	matType = static_cast<EMatrixType>(matrixId);
	break;
    default:
	ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
	break;
    }

   m_pDistrib->AllocMatrix( &probability, matType, 0);
//    m_pDistrib->GetMatrix(matType)->SetElementByIndexes(probability, &aIndex.front());
}

void WGaussianDistribFun::CreateDefaultDistribution()
{
    if (m_pDistrib != 0)
    {
	delete m_pDistrib;
	m_pDistrib = 0;
    }

    int NumberOfNodes = desc()->nNode();
    const CNodeType **nodeTypes = new const CNodeType *[NumberOfNodes];

    int node;
    for (node = 0; node < NumberOfNodes; node++)
    {
	nodeTypes[node] = new CNodeType(false, desc()->nodeSize(node), nsChance);
    }

    float *dataMean = new float[NumberOfNodes];
    float *dataCov = new float[NumberOfNodes*NumberOfNodes];
    float **dataWeight = new float *[NumberOfNodes];

    for (node = 0; node < NumberOfNodes; node++)
    {
        dataMean[node] = 1;
        dataWeight[node] = new float[NumberOfNodes-1];

        for (int node2 = 0; node2 < NumberOfNodes-1; node2++)
        {
            if (node != node2)
            {
                dataCov[node*NumberOfNodes+node2] = 0;
            }
            else 
            {
                dataCov[node*NumberOfNodes+node2] = 1;
            }

            dataWeight[node][node2] = 0;
        };

        if (node != (NumberOfNodes-1))
        {
            dataCov[node*NumberOfNodes+NumberOfNodes-1] = 0;
        }
        else 
        {
            dataCov[node*NumberOfNodes+NumberOfNodes-1] = 1;
        };       
    }

    m_pDistrib = CGaussianDistribFun::CreateInMomentForm(false, NumberOfNodes, nodeTypes, 
        dataMean, dataCov, (const float **)dataWeight);

/*    for (node = 0; node < NumberOfNodes; node++)
    {
	delete nodeTypes[node];
    }
*/

    delete nodeTypes;
    delete dataMean;
    delete dataCov;
    for (node = 0; node < NumberOfNodes; node++)
    {
        delete dataWeight[node];     
    };
    delete dataWeight;
}

int WGaussianDistribFun::IsDistributionSpecific()
{
    return m_pDistrib->IsDistributionSpecific();
}
