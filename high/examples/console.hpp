#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include "pnlHigh.hpp"

// FORWARDS
class BayesNet;
struct TokArr;

namespace pnl
{
    template<typename Type, typename Allocator> class pnlVector;
    class pnlString;
};

class Scripting
{
public:
    Scripting() {}
    int Execute(FILE *stream, BayesNet *bnet = 0);
    int ExecuteACommand(pnl::pnlString &fname, pnl::pnlVector<pnl::pnlString> &args);
    ~Scripting() {}
    BayesNet &BNet() const;

protected:
    void Enter(BayesNet *bnet, bool bAutoDelete)
    {
	m_apBnet.push_back(bnet);
	m_aBnetAutodelete.push_back(bAutoDelete);
    }

    void Leave()
    {
	if(m_aBnetAutodelete.back())
	{
	    delete m_apBnet.back();
	}
	m_apBnet.pop_back();
	m_aBnetAutodelete.pop_back();
    }

private:
    pnl::pnlVector<BayesNet*> m_apBnet;
    pnl::pnlVector<bool> m_aBnetAutodelete;
};

#endif
