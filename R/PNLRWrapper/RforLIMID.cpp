#ifndef R_LIMID
#define R_LIMID

#include <math.h>
#include "BNet.hpp"
#include "LIMID.hpp"
#include "pnlException.hpp"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateLIMID();

extern "C" __declspec(dllexport) SEXP pnlSetPChance(SEXP net, SEXP value, SEXP prob);
extern "C" __declspec(dllexport) SEXP pnlSetPDecision(SEXP net, SEXP value, SEXP prob);
extern "C" __declspec(dllexport) SEXP pnlSetValueCost(SEXP net, SEXP value, SEXP prob);
extern "C" __declspec(dllexport) SEXP pnlSetPChanceCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue);
extern "C" __declspec(dllexport) SEXP pnlSetPDecisionCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue);
extern "C" __declspec(dllexport) SEXP pnlSetValueCostCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue);

extern "C" __declspec(dllexport) SEXP pnlGetPChanceString(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPChanceFloat(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPChanceCondFloat(SEXP net, SEXP value, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetPChanceCondString(SEXP net, SEXP value, SEXP parents);

extern "C" __declspec(dllexport) SEXP pnlGetPDecisionString(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPDecisionFloat(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPDecisionCondFloat(SEXP net, SEXP value, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetPDecisionCondString(SEXP net, SEXP value, SEXP parents);

extern "C" __declspec(dllexport) SEXP pnlGetValueCostString(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetValueCostFloat(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetValueCostCondFloat(SEXP net, SEXP value, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlGetValueCostCondString(SEXP net, SEXP value, SEXP parents);
extern "C" __declspec(dllexport) SEXP pnlSetIterMax(SEXP net, SEXP IterMax);
extern "C" __declspec(dllexport) SEXP pnlGetExpectation(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlGetPoliticsString(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlGetPoliticsFloat(SEXP net);

PNLW_USING

extern "C"
{
    extern int DBNCurrentSize;
	extern int NetsCount;
	extern int CurrentSize;
	extern BayesNet ** pBNets;

    extern std::string ErrorString;

	extern int LIMIDCurrentSize;
	extern int LIMIDCount;
	extern LIMID ** pLIMIDs;

//----------------------------------------------------------------------------
    SEXP pnlCreateLIMID()
    {
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        if (LIMIDCount == LIMIDCurrentSize)
        {
            LIMIDCurrentSize *= 2;
            LIMID ** pLIMID_new = new LIMID * [LIMIDCurrentSize];
            for (int i=0; i < LIMIDCount; i++)
            {
                pLIMID_new[i] = pLIMIDs[i];
            }
            delete [] pLIMIDs;
            pLIMIDs = pLIMID_new;
        }
        pLIMIDs[LIMIDCount] = new LIMID();

        if (pLIMIDs[LIMIDCount] != NULL)
        {
            pres[0] = LIMIDCount;
            LIMIDCount++;
        }
        else
        {
            pres[0] = -1;
        }
        UNPROTECT(1);
        return (res);
    }
//----------------------------------------------------------------------------
SEXP pnlSetPChance(SEXP net, SEXP value, SEXP prob)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
			if (IS_CHARACTER(prob)) pLIMIDs[NetNum]->SetPChance(arg1, arg2);
			if (IS_NUMERIC(prob)) pLIMIDs[NetNum]->SetPChance(arg1, arg3);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPChance function";
            flag = 1;
        }
        

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);

}
//----------------------------------------------------------------------------
SEXP pnlSetPDecision(SEXP net, SEXP value, SEXP prob)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
			if (IS_CHARACTER(prob)) pLIMIDs[NetNum]->SetPDecision(arg1, arg2);
			if (IS_NUMERIC(prob)) pLIMIDs[NetNum]->SetPDecision(arg1, arg3);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPDecision function";
            flag = 1;
        }
        

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);

}
//----------------------------------------------------------------------------
SEXP pnlSetValueCost(SEXP net, SEXP value, SEXP prob)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
			if (IS_CHARACTER(prob)) pLIMIDs[NetNum]->SetValueCost(arg1, arg2);
			if (IS_NUMERIC(prob)) pLIMIDs[NetNum]->SetValueCost(arg1, arg3);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetValueCost function";
            flag = 1;
        }
        

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(4);
        return (res);

}
//----------------------------------------------------------------------------
SEXP pnlSetPChanceCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
 			if (IS_CHARACTER(prob)) 
				pLIMIDs[NetNum]->SetPChance(arg1, arg2, arg4);
			if (IS_NUMERIC(prob)) 
				pLIMIDs[NetNum]->SetPChance(arg1, arg3, arg4);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPChance function";    
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(5);
        return (res);
}
//----------------------------------------------------------------------------
SEXP pnlSetPDecisionCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
 			if (IS_CHARACTER(prob)) 
				pLIMIDs[NetNum]->SetPDecision(arg1, arg2, arg4);
			if (IS_NUMERIC(prob)) 
				pLIMIDs[NetNum]->SetPDecision(arg1, arg3, arg4);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetPDecision function";    
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(5);
        return (res);
}
//----------------------------------------------------------------------------
SEXP pnlSetValueCostCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
 			if (IS_CHARACTER(prob)) 
				pLIMIDs[NetNum]->SetValueCost(arg1, arg2, arg4);
			if (IS_NUMERIC(prob)) 
				pLIMIDs[NetNum]->SetValueCost(arg1, arg3, arg4);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetValueCost function";    
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(5);
        return (res);
}
//----------------------------------------------------------------------------
	SEXP pnlGetPChanceString(SEXP net, SEXP value)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetPChance(arg);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPChance function";
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
	SEXP pnlGetPChanceFloat(SEXP net, SEXP value)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetPChance(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPChance function";
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetPDecisionString(SEXP net, SEXP value)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetPDecision(arg);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPDecision function";
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
	SEXP pnlGetPDecisionFloat(SEXP net, SEXP value)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetPDecision(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPDecision function";
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetValueCostString(SEXP net, SEXP value)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetValueCost(arg);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetValueCost function";
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
	SEXP pnlGetValueCostFloat(SEXP net, SEXP value)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetValueCost(arg);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetValueCost function";
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetPChanceCondString(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        PROTECT(parents = AS_CHARACTER(parents));
        char * arg1 = CHAR(asChar(value));
        char * arg2 = CHAR(asChar(parents));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetPChance(arg1, arg2);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPChance function";
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
	SEXP pnlGetPChanceCondFloat(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetPChance(arg, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPChance function";
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
	SEXP pnlGetPDecisionCondString(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        PROTECT(parents = AS_CHARACTER(parents));
        char * arg1 = CHAR(asChar(value));
        char * arg2 = CHAR(asChar(parents));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetPDecision(arg1, arg2);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPDecision function";
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
	SEXP pnlGetPDecisionCondFloat(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetPDecision(arg, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPDecision function";
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
	SEXP pnlGetValueCostCondString(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        PROTECT(parents = AS_CHARACTER(parents));
        char * arg1 = CHAR(asChar(value));
        char * arg2 = CHAR(asChar(parents));
        
        try
        {
			temp = pLIMIDs[NetNum]->GetValueCost(arg1, arg2);
            result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetValueCost function";
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
	SEXP pnlGetValueCostCondFloat(SEXP net, SEXP value, SEXP parents)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(value = AS_CHARACTER(value));
        char * arg = CHAR(asChar(value));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        
        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetValueCost(arg, arg2);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetValueCost function";
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
	SEXP pnlSetIterMax(SEXP net, SEXP IterMax)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		PROTECT(IterMax = AS_INTEGER(IterMax));
        int arg1 = INTEGER_VALUE(IterMax);

        try
        {
			pLIMIDs[NetNum]->SetIterMax(arg1);
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetIterMax function";
            flag = 1;
        }
        

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetExpectation(SEXP net)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        TokArr ResTok;
        try
        {
			ResTok = pLIMIDs[NetNum]->GetExpectation();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetExpectation function";
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
        UNPROTECT(2);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetPoliticsString(SEXP net)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        TokArr ResTok;
		try
        {
			ResTok = pLIMIDs[NetNum]->GetPolitics();
            //result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPolitics function";
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
            PROTECT(res = allocVector(STRSXP, len));
            for (int i=0; i < len; i++)
			{
				temp = ResTok[i];
				SET_STRING_ELT(res, i, mkChar(temp.c_str()));
			}
        }
        UNPROTECT(2);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetPoliticsFloat(SEXP net)
	{
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        TokArr ResTok;
		try
        {
			ResTok = pLIMIDs[NetNum]->GetPolitics();
            //result = temp.c_str();
        }
        catch (pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetPolitics function";
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
            int len = 0;
			for (int i=0; i<ResTok.size(); i++)
			{
				len += ResTok[i].fload.size();
			}
            PROTECT(res = NEW_NUMERIC(len));
            double * pRes = NUMERIC_POINTER(res);
			int curr = 0;
            for (i=0; i < ResTok.size(); i++)
			{
				for (int j=0; j<ResTok[i].fload.size(); j++)
				{
					pRes[curr] = ResTok[i].FltValue(j).fl;
					curr++;
				}
			}
        }
        UNPROTECT(2);
        return (res);
	}
//----------------------------------------------------------------------------
}
#endif