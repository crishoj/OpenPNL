/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlExampleModels.hpp                                        //
//                                                                         //
//  Purpose:   Definition of methods creating popular Graphical Models     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLEXAMPLEMODELS_HPP__
#define __PNLEXAMPLEMODELS_HPP__
#include "pnlBNet.hpp"
#include "pnlMRF2.hpp"
#include "pnlDBN.hpp"

PNL_BEGIN

PNL_API CBNet* pnlExCreateWaterSprinklerBNet();

PNL_API CBNet* pnlExCreateAsiaBNet();

PNL_API CBNet* pnlExCreateKjaerulfsBNet();

PNL_API CBNet* pnlExCreateWasteBNet();

#ifndef SWIG
PNL_API CBNet* pnlExCreateWaterSprinklerSparseBNet();
#endif

PNL_API CBNet* pnlExCreateSimpleGauMix();

PNL_API CBNet* pnlExCreateVerySimpleGauMix();

PNL_API CBNet* pnlExCreateCondGaussArBNet();
PNL_API CBNet* pnlExCreateBNetFormSMILE();
PNL_API CBNet* pnlExCreateSingleGauMix();
PNL_API CBNet* pnlExCreateBatNetwork( int seed = 10 );
PNL_API CBNet *pnlExCreateScalarGaussianBNet();

PNL_API CBNet * pnlExCreateRndArHMM();

PNL_API CMRF2* pnlExCreateBigMRF2(int variant, int m_numOfRows,
                                  int m_numOfCols, int m_Kolkand,
                                  float sigmahor, float sigmaver);

PNL_API CBNet* pnlExCreateRandomBNet( int nnodes, int nEdges, int nContNds = 0, 
				      int maxSzDiscrNds = 2,
				      int maxSzContNds = 1,
                                      float edge_probablility = 1.f,
                                      bool disconnected_ok = false );

PNL_API CDBN* pnlExCreateRandomDBN(int nnodesPerSlice, int nEdges, int nInterfNds, 
				   int nContNdsPerSlice = 0, 
				   int maxSzDiscrNds = 2,
				   int maxSzContNds = 1);


PNL_END

#endif
