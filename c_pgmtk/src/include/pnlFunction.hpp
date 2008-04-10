/**
 * @file pnlFunction.hpp
 * @brief Declaration of the CFunction class.
 * 
 * This source file declares the CFunction class.
 * 
 * See also: pnlFunction.cpp
 * 
 * TODO: Just a dummy declaration
 */

#ifndef __PNLFUNCTION_HPP__
#define __PNLFUNCTION_HPP__

#include "pnlConfig.hpp"

PNL_BEGIN

class PNL_API CFunction : public CPNLBase
{   

public:
    
    virtual CFunction& operator=( const CFunction& function);
    virtual CFunction* Clone() const;    
    
    ~CFunction();

protected:
    
    /*
     * CONSTRUCTORS
     */
    
    CFunction();
    
private:
    
    int m_value;

};

PNL_END

#endif //__PNLFUNCTION_HPP__