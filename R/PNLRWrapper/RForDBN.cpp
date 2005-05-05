#ifndef R_DBN
#define R_DBN

#include <math.h>
#include "DBN.hpp"
#include "BNet.hpp"
#include "pnlException.hpp"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateDBN();
/*extern "C" __declspec(dllexport) SEXP Parents(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP Children(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP Ancestors(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP Descendants(SEXP net, SEXP nodes);*/

extern "C" __declspec(dllexport) SEXP pnlSetNumSlices(SEXP net, SEXP nSlices);
extern "C" __declspec(dllexport) SEXP pnlGetNumSlices(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlSetLag(SEXP net, SEXP lag);
extern "C" __declspec(dllexport) SEXP pnlGetLag(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlIsFullDBN(SEXP net);
extern "C" __declspec(dllexport) SEXP dbnGenerateEvidences(SEXP net, SEXP numSlices);
extern "C" __declspec(dllexport) SEXP pnlUnroll(SEXP net);


PNLW_USING

extern "C"
{
    extern int DBNCurrentSize;
    extern DBN ** pDBNs;
    extern int DBNCount;
	extern int NetsCount;
	extern int CurrentSize;
	extern BayesNet ** pBNets;

    extern std::string ErrorString;

//----------------------------------------------------------------------------
    SEXP pnlCreateDBN()
    {
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        if (DBNCount == DBNCurrentSize)
        {
            DBNCurrentSize *= 2;
            DBN ** pDBN_new = new DBN * [DBNCurrentSize];
            for (int i=0; i < DBNCount; i++)
            {
                pDBN_new[i] = pDBNs[i];
            }
            delete [] pDBNs;
            pDBNs = pDBN_new;
        }
        pDBNs[DBNCount] = new DBN();

        if (pDBNs[DBNCount] != NULL)
        {
            pres[0] = DBNCount;
            DBNCount++;
        }
        else
        {
            pres[0] = -1;
        }
        UNPROTECT(1);
        return (res);
    }

//----------------------------------------------------------------------------
	SEXP pnlUnroll(SEXP net)
	{
		SEXP res;

		PROTECT(net = AS_INTEGER(net));
		int NetNum = INTEGER_VALUE(net);

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
	//	NetsCount++;

        if (pBNets[NetsCount] != NULL)
        {
            pres[0] = NetsCount;
			pBNets[NetsCount] = pDBNs[NetNum]->Unroll();
            NetsCount++;
        }
        else
        {
            pres[0] = -1;
        }
        UNPROTECT(2);
        return (res);

	}
//----------------------------------------------------------------------------
	SEXP pnlSetNumSlices(SEXP net, SEXP nSlices)
	{
        SEXP res;
		int flag = -1;

		PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nSlices = AS_INTEGER(nSlices));
        int SliceNum = INTEGER_VALUE(nSlices);

		try
		{
			pDBNs[NetNum]->SetNumSlices(SliceNum);
		}
		catch(pnl::CException &E)
		{
            ErrorString = E.GetMessage();
            flag = 1;
		}
		catch(...)
		{
            ErrorString = "Unrecognized exception during execution of SetNumSlices function";
            flag = 1;
		}

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
       
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetNumSlices(SEXP net)
	{
        SEXP res;
		int flag = 0;
		int result;

		PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		try
		{
			result = pDBNs[NetNum]->GetNumSlices();
		}
		catch(pnl::CException &E)
		{
            ErrorString = E.GetMessage();
            flag = 1;
		}
		catch(...)
		{
            ErrorString = "Unrecognized exception during execution of GetNumSlices function";
            flag = 1;
		}

        if (flag == 0)
		{
			//there were no exceptions
			PROTECT(res = NEW_INTEGER(1));
		    int * pres = INTEGER_POINTER(res);
			pres[0] = result;
		}
		if (flag == 1)
		{
			//there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
		}
       
        UNPROTECT(2);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlSetLag(SEXP net, SEXP lag)
	{
        SEXP res;
		int flag = -1;

		PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(lag = AS_INTEGER(lag));
        int LagNum = INTEGER_VALUE(lag);

		try
		{
			pDBNs[NetNum]->SetLag(LagNum);
		}
		catch(pnl::CException &E)
		{
            ErrorString = E.GetMessage();
            flag = 1;
		}
		catch(...)
		{
            ErrorString = "Unrecognized exception during execution of SetLag function";
            flag = 1;
		}

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;
       
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetLag(SEXP net)
	{
        SEXP res;
		int flag = 0;
		int result;

		PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

		try
		{
			result = pDBNs[NetNum]->GetLag();
		}
		catch(pnl::CException &E)
		{
            ErrorString = E.GetMessage();
            flag = 1;
		}
		catch(...)
		{
            ErrorString = "Unrecognized exception during execution of GetLag function";
            flag = 1;
		}

        if (flag == 0)
		{
			//there were no exceptions
			PROTECT(res = NEW_INTEGER(1));
		    int * pres = INTEGER_POINTER(res);
			pres[0] = result;
		}
		if (flag == 1)
		{
			//there were exceptions
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
		}
       
        UNPROTECT(2);
        return (res);
	}

//----------------------------------------------------------------------------
	SEXP pnlIsFullDBN(SEXP net)
	{
		SEXP res;
		int flag = 0;
		bool result;

		PROTECT(net = AS_INTEGER(net));
		int NetNum = INTEGER_VALUE(net);

		try
		{
			result = pDBNs[NetNum]->IsFullDBN();
		}
		catch(pnl::CException &E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of IsFullDBN function";
            flag = 1;
        }

		if (flag == 0)
		{
			PROTECT(res = NEW_LOGICAL(1));
			int * pRes = LOGICAL_POINTER(res);
			pRes[0] = result;
		}
		if (flag == 1)
		{
            PROTECT(res = allocVector(STRSXP, 1));
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
		}
		UNPROTECT(2);
		return (res);
	}

//----------------------------------------------------------------------------
	SEXP dbnGenerateEvidences(SEXP net, SEXP numSlices)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(numSlices = AS_CHARACTER(numSlices));
        char * arg1 = CHAR(asChar(numSlices));
        
        try
        {
			pDBNs[NetNum]->GenerateEvidences(arg1);
        }
        catch (pnl::CException &E)
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
        
        UNPROTECT(3);
        return (res);

	}
//----------------------------------------------------------------------------
/*    TokArr Parents(TokArr nodes);
    TokArr Children(TokArr nodes);
    TokArr Ancestors(TokArr nodes);
    TokArr Descendants(TokArr nodes);*/

/*	SEXP Parents(SEXP net, SEXP nodes)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;
        
        try
        {
			ResTok = pDBNs[NetNum]->Parents(arg);
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP Children(SEXP net, SEXP nodes)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;
        
        try
        {
			ResTok = pDBNs[NetNum]->Children(arg);
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP Ancestors(SEXP net, SEXP nodes)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;
        
        try
        {
			ResTok = pDBNs[NetNum]->Ancestors(arg);
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP Descendants(SEXP net, SEXP nodes)
	{
        SEXP res;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;
        
        try
        {
			ResTok = pDBNs[NetNum]->Descendants(arg);
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
        UNPROTECT(3);
        return (res);
	}*/

//----------------------------------------------------------------------------
} //extern "C"

#endif

