#include "PersistWTypes.hpp"
#include "Tokens.hpp"

#include "pnlPersistCover.hpp"
#include "pnlPersistArray.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

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

    for(iBeg = 0; text[iBeg] && text[iBeg] != '[' && iBeg < text.length(); iBeg++);
    if(text[iBeg] == '[')
    {
	++iBeg;
    }
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

const char *PersistSSMap::Signature()
{
    return "NameValue_Pair";
}

void PersistSSMap::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    std::map<String, String>::iterator it, itEnd;
    std::map<String, String> &map = *dynamic_cast<pnl::CCover<
	std::map<String, String> >*>(pObj)->GetPointer();

    for(it = map.begin(), itEnd = map.end(); it != itEnd; it++)
    {
	pContext->AddAttribute(it->first.c_str(), it->second.c_str());
    }
}

pnl::CPNLBase *PersistSSMap::Load(pnl::CContextLoad *pContext)
{
    std::map<String, String> *map = new std::map<String, String>;
    pnl::CCover<std::map<String, String> >* cover = new pnl::CCoverDel<std::map<String, String> >(map);
    pnl::pnlVector<pnl::pnlString> aAttrName;
    pnl::pnlString attrValue;

    pContext->GetAttrNames(&aAttrName);

    for(int i = aAttrName.size(); --i >= 0;)
    {
	pContext->GetAttribute(attrValue, aAttrName[i]);
	(*map)[aAttrName[i]] = attrValue;
    }

    // we can't delete it here - it may be retrieved after parsing
    //pContext->AutoDelete(cover);
    return cover;
}

bool PersistSSMap::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<std::map<String, String> >*>(pObj) != 0;
}

PNLW_END
