#ifndef __PNLCONFIG_H__
#define __PNLCONFIG_H__

#if defined(_MSC_VER)
#pragma warning(disable : 4251) // class needs to have dll-interface to be used by clients of class
#pragma warning(disable : 4018) // signed/unsigned mismatch
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4786) // The identifier string exceeded the maximum allowable length and was truncated
#pragma warning(disable : 4275) // non dll-interface class
#pragma warning(disable : 4552) // operator has no effect
#pragma warning(disable : 4512) // operator has no effect
#pragma warning(disable : 4284) // return type for 'const_iterator::operator ->' is 'void *const * '
                                // (ie; not a UDT or reference to a UDT.
                                // Will produce errors if applied using infix notation)
//temporary
#pragma warning(disable : 4702) //unreachable code
#pragma warning(disable : 4127) // conditional expression is constant
#endif // _MSC_VER

#if defined(WIN32) && defined(__INTEL_COMPILER)
#pragma warning(disable : 909)
#pragma warning(disable : 444)
#pragma warning(disable : 1418)
#if 0
#pragma warning(disable : 981)
#pragma warning(disable : 424)
#pragma warning(disable : 383)
#pragma warning(disable : 171)
#pragma warning(disable : 9)
#pragma warning(disable : 654)
#pragma warning(disable : 858)
#pragma warning(disable : 1125)
#endif
#endif // defined(WIN32) && defined(__INTEL_COMPILER)

#if defined(WIN32)
#if _MSC_VER >= 1200 && _MSC_VER < 1300
#define PNL_VC6
#endif
#if _MSC_VER >= 1300 && _MSC_VER < 1311
#define PNL_VC7
#endif
#else
#ifdef __GNUG__ 
#define GCC_VERSION ( __GNUC__ * 10000 \
                      + __GNUC_MINOR__ * 100 \
                      + __GNUC_PATCHLEVEL__ )
#else
#define GCC_VERSION 0
#endif // __GNUG__
#endif // WIN32

#if defined(PNL_VC6) || defined(PNL_VC7)
#define PNL_BROKEN_TEMPLATE
// Visual Studio has broken template support - PNL works around it
#define PNL_MSC
#endif

#define PNL_PI 3.1415926536f

#if defined(PNL_STATIC) || !defined(WIN32)
#define PNL_API
#else
#ifdef PNL_EXPORTS
#define PNL_API __declspec(dllexport) 
#else
#define PNL_API __declspec(dllimport)
#endif
#endif // PNL_STATIC

#endif //__PNLCONFIG_H__
