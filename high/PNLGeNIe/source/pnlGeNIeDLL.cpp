#include "enginefactory.h"
#include "pnlGeNIe.hpp"

extern "C" __declspec(dllexport) IEngineFactory *__cdecl GetFactory();

class EngineFactoryPNLH: public IEngineFactory
{
public:
    virtual INetwork* CreateNetwork() { return new INetworkPNL; }
    virtual IStructureMatrix* CreateStructureMatrix() { return 0; }
    virtual IMechLibrary* CreateMechLibrary() { return 0; }
    virtual IXmlReader* CreateXmlReader() { return 0; }
    virtual IXmlWriter* CreateXmlWriter() { return 0; }
};

static IEngineFactory *sPNLHFactory;

IEngineFactory *GetFactory()
{
    if(!sPNLHFactory)
    {
	sPNLHFactory = new EngineFactoryPNLH;
    }

    return sPNLHFactory;
}
