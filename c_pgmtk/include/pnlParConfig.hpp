#ifndef __PNLPARCONFIG_HPP__
#define __PNLPARCONFIG_HPP__


// BUILD_OMP && BUILD_MPI must to be defined as project's option 
// when pnl compiles and in test module when compile test module
#ifdef BUILD_OMP
#define PAR_OMP
#else
#undef PAR_OMP
#endif // BUILD_OMP

#ifdef BUILD_MPI
#define PAR_MPI
#else
#undef PAR_MPI
#endif // BUILD_MPI

/*#if !defined(_OPENMP)
#undef PAR_OMP
#endif*/

#if !defined(WIN32)
#undef PAR_OMP
#undef PAR_MPI
// parallel addition of PNL is supporting only Windows platform yet 
#endif // WIN32

#if defined(PAR_OMP) || defined(PAR_MPI)
#define PAR_PNL
#else
#undef PAR_PNL
#endif

#if defined(PAR_OMP) && defined(_DEBUG)
#include "ParPNLCRTDBG.h"
#endif

#if defined(PAR_MPI)
#ifdef _DEBUG
#pragma comment(lib,"mpichd.lib")
#else
#pragma comment(lib,"mpich.lib")
#endif
#endif

#if defined(PAR_PNL) && defined(_DEBUG)
#define PAR_RESULTS_RELIABILITY
#endif

#endif // __PNLPARCONFIG_HPP__
