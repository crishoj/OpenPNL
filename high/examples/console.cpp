#include "pnlHigh.hpp"
#include "WInner.hpp"
#include "console.hpp"
#include "pnlWGraph.hpp"
#include "pnlWProbabilisticNet.hpp"

#include "pnlConfig.h"

enum EToken
{   eTOKEN_ID
,   eTOKEN_DELIMITER
,   eTOKEN_OP_CATENATION
,   eTOKEN_OP_CARTESIAN
,   eTOKEN_STRING
,   eTOKEN_BRACE_OPEN
,   eTOKEN_BRACE_CLOSE
,   eTOKEN_EOL
,   eTOKEN_EOF
,   eTOKEN_BAD
};

class ScriptingLex
{
public:
    // get token
    int GetToken(pnl::pnlString& str);   
        
    ScriptingLex(FILE *pFile)
        : m_pFile(pFile), m_Ungetch(-70000), m_BufSize(0), m_BufPos(0)
    {
    }

protected:
    int Getch();
    void Ungetch(int ch)
    {
        m_Ungetch = ch;
    }
    int GetchAfterSpaces();

    int GetTag(pnl::pnlString& str);
    int GetAttribute(pnl::pnlString& str);

    int GetField(pnl::pnlString& str, const char* aDelimiter = NULL);
    int GetQString(pnl::pnlString& str, int quotationMark);

private:
    FILE *m_pFile;
    unsigned char m_Buf[512];
    int m_BufSize;
    int m_BufPos;
    int m_Ungetch;
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

struct FuncDesc
{
    const char *name;// function name in lowercase
    int id;// unique id - from enum in WInner.hpp
    bool bSimple;// left and right arguments - TokArr's
    int nLeftArg;
    int nRightArg;
    int nOptArg;
};

FuncDesc aFuncDesc[] =
{   "savenet",      eSaveNet,   false, 0, 1, 0
,   "loadnet",      eLoadNet,	false, 0, 1, 0
,   "savelearnbuf", eSaveLearnBuf,false,0,1, 0
,   "loadlearnbuf", eLoadLearnBuf,false,0,1, 0
,   "addnode",	    eAddNode,	true,  0, 2, 0
,   "addarc",	    eAddArc,	true,  0, 2, 0
,   "setp",	    eSetP,	true,  0, 3, 1
,   "p",	    eP,		true,  1, 2, 1
,   "evid",	    eEvidence,	false, 0, 2, 1
,   "clearevid",    eClearEvid,	true,  0, 0, 0
,   "clearevidhistory", eClearEvidHistory, true, 0, 0, 0
,   "learn",	    eLearn,	true,  0, 0, 0
,   "mpe",	    eMPE,	true,  1, 1, 1
,   "generateevidences", eGenerateEvidences, false, 0, 3, 2
,   "learnstructure", eLearnStructure, true, 0, 0, 0
,   "gaussianmean", eGaussianMean,true, 1, 1, 0
,   "gaussiancovar", eGaussianCovar, true, 1, 2, 0 
,   "setgaussian", eSetGaussian, false, 0, 3, 2
// build-in commands
,   "execute",	    -1,		false, 0, 1, 0
,   "clear",	    -2,		true,  0, 0, 0
,   "clearall",	    -3,		true,  0, 0, 0
,   "listnodes",    -4,		true,  0, 0, 0
,   "exit",	    -5,		true,  0, 0, 0
,   "listcommands", -6,		true,  0, 0, 0
,   "print",	    -7,		true,  0, 1, 1
,   "new",	    -8,		true,  0, 0, 0
,   "console",	    -9,		true,  0, 0, 0
,   0, 0, 0, 0, 0, 0
};

FuncDesc* FunctionInfo(pnl::pnlString &fname);

int Scripting::Execute(FILE *file, BayesNet *bnet)
{
    ScriptingLex lex(file);
    pnl::pnlString fname;
    pnl::pnlVector<pnl::pnlString> args;
    int nCommand = 0;

    if(bnet)
    {
	Enter(bnet, false);
    } else if(!m_apBnet.size())
    {
	Enter(new BayesNet, true);
    }

    while(true)
    {
	if(file == stdin)
	{
	    fprintf(stdout, "= ");
	    fflush(stdout);
	}
	if(ScriptingSyntax(lex, fname, args) < 0)
	{
	    if(feof(file))
	    {
		break;
	    }
	    fprintf(stderr, "Syntax error, resync...\n");

	    int token = lex.GetToken(fname);

	    while(token != eTOKEN_EOL && token != eTOKEN_EOF)
	    {
		token = lex.GetToken(fname);
	    }
	    if(token == eTOKEN_EOF)
	    {
		break;
	    }
	    fprintf(stderr, "Resync done...\n");
	    continue;
	}

	if(ExecuteACommand(fname, args) == -1)// check for exit command
	{
	    break;
	}
    }

    if(bnet)
    {
	Leave();
    }

    return nCommand;
}

static void Print(Vector<String> &v)
{
    for(unsigned int i = 0; i < v.size(); ++i)
    {
	cout << v[i] << ((i == v.size() - 1) ? '\n':' ');
    }
}

static void Print(TokArr &v)
{
    cout << String(v) << "\n";
}

static void PrintStripped(TokArr &v)
{
    if(!v.size())
    {
	return;
    }
    for(int i = 0; i < v.size(); ++i)
    {
	cout << String(v[i].Name()) << ' ';
    }
    cout << '\n';
}

static void ListCommands()
{
    int i, len, maxName, cols;

    for(maxName = i = 0; aFuncDesc[i].name; ++i)
    {
	len = strlen(aFuncDesc[i].name);
	maxName = (maxName > len) ? maxName:len;
    }

    cols = (80 / (maxName + 1));

    for(i = 0; aFuncDesc[i].name; ++i)
    {
	len = strlen(aFuncDesc[i].name);
	cout << aFuncDesc[i].name;

	if(((i % 3) == 2) || !aFuncDesc[i + 1].name)
	{
	    cout << '\n';
	}
	else
	{
	    while(len < (maxName + 1))
	    {
		len++;
		cout << ' ';
	    }
	}
    }
}

int Scripting::ExecuteACommand(pnl::pnlString &fname, pnl::pnlVector<pnl::pnlString> &args)
{
    FuncDesc *pFD = FunctionInfo(fname);

    if(!pFD)
    {
	cout << "Unrecognized command: '" << fname << "'\n";
	return -2;
    }

    if(pFD->nRightArg > args.size())
    {
	if(pFD->nRightArg - pFD->nOptArg > args.size())
	{
	    cout << "Error (function '" << fname << "'): too few arguments\n";
	    return -2;
	}
	if(pFD->bSimple)
	{
	    args.resize(pFD->nRightArg);
	}
    }
    else if(pFD->nRightArg < args.size())
    {
	cerr << "'" << fname << "' warning: unnecessary arguments are ignored\n";
    }

    try
    {
	switch(pFD->id)
	{
	case -1:// execute
	    {
		FILE *fp = fopen(args[0].c_str(), "r");

		if(fp)
		{
		    Execute(fp);
		}

		fclose(fp);

		break;
	    }
	case -2:// clear
	    Leave();
	    Enter(new BayesNet, true);
	    break;
	case -3:// clearAll
	    for(;m_apBnet.size();Leave());
	    Enter(new BayesNet, true);
	    break;
	case -4:/* ListNodes */ Print(BNet().Net().Graph()->Names()); break;
	case -5:/* Exit */ return -1;
	case -6:	ListCommands(); break;
	case -7:/* print */
	    {
		for(int i = 0; i < args.size(); ++i)
		{
		    cout << args[i];
		}
		cout << '\n';
	    }
	    break;
	case -8:/* new */ Enter(new BayesNet, true); break;
	case -9:/* console */ Execute(stdin, &BNet()); break;
	case eAddNode:	    BNet().AddNode(args[0], args[1]); break;
	case eAddArc:	    BNet().AddArc (args[0], args[1]); break;
	case eLoadNet:	    BNet().LoadNet(args[0].c_str());  break;
	case eSaveNet:	    BNet().SaveNet(args[0].c_str());  break;
	case eLoadLearnBuf: BNet().LoadLearnBuf(args[0].c_str());  break;
	case eSaveLearnBuf: BNet().SaveLearnBuf(args[0].c_str());  break;
	case eSetP:	    BNet().SetP(args[0], args[1], args[2]); break;
	case eP:	    Print(BNet().P(args[0], args[1]));break;
	case eLearnStructure:BNet().LearnStructure(0, 0); break;
        case eGaussianMean: Print(BNet().GaussianMean(args[0]));break;
        case eGaussianCovar: Print(BNet().GaussianCovar(args[0], args[1]));break;
        case eSetGaussian: BNet().SetGaussian(args[0], args[1], args[2]);break;

	case eGenerateEvidences:
	    {
		int nSample = atoi(args[0].c_str());
		bool bIgnoreCurEvid = false;
		TokArr whatNodes;

		if(args.size() > 1)
		{
		    bIgnoreCurEvid = (atoi(args[1].c_str()) != 0);
		    if(args.size() > 2)
		    {
			whatNodes = args[2];
		    }
		}

		if(nSample)
		{
		    BNet().GenerateEvidences(nSample, bIgnoreCurEvid, whatNodes);
		}
		break;
	    }
	case eEvidence:
	    {
		TokArr arg1;
		bool arg2 = false;

		if(args.size() > 0)
		{
		    arg1 = args[0];
		    if(args.size() > 1)
		    {
			arg2 = (atoi(args[1].c_str()) != 0);
		    }
		}

		BNet().Evid(arg1, arg2);
		break;
	    }
	case eClearEvid:    BNet().ClearEvid(); break;
	case eClearEvidHistory:BNet().ClearEvidHistory(); break;
	case eLearn:	    BNet().Learn(); break;
	case eMPE:	    Print(BNet().MPE(args[0])); break;
	case eNodeType:
//	case eSetGaussian:
	default:
	    cout << "Unrealized command: '" << fname << "'\n";
	    return -2;
	}
    }

    catch(pnl::CException &ex)
    {
	cout << "\nException during execution of '" << fname << "' command: "
	    << ex.GetMessage() << "\n";
	return -2;
    }

    catch(...)
    {
	cout << "\nUnrecognized exception during execution of '" << fname << "'\n";
	return -2;
    }

    return 0;
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

BayesNet &Scripting::BNet() const
{
    return *m_apBnet.back();
}
