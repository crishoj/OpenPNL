#ifndef __PNLHIGHCONFIG_HPP__
#define __PNLHIGHCONFIG_HPP__

#if defined(_MSC_VER)
//#pragma warning(disable : 4251) // class needs to have dll-interface to be used by clients of class
//#pragma warning(disable : 4018) // signed/unsigned mismatch
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4786) // The identifier string exceeded the maximum allowable length and was truncated
//#pragma warning(disable : 4275) // non dll-interface class
#pragma warning(disable : 4552) // operator has no effect
#pragma warning(disable : 4512) // operator has no effect
//temporary
//#pragma warning(disable : 4702) //unreachable code
#pragma warning(disable : 4127) // conditional expression is constant
#endif // _MSC_VER

#ifdef PNLHIGH_EXPORTS
#define PNLHIGH_API __declspec(dllexport) 
#else
#define PNLHIGH_API __declspec(dllimport)
#endif

#pragma warning(push, 2)
#define Vector pnl::pnlVector
#include "pnlConfig.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlString.hpp"

typedef pnl::pnlString String;
#pragma warning(pop)

void ThrowInternalError(const char *message, const char *func);
void ThrowUsingError(const char *message, const char *func);

namespace pnl
{
}

using namespace pnl;

#endif //__PNLHIGHCONFIG_HPP__
