#include "memtrack.h"

#if defined(_DEBUG) && (_MSC_VER) 

static void*** g_pppStackInfo;
static int nStackInfo = INITIAL_STACK_TABLE_SIZE; 
static _CRT_ALLOC_HOOK g_pfnOldAllocHook = 0;
#define PGMT_LOG_NAME "D:\\PGMT\\pgmt.log"
//static int is_terminating = 0;

#define DUMP_ALLOC 0
#define STACK_DUMP_LEAKS 0

#if DUMP_ALLOC
	static FILE* g_dump_alloc_file = NULL;
#endif

inline void** GetStackInfo(long lRequest)
{
	return (lRequest < nStackInfo) ? g_pppStackInfo[lRequest] : NULL;
}

inline void** AddStackInfo(long lRequest)
{
	if (lRequest >= nStackInfo)
	{
		int nNewStackInfo = __max(lRequest + 128, nStackInfo * 2);
		g_pppStackInfo = 
			(void***)_realloc_dbg(g_pppStackInfo, nNewStackInfo * sizeof(void**), 
			                     _CRT_BLOCK, 0 , 0);
		memset(g_pppStackInfo + nStackInfo , 0, (nNewStackInfo - nStackInfo) * sizeof(void**) );
		nStackInfo = nNewStackInfo;
	}
	else
	{
		assert(g_pppStackInfo[lRequest] == NULL);
	}
	return (g_pppStackInfo[lRequest] = (void**)
			_calloc_dbg( STACK_SIZE, sizeof(void**), 
						_CRT_BLOCK, 0 , 0));
}

#pragma warning (disable : 4100 4127 4074)
static void printMemBlockData(_CrtMemBlockHeader* pHead )
{
	int i;
	unsigned char ch;
	char printbuff[MAXPRINT+1];
	char valbuff[MAXPRINT*3+1];
	
	for (i = 0; i < __min((int)pHead->nDataSize, MAXPRINT); i++)
	{
		ch = pbData(pHead)[i];
		printbuff[i] = isprint(ch) ? ch : ' ';
		sprintf(&valbuff[i*3], "%.2X ", ch);
	}
	printbuff[i] = '\0';
	
	_RPT2(_CRT_WARN, " Data: <%s> %s\n", printbuff, valbuff);
}

#define BLOCK_TYPE_IS_VALID(use) (_BLOCK_TYPE(use) == _CLIENT_BLOCK || \
                                              (use) == _NORMAL_BLOCK || \
                                   _BLOCK_TYPE(use) == _CRT_BLOCK    || \
                                              (use) == _IGNORE_BLOCK)
//FILE* g_dump_alloc_file = fopen("dumpalloc.txt","wt");

int MyAllocHook( int allocType, void* pUserData, 
				 size_t size, int blockType, 
				 long lRequest, 
				 const unsigned char* filename,
				 int lineNumber)
{
//	if (28625 == lRequest/* && allocType != _HOOK_REALLOC*/)
//		__asm int 3;
#if STACK_DUMP_ALLOC
	if (blockType == _IGNORE_BLOCK ||
		blockType == _CRT_BLOCK ||
 		blockType == _FREE_BLOCK)
	{
		return g_pfnOldAllocHook(allocType , pUserData, size , blockType,
							lRequest, filename, lineNumber);;
	}
	if (allocType == _HOOK_ALLOC)
	{
		void** ppStack = AddStackInfo( lRequest );
		assert(ppStack);
		CONTEXT context;
		GetCurrentContext(&context);
		GetStackAddresses(&context, ppStack, STACK_SIZE); 
	}
	else if (allocType == _HOOK_FREE)
	{
		_CrtMemBlockHeader* pHead = pHdr(pUserData);
//// verify block  
	    _ASSERTE(BLOCK_TYPE_IS_VALID(pHead->nBlockUse));
		_ASSERTE(_CrtIsValidHeapPointer(pUserData));
    
		void* ppStack = GetStackInfo( pHead->lRequest );
		if (ppStack)
		{
			_free_dbg(ppStack, _CRT_BLOCK);
			g_pppStackInfo[ pHead->lRequest ] = NULL;
		}
	}
#elif DUMP_ALLOC
	if (g_dump_alloc_file)
	{
		char buf[256];
		if (allocType == _HOOK_ALLOC)
		{
			sprintf(buf, "M;%d;%d;%d\n", blockType, lRequest, size);
		}
		else if (allocType == _HOOK_FREE)
		{
			_CrtMemBlockHeader* pHead = pHdr(pUserData);
			sprintf(buf, "F;%d;%d;%d;0x%08X\n", blockType,
			    pHead->lRequest, pHead->nDataSize, pUserData);
		}
		else //(allocType == _HOOK_REALLOC)
		{
			_CrtMemBlockHeader* pHead = pHdr(pUserData);
			sprintf(buf, "R;%d;%d;%d;0x%08X;%d\n", blockType,
			    pHead->lRequest, size, pUserData, lRequest);
		}
		fputs(buf, g_dump_alloc_file);
	}

#endif
	return g_pfnOldAllocHook(allocType , pUserData, size , blockType,
						lRequest, filename, lineNumber);;
}

static char g_szBuff[1024];

static char* g_strIgnoreModuleList[] =
{
//	"MSVCP60D.dll",
	"MSVCRTD.dll"
};

static char* g_strIgnoreModuleList1[] =
{
	"MSVCP60D.dll"
};

static char* g_strIgnoreFileList[] =
{
/*	"xxxxxxxxxxxxx",
	"vc98\\include\\vector",
	"vc98\\include\\crtdbg.h",
	"vc98\\include\\xmemory",
	"vc98\\include\\crtdbg.h",*/
	"vc98\\include",
	"include\\pnltypedefs.hpp"
};

int DumpMyBlock(_CrtMemBlockHeader* pHead)
{
	void** ppStack = (void**)GetStackInfo( pHead->lRequest );
 	assert(ppStack);
	void** _pStack = ppStack;
	void** pStackEnd = ppStack + STACK_SIZE;

//// Check for 'skip' libraries
	do
	{
		GetShortModuleNameFromAddr(*(++_pStack), g_szBuff, MAX_PATH);
		for (int i = 0 ; i < sizeof(g_strIgnoreModuleList1)/sizeof(char*) ; i++)
		{
			if (strcmp(g_szBuff, g_strIgnoreModuleList1[i]) == 0)
				return 0 ;
		}
	}
	while (_pStack < pStackEnd);

//// Dump general block info	
	_RPT4(_CRT_WARN, "My block {%ld} at 0x%08X, type %x, %u bytes long.\n", pHead->lRequest,
		(BYTE *)pbData(pHead), _BLOCK_TYPE(pHead->nBlockUse), pHead->nDataSize);
	printMemBlockData(pHead);
	
	_pStack = ppStack;
	int ignored ;
//// Skip stack entries for 'ignored' libraries (CRT for example - MSVCRTD.dll).
	do
	{
		GetShortModuleNameFromAddr(*(++_pStack), g_szBuff, MAX_PATH);
		ignored = 0;
		for (int i = 0 ; !ignored && (i < sizeof(g_strIgnoreModuleList)/sizeof(char*)) ; i++)
			ignored = (strcmp(g_szBuff, g_strIgnoreModuleList[i]) == 0);
	}
	while (ignored && (_pStack < pStackEnd));

//// Output allocation stack
	_RPT0(_CRT_WARN, "Stack at allocation time : \r\n");
//	pStackEnd = min(_pStack + 8, pStackEnd);
	int nTraces = 0;
	while ( _pStack < pStackEnd && nTraces < 8)
	{
		if (DumpAddressInfo(*_pStack, g_szBuff, 1024))
		{
			ignored = 0;
//// Ignore common CRT headers
			for (int i = 0 ; !ignored && (i < sizeof(g_strIgnoreFileList)/sizeof(char*)) ; i++)
				ignored = (strstr(g_szBuff, g_strIgnoreFileList[i]) != NULL);
			if (!ignored)
			{
				OutputDebugString(g_szBuff);
				nTraces++;
			}
		}
		_pStack ++;
	}
	return 1;
}


static _CrtMemBlockHeader* GetFirstCrtBlock()
{
	_CrtMemState state;
	_CrtMemCheckpoint(&state);
	_CrtMemBlockHeader* pHead = state.pBlockHeader;
	_CrtMemBlockHeader* pHeadFirst = pHead;
	while (pHead)
	{
		pHeadFirst = pHead;
		pHead = pHead->pBlockHeaderPrev;
	}
	return pHeadFirst;
}

static int DumpAllMyBlocks(int lRequest = 0)
{
	_CrtMemBlockHeader* pHead = GetFirstCrtBlock();
	int nDumpedBlocks = 0;
	while (pHead)
	{
		int lCurRequest = pHead->lRequest;
		int nBlockUse = pHead->nBlockUse;
		if (lCurRequest > lRequest &&
			nBlockUse != _FREE_BLOCK &&
			nBlockUse != _IGNORE_BLOCK && 
			nBlockUse != _CRT_BLOCK &&
			(nBlockUse != _CLIENT_BLOCK || _BLOCK_SUBTYPE(nBlockUse) == 0) &&
			g_pppStackInfo[lCurRequest])
		{
		//	if (is_terminating == 0)
			nDumpedBlocks += DumpMyBlock( pHead );
			pHead->nBlockUse = _CRT_BLOCK;
		}
		pHead = pHead->pBlockHeaderNext;
	}
	return nDumpedBlocks;
}

int DumpMemoryLeaks(int lRequest)
{
// only dump leaks when there are in fact leaks 
	_CrtMemState msNow;
	_CrtMemCheckpoint(&msNow);
	int ret = 0;
	if (msNow.lCounts[_CLIENT_BLOCK] != 0 ||
		msNow.lCounts[_NORMAL_BLOCK] != 0 ||
		(_crtDbgFlag & _CRTDBG_CHECK_CRT_DF &&
		msNow.lCounts[_CRT_BLOCK] != 0)	)
	{
// difference detected: dump objects since start. 
		_RPT0(_CRT_WARN, "Detected memory leaks!\n");
		ret = DumpAllMyBlocks(lRequest);
		_RPT0(_CRT_WARN, "Object dump complete.\n");
	}
	return ret;
}
/*
void my_exit(void)
{
//	is_terminating = 1;
	return ;
}
*/
int InitMemTrack()
{
	static int init = 0;
	_CrtMemState state;
	_CrtMemCheckpoint(&state);
	if (!init)
	{
		_set_error_mode( _OUT_TO_MSGBOX );
		g_pfnOldAllocHook = _CrtSetAllocHook(MyAllocHook);
#if STACK_DUMP_ALLOC
		g_pppStackInfo = (void***)_calloc_dbg( INITIAL_STACK_TABLE_SIZE, 
			sizeof(void**), _CRT_BLOCK, 0 , 0);
#endif
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG );
		_CrtSetDbgFlag(flag | _CRTDBG_LEAK_CHECK_DF);
		init = 1;
	}
	return state.pBlockHeader->lRequest;
}


int DumpLeaksAndCleanup(int lRequest)
{
	_CrtSetAllocHook(g_pfnOldAllocHook);
	int ret = DumpMemoryLeaks(lRequest);
	for (int i = 0 ; i < nStackInfo ; i++)
	{
		void** ppStack = g_pppStackInfo[i] ;
		if (ppStack)
			_free_dbg(ppStack, _CRT_BLOCK);
	}
	_free_dbg(g_pppStackInfo , _CRT_BLOCK);
	CleanupSymEng();
	return ret;
}

#if STACK_DUMP_ALLOC || DUMP_ALLOC // Uncomment this block to enable allocation stack unrolling
#pragma init_seg(compiler)

class DumpUsingStatic {
public:
	DumpUsingStatic()
	{
#if DUMP_ALLOC
		g_dump_alloc_file = fopen("dumpalloc.txt","wt");
		fputs("Allocation log\n", g_dump_alloc_file);
#endif
		m_lRequest = InitMemTrack();
	}
	~DumpUsingStatic()
	{
#if STACK_DUMP_ALLOC
		assert(DumpLeaksAndCleanup(m_lRequest) == 0);
#elif DUMP_ALLOC
		if (g_dump_alloc_file)
		{
			fclose( g_dump_alloc_file );
			g_dump_alloc_file = NULL;
		}
#endif
	}
	int m_lRequest;
};

static DumpUsingStatic dumpUsing;
#endif

#endif//#if defined(_DEBUG && _MSC_VER ) 

