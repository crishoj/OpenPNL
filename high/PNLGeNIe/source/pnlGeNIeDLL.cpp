#undef _DEBUG
#undef DEBUG
#include <vector>

#include "enginefactory.h"
#include "mechanism.h"

#include "XMLReaderPNLH.hpp"
#include "XMLWriterPNLH.hpp"
#include "XMLBindingPNLH.hpp"
#include "networkPNLH.hpp"

extern "C" __declspec(dllexport) IEngineFactory *__cdecl GetFactory();

class StructureMatrixPNLH: public IStructureMatrix
{
public:
    virtual void Delete()
    {
	delete this;
    }

    virtual bool InstantiateMechanism(const IMechanism *mechanism) { return false; }
    virtual IStructureMatrix* Clone() { return new StructureMatrixPNLH(); }
    virtual void Serialize(ISerializer *serializer) {}
    virtual void Deserialize(ISerializer *serializer) {}

    virtual void SetErrorOutput(IErrorOutput *errOut) {}
    virtual void GetVariableIds(std::vector<std::string> &varIds) {}
    virtual void GetUndeterminedVariables(std::vector<std::string> &varIds) {}
    virtual void GetVarName(const std::string &id, std::string & name) {}
    virtual void GetVarDescription(const std::string &id, std::string & description) {}
    virtual void GetVarEquation(const std::string & id, std::string & equation) {}
    virtual void GetVarMechanism(const std::string & id, std::string & mechanism) {}

    virtual void GetCausalMapping(std::vector<std::pair<std::string, std::vector<std::string> > > &causal) {}
    virtual bool MakeExogenous(const std::string & id, double value, const std::string & mechToRelease) { return false; }
    virtual bool ReleaseExogenous(const std::string &id) { return false; }
    virtual void GetMechanisms(std::vector<std::string> &mechanisms) {}
    virtual void GetReleasableMechanisms(const std::string & variable, std::vector<std::string> &mechanisms) {}
    virtual void GetMechanismEquation(const std::string & mechanism, std::string &equation) {}
    virtual void GetMechanismName(const std::string & mechanism, std::string &name) {}
    virtual bool RemoveMechanism(const std::string & mechanism) { return false; }

    virtual bool ChangeVariableId(const std::string &oldId, const std::string &newId) { return false; }
    virtual bool ChangeVariableName(const std::string &id, const std::string &name) { return false; }
    virtual bool IsSelfContained() { return false; }
    virtual bool Merge(const std::string & varId,  IStructureMatrix *matrix2, const std::string & varId2) { return false; }
    virtual bool ReplaceVariable(const std::string &id1, const std::string &id2) { return false; }
    virtual bool IsExogenous(const std::string & varId) { return false; }
    virtual bool IsEndogenous(const std::string & varId) { return false; }
    virtual bool IsManipulable(const std::string & varId) { return false; }
    virtual bool Solve(std::vector<std::pair<std::string, std::pair<double, double> > > &values) { return false; }
    virtual void GetMatrix(std::vector<std::string> &variables, std::vector<std::string> &equations, std::vector<bool> &values) {}
    virtual void GetControllableCandidates(const std::string & mechanism, std::vector<std::string> & candidates) {}

    virtual void XmlWrite(IXmlWriter *xmlWriter) {}
    virtual IXmlBinding* CreateXmlBinding() { return 0; }

    virtual ITradeoffAnalyzer* CreateTradeoffAnalyzer() { return 0; }
};

class EngineFactoryPNLH: public IEngineFactory
{
public:
    virtual INetwork* CreateNetwork() { return new NetworkPNL; }
    virtual ILearning* CreateLearning() { return 0; }
    virtual IStructureMatrix* CreateStructureMatrix() { return new StructureMatrixPNLH; }
    virtual IMechLibrary* CreateMechLibrary() { return 0; }
    virtual IXmlReader* CreateXmlReader() { return new XmlReaderPNLH; }
    virtual IXmlWriter* CreateXmlWriter() { return new XmlWriterPNLH; }
};

IEngineFactory *GetFactory()
{
    return new EngineFactoryPNLH;
}
