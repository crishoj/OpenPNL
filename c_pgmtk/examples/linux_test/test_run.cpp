#include "pnl_dll.hpp"

int main()
{
    pnl::CNodeType a(1,2);
    pnl::CNodeType b(0,5);

    printf("(1,2) isdiscrete %i, nodesize %i\n",
	a.IsDiscrete(), a.GetNodeSize());
    printf("(0,5) isdiscrete %i, nodesize %i\n",
	b.IsDiscrete(), b.GetNodeSize());

    return 0;
}

