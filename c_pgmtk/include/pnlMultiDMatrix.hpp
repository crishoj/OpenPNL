/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMultiDMatrix.hpp                                         //
//                                                                         //
//  Purpose:   CMultiDMatrix class definition                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// MultiDMatrix.hpp: interface for the MultiDMatrix class.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMULTIDMATRIX_HPP__
#define __PNLMULTIDMATRIX_HPP__

#include "pnlReferenceCounter.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlException.hpp"
//using namespace std;

PNL_BEGIN

enum EMargType
{
    PNL_MARG_TYPE_SUM,
    PNL_MARG_TYPE_ADD = PNL_MARG_TYPE_SUM,
    PNL_MARG_TYPE_MAX
};

class PNL_API CMultiDMatrix : public CReferenceCounter
{
	
public:
	~CMultiDMatrix();
	
	static CMultiDMatrix* Create (int dim, const int *range, 
										const float *data, int Clamp = 0);
	static CMultiDMatrix* Copy( const CMultiDMatrix *pInputMat );
	//Returns new MultiDMatrix from the data 
	PNL_API friend CMultiDMatrix *pnlCombineMatrices(const CMultiDMatrix *mat1, 
						const CMultiDMatrix *mat2, int combineFlag = 1);
    void CombineInSelf( const CMultiDMatrix* matAdd, int combineFlag = 1 );
	//combine flag = 1 if we want to sum matrices,
	//0 - substract second matrix from first, 
	//-1  -choose max values from matrices
	CMultiDMatrix & operator = ( const CMultiDMatrix& pInputMatrix );
	void GetRawData(int *dataLengthOut, const float **dataOut) const;  
	//Returns the reference on the array
	const floatVector *GetVector() const;
	//Returns the reference to vector
	int GetNumberDims() const;
	void GetRanges(int *numOfDimsOut, const int **rangesOut) const;
	int SetClamp( int Clamp);
	int GetClampValue() const;
	//to prevent changing Matrix (it can be shared by several factors)
	//return value of m_bClamped after the operation 
	//(in case Clamp<0 m_bClamped doesn't change)
	int SetData(const float *NewData);
	//to set new data to the matrix (we can do it if matrix isn't clamped)
	//this method returns 1 if we change the data, 0 otherwise 
	//(if matrix is clamped or Newdata = NULL)
	inline float GetElementByIndexes(const int *multidimindexes) const;
	inline float GetElementByOffset(int linearindex) const;
	//these functions return an element of matrix for
	//given multidimensional indexes or linear index (offset)
	inline void SetElementByOffset(float value, int offset);
	inline void SetElementByIndexes(float value, const int *multidimindexes);
	//these functions set matrix element equal value (for
	//given multidimensional indexes or linear index)
	CMultiDMatrix *Marg( int const *pDimsToKeep, int numDimsToKeep, EMargType ) const;
	CMultiDMatrix *FixDims( int const *pDimsToFix, int numDimsToFix,
                                int const *pObservedValues ) const;
	CMultiDMatrix *ReduceOp( int const *pDimsOfInterest, int numDimsOfInterest,
                                 int action, int const *pObservedValues = NULL) const;
	CMultiDMatrix *OldReduceOp( int const *pDimsOfInterest, int numDimsOfInterest,
                                    int action, int const *pObservedValues = NULL) const;
	/*this method is used for Factor::Marginalize (1) and 
	Factor::ShrinkObservedNodes(2). It reduses matrix dimensions dependently of
	action = 0 - Sum of other(exept DimOfKeep) dimensions
	action = 1 - choosing Maximum of other dimensions
	action = 2 - choosing given value of node(dimension), values of nodes (DimOfKeep) 
	are contained in pObsValues*/
	CMultiDMatrix *ExpandDims(const int *dimsToExtend, 
								const int *keepPosOfDims, 
								const int *sizesOfExpandDims,
								int numDimsToExpand) const;
	/*this method extend dimensions dimsToExpand(their sizes should be 1 
	before expand) to size sizesOfExpandDims by adding zeros
	at the matrix points in this dimsToExpand except valuesOfDims - 
	for these matrix points values will be set from original matrix*/
	float SumAll(int byAbsValue = 0) const;
	inline void ClearData();
	/*returns sum of all data cells in the matrix(for CPD it should return 1)*/
	CMultiDMatrix * NormalizeAll() const;

	
protected:
	CMultiDMatrix();
	CMultiDMatrix(int dim, const int *range, const float *data, int Clamp);
	CMultiDMatrix( const CMultiDMatrix & inputMat );
	int m_bClamped;
	//If we can't change this Matrix during learning m_bClamped = 1
	int m_Dim;  //Contain number of dimentions of matrix 
	// (number of nodes in cliques - for factor or number of nodes in family - 
	// for CPD)
	int *m_Range; 
	//array of node ranges (nodes which are discribed by this Matrix) 
	pnlVector<float> m_Table; 
private:
};

#ifndef SWIG

inline void CMultiDMatrix :: ClearData()
{
	for( int i = 0; i < m_Table.size(); i++ )
	{
		m_Table[i] = 0.0f;
	}
}
inline float CMultiDMatrix :: GetElementByOffset(int offset) const
{
	return m_Table[offset];
}

inline void CMultiDMatrix :: SetElementByOffset(float value, int offset)
{
	if(!m_bClamped)
	{
		m_Table[offset] = value;
	}
	else
	{
		PNL_THROW( CInvalidOperation, "can't change clamped matrix" )
	}
}
inline float 
CMultiDMatrix :: GetElementByIndexes( const int *multidimindexes) const
{
	if( !m_bClamped )
	{
		int offset = 0;
	    for( int i = 0; i < m_Dim; i++)
	    {
    		offset = offset * m_Range[i] + multidimindexes[i];
    	}
        float result = GetElementByOffset(offset);
    	return result;
    }
	else
    {
    	PNL_THROW( CInvalidOperation, "can't change clamped matrix" )
	}
}
	
inline void
CMultiDMatrix :: SetElementByIndexes(float value, const int *multidimindexes)
{
	int offset = 0;
	for( int i = 0; i < m_Dim; i++)
	{
		offset = offset * m_Range[i] + multidimindexes[i];
	}
	SetElementByOffset(value, offset);
}

#endif

PNL_END

#endif //__PNLMULTIDMATRIX_HPP__
