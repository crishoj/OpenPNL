#include <math.h>

#include "BNet.hpp"
#include "DBN.hpp"
#include "LIMID.hpp"
#include "MRF.hpp"
#include "pnlException.hpp"
//#include "RForDBN.h"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateBNet();
extern "C" __declspec(dllexport) SEXP pnlReturnError();

extern "C" __declspec(dllexport) SEXP pnlAddNode(SEXP net, SEXP type, SEXP names, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlDelNode(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetNodeType(SEXP net, SEXP type, SEXP nodes);

extern "C" __declspec(dllexport) SEXP pnlAddArc(SEXP net, SEXP type, SEXP source, SEXP dest);
extern "C" __declspec(dllexport) SEXP pnlDelArc(SEXP net, SEXP type, SEXP source, SEXP dest);

extern "C" __declspec(dllexport) SEXP pnlGetNeighbors(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetParents(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetChildren(SEXP net, SEXP type, SEXP nodes);

extern "C" __declspec(dllexport) SEXP pnlSetPTabular(SEXP net, SEXP type, SEXP value, SEXP prob);
extern "C" __declspec(dllexport) SEXP pnlSetPTabularCond(SEXP net, SEXP type, SEXP value, SEXP prob, SEXP parentValue);

extern "C" __declspec(dllexport) SEXP pnlGetPTabularString(SEXP net, SEXP type, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPTabularStringCond(SEXP net, SEXP type, SEXP value, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetPTabularFloat(SEXP net, SEXP type, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPTabularFloatCond(SEXP net, SEXP type, SEXP value, SEXP parents);

extern "C" __declspec(dllexport) SEXP pnlSetPGaussian(SEXP net, SEXP type, SEXP node, SEXP mean, SEXP variance);
extern "C" __declspec(dllexport) SEXP pnlSetPGaussianCond(SEXP net, SEXP type, SEXP node, SEXP mean, SEXP variance, SEXP weight);
extern "C" __declspec(dllexport) SEXP pnlSetPGaussianCondTParents(SEXP net, SEXP node, SEXP mean, SEXP variance, SEXP weight, SEXP tabParentValue);

extern "C" __declspec(dllexport) SEXP pnlEditEvidence(SEXP net, SEXP type, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlClearEvid(SEXP net, SEXP type);
extern "C" __declspec(dllexport) SEXP pnlCurEvidToBuf(SEXP net, SEXP type);
extern "C" __declspec(dllexport) SEXP pnlAddEvidToBuf(SEXP net, SEXP type, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlClearEvidBuf(SEXP net, SEXP type);

extern "C" __declspec(dllexport) SEXP pnlGetMPE(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetJPDString(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetJPDFloat(SEXP net, SEXP type, SEXP nodes);

extern "C" __declspec(dllexport) SEXP pnlGetGaussianMean(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianMeanCond(SEXP net, SEXP type, SEXP nodes, SEXP tabParents);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianCovar(SEXP net, SEXP type, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianCovarCond(SEXP net, SEXP type, SEXP nodes, SEXP tabParents);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianWeights (SEXP net, SEXP type, SEXP nodes, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianWeightsCond (SEXP net, SEXP type, SEXP nodes, SEXP parents, SEXP tabParents);

extern "C" __declspec(dllexport) SEXP pnlSetProperty(SEXP net, SEXP type, SEXP name, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetProperty(SEXP net, SEXP type, SEXP name);

extern "C" __declspec(dllexport) SEXP pnlLearnParameters(SEXP net, SEXP type);
extern "C" __declspec(dllexport) SEXP pnlLearnStructure(SEXP net);

extern "C" __declspec(dllexport) SEXP pnlSaveEvidBuf(SEXP net, SEXP type, SEXP filename);  
extern "C" __declspec(dllexport) SEXP pnlLoadEvidBufNative(SEXP net, SEXP type, SEXP filename);
extern "C" __declspec(dllexport) SEXP pnlLoadEvidBufForeign(SEXP net, SEXP type, SEXP filename, SEXP columns);

extern "C" __declspec(dllexport) SEXP pnlGenerateEvidences(SEXP net, SEXP type, SEXP nSample);
extern "C" __declspec(dllexport) SEXP pnlGenerateEvidencesCurr(SEXP net, SEXP type, SEXP nSample, SEXP ignoreCurrEvid);
extern "C" __declspec(dllexport) SEXP pnlGenerateEvidencesCurrSome(SEXP net, SEXP type, SEXP nSample, SEXP ignoreCurrEvid, SEXP whatNodes);
    
extern "C" __declspec(dllexport) SEXP pnlMaskEvidBufFull(SEXP net, SEXP type);    
extern "C" __declspec(dllexport) SEXP pnlMaskEvidBufPart(SEXP net, SEXP type, SEXP whatNodes);

extern "C" __declspec(dllexport) SEXP pnlSaveNet(SEXP net, SEXP type, SEXP file);
extern "C" __declspec(dllexport) SEXP pnlLoadNet(SEXP net, SEXP type, SEXP filename);

extern "C" __declspec(dllexport) SEXP pnlGetCurEvidenceLogLik(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlGetEvidBufLogLik(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlGetEMLearningCriterionValue(SEXP net, SEXP type);

extern "C" __declspec(dllexport) SEXP pnlSetPSoftMax(SEXP net, SEXP node, SEXP weight, SEXP offset);
extern "C" __declspec(dllexport) SEXP pnlSetPSoftMaxCond(SEXP net, SEXP node, SEXP weight, SEXP offset, SEXP parentValue);

extern "C" __declspec(dllexport) SEXP pnlGetSoftMaxOffset(SEXP net, SEXP node);
extern "C" __declspec(dllexport) SEXP pnlGetSoftMaxOffsetCond(SEXP net, SEXP node, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetSoftMaxWeights(SEXP net, SEXP node);
extern "C" __declspec(dllexport) SEXP pnlGetSoftMaxWeightsCond(SEXP net, SEXP node, SEXP parent);


PNLW_USING

extern "C"
{
    int CurrentSize = 1;
    BayesNet ** pBNets = new BayesNet * [CurrentSize];
    int NetsCount = 0;
    char * my_result;

    std::string ErrorString;

    int DBNCurrentSize = 1;
    DBN ** pDBNs = new DBN * [DBNCurrentSize];
    int DBNCount = 0;

	int LIMIDCurrentSize = 1;
	int LIMIDCount = 0;
	LIMID ** pLIMIDs = new LIMID * [LIMIDCurrentSize];

	int MRFCurrentSize = 1;
	int MRFCount = 0;
	MRF ** pMRFs = new MRF * [MRFCurrentSize];

    
//----------------------------------------------------------------------------
    SEXP pnlCreateBNet()
    {
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        if (NetsCount == CurrentSize)
        {
            CurrentSize *= 2;
            BayesNet ** pBNets_new = new BayesNet * [CurrentSize];
            for (int i=0; i < NetsCount; i++)
            {
                pBNets_new[i] = pBNets[i];
            }
            delete [] pBNets;
            pBNets = pBNets_new;
        }
        pBNets[NetsCount] = new BayesNet();

        if (pBNets[NetsCount] != NULL)
        {
            pres[0] = NetsCount;
            NetsCount++;
        }
        else
        {
            pres[0] = -1;
        }
        UNPROTECT(1);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlReturnError()
    {
        SEXP res;
        PROTECT(res = allocVector(STRSXP, 1));
        SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
       
        UNPROTECT(1);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlAddNode(SEXP net, SEXP type, SEXP names, SEXP values)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        PROTECT(names = AS_CHARACTER(names));
        PROTECT(values = AS_CHARACTER(values));
        char * arg1 = CHAR(asChar(names));
        char * arg2 = CHAR(asChar(values));
        
        try
        {
            if (NetType == 0) 
				pBNets[NetNum]->AddNode(arg1, arg2);
            if (NetType == 1) 
				pDBNs[NetNum]->AddNode(arg1, arg2);
			if (NetType == 2)
				pLIMIDs[NetNum]->AddNode(arg1, arg2);
			if (NetType == 3)
				pMRFs[NetNum]->AddNode(arg1, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of AddNode function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        
        UNPROTECT(5);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlDelNode(SEXP net, SEXP type,  SEXP nodes)
    {
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->DelNode(arg);
			if (NetType == 1)
				pDBNs[NetNum]->DelNode(arg);
			if (NetType == 2)
				pLIMIDs[NetNum]->DelNode(arg);
			if (NetType == 3)
				pMRFs[NetNum]->DelNode(arg);
        }
        catch (pnl::CException E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of DelNode function";
            flag = 1;
        }

        pres[0] = flag;
        UNPROTECT(4);
        return (res);

    }

//----------------------------------------------------------------------------
    SEXP pnlGetNodeType(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);
        int flag = 0;

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        TokArr ResTok;

        try
        {
            if (NetType == 0) 
				ResTok = pBNets[NetNum]->GetNodeType(arg);
            if (NetType == 1) 
				ResTok = pDBNs[NetNum]->GetNodeType(arg);
			if (NetType == 3)
				ResTok = pMRFs[NetNum]->GetNodeType(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetNodeType function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int size = ResTok.size();
            PROTECT(res = allocVector(STRSXP, size));
            for (int i=0; i<size; i++)
                SET_STRING_ELT(res, i, mkChar(String(ResTok[i]).c_str()));
        }

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlAddArc(SEXP net, SEXP type, SEXP source, SEXP dest)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(source = AS_CHARACTER(source));
        PROTECT(dest = AS_CHARACTER(dest));
        char * arg1 = CHAR(asChar(source));
        char * arg2 = CHAR(asChar(dest));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->AddArc(arg1, arg2);
			if (NetType == 1)
				pDBNs[NetNum]->AddArc(arg1, arg2);
			if (NetType == 2)
				pLIMIDs[NetNum]->AddArc(arg1, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of AddArc function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(5);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlDelArc(SEXP net, SEXP type, SEXP source, SEXP dest)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(source = AS_CHARACTER(source));
        PROTECT(dest = AS_CHARACTER(dest));
        char * arg1 = CHAR(asChar(source));
        char * arg2 = CHAR(asChar(dest));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->DelArc(arg1, arg2);
			if (NetType == 1)
				pDBNs[NetNum]->DelArc(arg1, arg2);
			if (NetType == 2)
				pLIMIDs[NetNum]->DelArc(arg1, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of DelArc function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
       
        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetNeighbors(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        TokArr ResTok;
        
        try
        {
            if (NetType == 0)
				ResTok = pBNets[NetNum]->GetNeighbors(arg);
			if (NetType == 1)
				ResTok = pDBNs[NetNum]->GetNeighbors(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetNeighbors function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int size = ResTok.size();
            PROTECT(res = allocVector(STRSXP, size));
            for (int i=0; i<size; i++)
                SET_STRING_ELT(res, i, mkChar(String(ResTok[i]).c_str()));
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetParents(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        TokArr ResTok;
        
        try
        {
            if (NetType == 0)
				ResTok = pBNets[NetNum]->GetParents(arg);
			if (NetType == 1)
				ResTok = pDBNs[NetNum]->GetParents(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetParents function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int size = ResTok.size();
            PROTECT(res = allocVector(STRSXP, size));
            for (int i=0; i<size; i++)
                SET_STRING_ELT(res, i, mkChar(String(ResTok[i]).c_str()));
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetChildren(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        TokArr ResTok;
        
        try
        {
            if (NetType == 0)
				ResTok = pBNets[NetNum]->GetChildren(arg);
			if (NetType == 1)
				ResTok = pDBNs[NetNum]->GetChildren(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetChildren function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int size = ResTok.size();
            PROTECT(res = allocVector(STRSXP, size));
            for (int i=0; i<size; i++)
                SET_STRING_ELT(res, i, mkChar(String(ResTok[i]).c_str()));
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlSetPTabular(SEXP net, SEXP type, SEXP value, SEXP prob)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);
		
		PROTECT(value = AS_CHARACTER(value));
        char * arg1 = CHAR(asChar(value));

        char * arg2;
        TokArr arg3;
        if (IS_CHARACTER(prob))
        {
            PROTECT(prob = AS_CHARACTER(prob));
            arg2 = CHAR(asChar(prob));
        }
        else //the float vector is the input parameter
        {
            if (IS_NUMERIC(prob))
            {
                PROTECT(prob = AS_NUMERIC(prob));
                double * Pprob = NUMERIC_POINTER(prob);
                int len = LENGTH(prob);
                
                for (int i=0; i<len; i++)
                {
                    arg3.push_back((float)Pprob[i]);
                }

            }
        }
        
        try
        {
            if (NetType == 0)
			{
				if (IS_CHARACTER(prob)) pBNets[NetNum]->SetPTabular(arg1, arg2);
				if (IS_NUMERIC(prob)) pBNets[NetNum]->SetPTabular(arg1, arg3);
			}
			if (NetType == 1)
			{
				if (IS_CHARACTER(prob)) pDBNs[NetNum]->SetPTabular(arg1, arg2);
				if (IS_NUMERIC(prob)) pDBNs[NetNum]->SetPTabular(arg1, arg3);
			}
			if (NetType == 3)
			{
				if (IS_CHARACTER(prob)) pMRFs[NetNum]->SetPTabular(arg1, arg2);
				if (IS_NUMERIC(prob)) pMRFs[NetNum]->SetPTabular(arg1, arg3);
			}
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPTabular function";
            flag = 1;
        }
        

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlSetPTabularCond(SEXP net, SEXP type, SEXP value, SEXP prob, SEXP parentValue)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(value = AS_CHARACTER(value));
        PROTECT(parentValue = AS_CHARACTER(parentValue));
        char * arg1 = CHAR(asChar(value));
        char * arg4 = CHAR(asChar(parentValue));

        char * arg2;
        TokArr arg3;
        if (IS_CHARACTER(prob))
        {
            PROTECT(prob = AS_CHARACTER(prob));
            arg2 = CHAR(asChar(prob));
        }
        else //the float vector is the input parameter
        {
            if (IS_NUMERIC(prob))
            {
                PROTECT(prob = AS_NUMERIC(prob));
                double * Pprob = NUMERIC_POINTER(prob);
                int len = LENGTH(prob);
                
                for (int i=0; i<len; i++)
                {
                    arg3.push_back((float)Pprob[i]);
                }

            }
        }

        try
        {
            if (NetType == 0)
			{
				if (IS_CHARACTER(prob)) 
					pBNets[NetNum]->SetPTabular(arg1, arg2, arg4);
				if (IS_NUMERIC(prob)) 
					pBNets[NetNum]->SetPTabular(arg1, arg3, arg4);
			}
			if (NetType == 1)
			{
				if (IS_CHARACTER(prob)) 
					pDBNs[NetNum]->SetPTabular(arg1, arg2, arg4);
				if (IS_NUMERIC(prob)) 
					pDBNs[NetNum]->SetPTabular(arg1, arg3, arg4);
			}
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPTabular function";    
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(6);
        return (res);

    }
//----------------------------------------------------------------------------
    SEXP pnlGetPTabularString(SEXP net, SEXP type, SEXP value)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        try
        {
            if (NetType == 0)
				temp = pBNets[NetNum]->GetPTabular(arg);
			if (NetType == 1)
				temp = pDBNs[NetNum]->GetPTabular(arg);
			if (NetType == 3)
				temp = pMRFs[NetNum]->GetPTabular(arg);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPTabular function";
            flag = 1;
        }

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetPTabularStringCond(SEXP net, SEXP type, SEXP value, SEXP parents)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(value = AS_CHARACTER(value));
        PROTECT(parents = AS_CHARACTER(parents));
        char * arg1 = CHAR(asChar(value));
        char * arg2 = CHAR(asChar(parents));
        
        try
        {
            if (NetType == 0)
				temp = pBNets[NetNum]->GetPTabular(arg1, arg2);
			if (NetType == 1)
				temp = pDBNs[NetNum]->GetPTabular(arg1, arg2);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPTabular function";
            flag = 1;
        }

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));
        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetPTabularFloat(SEXP net, SEXP type, SEXP value)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        TokArr ResTok;
        try
        {
            if (NetType == 0)
				ResTok = pBNets[NetNum]->GetPTabular(arg);
			if (NetType == 1)
				ResTok = pDBNs[NetNum]->GetPTabular(arg);
			if (NetType == 3)
				ResTok = pMRFs[NetNum]->GetPTabular(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPTabular function";
            flag = 1;
        }

        if (flag == 1)
        {
            //there were exceptions during the function executions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            //there were no exceptions
            int len = ResTok.size();
            PROTECT(res = NEW_NUMERIC(len));
            double * pRes = NUMERIC_POINTER(res);
            for (int i=0; i < len; i++)
            {
                pRes[i] = ResTok[i].FltValue();
            }
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetPTabularFloatCond(SEXP net, SEXP type, SEXP value, SEXP parents)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        
        TokArr ResTok;
        try
        {
            if (NetType == 0)
				ResTok = pBNets[NetNum]->GetPTabular(arg, arg2);
			if (NetType == 1)
				ResTok = pDBNs[NetNum]->GetPTabular(arg, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPTabular function";
            flag = 1;
        }

        if (flag == 1)
        {
            //there were exceptions during the function executions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            //there were no exceptions
            int len = ResTok.size();
            PROTECT(res = NEW_NUMERIC(len));
            double * pRes = NUMERIC_POINTER(res);
            for (int i=0; i < len; i++)
            {
                pRes[i] = ResTok[i].FltValue();
            }
        }
        UNPROTECT(5);
        return (res);
    }

//----------------------------------------------------------------------------
	SEXP pnlSetPSoftMax(SEXP net, SEXP node, SEXP weight, SEXP offset)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg1 = CHAR(asChar(node));
        
        char * weight_str;
        char * offset_str;
        TokArr WeightTok;
        TokArr OffsetTok;

        if (IS_CHARACTER(weight))
        {
            PROTECT(weight = AS_CHARACTER(weight));
            weight_str = CHAR(asChar(weight));
        }
        else if (IS_NUMERIC(weight))
        {
            PROTECT(weight = AS_NUMERIC(weight));
            double * pWeight = NUMERIC_POINTER(weight);
            int len = LENGTH(weight);
            for (int i=0; i<len; i++)
            {
                WeightTok.push_back((float)pWeight[i]);
            }
        }

        if (IS_CHARACTER(offset))
        {
            PROTECT(offset = AS_CHARACTER(offset));
            offset_str = CHAR(asChar(offset));
        }
        else if (IS_NUMERIC(offset))
        {
            PROTECT(offset = AS_NUMERIC(offset));
            double * pOffset = NUMERIC_POINTER(offset);
            int len = LENGTH(offset);
            for (int i=0; i<len; i++)
            {
                OffsetTok.push_back((float)pOffset[i]);
            }
        }

            try
            {
					if ((IS_CHARACTER(weight)) && (IS_CHARACTER(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, weight_str, offset_str);
					if ((IS_CHARACTER(weight)) && (IS_NUMERIC(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, weight_str, OffsetTok);
					if ((IS_NUMERIC(weight)) && (IS_CHARACTER(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, WeightTok, offset_str);
					if ((IS_NUMERIC(weight)) && (IS_NUMERIC(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, WeightTok, OffsetTok);
            }
            catch (pnl::CException &E)
            {
                ErrorString = E.GetMessage();
                flag = 1;
            }
            catch(...)
            {
                ErrorString = "Unrecognized exception during execution of SetPSoftMax function"; 
                flag = 1;
            }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(5);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlSetPSoftMaxCond(SEXP net, SEXP node, SEXP weight, SEXP offset, SEXP parentValue)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg1 = CHAR(asChar(node));

		PROTECT(parentValue = AS_CHARACTER(parentValue));
		char * parents = CHAR(asChar(parentValue));
        
        char * weight_str;
        char * offset_str;
        TokArr WeightTok;
        TokArr OffsetTok;

        if (IS_CHARACTER(weight))
        {
            PROTECT(weight = AS_CHARACTER(weight));
            weight_str = CHAR(asChar(weight));
        }
        else if (IS_NUMERIC(weight))
        {
            PROTECT(weight = AS_NUMERIC(weight));
            double * pWeight = NUMERIC_POINTER(weight);
            int len = LENGTH(weight);
            for (int i=0; i<len; i++)
            {
                WeightTok.push_back((float)pWeight[i]);
            }
        }

        if (IS_CHARACTER(offset))
        {
            PROTECT(offset = AS_CHARACTER(offset));
            offset_str = CHAR(asChar(offset));
        }
        else if (IS_NUMERIC(offset))
        {
            PROTECT(offset = AS_NUMERIC(offset));
            double * pOffset = NUMERIC_POINTER(offset);
            int len = LENGTH(offset);
            for (int i=0; i<len; i++)
            {
                OffsetTok.push_back((float)pOffset[i]);
            }
        }

            try
            {
					if ((IS_CHARACTER(weight)) && (IS_CHARACTER(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, weight_str, offset_str, parents);
					if ((IS_CHARACTER(weight)) && (IS_NUMERIC(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, weight_str, OffsetTok, parents);
					if ((IS_NUMERIC(weight)) && (IS_CHARACTER(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, WeightTok, offset_str, parents);
					if ((IS_NUMERIC(weight)) && (IS_NUMERIC(offset)))
						pBNets[NetNum]->SetPSoftMax(arg1, WeightTok, OffsetTok, parents);
            }
            catch (pnl::CException &E)
            {
                ErrorString = E.GetMessage();
                flag = 1;
            }
            catch(...)
            {
                ErrorString = "Unrecognized exception during execution of SetPSoftMax function"; 
                flag = 1;
            }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(6);
        return (res);
	}

//----------------------------------------------------------------------------
	SEXP pnlGetSoftMaxOffset(SEXP net, SEXP node)
	{
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg = CHAR(asChar(node));
        
        TokArr ResTok;

        try
        {
			ResTok = pBNets[NetNum]->GetSoftMaxOffset(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetSoftMaxOffset  function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetSoftMaxOffsetCond(SEXP net, SEXP node, SEXP parents)
	{
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg = CHAR(asChar(node));

		PROTECT(parents = AS_CHARACTER(parents));
		char * par = CHAR(asChar(parents));
        
        TokArr ResTok;

        try
        {
			ResTok = pBNets[NetNum]->GetSoftMaxOffset(arg, par);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetSoftMaxOffset execution";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(4);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetSoftMaxWeights(SEXP net, SEXP node)
	{
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg = CHAR(asChar(node));
        
        TokArr ResTok;

        try
        {
			ResTok = pBNets[NetNum]->GetSoftMaxWeights(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetSoftMaxWeights function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetSoftMaxWeightsCond(SEXP net, SEXP node, SEXP parents)
	{
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg = CHAR(asChar(node));

		PROTECT(parents = AS_CHARACTER(parents));
		char * par = CHAR(asChar(parents));
        
        TokArr ResTok;

        try
        {
			ResTok = pBNets[NetNum]->GetSoftMaxWeights(arg, par);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetSoftMaxWeights execution";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(4);
        return (res);
	}
//----------------------------------------------------------------------------
    SEXP pnlSetPGaussian(SEXP net, SEXP type, SEXP node, SEXP mean, SEXP variance)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(node = AS_CHARACTER(node));
        char * arg1 = CHAR(asChar(node));
        
        char * mean_str;
        char * variance_str;
        TokArr MeanTok;
        TokArr VarianceTok;

        if (IS_CHARACTER(mean))
        {
            PROTECT(mean = AS_CHARACTER(mean));
            mean_str = CHAR(asChar(mean));
        }
        else if (IS_NUMERIC(mean))
        {
            PROTECT(mean = AS_NUMERIC(mean));
            double * pMean = NUMERIC_POINTER(mean);
            int len = LENGTH(mean);
            for (int i=0; i<len; i++)
            {
                MeanTok.push_back((float)pMean[i]);
            }
        }

        if (IS_CHARACTER(variance))
        {
            PROTECT(variance = AS_CHARACTER(variance));
            variance_str = CHAR(asChar(variance));
        }
        else if (IS_NUMERIC(variance))
        {
            PROTECT(variance = AS_NUMERIC(variance));
            double * pVariance = NUMERIC_POINTER(variance);
            int len = LENGTH(variance);
            for (int i=0; i<len; i++)
            {
                VarianceTok.push_back((float)pVariance[i]);
            }
        }

            try
            {
                if (NetType == 0)
				{
					if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
						pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str);
					if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
						pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok);
					if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
						pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str);
					if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
						pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok);
				}
				if (NetType == 1)
				{
					if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
						pDBNs[NetNum]->SetPGaussian(arg1, mean_str, variance_str);
					if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
						pDBNs[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok);
					if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
						pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, variance_str);
					if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
						pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok);
				}

            }
            catch (pnl::CException &E)
            {
                ErrorString = E.GetMessage();
                flag = 1;
            }
            catch(...)
            {
                ErrorString = "Unrecognized exception during execution of SetPGaussian function"; 
                flag = 1;
            }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(6);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlSetPGaussianCond(SEXP net, SEXP type, SEXP node, SEXP mean, SEXP variance, SEXP weight)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(node = AS_CHARACTER(node));
        char * arg1 = CHAR(asChar(node));

        char * mean_str;
        char * variance_str;
        char * weight_str;
        TokArr MeanTok;
        TokArr VarianceTok;
        TokArr WeightTok;

        int len;

        if (IS_CHARACTER(mean))
        {
            PROTECT(mean = AS_CHARACTER(mean));
            mean_str = CHAR(asChar(mean));
        }
        else if (IS_NUMERIC(mean))
        {
            PROTECT(mean = AS_NUMERIC(mean));
            double * pMean = NUMERIC_POINTER(mean);
            len = LENGTH(mean);
            for (int i=0; i<len; i++)
            {
                MeanTok.push_back((float)pMean[i]);
            }
        }

        if (IS_CHARACTER(variance))
        {
            PROTECT(variance = AS_CHARACTER(variance));
            variance_str = CHAR(asChar(variance));
        }
        else if (IS_NUMERIC(variance))
        {
            PROTECT(variance = AS_NUMERIC(variance));
            double * pVariance = NUMERIC_POINTER(variance);
            len = LENGTH(variance);
            for (int i=0; i<len; i++)
            {
                VarianceTok.push_back((float)pVariance[i]);
            }
        }
      
        if (IS_CHARACTER(weight))
        {
            PROTECT(weight = AS_CHARACTER(weight));
            weight_str = CHAR(asChar(weight));
        }
        else if (IS_NUMERIC(weight))
        {
            PROTECT(weight = AS_NUMERIC(weight));
            double * pWeight = NUMERIC_POINTER(weight);
            len = LENGTH(weight);
            for (int i=0; i<len; i++)
            {
                WeightTok.push_back((float)pWeight[i]);
            }
        }


        try
        {
            if (IS_CHARACTER(weight))
            {
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, weight_str);
                    if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, mean_str, variance_str, weight_str);
				}
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, weight_str);
                    if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, weight_str);
				}
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, weight_str);
                    if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, weight_str);
				}
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, weight_str);
                    if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, weight_str);
				}
            }
            else if (IS_NUMERIC(weight))
            {
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, WeightTok);
					if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, mean_str, variance_str, WeightTok);
				}
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, WeightTok);
					if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, WeightTok);
				}
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, WeightTok);
					if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, WeightTok);
				}
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
				{
                    if (NetType == 0) pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, WeightTok);
                    if (NetType == 1) pDBNs[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, WeightTok);
				}
            }
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPGaussian function"; 
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(7);
        return (res);
    }

//----------------------------------------------------------------------------
	SEXP pnlSetPGaussianCondTParents(SEXP net, SEXP node, 
		SEXP mean, SEXP variance, SEXP weight, SEXP tabParentValue)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(node = AS_CHARACTER(node));
        char * arg1 = CHAR(asChar(node));

		PROTECT(tabParentValue = AS_CHARACTER(tabParentValue));
		char * TabParent = CHAR(asChar(tabParentValue));

        char * mean_str;
        char * variance_str;
        char * weight_str;
        TokArr MeanTok;
        TokArr VarianceTok;
        TokArr WeightTok;

        int len;

        if (IS_CHARACTER(mean))
        {
            PROTECT(mean = AS_CHARACTER(mean));
            mean_str = CHAR(asChar(mean));
        }
        else if (IS_NUMERIC(mean))
        {
            PROTECT(mean = AS_NUMERIC(mean));
            double * pMean = NUMERIC_POINTER(mean);
            len = LENGTH(mean);
            for (int i=0; i<len; i++)
            {
                MeanTok.push_back((float)pMean[i]);
            }
        }

        if (IS_CHARACTER(variance))
        {
            PROTECT(variance = AS_CHARACTER(variance));
            variance_str = CHAR(asChar(variance));
        }
        else if (IS_NUMERIC(variance))
        {
            PROTECT(variance = AS_NUMERIC(variance));
            double * pVariance = NUMERIC_POINTER(variance);
            len = LENGTH(variance);
            for (int i=0; i<len; i++)
            {
                VarianceTok.push_back((float)pVariance[i]);
            }
        }
      
        if (IS_CHARACTER(weight))
        {
            PROTECT(weight = AS_CHARACTER(weight));
            weight_str = CHAR(asChar(weight));
        }
        else if (IS_NUMERIC(weight))
        {
            PROTECT(weight = AS_NUMERIC(weight));
            double * pWeight = NUMERIC_POINTER(weight);
            len = LENGTH(weight);
            for (int i=0; i<len; i++)
            {
                WeightTok.push_back((float)pWeight[i]);
            }
        }


        try
        {
            if (IS_CHARACTER(weight))
            {
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, weight_str, TabParent);
				}
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, weight_str, TabParent);
				}
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, weight_str, TabParent);
				}
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, weight_str, TabParent);
				}
            }
            else if (IS_NUMERIC(weight))
            {
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, WeightTok, TabParent);
				}
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, WeightTok, TabParent);
				}
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, WeightTok, TabParent);
				}
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
				{
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, WeightTok, TabParent);
				}
            }
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPGaussian function"; 
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(7);
        return (res);
	}

//----------------------------------------------------------------------------
    SEXP pnlEditEvidence(SEXP net, SEXP type, SEXP values)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(values = AS_CHARACTER(values));
        char * arg = CHAR(asChar(values));

        try
        {
            if (NetType == 0) pBNets[NetNum]->EditEvidence(arg);
			if (NetType == 1) pDBNs[NetNum]->EditEvidence(arg);
			if (NetType == 3) pMRFs[NetNum]->EditEvidence(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of EditEvidence function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);

    }    
//----------------------------------------------------------------------------
    SEXP pnlClearEvid(SEXP net, SEXP type)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->ClearEvid();
			if (NetType == 1) pDBNs[NetNum]->ClearEvid(); 
			if (NetType == 3) pMRFs[NetNum]->ClearEvid(); 
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of ClearEvid function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(3);
        return (res);
    }
    
//----------------------------------------------------------------------------
    SEXP pnlCurEvidToBuf(SEXP net, SEXP type)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->CurEvidToBuf();
			if (NetType == 1) pDBNs[NetNum]->CurEvidToBuf();
			if (NetType == 3) pMRFs[NetNum]->CurEvidToBuf();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of CurEvidToBuf function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(3);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlAddEvidToBuf(SEXP net, SEXP type, SEXP values)
    {
        SEXP res;
        int flag = -1;

        PROTECT(values = AS_CHARACTER(values));
        char * arg = CHAR(asChar(values));

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            if (NetType == 0) pBNets[NetNum]->AddEvidToBuf(arg);
			if (NetType == 1) pDBNs[NetNum]->AddEvidToBuf(arg);
			if (NetType == 3) pMRFs[NetNum]->AddEvidToBuf(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of AddEvidToBuf function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);
    }
    
//----------------------------------------------------------------------------
    SEXP pnlClearEvidBuf(SEXP net, SEXP type)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->ClearEvidBuf();
			if (NetType == 1) pDBNs[NetNum]->ClearEvidBuf();
			if (NetType == 3) pMRFs[NetNum]->ClearEvidBuf();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of ClearEvidBuf function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetMPE(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        DBN * qqq = pDBNs[NetNum];
		qqq ->SaveNet("dbn.xml");
		try
        {
            if (NetType == 0) 
				temp = pBNets[NetNum]->GetMPE(arg);
			if (NetType == 1) 
				temp = pDBNs[NetNum]->GetMPE(arg);
			if (NetType == 3) 
				temp = pMRFs[NetNum]->GetMPE(arg);
            result = temp.c_str();
		}
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetMPE function";
            flag = 1;
        }

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
   SEXP pnlGetJPDFloat(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;

        try
        {
            if (NetType == 0) ResTok = pBNets[NetNum]->GetJPD(arg);
			if (NetType == 1) ResTok = pDBNs[NetNum]->GetJPD(arg);
			if (NetType == 3) ResTok = pMRFs[NetNum]->GetJPD(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetJPD function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int len = ResTok.size();
            PROTECT(res = NEW_NUMERIC(len));
            double * pRes = NUMERIC_POINTER(res);
            for (int i=0; i < len; i++)
            {
                pRes[i] = ResTok[i].FltValue();
            }
        }

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
   SEXP pnlGetJPDString(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            if (NetType == 0) temp = pBNets[NetNum]->GetJPD(arg);
			if (NetType == 1) temp = pDBNs[NetNum]->GetJPD(arg); 
			if (NetType == 3) temp = pMRFs[NetNum]->GetJPD(arg); 
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetJPD function";
            flag = 1;
        }

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianMean(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;

        try
        {
            if (NetType == 0) 
				ResTok = pBNets[NetNum]->GetGaussianMean(arg);
            if (NetType == 1) 
				ResTok = pDBNs[NetNum]->GetGaussianMean(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianMean function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(4);
        return (res);

    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianMeanCond(SEXP net, SEXP type, SEXP nodes, SEXP tabParents)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

		PROTECT(tabParents = AS_CHARACTER(tabParents));
		char * parents = CHAR(asChar(tabParents));
        
        TokArr ResTok;

        try
        {
            if (NetType == 0) 
				ResTok = pBNets[NetNum]->GetGaussianMean(arg, parents);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianMean function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(5);
        return (res);

    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianCovar(SEXP net, SEXP type, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        TokArr ResTok;
        
        try
        {
            if (NetType == 0) ResTok = pBNets[NetNum]->GetGaussianCovar(arg);
            if (NetType == 1) ResTok = pDBNs[NetNum]->GetGaussianCovar(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianCovar function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(4);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlGetGaussianCovarCond(SEXP net, SEXP type, SEXP nodes, SEXP tabParents)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
		
		PROTECT(tabParents = AS_CHARACTER(tabParents));
		char * parents = CHAR(asChar(tabParents));

        TokArr ResTok;
        
        try
        {
            if (NetType == 0) ResTok = pBNets[NetNum]->GetGaussianCovar(arg, parents);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianCovar function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianWeights (SEXP net, SEXP type, SEXP nodes, SEXP parents)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        TokArr ResTok;
        
        try
        {
            if (NetType == 0) ResTok = pBNets[NetNum]->GetGaussianWeights(arg, arg2);
			if (NetType == 1) ResTok = pDBNs[NetNum]->GetGaussianWeights(arg, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianWeights function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianWeightsCond (SEXP net, SEXP type, SEXP nodes, SEXP contParents, SEXP tabParents)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        PROTECT(contParents = AS_CHARACTER(contParents));
        char * arg2 = CHAR(asChar(contParents));

        PROTECT(tabParents = AS_CHARACTER(tabParents));
        char * arg3 = CHAR(asChar(tabParents));
        
		TokArr ResTok;
        
        try
        {
            if (NetType == 0) ResTok = pBNets[NetNum]->GetGaussianWeights(arg, arg2, arg3);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            temp = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetGaussianWeights function";
            temp = 1;
        }

        if (temp == 0)
        {
            //there was no exceptions
            int size = ResTok[0].fload.size();
            PROTECT(res = NEW_NUMERIC(size));
            double * pres = NUMERIC_POINTER(res);
            for (int i=0; i<size; i++)
            {
                pres [i] = ResTok[0].FltValue(i).fl; 
            }
        }
        else
        {
            //there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }

        UNPROTECT(6);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlSetProperty(SEXP net, SEXP type, SEXP name, SEXP value)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(name = AS_CHARACTER(name));
        char * arg1 = CHAR(asChar(name));

        PROTECT(value = AS_CHARACTER(value));
        char * arg2 = CHAR(asChar(value));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->SetProperty(arg1, arg2);
			if (NetType == 1)
				pDBNs[NetNum]->SetProperty(arg1, arg2);
			if (NetType == 2)
				pLIMIDs[NetNum]->SetProperty(arg1, arg2);
			if (NetType == 3)
				pMRFs[NetNum]->SetProperty(arg1, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetProperty function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetProperty(SEXP net, SEXP type, SEXP name)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(name = AS_CHARACTER(name));
        char * arg1 = CHAR(asChar(name));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) 
				temp = pBNets[NetNum]->GetProperty(arg1);
			if (NetType == 1)
				temp = pDBNs[NetNum]->GetProperty(arg1);
			if (NetType == 2)
				temp = pLIMIDs[NetNum]->GetProperty(arg1);
			if (NetType == 3)
				temp = pMRFs[NetNum]->GetProperty(arg1);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetProperty function";
            flag = 1;
        }

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));

        UNPROTECT(3);
        return (res);
    }
   
//----------------------------------------------------------------------------
SEXP pnlLearnParameters(SEXP net, SEXP type)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->LearnParameters();
			if (NetType == 1) pDBNs[NetNum]->LearnParameters();
			if (NetType == 3) pMRFs[NetNum]->LearnParameters();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of LearnParameters function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(3);
        return (res);
}

//----------------------------------------------------------------------------
    SEXP pnlLearnStructure(SEXP net)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->LearnStructure();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of LearnStructure function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(2);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlSaveEvidBuf(SEXP net, SEXP type, SEXP filename)
    {
        SEXP res;
        int flag = 0;
        int count;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * arg1 = CHAR(asChar(filename));

        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				count = pBNets[NetNum]->SaveEvidBuf(arg1);
			if (NetType == 1)
				count = pDBNs[NetNum]->SaveEvidBuf(arg1);
			if (NetType == 3)
				count = pMRFs[NetNum]->SaveEvidBuf(arg1);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch (...)
        {
            ErrorString = "Unrecognized exception during execution of SaveEvidBuf function";
            flag = 1;
        }

        if (flag == 0)
        {
            PROTECT(res = NEW_INTEGER(1));
            int * pres = INTEGER_POINTER(res);
            pres [0] = count; 
        }
        else
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadEvidBufNative(SEXP net, SEXP type, SEXP filename)
    {
        SEXP res;
        int flag = 0;
        int count;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * arg1 = CHAR(asChar(filename));

        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				count = pBNets[NetNum]->LoadEvidBuf(arg1);
			if (NetType == 1)
				count = pDBNs[NetNum]->LoadEvidBuf(arg1);
			if (NetType == 3)
				count = pMRFs[NetNum]->LoadEvidBuf(arg1);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch (...)
        {
            ErrorString = "Unrecognized exception during execution of LoadEvidBuf function";
            flag = 1;
        }

        if (flag == 0)
        {
            PROTECT(res = NEW_INTEGER(1));
            int * pres = INTEGER_POINTER(res);
            pres [0] = count; 
        }
        else
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadEvidBufForeign(SEXP net, SEXP type, SEXP filename, SEXP columns)
    {
        SEXP res;
        int flag = 0;
        int count;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        PROTECT(columns = AS_CHARACTER(columns));
        char * arg1 = CHAR(asChar(filename));
        char * arg2 = CHAR(asChar(columns));

        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				count = pBNets[NetNum]->LoadEvidBuf(arg1, NetConst::eCSV, arg2);
			if (NetType == 1)
				count = pDBNs[NetNum]->LoadEvidBuf(arg1, NetConst::eCSV, arg2);
			if (NetType == 3)
				count = pMRFs[NetNum]->LoadEvidBuf(arg1, NetConst::eCSV, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch (...)
        {
            ErrorString = "Unrecognized exception during execution of LoadEvidBuf function";
            flag = 1;
        }

        if (flag == 0)
        {
            PROTECT(res = NEW_INTEGER(1));
            int * pres = INTEGER_POINTER(res);
            pres [0] = count; 
        }
        else
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGenerateEvidences(SEXP net, SEXP type, SEXP nSample)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nSample = AS_INTEGER(nSample));
        int * arg = INTEGER_POINTER(nSample);

        try
        {
            if (NetType == 0) pBNets[NetNum]->GenerateEvidences(arg[0]) ;
            if (NetType == 3) pMRFs[NetNum]->GenerateEvidences(arg[0]) ;
        }
        catch(pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GenerateEvidences function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGenerateEvidencesCurr(SEXP net, SEXP type, SEXP nSample, SEXP ignoreCurrEvid)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        PROTECT(nSample = AS_INTEGER(nSample));
        PROTECT(ignoreCurrEvid = AS_LOGICAL(ignoreCurrEvid));
        int * arg1 = INTEGER_POINTER(nSample);
        bool arg2 = LOGICAL_VALUE(ignoreCurrEvid);

        try
        {
            if (NetType == 0) pBNets[NetNum]->GenerateEvidences(arg1[0], arg2) ;
            if (NetType == 3) pMRFs[NetNum]->GenerateEvidences(arg1[0], arg2) ;
        }
        catch(pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GenerateEvidences function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGenerateEvidencesCurrSome(SEXP net, SEXP type, SEXP nSample, SEXP ignoreCurrEvid, SEXP whatNodes)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nSample = AS_INTEGER(nSample));
        PROTECT(ignoreCurrEvid = AS_LOGICAL(ignoreCurrEvid));
        PROTECT(whatNodes = AS_CHARACTER(whatNodes));
        int * arg1 = INTEGER_POINTER(nSample);
        bool arg2 = LOGICAL_VALUE(ignoreCurrEvid);
        char * arg3 = CHAR(asChar(whatNodes));

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->GenerateEvidences(arg1[0], arg2, arg3) ;
            if (NetType == 3) pMRFs[NetNum]->GenerateEvidences(arg1[0], arg2, arg3) ;
        }
        catch(pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GenerateEvidences function";
            flag = 1;
        }
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(5);
        return (res);
    }
//----------------------------------------------------------------------------
SEXP pnlMaskEvidBufFull(SEXP net, SEXP type)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->MaskEvidBuf() ;
            if (NetType == 3) pMRFs[NetNum]->MaskEvidBuf() ;
        }
        catch(pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of MaskEvidBuf function";
            flag = 1;
        }
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(2);
        return (res);
}
//----------------------------------------------------------------------------
SEXP pnlMaskEvidBufPart(SEXP net, SEXP type, SEXP whatNodes)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(whatNodes = AS_CHARACTER(whatNodes));
        char * arg = CHAR(asChar(whatNodes));
        
		PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0) pBNets[NetNum]->MaskEvidBuf(arg);
            if (NetType == 3) pMRFs[NetNum]->MaskEvidBuf(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of MaskEvidBuf function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(3);
        return (res);

}

//----------------------------------------------------------------------------
    SEXP pnlSaveNet(SEXP net, SEXP type, SEXP file)
    {
        SEXP res;
        
        int flag = -1;
        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(file = AS_CHARACTER(file));
        char * filename = CHAR(asChar(file));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->SaveNet(filename);
            if (NetType == 1)
				pDBNs[NetNum]->SaveNet(filename);
			if (NetType == 2)
				pLIMIDs[NetNum]->SaveNet(filename);
			if (NetType == 3)
				pMRFs[NetNum]->SaveNet(filename);

        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SaveNet function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadNet(SEXP net, SEXP type, SEXP filename)
    {
        SEXP res;
        int flag = -1;
        const char * result = "ok";

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * file = CHAR(asChar(filename));
        
        PROTECT(type = AS_INTEGER(type));
        int NetType = INTEGER_VALUE(type);

        try
        {
            if (NetType == 0)
				pBNets[NetNum]->LoadNet(file);
			if (NetType == 1)
				pDBNs[NetNum]->LoadNet(file);
			if (NetType == 2)
				pLIMIDs[NetNum]->LoadNet(file);
			if (NetType == 3)
				pMRFs[NetNum]->LoadNet(file);

        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of LoadNet function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------

    SEXP pnlGetCurEvidenceLogLik(SEXP net)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        float result;
        try
        {
            result = pBNets[NetNum]->GetCurEvidenceLogLik();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetCurEvidenceLogLik function";
            flag = 1;
        }

        if (flag == 1)
        {
            //there were exceptions during the function executions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            //there were no exceptions
            PROTECT(res = NEW_NUMERIC(1));
            double * pRes = NUMERIC_POINTER(res);
            pRes[0] = result;
        }
        UNPROTECT(2);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetEvidBufLogLik(SEXP net)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        TokArr ResTok;
        try
        {
            ResTok = pBNets[NetNum]->GetEvidBufLogLik();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetEvidBufLogLik function";
            flag = 1;
        }

        if (flag == 1)
        {
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            int len = ResTok.size();
            PROTECT(res = NEW_NUMERIC(len));
            double * pRes = NUMERIC_POINTER(res);
            for (int i=0; i < len; i++)
            {
                pRes[i] = ResTok[i].FltValue();
            }
        }

        UNPROTECT(2);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetEMLearningCriterionValue(SEXP net, SEXP type)
    {
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(type = AS_INTEGER(type));
		int NetType = INTEGER_VALUE(type);

		float result;
        try
        {
            if (NetType == 0) 
				result = pBNets[NetNum]->GetEMLearningCriterionValue();
			if (NetType == 1)
				result = pDBNs[NetNum]->GetEMLearningCriterionValue();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetEMLearningCriterionValue function";
            flag = 1;
        }

        if (flag == 1)
        {
            //there were exceptions during the function executions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        }
        else
        {
            //there were no exceptions
            PROTECT(res = NEW_NUMERIC(1));
            double * pRes = NUMERIC_POINTER(res);
            pRes[0] = result;
        }
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    
    
}//extern "C"