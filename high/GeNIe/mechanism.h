// mechanism.h
#ifndef GENIE_INTERFACES_MECHANISM_H
#define GENIE_INTERFACES_MECHANISM_H

class IErrorOutput;
class ISerializer;
class IXmlWriter;
class IXmlBinding;

class ITradeoffAnalyzer;

// IMechanism and ISubsystem aren't defined anywhere
// they're used only to provide type-safe pointers used
// for IMechLibrary method parameters
class IMechanism;
class ISubsystem;

// IMPORTANT:
// names used for mech- and subsystem-related methods
// are identical to make template programming tasks
// related to mechs and subsystems easier
// for example, there are two GetId methods:
// one accepting IMechanism*, another for ISubsystem*


class IMechLibrary
{
public:
	virtual void Delete() = 0;

	virtual void SetErrorOutput(IErrorOutput *errOut) = 0;

	virtual bool Load(const std::string & filename) = 0;
	virtual bool Save(const std::string & filename) = 0;

	virtual ISubsystem* GetRootSubsystem() = 0;
	virtual void GetChildren(const ISubsystem *subsystem, std::vector<IMechanism *> &mechs) = 0;
	virtual void GetChildren(const ISubsystem *subsystem, std::vector<ISubsystem *> &subsystems) = 0;
	
	virtual const std::string & GetId(const ISubsystem *sub) = 0;
	virtual const std::string & GetName(const ISubsystem *sub) = 0;
	virtual const std::string & GetDescription(const ISubsystem *sub) = 0;
	virtual bool SetId(ISubsystem *sub, const std::string & id) = 0;
	virtual void SetName(ISubsystem *sub, const std::string & name) = 0;
	virtual void SetDescription(ISubsystem *sub, const std::string & description) = 0;

	virtual const std::string & GetId(const IMechanism *mech) = 0;
	virtual const std::string & GetName(const IMechanism *mech) = 0;
	virtual const std::string & GetDescription(const IMechanism *mech) = 0;
	virtual const std::string & GetEquation(const IMechanism *mech) = 0;
	virtual bool SetId(IMechanism *mech, const std::string & id) = 0;
	virtual void SetName(IMechanism *mech, const std::string & name) = 0;
	virtual void SetDescription(IMechanism *mech, const std::string & description) = 0;
	virtual bool SetEquation(IMechanism *mech, const std::string & equation) = 0;

	virtual void GetVariables(IMechanism *mech, std::vector<std::string> &vars) = 0;
	virtual const std::string & GetVarName(const IMechanism *mech, const std::string &var) = 0;
	virtual const std::string & GetVarDescription(const IMechanism *mech, const std::string &var) = 0;
	virtual bool IsVarManipulable(const IMechanism *mech, const std::string &var) = 0;
	virtual bool IsVarObservable(const IMechanism *mech, const std::string &var) = 0;
	virtual bool IsVarEffective(const IMechanism *mech, const std::string &var) = 0;

	virtual void SetVarName(IMechanism *mech, const std::string & var, const std::string &name) = 0;
	virtual void SetVarDescription(IMechanism *mech, const std::string & var, const std::string &desc) = 0;
	virtual void SetVarManipulable(IMechanism *mech, const std::string & var, bool manipulable) = 0;
	virtual void SetVarObservable(IMechanism *mech, const std::string & var, bool observable) = 0;
	virtual void SetVarEffective(IMechanism *mech, const std::string & var, bool effective) = 0;

	virtual void AddChild(ISubsystem *parent, const std::string &id, IMechanism *&child) = 0;
	virtual void AddChild(ISubsystem *parent, const std::string &id, ISubsystem *&child) = 0;
	virtual void DeleteChild(ISubsystem *parent, IMechanism *mechanism) = 0;
	virtual void DeleteChild(ISubsystem *parent, ISubsystem *subsystem) = 0;
	
	virtual IMechanism* CopyMechanism(ISubsystem *parent, IMechanism *sourceMech) = 0;
	virtual ISubsystem* CopySubsystem(ISubsystem *parent, ISubsystem *sourceSubsys) = 0;

	virtual bool MechanismsEqual(IMechanism *mech1, IMechanism *mech2) = 0;
};

class IStructureMatrix
{
public:
	virtual void Delete() = 0;

	virtual bool InstantiateMechanism(const IMechanism *mechanism) = 0;
	virtual IStructureMatrix* Clone() = 0;
	virtual void Serialize(ISerializer *serializer) = 0;
	virtual void Deserialize(ISerializer *serializer) = 0;
	
	virtual void SetErrorOutput(IErrorOutput *errOut) = 0;
	virtual void GetVariableIds(std::vector<std::string> &varIds) = 0;
	virtual void GetUndeterminedVariables(std::vector<std::string> &varIds) = 0;
	virtual void GetVarName(const std::string &id, std::string & name) = 0;
	virtual void GetVarDescription(const std::string &id, std::string & description) = 0;
	virtual void GetVarEquation(const std::string & id, std::string & equation) = 0;
	virtual void GetVarMechanism(const std::string & id, std::string & mechanism) = 0;

	virtual void GetCausalMapping(std::vector<std::pair<std::string, std::vector<std::string> > > &causal) = 0;
	virtual bool MakeExogenous(const std::string & id, double value, const std::string & mechToRelease) = 0;
	virtual bool ReleaseExogenous(const std::string &id) = 0;
	virtual void GetMechanisms(std::vector<std::string> &mechanisms) = 0;
	virtual void GetReleasableMechanisms(const std::string & variable, std::vector<std::string> &mechanisms) = 0;
	virtual void GetMechanismEquation(const std::string & mechanism, std::string &equation) = 0;
	virtual void GetMechanismName(const std::string & mechanism, std::string &name) = 0;
	virtual bool RemoveMechanism(const std::string & mechanism) = 0;
	
	virtual bool ChangeVariableId(const std::string &oldId, const std::string &newId) = 0;
	virtual bool ChangeVariableName(const std::string &id, const std::string &name) = 0;
	virtual bool IsSelfContained() = 0;
	virtual bool Merge(const std::string & varId,  IStructureMatrix *matrix2, const std::string & varId2) = 0;
	virtual bool ReplaceVariable(const std::string &id1, const std::string &id2) = 0;
	virtual bool IsExogenous(const std::string & varId) = 0;
	virtual bool IsEndogenous(const std::string & varId) = 0;
	virtual bool IsManipulable(const std::string & varId) = 0;
	virtual bool Solve(std::vector<std::pair<std::string, std::pair<double, double> > > &values) = 0;
	virtual void GetMatrix(std::vector<std::string> &variables, std::vector<std::string> &equations, std::vector<bool> &values) = 0;
	virtual void GetControllableCandidates(const std::string & mechanism, std::vector<std::string> & candidates) = 0;
	
	virtual void XmlWrite(IXmlWriter *xmlWriter) = 0;
	virtual IXmlBinding* CreateXmlBinding() = 0;

	virtual ITradeoffAnalyzer* CreateTradeoffAnalyzer() = 0;
};

class ITradeoffAnalyzer
{
public:
	virtual void Delete() = 0;

	enum 
	{
		TradeoffDimStepCount,
		TradeoffDimInterval
	};

	struct DimInfo
	{
		std::string exovarId;
		double rangeFrom;
		double rangeTo;
		int mode;
		double step;
	};

	virtual void AddDimension(const DimInfo &di) = 0;
	virtual bool Analyze() = 0;
	virtual int GetDimensionCount() = 0;
	virtual void GetDimension(int dimIndex, DimInfo &di) = 0;
	virtual int GetMaxDimensionIndex(int dimIndex) = 0;

	virtual void GetPosition(const std::vector<int> &odometer, std::vector<double> &position) = 0;
	virtual double GetMean(const std::string &endoVar, const std::vector<int> &odometer) = 0;
	virtual double GetStdDev(const std::string &endoVar, const std::vector<int> &odometer) = 0;
};

#endif // GENIE_INTERFACES_MECHANISM_H
