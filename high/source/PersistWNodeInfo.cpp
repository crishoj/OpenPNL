#include "PersistProbNet.hpp"
#include "PersistWNodeInfo.hpp"
#include "pnlPersistCover.hpp"
#include "pnlPersistArray.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

// WNodeInfo

WNodeInfo::WNodeInfo(const String &name, pnl::CNodeType nt): m_Name(name), m_NodeType(nt)
{
}


// Persistence for WNodeInfo
const char *PersistWNodeInfo::Signature()
{
    return "WNode";
}

void PersistWNodeInfo::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    WNodeInfo *nodeInfo = dynamic_cast<WNodeInfo*>(pObj);

    pContext->AddAttribute("NodeName", nodeInfo->m_Name.c_str());
    pContext->AddText("[");
    pContext->AddText(nodeInfo->m_aValue.c_str());
    pContext->AddText("]");
}

pnl::CPNLBase *PersistWNodeInfo::Load(pnl::CContextLoad *pContext)
{
    WNodeInfo *nodeInfo = new WNodeInfo;

    pContext->GetAttribute(nodeInfo->m_Name, "NodeName");

    pnl::CNodeType *nodeType = static_cast<pnl::CNodeType*>(pContext->Get("NodeType"));
    nodeInfo->m_NodeType = *nodeType;
    //pContext->AutoDelete(nodeType);

    String text;

    pContext->GetText(text);

    int iBeg, iEnd;

    for(iBeg = 0; text[iBeg] && text[iBeg] != '[' && iBeg < text.length(); iBeg++);
    if(text[iBeg] == '[')
    {
	++iBeg;
    }
    for(iEnd = text.size() - 1; iEnd > iBeg && text[iEnd] != ']'; --iEnd);

    if(iEnd > iBeg)
    {
	nodeInfo->m_aValue.append(text.c_str() + iBeg, iEnd - iBeg);
    }

    return nodeInfo;
}

void PersistWNodeInfo::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
{
    WNodeInfo *nodeInfo = dynamic_cast<WNodeInfo*>(pObj);

    pContext->Put(&nodeInfo->m_NodeType, "NodeType");
}

bool PersistWNodeInfo::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<WNodeInfo*>(pObj) != 0;
}

PNLW_END
