#ifndef R_DBN
#define R_DBN

#include <math.h>
#include "DBN.hpp"
#include "pnlException.hpp"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateDBN();
extern "C" __declspec(dllexport) SEXP dbnAddNode(SEXP net, SEXP nodes, SEXP subnodes);
extern "C" __declspec(dllexport) SEXP dbnDelNode(SEXP net, SEXP nodes); 

PNLW_USING

extern "C"
{
    extern int DBNCurrentSize;
    extern DBN ** pDBNs;
    extern int DBNCount;

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
	SEXP dbnAddNode(SEXP net, SEXP names, SEXP values)
	{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(names = AS_CHARACTER(names));
        PROTECT(values = AS_CHARACTER(values));
        char * arg1 = CHAR(asChar(names));
        char * arg2 = CHAR(asChar(values));
        
        try
        {
            pDBNs[NetNum]->AddNode(arg1, arg2);
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
        
        UNPROTECT(4);
        return (res);
	}
//----------------------------------------------------------------------------
	SEXP dbnDelNode(SEXP net, SEXP nodes)
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
            pDBNs[NetNum]->DelNode(arg);
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
        UNPROTECT(3);
        return (res);
	}
//----------------------------------------------------------------------------
} //extern "C"

#endif

