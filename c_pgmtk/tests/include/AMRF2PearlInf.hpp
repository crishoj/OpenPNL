/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMRF2PearlInf.hpp                                           //
//                                                                         //
//  Purpose:   Test on Pearl Inference Engine correctness for MRF2         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __AMRF2PEARLINF_HPP__
#define __AMRF2PEARLINF_HPP__

// the greater the constants the slower the test
#define MAX_NUM_OF_ROWS          16000
#define MAX_NUM_OF_COLS          16000
#define MAX_NUM_OF_NODE_VALS     100
#define MAX_NUM_OF_NODE_VAL_DIMS 100
#define MAX_NUM_OF_ITERS         10000


#define EQUAL_VAL_ELEM           10.0f

#define NODE_VAL_ELEM_MIN        1.0f
#define NODE_VAL_ELEM_MAX        100.f

#define SIGMA_SCENE              15.0f
#define SIGMA_IMAGE              16.0f

#include <functional>

namespace super_helper
{
    struct GenFloat: public std::unary_function<float, void>
    {
        float operator()(void)
        {
            return pnl::pnlRand( NODE_VAL_ELEM_MIN, NODE_VAL_ELEM_MAX );
        }
    };
    //////////////////////////////////////////////////////////////////////////

    inline static void GetEqualNodeVal( pnl::floatVector* pVecOut )
    {
        std::fill( pVecOut->begin(), pVecOut->end(), EQUAL_VAL_ELEM );
    }
    //////////////////////////////////////////////////////////////////////////
    
    inline static void GetRandomNodeVal( pnl::floatVector* pVecOut )
    {
        std::generate( pVecOut->begin(), pVecOut->end(), GenFloat() );
    }
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct Print : std::unary_function< void, const T& >
    {
        Print( const std::string& delim = ", " ): m_delim(delim) {}
        
        void operator()(const T& t)
        {
            std::cout << t << m_delim.c_str();
        }

    private:
        std::string m_delim;
    };
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct PrintVector : std::unary_function<void, const pnl::pnlVector<T>& >
    {
        PrintVector(bool bEndl = true) : m_bEndl(bEndl) { }
        
        void operator()(const pnl::pnlVector<T>& vec)
        {
            std::for_each( vec.begin(), vec.end(), Print<T>() );

            if( m_bEndl ){ std::cout << std::endl; }
        }

    private:
        bool m_bEndl;
    };
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct NotEqual: public std::unary_function<bool, const T& >
    {
        NotEqual(T toCompWith) : m_toCompWith(toCompWith) {}
        
        bool operator()(const T& t)
        {
            return m_toCompWith != t;
        }
        
    private:
        T m_toCompWith;
    };
    //////////////////////////////////////////////////////////////////////////
    
};
//////////////////////////////////////////////////////////////////////////

pnl::CTabularPotential* SuperResolutionTabPot( const pnl::intVector& clq,
                                               int clqInd,
                                               const pnl::pnlVector < pnl::
                                               floatVecVector >& nodeVals,
                                               pnl::CModelDomain* pModelDomain );
//////////////////////////////////////////////////////////////////////////

pnl::CMRF2* SuperResolution2lMRF2( int numOfRows, int numOfCols,
                                   pnl::CModelDomain* pModelDomain,
                                   const pnl::pnlVector< 
                                   pnl::floatVecVector >& nodeVals );
//////////////////////////////////////////////////////////////////////////

#endif // __AMRF2PEARLINF_HPP__