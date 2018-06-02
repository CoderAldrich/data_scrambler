#include "common_func.h"
#include "pipe_line.h"

LRESULT close_pipe( HANDLE pipe )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	if( NULL != pipe )
	{
		CloseHandle( pipe ); 
	}
	else
	{
		ret = ERROR_INVALID_PARAMETER; 
	}

	return ret; 
}


LRESULT create_name_pipe( LPCTSTR pipe_name, HANDLE *pipe_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE _pipe; 

	ASSERT( pipe_out != NULL ); 
	
	*pipe_out = NULL; 

	_pipe = CreateNamedPipe( pipe_name, 
		PIPE_ACCESS_DUPLEX, 
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 
		1, 
		0, 
		0, 
		1000, 
		NULL ); 
	
	if( _pipe == INVALID_HANDLE_VALUE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create name pipe failed \n" ) ); 
		goto _return; 
	}
	else
	{
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "create name pipe successfully\n" ) ); 
	}

	*pipe_out = _pipe; 

_return:
	return ret; 
}

UINT accept_name_pipe_client_sync( HANDLE _pipe )
{
	LRESULT ret = ERROR_SUCCESS;
	INT32 _ret; 

	_ret = ConnectNamedPipe( _pipe, NULL ); 

	if( _ret == FALSE ) 
	{
		SAFE_SET_ERROR_CODE( ret ); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT disconnect_name_pipe_client( HANDLE pipe )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	_ret = DisconnectNamedPipe( pipe ) ; 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "disconnect the pipe failed\n" ) ); 
		goto _return; 
	}

_return:
	return ret; 
}

LRESULT write_to_name_pipe( LPCWSTR pipe_name, CHAR* data, ULONG data_len, ULONG *ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HANDLE pipe; 

	ASSERT( pipe_name != NULL ); 
	ASSERT( ret_len != NULL ); 

	_ret = WaitNamedPipe( pipe_name, NMPWAIT_WAIT_FOREVER ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}
	
	pipe = CreateFile( pipe_name, 
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL ); 

	if( pipe == INVALID_HANDLE_VALUE )
	{

		ret = GetLastError(); 
		goto _return; 
	} 

	_ret = WriteFile( pipe, data, data_len, ret_len, NULL ); 
	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		goto _return; 
	}

_return: 
	if( pipe != NULL )
	{
		CloseHandle( pipe ); // 关闭管道句柄 
	}

	return ret; 
}

#if 0
LRESULT create_non_name_pipe()
{
	LRESULT ret = ERROR_SUCCESS; 
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char ReadBuf[ 100 ];
	DWORD ReadNum;
	HANDLE hRead; // 管道读句柄
	HANDLE hWrite; // 管道写句柄
	HANDLE hTemp; 
	BOOL bRet = CreatePipe(&hRead, &hWrite, NULL, 0); // 创建匿名管道
	if (bRet == TRUE)
		printf("成功创建匿名管道!\n");
	else
		printf("创建匿名管道失败,错误代码:%d\n", GetLastError());
	// 得到本进程的当前标准输出
	hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
	// 设置标准输出到匿名管道
	SetStdHandle(STD_OUTPUT_HANDLE, hWrite);
	GetStartupInfo(&si); // 获取本进程的STARTUPINFO结构信息
	bRet = CreateProcess(NULL, "Client.exe", NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi); // 创建子进程
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp); // 恢复本进程的标准输出
	
	if (bRet == TRUE) // 输入信息
		printf("成功创建子进程!\n");
	else
		printf("创建子进程失败,错误代码:%d\n", GetLastError());
	
	CloseHandle(hWrite); // 关闭写句柄
	
	// 读管道直至管道关闭
	while (ReadFile(hRead, ReadBuf, 100, &ReadNum, NULL))
	{
		ReadBuf[ReadNum] = '\0';
		printf("从管道[%s]读取%d字节数据\n", ReadBuf, ReadNum);
	}
	
	if( GetLastError() == ERROR_BROKEN_PIPE ) // 输出信息
		printf("管道被子进程关闭\n");
	else
		printf("读数据错误,错误代码:%d\n", GetLastError());
	return ret; 
}
#endif //0

LRESULT WINAPI init_pipe_point( pipe_ipc_point *point, LPCWSTR pipe_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG name_len; 

	ASSERT( point != NULL ); 
	
	ret = init_mutex( &point->lock ); 
	if( ERROR_SUCCESS != ret )
	{ 
		goto _return; 
	}

	if( pipe_name != NULL )
	{
		name_len = wcslen( pipe_name ); 
		point->pipe_name = malloc( ( name_len + 1 ) * sizeof( WCHAR ) ); 
		if( point->pipe_name == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		wcscpy( point->pipe_name, pipe_name ); 
	}
	else
	{
		point->pipe_name = NULL; 

	}

	point->pipe = NULL; 

_return:
	return ret; 
}

LRESULT __stdcall _exec_cmd_from_pipe( pipe_ipc_point *point, 
									  BYTE* data, 
									  ULONG data_len,
									  VOID** data_ret,
									  ULONG* data_ret_len
									  )

{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG data_send_len; 
	CHAR *send_buf = NULL; 
	ULONG offset; 
	CHAR *_data_ret = NULL; 
	ULONG _data_ret_len; 
	INT32 held_lock = FALSE; 

	if( NULL == point 
		|| data == NULL 
		|| data_len == 0 ) 
	{
		ret = ERROR_INVALID_PARAMETER; ;
		goto _return; 
	}

	if( ( data == NULL 
		&& data_len != 0 ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	ASSERT( data_ret != NULL ? data_ret_len != 0 : data_ret_len == 0 ); 

	if( data_ret != NULL )
	{
		*data_ret = NULL;
		*data_ret_len = 0;
	}

	if( NULL == point->pipe ) 
	{
		ret= check_name_pipe_exist( point->pipe_name );
		if( ERROR_SUCCESS != ret )
		{
			goto _return; 
		}
	}
	else
	{
		ret = check_named_pipe_server_exist( point->pipe ); 
		if( ERROR_SUCCESS != ret )
		{
			goto _return; 
		}
	}

	lock_mutex( point->lock ); 
	held_lock = TRUE; 
		
	if( NULL == point->pipe ) 
	{
		ret = connect_to_pipe( point->pipe_name, &point->pipe ); 

		if( ERROR_SUCCESS != ret ) 
		{
			goto _return; 
		}
	}

	data_send_len = data_len; 

	send_buf = ( CHAR* )malloc( data_send_len + sizeof( ULONG ) );
	if( NULL == send_buf )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	memcpy( send_buf, &data_send_len, sizeof( ULONG ) ); 

	offset = sizeof( ULONG ); 
	memcpy( send_buf + offset, data, data_len ); 

	ret = write_pipe_sync( point, send_buf, data_send_len + sizeof( ULONG ) ); 
	if( ERROR_SUCCESS != ret )
	{
		goto _return; 
	}

	/**
	* 只要 WritePipeSynchronously 成功，即成功地发送出了数据，
	* 则认为CallProductPipe成功，所以接着等待返回数据的到来
	*/

	//4.... 接收客户端的返回数据

	if( data_ret == NULL )
	{
		goto _return; 
	}

	// 先读取返回数据的长度
	ret = read_pipe_sync( point, ( CHAR* )&_data_ret_len, sizeof( ULONG ) );
	if( ERROR_SUCCESS != ret )
	{
		goto _return; 
	}

	if ( 0 == _data_ret_len)
	{
		// 将out参数置空，释放动态分配的发送缓冲区，关闭管道句柄
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	// 再读取返回数据
	_data_ret = ( CHAR* )malloc( _data_ret_len );
	if( NULL == _data_ret )
	{
		ret = ERROR_NOT_ENOUGH_MEMORY; 
		goto _return; 
	}

	ret = read_pipe_sync(
		point, 
		( CHAR* )_data_ret, 
		_data_ret_len ); 

	if( ERROR_SUCCESS != ret )
	{
		goto _return; 
	}

	*data_ret = _data_ret; 
	*data_ret_len = _data_ret_len; 

_return:
	if( held_lock == TRUE )
	{
		unlock_mutex( point->lock ); 
	}

	if( ret != ERROR_SUCCESS )
	{
		if( _data_ret != NULL )
		{
			free( _data_ret ); 
		}

		if( send_buf != NULL )
		{
			free( send_buf ); 
		}

		if( point->pipe != NULL )
		{
			CloseHandle( point->pipe );
			point->pipe = NULL;
		}
	}
	else
	{
		ASSERT( send_buf != NULL ); 
		free( send_buf ); 
	}

	return ret;
}

#define KXEPIPE_WAIT_PIPE_INTERVAL 100	// WaitNamedPipe 时间间隔（毫秒）
#define KXEPIPE_MAX_CONNECT_TIMES 5		// 尝试连接指定的管道服务端的最大次数

/**
* @brief 建立与服务端命名管道服务器的连接
* @return  0 成功，其他为失败错误码
*/
LRESULT __stdcall connect_to_pipe( LPCWSTR pipe_name, HANDLE *pipe_out )
{
	LRESULT ret; 
	HANDLE _pipe; 
	INT32 retry_time = 0; 

	ASSERT( pipe_name != NULL ); 
	ASSERT( pipe_out != NULL ); 

	*pipe_out = NULL; 

	ret = check_name_pipe_exist( pipe_name ); 
	if( ERROR_SUCCESS != ret )
	{
		goto _return; 
	}

	do 
	{
		_pipe = CreateFile( pipe_name,		// 命名管道名
			GENERIC_READ | GENERIC_WRITE,	// 管道读写模式：双向
			0, 
			( LPSECURITY_ATTRIBUTES )NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL |
			SECURITY_SQOS_PRESENT |	// 允许服务器在本地系统上识别和模仿该客户
			SECURITY_IMPERSONATION,
			( HANDLE )NULL 
			); 

		if( INVALID_HANDLE_VALUE != _pipe )
		{
			ret = GetLastError(); 
			break;	// 成功，则 nCount 一定小于 KXEPIPE_MAX_CONNECT_TIMES
		}
		else
		{
			retry_time ++; 
			Sleep( 50 ); 
		}

	} while( retry_time != KXEPIPE_MAX_CONNECT_TIMES ); 

	if( retry_time == KXEPIPE_MAX_CONNECT_TIMES )
	{
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}
		goto _return; 
	}

	*pipe_out = _pipe; 

_return:
	return ret; 
}

/**
* @brief 向管道同步地写入指定数目的数据
* @param[in] pszSendBuffer 发送缓冲区首址
* @param[in] dwSendDataLength 要发送的数据长度
* @return  0 成功，其他为错误码
*/
LRESULT __stdcall write_pipe_sync( 
								  pipe_ipc_point *point, 
								  const CHAR* data, 
								  const ULONG data_len
								  )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG writed_once = 0;
	ULONG transfer_point = 0;
	INT32 _ret = FALSE;

	ASSERT( point != NULL ); 
	ASSERT( NULL != point->pipe );

	ASSERT( point != NULL ); 
	ASSERT( point->pipe != NULL ); 

	while( transfer_point < data_len )
	{
		writed_once = 0;
		_ret = WriteFile( point->pipe, 
			( LPCVOID )( data + transfer_point ),	// 每次循环都要调整文件指针
			data_len - transfer_point,
			&writed_once,
			NULL	// 同步方式 
			);

		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}
		else
		{
			transfer_point += writed_once; 
		}
	}

_return:
	return ret;
}

/**
* @brief 从管道同步地读取指定数目的数据
* @param[in, out] pszReceiveBuffer 接收缓冲区首址
* @param[in] dwReceiveDataLength 要接收的数据长度
* @return  0 成功，其他为错误码
*/
LRESULT __stdcall read_pipe_sync(
								 pipe_ipc_point *point, 
								 CHAR* data, 
								 const ULONG data_len )
{
	LRESULT ret = ERROR_SUCCESS;
	DWORD readed_once = 0;
	DWORD transfer_point = 0;
	BOOL _ret; 

	ASSERT( point != NULL ); 
	ASSERT( point->pipe != NULL ); 

	while( data_len != transfer_point)
	{
		readed_once = 0; 

		_ret = ReadFile( 
			point->pipe,
			( LPVOID )( data + transfer_point ),
			data_len - transfer_point,
			&readed_once,
			NULL
			);
		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			goto _return; 
		}
		else
		{
			transfer_point += readed_once; 
		}
	}

_return:
	return ret;
}
