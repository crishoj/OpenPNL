CreateBNet <- function ()
{
	result <- .Call("pnlCreateBNet")
	class(result) <- "pnlBNet"
	result
}
pnlCreateBNet <- function() CreateBNet()

AddNode <- function (x, names, values) UseMethod("AddNode", x)
AddNode.pnlBNet <- function(x, names, values) 
{
	res <- "ok"
	result <- .Call("pnlAddNode", x, names, values)
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
	res <- .Call("pnlDelNode", x, nodes, result)
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
	result <- .Call("pnlAddArc", x, Start, Finish)
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
	result <- .Call("pnlDelArc", x, StartOfArc, EndOfArc)
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
	result <- .Call("pnlSaveNet", x, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}


GetNodeType <- function(x, nodes) UseMethod("GetNodeType", x)
GetNodeType.pnlBNet <- function(x, nodes) .Call("pnlGetNodeType", x, nodes)

GetNeighbors <- function(x, nodes) UseMethod("GetNeighbors", x)
GetNeighbors.pnlBNet <- function(x, nodes) .Call("pnlGetNeighbors", x, nodes)

GetParents <- function(x, nodes) UseMethod("GetParents", x)
GetParents.pnlBNet <- function(x, nodes) .Call("pnlGetParents", x, nodes)

GetChildren <- function(x, nodes) UseMethod("GetChildren", x)
GetChildren.pnlBNet <- function(x, nodes) .Call("pnlGetChildren", x, nodes)

SetPTabular <- function(x, value, probability, ParentValue) UseMethod("SetPTabular", x)
SetPTabular.pnlBNet <- function (x, value, probability, ParentValue = -1)
{
	res <- "ok"
	if (ParentValue < 0) result <- .Call("pnlSetPTabular", x, value, probability)
	else result <- .Call("pnlSetPTabularCond",x, value, probability, ParentValue)
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
	if (parents < 0) .Call("pnlGetPTabularString", x, value)
	else .Call("pnlGetPTabularStringCond", x, value, parents)
}

GetPTabularFloat <- function(x, value, parents) UseMethod("GetPTabularFloat", x)
GetPTabularFloat.pnlBNet <- function(x, value, parents = -1)
{
	if (parents < 0) .Call("pnlGetPTabularFloat", x, value)
	else .Call("pnlGetPTabularFloatCond", x, value, parents)
}


SetPGaussian <- function(x, node, mean, variance, weight) UseMethod("SetPGaussian", x)
SetPGaussian.pnlBNet <- function(x, node, mean, variance, weight = -1)
{
	res <- "ok"
	if (weight < 0) result <- .Call("pnlSetPGaussian", x,  node, mean, variance)
	else result <- .Call("pnlSetPGaussianCond", x, node, mean, variance, weight)
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
	result <- .Call("pnlEditEvidence", x, values)
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
	result <- .Call("pnlClearEvid", x)
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
	result <- .Call("pnlCurEvidToBuf", x)
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
	result <- .Call("pnlAddEvidToBuf", x, values)
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
	result <- .Call("pnlClearEvidBuf", x)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetMPE <- function(x, nodes) UseMethod("GetMPE", x)
GetMPE.pnlBNet <- function(x, nodes) .Call("pnlGetMPE",x, nodes)

GetJPDString <- function(x, nodes) UseMethod("GetJPDString", x)
GetJPDString.pnlBNet <- function(x, nodes) .Call("pnlGetJPDString", x, nodes)

GetJPDFloat <- function(x, nodes) UseMethod("GetJPDFloat", x)
GetJPDFloat.pnlBNet <- function(x, nodes) .Call("pnlGetJPDFloat", x, nodes)

GetGaussianMean <- function(x, nodes) UseMethod("GetGaussianMean", x)
GetGaussianMean.pnlBNet <- function(x, nodes) .Call("pnlGetGaussianMean", x, nodes)

GetGaussianCovar <- function(x, nodes) UseMethod("GetGaussianCovar", x)
GetGaussianCovar.pnlBNet <- function(x, nodes) .Call("pnlGetGaussianCovar", x, nodes)

GetGaussianWeights <- function(x, nodes, parents) UseMethod("GetGaussianWeights", x)
GetGaussianWeights.pnlBNet <- function(x, nodes, parents) .Call("pnlGetGaussianWeights", x, nodes, parents)

SetProperty <- function(x, name, value) UseMethod("SetProperty", x)
SetProperty.pnlBNet <- function(x, name, value) 
{
	res <- "ok"
	result <- .Call("pnlSetProperty", x, name, value)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}

GetProperty <- function(x, name) UseMethod("GetProperty", x)
GetProperty.pnlBNet <- function(x, name) .Call("pnlGetProperty", x, name)

LearnParameters <- function(x) UseMethod("LearnParameters", x)
LearnParameters.pnlBNet <- function(x) 
{
	res <- "ok"
	result <- .Call("pnlLearnParameters", x)
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
SaveEvidBuf.pnlBNet <- function (x, filename) .Call("pnlSaveEvidBuf", x, filename)

LoadEvidBuf <- function(x, filename, columns) UseMethod("LoadEvidBuf", x)
LoadEvidBuf.pnlBNet <- function (x, filename, columns = -1)
{
	if (columns < 0) .Call("pnlLoadEvidBufNative", x, filename)
	else .Call("pnlLoadEvidBufForeign", x, filename, columns)
}

GenerateEvidences <- function(x, nSamples, ignoreCurrEvid, whatNodes) UseMethod("GenerateEvidences", x)
GenerateEvid.pnlBNet <- function(x, nSamples, ignoreCurrEvid = -1, whatNodes = -1)
{
	res <- "ok"
	if (ignoreCurrEvid < 0) result <- .Call("pnlGenerateEvidences", x, nSamples)
	else
	{ 
		if (whatNodes < 0) result <- .Call("pnlGenerateEvidencesCurr", x, nSamples, ignoreCurrEvid)
		else result <- .Call("pnlGenerateEvidencesCurrSome", x,  nSamples, ignoreCurrEvid, whatNodes)
	}
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
	if (whatNodes < 0) result <- .Call("pnlMaskEvidBufFull", x)
	else result <- .Call("pnlMaskEvidBufPart", x, whatNodes)
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
	result <- .Call("pnlLoadNet", x, filename)
	if (result < 0) invisible(result)
	else 
	{
		res <- .Call("pnlReturnError")
		res
	}
}
