#include "XMLWriterPNLH.hpp"
#undef _DEBUG
#undef DEBUG
#include "pnlConfig.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistence.hpp"
#include "pnlXMLRead.hpp"
#include "pnlXMLWrite.hpp"

bool XmlWriterPNLH::OpenFile(const char * filename)
{
    return m_pRealWriter->OpenFile(filename);
}

bool XmlWriterPNLH::CloseFile()
{
    return m_pRealWriter->CloseFile();
}

void XmlWriterPNLH::OpenElement(const char * name)
{
    m_pRealWriter->OpenElement(name);
}

void XmlWriterPNLH::CloseElement(const char * name)
{
    m_pRealWriter->CloseElement(name);
}

void XmlWriterPNLH::WriteElement(
	const char * name, 
	const char * content, 
	bool escapeWhitespace)
{
    m_pRealWriter->WriteElement(name, content, escapeWhitespace);
}

void XmlWriterPNLH::PushAttribute(const char *name, const char * value)
{
    m_pRealWriter->PushAttribute(name, value);
}

void XmlWriterPNLH::PushAttribute(const char *name, int value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}

void XmlWriterPNLH::PushAttribute(const char *name, bool value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}

void XmlWriterPNLH::PushAttribute(const char *name, double value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}
