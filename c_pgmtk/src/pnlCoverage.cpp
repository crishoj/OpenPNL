/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCoverage.cpp                                             //
//                                                                         //
//  Purpose:   needed to measure code/decision coverage                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlCoverage.hpp"

#if defined( WIN32 )
#include <windows.h>
#endif

#if defined( PNL_I_WANT_CODE_COVERAGE ) || defined( PNL_I_WANT_DECISION_COVERAGE )

PNL_BEGIN

#undef if
#undef for
#undef return

bool PNL_API cc_if_gadget( bool cond, char const *file, int line )
{
    static std::map< ccIfId, ccIfInfo > cc_if_map;
    ccIfInfo &info = cc_if_map[ccIfId( file, line )];
    if ( !info.taken[cond] )
    {
        info.taken[cond] = true;
        std::cout << std::endl << "DECKOV: " << file << " " << line
                  << " -- taken " << cond << std::endl;
    }
    return cond;
}

bool PNL_API cc_gen_gadget( char const *file, int line, ccStat stat )
{
#if defined( WIN32 )
    static int coun = 0;
#endif
    static std::map< ccGenId, ccGenInfo > cc_map;
    ccGenInfo &info = cc_map[ccGenId( file, line, stat )];
    if ( info.stat == PNL_CC_UNREACHED )
    {
        info.stat = stat;
        std::cout << std::endl << "KODKOV: " << file << " " << line
                  << " -- reached, stat " << stat << std::endl;
    }

// needed due to bad behaviour of W2K scheduler
#if defined( WIN32 )
    if ( ++coun > 10000 )
    {
        coun = 0;
        Sleep( 1 );
    } 
#endif

    return false;
}

PNL_END

#endif
