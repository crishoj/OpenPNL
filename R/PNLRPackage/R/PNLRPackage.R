CreateBNet <- function ()
{
	result <- .Call("pnlCreateBNet")
	class(result) <- "pnlBNet"
	result
}
pnlCreateBNet <- function() CreateBNet()

CreateDBN <- function ()
{
	result <- .Call("pnlCreateDBN")
	class(result) <- "pnlDBN"
	result
}
pnlCreateDBN <- function() CreateDBN()

Unroll <- function(x) UseMethod("Unroll", x)
Unroll.pnlDBN <- function(x)
{
	result <- .Call("pnlUnroll", x)
	class(result) <- "pnlBNet"
	result
}

CreateLIMID <- function ()
{
	result <- .Call("pnlCreateLIMID")
	class(result) <- "pnlLIMID"
	result
}
pnlCreateLIMID <- function() CreateLIMID()

CreateMRF <- function ()
{
	result <- .Call("pnlCreateMRF")
	class(result) <- "pnlMRF"
	result
}
pnlCreateMRF <- function() CreateMRF()

AddNode <- function (x, names, values) UseMethod("AddNode", x)
AddNode.pnlBNet <- function(x, names, values) 
{
	res <- "ok"
	result <- .Call("pnlAddNode", x, 0, names, values)
	if (result < 0) invisible (result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddNode.pnlDBN <- function(x, names, values) 
{
	res <- "ok"
	result <- .Call("pnlAddNode", x, 1, names, values)
	if (result < 0) invisible (result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddNode.pnlLIMID <- function(x, names, values) 
{
	res <- "ok"
	result <- .Call("pnlAddNode", x, 2, names, values)
	if (result < 0) invisible (result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddNode.pnlMRF <- function(x, names, values) 
{
	res <- "ok"
	result <- .Call("pnlAddNode", x, 3, names, values)
	if (result < 0) invisible (result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

DelNode <- function(x, nodes) UseMethod("DelNode", x)
DelNode.pnlBNet <- function(x, nodes) 
{
	result <- "ok"
	res <- .Call("pnlDelNode", x, 0, nodes, result)
	if (res < 0) invisible(result)
	else 
	{
		result <- .Call("pnlReturnError")
		result
	}
}
DelNode.pnlDBN <- function(x, nodes) 
{
	result <- "ok"
	res <- .Call("pnlDelNode", x, 1, nodes, result)
	if (res < 0) invisible(result)
	else 
	{
		result <- .Call("pnlReturnError")
		result
	}
}

DelNode.pnlLIMID <- function(x, nodes) 
{
	result <- "ok"
	res <- .Call("pnlDelNode", x, 2, nodes, result)
	if (res < 0) invisible(result)
	else 
	{
		result <- .Call("pnlReturnError")
		result
	}
}
DelNode.pnlMRF <- function(x, nodes) 
{
	result <- "ok"
	res <- .Call("pnlDelNode", x, 3, nodes, result)
	if (res < 0) invisible(result)
	else 
	{
		result <- .Call("pnlReturnError")
		result
	}
}

AddArc <- function (x, Start, Finish) UseMethod("AddArc", x)
AddArc.pnlBNet <- function(x, Start, Finish) 
{
	res <- "ok"
	result <- .Call("pnlAddArc", x, 0, Start, Finish)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddArc.pnlDBN <- function(x, Start, Finish) 
{
	res <- "ok"
	result <- .Call("pnlAddArc", x, 1, Start, Finish)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddArc.pnlLIMID <- function(x, Start, Finish) 
{
	res <- "ok"
	result <- .Call("pnlAddArc", x, 2, Start, Finish)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


DelArc <- function(x, StartOfArc, EndOfArc) UseMethod("DelArc", x)
DelArc.pnlBNet <- function(x, StartOfArc, EndOfArc) 
{
	res <- "ok"
	result <- .Call("pnlDelArc", x, 0, StartOfArc, EndOfArc)
	if (result < 0) invisible(result)
	else
	{
		res <- .Call("pnlReturnError")
		res
	}
}
DelArc.pnlDBN <- function(x, StartOfArc, EndOfArc) 
{
	res <- "ok"
	result <- .Call("pnlDelArc", x, 1, StartOfArc, EndOfArc)
	if (result < 0) invisible(result)
	else
	{
		res <- .Call("pnlReturnError")
		res
	}
}
DelArc.pnlLIMID <- function(x, StartOfArc, EndOfArc) 
{
	res <- "ok"
	result <- .Call("pnlDelArc", x, 2, StartOfArc, EndOfArc)
	if (result < 0) invisible(result)
	else
	{
		res <- .Call("pnlReturnError")
		res
	}
}

SaveNet <- function(x, filename) UseMethod("SaveNet", x)
SaveNet.pnlBNet <- function(x, filename) 
{
	res <- "ok"
	result <- .Call("pnlSaveNet", x, 0, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SaveNet.pnlDBN <- function(x, filename) 
{
	res <- "ok"
	result <- .Call("pnlSaveNet", x, 1, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SaveNet.pnlLIMID <- function(x, filename) 
{
	res <- "ok"
	result <- .Call("pnlSaveNet", x, 2, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SaveNet.pnlMRF <- function(x, filename) 
{
	res <- "ok"
	result <- .Call("pnlSaveNet", x, 3, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


GetNodeType <- function(x, nodes) UseMethod("GetNodeType", x)
GetNodeType.pnlBNet <- function(x, nodes) .Call("pnlGetNodeType", x, 0, nodes)
GetNodeType.pnlDBN <- function(x, nodes) .Call("pnlGetNodeType", x, 1, nodes)
GetNodeType.pnlMRF <- function(x, nodes) .Call("pnlGetNodeType", x, 3, nodes)

GetNeighbors <- function(x, nodes) UseMethod("GetNeighbors", x)
GetNeighbors.pnlBNet <- function(x, nodes) .Call("pnlGetNeighbors", x, 0, nodes)
GetNeighbors.pnlDBN <- function(x, nodes) .Call("pnlGetNeighbors", x, 1, nodes)

GetParents <- function(x, nodes) UseMethod("GetParents", x)
GetParents.pnlBNet <- function(x, nodes) .Call("pnlGetParents", x, 0, nodes)
GetParents.pnlDBN <- function(x, nodes) .Call("pnlGetParents", x, 1, nodes)

GetChildren <- function(x, nodes) UseMethod("GetChildren", x)
GetChildren.pnlBNet <- function(x, nodes) .Call("pnlGetChildren", x, 0, nodes)
GetChildren.pnlDBN <- function(x, nodes) .Call("pnlGetChildren", x, 1, nodes)

SetPTabular <- function(x, value, probability, ParentValue) UseMethod("SetPTabular", x)
SetPTabular.pnlBNet <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetPTabular", x, 0, value, probability)
	else result <- .Call("pnlSetPTabularCond",x, 0, value, probability, ParentValue)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetPTabular.pnlDBN <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetPTabular", x, 1, value, probability)
	else result <- .Call("pnlSetPTabularCond",x, 1, value, probability, ParentValue)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetPTabular.pnlMRF <- function (x, value, probability)
{
	res <- "ok"
	result <- .Call("pnlSetPTabular", x, 3, value, probability)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


SetPChance <- function(x, value, probability, ParentValue) UseMethod("SetPChance", x)
SetPChance.pnlLIMID <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetPChance", x, value, probability)
	else result <- .Call("pnlSetPChanceCond",x, value, probability, ParentValue)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

SetPDecision <- function(x, value, probability, ParentValue) UseMethod("SetPDecision", x)
SetPDecision.pnlLIMID <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetPDecision", x, value, probability)
	else result <- .Call("pnlSetPDecisionCond",x, value, probability, ParentValue)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

SetValueCost <- function(x, value, probability, ParentValue) UseMethod("SetValueCost", x)
SetValueCost.pnlLIMID <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetValueCost", x, value, probability)
	else result <- .Call("pnlSetValueCostCond",x, value, probability, ParentValue)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
GetPTabularString <- function(x, value, parents) UseMethod("GetPTabularString", x)
GetPTabularString.pnlBNet <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPTabularString", x, 0, value)
	else .Call("pnlGetPTabularStringCond", x, 0, value, parents)
}
GetPTabularString.pnlDBN <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPTabularString", x, 1, value)
	else .Call("pnlGetPTabularStringCond", x, 1, value, parents)
}
GetPTabularString.pnlMRF <- function(x, value) .Call("pnlGetPTabularString", x, 3, value)

GetPTabularFloat <- function(x, value, parents) UseMethod("GetPTabularFloat", x)
GetPTabularFloat.pnlBNet <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPTabularFloat", x, 0, value)
	else .Call("pnlGetPTabularFloatCond", x, 0, value, parents)
}
GetPTabularFloat.pnlDBN <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPTabularFloat", x, 1, value)
	else .Call("pnlGetPTabularFloatCond", x, 1, value, parents)
}
GetPTabularFloat.pnlMRF <- function(x, value) .Call("pnlGetPTabularFloat", x, 3, value)

GetPChanceString <- function(x, value, parents) UseMethod("GetPChanceString", x)
GetPChanceString.pnlLIMID <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPChanceString", x, value)
	else .Call("pnlGetPChanceCondString", x, value, parents)
}

GetPChanceFloat <- function(x, value, parents) UseMethod("GetPChanceFloat", x)
GetPChanceFloat.pnlLIMID <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPChanceFloat", x, value)
	else .Call("pnlGetPChanceCondFloat", x, value, parents)
}

GetPDecisionString <- function(x, value, parents) UseMethod("GetPDecisionString")
GetPDecisionString <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPDecisionString", x, value)
	else .Call("pnlGetPDecisionCondString", x, value, parents)
}

GetPDecisionFloat <- function(x, value, parents) UseMethod("GetPDecisionFloat")
GetPDecisionFloat.pnlLIMID <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPDecisionFloat", x, value)
	else .Call("pnlGetPDecisionCondFloat", x, value, parents)
}

GetValueCostString <- function(x, value, parents) UseMethod("GetValueCostString")
GetValueCostString.pnlLIMID <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetValueCostString", x, value)
	else .Call("pnlGetValueCostCondString", x, value, parents)
}

GetValueCostFloat <- function(x, value, parents) UseMethod("GetValueCostFloat")
GetValueCostFloat.pnlLIMID <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetValueCostFloat", x, value)
	else .Call("pnlGetValueCostCondFloat", x, value, parents)
}

SetPGaussian <- function(x, node, mean, variance, weight, tabParents) UseMethod("SetPGaussian", x)
SetPGaussian.pnlBNet <- function(x, node, mean, variance, weight = -1, tabParents = -1)
{
	res <- "ok"
	if (tabParents < 0)
	{
		if (weight < 0)  result <- .Call("pnlSetPGaussian", x, 0, node, mean, variance)
		else result <- .Call("pnlSetPGaussianCond", x, 0, node, mean, variance, weight)
	}
	else result <- .Call("pnlSetPGaussianCondTParents", x, node, mean, variance, weight, tabParents)

	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetPGaussian.pnlDBN <- function(x, node, mean, variance, weight = -1)
{
	res <- "ok"
	if (weight < 0) result <- .Call("pnlSetPGaussian", x, 1, node, mean, variance)
	else result <- .Call("pnlSetPGaussianCond", x, 1, node, mean, variance, weight)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

SetPSoftMax <- function(x, node, weight, offset, parents) UseMethod("SetPSoftMax", x)
SetPSoftMax.pnlBNet <- function(x, node, weight, offset, parents = -1)
{
	res <- "ok"
	if (parents < 0) result <- .Call("pnlSetPSoftMax", x, node,  weight, offset)
	else result <- .Call("pnlSetPSoftMaxCond", x, node,  weight, offset, parents)

	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


EditEvidence <- function(x, values) UseMethod("EditEvidence", x)
EditEvidence.pnlBNet <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlEditEvidence", x, 0, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
EditEvidence.pnlDBN <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlEditEvidence", x, 1, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
EditEvidence.pnlMRF <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlEditEvidence", x, 3, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

ClearEvid <- function(x) UseMethod("ClearEvid", x)
ClearEvid.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvid", x, 0)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
ClearEvid.pnlDBN <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvid", x, 1)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
ClearEvid.pnlMRF <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvid", x, 3)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

CurEvidToBuf <- function(x) UseMethod("CurEvidToBuf", x)
CurEvidToBuf.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlCurEvidToBuf", x, 0)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
CurEvidToBuf.pnlDBN <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlCurEvidToBuf", x, 1)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
CurEvidToBuf.pnlMRF <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlCurEvidToBuf", x, 3)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

AddEvidToBuf <- function(x, values) UseMethod("AddEvidToBuf", x)
AddEvidToBuf.pnlBNet <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlAddEvidToBuf", x, 0, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddEvidToBuf.pnlDBN <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlAddEvidToBuf", x, 1, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
AddEvidToBuf.pnlMRF <- function(x, values) 
{
	res <- "ok"
	result <- .Call("pnlAddEvidToBuf", x, 3, values)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

ClearEvidBuf <- function(x) UseMethod("ClearEvidBuf", x)
ClearEvidBuf.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvidBuf", x, 0)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
ClearEvidBuf.pnlDBN <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvidBuf", x, 1)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
ClearEvidBuf.pnlMRF <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlClearEvidBuf", x, 3)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetMPE <- function(x, nodes) UseMethod("GetMPE", x)
GetMPE.pnlBNet <- function(x, nodes) .Call("pnlGetMPE",x, 0, nodes)
GetMPE.pnlDBN <- function(x, nodes) .Call("pnlGetMPE",x, 1, nodes)
GetMPE.pnlMRF <- function(x, nodes) .Call("pnlGetMPE",x, 3, nodes)

GetJPDString <- function(x, nodes) UseMethod("GetJPDString", x)
GetJPDString.pnlBNet <- function(x, nodes) .Call("pnlGetJPDString", x, 0, nodes)
GetJPDString.pnlDBN <- function(x, nodes) .Call("pnlGetJPDString", x, 1, nodes)
GetJPDString.pnlMRF <- function(x, nodes) .Call("pnlGetJPDString", x, 3, nodes)

GetJPDFloat <- function(x, nodes) UseMethod("GetJPDFloat", x)
GetJPDFloat.pnlBNet <- function(x, nodes) .Call("pnlGetJPDFloat", x, 0, nodes)
GetJPDFloat.pnlDBN <- function(x, nodes) .Call("pnlGetJPDFloat", x, 1, nodes)
GetJPDFloat.pnlMRF <- function(x, nodes) .Call("pnlGetJPDFloat", x, 3, nodes)

GetSoftMaxOffset <- function(x, node, ParentValue) UseMethod("GetSoftMaxOffset", x)
GetSoftMaxOffset.pnlBNet <- function(x, node, ParentValue = -1)
{
	if (ParentValue < 0) .Call("pnlGetSoftMaxOffset", x, node)
	else .Call("pnlGetSoftMaxOffsetCond", x, node, ParentValue)
}

GetSoftMaxWeights <- function(x, node, ParentValue) UseMethod("GetSoftMaxWeights", x)
GetSoftMaxWeights.pnlBNet <- function(x, node, ParentValue = -1)
{
	if (ParentValue < 0) .Call("pnlGetSoftMaxWeights", x, node)
	else .Call("pnlGetSoftMaxWeightsCond", x, node, ParentValue)
}

GetGaussianMean <- function(x, nodes, tabParents) UseMethod("GetGaussianMean", x)
GetGaussianMean.pnlBNet <- function(x, nodes, tabParents = -1) 
{
	if (tabParents < 0) .Call("pnlGetGaussianMean", x, 0, nodes)
	else .Call("pnlGetGaussianMeanCond", x, 0, nodes, tabParents)
}
GetGaussianMean.pnlDBN <- function(x, nodes) .Call("pnlGetGaussianMean", x, 1, nodes)

GetGaussianCovar <- function(x, nodes, tabParents) UseMethod("GetGaussianCovar", x)
GetGaussianCovar.pnlBNet <- function(x, nodes, tabParents = -1)
{
	if (tabParents < 0)  .Call("pnlGetGaussianCovar", x, 0, nodes)
	else  .Call("pnlGetGaussianCovarCond", x, 0, nodes, tabParents)
}
GetGaussianCovar.pnlDBN <- function(x, nodes) .Call("pnlGetGaussianCovar", x, 1, nodes)

GetGaussianWeights <- function(x, nodes, parents, tabParents) UseMethod("GetGaussianWeights", x)
GetGaussianWeights.pnlBNet <- function(x, nodes, parents, tabParents = -1)
{
	if (tabParents < 0) .Call("pnlGetGaussianWeights", x, 0, nodes, parents)
	else  .Call("pnlGetGaussianWeightsCond", x, 0, nodes, parents, tabParents)
}
GetGaussianWeights.pnlDBN <- function(x, nodes, parents) .Call("pnlGetGaussianWeights", x, 1, nodes, parents)

SetProperty <- function(x, name, value) UseMethod("SetProperty", x)
SetProperty.pnlBNet <- function(x, name, value) 
{
	res <- "ok"
	result <- .Call("pnlSetProperty", x, 0, name, value)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetProperty.pnlDBN <- function(x, name, value) 
{
	res <- "ok"
	result <- .Call("pnlSetProperty", x, 1, name, value)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetProperty.pnlLIMID <- function(x, name, value) 
{
	res <- "ok"
	result <- .Call("pnlSetProperty", x, 2, name, value)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
SetProperty.pnlMRF <- function(x, name, value) 
{
	res <- "ok"
	result <- .Call("pnlSetProperty", x, 3, name, value)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetProperty <- function(x, name) UseMethod("GetProperty", x)
GetProperty.pnlBNet <- function(x, name) .Call("pnlGetProperty", x, 0, name)
GetProperty.pnlDBN <- function(x, name) .Call("pnlGetProperty", x, 1, name)
GetProperty.pnlLIMID <- function(x, name) .Call("pnlGetProperty", x, 2, name)
GetProperty.pnlMRF <- function(x, name) .Call("pnlGetProperty", x, 3, name)

LearnParameters <- function(x) UseMethod("LearnParameters", x)
LearnParameters.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlLearnParameters", x, 0)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
LearnParameters.pnlDBN <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlLearnParameters", x, 1)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
LearnParameters.pnlMRF <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlLearnParameters", x, 3)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


LearnStructure <- function(x) UseMethod("LearnStructure", x)
LearnStructure.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlLearnStructure", x)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


SaveEvidBuf <- function(x, filename) UseMethod("SaveEvidBuf", x)
SaveEvidBuf.pnlBNet <- function (x, filename) .Call("pnlSaveEvidBuf", x, 0, filename)
SaveEvidBuf.pnlDBN <- function (x, filename) .Call("pnlSaveEvidBuf", x, 1, filename)
SaveEvidBuf.pnlMRF <- function (x, filename) .Call("pnlSaveEvidBuf", x, 3, filename)

LoadEvidBuf <- function(x, filename, columns) UseMethod("LoadEvidBuf", x)
LoadEvidBuf.pnlBNet <- function (x, filename, columns = -1)
{
	if (columns < 0) .Call("pnlLoadEvidBufNative", x, 0, filename)
	else .Call("pnlLoadEvidBufForeign", x, 0, filename, columns)
}
LoadEvidBuf.pnlDBN <- function (x, filename, columns = -1)
{
	if (columns < 0) .Call("pnlLoadEvidBufNative", x, 1, filename)
	else .Call("pnlLoadEvidBufForeign", x, filename, 1, columns)
}
LoadEvidBuf.pnlMRF <- function (x, filename, columns = -1)
{
	if (columns < 0) .Call("pnlLoadEvidBufNative", x, 3, filename)
	else .Call("pnlLoadEvidBufForeign", x, filename, 3, columns)
}


GenerateEvidences <- function(x, nSamples, ignoreCurrEvid, whatNodes) UseMethod("GenerateEvidences", x)
GenerateEvidences.pnlBNet <- function(x, nSamples, ignoreCurrEvid = -1, whatNodes = -1)
{
	res <- "ok"
	if (ignoreCurrEvid < 0) result <- .Call("pnlGenerateEvidences", x, 0, nSamples)
	else
	{ 
		if (whatNodes < 0) result <- .Call("pnlGenerateEvidencesCurr", x, 0, nSamples, ignoreCurrEvid)
		else result <- .Call("pnlGenerateEvidencesCurrSome", x, 0,  nSamples, ignoreCurrEvid, whatNodes)
	}
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
GenerateEvidences.pnlMRF <- function(x, nSamples, ignoreCurrEvid = -1, whatNodes = -1)
{
	res <- "ok"
	if (ignoreCurrEvid < 0) result <- .Call("pnlGenerateEvidences", x, 3, nSamples)
	else
	{ 
		if (whatNodes < 0) result <- .Call("pnlGenerateEvidencesCurr", x, 3, nSamples, ignoreCurrEvid)
		else result <- .Call("pnlGenerateEvidencesCurrSome", x, 3, nSamples, ignoreCurrEvid, whatNodes)
	}
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
GenerateEvidences.pnlDBN <- function(x, numSamples)
{
	res <- "ok"
	result <- .Call("dbnGenerateEvidences", x, numSamples)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

MaskEvidBuf <- function(x, whatNodes) UseMethod("MaskEvidBuf", x)
MaskEvidBuf.pnlBNet <- function(x, whatNodes = -1)
{
	res <- "ok"
	if (whatNodes < 0) result <- .Call("pnlMaskEvidBufFull", x, 0)
	else result <- .Call("pnlMaskEvidBufPart", x, 0, whatNodes)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
MaskEvidBuf.pnlMRF <- function(x, whatNodes = -1)
{
	res <- "ok"
	if (whatNodes < 0) result <- .Call("pnlMaskEvidBufFull", x, 3)
	else result <- .Call("pnlMaskEvidBufPart", x, 3, whatNodes)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

LoadNet <- function(x, filename) UseMethod("LoadNet", x)
LoadNet.pnlBNet <- function (x, filename) 
{
	res <- "ok"
	result <- .Call("pnlLoadNet", x, 0, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
LoadNet.pnlDBN <- function (x, filename) 
{
	res <- "ok"
	result <- .Call("pnlLoadNet", x, 1, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
LoadNet.pnlLIMID <- function (x, filename) 
{
	res <- "ok"
	result <- .Call("pnlLoadNet", x, 2, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
LoadNet.pnlMRF <- function (x, filename) 
{
	res <- "ok"
	result <- .Call("pnlLoadNet", x, 3, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetCurEvidenceLogLik <- function(x) UseMethod("GetCurEvidenceLogLik", x)
GetCurEvidenceLogLik.pnlBNet <- function(x) .Call("pnlGetCurEvidenceLogLik", x)

GetEvidBufLogLik <- function(x) UseMethod("GetEvidBufLogLik", x)
GetEvidBufLogLik.pnlBNet <- function(x) .Call("pnlGetEvidBufLogLik", x)

GetEMLearningCriterionValue <- function(x) UseMethod("GetEMLearningCriterionValue", x)
GetEMLearningCriterionValue.pnlBNet <- function(x) .Call("pnlGetEMLearningCriterionValue", x, 0)
GetEMLearningCriterionValue.pnlDBN <- function(x) .Call("pnlGetEMLearningCriterionValue", x, 1)

SetNumSlices <- function(x, NumSlice) UseMethod("SetNumSlices", x)
SetNumSlices.pnlDBN <- function(x, NumSlice)
{
	res <- "ok"
	result <- .Call("pnlSetNumSlices", x, NumSlice)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
GetNumSlices <- function(x) UseMethod("GetNumSlices")
GetNumSlices.pnlDBN <- function(x) .Call("pnlGetNumSlices", x)

SetLag <- function(x, LagNum) UseMethod("SetLag", x)
SetLag.pnlDBN <- function(x, LagNum)
{
	res <- "ok"
	result <- .Call("pnlSetLag", x, LagNum)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
GetLag <- function(x) UseMethod("GetLag")
GetLag.pnlDBN <- function(x) .Call("pnlGetLag", x)

IsFullDBN <- function(x) UseMethod("IsFullDBN", x)
IsFullDBN.pnlDBN <- function(x) .Call("pnlIsFullDBN", x)

SetIterMax <- function(x, IterMax) UseMethod("SetIterMax")
SetIterMax.pnlLIMID <- function(x, IterMax)
{
	res <- "ok"
	result <- .Call("pnlSetIterMax", x, IterMax)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetExpectation <- function(x) UseMethod("GetExpectation")
GetExpectation.pnlLIMID <- function(x) .Call("pnlGetExpectation", x)

GetPoliticsString <- function(x) UseMethod("GetPoliticsString")
GetPoliticsString.pnlLIMID <- function(x) .Call("pnlGetPoliticsString", x)

GetPoliticsFloat <- function(x) UseMethod("GetPoliticsFloat")
GetPoliticsFloat.pnlLIMID <- function(x) .Call("pnlGetPoliticsFloat", x)

SetClique <- function(x, nodes) UseMethod("SetClique", x)
SetClique.pnlMRF <- function(x, nodes) 
{
	res <- "ok"
	result <- .Call("pnlSetClique", x, nodes)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

DestroyClique <- function(x, nodes) UseMethod("DestroyClique", x)
DestroyClique.pnlMRF <- function(x, nodes) 
{
	res <- "ok"
	result <- .Call("pnlDestroyClique", x, nodes)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetNumberOfNodes <- function(x) UseMethod("GetNumberOfNodes")
GetNumberOfNodes.pnlMRF <- function(x) .Call("pnlGetNumberOfNodes")

GetNumberOfCliques <- function(x) UseMethod("GetNumberOfCliques")
GetNumberOfCliques.pnlMRF <- function(x) .Call("pnlGetNumberOfCliques")