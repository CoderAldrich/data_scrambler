#ifndef __CRT_MEM_LEAK_DBG_H__
#define __CRT_MEM_LEAK_DBG_H__

#ifdef _DEBUG_MEM_LEAKS

#ifndef _DEBUG_MEM_LEAKS_DEFINED
#define _DEBUG_MEM_LEAKS_DEFINED
#define CRTDBG_MAP_ALLOC 
#define _CRTDBG_MAP_ALLOC 

#include <stdlib.h> 
#include <crtdbg.h> 

#ifndef DBG_NORMAL_BLOCK
#define DBG_NORMAL_BLOCK new ( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif //DBG_NORMAL_BLOCK

#ifndef DEBUG_CLIENT_BLOCK 
#define DEBUG_CLIENT_BLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif //DEBUG_CLIENT_BLOCK 

//#define new DEBUG_CLIENTBLOCK 
#define new DBG_NORMAL_BLOCK

//#ifdef new
//#undef new
//#endif //new 

//#ifdef _DEBUG_MEM_LEAKS
//
//#ifdef DBG_NORMAL_BLOCK
//#define new DBG_NORMAL_BLOCK
//#endif //DBG_NORMAL_BLOCK
//
//#endif //_DEBUG_MEM_LEAKS
#endif //_DEBUG_MEM_LEAKS_DEFINED
#endif //_DEBUG_MEM_LEAKS

#endif //__CRT_MEM_LEAK_DBG_H__