/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTypeDefs.hpp                                             //
//                                                                         //
//  Purpose:   PNL's types definitions (vectors, lists)                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLTYPEDEFS_H__
#define __PNLTYPEDEFS_H__

PNL_BEGIN

using namespace std;

enum EMargType
{
    PNL_MARG_TYPE_SUM,
    PNL_MARG_TYPE_ADD = PNL_MARG_TYPE_SUM,
    PNL_MARG_TYPE_MAX
};

enum EAccumType
{
    PNL_ACCUM_TYPE_STORE,
    PNL_ACCUM_TYPE_ACCUMULATE = -1,
    PNL_ACCUM_TYPE_SUM = PNL_MARG_TYPE_SUM,
    PNL_ACCUM_TYPE_ADD = PNL_ACCUM_TYPE_SUM,
    PNL_ACCUM_TYPE_MAX = PNL_MARG_TYPE_MAX,
    PNL_ACCUM_TYPE_MUL = 4
};

typedef enum
{
  mmGradient,    // gradient method
  mmHessian,     // method with Hessian usage
  mmConjGradient // conjugate gradient method
} EMaximizingMethod;

typedef struct _TreeNodeFields
{ 
    bool isTerminal;     // is this tree node terminal
    int Question;        // question type 
    int node_index;      // node of Baysam net that asks   
    float questionValue; // question value
    float *probVect;     // probalistic vector if this tetminal node is discrete
    float expectation;   // terminal node expectation if this node is continuous
    float variance;      // terminal node variance if this node is continuous
} TreeNodeFields;

template<class T> class GeneralAllocator: public std::allocator<T>
{
public:

#if GCC_VERSION >= 30300
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::size_type size_type;
#endif

    GeneralAllocator() {}
#ifndef PNL_VC6
    template <class TOther>
        GeneralAllocator(const std::allocator<TOther> &al) {}
#endif
    GeneralAllocator& operator=(const std::allocator<T> &al)
    {
        return *this;
    }
    bool operator==(const std::allocator<T> &al)
    {
        return false;
    }
    bool operator!=(const std::allocator<T> &al)
    {
        return true;
    }
    virtual ~GeneralAllocator() {}
    virtual pointer allocate( size_type n, const void *hint = 0 )
    {   return allocator<T>::allocate(n, hint); }
    virtual void deallocate(pointer p, size_type n)
    {   allocator<T>::deallocate(p, n); }
    virtual void construct(pointer p, const T& v)
    {   allocator<T>::construct(p, v); }
    virtual void destroy(pointer p)
    {   allocator<T>::destroy(p); }
#ifndef PNL_VC6
    template<class TOther>
        struct rebind
        {   // convert an GeneralAllocator<T> to an GeneralAllocator<TOther>
            typedef GeneralAllocator<TOther> other;
        };
#endif
};

#ifndef SWIG
class PNL_API Value
{
public:
    int   GetInt() const { return m_tVal; }
    float GetFlt() const { return m_cVal; }
    void SetInt(int v)     { m_tVal = v; m_cVal = float(v); m_bInt = true; }
    void SetFlt(float v)   { m_tVal = int(v);   m_cVal = v; m_bInt = false; }
    void SetUnknownAsFlt(float v) { m_tVal = int(v); m_cVal = v; m_bInt = 2; }
    Value( int v ):   m_bInt(true),  m_tVal(v), m_cVal(float(v)) {}
    Value( float v ): m_bInt(false), m_tVal(int(v)), m_cVal(v) {}
#ifdef PNL_VC7
    bool operator<(const Value &nt) const
    {
  return m_cVal < nt.m_cVal;
    }
    bool operator==(const Value &nt) const
    {
  return m_cVal == nt.m_cVal;
    }
#endif

    Value() {}
    char IsDiscrete() const { return m_bInt; }

private:
    char m_bInt;
    float m_cVal;
    int m_tVal;
};
#endif

#pragma warning(push, 3)
template<class Type, class Allocator = GeneralAllocator<Type> >
class pnlVector: public std::vector<Type, Allocator>
{
public:
    typedef pnlVector<Type,Allocator> MyType;
    typedef typename std::vector<Type, Allocator> ParentType;

#if 1
    typedef typename ParentType::iterator iterator;
#else
    typedef unsigned char * iterator;
#endif
    typedef typename ParentType::const_iterator const_iterator;
    typedef typename ParentType::allocator_type allocator_type;
    typedef typename ParentType::size_type size_type;

    pnlVector() {}
    explicit pnlVector(const Allocator& Al)
        : ParentType(Al) {}

    explicit pnlVector( size_type N, const Type& V = Type(),
                        const Allocator& Al = Allocator() )
        : std::vector<Type, Allocator>(N, V, Al) {}

    pnlVector(const MyType& X)
        : std::vector<Type, Allocator>(X) {}

#ifndef PNL_VC6
    template< typename InputIterator >
    pnlVector( InputIterator first, InputIterator last, const Allocator &ator = Allocator() )
        : std::vector< Type, Allocator >( first, last, ator )
    {}
#else
    pnlVector( const_iterator F, const_iterator L, const allocator_type &Al = Allocator() )
        : std::vector<Type, Allocator>( F, L, Al )
    {}
#endif
    bool lexicographical_compare(iterator First1, iterator Last1,
	iterator First2, iterator Last2)
    {
	return lexicographical_compare(
	    (const unsigned char *)First1, (const unsigned char *)Last1,
	    (const unsigned char *)First2, (const unsigned char *)Last2);
    }

    MyType &operator=( const MyType &X )
    {
        ParentType::operator=( static_cast< const ParentType >( X ) );
        return *this;
    }
};
#pragma warning (pop)

typedef std::queue<int>                 intQueue;

typedef pnlVector<Value>                valueVector;
typedef pnlVector<Value*>               pValueVector;
typedef pnlVector<valueVector>          valueVecVector;
typedef pnlVector<const Value*>         pConstValueVector;

typedef pnlVector<int>                  intVector;
typedef pnlVector<int *>                intPVector;
typedef pnlVector<const int*>           pConstIntVector;


typedef pnlVector< intVector>           intVecVector;

typedef pnlVector< intVector* >         intVecPVector;

typedef pnlVector<float>                floatVector;

typedef pnlVector<double>               doubleVector;

typedef pnlVector< floatVector>         floatVecVector;

typedef pnlVector<bool>                 boolVector;
typedef pnlVector< boolVector>          boolVecVector;


class CNodeType;

typedef pnlVector< CNodeType>           nodeTypeVector;

typedef pnlVector< CNodeType*>          pNodeTypeVector;

typedef pnlVector< const CNodeType*>    pConstNodeTypeVector;


class CEvidence;

typedef pnlVector< CEvidence*>          pEvidencesVector;

typedef pnlVector< const CEvidence*>    pConstEvidenceVector;

typedef pnlVector<pConstEvidenceVector> pConstEvidencesVecVector;

typedef pnlVector< pEvidencesVector>    pEvidencesVecVector;


class CFactor;

typedef pnlVector<CFactor*>             pFactorVector;

typedef pnlVector<const CFactor*>       pConstFactorVector;


class CPotential;
typedef pnlVector< const CPotential*>   pConstPotVector;
typedef pnlVector< CPotential*>         potsPVector;

typedef pnlVector< potsPVector>         potsPVecVector;

class CCPD;
typedef pnlVector<const CCPD *>         pConstCPDVector;
typedef pnlVector<CCPD*>				pCPDVector;

class CGaussianCPD;
typedef pnlVector<CGaussianCPD *>       pGaussianCPDVector;

class CDistribFun;
typedef pnlVector< CDistribFun*>        distrPVector;

template<class T> class C2DNumericDenseMatrix;
typedef pnlVector<const C2DNumericDenseMatrix<float>*> p2DDenseMatrixVector;

class CIDPotential;

typedef pnlVector< CIDPotential* >      pIDPotentialVector;

typedef pnlVector<pFactorVector>        pFactorVecVector;

PNL_END

#endif // __PNLTYPEDEFS_H__
