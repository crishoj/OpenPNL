#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include "pnlHigh.hpp"

// FORWARDS
namespace pnlw {
class BayesNet;
class TokArr;
};

PNLW_USING

namespace pnl
{
    template<typename Type, typename Allocator> class pnlVector;
    class pnlString;
};

class Scripting
{
public:
    Scripting();
    ~Scripting();
    int Execute(FILE *stream, BayesNet *bnet = 0);
    int ExecuteACommand(pnl::pnlString &fname, pnl::pnlVector<pnl::pnlString> &args);
    BayesNet &BNet() const;
    FILE *Stdin() const;
    FILE *Stdout() const;
    bool CheckOrCreate(const char *etalon, const char *script, const char *id);

protected:
    void Enter(BayesNet *bnet, bool bAutoDelete);
    void Enter(FILE *stdinF = 0, FILE *stdoutF = 0, char bAutodelete = 0);
    void Enter(const char *stdinName, const char *stdoutName);
    void Leave();

    void Print(const Vector<String> &v);
    void Print(const TokArr &v);
    void Print(const String &s);
    void PrintStripped(TokArr &v);

private:
    pnl::pnlVector<BayesNet*> m_apBnet;
    pnl::pnlVector<FILE*> m_aStdin;
    pnl::pnlVector<FILE*> m_aStdout;
    pnl::pnlVector<char> m_aAutodeleteFlags;
};

#endif
