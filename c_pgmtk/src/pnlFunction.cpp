/**
 * @file pnlFunction.cpp
 * @brief Implementation of the CFunction class.
 * 
 * This source file implements the CFunction class.
 * 
 * See also: pnlFunction.hpp
 * 
 * TODO: Just a dummy implementation
 */

#include "pnlFunction.hpp"

PNL_USING

/*
 * PUBLIC
 */

CFunction& CFunction::operator=( const CFunction&)
{
    return *this;
}

CFunction* CFunction::Clone() const
{
    return new CFunction();
}

CFunction::~CFunction()
{
    return;
}

/*
 * PROTECTED
 */

/*
 * CONSTRUCTORS
 */

CFunction::CFunction()
{
    m_value = 0;
}