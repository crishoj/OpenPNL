#ifndef __PNLWDISTRIBUTIONS_HPP__
#define __PNLWDISTRIBUTIONS_HPP__

#include "pnlHighConf.hpp"
#include "ModelEngine.hpp"

// FORWARDS
class WDistribFun;
class TokenCover;

class PNLHIGH_API WDistributions: public ModelEngine
{
public:
    WDistributions(TokenCover *pToken);
    virtual ~WDistributions() {}
    virtual void Setup(int iNode);
    virtual void Apply(int iNode);
    WDistribFun *Distribution(int iNode)
    {
	Apply(iNode);
	return m_aDistribution[iNode];
    }
    void DropDistribution(int iNode);
    bool IsValid(int iNode);
    void GetNodeTypeInfo(bool *pbDiscrete, int *pSize, int iNode);

protected:
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj);
    virtual int InterestedIn() const { return eDelNode|eChangeNState|eChangeParentNState|eInit; }

private:
    TokenCover *m_pToken;			// token handler
    Vector<WDistribFun*> m_aDistribution;	// distribution for each node
    Vector<bool> m_abDiscrete;			// 'discrete' flag for each node
};

#endif //__PNLWDISTRIBUTIONS_HPP__