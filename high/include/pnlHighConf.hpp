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
#pragma warning(disable : 4251) // class 'xx' needs to have dll-interface to be used by clients of class 'yy'
#pragma warning(disable : 4512)
//temporary
//#pragma warning(disable : 4702) //unreachable code
#pragma warning(disable : 4127) // conditional expression is constant
#endif // _MSC_VER

#if !defined(WIN32)
#define PNLHIGH_API
#else
#ifdef PNLHIGH_EXPORTS
#define PNLHIGH_API __declspec(dllexport) 
#else
#define PNLHIGH_API __declspec(dllimport)
#endif
#endif // WIN32

namespace NetConst
{
    typedef enum
    {	eCSV
    ,	eTSV
    } ESavingType;    
};

#define vectorOffset ((pnl::EMatrixType)-1)

#define PNLW_BEGIN namespace pnlw {
#define PNLW_END };
#define PNLW_USING using namespace pnlw;

// FORWARDS
PNLW_BEGIN
class TokArr;
PNLW_END

namespace pnl
{
    class pnlString;
};

#define Vector pnl::pnlVector
#include "pnlConfig.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlString.hpp"

typedef pnl::pnlString String;

PNLW_BEGIN

// print error and throw exception
PNLHIGH_API void ThrowInternalError(const char *message, const char *func);
// print error and throw exception (differs from previous by message)
PNLHIGH_API void ThrowUsingError(const char *message, const char *func);
// Tok identifies for discrete and continuous node types

extern PNLHIGH_API TokArr discrete;
extern PNLHIGH_API TokArr continuous;

extern PNLHIGH_API TokArr chance;
extern PNLHIGH_API TokArr decision;
extern PNLHIGH_API TokArr value;

class TestsPnlHigh;
#define ALLOW_TESTING friend class TestsPnlHigh;

PNLW_END

#endif //__PNLHIGHCONFIG_HPP__
