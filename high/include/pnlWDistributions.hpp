#ifndef __PNLWDISTRIBUTIONS_HPP__
#define __PNLWDISTRIBUTIONS_HPP__

#include "pnlHighConf.hpp"
#include "ModelEngine.hpp"
#include "pnlNodeType.hpp"

PNLW_BEGIN

// FORWARDS
class WDistribFun;
class TokenCover;
class WCliques;

class PNLHIGH_API WDistributions: public ModelEngine
{
public:
    WDistributions(TokenCover *pToken);
    virtual ~WDistributions();
    virtual void Setup(int iNode);
    virtual void SetupNew(int iNode);
    virtual void Apply(int iNode);
    virtual void ApplyNew(int iNode);
    WDistribFun *Distribution(int iNode)
    {
	if(!m_aDistribution[iNode])
	{
	    Setup(iNode);
	}
	Apply(iNode);
	return m_aDistribution[iNode];
    }
    void DropDistribution(int iNode);
    bool IsValid(int iNode);
    void GetNodeTypeInfo(bool *pbDiscrete, int *pSize, pnl::EIDNodeState *, int iNode);
    void ResetDistribution(int iNode, pnl::CFactor &ft);
    void FillData(TokArr &value, TokArr &probability, const TokArr &parentValue, 
        pnl::EMatrixType matType = pnl::matTable);
    void FillDataNew(pnl::EMatrixType matType, TokArr &matrix);

    void SetMRF(bool mrfFlag /* = true */);
    WCliques *Cliques() { return m_pCliques; }

protected:
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj);
    virtual int InterestedIn() const { return eDelNode|eChangeNState|eChangeParentNState|eInit; }
    TokenCover &Token() { return *m_pToken; }
    WGraph &Graph();
    bool IsDiscrete(int iNode) const;
    bool IsMRF() const { return m_bMRF; }

private:
    TokenCover *m_pToken;			// token handler
    Vector<WDistribFun*> m_aDistribution;	// distribution for each node
    Vector<bool> m_abDiscrete;			// 'discrete' flag for each node

    bool m_bMRF;				// is net MRF flag
    WCliques *m_pCliques;			// cliques for MRF
};

PNLW_END

#endif //__PNLWDISTRIBUTIONS_HPP__
