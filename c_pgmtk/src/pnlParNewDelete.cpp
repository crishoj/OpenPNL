/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParNewDelete.cpp                                         //
//                                                                         //
//  Purpose:   implementation of operator new and operator delete          //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlParNewDelete.hpp"

#ifdef PAR_USE_OMP_ALLOCATOR

#include <windows.h>
#include <omp.h>

#define NUMBER_OF_HEAPS 256

HANDLE HeapsArray[NUMBER_OF_HEAPS] =
{
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
// ----------------------------------------------------------------------------

void *operator new(size_t Size)
{
    int myid = omp_get_thread_num();
    
    if ((myid<0)||(myid>=NUMBER_OF_HEAPS))
        return NULL;
    
    char *pCharPointer;
    
    if (HeapsArray[myid]==NULL)
        HeapsArray[myid] = HeapCreate(0, 0, 0);
    
    pCharPointer = reinterpret_cast<char*>(HeapAlloc(HeapsArray[myid], 0, 
        Size + 1));
    
    if (pCharPointer!=NULL)
    {
        *pCharPointer = myid;
        
        return (pCharPointer + 1);
    }
    else 
        return NULL;
}
// ----------------------------------------------------------------------------

void operator delete (void *pPointer)
{
    if (pPointer == NULL)
        return;
    
    char *pCharPointer = reinterpret_cast<char*>(pPointer)-1;
    
    int myheap = *(pCharPointer);
    
    if (HeapsArray[myheap]!=NULL)
        HeapFree(HeapsArray[myheap], 0, reinterpret_cast<void*>(pCharPointer));
}

#endif // PAR_USE_OMP_ALLOCATOR

// end of file ----------------------------------------------------------------
