#include "PersistWBNet.hpp"
#include "PersistWNodeInfo.hpp"
#include "pnlPersistCover.hpp"
#include "pnlPersistArray.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

// WNodeInfo

WNodeInfo::WNodeInfo(String &name, pnl::CNodeType nt): m_Name(name), m_NodeType(nt)
{
}


// Persistence
const char *PersistWNodeInfo::Signature()
{
    return "WNode";
}

void PersistWNodeInfo::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    WNodeInfo *nodeInfo = dynamic_cast<pnl::CCover<WNodeInfo>*>(pObj)->GetPointer();

    pContext->AddAttribute("NodeName", nodeInfo->m_Name.c_str());
}

pnl::CPNLBase *PersistWNodeInfo::Load(pnl::CContextLoad *pContext)
{
    WNodeInfo *nodeInfo = new WNodeInfo;

    pContext->GetAttribute(nodeInfo->m_Name, "NodeName");

    pnl::CNodeType *nodeType = static_cast<pnl::CNodeType*>(pContext->Get("NodeType"));
    nodeInfo->m_NodeType = *nodeType;
    pContext->AutoDelete(nodeType);

    pnl::CCover<TokArr> *pCov = static_cast<pnl::CCover<TokArr>*>(pContext->Get("Values"));
    nodeInfo->m_aValue = *pCov->GetPointer();

    return nodeInfo;
}

void PersistWNodeInfo::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
{
    WNodeInfo *nodeInfo = dynamic_cast<pnl::CCover<WNodeInfo>*>(pObj)->GetPointer();

    pContext->Put(&nodeInfo->m_NodeType, "NodeType");
    pnl::CCover<TokArr> *pCov = new pnl::CCover<TokArr>(&nodeInfo->m_aValue);
    pContext->AutoDelete(pCov);
    pContext->Put(pCov, "Values");
}

bool PersistWNodeInfo::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<WNodeInfo>*>(pObj) != 0;
}

// TokArr
const char *PersistTokenArray::Signature()
{
    return "TokenArray";
}

void PersistTokenArray::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    TokArr *paTok = dynamic_cast<pnl::CCover<TokArr>*>(pObj)->GetPointer();
    pnl::pnlVector<pnl::pnlString> aString;
    
    aString.resize(paTok->size());
    for(int i = 0; i < paTok->size(); ++i)
    {
	aString[i] = (*paTok)[i].Name();
    }
    pnl::SaveArray(*pContext, &aString.front(), aString.size());
}

pnl::CPNLBase *PersistTokenArray::Load(pnl::CContextLoad *pContext)
{
    pnl::pnlString text;

    pContext->GetText(text);

    int iBeg, iEnd;

    for(iBeg = 0; text[iBeg] && text[iBeg] != '['; iBeg++);
    for(iEnd = text.size() - 1; iEnd > iBeg && text[iEnd] != ']'; --iEnd);

    pnl::pnlString textStripped;

    if(iEnd > iBeg)
    {
	textStripped.append(text.c_str() + iBeg, iEnd - iBeg);
    }

    TokArr *paTok = new TokArr(textStripped);

    pnl::CCover<TokArr> *pCov = new pnl::CCoverDel<TokArr>(paTok);
    pContext->AutoDelete(pCov);

    return pCov;
}

bool PersistTokenArray::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<TokArr>*>(pObj) != 0;
}
