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
};

void CException::GenMessage() throw()
{
    std::stringstream buf;

    buf << GetErrorType(pgmErrorType(m_code)) << " in " << m_file << " at line "
	<< m_line << ". The error description is: " << m_description ;
    m_message = buf.str();
}

CException::CException( std::string file, int line, std::string description)
throw(): m_code(pgmFail), m_file(file), m_line(line), m_description(description)
{
    GenMessage();
}

CException::CException( std::string file, int line, std::string description,
		       pgmErrorType code) throw()
		       : m_code(code), m_file(file), m_line(line),
		       m_description(description)
{
    GenMessage();
}

CNumericException::CNumericException( std::string file, int line,
				     std::string description  ) throw()
				     : CException( file, line, description, pgmNumericFail )
{
}

CNumericException::CNumericException( std::string file, int line,
				     std::string description, pgmErrorType code ) throw()
				     : CException( file, line, description, code)
{
}


COverflow::COverflow( std::string file, int line,
		     std::string description  ) throw():
CNumericException( file, line, description, pgmOverflow )
{
}


CUnderflow::CUnderflow( std::string file, int line,
		       std::string description  ) throw():
CNumericException( file, line, description, pgmUnderflow )
{
}

CAlgorithmicException::CAlgorithmicException(std::string file, int line,
					     std::string description) throw() :
CException( file, line, description, pgmAlgorithmic)
{
}

CAlgorithmicException::CAlgorithmicException(std::string file, int line,
					     std::string description,
					     pgmErrorType code) throw() :
CException( file, line, description, code)
{
}

CNotConverged::CNotConverged( std::string file, int line,
			     std::string description  ) throw():
CAlgorithmicException( file, line, description, pgmNotConverged )
{
}

CInvalidOperation::CInvalidOperation( std::string file, int line,
				     std::string description  ) throw():
CAlgorithmicException( file, line, description, pgmInvalidOperation )
{
}

CMemoryException::CMemoryException( std::string file, int line,
				   std::string description) throw():
CException( file, line, description, pnlMemoryFail)
{
}

CMemoryException::CMemoryException( std::string file, int line,
				   std::string description, pgmErrorType code)
				   throw():
CException( file, line, description, code)
{
}

CNotEnoughMemory::CNotEnoughMemory( std::string file, int line,
				   std::string description  ) throw():
CMemoryException( file, line, description, pgmNotEnoughMemory )
{
}

CDamagedMemory::CDamagedMemory(std::string file, int line,
			       std::string description) throw():
CMemoryException( file, line, description, pgmDamagedMemory )
{
}

CBadPointer::CBadPointer( std::string file, int line,
			 std::string description  ) throw():
CMemoryException( file, line, description, pgmBadPointer )
{
}

CBadArg::CBadArg( std::string file, int line,
		 std::string description) throw():
CException( file, line, description, pgmBadArg)
{
}

CBadArg::CBadArg( std::string file, int line,
		 std::string description, pgmErrorType code) throw():
CException( file, line, description, code )
{
}


COutOfRange::COutOfRange(std::string file, int line, std::string description)
throw(): CBadArg( file, line, description, pgmOutOfRange )
{
}

CNULLPointer::CNULLPointer( std::string file, int line,
			   std::string description  ) throw():
CBadArg( file, line, description, pgmNULLPointer )
{
}

CInconsistentType::CInconsistentType( std::string file, int line,
				     std::string description  ) throw():
CBadArg( file, line, description, pgmInconsistentType )
{
}

CInconsistentSize::CInconsistentSize( std::string file, int line,
				     std::string description  ) throw():
CBadArg( file, line, description, pgmInconsistentSize )
{
}

CInconsistentState::
CInconsistentState(std::string file, int line, std::string description) throw()
: CAlgorithmicException( file, line, description, pgmAlgorithmic )
{
}


CBadConst::CBadConst(std::string file, int line, std::string description) throw()
: CBadArg( file, line, description, pgmBadConst )
{
}

CInternalError::CInternalError(std::string file, int line, std::string description)
throw(): CException( file, line, description, pgmInternalError )
{
}

CInternalError::CInternalError(std::string file, int line, std::string description,
			       pgmErrorType code  ) throw():
CException( file, line, description, code )
{
}

CNotImplemented::CNotImplemented( std::string file, int line,
				 std::string description  ) throw():
CInternalError( file, line, description, pgmNotImplemented )
{
}
