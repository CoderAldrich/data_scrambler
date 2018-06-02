#ifndef __IE_CACHE_H__
#define __IE_CACHE_H__

INT32 WINAPI EnumerateCacheOld(); 

typedef LRESULT ( CALLBACK* on_data_readed )( PVOID data, ULONG data_size, PVOID context ); 

LRESULT WINAPI get_ie_cache_data( LPCWSTR url, on_data_readed callback_func, PVOID context ); 

#endif //__IE_CACHE_H__

