#include "dbginfo.h"

#if defined(_DEBUG) && (_MSC_VER) 

#ifdef _ALPHA_
#define CH_MACHINE IMAGE_FILE_MACHINE_ALPHA
#else
#define CH_MACHINE IMAGE_FILE_MACHINE_I386
#endif

// The pointers to IMAGEHLP.DLL functions
static PFNSYMGETLINEFROMADDR g_pfnSymGetLineFromAddr = NULL;
extern PFNSYMGETSYMFROMADDR  g_pfnSymGetSymFromAddr  = NULL;
static PFNSYMGETLINEFROMNAME g_pfnSymGetLineFromName = NULL;
static PFNSYMINITIALIZE      g_pfnSymInitialize      = NULL;
static PFNSYMCLEANUP         g_pfnSymCleanup         = NULL;
static PFNSTACKWALK          g_pfnStackWalk          = NULL;
static PFNSYMFUNCTABLEACCESS g_pfnSymFunctionTableAccess = NULL;
static PFNSYMGETMODULEBASE   g_pfnSymGetModuleBase   = NULL;
static PFNSYMSETOPTIONS      g_pfnSymSetOptions      = NULL;  
static PFNSYMGETOPTIONS      g_pfnSymGetOptions      = NULL;   

#define SYM_BUFF_SIZE 512
#define BUFF_SIZE 1024

// The flag that indicates that the symbol engine as been initialized.
static BOOL g_bSymEngInit = FALSE ;

// The stack frame used in walking the stack.
static STACKFRAME g_stFrame ;

// The static buffer returned by various functions.  This avoids putting
//  things on the stack.
static TCHAR g_szBuff [ BUFF_SIZE ] ;

// The static symbol lookup buffer.  This gets casted to make it work.
static BYTE g_stSymbol [ SYM_BUFF_SIZE ] ;

// Current module path
static char g_szModulePath [ MAX_PATH + 1 ] ;

// The static source and line structure.
static IMAGEHLP_LINE g_stLine ;

static HINSTANCE   g_hImageHlp = NULL;
static CONTEXT context;

//// Symbol paths relative to <exe dir>//.. ( usually PGMT root dir ).
static char* g_strRelSymPath[] = 
{
	"temp\\tests_pnl_c\\Debug",
	"temp\\learn_param\\Debug",
	"temp\\trial\\debug",
	"temp\\inf_learn_bnet\\Debug",
	"temp\\inf_learn_dbn\\Debug",
	"temp\\pnl\\Debug",
	"cxcore\\bin"				 
};

//// Comma - separated absolute symbol paths
#define ABS_SYM_PATH "S:\\dp.BNT\\heap\\cxcore\\_temp\\cxcore_Dbg;"

#pragma warning (disable : 4100 4127 4074)
// Initializes the symbol engine if needed.
void InitSymEng ( void )
{
	char szPGMTPath [ MAX_PATH + 1 ] ;
	char szExePath [ MAX_PATH + 1 ] ;

	if ( NULL == g_hImageHlp)
	{
#ifdef _DEBUG
	HMODULE hMod = GetModuleHandle("pnld.dll");
#else
	HMODULE hMod = GetModuleHandle("pnl.dll");
#endif
	if (GetModuleFileName(hMod, g_szModulePath, MAX_PATH))
	{
		char* pos = strrchr(g_szModulePath, '\\');
		if (pos)
			*pos = 0;
	}
	if (GetModuleFileName(NULL, szExePath, MAX_PATH))
	{
		char* pos = strrchr(szExePath, '\\');
		*szPGMTPath = 0;
		if (pos)
		{
			*pos = 0;
			pos = strrchr(szExePath, '\\');
			if (pos)
			{
				int len = pos - szExePath;
				strncpy( szPGMTPath , szExePath , len );
				szPGMTPath[len] = 0;
			}

		}
	}
	g_hImageHlp = LoadLibrary("IMAGEHLP.DLL") ;
	if (g_hImageHlp == NULL)
	{
		GetSystemDirectory(g_szModulePath, MAX_PATH);
		strcat(g_szModulePath, "\\IMAGEHLP.DLL"); 
		g_hImageHlp = LoadLibrary(g_szModulePath) ;
	}
	if (g_hImageHlp && g_pfnSymGetLineFromAddr == NULL)
		g_pfnSymGetLineFromAddr = 
		(PFNSYMGETLINEFROMADDR)GetProcAddress(g_hImageHlp ,
		"SymGetLineFromAddr");
	if ( NULL == g_pfnSymGetLineFromAddr )
	{
		g_pfnSymGetLineFromName		= NULL ;
		g_pfnSymInitialize			= NULL ;
		g_pfnSymCleanup				= NULL ;
		g_pfnStackWalk				= NULL ;
      		g_pfnSymFunctionTableAccess = NULL ;
			g_pfnSymGetModuleBase		= NULL ;
			g_pfnSymSetOptions			= NULL ;  
			g_pfnSymGetOptions			= NULL ;   
	}
	else
	{
		g_pfnSymGetLineFromName =
			(PFNSYMGETLINEFROMNAME)GetProcAddress(g_hImageHlp ,
			"SymGetLineFromName");
		g_pfnSymGetSymFromAddr =
			(PFNSYMGETSYMFROMADDR)GetProcAddress(g_hImageHlp ,
			"SymGetSymFromAddr");
		
		g_pfnSymCleanup    =
			(PFNSYMCLEANUP)GetProcAddress ( g_hImageHlp  ,
			"SymCleanup" ) ;
		g_pfnSymInitialize  =
			(PFNSYMINITIALIZE)GetProcAddress ( g_hImageHlp  ,
			"SymInitialize"  ) ;
		g_pfnStackWalk =
			(PFNSTACKWALK)GetProcAddress(g_hImageHlp , "StackWalk");
		
		g_pfnSymFunctionTableAccess = 
			(PFNSYMFUNCTABLEACCESS)GetProcAddress(g_hImageHlp ,
			"SymFunctionTableAccess");
		g_pfnSymGetModuleBase   =
			(PFNSYMGETMODULEBASE)GetProcAddress (g_hImageHlp ,
			"SymGetModuleBase");
		g_pfnSymSetOptions =   
			(PFNSYMSETOPTIONS)GetProcAddress (g_hImageHlp ,
			"SymSetOptions");
		g_pfnSymGetOptions      =  
			(PFNSYMGETOPTIONS)GetProcAddress (g_hImageHlp ,
			"SymGetOptions");
		
	}
	}
    if ( FALSE == g_bSymEngInit )
    {
		if (g_hImageHlp && g_pfnSymGetLineFromAddr 
			&& g_pfnSymInitialize)
        {
			// Set up the symbol engine.
			DWORD dwOpts = g_pfnSymGetOptions ( ) ;

			// Always defer loading to make life faster.
			g_pfnSymSetOptions (dwOpts                |
								SYMOPT_DEFERRED_LOADS |
								SYMOPT_LOAD_LINES ) ;

			// Initialize the symbol engine.
			HANDLE hProcess = GetCurrentProcess( );
			char* _buf = g_szBuff;
			_buf += sprintf(_buf, "%s;%s;"ABS_SYM_PATH,
							g_szModulePath, szExePath);
			char* end_buf = g_szBuff + sizeof(g_szBuff);
			for (int i = 0 ; i < sizeof(g_strRelSymPath)/sizeof(char*) ; i++)
			{
				int count = _snprintf(_buf, end_buf - _buf , "%s\\%s;", szPGMTPath, g_strRelSymPath[i]); 
				if (count > 0)
					_buf += count;
				else
				{
					*(end_buf-1)=0;
					break;
				}
			}
						
			GetSystemDirectory(_buf, g_szBuff + BUFF_SIZE - _buf); 
			_RPT1 (_CRT_WARN, "Loading symbols in directories : \r\n%s\r\n", g_szBuff);
			g_bSymEngInit =  g_pfnSymInitialize ( hProcess, g_szBuff, TRUE ) ;
		}
	}
}

// Cleans up the symbol engine if needed.
void CleanupSymEng ( void )
{
	if (NULL == g_hImageHlp)
		return;
    if ( g_bSymEngInit && g_pfnSymCleanup)
    {
        VERIFY( g_pfnSymCleanup (GetCurrentProcess ()));
	}
    g_bSymEngInit = FALSE ;
	FreeLibrary(g_hImageHlp);
	g_hImageHlp = NULL;
	g_pfnSymGetLineFromName = NULL ;
	g_pfnSymInitialize      = NULL ;
	g_pfnSymCleanup         = NULL ;
	g_pfnStackWalk          = NULL ;
	g_pfnSymFunctionTableAccess = NULL;
	g_pfnSymGetModuleBase   = NULL;
	g_pfnSymSetOptions      = NULL;  
	g_pfnSymGetOptions      = NULL;   
}

/*	if (!IsDebuggerPresent())
		return;*/

DWORD GetStackAddresses(CONTEXT* pContext, PVOID* ppAddr, DWORD dwMaxAddr)
{
	DWORD dwAddr = dwMaxAddr;
	__try
    {
        // Initialize the symbol engine in case it is not initialized.
        InitSymEng ( ) ;
		if (NULL == g_pfnSymGetSymFromAddr || 
			NULL == g_pfnStackWalk ||
			NULL == g_pfnSymFunctionTableAccess ||
			NULL == g_pfnSymGetModuleBase)
			return 0 ;

    	HANDLE hProcess = GetCurrentProcess();
      	// Initialize the STACKFRAME structure.
		ZeroMemory ( &g_stFrame , sizeof ( STACKFRAME ) ) ;
		CONTEXT context = *pContext;
	
#ifdef _X86_
		g_stFrame.AddrPC.Offset       = context.Eip  ;
		g_stFrame.AddrPC.Mode         = AddrModeFlat ;
		g_stFrame.AddrStack.Offset    = context.Esp  ;
		g_stFrame.AddrStack.Mode      = AddrModeFlat ;
		g_stFrame.AddrFrame.Offset    = context.Ebp  ;
		g_stFrame.AddrFrame.Mode      = AddrModeFlat ;
#else
		g_stFrame.AddrPC.Offset       = (DWORD)context.Fir ;
		g_stFrame.AddrPC.Mode         = AddrModeFlat ;
		g_stFrame.AddrReturn.Offset   = (DWORD)context.IntRa;
		g_stFrame.AddrReturn.Mode     = AddrModeFlat ;
		g_stFrame.AddrStack.Offset    = (DWORD)context.IntSp;
		g_stFrame.AddrStack.Mode      = AddrModeFlat ;
		g_stFrame.AddrFrame.Offset    = (DWORD)context.IntFp;
		g_stFrame.AddrFrame.Mode      = AddrModeFlat ;
#endif
		while(g_pfnStackWalk ( CH_MACHINE, hProcess, GetCurrentThread ( ),
							   &g_stFrame, &context, NULL ,
							   g_pfnSymFunctionTableAccess ,
							   g_pfnSymGetModuleBase, NULL ) && g_stFrame.AddrFrame.Offset && dwAddr--)
       
		{	
			*(ppAddr++) = (PVOID)g_stFrame.AddrPC.Offset;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return dwMaxAddr - dwAddr;
}


DWORD GetShortModuleNameFromAddr( PVOID addr , LPTSTR szModName, DWORD dwLen)
{
  	 MEMORY_BASIC_INFORMATION mbi;
	 strncpy(szModName , "<UNKNOWN>", dwLen);
     if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
         return 0;
	 if (mbi.State == MEM_FREE)
		 return 0;
     DWORD hMod = (DWORD)mbi.AllocationBase;
   	 if (hMod == 0)
		 return 0;
  	 DWORD dwRet = GetModuleFileName ( (HINSTANCE)hMod , g_szModulePath , dwLen ) ;
	 if ( dwRet <= 0)
		 return 0;
	 char* pStart = strrchr ( g_szModulePath , '\\' ) ;
     if (!pStart) 
		 pStart = g_szModulePath;
	 else
		 pStart++;
	 strncpy( szModName , pStart , dwLen);
	 return (g_szModulePath + dwRet ) - pStart ;
}

DWORD GetModuleNameFromAddr( PVOID addr , LPTSTR szModName, DWORD dwLen)
{
	 MEMORY_BASIC_INFORMATION mbi;
	 strncpy(szModName , "<UNKNOWN>", dwLen);
     if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
         return 0;
	 if (mbi.State == MEM_FREE)
		 return 0;
     DWORD hMod = (DWORD)mbi.AllocationBase;
	 if (hMod == 0)
		 return 0;
     DWORD dwRet = GetModuleFileName ((HINSTANCE) hMod , szModName , dwLen ) ;
	 return dwRet;
}

DWORD DumpAddressInfo( PVOID pAddr, LPTSTR szBuf, DWORD dwLen)
{
	if (g_pfnSymGetSymFromAddr == NULL ||
		g_pfnSymGetLineFromAddr == NULL)
		return 0;
	InitSymEng( );
	// Start looking up the exception address.
	PIMAGEHLP_SYMBOL pSym = (PIMAGEHLP_SYMBOL)&g_stSymbol;
	ZeroMemory(pSym , SYM_BUFF_SIZE );
	pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) ;
	pSym->MaxNameLength = SYM_BUFF_SIZE - sizeof ( IMAGEHLP_SYMBOL ) ;
	ZeroMemory ( &g_stLine , sizeof ( IMAGEHLP_LINE ) ) ;
	g_stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;

	DWORD dwDisp = 0;
	char szModPath[ MAX_PATH + 1 ];
	if (!GetShortModuleNameFromAddr(pAddr, szModPath, MAX_PATH))
		return 0;
	DWORD dwRet = 0;
	if (g_pfnSymGetSymFromAddr(GetCurrentProcess(), (DWORD)pAddr , &dwDisp, pSym)  &&
		g_pfnSymGetLineFromAddr ( GetCurrentProcess (), (DWORD)pAddr, &dwDisp , &g_stLine ))
	{
		dwRet = _snprintf(szBuf , dwLen, "%hs(%d) : %s ( %s )\n",
						  g_stLine.FileName, g_stLine.LineNumber, 
						  pSym->Name, szModPath);
	}
	else
	{
		dwRet = _snprintf(szBuf , dwLen, "<no symbols> : 0x%08X ( %s )\n",
						  pAddr, szModPath);
	}
	return (dwRet < 0 ? dwLen : dwRet);
}

void __declspec(naked) GetCurrentContext(CONTEXT* pContext)
{
	__asm
	{
		mov eax, [esp+4]
		mov [eax]CONTEXT.Esp, esp
		mov [eax]CONTEXT.Ebp, ebp
		mov ebx, [esp]
		mov [eax]CONTEXT.Eip, ebx
		ret
	}
}
#pragma warning (default : 4100 4127 4074)
#endif//#if defined(_DEBUG && _MSC_VER ) 
