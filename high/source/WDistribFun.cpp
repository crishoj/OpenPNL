#include "WDistribFun.hpp"
#include "pnl_dll.hpp"
#include "pnlString.hpp"
//#include "pnlNumericDenseMatrix.hpp"
#include "pnlTok.hpp"
#include "BNet.hpp"

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
	    *value = INDEX(tok.Node());
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
	ThrowInternalError("desc not null", "Setup");
    }

    m_pDesc = new DistribFunDesc(node, aParent);
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
    if(value.size() != probability.size() || value.size() == 0)
    {
	ThrowUsingError("The number of values must be equals to number of probabilities", fname);
    }

    if(parentValue.size() != (desc()->nNode() - 1))
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
	if(!probability[i].FltValue(0).IsUndef())
	{
	    SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
	}
    }
}

void WTabularDistribFun::MakeUniform()
{
    if(desc() == 0)
    {
	ThrowInternalError("desc is null", "MakeUniform");
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

pnl::CDistribFun *WTabularDistribFun::DistribFun() const
{
    return 0;
}

WTabularDistribFun::~WTabularDistribFun()
{
    delete m_pMatrix;
}
