/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistenceGraph.cpp                                     //
//                                                                         //
//  Purpose:   Saving/Loading Graph                                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <ctype.h>
#include <sstream>
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlPersistGraph.hpp"
#include "pnlGraph.hpp"

PNL_USING

static const char neighTypeSymbols[3] = { 'p', 'c', 'n' };

void
CGraphPersistence::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CGraph *pG = dynamic_cast<CGraph*>(pObj);
    std::stringstream buf;

    PNL_CHECK_IS_NULL_POINTER(pG);
    int nNode = pG->GetNumberOfNodes();
    intVector neig;
    neighborTypeVector neigType;

    pContext->AddAttribute("SavingType", "ByEdges");

    {
        char buf2[12];

        sprintf(buf2, "%i", nNode);
        pContext->AddAttribute("NumberOfNodes", buf2);
    }

    for(int i = 0; i < nNode; ++i)
    {
        pG->GetNeighbors(i, &neig, &neigType);
        buf << i << ":";
        for(int j = neig.size(); --j >= 0;)
        {
            buf << neig[j] << "_(" << neighTypeSymbols[neigType[j]] << ") ";
        }
        buf << '\n';
    }

    pContext->AddText(buf.str().c_str());
}

CPNLBase *
CGraphPersistence::Load(CContextLoad *pContext)
{
    pnlString savingType;
    pnlString nNodeStr;
    
    pContext->GetAttribute(savingType, "SavingType");
    pContext->GetAttribute(nNodeStr, "NumberOfNodes");
    
    int nNode = atoi(nNodeStr.c_str());
    intVecVector aNbrs;
    neighborTypeVecVector aNbrsTypes;
    pContext->GetText(nNodeStr);
    std::istringstream buf(nNodeStr.c_str());
    int i, j, k;
    char ch;

    aNbrs.resize(nNode);
    aNbrsTypes.resize(nNode);

    for( i = 0; i < nNode && buf.good() && !buf.eof(); ++i )
    {
        buf >> j;
        ASSERT(j == i);
        buf >> ch;
        ASSERT(ch == ':');
        ch = (char)buf.peek();
        if(isdigit(ch))
        {
            for(;;)
            {
                buf >> j;
                buf >> ch;
                ASSERT(ch == '_');
                buf >> ch;
                ASSERT(ch == '(');
                buf >> ch;
                for(k = 0; k < sizeof(neighTypeSymbols) && ch != neighTypeSymbols[k]; ++k);
                
                ASSERT(k < sizeof(neighTypeSymbols));
                aNbrs[i].push_back(j);
                aNbrsTypes[i].push_back((const ENeighborType)k);
                buf >> ch;
                ASSERT(ch == ')');
		while(isspace(buf.peek()))
		{
		    buf.get(ch);
		    if(ch == '\n' || ch == '\r')
		    {
			break;
		    }
		}
                if(ch == '\n' || ch == '\r')
                {
                    break;
                }
            }
        }
        ASSERT(ch == '\n' || ch == '\r');
    }

    ASSERT( i == nNode );

    return CGraph::Create(aNbrs, aNbrsTypes);
}

bool CGraphPersistence::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CGraph*>(pObj) != 0;
}
