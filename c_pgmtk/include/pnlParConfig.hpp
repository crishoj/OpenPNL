#ifndef __PNLPARCONFIG_HPP__
#define __PNLPARCONFIG_HPP__

#ifndef WIN32
// for linux
//#define BUILD_OMP
//#define BUILD_MPI
#endif

#if defined(BUILD_OMP) || defined(_CLUSTER_OPENMP)
#define PAR_OMP
#define PAR_DYNAMIC_THREADS
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

#if defined(PAR_OMP) || defined(PAR_MPI)
#define PAR_PNL
#else
#undef PAR_PNL
#endif

#if defined(PAR_OMP) && defined(_DEBUG) && defined(WIN32)
#include "ParPNLCRTDBG.h"
#endif

#if defined(PAR_MPI) && defined(WIN32)
#ifdef _DEBUG
#pragma comment(lib,"mpichd.lib")
#else
#pragma comment(lib,"mpich.lib")
#endif
#endif

#if (defined(PAR_OMP) && defined(WIN32)) || defined(_CLUSTER_OPENMP)
#define PAR_USE_OMP_ALLOCATOR
#endif

#if defined(PAR_PNL)
#define PAR_RESULTS_RELIABILITY
#endif

#endif // __PNLPARCONFIG_HPP__
