/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticLearningEngine.cpp                                 //
//                                                                         //
//  Purpose:   Implementation of the base class for all learning           //
//             algorithms for static graphical models                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlStaticLearningEngine.hpp"
#include "pnlDistribFun.hpp"

PNL_USING

CStaticLearningEngine::CStaticLearningEngine(CStaticGraphicalModel *pGrModel,
									   ELearningTypes learnType): 
CLearningEngine( pGrModel, learnType ),
m_maxIterIPF(10),
m_numberOfAllEvidences(0),
m_precisionIPF(0.001f)

{

}

CStaticLearningEngine::~CStaticLearningEngine()
{
	m_Vector_pEvidences.clear();
}	

void CStaticLearningEngine::SetMaxIterIPF(int max_iter)
{
	if( max_iter <= 0)
	{
		PNL_THROW(COverflow, "maximum number of iterations for IPF procedure")
	}
	m_maxIterIPF = max_iter;
}
void  CStaticLearningEngine::SetTerminationToleranceIPF(float precision)
{
	if( precision <= FLT_MIN )
	{
		PNL_THROW(CBadConst, "precision be positively" )
	}
	m_precisionIPF = precision;
	
}


void CStaticLearningEngine::AppendData(int dim, const CEvidence* const* pEvidences)
{
    
    //create vector consists of points to evidences
    
    int i = 0;
    for( i = 0; i < dim; i++ )
    {
	if (!pEvidences[i])
	{
	    PNL_THROW(CNULLPointer,"evidence")
	}
	m_Vector_pEvidences.push_back( pEvidences[i] );
    }
    m_numberOfAllEvidences = m_Vector_pEvidences.size();
    ClearStatisticData();
    
}

void CStaticLearningEngine::AppendData(const pConstEvidenceVector& evidencesIn)
{
    AppendData( evidencesIn.size(), &evidencesIn.front() );
}



void CStaticLearningEngine::SetData(int dim, const CEvidence * const* pEvidences)
{
    m_critValue.clear();
    m_Vector_pEvidences.clear();
    ClearStatisticData();
    AppendData(dim, pEvidences);
    m_numberOfAllEvidences=m_Vector_pEvidences.size();
    
}

void CStaticLearningEngine::SetData(const pConstEvidenceVector& evidencesIn )
{
    SetData( evidencesIn.size(), &evidencesIn.front() );
}
// ----------------------------------------------------------------------------

void CStaticLearningEngine::BuildFullEvidenceMatrix(float ***full_evid)
{
    int i, j;
    CStaticGraphicalModel *grmodel = GetStaticModel();
    const CEvidence* pCurrentEvid;
    //valueVector *values = new valueVector();
  
    int NumOfNodes = grmodel->GetGraph()->GetNumberOfNodes();
    int NumOfEvid = m_Vector_pEvidences.size();

    (*full_evid) = new float * [NumOfNodes];
    for (i = 0; i < NumOfNodes; i++)
    {
        (*full_evid)[i] = new float [NumOfEvid];
    }

    for (i = 0; i < NumOfNodes; i++)
    {
        for (j = 0; j < NumOfEvid; j++)
        {
            (*full_evid)[i][j] = -10000;
        }
    }

    const int * obs;
    for (j = 0; j < NumOfEvid; j++)
    {
        pCurrentEvid = m_Vector_pEvidences[j];
        //int ObsNum = pCurrentEvid->GetNumberObsNodes();
        obs = pCurrentEvid->GetAllObsNodes(); 
        for (i = 0; i < (pCurrentEvid->GetNumberObsNodes()); i++)
        {
            float fl = (pCurrentEvid->GetValue(obs[i]))->GetFlt();
            (*full_evid)[int(obs[i])][j] = fl;
        }
    }

/*  printf ("\n My Full Evidence Matrix");
    for (i=0; i<NumOfNodes; i++)
    {
        for (j=0; j<NumOfEvid; j++)
        {
            printf ("%f   ", (*full_evid)[i][j]);
        }
        printf("\n");
    }
*/            
}
// ----------------------------------------------------------------------------


#ifdef PNL_RTTI
const CPNLType CStaticLearningEngine::m_TypeInfo = CPNLType("CStaticLearningEngine", &(CLearningEngine::m_TypeInfo));

#endif
