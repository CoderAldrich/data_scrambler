#include "stdafx.h"
#include "webbrowser.h"

ULONG browser_process_delay_time = DEFAULT_DELAY_PROCESS_PAGE_TIME; 
ULONG browser_page_load_time = DEFAULT_WAIT_WEB_PAGE_LOAD_TIME; 
ULONG hilight_delay_time = DEFAULT_HILIGHT_DELAY_TIME; 

LRESULT WINAPI _start_webbrowser( ULONG flags, ULONG *proc_id )
{
#define MAX_CMD_LINE 1024
	LRESULT ret = ERROR_SUCCESS; 
	ULONG _ret; 
	WCHAR module_file_name[ MAX_PATH ]; 
	WCHAR cmd_line[ MAX_CMD_LINE ]; 
	HRESULT hr; 

	do 
	{
		if( proc_id != NULL )
		{
			*proc_id = 0; 
		}

		_ret = GetModuleFileNameW( NULL, 
			module_file_name, 
			ARRAYSIZE( module_file_name ) ); 

		if( _ret <= 0 
			|| _ret >= ARRAYSIZE( module_file_name ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break;  
		}

		*cmd_line = L'\"'; 
		cmd_line[ 1 ] = L'\0'; 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), module_file_name ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#define QUOTE_SIGN L"\"" 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L" " ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), QUOTE_SIGN ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L" " ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L"-x" ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		{
			ULONG pid = GetCurrentProcessId(); 
			WCHAR text[ 20 ]; 

			hr = StringCchPrintfW( text, ARRAYSIZE( text ), L" %u", pid ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), text ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( flags & WEB_RROWSER_LOAD_PAGE_NO_DELAY )
			{
				hr = StringCchPrintfW( text, ARRAYSIZE( text ), L" %u", 0 ); 
			}
			else
			{
				hr = StringCchPrintfW( text, ARRAYSIZE( text ), L" %u", browser_process_delay_time ); 
			}

			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), text ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = StringCchPrintfW( text, ARRAYSIZE( text ), L" %u", browser_page_load_time ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), text ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		//hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L" " ); 
		//if( FAILED(hr))
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), QUOTE_SIGN ); 
		//if( FAILED(hr))
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), url ); 
		//if( FAILED(hr))
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), QUOTE_SIGN ); 
		//if( FAILED(hr))
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		if( flags & WEB_RROWSER_PROCESS_SYNCH )
		{
			ret = run_proc_sync( cmd_line ); 
		}
		else
		{
			ret = run_proc( cmd_line, proc_id ); 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI embed_web_browser_window(HWND sub_window, 
										HWND parent_window, 
										RECT sub_window_rect )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HWND old_parent; 
	
	_ret = MoveWindow( sub_window,sub_window_rect.left, sub_window_rect.top, sub_window_rect.right, sub_window_rect.bottom, TRUE ); 
	if( FALSE == _ret )
	{
		dbg_print( MSG_IMPORTANT | DBG_MSG_AND_ERROR_OUT, "move sub window failed\n" ); 
		//break; 
	}

	//SetWindowLong( sub_window, ); 
	old_parent = SetParent( sub_window, parent_window ); 

	return ret; 
}

typedef struct tagWNDINFO
{
	DWORD pid;
	HWND wnd; 
} WNDINFO, *LPWNDINFO;

//HWND find_inst_main_wnd( INT32 is_update )
//{
//	HWND wnd_find = NULL; 
//	LPCTSTR prop_name = NULL; 
//
//	if( is_update == TRUE )
//	{
//		prop_name = BITSAFE_UPDATE_INST_PROP_NAME; 
//	}
//	else
//	{
//		prop_name = BITSAFE_INST_PROP_NAME; 
//	}
//
//	wnd_find = ::GetWindow( ::GetDesktopWindow(), GW_CHILD );
//
//	while( ::IsWindow( wnd_find ) )
//	{
//		if( ::GetProp( wnd_find, prop_name ) )
//		{    
//			goto _return; 
//		}
//
//		wnd_find = ::GetWindow( wnd_find, GW_HWNDNEXT );
//	}  
//
//	wnd_find = NULL; 
//
//_return:
//	return wnd_find; 
//}

BOOL CALLBACK enum_window_proc(HWND hWnd, LPARAM lParam)
{
	BOOL ret = TRUE; 
	INT32 _ret; 
	DWORD dwProcessId;
	LPWNDINFO pInfo = (LPWNDINFO)lParam;
	WCHAR wnd_class_name[ MAX_PATH ]; 

	GetWindowThreadProcessId(hWnd, &dwProcessId);

	do 
	{
		if( ( dwProcessId != pInfo->pid ) )
		{
			break; 
		}

		_ret = GetClassName( hWnd, wnd_class_name, ARRAYSIZE( wnd_class_name ) ); 

		if( _ret <= 0 )
		{
			break; 
		}

		if( 0 != _wcsicmp( wnd_class_name, L"#32770" ) )
		{
			break; 
		}

		if( NULL == ::GetProp( hWnd, DATA_SCRAMBLER_MAIN_WINDOW_PROP_NAME ) )
		{
			break; 
		}

		//{
		//	WCHAR text[ MAX_PATH ]; 
		//	GetWindowTextW( hWnd, text, ARRAYSIZE( text ) ); 
		//	
		//	if( L'\0' == *text )
		//	{
		//		break; 
		//	}
		//}

		ret = FALSE; 
		pInfo->wnd = hWnd; 
	} while( FALSE );

	return ret;
}

HWND WINAPI find_web_browser_window( ULONG pid )
{
	LRESULT ret; 
	//HWND wnd; 

	WNDINFO info = { 0 }; 
	info.pid = pid; 

	ret = EnumWindows( enum_window_proc, ( LPARAM )&info ); 

	//wnd = info.wnd; 

	return info.wnd; 
}

LRESULT WINAPI get_html_action_global_config( ULONG *hilight_time, 
							   ULONG *web_page_load_time, 
							   ULONG *process_delay_time )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG cc_ret_len; 
	WCHAR module_file_name[ MAX_PATH ]; 
	HRESULT hr; 
	UINT _time; 

	do
	{
#ifdef FIXED_WEB_PAGE_LOAD_TIME 
		if( NULL != web_page_load_time )
		{
			*web_page_load_time = browser_page_load_time; 
		}

		if( NULL != process_delay_time )
		{
			*process_delay_time = browser_process_delay_time; 
		}

		if( NULL != hilight_time )
		{
			*hilight_time = hilight_delay_time; 
		}
		break; 
#endif //FIXED_WEB_PAGE_LOAD_TIME 

		ret = get_app_path( module_file_name, ARRAYSIZE( module_file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( module_file_name, ARRAYSIZE( module_file_name ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL != hilight_time )
		{
			_time = GetPrivateProfileIntW( L"MAIN", 
				L"HILIGHT_TIME", 
				DEFAULT_HILIGHT_DELAY_TIME, 
				module_file_name ); 

			if( _time > MAX_HILIGHT_TIME )
			{
				_time = MAX_HILIGHT_TIME; 
			}
			else if( _time < MIN_HILIGHT_TIME )
			{
				_time = MIN_HILIGHT_TIME; 
			}

			*hilight_time = _time; 
		}

		if( NULL != web_page_load_time )
		{
			_time = GetPrivateProfileIntW( L"MAIN", 
				L"WAIT_LOAD_PAGE_TIME", 
				DEFAULT_WAIT_WEB_PAGE_LOAD_TIME, 
				module_file_name ); 


			if( _time > MAX_WEB_PAGE_LOAD_TIME )
			{
				_time = MAX_WEB_PAGE_LOAD_TIME; 
			}
			else if( _time < MIN_WEB_PAGE_LOAD_TIME )
			{
				_time = MIN_WEB_PAGE_LOAD_TIME; 
			}

			*web_page_load_time = _time; 
		}

		if( NULL != process_delay_time )
		{
			_time = GetPrivateProfileIntW( L"MAIN", 
				L"PROCESS_DELAY_TIME", 
				DEFAULT_DELAY_PROCESS_PAGE_TIME, 
				module_file_name ); 

			if( _time > MAX_PROCESS_DELAY_TIME )
			{
				_time = MAX_PROCESS_DELAY_TIME ; 
			}
			else if( _time < MIN_PROCESS_DELAY_TIME )
			{
				_time = MIN_PROCESS_DELAY_TIME; 
			}

			*process_delay_time = _time; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI set_html_action_global_config( ULONG hilight_time, 
									  ULONG web_page_load_time, 
									  ULONG process_delay_time )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG cc_ret_len; 
	WCHAR module_file_name[ MAX_PATH ]; 
#define MAX_NUMBER_TEXT_LEN 32
	WCHAR number_text[ MAX_NUMBER_TEXT_LEN ]; 
	HRESULT hr; 

	do
	{
#ifdef FIXED_WEB_PAGE_LOAD_TIME 
		hilight_delay_time = hilight_time; 
		browser_page_load_time = web_page_load_time; 
		browser_process_delay_time = process_delay_time; 
		break; 
#endif //FIXED_WEB_PAGE_LOAD_TIME 

		ret = get_app_path( module_file_name, ARRAYSIZE( module_file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( module_file_name, ARRAYSIZE( module_file_name ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		do 
		{
			if( hilight_time == INVALID_TIME_VALUE )
			{
				break; 
			}
			
			if( hilight_time > MAX_HILIGHT_TIME )
			{
				hilight_time = MAX_HILIGHT_TIME; 
			}

			if( hilight_time < MIN_HILIGHT_TIME )
			{
				hilight_time = MIN_HILIGHT_TIME; 
			}

			hilight_delay_time = hilight_time; 

			hr = StringCchPrintfW( number_text, ARRAYSIZE( number_text ), L"%u", hilight_time ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_ret  = WritePrivateProfileStringW( L"MAIN", 
				L"HILIGHT_TIME", 
				number_text, 
				module_file_name ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		}while( FALSE );

		do 
		{
			if( web_page_load_time == INVALID_TIME_VALUE )
			{
				break; 
			}

			if( web_page_load_time > MAX_WEB_PAGE_LOAD_TIME )
			{
				web_page_load_time  = MAX_WEB_PAGE_LOAD_TIME; 
			}

			if( web_page_load_time < MIN_WEB_PAGE_LOAD_TIME )
			{
				web_page_load_time = MIN_WEB_PAGE_LOAD_TIME; 
			}

			browser_page_load_time = web_page_load_time; 

			hr = StringCchPrintfW( number_text, ARRAYSIZE( number_text ), L"%u", web_page_load_time ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_ret  = WritePrivateProfileStringW( L"MAIN", 
				L"WAIT_LOAD_PAGE_TIME", 
				number_text, 
				module_file_name ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

		}while( FALSE );

		do 
		{
			if( process_delay_time == INVALID_TIME_VALUE )
			{
				break; 
			}

			if( process_delay_time > MAX_PROCESS_DELAY_TIME )
			{
				process_delay_time = MAX_PROCESS_DELAY_TIME ; 
			}

			if( process_delay_time < MIN_PROCESS_DELAY_TIME )
			{
				process_delay_time = MIN_PROCESS_DELAY_TIME; 
			}

			browser_process_delay_time = process_delay_time; 

			hr = StringCchPrintfW( number_text, ARRAYSIZE( number_text ), L"%u", process_delay_time ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_ret  = WritePrivateProfileStringW( L"MAIN", 
				L"PROCESS_DELAY_TIME", 
				number_text, 
				module_file_name ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		}while( FALSE ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI _set_html_action_global_config( ULONG hilight_time, 
									  ULONG web_page_load_time, 
									  ULONG process_delay_time )
{
	LRESULT ret = ERROR_SUCCESS; 

	do
	{
		do 
		{
			if( hilight_time == INVALID_TIME_VALUE )
			{
				break; 
			}

			if( hilight_time > MAX_HILIGHT_TIME )
			{
				hilight_time = MAX_HILIGHT_TIME; 
			}

			if( hilight_time < MIN_HILIGHT_TIME )
			{
				hilight_time = MIN_HILIGHT_TIME; 
			}

			hilight_delay_time = hilight_time; 
		}while( FALSE );

		do 
		{
			if( web_page_load_time == INVALID_TIME_VALUE )
			{
				break; 
			}

			if( web_page_load_time > MAX_WEB_PAGE_LOAD_TIME )
			{
				web_page_load_time  = MAX_WEB_PAGE_LOAD_TIME; 
			}

			if( web_page_load_time < MIN_WEB_PAGE_LOAD_TIME )
			{
				web_page_load_time  = MIN_WEB_PAGE_LOAD_TIME; 
			}

			browser_page_load_time = web_page_load_time; 

		}while( FALSE );

		do 
		{
			if( process_delay_time == INVALID_TIME_VALUE )
			{
				break; 
			}

			if( process_delay_time > MAX_PROCESS_DELAY_TIME )
			{
				process_delay_time = MAX_PROCESS_DELAY_TIME ; 
			}
			
			if( process_delay_time < MIN_PROCESS_DELAY_TIME )
			{
				process_delay_time = MIN_PROCESS_DELAY_TIME ; 
			}

			browser_process_delay_time = process_delay_time; 
		}while( FALSE ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI save_html_action_global_config()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = set_html_action_global_config( hilight_delay_time, 
			browser_page_load_time, 
			browser_process_delay_time ); 
	}while( FALSE ); 
	
	return ret; 
}