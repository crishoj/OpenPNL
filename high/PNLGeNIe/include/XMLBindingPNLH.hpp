#ifndef __XMLBINDINGPNLH_HPP__
#define __XMLBINDINGPNLH_HPP__

#undef _DEBUG
#undef DEBUG
#include "xmlreader.h"
#include "constants.h"
#include "typesPNLH.hpp"

class XmlBindingPNL: public IXmlBinding
{
public:
    XmlBindingPNL(IXmlHandler *handler, const char *name, int validation):
    m_Name(name), m_Handler(handler) {}

    virtual ~XmlBindingPNL() {}

    virtual void AddChild(const IXmlBinding *child, int occurences);
    virtual void AddAttribute(const char *attributeName, bool required);
    virtual void Delete();

    StringGP Name() const { return m_Name; }
    IXmlHandler *Handler() const { return m_Handler; }

#ifndef NDEBUG
    virtual void Dump(int indent, std::string &output) const;
#endif

    enum
    {
	ChildRequired = 1,
	ChildMultiple = 2,
	ZeroOrMore = ChildMultiple,
	OneOrMore = ChildRequired | ChildMultiple,
	ExactlyOne = ChildRequired,
	ZeroOrOne = 0,
    };

    enum 
    {
	ValidateNone = 0,
	ValidateAttributes = 1,
	ValidateChildren = 2,
	ValidateAll = ValidateAttributes | ValidateChildren,
	ValidateInherit = 4,
    };

    const std::vector<const IXmlBinding*>& Children() const
    { return m_Children; }

private:
    StringGP m_Name;
    IXmlHandler *m_Handler;
    std::vector<const IXmlBinding*> m_Children;
    std::vector<StringGP> m_aAttrName;
};

#endif // include guard
