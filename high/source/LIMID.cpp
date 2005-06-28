#include "LIMID.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "LIMIDCB.hpp"

#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

LIMID::LIMID(): m_Inf(0)
{
    m_pLIMID = new ProbabilisticNet("idnet");

/*    TokIdNode *discrete = Net().Token()->Node("discrete");

    TokIdNode *chance = discrete->Add("chance");
    TokIdNode *decision = discrete->Add("decision");
    TokIdNode *value = discrete->Add("value");

    chance->tag = eTagNodeType;
    decision->tag = eTagNodeType;
    value->tag = eTagNodeType;

    chance->data = (void *)(new pnl::CNodeType(1, 2));
    decision->data = (void *)(new pnl::CNodeType(1, 2, pnl::nsDecision));
    value->data =  (void *)(new pnl::CNodeType(1, 1, pnl::nsValue));
*/
    m_pLIMID->SetCallback(new LIMIDCallback());
}

LIMID::~LIMID()
{
    pnl::CLIMIDInfEngine::Release(&m_Inf);
    delete m_pLIMID;
}

void LIMID::AddNode(TokArr nodes, TokArr subnodes)
{
    Net().AddNode(nodes, subnodes);
}

void LIMID::DelNode(TokArr nodes)
{
    Net().DelNode(nodes);
}

// manipulating arcs
void LIMID::AddArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);
}

void LIMID::DelArc(TokArr from, TokArr to)
{
    Net().DelArc(from, to);
}


void LIMID::SetPChance(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions().FillData(value, prob, parentValue);
}

void LIMID::SetPDecision(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions().FillData(value, prob, parentValue);
}

void LIMID::SetValueCost(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions().FillData(value, prob, parentValue);
}

TokArr LIMID::GetPChance(TokArr value, TokArr parents)
{
    return GetP(value, parents);
}

TokArr LIMID::GetPDecision(TokArr value, TokArr parents)
{
    return GetP(value, parents);
}

TokArr LIMID::GetValueCost(TokArr value, TokArr parents)
{
    return GetP(value, parents);
}

String LIMID::GetProperty(const char *name) const
{
    return m_pLIMID->GetProperty(name);
}

void LIMID::SetProperty(const char *name, const char *value)
{
    m_pLIMID->SetProperty(name, value);
}

void LIMID::SaveNet(const char *filename)
{
    pnl::CContextPersistence saver;

    if(!Net().SaveNet(&saver))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }

    if(!saver.SaveAsXML(filename))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
    // we must save net-specific data here.
    // LIMID haven't any specific data for now
}

void LIMID::LoadNet(const char *filename)
{
    pnl::CContextPersistence loader;

    if(!loader.LoadXML(filename))
    {
	ThrowUsingError("Can't load file - bad file?", "LoadNet");
    }

    ProbabilisticNet *pNewNet = ProbabilisticNet::LoadNet(&loader);
    if(!pNewNet)
    {
	ThrowUsingError("Can't load file - bad file?", "LoadNet");
    }

    delete &Net();
    m_pLIMID = pNewNet;
    m_pLIMID->SetCallback(new LIMIDCallback());

    // We must load net-specific data here.
    // LIMID haven't any specific data for now
}

void LIMID::SetIterMax(int IterMax)
{
    pnl::CLIMIDInfEngine *infEngine = &Inference();
    infEngine->SetIterMax(IterMax);
}

TokArr LIMID::GetExpectation()
{
    TokArr res = "";

    pnl::CLIMIDInfEngine *infEngine = &Inference();

/*    if (Net().IsModelChanged())
    {
*/
        infEngine->DoInference();
/*        Net().SetModelChanged(0);
    }
*/
    float resExp = infEngine->GetExpectation();
    res << Tok(resExp);
    Net().Token().SetContext(res);

    return res;
}

TokArr LIMID::GetPolitics()
{
    TokArr res = "";

    pnl::CLIMIDInfEngine *infEngine = &Inference();

/*    if (Net().IsModelChanged())
    {
*/
        infEngine->DoInference();
/*        Net().SetModelChanged(0);
    }
*/
    pnl::pFactorVector *Vec = infEngine->GetPolitics();

    for (int i = 0; i < Vec->size(); i++)
    {
        const pnl::CMatrix<float> *mat = (*Vec)[i]->GetMatrix(pnl::matTable);
        res << Net().ConvertMatrixToToken(mat);
    }

    Net().Token().SetContext(res);

    return res;
}

//= private functions  =================================================

void LIMID::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eSetModelInvalid:
	m_Inf->Release(&m_Inf);
	m_Inf = 0;
	break;
    default:
	ThrowInternalError("Unhandled message arrive" ,"DoNotify");
	return;
    }
}

pnl::CIDNet &LIMID::Model()
{
    return static_cast<pnl::CIDNet&>(Net().Model());
}

pnl::CLIMIDInfEngine & LIMID::Inference()
{
/*    if(m_Inf)
    {
        return *m_Inf;
    }
    else
    {
        m_Inf = pnl::CLIMIDInfEngine::Create(Model());
        return *m_Inf;
    }
*/
    if(m_Inf)
    {
        pnl::CLIMIDInfEngine::Release(&m_Inf);
    }
    m_Inf = pnl::CLIMIDInfEngine::Create(&Model());
    return *m_Inf;
}

TokArr LIMID::GetP(TokArr child, TokArr parents)
{

    static const char fname[] = "GetP";
    
    int nchldComb = child.size();
    if( !nchldComb )
    {
	ThrowUsingError("Must be at least one combination for a child node", fname);
    }

    Vector<int> childNd, childVl;
    Vector<int> childNdInner;
    int i;
    Net().ExtractTokArr(child, &childNd, &childVl);

    Net().Graph().IGraph( &childNd, &childNdInner);

    // node for all child must be same (its may differ by combination only)
    for(i = childNd.size(); --i > 0; ++i)
    {
	if(childNd[i] != childNd[0])
	{
	    ThrowUsingError("Can't return probabilities for different nodes", fname);
	}
    }
	
    if( !childVl.size())
    {
	childVl.assign(nchldComb, -1);
    }

    Vector<int> parentNds, parentVls;
    Vector<int> parentNdsInner;
    int nparents = parents.size();
    if( nparents )
    {
	Net().ExtractTokArr(parents, &parentNds, &parentVls);
	Net().Graph().IGraph( &parentNds, &parentNdsInner);
	if( parentVls.size() == 0 ||
	    std::find(parentVls.begin(), parentVls.end(), -1 ) != parentVls.end() )
	{
	    ThrowInternalError("undefindes values for given parent nodes", "P");
	}
    }
    else
    {
	Net().Graph().Graph()->GetParents( childNdInner.front(), &parentNdsInner );
	Net().Graph().IOuter( &parentNdsInner, &parentNds);
	nparents = parentNds.size();
	parentVls.assign(nparents, -1);
    }

    parentNds.resize(nparents + 1);
    parentNdsInner.resize(nparents + 1);
    parentVls.resize(nparents + 1);

    const pnl::CFactor * cpd = Model().GetFactor(childNdInner.front());
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matTable);
	
    TokArr result = "";
    for( i = 0; i < nchldComb; i++ )
    {
	parentNds[nparents] = childNd.front();
	parentNdsInner[nparents] = childNdInner.front();
	parentVls[nparents] = childVl[i];
	result << Net().CutReq( parentNds, parentVls, mat);
    }

    Net().Token().SetContext(result);
    return result;
}

PNLW_END
