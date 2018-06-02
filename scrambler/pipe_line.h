#ifndef __PIPE_LINE_H__
#define __PIPE_LINE_H__

#define RASBASE 600
#define ERROR_BUFFER_TOO_SMALL               (RASBASE+3)

typedef struct _pipe_ipc_point
{
	LPWSTR pipe_name;
	HANDLE pipe; 
	HANDLE lock; 
} pipe_ipc_point, *ppipe_ipc_point; 

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

INLINE LRESULT WINAPI check_named_pipe_server_exist( HANDLE pipe )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG server_process_id; 

	do 
	{
		_ret = GetNamedPipeServerProcessId( pipe, &server_process_id ); 

		//GetNamedPipeAttribute()

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT WINAPI check_named_pipe_client_exist( HANDLE pipe )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG server_process_id; 

	do 
	{
		_ret = GetNamedPipeClientProcessId( pipe, &server_process_id ); 

		//GetNamedPipeAttribute()

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT WINAPI check_name_pipe_exist( LPCWSTR pipe_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret = FALSE;

	_ret = WaitNamedPipe(
		pipe_name, 
		NMPWAIT_USE_DEFAULT_WAIT
		);

	if( FALSE == _ret )
	{
		ret = GetLastError(); 
		if( ERROR_PIPE_BUSY != ret )
		{
			goto _return; 
		}
		else 
		{
			ret = ERROR_SUCCESS; 
		}
	}

_return:
	return ret; 
}

LRESULT create_name_pipe( LPCTSTR pipe_name, HANDLE *pipe_out ); 
LRESULT close_pipe( HANDLE pipe ); 
LRESULT accept_name_pipe_client_sync( HANDLE _pipe ); 
LRESULT disconnect_name_pipe_client( HANDLE pipe ); 
LRESULT write_to_name_pipe( HANDLE pipe, CHAR* data, ULONG data_len, ULONG *ret_len ); 
LRESULT create_non_name_pipe(); 

LRESULT WINAPI init_pipe_point( pipe_ipc_point *point, LPCWSTR pipe_name ); 

INLINE LRESULT WINAPI uninit_pipe_point( pipe_ipc_point *point )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( point != NULL ); 
	if( point->pipe != NULL )
	{
		CloseHandle( point->pipe ); 
	}
	
	if( point->lock != NULL )
	{
		uninit_mutex( point->lock ); 
	}

	if( NULL != point->pipe_name )
	{
		free( point->pipe_name ); 
	}

	point->pipe = NULL; 
	point->lock = NULL; 
	point->pipe_name = NULL; 
	
	return ret; 
}

LRESULT __stdcall _exec_cmd_from_pipe( pipe_ipc_point *point, 
									  BYTE* data, 
									  ULONG data_len,
									  VOID** data_ret,
									  ULONG* data_ret_len
									  ); 


/**
* @brief 释放通过此类分配的内存
* @param[in] pBuffer 需要释放的内存地址
* @return 0 成功，其他为失败错误码
*/
INLINE LRESULT __stdcall release_pipe_buf(
	PVOID *data
	)
{
	free( *data ); ;
	*data= NULL;
	return ERROR_SUCCESS;
}

/**
* @brief 建立与服务端命名管道服务器的连接
* @return  0 成功，其他为失败错误码
*/
LRESULT __stdcall connect_to_pipe( LPCWSTR pipe_name, HANDLE *pipe_out ); 


LRESULT __stdcall write_pipe_sync( 
								  pipe_ipc_point *point, 
								  const CHAR* data, 
								  const ULONG data_len
								  ); 
LRESULT __stdcall read_pipe_sync( 
								 pipe_ipc_point *point, 
								 CHAR* data, 
								 const ULONG data_len ); 

INLINE LRESULT exec_cmd_from_pipe(
								  pipe_ipc_point *point, 
								  BYTE* param,
								  ULONG param_size,
								  CHAR **data_ret, 
								  ULONG *data_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	ASSERT( point != NULL ); 
	ASSERT( point->pipe_name != NULL ); 

	ret = check_name_pipe_exist( point->pipe_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = _exec_cmd_from_pipe( point, 
		param,
		param_size,
		( VOID ** )data_ret,
		data_ret_len );

	if( ret != ERROR_SUCCESS ) 		{
		goto _return; 
	}

_return: 
#ifdef _DEBUG
	if( ret != ERROR_SUCCESS )
	{
		if( data_ret != NULL )
		{
			ASSERT( *data_ret != NULL ); 
		}
	}
#endif //_DEBUG
	
	return ret; 
}

#ifdef __cplusplus
}
#endif //__cplusplus


#endif //__PIPE_LINE_H__