/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlException.cpp                                            //
//                                                                         //
//  Purpose:   CEvidence class member functions implementation             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlException.hpp"
#include "pnlError.h"
#include <sstream>

PNL_USING

//macros to substitute error type description
#define _CASE( __erType, __erDescr )		\
	 case __erType: { return (__erDescr);}

static char* GetErrorType( pgmErrorType erType )
{
    switch (erType)
    {
	_CASE( pgmOK, "Ok")
	    _CASE( pgmFail, "generic fail" )
	    _CASE( pnlMemoryFail, "memory fail" )
	    _CASE( pgmDamagedMemory, "damaged memory" )
	    _CASE( pgmBadPointer, "bad pointer" )
	    _CASE( pgmNumericFail, "numeric fail" )
	    _CASE( pgmOverflow, "overflow" )
	    _CASE( pgmUnderflow, "underflow" )
	    _CASE( pgmAlgorithmic, "algorithmic error" )
	    _CASE( pgmNotConverged, "not converged")
	    _CASE( pgmInvalidOperation, "invalid operation" )
	    _CASE( pgmBadArg, "bad arg" )
	    _CASE( pgmNULLPointer, "NULL pointer" )
	    _CASE( pgmOutOfRange, "out of range" )
	    _CASE( pgmInconsistentSize, "inconsistent size" )
	    _CASE( pgmInconsistentType, "inconsistent type" )
	    _CASE( pgmBadConst, "bad const" )
	    _CASE( pgmInternalError, "internal error" )
	    _CASE( pgmNotImplemented, "not implemented" )
    default:
	{
	    return "something wrong in exception";
	}
    }
}

void CException::GenMessage() throw()
{
    pnlString buf;

    buf << GetErrorType(pgmErrorType(m_code)) << " in " << m_file << " at line "
	<< m_line << ". The error description is: " << m_description ;
    m_message = buf;
}

CException::CException(pnlString file, int line, pnlString description)
throw(): m_code(pgmFail), m_file(file), m_line(line), m_description(description)
{
    GenMessage();
}

CException::CException(pnlString file, int line, pnlString description,
		       pgmErrorType code) throw()
		       : m_code(code), m_file(file), m_line(line),
		       m_description(description)
{
    GenMessage();
}

CNumericException::CNumericException(pnlString file, int line,
				     pnlString description) throw()
				     : CException( file, line, description, pgmNumericFail )
{
}

CNumericException::CNumericException(pnlString file, int line,
				     pnlString description, pgmErrorType code ) throw()
				     : CException( file, line, description, code)
{
}


COverflow::COverflow(pnlString file, int line,
		     pnlString description) throw():
CNumericException( file, line, description, pgmOverflow )
{
}


CUnderflow::CUnderflow(pnlString file, int line,
		       pnlString description) throw():
CNumericException( file, line, description, pgmUnderflow )
{
}

CAlgorithmicException::CAlgorithmicException(pnlString file, int line,
					     pnlString description) throw() :
CException( file, line, description, pgmAlgorithmic)
{
}

CAlgorithmicException::CAlgorithmicException(pnlString file, int line,
					     pnlString description,
					     pgmErrorType code) throw() :
CException( file, line, description, code)
{
}

CNotConverged::CNotConverged(pnlString file, int line,
			     pnlString description) throw():
CAlgorithmicException( file, line, description, pgmNotConverged )
{
}

CInvalidOperation::CInvalidOperation(pnlString file, int line,
				     pnlString description) throw():
CAlgorithmicException( file, line, description, pgmInvalidOperation )
{
}

CMemoryException::CMemoryException(pnlString file, int line,
				   pnlString description) throw():
CException( file, line, description, pnlMemoryFail)
{
}

CMemoryException::CMemoryException(pnlString file, int line,
				   pnlString description, pgmErrorType code)
				   throw():
CException( file, line, description, code)
{
}

CNotEnoughMemory::CNotEnoughMemory(pnlString file, int line,
				   pnlString description) throw():
CMemoryException( file, line, description, pgmNotEnoughMemory )
{
}

CDamagedMemory::CDamagedMemory(pnlString file, int line,
			       pnlString description) throw():
CMemoryException( file, line, description, pgmDamagedMemory )
{
}

CBadPointer::CBadPointer(pnlString file, int line,
			 pnlString description) throw():
CMemoryException( file, line, description, pgmBadPointer )
{
}

CBadArg::CBadArg(pnlString file, int line, pnlString description) throw():
CException( file, line, description, pgmBadArg)
{
}

CBadArg::CBadArg(pnlString file, int line,
		 pnlString description, pgmErrorType code) throw():
CException( file, line, description, code )
{
}


COutOfRange::COutOfRange(pnlString file, int line, pnlString description)
throw(): CBadArg( file, line, description, pgmOutOfRange )
{
}

CNULLPointer::CNULLPointer(pnlString file, int line,
			   pnlString description) throw():
CBadArg( file, line, description, pgmNULLPointer )
{
}

CInconsistentType::CInconsistentType(pnlString file, int line,
				     pnlString description) throw():
CBadArg( file, line, description, pgmInconsistentType )
{
}

CInconsistentSize::CInconsistentSize(pnlString file, int line,
				     pnlString description) throw():
CBadArg( file, line, description, pgmInconsistentSize )
{
}

CInconsistentState::
CInconsistentState(pnlString file, int line, pnlString description) throw()
: CAlgorithmicException( file, line, description, pgmAlgorithmic )
{
}


CBadConst::CBadConst(pnlString file, int line, pnlString description) throw()
: CBadArg( file, line, description, pgmBadConst )
{
}

CInternalError::CInternalError(pnlString file, int line, pnlString description)
throw(): CException( file, line, description, pgmInternalError )
{
}

CInternalError::CInternalError(pnlString file, int line, pnlString description,
			       pgmErrorType code  ) throw():
CException( file, line, description, code )
{
}

CNotImplemented::CNotImplemented(pnlString file, int line,
				 pnlString description) throw():
CInternalError( file, line, description, pgmNotImplemented )
{
}
