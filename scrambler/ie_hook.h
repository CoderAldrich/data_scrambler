#ifndef __IE_HOOK_H__
#define __IE_HOOK_H__

#define NUMHOOKS 1
#define HOOK_INDEX_MOUSE 0

typedef struct _MYHOOKDATA   
{   
	INT32 nType;   
	HOOKPROC hkprc;   
	HHOOK hhook;   
} MYHOOKDATA;   

extern MYHOOKDATA myhookdata[NUMHOOKS]; 

LRESULT WINAPI init_hook_config(); 
LRESULT WINAPI set_hook( INT32 index ); 
LRESULT WINAPI reset_hook( INT32 index ); 

#define ERROR_NOT_INSTALLED (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x1000)

void WINAPI LookUpTheMessage(PMSG pmsg, TCHAR* szMsg); 

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam); 

#endif //__IE_HOOK_H__