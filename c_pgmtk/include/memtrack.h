#ifndef _MEMTRACK_H
#define _MEMTRACK_H

#if defined(_DEBUG) && (_MSC_VER) 

#include <crtdbg.h>
#include <malloc.h>

#include "dbginfo.h"
#include "pnlConfig.h"

#define STACK_SIZE 32
#define INITIAL_STACK_TABLE_SIZE 4096
#define MY_BLOCK_SUBTYPE 1
#define MY_BLOCK_TYPE (_CLIENT_BLOCK | (MY_BLOCK_SUBTYPE << 16))
#define MY_TYPE_MASK ((unsigned)-1 >> (32-17))

#define nNoMansLandSize 4
#define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)
#define MAXPRINT 16

typedef struct _CrtMemBlockHeader
{
	struct _CrtMemBlockHeader * pBlockHeaderNext;
	struct _CrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	size_t                      nDataSize;
	int                         nBlockUse;
	long                        lRequest;
	unsigned char               gap[nNoMansLandSize];
	/* followed by:
			 *  unsigned char           data[nDataSize];
			 *  unsigned char           anotherGap[nNoMansLandSize];
			 */
} _CrtMemBlockHeader;

int DumpLeaksAndCleanup(int lRequest = 0);

PNL_API int InitMemTrack();
PNL_API int DumpMemoryLeaks(int lRequest = 0);
class DumpUsing 
{
public:
	DumpUsing()
	{
		m_lRequest = InitMemTrack();
	}
	~DumpUsing()
	{
		DumpMemoryLeaks(m_lRequest);
	}
	int m_lRequest;
};

#endif // #if defined(_DEBUG && _MSC_VER ) 

#endif