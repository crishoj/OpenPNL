#undef _DEBUG
#undef DEBUG
#include "xmlreader.h"

#include "XMLReaderPNLH.hpp"
#include "XMLBindingPNLH.hpp"
#include "XMLContainer.hpp"

bool XmlReaderPNLH::Parse(const char *filename, const IXmlBinding *root, IXmlErrorHandler *errorHandler)
{
    return true;
}

bool XmlReaderPNLH::Parse(XMLContainer &container, const IXmlBinding *root)
{
    const XmlBindingPNL *binding = dynamic_cast<const XmlBindingPNL*>(root);

    if(!binding)
    {
	ThrowInternalError("Unknown Binding object type", "XmlReaderPNLH::Parse");
    }

    std::vector<const XmlBindingPNL *> aBinding;

    // prepare start and run recursive handling
    m_Container = &container;

    aBinding.push_back(binding);
    RecursiveHandling(aBinding);

    m_Container = 0;

    return true;
}

void XmlReaderPNLH::RecursiveHandling(std::vector<const XmlBindingPNL *> &aBinding)
{
    std::vector<pnl::pnlString> aName;
    int i, iHandler, fetch;
    pnl::pnlString tag;

    if(!(fetch = Container().OpenChild(&tag)))
    {
	return;
    }
    aName.reserve(aBinding.size());
    for(i = 0; i < aBinding.size(); ++i)
    {
	aName.push_back(aBinding[i]->Name());
    }
    for(; fetch; fetch = Container().NextBrother(&tag))
    {
	for(iHandler = 0; iHandler < aName.size() && !(tag == aName[iHandler]); ++iHandler);
	if(iHandler >= aName.size())
	{
	    continue;
	}
	const XmlBindingPNL& handlerForTag = *aBinding[iHandler];
	handlerForTag.Handler()->OnStartElement(this);
	std::vector<const XmlBindingPNL *> aChildBinding;
	const std::vector<const IXmlBinding*> &children = handlerForTag.Children();
	aChildBinding.resize(children.size());
	for(i = 0; i < children.size(); ++i)
	{
	    aChildBinding[i] = dynamic_cast<const XmlBindingPNL*>(children[i]);
	}
	RecursiveHandling(aChildBinding);
	handlerForTag.Handler()->OnEndElement(this);
    }
    Container().Close();
}

void XmlReaderPNLH::StopParse(const char* errorMessage)
{
    ThrowInternalError("not yet realized", "XmlReaderPNLH::StopParse");
}

void XmlReaderPNLH::GetCurrentPosition(int &line, int &column)
{
    ThrowInternalError("not yet realized", "XmlReaderPNLH::GetCurrentPosition");
}

static void ChompSpaces(pnl::pnlString &from, std::string *to)
{
    int iBegin, iEnd;

    for(iBegin = 0; iBegin < from.size() && isspace(from[iBegin]); ++iBegin);
    for(iEnd = from.size() - 1; iBegin < iEnd && isspace(from[iEnd]); --iEnd);
    to->assign(from.c_str() + iBegin, from.c_str() + iEnd + 1);
}

const std::string& XmlReaderPNLH::GetContent()
{
    ChompSpaces(m_Container->GetContent(), &m_TmpString);
    return m_TmpString;
}

void XmlReaderPNLH::GetUnescapedContent(std::string &unescaped)
{
    ChompSpaces(m_Container->GetContent(), &unescaped);
}

void XmlReaderPNLH::GetAttribute(const char *name, std::string &value)
{
    pnl::pnlString attr = Container().GetAttribute(pnl::pnlString(name));

    value.assign(attr.c_str(), attr.c_str() + attr.length());
}

bool XmlReaderPNLH::GetAttribute(const char *name, bool &value, bool defValue)
{
    pnl::pnlString attr = Container().GetAttribute(pnl::pnlString(name));

    if(attr.length())
    {
	value = (attr[0] == '1');
	return true;
    }
    else
    {
	value = defValue;
	return false;
    }
}

bool XmlReaderPNLH::GetAttribute(const char *name, int &value, int defValue)
{
    pnl::pnlString attr = Container().GetAttribute(pnl::pnlString(name));

    if(attr.length())
    {
	value = atoi(attr.c_str());
	return true;
    }
    else
    {
	value = defValue;
	return false;
    }
}

bool XmlReaderPNLH::GetAttribute(const char *name, double &value, double defValue)
{
    pnl::pnlString attr = Container().GetAttribute(pnl::pnlString(name));

    if(attr.length())
    {
	value = atof(attr.c_str());
	return true;
    }
    else
    {
	value = defValue;
	return false;
    }
}

IXmlBinding* XmlReaderPNLH::CreateBinding(IXmlHandler *handler, const char *name, int validation)
{
    return new XmlBindingPNL(handler, name, validation);
}
