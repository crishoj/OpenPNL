#ifndef _DBGINFO_H
#define _DBGINFO_H

#if defined(_DEBUG) && (_MSC_VER) 

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <assert.h>
#include <crtdbg.h>

// Typedefs for the new source and line functions.
typedef BOOL (__stdcall *PFNSYMGETLINEFROMADDR)
                              ( IN  HANDLE         hProcess         ,
                                IN  DWORD          dwAddr           ,
                                OUT PDWORD         pdwDisplacement  ,
                                OUT PIMAGEHLP_LINE Line              ) ;
typedef BOOL (__stdcall *PFNSYMGETSYMFROMADDR)
  ( IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PDWORD              pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL    Symbol );

typedef BOOL (__stdcall *PFNSYMGETLINEFROMNAME)
                              ( IN     HANDLE         hProcess      ,
                                IN     LPSTR          ModuleName    ,
                                IN     LPSTR          FileName      ,
                                IN     DWORD          dwLineNumber  ,
                                OUT    PLONG          plDisplacement,
                                IN OUT PIMAGEHLP_LINE Line           ) ;

typedef BOOL (__stdcall *PFNSYMINITIALIZE) (IN HANDLE   hProcess,
									IN LPSTR    UserSearchPath,
									IN BOOL     fInvadeProcess );

typedef BOOL (__stdcall *PFNSYMCLEANUP) (IN HANDLE   hProcess );

typedef BOOL (__stdcall *PFNSTACKWALK) (DWORD                     MachineType,
								HANDLE                            hProcess,
								HANDLE                            hThread,
								LPSTACKFRAME                      StackFrame,
								LPVOID                            ContextRecord,
								PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
								PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
								PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
								PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress );

typedef PVOID  (__stdcall *PFNSYMFUNCTABLEACCESS) (HANDLE  hProcess,
												   DWORD   AddrBase);

typedef DWORD (__stdcall *PFNSYMGETMODULEBASE)( IN  HANDLE  hProcess,
	  								    IN  DWORD   dwAddr );
 
typedef DWORD (__stdcall *PFNSYMGETOPTIONS)( VOID );

typedef DWORD (__stdcall *PFNSYMSETOPTIONS)( IN DWORD SymOptions );


DWORD GetModuleNameFromAddr( PVOID addr , LPTSTR szModName, DWORD dwLen);
DWORD GetShortModuleNameFromAddr( PVOID addr , LPTSTR szModName, DWORD dwLen);

void InitSymEng ( void ) ;
// Cleans up the symbol engine if needed.
void CleanupSymEng ( void ) ;

void GetCurrentContext(CONTEXT* pContext);
DWORD GetStackAddresses(CONTEXT* pContext, PVOID* addr, DWORD dwMaxAddr);
DWORD DumpAddressInfo( PVOID addr, LPTSTR szBuf, DWORD dwLen);

#ifndef VERIFY
	#ifdef _DEBUG
		#define VERIFY(f) assert(f)
	#else
		#define VERIFY(f) (f)
	#endif
#endif

#endif//#if defined(_DEBUG && _MSC_VER ) 

#endif

