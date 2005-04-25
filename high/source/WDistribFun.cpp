#include "WDistribFun.hpp"
#include "WInner.hpp"
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#include "Tokens.hpp"
#include "BNet.hpp"
#include "TokenCover.hpp"
//#include "pnlSoftMaxDistribFun.hpp"
using namespace pnl;

PNLW_BEGIN

pnl::CGaussianDistribFun *CreateGaussianDefaultDistribution(DistribFunDesc *pDesk)
{
    int NumberOfNodes = pDesk->nNode();
    const CNodeType **nodeTypes = new const CNodeType *[NumberOfNodes];

    int ChildNodeSize = pDesk->nodeSize(NumberOfNodes-1);
    int WeightsSize = 0;

    //We think that all parents are continuous 
    int node;
    int dim;
    for (node = 0; node < NumberOfNodes; node++)
    {
	nodeTypes[node] = new CNodeType(false, pDesk->nodeSize(node), nsChance);
	
	if (node != (NumberOfNodes-1))
	{
	    WeightsSize += pDesk->nodeSize(node);
	};
    }

    float *dataMean = new float[ChildNodeSize];
    float *dataCov = new float[ChildNodeSize*ChildNodeSize];
    float **dataWeight = ((NumberOfNodes)!=1)?(new float *[NumberOfNodes-1]):(NULL);

    for (dim = 0; dim < ChildNodeSize; dim++)
    {
        dataMean[dim] = 1;

        for (int dim2 = 0; dim2 < ChildNodeSize; dim2++)
        {
            if (dim != dim2)
            {
                dataCov[dim*ChildNodeSize+dim2] = 0;
            }
            else 
            {
                dataCov[dim*ChildNodeSize+dim2] = 1;
            }
        };      
    }

    for (node = 0; node < NumberOfNodes-1; node++)
    {
	int WeightsVecSize = pDesk->nodeSize(NumberOfNodes-1)*pDesk->nodeSize(node);
        dataWeight[node] = new float[WeightsVecSize];

        for (int index = 0; index < WeightsVecSize; index++)
        {
            dataWeight[node][index] = 0;
        };      
    }

    CGaussianDistribFun *pDistrib = CGaussianDistribFun::CreateInMomentForm(false, NumberOfNodes, nodeTypes, 
        dataMean, dataCov, (const float **)dataWeight);
    pDistrib->CheckMomentFormValidity();

    delete nodeTypes;

    delete dataMean;
    delete dataCov;
    for (node = 0; node < NumberOfNodes-1; node++)
    {
        delete dataWeight[node];     
    };
    delete dataWeight;


    return pDistrib;
};

static int nSubNodes(TokIdNode *node)
{
    register int result = 0;
    
    PNL_CHECK_IS_NULL_POINTER(node);

    for(node = node->v_next; node; node = node->h_next, ++result);

    return result;
}

static bool isDiscrete(TokIdNode *node)
{
    TokIdNode * pType = node->v_prev;
    TokIdNode * pContinuous = continuous[0].Node();

    return (pContinuous != pType);
}

DistribFunDesc::DistribFunDesc(TokIdNode *node, Vector<TokIdNode*> &aParent)
: m_aNode(aParent)
{
    m_aNode.push_back(node);
    int i = m_aNode.size();

    m_aNodeSize.resize(i);
    m_aNodeTypeIsTabFlag.resize(i);
    for(; --i >= 0;)
    {
	m_aNodeSize[i] = nSubNodes(m_aNode[i]);
	m_aNodeTypeIsTabFlag[i] = pnlw::isDiscrete(m_aNode[i]);
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

Vector<int> DistribFunDesc::GetValuesAsIndex(Tok &tok)
{
    static const char fname[] = "GetValuesAsIndex";
    int i;
    Vector<TokIdNode *> aIdValue = tok.Nodes();
    if(aIdValue.size() != nNode())
    {
        ThrowUsingError("Numbers of nodes in domain and in tok are different", fname);
    }
    Vector<int> result(aIdValue.size(), -1);
    int index, mValue;
    for(i = 0; i < aIdValue.size(); i++)
    {
        if(aIdValue[i]->tag != eTagValue)
        {
            ThrowUsingError("There is must be value", fname);
        }
        mValue = TokenCover::Index(aIdValue[i]);
        aIdValue[i] = aIdValue[i]->v_prev;

        if(aIdValue[i]->tag != eTagNetNode)
        {
            ThrowUsingError("There is must be node", fname);
        }

        for(index = nNode(); index >= 0; --index)
        {
            if(aIdValue[i] == m_aNode[index])
            {
                if(result[index] != -1)
                {
                    ThrowInternalError("Value for each node must be set once", fname);
                }
                result[index] = mValue;
                break;
            }
        }

        if(index == -1)
        {
            ThrowInternalError("Node is not from domain of this distribution", fname);
        }
    }

    return result;
}

int DistribFunDesc::nTabular() const
{
    int nT = 0;
    int nNodes = nNode();

    for (int i = 0; i < nNodes; i++)
    {
	if (m_aNodeTypeIsTabFlag[i])
	{
	    nT++;
	};
    };

    return nT;
}

int DistribFunDesc::nContinuous() const
{
    return nNode() - nTabular();
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

    Vector<int> aIndex;
    int i, mIndex, mValue;

    if(m_pDesc == 0)
    {
	ThrowInternalError("desc is null", fname);
    }

    int NumberOfNodes = desc()->nNode();
    int ChildNodeSize = desc()->nodeSize(NumberOfNodes-1);
    int WeightsSize = 0;

    // argument checking
    if (matrixId == matTable) 
    {
	//tabular case
	if((value.size() != probability.size() || value.size() == 0)/*&&(matrixId == matTable)*/)
	{
    	    ThrowUsingError("The number of values must be equals to number of probabilities", fname);
	}

	aIndex.resize(desc()->nNode(), -1);
    }
    else 
    {
	for (int parent = 0; parent < (NumberOfNodes-1); parent++)
	{
	    WeightsSize += desc()->nodeSize(parent);
	}
/*        if (WeightsSize == 0)
	    WeightsSize = ChildNodeSize;
        else
*/
        WeightsSize *= ChildNodeSize;

	//cont case
	switch(matrixId)
	{
	case matMean:
	//Number of values and number of child node size must be equal
	    if(ChildNodeSize != probability.size())
	    {
    		ThrowUsingError("The number of probabilities must be equals to number of dimensions of child node", fname);
	    }
	    aIndex.resize(1, -1);
	    break;

	case matCovariance:
	//Number of values and (number of child node size)^2 must be equal
	    if((ChildNodeSize*ChildNodeSize) != probability.size())
	    {
    		ThrowUsingError("The number of probabilities must be equals to (number of dimensions of child node)^2", fname);
	    }
	    aIndex.resize(2, -1);
	    break;

	case matWeights:
	    if((WeightsSize) != probability.size())
	    {
    		ThrowUsingError("The number of probabilities is wrong. The correct value is sum(nodeSize(parent))*ChildNodeSize", fname);
	    }
	    aIndex.resize(3, -1);
	    aIndex[1] = 0;
	    break;
	case vectorOffset:
            aIndex.resize(1, -1);
            if (probability.size() != ChildNodeSize)
            {
                ThrowUsingError("The number of probabilities is wrong. The correct value is ChildNodeSize", fname);
            }

            break;

	default:
		ThrowUsingError("This matrixId is not supported", fname);
	    break;
	}
    }

    if((parentValue.size() != (desc()->nNode() - 1))&&(matrixId == matTable))
    {
	ThrowUsingError("Wrong number of parent's values", fname);
    }

    TokIdNode *valueBayesNode = value[0].Node();
    TokIdNode *node = (valueBayesNode->tag != eTagNetNode) ? valueBayesNode->v_prev:valueBayesNode;

    if(!node || node->tag != eTagNetNode)
    {
	ThrowUsingError("Token must point to Bayes node", fname);
    }
    for(i = value.size(); --i >= 1;)
    {
	TokIdNode *tnode = value[i].Node();
	if(tnode->tag != eTagNetNode)
	{
	    tnode = tnode->v_prev;
	}
	if(!tnode || tnode->tag != eTagNetNode)
	{
	    ThrowUsingError("Token must point to Bayes node", fname);
	}
	if(tnode != node)
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

    bool isSoftMax = false;
    for (int j=0; j< desc()->nNode(); j++)
    {
        TokIdNode *tokId = desc()->node(j);
        if (!static_cast<pnl::CNodeType*>(tokId->v_prev->data)->IsDiscrete() )
        {
            isSoftMax = true;
            break;
        }
    }

    int IndexWeightsMatrix = 0;
    for(i = 0; i < probability.size(); i++)
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
	    SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
        }

	//In continuous case aIndex means an index in vectors mean or cov or weights
	if (matrixId == matMean)
	{
	    //aIndex[0] - index of a dimension
	    aIndex[0] = i;

	    if(!probability[i].FltValue(0).IsUndef())
	    {
		SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
	    }
	}

	if (matrixId == matCovariance)
	{
	    //aIndex[0] - col index in the covariance matrix
	    //aIndex[1] - row index in the covariance matrix
	    int col = static_cast<int> (i/ChildNodeSize);
	    int row = i % ChildNodeSize;
	    aIndex[0] = col;
	    aIndex[1] = row;

	    if(!probability[i].FltValue(0).IsUndef())
	    {
		SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
	    }
	}

	if (matrixId == matWeights)
	{
            //aIndex[0] - index of weights matrix
	    //aIndex[1] - col index in the weights matrix
	    //aIndex[2] - row index in the weights matrix
            int col;
            int row;
            if (!isSoftMax)
            {
                aIndex[0] = IndexWeightsMatrix;
                aIndex[1] = (aIndex[2] == desc()->nodeSize(IndexWeightsMatrix)-1)?(aIndex[1]+1):(aIndex[1]);
                aIndex[2] = (aIndex[2] == desc()->nodeSize(IndexWeightsMatrix)-1)?(0):(aIndex[2]+1);
            }
            else
            {
                col = i % (NumberOfNodes-1);
                row = i / (NumberOfNodes-1);
                aIndex[0] = col;
                aIndex[1] = row;
            }

            if(!probability[i].FltValue(0).IsUndef())
	    {
                    SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
	    }

	    if ((aIndex[1] == ChildNodeSize -1)&&(aIndex[2] == desc()->nodeSize(IndexWeightsMatrix)-1))
	    {
		IndexWeightsMatrix++;
		aIndex[1] = 0;
		aIndex[2] = -1;
	    }
	}
        if (matrixId == vectorOffset)
	{
            aIndex[0] = i;
            
            if(!probability[i].FltValue(0).IsUndef())
            {
                SetAValue(matrixId, aIndex, probability[i].FltValue(0).fl);
            }
	}
    }
}

void WDistribFun::FillDataNew(int matrixType, TokArr &matrix)
{
    int i;
    Vector<int> aIndex;
    for(i = 0; i < matrix.size(); i++)
    {
        aIndex = desc()->GetValuesAsIndex(matrix[i]);
        SetAValue(matrixType, aIndex, matrix[i].FltValue());
    }
}

void WDistribFun::ExtractData(int matrixType, TokArr &matrix)
{
    int i;
    Vector<int> aIndex;
    for(i = 0; i < matrix.size(); i++)
    {
        aIndex = desc()->GetValuesAsIndex(matrix[i]);
        matrix[i] ^= GetAValue(matrixType, aIndex);
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

void WTabularDistribFun::SetDefaultUtilityFunction()
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
    float value = 0.0f;
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

// matrix must be created
float WTabularDistribFun::GetAValue(int matrixId, Vector<int> &aIndex)
{
    if(!m_pMatrix)
    {
	ThrowUsingError("Matrix of probabilities does not exist", "GetAValue");
    }
    return m_pMatrix->GetElementByIndexes(&aIndex.front());
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
            const pnl::pConstNodeTypeVector *ntVec = m_pDistrib->GetNodeTypesVector();
            int NumberOfNodes = ntVec->size();
            for (int node = 0; node < NumberOfNodes; node++)
            {
                delete const_cast<CNodeType*>((*ntVec)[node]);
            }
        delete m_pDistrib;
	m_pDistrib = 0;
    };
}

void WGaussianDistribFun::SetDefaultDistribution()
{
//    CreateDistribution();
    CreateDefaultDistribution();
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
    delete nodeTypes;
}


Vector<int> WGaussianDistribFun::Dimensions(int matrixType)
{
    if(!m_pDistrib)
    {
//	CreateDistribution();
        CreateDefaultDistribution();
    }

    return desc()->nodeSizes();
}


void WGaussianDistribFun::DoSetup()
{
    CreateDefaultDistribution();
//    CreateDistribution();
}


pnl::CDenseMatrix<float> *WGaussianDistribFun::Matrix(int matrixType, int numWeightMat, const int* pDiscrParentValues) const
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
	pMatrix = m_pDistrib->GetMatrix(matWeights, numWeightMat);
	break;
    case matWishartMean:
        pMatrix = m_pDistrib->GetMatrix(matWishartMean);
	break;
    case matWishartCov:
        pMatrix = m_pDistrib->GetMatrix(matWishartCov);
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

    //In continuous case aIndex means an index in vectors mean or cov or weights
#ifdef _DEBUG
    for(int i = aIndex.size(); --i >= 0;)
    {
	if(aIndex[i] < 0)
	{
	    ThrowInternalError("One of indeces < 0", "WGaussianDistribFun::SetAValue");
	}
    }
#endif

    int Index[2];
    CMatrix<float> * pMatrix;
    switch (matrixId)
    {
    case matMean:
	Index[0] = aIndex[0];
	Index[1] = 0;
	pMatrix = m_pDistrib->GetMatrix( static_cast<EMatrixType>(matrixId) );
	pMatrix->SetElementByIndexes(probability, Index);
	break;
    case matCovariance:
	Index[0] = aIndex[0];
	Index[1] = aIndex[1];
	pMatrix = m_pDistrib->GetMatrix( static_cast<EMatrixType>(matrixId) );
	pMatrix->SetElementByIndexes(probability, Index);

	Index[0] = aIndex[1];
	Index[1] = aIndex[0];
	pMatrix->SetElementByIndexes(probability, Index);
	break;
    case matWeights:
	Index[0] = aIndex[1];
	Index[1] = aIndex[2];
	pMatrix = m_pDistrib->GetMatrix( static_cast<EMatrixType>(matrixId), aIndex[0]);
	pMatrix->SetElementByIndexes(probability, Index);
	break;
    default:
	ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
	break;
    }
}

float WGaussianDistribFun::GetAValue(int matrixId, Vector<int> &aIndex)
{
    static const char fname[] = "GetAValue";

    if(!m_pDistrib)
    {
	ThrowUsingError("Distribution does not exist", fname);
    }

    //In continuous case aIndex means an index in vectors mean or cov or weights

    int Index[2];
    switch (matrixId)
    {
    case matMean:
	Index[0] = aIndex[0];
	Index[1] = 0;
	break;
    case matCovariance:
	Index[0] = aIndex[0];
	Index[1] = aIndex[1];
	break;
    case matWeights:
	Index[0] = aIndex[1];
	Index[1] = aIndex[2];
	break;
    default:
	ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
	break;
    }

    return m_pDistrib->GetMatrix(static_cast<EMatrixType>(matrixId))->GetElementByIndexes(Index);
}

//Changed: I changed sizes of dataMean and dataCov and dataWeight
void WGaussianDistribFun::CreateDefaultDistribution()
{
    m_pDistrib = CreateGaussianDefaultDistribution(desc());
}

int WGaussianDistribFun::IsDistributionSpecific()
{
    return m_pDistrib->IsDistributionSpecific();
}

void WGaussianDistribFun::SetData(int matrixId, const float *probability, int numWeightMat)
{
    static const char fname[] = "SetData";

    if(!m_pDistrib)
    {
        CreateDefaultDistribution();
    }

    EMatrixType matType;
    switch (matrixId)
    {
    case matMean:
    case matCovariance:
    case matWeights:
    case matWishartMean:
    case matWishartCov:
	matType = static_cast<EMatrixType>(matrixId);
	break;
    default:
	ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
	break;
    }

   m_pDistrib->AllocMatrix( probability, matType, numWeightMat);
}

WSoftMaxDistribFun::WSoftMaxDistribFun():WDistribFun(), m_pDistrib(0) {}

WSoftMaxDistribFun::~WSoftMaxDistribFun()
{
    if (m_pDistrib != 0) 
    { 
        const pnl::pConstNodeTypeVector *ntVec = m_pDistrib->GetNodeTypesVector();
        int NumberOfNodes = ntVec->size();
        for (int node = 0; node < NumberOfNodes; node++)
        {
            delete const_cast<CNodeType*>((*ntVec)[node]);
        }
        
        delete m_pDistrib;
	m_pDistrib = 0;
    }
}

void WSoftMaxDistribFun::SetDefaultDistribution()
{
    CreateDefaultDistribution();
}


Vector<int> WSoftMaxDistribFun::Dimensions(int matrixType)
{
    if(!m_pDistrib)
    {
        CreateDefaultDistribution();
    }
    return desc()->nodeSizes();
}


void WSoftMaxDistribFun::DoSetup() // OK
{
    CreateDefaultDistribution();
}

pnl::CDenseMatrix<float> *WSoftMaxDistribFun::Matrix(int matrixType, int numWeightMat, const int* pDiscrParentValues) const
{
    static const char fname[] = "Matrix";

    if (!m_pDistrib)
    {
	ThrowUsingError("Distribution function is not set", fname);
    }

    pnl::CMatrix<float> *pMatrix = 0;

    switch (matrixType)
    {
    case matWeights:
	pMatrix = m_pDistrib->GetMatrix(matWeights, numWeightMat);
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

pnl::floatVector* WSoftMaxDistribFun::OffsetVector() const
{
    return m_pDistrib->GetOffsetVector();
}

void WSoftMaxDistribFun::SetAValue(int matrixId, Vector<int> &aIndex, float probability) //OK
{
    static const char fname[] = "SetAValue";

    if(!m_pDistrib)
    {
        CreateDefaultDistribution();
    }

    int Index[2];
    CMatrix<float> * pMatrix;
    floatVector *offVector;
    float * off;
    switch (matrixId)
    {
    case matWeights:
        Index[0] = aIndex[1];
	Index[1] = aIndex[0];
	pMatrix = m_pDistrib->GetMatrix( static_cast<EMatrixType>(matrixId) );
	pMatrix->SetElementByIndexes(probability, Index);
	break;
    case vectorOffset:
        Index[0] = aIndex[0];
        offVector = m_pDistrib->GetOffsetVector();
        (*offVector)[Index[0]] = probability;
        off = new float[offVector->size()];
        memcpy(&off[0], &offVector->front(), (offVector->size())*sizeof(float) );
        m_pDistrib->AllocOffsetVector(off);
        break;

    default:
	ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
	break;
    }
}

float WSoftMaxDistribFun::GetAValue(int matrixId, Vector<int> &aIndex)
{
    static const char fname[] = "GetAValue";

    if(!m_pDistrib)
    {
	ThrowUsingError("Distribution does not exist", fname);
    }

    int Index[2];
    floatVector *offVector;
    CMatrix<float> * pMatrix;
    float val;
    switch (matrixId)
    {
    case matWeights:
        Index[0] = aIndex[1];
        Index[1] = aIndex[2];
        val = m_pDistrib->GetMatrix(static_cast<EMatrixType>(matrixId))->GetElementByIndexes(Index);
        break;
    case vectorOffset:
        Index[0] = aIndex[1];
        offVector = m_pDistrib->GetOffsetVector();
        val = (*offVector)[Index[0]];
        break;
    default:
        ThrowUsingError("Unsupported type of matrix in gaussian distribution", fname);
        break;
    }
    return val;
}

void WSoftMaxDistribFun::CreateDefaultDistribution() //OK
{
    if (m_pDistrib != 0)
    {
	delete m_pDistrib;
	m_pDistrib = 0;
    }

    int NumberOfNodes = desc()->nNode();

    TokIdNode *TokID = desc()->node(NumberOfNodes-1);
    int node = TokID->id[NumberOfNodes-1].int_id;

    const CNodeType **nodeTypes = new const CNodeType *[NumberOfNodes];

    int ChildNodeSize = desc()->nodeSize(NumberOfNodes-1);

    for (node = 0; node < NumberOfNodes-1; node++)
    {
        nodeTypes[node] = new CNodeType(false, desc()->nodeSize(node), nsChance);
    }

    nodeTypes[NumberOfNodes-1] = new CNodeType(true, desc()->nodeSize(node), nsChance);

    m_pDistrib = CSoftMaxDistribFun::Create(NumberOfNodes, nodeTypes, NULL, NULL);
  
    m_pDistrib->CreateDefaultMatrices(1);

}

WCondGaussianDistribFun::WCondGaussianDistribFun(): WDistribFun(), m_pDistrib(0) {};

WCondGaussianDistribFun::~WCondGaussianDistribFun()
{
    if (m_pDistrib != 0) 
    { 
            const pnl::pConstNodeTypeVector *ntVec = m_pDistrib->GetNodeTypesVector();
            int NumberOfNodes = ntVec->size();
            for (int node = 0; node < NumberOfNodes; node++)
            {
                delete const_cast<CNodeType*>((*ntVec)[node]);
            }
        delete m_pDistrib;
	m_pDistrib = 0;
    };
};

Vector<int> WCondGaussianDistribFun::Dimensions(int matrixType)
{
    if(!m_pDistrib)
    {
        CreateDefaultDistribution();
    }

    return desc()->nodeSizes();
};

void WCondGaussianDistribFun::DoSetup()
{
    CreateDefaultDistribution();
};

pnl::CDenseMatrix<float> *WCondGaussianDistribFun::Matrix(int matrixType, int numWeightMat, const int* pDiscrParentValues) const
{
    static const char fname[] = "Matrix";

    if (!m_pDistrib)
    {
	ThrowUsingError("Distribution function is not set", fname);
    }

    if (!pDiscrParentValues)
    {
	ThrowUsingError("Discrete parents combination is not set", fname);
    }

    const CGaussianDistribFun *pGDistribFun = m_pDistrib->GetDistribution(pDiscrParentValues);

    if (!pGDistribFun)
    {
	ThrowUsingError("No gaussian function corresponds to indexes", fname);
    };
    
    pnl::CMatrix<float> *pMatrix = 0;

    switch (matrixType)
    {
    case matMean:
	pMatrix = pGDistribFun->GetMatrix(matMean);
	break;
    case matCovariance:
	pMatrix = pGDistribFun->GetMatrix(matCovariance);
	break;
    case matWeights:
	pMatrix = pGDistribFun->GetMatrix(matWeights, numWeightMat);
	break;
    case matWishartMean:
        pMatrix = pGDistribFun->GetMatrix(matWishartMean);
	break;
    case matWishartCov:
        pMatrix = pGDistribFun->GetMatrix(matWishartCov);
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
};

void WCondGaussianDistribFun::CreateDefaultDistribution()
{
    int NumberOfNodes = desc()->nNode();
    const CNodeType **nodeTypes = new const CNodeType *[NumberOfNodes];

    for (int node = 0; node < NumberOfNodes; node++)
    {
	nodeTypes[node] = new CNodeType(desc()->isTabular(node), desc()->nodeSize(node), nsChance);
    }

    m_pDistrib = pnl::CCondGaussianDistribFun::Create( 0, NumberOfNodes,
        nodeTypes );

    delete nodeTypes;
};

PNLW_END
