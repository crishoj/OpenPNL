#ifndef __WDISTRIBFUN_HPP__
#define __WDISTRIBFUN_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

#include "pnlTok.hpp"

// FORWARDS
namespace pnl
{
    class CDistribFun;
    template<class Type> class CDenseMatrix;
    class CGaussianDistribFun;
}

class DistribFunDesc
{
public:
    DistribFunDesc(TokIdNode *node, Vector<TokIdNode*> &aParent);
    TokIdNode *node(int iNode) const { return m_aNode.at(iNode); }
    int nodeSize(int iNode) const { return m_aNodeSize.at(iNode); }
    int nNode() const { return m_aNode.size(); }
    bool getIndexAndValue(int *index, int *value, Tok &tok);
    const Vector<int> &nodeSizes() const { return m_aNodeSize; }

private:
    Vector<TokIdNode*> m_aNode;
    Vector<int> m_aNodeSize;
};

class PNLHIGH_API WDistribFun
{
public:
    virtual ~WDistribFun() {}
    virtual void SetDefaultDistribution() = 0;
    virtual Vector<int> Dimensions(int matrixType) = 0;
    void FillData(int matrixType, TokArr value, TokArr probability, TokArr parentValue = TokArr());
    void FillData(int matrixType, int index, double probability, int *parents = 0, int nParents = 0);
    void Setup(TokIdNode *node, Vector<TokIdNode*> &aParent);
    virtual void DoSetup() = 0;
    virtual pnl::CDenseMatrix<float> *Matrix(int maxtixType) const = 0;

protected:
    WDistribFun();
    DistribFunDesc *desc() const { return m_pDesc; }
    // matrix must be created
    virtual void SetAValue(int matrixId, Vector<int> &aIndex, float probability) = 0;

private:
    DistribFunDesc *m_pDesc;
};

class PNLHIGH_API WTabularDistribFun: public WDistribFun
{
public:
    WTabularDistribFun();
    virtual void SetDefaultDistribution();
    void CreateMatrix();
    virtual ~WTabularDistribFun();
    virtual void DoSetup();

    virtual Vector<int> Dimensions(int matrixType);
    virtual void SetAValue(int matrixType, Vector<int> &aIndex, float probability);
    virtual pnl::CDenseMatrix<float> *Matrix(int matrixType) const
    {
	return m_pMatrix;
    }

private:
    pnl::CDenseMatrix<float> *m_pMatrix;
};

class PNLHIGH_API WGaussianDistribFun: public WDistribFun
{
public:
    WGaussianDistribFun();
    virtual ~WGaussianDistribFun();
    void SetDefaultDistribution();
    Vector<int> Dimensions(int matrixType);
    void FillData(int matrixType, TokArr value, TokArr probability, TokArr parentValue = TokArr());
    void DoSetup();
    pnl::CDenseMatrix<float> *Matrix(int maxtixType) const;
    void CreateDistribution();
    void SetAValue(int matrixId, Vector<int> &aIndex, float probability);
    void CreateDefaultDistribution();

private:
    pnl::CGaussianDistribFun *m_pDistrib;

};

#endif //__WDISTRIBFUN_HPP__
