#ifndef __PLUGIN_API_H__
#define  __PLUGIN_API_H__

typedef struct _DATA_INFO
{
	ULONG index; 
	WCHAR name[ MAX_PATH ]; 
	PVOID data; 
	ULONG size; 
} DATA_INFO, *PDATA_INFO; 

#define PLUGIN_DATA_HANDLER_NAME "collected_data_process"
typedef INT32 ( WINAPI* collected_data_process_func )( PVOID data, ULONG data_size ); 

#endif //__PLUGIN_API_H__