/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDBN.hpp                                                  //
//                                                                         //
//  Purpose:   Implementation of the Dynamic Bayesian Network              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLDBN_HPP__
#define __PNLDBN_HPP__

#include "pnlDynamicGraphicalModel.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlModelTypes.hpp"
#include "pnlFactors.hpp"
#include "pnlGraph.hpp"
#include "pnlNodeType.hpp"
#include "pnlException.hpp"

PNL_BEGIN

class PNL_API CDBN : public CDynamicGraphicalModel
{
public:
    static CDBN* Create( CStaticGraphicalModel *pGrModel);

    CStaticGraphicalModel* UnrollDynamicModel(int time) const;

    CGraph *CreatePriorSliceGraph() const;

    virtual CStaticGraphicalModel *CreatePriorSliceGrModel() const;
    void GenerateSamples( pEvidencesVecVector* evidencesOut, const intVector& nSlices ) const;

protected:
    /* constructor of the Dynamic Graphical Model object */
    CDBN( EModelTypes modelType, CStaticGraphicalModel *pGrModel);
};

PNL_END

#endif //__PNLDBN_HPP__
