#undef _DEBUG
#include <vector>

#include "enginefactory.h"
#include "pnlGeNIe.hpp"
#include "xmlreader.h"
#include "xmlwriter.h"
#include "pnlHighConf.hpp"
#include "pnl_dll.hpp"
#include "pnlString.hpp"

extern "C" __declspec(dllexport) IEngineFactory *__cdecl GetFactory();

// FORWARDS
class IXmlWriterPNLH;
class IXmlReaderPNLH;
class INetworkPNL;

class IXmlWriterPNLH: public IXmlWriter
{
public:
    IXmlWriterPNLH(): m_pFile(0) {}
    virtual ~IXmlWriterPNLH() { if(m_pFile) fclose(m_pFile); }
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
    virtual void Delete();

private:
    FILE *m_pFile;
    std::vector<pnl::pnlString> m_aElement;
};

class EngineFactoryPNLH: public IEngineFactory
{
public:
    virtual INetwork* CreateNetwork() { return new INetworkPNL; }
    virtual IStructureMatrix* CreateStructureMatrix() { return 0; }
    virtual IMechLibrary* CreateMechLibrary() { return 0; }
    virtual IXmlReader* CreateXmlReader() { return 0; }
    virtual IXmlWriter* CreateXmlWriter() { return new IXmlWriterPNLH; }
};

static IEngineFactory *sPNLHFactory;

IEngineFactory *GetFactory()
{
    return new EngineFactoryPNLH;
    if(!sPNLHFactory)
    {
	sPNLHFactory = new EngineFactoryPNLH;
    }

    return sPNLHFactory;
}

bool IXmlWriterPNLH::OpenFile(const char * filename)
{
    m_pFile = fopen(filename, "w");
    
    return m_pFile != 0;
}

bool IXmlWriterPNLH::CloseFile()
{
    if(!m_pFile)
    {
	return false;
    }

    fclose(m_pFile);

    return true;
}

void IXmlWriterPNLH::OpenElement(const char * name)
{
    m_aElement.push_back(name);
    fprintf(m_pFile, "<%s", name);
}

void IXmlWriterPNLH::CloseElement(const char * name)
{
    if(!m_aElement.size() || !(m_aElement.back() == pnl::pnlString(name)))
    {
	ThrowInternalError("Unopen element is closed",
	    "XmlWriterPNLH::CloseElement");
    }

    fprintf(m_pFile, "</%s>", name);
    m_aElement.pop_back();
}

void IXmlWriterPNLH::WriteElement(
	const char * name, 
	const char * content, 
	bool escapeWhitespace)
{
    // Now we ignore 'escapeWhitespace' argument

    fprintf(m_pFile, ">%s", content);
}

void IXmlWriterPNLH::PushAttribute(const char *name, const char * value)
{
    fprintf(m_pFile, " %s='%s'", name, value);
}

void IXmlWriterPNLH::PushAttribute(const char *name, int value)
{
    fprintf(m_pFile, " %s='%i'", name, value);
}

void IXmlWriterPNLH::PushAttribute(const char *name, bool value)
{
    fprintf(m_pFile, " %s='%c'", name, value ? 'y':'n');
}

void IXmlWriterPNLH::PushAttribute(const char *name, double value)
{
    fprintf(m_pFile, " %s='%.8lf'", name, value);
}

void IXmlWriterPNLH::Delete()
{
    delete this;
}

#if 0
class 
#endif
