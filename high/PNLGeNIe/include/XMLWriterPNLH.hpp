#ifndef __XMLWRITERPNLH_HPP__
#define __XMLWRITERPNLH_HPP__

#undef _DEBUG
#undef DEBUG
#include <stdio.h>
#include <vector>
#include "xmlwriter.h"
#include "typesPNLH.hpp"

// FORWARDS
namespace pnl
{
    class CXMLWriter;
};

class XmlWriterPNLH: public IXmlWriter
{
public:
    XmlWriterPNLH(pnl::CXMLWriter *writer): m_pRealWriter(writer) {}
    XmlWriterPNLH(): m_pRealWriter(0) {}
    virtual bool OpenFile(const char * filename);
    virtual bool CloseFile();

    virtual void OpenElement(const char * name);
    virtual void CloseElement(const char * name);

    virtual void WriteElement(
	const char * name, 
	const char * content = NULL, 
	bool escapeWhitespace = false);

    virtual void PushAttribute(const char *name, const char * value);
    virtual void PushAttribute(const char *name, int value);
    virtual void PushAttribute(const char *name, bool value);
    virtual void PushAttribute(const char *name, double value);

    virtual void Delete() { delete this; }

private:
    pnl::CXMLWriter *m_pRealWriter;
};

#endif // include guard
