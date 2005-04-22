#ifndef __PNLWDISTRIBUTIONS_HPP__
#define __PNLWDISTRIBUTIONS_HPP__

#ifndef __PNLHIGHCONFIG_HPP__
#include "pnlHighConf.hpp"
#endif
#include "ModelEngine.hpp"
#include "pnlNodeType.hpp"

PNLW_BEGIN

// FORWARDS
class WDistribFun;
class TokenCover;
class WCliques;

class PNLHIGH_API WDistributions: public ModelEngine
{
ALLOW_TESTING
public:
    WDistributions(TokenCover *pToken);
    virtual ~WDistributions();
    virtual void Setup(int iNode);
    virtual void SetupNew(int iNode);
    virtual void Apply(int iNode);
    virtual void ApplyNew(int iNode);
    WDistribFun *Distribution(int iNode)
    {
	if(IsMRF())
	{
	    if(!m_aDistribution[iNode])
	    {
		SetupNew(iNode);
	    }
	    ApplyNew(iNode);
	}
	else
	{
	    if(!m_aDistribution[iNode])
	    {
		Setup(iNode);
	    }
	    Apply(iNode);
	}
	return m_aDistribution[iNode];
    }
    void DropDistribution(int iNode);
    void GetNodeTypeInfo(bool *pbDiscrete, int *pSize, pnl::EIDNodeState *, int iNode);
    pnl::CNodeType NodeType(int iDistribution) const;
    pnl::CNodeType DistributionType(int iDistribution) const;
    void ResetDistribution(int iNode, pnl::CFactor &ft);
    void FillData(TokArr &value, TokArr &probability, const TokArr &parentValue, 
        pnl::EMatrixType matType = pnl::matTable);
    void FillDataNew(pnl::EMatrixType matType, TokArr &matrix);
    void ExtractData(pnl::EMatrixType matType, TokArr &matrix);

    void SetMRF(bool mrfFlag /* = true */);
    bool IsMRF() const { return m_bMRF; }
    WCliques &Cliques() const { return *m_pCliques; }

protected:
    virtual void DoNotify(const Message &msg);
    virtual int InterestedIn() const
    {
	return Message::eMSGDelNode | Message::eChangeNState
	| Message::eChangeParentNState | Message::eInit;
    }
    TokenCover &Token() const { return *m_pToken; }
    WGraph &Graph() const;
    bool IsDiscrete(int iNode) const;
    int IDistribution(const Vector<TokIdNode *> &nodes) const;
    void GetDomain(Vector<int> *domain, int iDistibution) const;

private:
    TokenCover *m_pToken;			// token handler
    Vector<WDistribFun*> m_aDistribution;	// distribution for each node
    Vector<bool> m_abDiscrete;			// 'discrete' flag for each node
    Vector<bool> m_abValid;			// validity flag for every distribution
    bool m_bMRF;				// is net MRF flag
    WCliques *m_pCliques;			// cliques for MRF
};

PNLW_END

#endif //__PNLWDISTRIBUTIONS_HPP__
