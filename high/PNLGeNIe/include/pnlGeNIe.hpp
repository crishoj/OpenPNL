#ifndef __PNLGENIE_HPP__
#define __PNLGENIE_HPP__

#undef _DEBUG
#undef DEBUG
#include "windows.h"
#include <vector>
#include <string>
#include "network.h"
#include "constants.h"
#include "xmlreader.h"

#include "pnlXMLRead.hpp"

typedef std::string StringGP;

// FORWARDS
class BayesNet;
class WGraph;
class TokenCover;
class WDistributions;
class WEvidence;
class ProbabilisticNet;

namespace pnl
{
    class Log;
    class LogDrvStream;
    class pnlString;
};

class INetworkPNL: public INetwork
{
public:
    typedef std::vector<std::pair<std::string, std::string> > PropertyMap;
    // high-level operations
    virtual void Delete();
    INetworkPNL();
    
    virtual const char* GetId() { return m_NetName.c_str(); }
    virtual bool SetId(const char*id) { m_NetName.assign(id); return true; }
    
    virtual void SetErrorOutput(IErrorOutput *errorOutput) { m_ErrorOutput = errorOutput; }
    virtual bool Load(const char *filename, IXmlBinding *externalBinding);
    virtual bool Save(const char *filename, IXmlWriterExtension *externalExtension,
	const LegacyDslFileInfo *gnet);
    virtual bool UpdateBeliefs();
    
    virtual void SetBnAlgorithm(int algorithm);
    virtual int GetBnAlgorithm();
    virtual void SetIdAlgorithm(int algorithm);
    virtual int GetIdAlgorithm();
    
    virtual bool SetAlgorithmParams(int algorithm, const std::vector<double> &params);
    virtual void GetAlgorithmParams(int algorithm, std::vector<double> &params);
    virtual bool ValidateAlgorithmParams(int algorithm, const std::vector<double> &params);
    virtual void GetDefaultAlgorithmParams(int algorithm, std::vector<double> &params);
    
    virtual bool GetUpdateImmediately();
    virtual void SetUpdateImmediately(bool immediate);
    
    virtual int GetNumberOfSamples();
    virtual void SetNumberOfSamples(int samples);
    
    virtual void GetNetworkProperties(PropertyMap &mp);
    virtual void SetNetworkProperties(const PropertyMap &mp);
    
    virtual void InvalidateValues();
    virtual void ClearAllEvidence();
    virtual void ClearAllDecisions();
    virtual void ClearAllTargets();
    
    virtual IDiagNetwork* CreateDiagNetwork();
    virtual void SetEntropyCostRatio(double alpha, double alphaMax);
    
    virtual ISearchForOpportunities* CreateSfo();
    
    virtual bool Voi(int targetNode, std::vector<std::pair<int, double> > &observations);
    
    virtual bool Voi(
	const std::vector<int> & chanceNodes, 
	int decision, 
	int pointOfView, 
	std::vector<int> & parents,
	std::vector<double> & values);
    
    virtual bool GenerateDataFile(const GenerateDataFileParams &params);
    
    virtual bool AnnealedMap(
	const AnnealedMapParams& params,
	const std::vector<std::pair<int, int> > &evidence,
	const std::vector<int> &mapNodes, 
	std::vector<int> &mapStates,
	double &probM1E, double &probE);
    
    virtual void GetActionsAndDecisions(std::vector<int> &actions, std::vector<int> &decisions);
    
    virtual void GetNodes(std::vector<int> &nodes);
    
    virtual void GetTerminalUtilityMinMax(double &minUtility, double &maxUtility);
    
    // node creation/destruction
    virtual int AddNode(int nodeType, const char *nodeId);
    virtual void DeleteNode(int node);
    
    // arc creation/destrucion
    virtual bool AddArc(int nodeFrom, int nodeTo);
    virtual void DeleteArc(int nodeFrom, int nodeTo);
    
    // ACCESSORS - node properties
    virtual void GetParents(int node, std::vector<int> &parents);
    virtual int GetParentCount(int node);
    virtual int FindParent(int node, int parentNode);
    virtual void GetChildren(int node, std::vector<int> &children);
    virtual int GetChildrenCount(int node);
    virtual int FindChild(int node, int childNode);
    virtual int GetOutcomeCount(int node);
    virtual const char* GetOutcomeId(int node, int outcomeIndex);
    virtual const char* GetOutcomeLabel(int node, int outcomeIndex);
    virtual void GetDefinition(int node, std::vector<double> &definition);
    virtual void GetValue(int node, bool &valueValid, std::vector<int> &parents, std::vector<double> &values);
    virtual int GetValueParentCount(int node);
    virtual bool GetValueParents(int node, std::vector<int> &parents);
    virtual int GetEvidence(int node);
    virtual int GetControlledValue(int node);
    virtual int GetNodeType(int node);
    virtual bool IsIdUnique(const char *nodeId, int nodeToIgnore);
    virtual int FindNode(const char * id);
    virtual const char* GetNodeId(int node);
    virtual bool IsTarget(int node);
    virtual bool IsEvidence(int node);
    virtual bool IsRealEvidence(int node);
    virtual bool IsPropagatedEvidence(int node);
    virtual bool IsValueValid(int node);
    virtual bool IsControlled(int node);
    virtual int GetNodeValueStatus(int node);
    virtual bool GetMinMaxUtility(int node, double &minUtility, double &maxUtility);
    
    virtual void GetProperties(int node, PropertyMap &mp);
    
    // MUTATORS
    virtual bool SetNodeId(int node, const char *id);
    virtual void SetNodeType(int node, int type);
    virtual void AddOutcome(int node, int outcomeIndex, const char *outcomeId);
    virtual void DeleteOutcome(int node, int outcomeIndex);
    virtual void SetDefinition(int node, const std::vector<double> & definition);
    virtual bool SetOutcomeId(int node, int outcomeIndex, const char *id);
    virtual void SetOutcomeIds(int node, const std::vector<std::string> &ids);
    virtual bool SetOutcomeLabel(int node, int outcomeIndex, const char *label);
    virtual void SetOutcomeLabels(int node, const std::vector<std::string> &labels);
    
    virtual void SetEvidence(int node, int outcomeIndex);
    virtual void ClearEvidence(int node);
    virtual void SetControlledValue(int node, int outcomeIndex);
    virtual void ClearControlledValue(int node);
    virtual bool IsControllable(int node);
    virtual void SetTarget(int node, bool asTarget);
    virtual void SetProperties(int node, const PropertyMap &map);
    
    // Noisy-related functions
    virtual void NoisyToCpt(
	int node, 
	int outcomeCount, 
	const std::vector<int> &parentPermutation,
	const std::vector<int> &noisyMaxStrengths,
	const std::vector<double> &noisyAdderWeights,
	const std::vector<int> &noisyAdderDistStates,
	int adderFunction,
	const std::vector<double> &noisy, 
	std::vector<double> &cpt);
    virtual void NoisyToHenrion(
	int node,
	int outcomeCount,
	const std::vector<int> &parentPermutation,
	const std::vector<double> &noisy, 
	std::vector<double> &henrion);
    virtual void NoisyFromHenrion(
	int node,
	int outcomeCount,
	const std::vector<int> &parentPermutation,
	const std::vector<double> &henrion, 
	std::vector<double> &noisy);
    // noisymax
    virtual void NoisyGetParentStrengths(int node, int parentIndex, std::vector<int> &strengths);
    virtual void NoisySetParentStrengths(int node, int parentIndex, const std::vector<int> &strengths);
    // noisyadder
    virtual void NoisyGetDistStates(int node, std::vector<int> &distStates);
    virtual void NoisySetDistStates(int node, const std::vector<int> &distStates);
    virtual void NoisyGetWeights(int node, std::vector<double> &weights);
    virtual void NoisySetWeights(int node, const std::vector<double> &weights);
    virtual int NoisyGetFunction(int node);
    virtual void NoisySetFunction(int node, int function);
    
    // equation-related functions
    virtual bool SetEquation(int node, const char *equation);
    virtual const char* GetEquation(int node);
    virtual bool ValidateEquation(int node, const char *alternateId, const char *equation, std::string &errMsg);
    virtual bool ReplaceVariableEquation(std::string &equation, const char *oldVar, const char *newVar);
    
    // copy/paste and drag'n'drop support
    virtual bool CopyNodes(const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds);
    virtual bool CopyNodesFrom(const INetwork* sourceNetwork, const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds);
    virtual bool Serialize(
	ISerializer *serializer,
	const std::vector<int> &nodes,
	const std::vector<std::pair<int, UUID> > &uuids);
    virtual bool Deserialize(
	ISerializer *serializer, bool native,
	std::vector<std::pair<int, int> > &createdNodes,
	const std::vector<std::pair<int, UUID> > &uuids,
	const std::vector<std::string> &externalIds);
    
    // reordering parents
    virtual void ReorderParents(int node, const std::vector<int> & newOrder);
    
    // syncing outcomes with definition grid
    virtual void AddRemoveReorderOutcomes(
	int node, 
	const std::vector<std::string> &outcomeIds,
	const std::vector<int> &permutation);
    
    // qualitative stuff
    virtual double QualGetWeight(int node, int parentIndex, bool positive);
    virtual void QualSetWeight(int node, int parentIndex, bool positive, double weight);
    virtual double QualGetBaseline(int node);
    virtual void QualSetBaseline(int node, double baseline);
    virtual double QualGetValue(int node);
    virtual int QualGetArcSign(int node, int parentIndex);
    
    // diagnosis support
    virtual bool AddCostArc(int nodeFrom, int nodeTo);
    virtual void DeleteCostArc(int nodeFrom, int nodeTo);
    virtual void GetCostParents(int node, std::vector<int> &parents);
    virtual int GetCostParentCount(int node);
    virtual int FindCostParent(int node, int parentNode);
    virtual void GetCostChildren(int node, std::vector<int> &children);
    virtual int GetCostChildrenCount(int node);
    virtual int FindCostChild(int node, int childNode);
    virtual void GetCost(int node, std::vector<double> &cost);
    virtual void SetCost(int node, const std::vector<double> &cost);
    virtual double GetNotAvailableCostValue();
    virtual double GetNotRelevantTestValue();
    virtual void GetGroupCostProperty(std::string &name, std::string &value);
    virtual int GetDiagType(int node);
    virtual void SetDiagType(int node, int diagType);
    virtual void SetRanked(int node, bool ranked);
    virtual bool IsRanked(int node);
    virtual void SetMandatory(int node, bool mandatory);
    virtual bool IsMandatory(int node);
    virtual int GetDefaultOutcome(int node);
    virtual void SetDefaultOutcome(int node, int outcome);
    virtual bool IsFaultOutcome(int node, int outcome);
    virtual void SetFaultOutcome(int node, int outcome, bool fault);
    
    // learning 
    virtual bool LearnStructureAndParams(const char *dataFile, const char *networkFile);

protected:
    void MarkCallFunction(const char *name, bool bRealized = false, const char *args = 0);
    void SetValues(int iNode, const std::vector<std::string> &aId);
    ProbabilisticNet &Net() const;
    WGraph *Graph() const;
    TokenCover *Token() const;
    WDistributions *Distributions() const;

private:
    IErrorOutput *m_ErrorOutput;		// INetwork requires implycitly this pointer
    BayesNet *m_pWNet;				// Doesn't used in this time
    StringGP m_NetName;				// name of net
    StringGP m_Bad;				// stub (returned when nothing to return)
    PropertyMap m_NetProperty;			// properties for whole net
    std::vector<PropertyMap> m_aNodeProperty;	// properties for each node
    pnl::Log *m_pLog;				// log driver (head) for debugging purposes
    pnl::LogDrvStream *m_pLogStream;		// stream driver for debugging purposes
    std::vector<NetConst::NodeValueStatus> m_aNodeValueStatus;// nodevalue status for every node
    WEvidence *m_aEvidence;
    std::vector<bool> m_abEvidence;
};

class XMLContainer;

class XmlReaderPNLH: public IXmlReader
{
public:
    XmlReaderPNLH(): m_Container(0) {}
    virtual ~XmlReaderPNLH() {}

    virtual bool Parse(const char *filename, const IXmlBinding *root, IXmlErrorHandler *errorHandler);
    virtual bool Parse(XMLContainer &container, const IXmlBinding *root, IXmlErrorHandler *errorHandler);
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

private:
    XMLContainer *m_Container;
};

// This container filters output to underlying container.
// It filters by 'elementName'.
// The element with pointed name with all sub-elements is directed to 'output' container.
class XMLContainerFilter: public pnl::CXMLContainer
{
public:
    XMLContainerFilter(pnl::CXMLContainer *output, const char *elementName)
	: m_CurrentLevel(0), m_LevelOn(-1), m_Output(output), m_ElementName(name)
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
	if(--m_CurrentLevel == m_LevelOn)
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
    struct Node
    {
	Node() {}

	pnl::pnlString m_Name;
	pnl::pnlString m_Content;
	std::vector<pnl::pnlString> m_aAttribute;
	std::vector<int> m_aiChild;
    };

public:
    XMLContainer() {}
    virtual ~XMLContainer() {}

    virtual void CreateNode(const pnl::pnlString &name)
    {
	int iNode = m_aNode.size();
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
	m_Stack.pop_back();
    }

private:
    struct Node &LastElement()
    {
	return m_aNode.back();
    }

    pnl::pnlVector<struct Node> m_aNode;
    pnl::pnlVector<int> m_Stack;
};

#endif
