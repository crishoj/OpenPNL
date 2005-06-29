#include "PersistProbNet.hpp"
#include "PersistNodeDiagInfo.hpp"
#include "PersistWNodeInfo.hpp"
#include "pnlPersistCover.hpp"
//#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
//#include "pnlWDistributions.hpp"
//#include "pnlWGraph.hpp"
//#include "TokenCover.hpp"

#include "pnlPersistCover.hpp"

NodeDiagInfo::NodeDiagInfo(const String &diagType): m_diagType(diagType)
{
}

void PersistNodeDiagInfo::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    NodeDiagInfo *diagInfo = static_cast<pnl::CCover<NodeDiagInfo>*>(pObj)->GetPointer();
    pContext->AddAttribute("NodeDiagType", diagInfo->m_diagType.c_str());
    pContext->AddAttribute("IsMandatory", diagInfo->m_isMandatory);
    pContext->AddAttribute("IsRanked", diagInfo->m_isRanked);
    String targets;
    targets << "[";
    int i;
    for (i = 0; i < diagInfo->m_isTarget.size(); ++i)
    {
        targets << diagInfo->m_isTarget[i];
        targets << " ";
    }
    targets << "]";
    pContext->AddAttribute("IsTarget", targets.c_str());
    pContext->AddAttribute("Default", diagInfo->m_isDefault);
    pContext->AddAttribute("Cost", float(diagInfo->cost));
}

pnl::CPNLBase *PersistNodeDiagInfo::Load(pnl::CContextLoad *pContext)
{
    int i;
    NodeDiagInfo *dInfo = new NodeDiagInfo();
    pContext->GetAttribute(dInfo->m_diagType, "NodeDiagType");
    String tmp;
    pContext->GetAttribute(tmp, "IsMandatory");
    if (tmp == "0")
    {
        dInfo->m_isMandatory = false;
    }
    else
        if (tmp == "1")
        {
            dInfo->m_isMandatory = true;
        }
    pContext->GetAttribute(tmp, "IsRanked");
    if (tmp == "0")
    {
        dInfo->m_isRanked = false;
    }
    else
        if (tmp == "1")
        {
            dInfo->m_isRanked = true;
        }
    String targets;
    pContext->GetAttribute(targets, "IsTarget");
    if (targets != "[]")
    {
        for(i = 0; i < targets.size(); ++i)
        {
            if (targets[i] == '0')
            {
                dInfo->m_isTarget.push_back(false);
            }
            else 
                if (targets[i] == '1')
                {
                    dInfo->m_isTarget.push_back(true);
                }
        }
    }
    pContext->GetAttribute(tmp, "Default");
    dInfo->m_isDefault = atoi(tmp.c_str());
    pContext->GetAttribute(tmp, "Cost");
    dInfo->cost = atof(tmp.c_str());
    return new pnl::CCover<NodeDiagInfo>(dInfo);
}

void PersistNodeDiagInfo::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
{/*
    ProbabilisticNet *net = static_cast<pnl::CCover<ProbabilisticNet>*>(pObj)->GetPointer();
    int i, j;
    int nNode = net->nNetNode();
    Vector<String> aValue;

    for(i = 0; i < nNode; ++i)
    {
        pnl::pnlString name("Node");
        WNodeInfo *nodeInfo = new WNodeInfo(net->NodeName(i), net->pnlNodeType(i));

        name << i;
        net->Token().GetValues(i, aValue);
        nodeInfo->m_aValue << aValue[0];
        for(j = 1 ; j < aValue.size(); ++j)
        {
            nodeInfo->m_aValue << ' ' << aValue[j];
        }

        pContext->AutoDelete(nodeInfo);
        pContext->Put(nodeInfo, name.c_str());
    }*/
}

const char *PersistNodeDiagInfo::Signature()
{
    return "DiagnosisInfo";
}

bool PersistNodeDiagInfo::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<NodeDiagInfo>*>(pObj) != 0;
}

static PersistNodeDiagInfo m_DiagnosisInfoPersistence; //register object
