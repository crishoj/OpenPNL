#include <math.h>

#include "BNet.hpp"
#include "pnlException.hpp"

#include <Rdefines.h>
#include <Rinternals.h>
#include <R.h>

extern "C" __declspec(dllexport) SEXP pnlCreateBNet();
extern "C" __declspec(dllexport) SEXP pnlReturnError();

extern "C" __declspec(dllexport) SEXP pnlAddNode(SEXP net, SEXP names, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlDelNode(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetNodeType(SEXP net, SEXP nodes);

extern "C" __declspec(dllexport) SEXP pnlAddArc(SEXP net, SEXP source, SEXP dest);
extern "C" __declspec(dllexport) SEXP pnlDelArc(SEXP net, SEXP source, SEXP dest);

extern "C" __declspec(dllexport) SEXP pnlGetNeighbors(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetParents(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetChildren(SEXP net, SEXP nodes);

extern "C" __declspec(dllexport) SEXP pnlSetPTabular(SEXP net, SEXP value, SEXP prob);
extern "C" __declspec(dllexport) SEXP pnlSetPTabularCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue);

extern "C" __declspec(dllexport) SEXP pnlGetPTabular(SEXP net, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetPTabularCond(SEXP net, SEXP value, SEXP parents);

extern "C" __declspec(dllexport) SEXP pnlSetPGaussian(SEXP net, SEXP node, SEXP mean, SEXP variance);
extern "C" __declspec(dllexport) SEXP pnlSetPGaussianCond(SEXP net, SEXP node, SEXP mean, SEXP variance, SEXP weight);

extern "C" __declspec(dllexport) SEXP pnlEditEvidence(SEXP net, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlClearEvid(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlCurEvidToBuf(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlAddEvidToBuf(SEXP net, SEXP values);
extern "C" __declspec(dllexport) SEXP pnlClearEvidBuf(SEXP net);

extern "C" __declspec(dllexport) SEXP pnlGetMPE(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetJPD(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianMean(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianCovar(SEXP net, SEXP nodes);
extern "C" __declspec(dllexport) SEXP pnlGetGaussianWeights (SEXP net, SEXP nodes, SEXP parents);

extern "C" __declspec(dllexport) SEXP pnlSetProperty(SEXP net, SEXP name, SEXP value);
extern "C" __declspec(dllexport) SEXP pnlGetProperty(SEXP net, SEXP name);

extern "C" __declspec(dllexport) SEXP pnlLearnParameters(SEXP net);
extern "C" __declspec(dllexport) SEXP pnlLearnStructure(SEXP net);

extern "C" __declspec(dllexport) SEXP pnlSaveEvidBuf(SEXP net, SEXP filename);  
extern "C" __declspec(dllexport) SEXP pnlLoadEvidBufNative(SEXP net, SEXP filename);
extern "C" __declspec(dllexport) SEXP pnlLoadEvidBufForeign(SEXP net, SEXP filename, SEXP columns);

extern "C" __declspec(dllexport) SEXP pnlGenerateEvidences(SEXP net, SEXP nSample);
extern "C" __declspec(dllexport) SEXP pnlGenerateEvidencesCurr(SEXP net, SEXP nSample, SEXP ignoreCurrEvid);
extern "C" __declspec(dllexport) SEXP pnlGenerateEvidencesCurrSome(SEXP net, SEXP nSample, SEXP ignoreCurrEvid, SEXP whatNodes);
    
extern "C" __declspec(dllexport) SEXP pnlMaskEvidBufFull(SEXP net);    
extern "C" __declspec(dllexport) SEXP pnlMaskEvidBufPart(SEXP net, SEXP whatNodes);

extern "C" __declspec(dllexport) SEXP pnlSaveNet(SEXP net, SEXP file);
extern "C" __declspec(dllexport) SEXP pnlLoadNet(SEXP net, SEXP filename);


PNLW_USING

extern "C"
{
    int CurrentSize = 10;
    BayesNet ** pBNets = new BayesNet * [CurrentSize];
    int NetsCount = 0;
    char * my_result;
    std::string ErrorString;
    
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
    SEXP pnlAddNode(SEXP net, SEXP names, SEXP values)
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
            pBNets[NetNum]->AddNode(arg1, arg2);
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
    SEXP pnlDelNode(SEXP net, SEXP nodes)
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
            pBNets[NetNum]->DelNode(arg);
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
    SEXP pnlGetNodeType(SEXP net, SEXP nodes)
    {
        SEXP res;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);
        String temp;
        const char * result;
        int flag = 0;

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetNodeType(arg);
            result = temp.c_str();
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

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));

        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlAddArc(SEXP net, SEXP source, SEXP dest)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(source = AS_CHARACTER(source));
        PROTECT(dest = AS_CHARACTER(dest));
        char * arg1 = CHAR(asChar(source));
        char * arg2 = CHAR(asChar(dest));
        
        try
        {
            pBNets[NetNum]->AddArc(arg1, arg2);
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

        UNPROTECT(4);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlDelArc(SEXP net, SEXP source, SEXP dest)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(source = AS_CHARACTER(source));
        PROTECT(dest = AS_CHARACTER(dest));
        char * arg1 = CHAR(asChar(source));
        char * arg2 = CHAR(asChar(dest));
        
        try
        {
            pBNets[NetNum]->DelArc(arg1, arg2);
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
       
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetNeighbors(SEXP net, SEXP nodes)
    {
        SEXP res;
        String temp;
        const char * result = "";
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetNeighbors(arg);
            result = temp.c_str();
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

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetParents(SEXP net, SEXP nodes)
    {
        SEXP res;
        String temp;
        const char * result = "";
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetParents(arg);
            result = temp.c_str();
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

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));
        UNPROTECT(2);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetChildren(SEXP net, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetParents(arg);
            result = temp.c_str();
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

        PROTECT(res = allocVector(STRSXP, 1));
        if (flag == 1)
            SET_STRING_ELT(res, 0, mkChar(ErrorString.c_str()));
        else
            SET_STRING_ELT(res, 0, mkChar(result));
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlSetPTabular(SEXP net, SEXP value, SEXP prob)
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
            if (IS_CHARACTER(prob)) pBNets[NetNum]->SetPTabular(arg1, arg2);
            if (IS_NUMERIC(prob)) pBNets[NetNum]->SetPTabular(arg1, arg3);
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
        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlSetPTabularCond(SEXP net, SEXP value, SEXP prob, SEXP parentValue)
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
                pBNets[NetNum]->SetPTabular(arg1, arg2, arg4);
            if (IS_NUMERIC(prob)) 
                pBNets[NetNum]->SetPTabular(arg1, arg3, arg4);
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
    SEXP pnlGetPTabular(SEXP net, SEXP value)
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
            temp = pBNets[NetNum]->GetPTabular(arg);
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
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetPTabularCond(SEXP net, SEXP value, SEXP parents)
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
            temp = pBNets[NetNum]->GetPTabular(arg1, arg2);
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
    SEXP pnlSetPGaussian(SEXP net, SEXP node, SEXP mean, SEXP variance)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str);
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok);
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str);
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok);
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

        UNPROTECT(5);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlSetPGaussianCond(SEXP net, SEXP node, SEXP mean, SEXP variance, SEXP weight)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

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
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, weight_str);
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, weight_str);
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, weight_str);
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, weight_str);
            }
            else if (IS_NUMERIC(weight))
            {
                if ((IS_CHARACTER(mean)) && (IS_CHARACTER(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, variance_str, WeightTok);
                if ((IS_CHARACTER(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, mean_str, VarianceTok, WeightTok);
                if ((IS_NUMERIC(mean)) && (IS_CHARACTER(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, variance_str, WeightTok);
                if ((IS_NUMERIC(mean)) && (IS_NUMERIC(variance)))
                    pBNets[NetNum]->SetPGaussian(arg1, MeanTok, VarianceTok, WeightTok);
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
    SEXP pnlEditEvidence(SEXP net, SEXP values)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(values = AS_CHARACTER(values));
        char * arg = CHAR(asChar(values));

        try
        {
            pBNets[NetNum]->EditEvidence(arg);
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
        UNPROTECT(3);
        return (res);

    }    
//----------------------------------------------------------------------------
    SEXP pnlClearEvid(SEXP net)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->ClearEvid();
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
        UNPROTECT(2);
        return (res);
    }
    
//----------------------------------------------------------------------------
    SEXP pnlCurEvidToBuf(SEXP net)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->CurEvidToBuf();
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
        UNPROTECT(2);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlAddEvidToBuf(SEXP net, SEXP values)
    {
        SEXP res;
        int flag = -1;

        PROTECT(values = AS_CHARACTER(values));
        char * arg = CHAR(asChar(values));

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->AddEvidToBuf(arg);
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
        UNPROTECT(3);
        return (res);
    }
    
//----------------------------------------------------------------------------
    SEXP pnlClearEvidBuf(SEXP net)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->ClearEvidBuf();
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
        UNPROTECT(2);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetMPE(SEXP net, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetMPE(arg);
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

        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
   SEXP pnlGetJPD(SEXP net, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        try
        {
            temp = pBNets[NetNum]->GetJPD(arg);
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

        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianMean(SEXP net, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        
        TokArr ResTok;

        try
        {
            ResTok = pBNets[NetNum]->GetGaussianMean(arg);
           // result = str.c_str();
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

        UNPROTECT(3);
        return (res);

    }
//----------------------------------------------------------------------------
    SEXP pnlGetGaussianCovar(SEXP net, SEXP nodes)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));
        TokArr ResTok;
        
        try
        {
            ResTok = pBNets[NetNum]->GetGaussianCovar(arg);
         //   result = str.c_str();
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

        UNPROTECT(3);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlGetGaussianWeights (SEXP net, SEXP nodes, SEXP parents)
    {
        SEXP res;
        const char * result = "";
        int temp = 0;
        String str;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nodes = AS_CHARACTER(nodes));
        char * arg = CHAR(asChar(nodes));

        PROTECT(parents = AS_CHARACTER(parents));
        char * arg2 = CHAR(asChar(parents));
        TokArr ResTok;
        
        try
        {
            ResTok = pBNets[NetNum]->GetGaussianWeights(arg, arg2);
         //   result = str.c_str();
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
    SEXP pnlSetProperty(SEXP net, SEXP name, SEXP value)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(name = AS_CHARACTER(name));
        PROTECT(value = AS_CHARACTER(value));
        char * arg1 = CHAR(asChar(name));
        char * arg2 = CHAR(asChar(value));
        
        try
        {
            pBNets[NetNum]->SetProperty(arg1, arg2);
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

        UNPROTECT(4);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGetProperty(SEXP net, SEXP name)
    {
        SEXP res;
        const char * result = "";
        String temp;
        int flag = 0;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(name = AS_CHARACTER(name));
        char * arg1 = CHAR(asChar(name));
        
        try
        {
            temp = pBNets[NetNum]->GetProperty(arg1);
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
SEXP pnlLearnParameters(SEXP net)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->LearnParameters();
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

        UNPROTECT(2);
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
            ErrorString = "Unrecognized exception during execution of LearnParameters function";
            flag = 1;
        }

        PROTECT(res = NEW_INTEGER(1));
        int * pres = INTEGER_POINTER(res);
        pres[0] = flag;

        UNPROTECT(2);
        return (res);
    }

//----------------------------------------------------------------------------
    SEXP pnlSaveEvidBuf(SEXP net, SEXP filename)
    {
        SEXP res;
        int flag = 0;
        int count;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * arg1 = CHAR(asChar(filename));

        try
        {
            count = pBNets[NetNum]->SaveEvidBuf(arg1);
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
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadEvidBufNative(SEXP net, SEXP filename)
    {
        SEXP res;
        int flag = 0;
        int count;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * arg1 = CHAR(asChar(filename));

        try
        {
            count = pBNets[NetNum]->LoadEvidBuf(arg1);
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
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadEvidBufForeign(SEXP net, SEXP filename, SEXP columns)
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

        try
        {
            count = pBNets[NetNum]->LoadEvidBuf(arg1, NetConst::eCSV, arg2);
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
    SEXP pnlGenerateEvidences(SEXP net, SEXP nSample)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nSample = AS_INTEGER(nSample));
        int * arg = INTEGER_POINTER(nSample);

        try
        {
            pBNets[NetNum]->GenerateEvidences(arg[0]) ;
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
        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlGenerateEvidencesCurr(SEXP net, SEXP nSample, SEXP ignoreCurrEvid)
    {
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(nSample = AS_INTEGER(nSample));
        PROTECT(ignoreCurrEvid = AS_LOGICAL(ignoreCurrEvid));
        int * arg1 = INTEGER_POINTER(nSample);
        bool arg2 = LOGICAL_VALUE(ignoreCurrEvid);

        try
        {
            pBNets[NetNum]->GenerateEvidences(arg1[0], arg2) ;
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
    SEXP pnlGenerateEvidencesCurrSome(SEXP net, SEXP nSample, SEXP ignoreCurrEvid, SEXP whatNodes)
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

        try
        {
            pBNets[NetNum]->GenerateEvidences(arg1[0], arg2, arg3) ;
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
SEXP pnlMaskEvidBufFull(SEXP net)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        try
        {
            pBNets[NetNum]->MaskEvidBuf() ;
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
SEXP pnlMaskEvidBufPart(SEXP net, SEXP whatNodes)
{
        SEXP res;
        int flag = -1;

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(whatNodes = AS_CHARACTER(whatNodes));
        char * arg = CHAR(asChar(whatNodes));
        
        try
        {
            pBNets[NetNum]->MaskEvidBuf(arg);
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
    SEXP pnlSaveNet(SEXP net, SEXP file)
    {
        SEXP res;
        
        int flag = -1;
        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(file = AS_CHARACTER(file));
        char * filename = CHAR(asChar(file));
        
        try
        {
            pBNets[NetNum]->SaveNet(filename);
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

        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    SEXP pnlLoadNet(SEXP net, SEXP filename)
    {
        SEXP res;
        int flag = -1;
        const char * result = "ok";

        PROTECT(net = AS_INTEGER(net));
        int NetNum = INTEGER_VALUE(net);

        PROTECT(filename = AS_CHARACTER(filename));
        char * file = CHAR(asChar(filename));
        
        try
        {
            pBNets[NetNum]->LoadNet(file);
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

        UNPROTECT(3);
        return (res);
    }
//----------------------------------------------------------------------------
    
    
}//extern "C"