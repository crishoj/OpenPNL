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
    virtual pnl::CDistribFun *DistribFun() const = 0;
    virtual Vector<int> Dimensions(int matrixType) = 0;
    void FillData(int matrixType, TokArr value, TokArr probability, TokArr parentValue = TokArr());
    void Setup(TokIdNode *node, Vector<TokIdNode*> &aParent);

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
    void MakeUniform();
    void CreateMatrix();
    virtual ~WTabularDistribFun();

    virtual Vector<int> Dimensions(int matrixType);
    virtual void SetAValue(int matrixType, Vector<int> &aIndex, float probability);
    virtual pnl::CDistribFun *DistribFun() const;
    pnl::CDenseMatrix<float> *Matrix() const { return m_pMatrix; }

private:
    pnl::CDenseMatrix<float> *m_pMatrix;
};

class PNLHIGH_API WGaussianDistribFun: public WDistribFun
{
public:
    WGaussianDistribFun() {}
};

#endif //__BNET_HPP__
