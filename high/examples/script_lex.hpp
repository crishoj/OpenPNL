#ifndef __SCRIPT_LEX_HPP__
#define __SCRIPT_LEX_HPP__

#include "pnlHigh.hpp"

// FORWARDS
namespace pnl
{
    template<typename Type, typename Allocator> class pnlVector;
    class pnlString;
};
class ScriptingLex;

//struct FILE;

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

struct FuncDesc
{
    const char *name;// function name in lowercase
    int id;// unique id - from enum in WInner.hpp
    bool bSimple;// left and right arguments - TokArr's
    int nLeftArg;
    int nRightArg;
    int nOptArg;
};

FuncDesc* FunctionInfo(pnl::pnlString &fname);
void ListCommands(pnl::pnlString &arg);
int ScriptingSyntax(ScriptingLex &lex, pnl::pnlString &fname,
		    pnl::pnlVector<pnl::pnlString> &args);

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

#endif // include guard
