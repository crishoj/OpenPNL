#include "Tokens.hpp"
#include "WInner.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)

PNLW_BEGIN

TokIdNode* flatSearch(TokIdNode *parent, const char *name)
{
    TokIdNode *child = parent->v_next;
    String sname(name);

    for(; child; child = child->h_next)
    {
#if 1
	if(child->Name() == sname)
	{
	    return child;
	}
#else
	for(int j = child->id.size(); --j >= 0;)
	{
	    if(child->id[j].id == sname)
	    {
		return child;
	    }
	}
#endif
    }

    return 0;
}

TokIdNode *ancestorByTagValue(TokIdNode *node, int tag, int value)
{
    PNL_CHECK_IS_NULL_POINTER(node);
    for(node = node->v_prev; node; node = node->v_prev)
    {
	if(node->tag == tag)
	{
	    return node;
	}
    }

    return 0;
}

// print error and throw exception
void ThrowUsingError(const char *message, const char *func)
{
    pnl::pnlString str;

    str << "wrong call to function " << func << "(): " << message << '\n';
    //fputs(str.c_str(), stderr);
    PNL_THROW(pnl::CInconsistentState, str.c_str());
}

// print error and throw exception
void ThrowInternalError(const char *message, const char *func)
{
    pnl::pnlString str;

    str << "internal error in function " << func << "(): " << message << '\n';
    //fputs(str.c_str(), stderr);
    PNL_THROW(pnl::CInternalError, str.c_str());
}

static void recursivePrintTokTree(FILE *fout, TokIdNode *node)
{
    pnl::pnlString name = node->Name();
    fprintf(fout, "<%s>\n", name.c_str());
    TokIdNode *subNode = node->v_next;
    while(subNode)
    {
        recursivePrintTokTree(fout, subNode);
        subNode = subNode->h_next;
    }
    fprintf(fout, "</%s>\n", name.c_str());
}

void PrintTokTree(const char *filename, TokIdNode *node)
{
    FILE *fp = fopen(filename, "w");
    if(fp)
    {
        recursivePrintTokTree(fp, node);
        fclose(fp);
    }
}

PNLW_END
