// enginefactory.h

#ifndef GENIE_INTERFACES_ENGINEFACTORY_H
#define GENIE_INTERFACES_ENGINEFACTORY_H

// contract between genie and engine dll
// engine dll should export method with the following singature
// extern "C" __declspec(dllexport) IEngineFactory* __cdecl GetFactory();

class INetwork;
class ILearning;
class IStructureMatrix;
class IMechLibrary;
class IXmlReader;
class IXmlWriter;

class IEngineFactory
{
public:
	virtual INetwork* CreateNetwork() = 0;
	virtual ILearning* CreateLearning() = 0;
	virtual IStructureMatrix* CreateStructureMatrix() = 0;
	virtual IMechLibrary* CreateMechLibrary() = 0;
	virtual IXmlReader* CreateXmlReader() = 0;
	virtual IXmlWriter* CreateXmlWriter() = 0;
};

#endif // !GENIE_INTERFACES_SERIALIZER_H
