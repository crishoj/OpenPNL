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
#include "pnlObject.hpp"
#include "pnlError.h"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif

PNL_BEGIN

class PNL_API CException : public CPNLBase
{
public:

    CException( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CException() throw() {}

    pgmErrorType GetCode() const throw() {return m_code;}
    const char* GetMessage() const throw() {return m_message.c_str();}

    void GenMessage() throw();

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CException::m_TypeInfo;
    }
#endif
protected:
    CException( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
private:

    pgmErrorType m_code;
	std::string m_message;
    std::string m_file;
	int m_line;
	std::string m_description;

};

class PNL_API CMemoryException : public CException
{
public:
    CMemoryException( std::string file = "unknown",  int line = -1,
	std::string description = "error") throw();
    virtual ~CMemoryException() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CMemoryException::m_TypeInfo;
    }
#endif
protected:
    CMemoryException( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CNotEnoughMemory : public CMemoryException
{
public:
    CNotEnoughMemory( std::string file = "unknown",  int line = -1,
	std::string description = "error") throw();
    virtual ~CNotEnoughMemory() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNotEnoughMemory::m_TypeInfo;
    }
#endif
protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CDamagedMemory : public CMemoryException
{
public:
    CDamagedMemory( std::string file = "unknown",  int line = -1,
	std::string description = "error") throw();
    virtual ~CDamagedMemory() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDamagedMemory::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CBadPointer : public CMemoryException
{
public:
    CBadPointer( std::string file = "unknown",  int line = -1,
	std::string description = "error") throw();
    virtual ~CBadPointer() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CBadPointer::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CBadArg : public CException
{
public:
    CBadArg( std::string file = "unknown", int line = -1,
	std::string description = "error")
	throw();
    virtual ~CBadArg() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CBadArg::m_TypeInfo;
    }
#endif
protected:
    CBadArg( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API COutOfRange : public CBadArg
{
public:
    COutOfRange( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~COutOfRange() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return COutOfRange::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CNULLPointer : public CBadArg
{
public:
    CNULLPointer( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CNULLPointer() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNULLPointer::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CInconsistentType : public CBadArg
{
public:
    CInconsistentType( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CInconsistentType() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CInconsistentType::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CInconsistentSize : public CBadArg
{
public:
    CInconsistentSize( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CInconsistentSize() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CInconsistentSize::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CBadConst : public CBadArg
{
public:
    CBadConst( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CBadConst() throw() {};


#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CBadConst::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CInternalError : public CException
{
public:
    CInternalError( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CInternalError() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CInternalError::m_TypeInfo;
    }
#endif
protected:
    CInternalError( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CNotImplemented : public CInternalError
{
public:
    CNotImplemented( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CNotImplemented() throw() {}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNotImplemented::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CNumericException : public CException
{
public:
    CNumericException( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CNumericException() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNumericException::m_TypeInfo;
    }
#endif
protected:
    CNumericException( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API COverflow : public CNumericException
{
public:
    COverflow( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~COverflow() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return COverflow::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CUnderflow : public CNumericException
{
public:
    CUnderflow( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CUnderflow() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CUnderflow::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CAlgorithmicException : public CException
{
public:
    CAlgorithmicException( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CAlgorithmicException() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CAlgorithmicException::m_TypeInfo;
    }
#endif
protected:
    CAlgorithmicException( std::string file, int line, std::string description,
	pgmErrorType code ) throw();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CNotConverged : public CAlgorithmicException
{
public:
    CNotConverged( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CNotConverged() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNotConverged::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CInconsistentState : public CAlgorithmicException
{
public:
    CInconsistentState( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CInconsistentState() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CInconsistentState::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

class PNL_API CInvalidOperation : public CAlgorithmicException
{
public:
    CInvalidOperation( std::string file = "unknown", int line = -1,
	std::string description = "error" ) throw();
    virtual ~CInvalidOperation() throw() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }  
    static const CPNLType &GetStaticTypeInfo()
    {
      return CInvalidOperation::m_TypeInfo;
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif  
};

//macros for error handling
#define PNL_EXCEPTION(__type, __descr)         \
    __type( __FILE__, __LINE__, (__descr) );

#define PNL_THROW(__type, __descr)             \
    throw PNL_EXCEPTION( __type, __descr )

#define PNL_CHECK_RANGES(__var, __lr, __rr)		  \
    if( ( (__var) < (__lr) )  || ( (__var) > (__rr) ) ) { \
	std::string __descr( (#__var) );		  \
	__descr += " should be in a range between ";	  \
	__descr += (#__lr);				  \
	__descr += " and ";				  \
	__descr += (#__rr);				  \
	PNL_THROW( pnl::COutOfRange, __descr ); }

#define PNL_CHECK_LEFT_BORDER(__var, __lr)		  \
    if( (__var) < (__lr) ) {				  \
	std::string __descr( (#__var) );		  \
	__descr += " should be greater than or equal to ";\
	__descr += (#__lr);				  \
	PNL_THROW( pnl::COutOfRange, __descr ); }

#define PNL_CHECK_RIGHT_BORDER(__var, __rr)		  \
	if( (__var) > (__rr) ) {			  \
    std::string __descr( (#__var) );			  \
    __descr += " should be smaller than or equal to ";    \
	__descr += (#__rr);                               \
    PNL_THROW( pnl::COutOfRange, __descr ); }

#define PNL_CHECK_IS_NULL_POINTER(__var)                  \
	if( (__var) == NULL ) {                           \
    std::string __descr( (#__var) );                      \
	__descr += " should not be a NULL pointer ";      \
    PNL_THROW( pnl::CNULLPointer, __descr ); }

#define PNL_CHECK_FOR_ZERO(__var)                         \
	if( (__var) == 0 ) {                              \
    std::string __descr( (#__var) );                      \
	__descr += " should not be equal to zero ";       \
    PNL_THROW( pnl::COutOfRange, __descr ); }

#define PNL_CHECK_FOR_NON_ZERO(__var)                     \
    if( (__var) != 0 ) {                                  \
    std::string __descr( (#__var) );                      \
	__descr += " should be equal to zero ";           \
    PNL_THROW( pnl::COutOfRange, __descr ); }

#define PNL_CHECK_IF_MEMORY_ALLOCATED(__var)              \
    if( (__var) == NULL ) {                               \
    std::string __descr(" not enough memory to allocate ");\
	__descr += (#__var);				  \
    PNL_THROW( pnl::CNotEnoughMemory, __descr ); }

PNL_END

#endif // __PNLEXCEPTION_HPP__
