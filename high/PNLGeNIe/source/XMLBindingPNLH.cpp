#include "XMLBindingPNLH.hpp"
#undef _DEBUG
#undef DEBUG
#include "constants.h"
#include "TokenCover.hpp"

void XmlBindingPNL::AddChild(const IXmlBinding *child, int occurences)
{
    m_Children.push_back(child);
}

void XmlBindingPNL::AddAttribute(const char *attributeName, bool required)
{
    m_aAttrName.push_back(attributeName);
}

void XmlBindingPNL::Delete()
{
    delete this;
}

#ifndef NDEBUG
void XmlBindingPNL::Dump(int indent, std::string &output) const
{

}
#endif
