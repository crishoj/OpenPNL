#ifndef __XMLREADERPNLH_HPP__
#define __XMLREADERPNLH_HPP__

#undef _DEBUG
#undef DEBUG
#include "windows.h"
#include <vector>
#include <string>
#include "network.h"
#include "constants.h"
#include "xmlreader.h"

#include "pnlConfig.hpp"
#include "pnlXMLRead.hpp"

#include "WInner.hpp"

typedef std::string StringGP;

namespace pnl
{
    class pnlString;
};

class XMLContainer;
class XmlBindingPNL;

class XmlReaderPNLH: public IXmlReader
{
public:// functions from base class

    virtual bool Parse(const char *filename, const IXmlBinding *root, IXmlErrorHandler *errorHandler);
    virtual void StopParse(const char* errorMessage);

    virtual void GetCurrentPosition(int &line, int &column);

    virtual const std::string& GetContent();
    virtual void GetUnescapedContent(std::string &unescaped);
    virtual void GetAttribute(const char *name, std::string &value);
    virtual bool GetAttribute(const char *name, bool &value, bool defValue);
    virtual bool GetAttribute(const char *name, int &value, int defValue);
    virtual bool GetAttribute(const char *name, double &value, double defValue);

    virtual IXmlBinding* CreateBinding(IXmlHandler *handler, const char *name, int validation);

    virtual void Delete()
    {
	delete this;
    }

public:// own functions
    XmlReaderPNLH(): m_Container(0) {}
    virtual ~XmlReaderPNLH() {}

    bool Parse(XMLContainer &container, const IXmlBinding *root);
    XMLContainer &Container() const { return *m_Container; }
    void RecursiveHandling(std::vector<const XmlBindingPNL *> &aBinding);

private:
    XMLContainer *m_Container;
    std::string m_TmpString;
};

#endif // include guard
