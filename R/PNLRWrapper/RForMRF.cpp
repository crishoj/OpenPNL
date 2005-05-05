#ifndef R_MRF
#define R_MRF

#include <math.h>
#include "BNet.hpp"
#include "MRF.hpp"
#include "pnlException.hpp"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateMRF();
extern "C" __declspec(dllexport) SEXP pnlSetClique(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlDestroyClique(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetNumberOfNodes(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlGetNumberOfCliques(SEXP net);

PNLW_USING

extern "C"
{
    extern std::string ErrorString;

	extern int MRFCurrentSize;
	extern int MRFCount;
	extern MRF ** pMRFs;

//----------------------------------------------------------------------------
    SEXP pnlCreateMRF()
    {
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        if (MRFCount == MRFCurrentSize)
        {
            MRFCurrentSize *= 2;
            MRF ** pMRF_new = new MRF * [MRFCurrentSize];
            for (int i=0; i < MRFCount; i++)
            {
                pMRF_new[i] = pMRFs[i];
            }
            delete [] pMRFs;
            pMRFs = pMRF_new;
        }
        pMRFs[MRFCount] = new MRF();

        if (pMRFs[MRFCount] != NULL)
        {
            pres[0] = MRFCount;
            MRFCount++;
        }
        else
        {
            pres[0] = -1;
        }
        UNPROTECT(1);
        return (res);
    }
//----------------------------------------------------------------------------
	SEXP pnlSetClique(SEXP net, SEXP nodes)
	{
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        try
        {
			pMRFs[NetNum]->SetClique(arg);
        }
        catch (pnl::CException E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of SetClique function";
            flag = 1;
        }

        pres[0] = flag;
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlDestroyClique(SEXP net, SEXP nodes)
	{
        SEXP res;
        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        try
        {
			pMRFs[NetNum]->DestroyClique(arg);
        }
        catch (pnl::CException E)
        {
            ErrorString = E.GetMessage();
            flag = 1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of DestroyClique function";
            flag = 1;
        }

        pres[0] = flag;
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetNumberOfNodes(SEXP net)
	{
        SEXP res;
		int flag = 1;
		int result;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
			result = pMRFs[NetNum]->GetNumberOfNodes();
        }
        catch (pnl::CException E)
        {
            ErrorString = E.GetMessage();
            flag = -1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetNumberOfNodes function";
            flag = -1;
        }

        if (flag == 1)
		{
			PROTECT(res = NEW_INTEGER(1));
			int * pres = INTEGER_POINTER(res);
			pres[0] = result;
		}
		if (flag == -1)
		{
	        PROTECT(res = allocVector(STRSXP, 1));
		    SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
		}
		UNPROTECT(2);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP pnlGetNumberOfCliques(SEXP net)
	{
        SEXP res;
		int flag = 1;
		int result;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
			result = pMRFs[NetNum]->GetNumberOfCliques();
        }
        catch (pnl::CException E)
        {
            ErrorString = E.GetMessage();
            flag = -1;
        }
        catch(...)
        {
            ErrorString = "Unrecognized exception during execution of GetNumberOfCliques function";
            flag = -1;
        }

        if (flag == 1)
		{
			PROTECT(res = NEW_INTEGER(1));
			int * pres = INTEGER_POINTER(res);
			pres[0] = result;
		}
		if (flag == -1)
		{
	        PROTECT(res = allocVector(STRSXP, 1));
		    SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
		}
		UNPROTECT(2);
        return (res);

	}
//----------------------------------------------------------------------------

}//extern "C"
#endif