#include <stdio.h>

#include "pnlHigh.hpp"
#include "WInner.hpp"
#include "example_common_funs.hpp"
#include "script_lex.hpp"

PNLW_USING

/*
struct FuncDesc
{
    const char *name;// function name in lowercase
    int id;// unique id - from enum in WInner.hpp
    bool bSimple;// left and right arguments - TokArr's
    int nLeftArg;
    int nRightArg;
    int nOptArg;
};
*/

FuncDesc aFuncDesc[] =
{   "savenet",		eSaveNet,	false, 0, 1, 0
,   "loadnet",		eLoadNet,	false, 0, 1, 0
,   "saveevidbuf",	eSaveEvidBuf,	false, 0, 1, 0
,   "loadevidbuf",	eLoadEvidBuf,	false, 0, 1, 0
,   "addnode",		eAddNode,	true,  0, 2, 1
//,   "addnode",		eAddNode,	true,  0, 2, 0
,   "delnode",		eDelNode,	true,  0, 1, 0
,   "addarc",		eAddArc,	true,  0, 2, 0
,   "delarc",		eDelArc,	true,  0, 2, 0
,   "setptabular",	eSetPTabular,	true,  0, 3, 1
,   "getptabular",	eGetPTabular,	true,  1, 2, 1
,   "editevidence",	eEditEvidence,	true,  0, 1, 0
,   "clearevid",	eClearEvid,	true,  0, 0, 0
,   "addevidtobuf",	eAddEvidToBuf,	true,  0, 1, 1
,   "clearevidbuf",	eClearEvidBuf,	true,  0, 0, 0
,   "curevidtobuf",	eCurEvidToBuf,	true,  0, 0, 0
,   "learnparameters",	eLearn,		true,  0, 0, 0
,   "getmpe",		eMPE,		true,  1, 1, 1
,   "generateevidences",eGenerateEvidences, false, 0, 3, 2
,   "learnstructure",	eLearnStructure,true,  0, 0, 0
,   "getgaussianmean",	eGaussianMean,	true,  1, 1, 0
,   "getgaussiancovar", eGaussianCovar, true,  1, 1, 0 
,   "setpgaussian",	eSetPGaussian,	true,  0, 4, 3
,   "setproperty",	eSetProperty,	true,  0, 2, 0
,   "getproperty",	eGetProperty,	true,  0, 1, 0
,   "getjpd",		eJPD,		true,  1, 1, 0 
,   "getparents",	eGetParents,	true,  1, 1, 0
,   "getneighbors",	eGetNeighbors,	true,  1, 1, 0
,   "getchildren",	eGetChildren,	true,  1, 1, 0
// build-in commands
,   "execute",	      -1,	    false, 0, 1, 0
,   "clear",	      -2,	    true,  0, 0, 0
,   "clearall",	      -3,	    true,  0, 0, 0
,   "listnodes",      -4,	    true,  0, 0, 0
,   "exit",	      -5,	    true,  0, 0, 0
,   "listcommands",   -6,	    true,  0, 1, 1
,   "print",	      -7,	    true,  0, 1, 1
,   "new",	      -8,	    true,  0, 0, 0
,   "console",	      -9,	    true,  0, 0, 0
,   "comparefiles",   -10,	    true,  1, 3, 1
,   "checkorcreate",  -11,	    true,  0, 3, 0
,   0, 0, 0, 0, 0, 0
};

int
ScriptingLex::GetToken(pnl::pnlString& str)
{
    int ch = GetchAfterSpaces();

    str.resize(0);
    switch(ch)
    {
    case '"': case '\'':  return GetQString(str, ch);
    case '\n': case '\r': case ';': return eTOKEN_EOL;
    case -1:  return eTOKEN_EOF;
    case '(': return eTOKEN_BRACE_OPEN;
    case ')': return eTOKEN_BRACE_CLOSE;
    case '&': return eTOKEN_OP_CATENATION;
    case ',': return eTOKEN_DELIMITER;
    case '^': return eTOKEN_OP_CARTESIAN;
    }
    
    Ungetch(ch);
    ch = GetField(str, "^&,()'\"\n\t\r ");

    return (ch == eTOKEN_STRING) ? eTOKEN_ID:eTOKEN_BAD;
}

int
ScriptingLex::GetField(pnl::pnlString& str, const char* aDelimiter)
{
    int ch;
    char bitmap[32];
    
    str.resize(0);
    memset((void*)bitmap, 0, sizeof(bitmap));
    if(aDelimiter)
    {
	for(; *aDelimiter; ++aDelimiter)
	{
	    bitmap[*aDelimiter >> 3] |= (1 << (*aDelimiter & 7));
	}
    }
    for(;;)
    {
        if((ch = Getch()) == -1)
	{
            return eTOKEN_STRING;
	}
	if((bitmap[ch >> 3] & (1 << (ch & 7))) != 0)
	{
            Ungetch(ch);
            return eTOKEN_STRING;
	}
        str << char(ch);
    }
}

int
ScriptingLex::GetQString(pnl::pnlString& str, int quotationMark)
{
    char aDelimiter[2];
    
    aDelimiter[0] = char(quotationMark);
    aDelimiter[1] = 0;

    return (GetField(str, aDelimiter) != eTOKEN_STRING || Getch() != quotationMark)
	? eTOKEN_BAD:eTOKEN_STRING;
}

int
ScriptingLex::GetchAfterSpaces()
{
    int ch;
    
    for(;;)
    {
        ch = Getch();
        if(ch == -1 || (ch != ' ' && ch != '\t'))
	{
	    if(ch == '#' || ch == '/')
	    {
		for(;ch != -1; ch = Getch())
		{
		    if(ch == '\r' || ch == '\n')
		    {
			break;
		    }
		}
	    }
            return ch;
	}
    }
}

int ScriptingLex::Getch()
{
    if(m_Ungetch != -70000)
    {
	int ch = m_Ungetch;
	m_Ungetch = -70000;
	return ch;
    }
    
    if(m_BufPos < m_BufSize)
    {
	return m_Buf[m_BufPos++];
    }

    if(!feof(m_pFile))
    {
	m_BufSize = m_BufPos = 0;
	if(fgets((char*)m_Buf, sizeof(m_Buf), m_pFile))
	{
	    m_BufSize = strlen((char*)m_Buf);
	}
	if(m_BufPos < m_BufSize)
	{
	    return m_Buf[m_BufPos++];
	}
    }
    
    Ungetch(-1);

    return -1;
}

int ScriptingSyntax(ScriptingLex &lex, pnl::pnlString &fname, pnl::pnlVector<pnl::pnlString> &args)
{
    pnl::pnlString arg;
    int token;
    
    while((token = lex.GetToken(arg)) == eTOKEN_EOL);

    if(token != eTOKEN_ID)
    {
	return -1;
    }

    fname = arg;
    args.resize(0);

    if(lex.GetToken(arg) != eTOKEN_BRACE_OPEN)
    {
	return -1;
    }
    token = lex.GetToken(arg);
    if(token == eTOKEN_STRING || token == eTOKEN_ID)
    {
	args.push_back(arg);
	for(;(token = lex.GetToken(arg)) == eTOKEN_DELIMITER;)
	{
	    token = lex.GetToken(arg);
	    if(token == eTOKEN_STRING || token == eTOKEN_ID)
	    {
		args.push_back(arg);
	    }
	    else
	    {
		break;
	    }
	}
    }

    if(token != eTOKEN_BRACE_CLOSE)
    {
	return -1;
    }

    if((token = lex.GetToken(arg)) == eTOKEN_EOL)
    {
	return args.size();
    }

    return -1;
}

FuncDesc* FunctionInfo(pnl::pnlString &fname)
{
    pnl::pnlString fnameLowerCase;
    int i;

    for(i = 0; i < fname.size(); ++i)
    {
	fnameLowerCase << char(tolower(fname[i]));
    }

    for(i = 0; aFuncDesc[i].name; ++i)
    {
	if(fnameLowerCase == aFuncDesc[i].name)
	{
	    return aFuncDesc + i;
	}
    }

    return 0;
}

void ListCommands(pnl::pnlString &arg)
{
    int i, len, maxName, cols;
    pnl::pnlVector<pnl::pnlString> aCommand;
    pnl::pnlString carg;

    for(i = 0; i < arg.length(); ++i)
    {
	carg << char(tolower(arg[i]));
    }

    for(maxName = i = 0; aFuncDesc[i].name; ++i)
    {
	if(carg.length() && memcmp(aFuncDesc[i].name, carg.c_str(), carg.length()))
	{
	    continue;
	}
	aCommand.push_back(aFuncDesc[i].name);
	len = aCommand.back().length();
	maxName = (maxName > len) ? maxName:len;
    }

    cols = (80 / (maxName + 1));

    for(i = 0; i < aCommand.size(); ++i)
    {
	len = aCommand[i].length();
	std::cout << aCommand[i];

	if(((i % 3) == 2) || (i == aCommand.size() - 1))
	{
	    std::cout << '\n';
	}
	else
	{
	    while(len < (maxName + 1))
	    {
		len++;
		std::cout << ' ';
	    }
	}
    }
}
