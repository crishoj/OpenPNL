#ifndef __XMLCONTAINER_HPP__
#define __XMLCONTAINER_HPP__

#include "pnlConfig.hpp"
#include "pnlXMLContainer.hpp"

// FORWARDS
namespace pnl
{
    class pnlString;
};

class XmlBindingPNL;

PNLW_USING

// This container filters output to underlying container.
// It filters by 'elementName'.
// The element with pointed name with all sub-elements is directed to 'output' container.
class XMLContainerFilter: public pnl::CXMLContainer
{
public:
    XMLContainerFilter(pnl::CXMLContainer *output, const char *elementName)
	: m_CurrentLevel(0), m_LevelOn(-1), m_Output(output), m_ElementName(elementName)
    {}
    virtual ~XMLContainerFilter() {}

    virtual void CreateNode(const pnl::pnlString &name)
    {
	m_CurrentLevel++;
	if(IsOn())
	{
	    m_Output->CreateNode(name);
	}
	else if(name == m_ElementName)
	{
	    m_LevelOn = m_CurrentLevel;
	    m_Output->CreateNode(name);
	}
    }
    virtual void AddContent(const pnl::pnlString &addition)
    {
	if(IsOn())
	{
	    m_Output->AddContent(addition);
	}
    }
    virtual void AddAttribute(const pnl::pnlString &attrName, const pnl::pnlString &attrValue)
    {
	if(IsOn())
	{
	    m_Output->AddAttribute(attrName, attrValue);
	}
    }
    virtual void Close()
    {
	if(IsOn())
	{
	    m_Output->Close();
	}
	if(--m_CurrentLevel < m_LevelOn)
	{
	    m_LevelOn = -1;
	}
    }

private:
    bool IsOn() { return m_LevelOn >= 0; }

private:
    int m_CurrentLevel;
    int m_LevelOn;
    pnl::CXMLContainer *m_Output;
    pnl::pnlString m_ElementName;
};

// Simple container which keeps XML file as tree
class XMLContainer: public pnl::CXMLContainer
{
private:
    class Node
    {
    public:
	Node(): m_NextBrother(-1) {}

	pnl::pnlString m_Name;
	pnl::pnlString m_Content;
	int m_NextBrother;
	std::vector<pnl::pnlString> m_aAttribute;
	std::vector<int> m_aiChild;
    };

    const Node& CurNode() const
    {
	if(m_Stack.size() == 0)
	{
	    ThrowInternalError("There is no element", "XMLContainer::XMLNode");
	}
	return m_aNode[m_Stack.back()];
    }

    Node &LastElement()
    {
	return m_aNode.back();
    }

    pnl::pnlVector<Node> m_aNode;
    pnl::pnlVector<int> m_Stack;
    int m_iLastClosed;

public:
    XMLContainer(): m_iLastClosed(-1) {}
    virtual ~XMLContainer() {}

    virtual void CreateNode(const pnl::pnlString &name)
    {
	int iNode = m_aNode.size();
	if(m_iLastClosed >= 0)
	{
	    m_aNode[m_iLastClosed].m_NextBrother = iNode;
	    m_iLastClosed = -1;
	}
	if(m_Stack.size())
	{// add child to his parent's child list
	    m_aNode[m_Stack.back()].m_aiChild.push_back(iNode);
	}
	m_Stack.push_back(iNode);
	m_aNode.resize(iNode + 1);
	m_aNode.back().m_Name = name;
    }

    virtual void AddContent(const pnl::pnlString &addition)
    {
	LastElement().m_Content.append(addition.c_str());
    }

    virtual void AddAttribute(const pnl::pnlString &attrName, const pnl::pnlString &attrValue)
    {
	LastElement().m_aAttribute.push_back(attrName);
	LastElement().m_aAttribute.push_back(attrValue);
    }

    virtual void Close()
    {
	m_iLastClosed = m_Stack.back();
	m_Stack.pop_back();
    }

    int NextBrother(pnl::pnlString *name)
    {
	if(!m_Stack.size())
	{
	    return OpenChild(name);
	}
	int iBrother = CurNode().m_NextBrother;
	if(iBrother >= 0)
	{
	    m_Stack.back() = iBrother;
	    *name = m_aNode[iBrother].m_Name;
	    return 1;// found brother
	}
	return 0;// end
    }

    int OpenChild(pnl::pnlString *name)
    {
	int iChild;
	if(!m_Stack.size())
	{
	    if(!m_aNode.size())
	    {
		return 0; // end
	    }
	    iChild = 0;
	}
	else
	{
	    if(!CurNode().m_aiChild.size())
	    {
		return 0; // child not found
	    }
	    iChild = CurNode().m_aiChild[0];
	}
	m_Stack.push_back(iChild);
	*name = CurNode().m_Name;
	return 2; // found
    }

    pnl::pnlString GetAttribute(pnl::pnlString &name)
    {
	const struct Node &n = CurNode();
	for(int i = 0; i < n.m_aAttribute.size(); i += 2)
	{
	    if(name == n.m_aAttribute[i])
	    {
		return n.m_aAttribute[i + 1];
	    }
	}
	return pnl::pnlString();
    }

    pnl::pnlString GetContent()
    {
	return CurNode().m_Content;
    }
};

#endif // include guard
