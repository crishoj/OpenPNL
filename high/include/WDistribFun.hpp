#ifndef __WDISTRIBFUN_HPP__
#define __WDISTRIBFUN_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif
#ifndef __TOKENS_HPP__
#include "Tokens.hpp"
#endif

// FORWARDS
namespace pnl
{
    class CDistribFun;
    template<class Type> class CDenseMatrix;
    class CSoftMaxDistribFun;
    class CGaussianDistribFun;
    class CCondGaussianDistribFun;
    class CCondSoftMaxDistribFun;
}

PNLW_BEGIN

class PNLHIGH_API DistribFunDesc
{
ALLOW_TESTING
public:
    DistribFunDesc(TokIdNode *node, Vector<TokIdNode*> &aParent);
    TokIdNode *node(int iNode) const { return m_aNode.at(iNode); }
    int nodeSize(int iNode) const { return m_aNodeSize.at(iNode); }
    bool isTabular(int iNode) const { return m_aNodeTypeIsTabFlag.at(iNode);};
    int nNode() const { return m_aNode.size(); }
    int nTabular() const;
    int nContinuous() const;
    bool getIndexAndValue(int *index, int *value, Tok &tok);
    Vector<int> GetValuesAsIndex(Tok &tok);
    const Vector<int> &nodeSizes() const { return m_aNodeSize; }
    bool operator==(const DistribFunDesc &aDF) const
    {
	return m_aNode.size() == aDF.m_aNode.size() && m_aNode == aDF.m_aNode
	    && m_aNodeSize == aDF.m_aNodeSize;
    }

private:
    Vector<TokIdNode*> m_aNode;
    Vector<int> m_aNodeSize;
    Vector<bool> m_aNodeTypeIsTabFlag;
};

class PNLHIGH_API WDistribFun
{
ALLOW_TESTING
public:
    virtual ~WDistribFun() {}
    virtual void SetDefaultDistribution() = 0;
    virtual Vector<int> Dimensions(int matrixType) = 0;
    void FillData(int matrixType, TokArr value, TokArr probability, TokArr parentValue = TokArr());
    void FillData(int matrixType, int index, double probability, int *parents = 0, int nParents = 0);
    void FillDataNew(int matrixType, TokArr &matrix);
    void ExtractData(int matrixType, TokArr &matrix);
    void Setup(TokIdNode *node, Vector<TokIdNode*> &aParent);
    virtual void DoSetup() = 0;
    virtual pnl::CDenseMatrix<float> *Matrix(int matrixType, int numWeightMat = -1, const int* pDiscrParentValues = 0) const = 0;
    DistribFunDesc *desc() const { return m_pDesc; }
    Vector<int> GetDiscreteParentValuesIndexes(TokArr &tabParentValue);

protected:
    WDistribFun();
    // matrix must be created
    virtual void SetAValue(int matrixId, Vector<int> &aIndex, float probability) = 0;
    virtual float GetAValue(int matrixType, Vector<int> &aIndex) = 0;

private:
    DistribFunDesc *m_pDesc;
};

class PNLHIGH_API WTabularDistribFun: public WDistribFun
{
ALLOW_TESTING
public:
    WTabularDistribFun();
    virtual void SetDefaultDistribution();
    void CreateMatrix();
    virtual ~WTabularDistribFun();
    virtual void DoSetup();

    virtual Vector<int> Dimensions(int matrixType);
    virtual void SetAValue(int matrixType, Vector<int> &aIndex, float probability);
    virtual float GetAValue(int matrixType, Vector<int> &aIndex);
    virtual pnl::CDenseMatrix<float> *Matrix(int matrixType, int numWeightMat = -1, 
        const int* pDiscrParentValues = 0) const
    {
	return m_pMatrix;
    }
    void SetDefaultUtilityFunction();
private:
    pnl::CDenseMatrix<float> *m_pMatrix;
};

class PNLHIGH_API WGaussianDistribFun: public WDistribFun
{
ALLOW_TESTING
public:
    WGaussianDistribFun();
    virtual ~WGaussianDistribFun();
    void SetDefaultDistribution();
    Vector<int> Dimensions(int matrixType);
    void FillData(int matrixType, TokArr value, TokArr probability, 
        TokArr parentValue = TokArr());
    void DoSetup();
    pnl::CDenseMatrix<float> *Matrix(int matrixType, int numWeightMat = -1, 
        const int* pDiscrParentValues = 0) const;
    void CreateDistribution();
    void SetAValue(int matrixId, Vector<int> &aIndex, float probability);
    virtual float GetAValue(int matrixType, Vector<int> &aIndex);
    void CreateDefaultDistribution();
    int IsDistributionSpecific();
    void SetData(int matrixId, const float *probability, int numWeightMat = -1);

private:
    pnl::CGaussianDistribFun *m_pDistrib;

};


class PNLHIGH_API WSoftMaxDistribFun: public WDistribFun
{
ALLOW_TESTING
public:
    WSoftMaxDistribFun();
    virtual ~WSoftMaxDistribFun();
    void SetDefaultDistribution();
    Vector<int> Dimensions(int matrixType);
    void DoSetup();
    pnl::CDenseMatrix<float> *Matrix(int maxtixType, int numWeightMat = -1, const int* pDiscrParentValues = 0) const;
    pnl::floatVector* OffsetVector() const;

    void SetAValue(int matrixId, Vector<int> &aIndex, float probability);
    void SetAValue(int NumberOfNode, int matrixId, Vector<int> &aIndex, float probability);

    virtual float GetAValue(int matrixType, Vector<int> &aIndex);
    void CreateDefaultDistribution();
    void SetData(int matrixId, const float *probability, int numWeightMat = -1);
    void SetVector(const float *probability);

private:
    pnl::CSoftMaxDistribFun *m_pDistrib;

};

class PNLHIGH_API WCondGaussianDistribFun: public WDistribFun
{
ALLOW_TESTING
public:
    WCondGaussianDistribFun();
    ~WCondGaussianDistribFun();
    Vector<int> Dimensions(int matrixType);
    void DoSetup();
    pnl::CDenseMatrix<float> *Matrix(int matrixType, int numWeightMat = -1, const int* pDiscrParentValues = 0) const;
    void CreateDefaultDistribution();
    void CreateUniformDistribution();
    void SetDefaultDistribution();
    int IsDistributionSpecific();
    pnl::CCondGaussianDistribFun *GetPNLDistribFun() {return m_pDistrib;};
    void SetData(int matrixId, const float *probability, int numWeightMat = -1, const int* pDiscrParentValues = 0);

protected:
    virtual void SetAValue(int matrixId, Vector<int> &aIndex, float probability);
    virtual float GetAValue(int matrixType, Vector<int> &aIndex);
    Vector<int> GetDiscreteParentIndexes(Vector<int> &aIndex);

private:
    pnl::CCondGaussianDistribFun *m_pDistrib;

};

class PNLHIGH_API WCondSoftMaxDistribFun: public WDistribFun
{
ALLOW_TESTING
public:
    WCondSoftMaxDistribFun();
    ~WCondSoftMaxDistribFun();
    virtual void SetDefaultDistribution();
    Vector<int> Dimensions(int matrixType);
    void DoSetup();
    pnl::CDenseMatrix<float> *Matrix(int matrixType, int numWeightMat = -1, 
        const int* pDiscrParentValues = 0) const;
    void CreateDefaultDistribution();
    pnl::CCondSoftMaxDistribFun *GetDistribution();
protected:
    virtual void SetAValue(int matrixId, Vector<int> &aIndex, float probability);
    void SetAValue(int matrixId, Vector<int> &aIndex, float probability, TokArr parentValue);

    virtual float GetAValue(int matrixType, Vector<int> &aIndex);
    float GetAValue(int matrixType, Vector<int> &aIndex, TokArr parentValue);
    Vector<int> GetDiscreteParentIndexes(Vector<int> &aIndex);

private:
    pnl::CCondSoftMaxDistribFun *m_pDistrib;

};
PNLW_END

#endif //__WDISTRIBFUN_HPP__
