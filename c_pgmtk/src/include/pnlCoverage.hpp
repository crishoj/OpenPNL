/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCoverage.hpp                                             //
//                                                                         //
//  Purpose:   needed to measure code/decision coverage                    //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLCOVERAGE_HPP__
#define __PNLCOVERAGE_HPP__

#if defined( PNL_I_WANT_CODE_COVERAGE ) || defined( PNL_I_WANT_DECISION_COVERAGE )

#include <iostream>
#include <map>
#include <utility>
#include <string.h>

#include "pnlConfig.hpp"

PNL_BEGIN

struct ccIfInfo
{
    bool taken[2];

    ccIfInfo()
    {
        taken[0] = taken[1] = false;
    }
};

struct ccIfId
{
    char const *file;
    int line;
    ccIfId( char const *file, int line )
    {
        this->file = file;
        this->line = line;
    }

    bool operator==( ccIfId b ) const
    {
        return (line == b.line) && (strcmp( file, b.file ) == 0 );
    }

    bool operator<( ccIfId b ) const
    {
        if ( line < b.line )
        {
            return true;
        }
        if ( line > b.line )
        {
            return false;
        }
        return strcmp( file, b.file ) < 0;
    }
};

enum ccStat
{
    PNL_CC_UNREACHED = -1,
    PNL_CC_IF = 0,
    PNL_CC_FOR,
    PNL_CC_SWITCH,
    PNL_CC_RETURN,
    PNL_CC_DO,
    PNL_CC_GOTO,
    PNL_CC_BREAK
};

struct ccGenInfo
{
    ccStat stat;

    ccGenInfo()
    {
        stat = PNL_CC_UNREACHED;
    }
};

struct ccGenId
{
    char const *file;
    int line;
    ccStat stat;

    ccGenId( char const *file, int line, ccStat stat )
    {
        this->file = file;
        this->line = line;
        this->stat = stat;
    }

    bool operator==( ccGenId b ) const
    {
        return (line == b.line) && (strcmp( file, b.file ) == 0 ) && (stat == b.stat);
    }

    bool operator<( ccGenId b ) const
    {
        if ( line < b.line )
        {
            return true;
        }
        if ( line > b.line )
        {
            return false;
        }
        if ( stat < b.stat )
        {
            return true;
        }
        if ( stat > b.stat )
        {
            return false;
        }
        return strcmp( file, b.file ) < 0;
    }
};

extern PNL_API bool cc_if_gadget( bool cond, char const *file, int line );
extern PNL_API bool cc_gen_gadget( char const *file, int line, ccStat );

#define PNL_IF \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_IF_" __FILE__, __LINE__, pnl::PNL_CC_IF ) ) {} else if

#define PNL_FOR \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_FOR_" __FILE__, __LINE__, pnl::PNL_CC_FOR ) ) {} else for

#define PNL_SWITCH \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_SWITCH_" __FILE__, __LINE__, pnl::PNL_CC_SWITCH ) ) {} else switch

#define PNL_RETURN \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_RETURN_" __FILE__, __LINE__, pnl::PNL_CC_RETURN ) ) {} else return

#define PNL_DO \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_DO_" __FILE__, __LINE__, pnl::PNL_CC_DO ) ) {} else do

#define PNL_GOTO \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_GOTO_" __FILE__, __LINE__, pnl::PNL_CC_GOTO ) ) {} else goto

#define PNL_BREAK \
    if ( pnl::cc_gen_gadget( "CC_SPECIALUNIQMARK_GEN_MARK_BREAK_" __FILE__, __LINE__, pnl::PNL_CC_BREAK ) ) {} else break

// this one is for measuring decision coverage statistics
#define PNL_IF2( COND ) \
    if ( pnl::cc_if_gadget( (COND), "CC_SPECIALUNIQMARK_GEN_MARK_IF_" __FILE__, __LINE__ ) )

#if PNL_I_WANT_DECISION_COVERAGE
#define if( COND ) PNL_IF2( COND )

#else

#define if PNL_IF
#define for PNL_FOR
#define switch PNL_SWITCH
#define return PNL_RETURN
#define do PNL_DO
#define goto PNL_GOTO
#define break PNL_BREAK
#endif

PNL_END

#endif
#endif

/* end of file */
