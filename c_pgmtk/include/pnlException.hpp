/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlException.hpp                                            //
//                                                                         //
//  Purpose:   CException class (and descendants) definitions              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// pgmException.hpp interface for class CException

#ifndef __PNLEXCEPTION_HPP__
#define __PNLEXCEPTION_HPP__

#include <exception>
#include <string>
#ifndef __PNLOBJECT_HPP__
#include "pnlObject.hpp"
#endif
#include "pnlError.h"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif

#ifndef __PNLSTRING_HPP__
#include "pnlString.hpp"
#endif

PNL_BEGIN

class PNL_API CException : public CPNLBase
{
public:
    CException(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CException() throw() {}

    pgmErrorType GetCode() const throw() {return m_code;}
    const char* GetMessage() const throw() {return m_message.c_str();}

    void GenMessage() throw();

protected:
    CException(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();

private:
    pgmErrorType m_code;
    pnlString m_message;
    pnlString m_file;
    int m_line;
    pnlString m_description;
};

class PNL_API CMemoryException : public CException
{
public:
    CMemoryException(pnlString file = "unknown",  int line = -1,
	pnlString description = "error") throw();
    virtual ~CMemoryException() throw() {}

protected:
    CMemoryException(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();
};

class PNL_API CNotEnoughMemory : public CMemoryException
{
public:
    CNotEnoughMemory(pnlString file = "unknown",  int line = -1,
	pnlString description = "error") throw();
    virtual ~CNotEnoughMemory() throw() {}
};

class PNL_API CDamagedMemory : public CMemoryException
{
public:
    CDamagedMemory(pnlString file = "unknown",  int line = -1,
	pnlString description = "error") throw();
    virtual ~CDamagedMemory() throw() {}
};

class PNL_API CBadPointer : public CMemoryException
{
public:
    CBadPointer(pnlString file = "unknown",  int line = -1,
	pnlString description = "error") throw();
    virtual ~CBadPointer() throw() {}
};

class PNL_API CBadArg : public CException
{
public:
    CBadArg(pnlString file = "unknown", int line = -1,
	pnlString description = "error")
	throw();
    virtual ~CBadArg() throw() {}

protected:
    CBadArg(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();
};

class PNL_API COutOfRange : public CBadArg
{
public:
    COutOfRange(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~COutOfRange() throw() {}
};

class PNL_API CNULLPointer : public CBadArg
{
public:
    CNULLPointer(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CNULLPointer() throw() {}
};

class PNL_API CInconsistentType : public CBadArg
{
public:
    CInconsistentType(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CInconsistentType() throw() {}
};

class PNL_API CInconsistentSize : public CBadArg
{
public:
    CInconsistentSize(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CInconsistentSize() throw() {}
};

class PNL_API CBadConst : public CBadArg
{
public:
    CBadConst(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CBadConst() throw() {};
};

class PNL_API CInternalError : public CException
{
public:
    CInternalError(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CInternalError() throw() {}

protected:
    CInternalError(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();
};

class PNL_API CNotImplemented : public CInternalError
{
public:
    CNotImplemented(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CNotImplemented() throw() {}
};

class PNL_API CNumericException : public CException
{
public:
    CNumericException(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CNumericException() throw() {};

protected:
    CNumericException(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();
};

class PNL_API COverflow : public CNumericException
{
public:
    COverflow(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~COverflow() throw() {};
};

class PNL_API CUnderflow : public CNumericException
{
public:
    CUnderflow(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CUnderflow() throw() {};
};

class PNL_API CAlgorithmicException : public CException
{
public:
    CAlgorithmicException(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CAlgorithmicException() throw() {};

protected:
    CAlgorithmicException(pnlString file, int line, pnlString description,
	pgmErrorType code ) throw();
};

class PNL_API CNotConverged : public CAlgorithmicException
{
public:
    CNotConverged(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CNotConverged() throw() {};
};

class PNL_API CInconsistentState : public CAlgorithmicException
{
public:
    CInconsistentState(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CInconsistentState() throw() {};
};

class PNL_API CInvalidOperation : public CAlgorithmicException
{
public:
    CInvalidOperation(pnlString file = "unknown", int line = -1,
	pnlString description = "error" ) throw();
    virtual ~CInvalidOperation() throw() {};
};

//macros for error handling
#define PNL_EXCEPTION(__type, __descr)         \
    __type( __FILE__, __LINE__, (__descr) );

#define PNL_THROW(__type, __descr)             \
    throw PNL_EXCEPTION( __type, __descr )

#define PNL_CHECK_RANGES(__var, __lr, __rr)		  \
    if( ( (__var) < (__lr) )  || ( (__var) > (__rr) ) ) { \
	PNL_THROW(pnl::COutOfRange, #__var " must be in a range between " #__lr " and " #__rr); \
    }

#define PNL_CHECK_LEFT_BORDER(__var, __lr)		  \
    if( (__var) < (__lr) ) {				  \
	PNL_THROW( pnl::COutOfRange, #__var " must be greater than or equal to " #__lr); }

#define PNL_CHECK_RIGHT_BORDER(__var, __rr)		  \
	if( (__var) > (__rr) ) {			  \
    PNL_THROW( pnl::COutOfRange, #__var " must be smaller than or equal to " #__rr ); }

#define PNL_CHECK_IS_NULL_POINTER(__var)                  \
	if( (__var) == NULL ) {                           \
    PNL_THROW( pnl::CNULLPointer, #__var " must not be a NULL pointer "); }

#define PNL_CHECK_FOR_ZERO(__var)                         \
	if( (__var) == 0 ) {                              \
    PNL_THROW( pnl::COutOfRange, #__var " must not be equal to zero "); }

#define PNL_CHECK_FOR_NON_ZERO(__var)                     \
    if( (__var) != 0 ) {                                  \
    PNL_THROW( pnl::COutOfRange, #__var " must be equal to zero "); }

#define PNL_CHECK_IF_MEMORY_ALLOCATED(__var)              \
    if( (__var) == NULL ) {                               \
    PNL_THROW( pnl::CNotEnoughMemory, " not enough memory to allocate " #__var  ); }

PNL_END

#endif // __PNLEXCEPTION_HPP__
