#include "pnlHigh.hpp"
#include "WInner.hpp"
#include "console.hpp"
#include "pnlWGraph.hpp"
#include "pnlWProbabilisticNet.hpp"

#include "pnlConfig.h"
#include "example_common_funs.hpp"

#include "script_lex.hpp"

Scripting::Scripting()
{
    Enter(stdin, stdout, 0);
    Enter(new BayesNet, true);
}

Scripting::~Scripting()
{
    Leave();
    Leave();
}

int Scripting::Execute(FILE *file, BayesNet *bnet)
{
    ScriptingLex lex(file);
    pnl::pnlString fname;
    pnl::pnlVector<pnl::pnlString> args;
    int nCommand = 0;

    if(bnet)
    {
	Enter(bnet, false);
    }
    else if(!m_apBnet.size())
    {
	Enter(new BayesNet, true);
    }

    while(true)
    {
	if(Stdout() == stdout && file == stdin)
	{
	    fprintf(stderr, "= ");
	    fflush(stderr);
	}
	if(ScriptingSyntax(lex, fname, args) < 0)
	{
	    if(feof(file))
	    {
		break;
	    }
	    fprintf(Stdout(), "Syntax error, resync...\n");

	    int token = lex.GetToken(fname);

	    while(token != eTOKEN_EOL && token != eTOKEN_EOF)
	    {
		token = lex.GetToken(fname);
	    }
	    if(token == eTOKEN_EOF)
	    {
		break;
	    }
	    fprintf(Stdout(), "Resync done...\n");
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

int Scripting::ExecuteACommand(pnl::pnlString &fname, pnl::pnlVector<pnl::pnlString> &args)
{
    FuncDesc *pFD = FunctionInfo(fname);

    if(!pFD)
    {
	fprintf(Stdout(), "Unrecognized command: '%s'\n", fname.c_str());
	return -2;
    }

    if(pFD->nRightArg > args.size())
    {
	if(pFD->nRightArg - pFD->nOptArg > args.size())
	{
	    fprintf(Stdout(), "Error (function '%s'): too few arguments\n", fname.c_str());
	    return -2;
	}
	if(pFD->bSimple)
	{
	    args.resize(pFD->nRightArg);
	}
    }
    else if(pFD->nRightArg < args.size())
    {
	fprintf(Stdout(), "'%s' warning: unnecessary arguments are ignored\n", fname.c_str());
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
		    fclose(fp);
		}
		else
		{
		    fprintf(Stdout(), "can't open file '%s'\n", args[0].c_str());
		}

		break;
	    }
	case -2:// clear
	    Leave();
	    Enter(new BayesNet, true);
	    break;
	case -3:// clearAll
	    for(;m_apBnet.size() > 1;Leave());
	    Enter(new BayesNet, true);
	    break;
	case -4:/* ListNodes */ Print(BNet().Net().Graph().Names()); break;
	case -5:/* Exit */ return -1;
	case -6:	ListCommands(args[0]); break;
	case -7:/* print */ Print(args); break;
	case -8:/* new */ Enter(new BayesNet, true); break;
	case -9:/* console */ Execute(stdin, &BNet()); break;
	case -10:/* comparefiles */
	    {
		const char *ptr = (CompareFiles(args[0].c_str(), args[1].c_str()) == 0)
		    ? "Ok":"FAILED";

		if(args[2].length())
		{
		    args[2] << ": " << ptr;
		}
		else
		{
		    args[2] = ptr;
		}
		Print(args[2]);
                break;

	    }
	case -11:/* checkOrCreate */ CheckOrCreate(args[0].c_str(), args[1].c_str(), args[2].c_str());break;
	case eAddNode:	    BNet().AddNode(args[0], args[1]); break;
	case eDelNode:	    BNet().DelNode(args[0]); break;
	case eAddArc:	    BNet().AddArc (args[0], args[1]); break;
	case eDelArc:	    BNet().DelArc (args[0], args[1]); break;
	case eLoadNet:	    BNet().LoadNet(args[0].c_str());  break;
	case eSaveNet:	    BNet().SaveNet(args[0].c_str());  break;
	case eLoadEvidBuf:  BNet().LoadEvidBuf(args[0].c_str());  break;
	case eSaveEvidBuf:  BNet().SaveEvidBuf(args[0].c_str());  break;
	case eSetPTabular:  BNet().SetPTabular(args[0], args[1], args[2]); break;
	case eGetPTabular:  Print(BNet().GetPTabular(args[0], args[1]));break;
	case eLearnStructure:	BNet().LearnStructure(0, 0); break;
        case eGaussianMean: Print(BNet().GetGaussianMean(args[0]));break;
        case eGaussianCovar:Print(BNet().GetGaussianCovar(args[0]));break;
	case eSetProperty:  BNet().SetProperty(args[0].c_str(), args[1].c_str()); break;
	case eGetProperty:  Print(BNet().GetProperty(args[0].c_str())); break;
        case eSetPGaussian:  
            {
                BNet().SetPGaussian(args[0], args[1], args[2], args[3]);
                break;
            }

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
	case eEditEvidence: BNet().EditEvidence(args[0]); break;
	case eClearEvid:    BNet().ClearEvid(); break;
	case eAddEvidToBuf: BNet().AddEvidToBuf(args[0]); break;
	case eClearEvidBuf: BNet().ClearEvidBuf(); break;
	case eCurEvidToBuf: BNet().CurEvidToBuf(); break;
	case eLearn:	    BNet().LearnParameters(); break;
	case eMPE:	    Print(BNet().GetMPE(args[0])); break;
	case eJPD:	    Print(BNet().GetJPD(args[0])); break;
	case eNodeType:	    Print(BNet().GetNodeType(args[0])); break;
	case eGetNeighbors: Print(BNet().GetNeighbors(args[0])); break;
	case eGetChildren:  Print(BNet().GetChildren (args[0])); break;
	case eGetParents:   Print(BNet().GetParents  (args[0])); break;
	default:
	    fprintf(Stdout(), "Unrealized command: '%s'\n", fname.c_str());
	    return -2;
	}
    }

    catch(pnl::CException &ex)
    {
	fprintf(Stdout(), "\nException during execution of '%s' command: %s\n",
	    fname.c_str(), ex.GetMessage());
	return -2;
    }

    catch(...)
    {
	fprintf(Stdout(), "\nUnrecognized exception during execution of '%s'\n", fname.c_str());
	return -2;
    }

    return 0;
}

BayesNet &Scripting::BNet() const
{
    return *m_apBnet.back();
}

FILE *Scripting::Stdin() const
{
    return m_aStdin.back();
}

FILE *Scripting::Stdout() const
{
    return m_aStdout.back();
}

bool Scripting::CheckOrCreate(const char *etalon, const char *script, const char *id)
{
    FILE *etalonF = fopen(etalon, "r");
    const char *tmpFile = "tmpCompFile.txt";

    if(!etalonF)
    {
	tmpFile = etalon;
    }
    else
    {
	fclose(etalonF);
    }

    Enter(script, tmpFile);
    Execute(Stdin(), &BNet());
    Leave();
    if(tmpFile != etalon)
    {
	int result = CompareFiles(etalon, tmpFile);
	fprintf(Stdout(), "%s: %s\n", id, (result == 0) ? "Ok":"FAILED");
	if(remove(tmpFile))
	{
	    fprintf(Stdout(), "can't delete tmp file '%s'\n", tmpFile);
	}
	return result == 0;
    }

    return false;
}

void Scripting::Enter(BayesNet *bnet, bool bAutoDelete)
{
    m_apBnet.push_back(bnet);
    m_aStdin.push_back(m_aStdin.back());
    m_aStdout.push_back(m_aStdout.back());
    m_aAutodeleteFlags.push_back(bAutoDelete ? 1:0);
}

void Scripting::Enter(FILE *stdinF, FILE *stdoutF, char bAutodelete)
{
    if(!stdinF)
    {
	stdinF = Stdin();
	bAutodelete &= ~2;
    }
    if(!stdoutF)
    {
	stdoutF = Stdout();
	bAutodelete &= ~4;
    }
    m_aStdin.push_back(stdinF);
    m_aStdout.push_back(stdoutF);
    m_apBnet.push_back(m_apBnet.size() ? m_apBnet.back():0);
    m_aAutodeleteFlags.push_back(bAutodelete);
}

void Scripting::Enter(const char *stdinName, const char *stdoutName)
{
    FILE *stdinF = 0, *stdoutF = 0;
    char autoDelete = 0;

    if(stdinName)
    {
	stdinF = fopen(stdinName, "r");
	if(!stdinF)
	{
	    fprintf(Stdout(), "can't open file '%s' for input\n", stdinName);
	}
	else
	{
	    autoDelete |= 2;
	}
    }
    if(stdoutName)
    {
	stdoutF = fopen(stdoutName, "w");
	if(!stdoutF)
	{
	    fprintf(Stdout(), "can't open file '%s' for output\n", stdoutName);
	}
	else
	{
	    autoDelete |= 4;
	}
    }
    Enter(stdinF, stdoutF, autoDelete);
}

void Scripting::Leave()
{
    int flags = m_aAutodeleteFlags.back();

    if(flags & 1)
    {
	delete m_apBnet.back();
    }
    if(flags & 2)
    {
	fclose(m_aStdin.back());
    }
    if(flags & 4)
    {
	fclose(m_aStdout.back());
    }
    m_aAutodeleteFlags.pop_back();
    m_apBnet.pop_back();
    m_aStdin.pop_back();
    m_aStdout.pop_back();
}

void Scripting::Print(const Vector<String> &v)
{
    for(unsigned int i = 0; i < v.size(); ++i)
    {
	fprintf(Stdout(), "%s%c", v[i].c_str(), ((i == v.size() - 1) ? '\n':' '));
    }
}

void Scripting::Print(const TokArr &v)
{
    fprintf(Stdout(), "%s\n", String(v).c_str());
}

void Scripting::Print(const String &s)
{
    fprintf(Stdout(), "%s\n", s.c_str());
}

void Scripting::PrintStripped(TokArr &v)
{
    if(!v.size())
    {
	return;
    }
    for(int i = 0; i < v.size(); ++i)
    {
	fprintf(Stdout(), "%s%c", String(v[i].Name()).c_str(), ((i == int(v.size() - 1)) ? '\n':' '));
    }
}

