/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:                                                                  //
//                                                                         //
//  Purpose:                                                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLRNG_HPP__
#define __PNLRNG_HPP__


PNL_BEGIN

//global functions for generating random numbers
//reinitialize generator with new seed
void PNL_API pnlSeed(int s);

#ifndef SWIG
//generate uniform integer distribution in the range [left,right]
int PNL_API pnlRand(int left, int right);
void PNL_API pnlRand(int numElem, int* vec, int left, int right);

//generate uniform distribution (single or double precision) on (left,right) range
float PNL_API pnlRand(float left, float right);
double PNL_API pnlRand(double left, double right);
void PNL_API pnlRand(int numElem, float* vec, float left, float right);
void PNL_API pnlRand(int numElem, double* vec, double left, double right);
void PNL_API pnlRand(int numElem, double* vecOut, double left, double right);

float PNL_API pnlRandNormal( float mean, float sigma );
double PNL_API pnlRandNormal( double mean, double sigma );
void PNL_API pnlRandNormal(doubleVector* vlsOut, doubleVector &mean, doubleVector &sigma );

void PNL_API pnlRandNormal( int numElem, float* vec, float mean, float sigma );
void PNL_API pnlRandNormal( int numElem, double* vec, double mean, double sigma );
#endif

double PNL_API pnlRand(double left, double right);
void PNL_API pnlRandNormal(floatVector* vlsOut, floatVector &mean, floatVector &sigma );

PNL_END

#endif // __PNLRNG_HPP__

