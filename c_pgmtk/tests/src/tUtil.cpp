#include "tConfig.h"
#include <time.h>

int pnlTestRandSeed()
{
#ifdef _DEBUG
        return 121;
#else
        return int(time(NULL));
#endif
}
