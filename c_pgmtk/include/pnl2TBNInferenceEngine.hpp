/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl2TBNInferenceEngine.hpp                                  //
//                                                                         //
//  Purpose:   Base class definition for the inference algorithms that     //
//             use forward-backward operations                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNL2TBNINFERENCEENGINE_HPP__
#define __PNL2TBNINFERENCEENGINE_HPP__

#include "pnlDynamicInferenceEngine.hpp"

PNL_BEGIN

class PNL_API C2TBNInfEngine : public CDynamicInfEngine
{
public:
	virtual ~C2TBNInfEngine();
	virtual void ForwardFirst(const CEvidence *pEvidence, int maximize = 0)=0;
	virtual void Forward(const CEvidence *pEvidence, int maximize = 0 )=0;
	virtual void BackwardT()=0;
	virtual void Backward( int maximize = 0 )=0;
	virtual void BackwardFixLag()=0;
	virtual void DefineProcedure(EProcedureTypes infType, int lag = 0)=0;
#ifndef SWIG
	
	virtual void MarginalNodes( const int *queryIn, int querySize, int time=0,int notExpandJPD = 0  )=0;
#endif
	const CPotential* GetQueryJPD()=0;
	const CEvidence* GetMPE() = 0;
	void Filtering(int time);
	void Prediction(int time);
	void Smoothing();
	void FixLagSmoothing(int time);
    virtual void FindMPE();

protected:
	C2TBNInfEngine( const CDynamicGraphicalModel *pGraphicalModel );
};

PNL_END

#endif //__PNL2TBNINFERENCEENGINE_HPP__
