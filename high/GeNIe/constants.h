// constants.h

#ifndef GENIE_INTERFACES_CONSTANTS_H
#define GENIE_INTERFACES_CONSTANTS_H

namespace NetConst
{
	enum NodeType
	{
		Chance = 1,
		ChanceNoisyMax,
		ChanceNoisyAdder,
		Cast,
		Deterministic,
		Decision,
		Utility,
		Mau,
		Equation,
	};

	enum DiagNodeType
	{
		Target = 1,
		Observation,
		Auxiliary
	};

	enum NodeValueStatus
	{
		NotUpdated = 1,
		Updated,
		Evidence,
		PropagatedEvidence,
		Controlled,
	};

	enum BnAlgorithm
	{
		Clustering = 1,
		RelevanceDecomp,
		Polytree,
		LogicSampling,
		BackwardSampling,
		LikelihoodSampling,
		AisSampling,
		SelfImportance,
		EpisSampling,
	};

	enum IdAlgorithm
	{
		PolicyEvaluation = 1,
		FindBestPolicy
	};

	enum MultipleFaultAlgorithm
	{
		Marginal1 = 1,
		Marginal2,
		DependenceAtLeastOne,
		DependenceOnlyOne,
		DependenceOnlyAll,
		IndependenceAtLeastOne,
		IndependenceOnlyOne,
		IndependenceOnlyAll,
	};

	// this is basically used only on GeNIe side,
	// but we need these IDs for saving files in 
	// non-native formats (like classic non-xml .dsl)
	enum SpecialNameFormat
	{
		UserDefined,
		NodeName,
		StateName,
		NodeAndStateName,
		Inherit
	};

	enum NoisyAdderFunction
	{
		FuncAverage = 1,
		FuncSingleFault,
	};
}

#endif	// GENIE_INTERFACES_CONSTANTS_H

