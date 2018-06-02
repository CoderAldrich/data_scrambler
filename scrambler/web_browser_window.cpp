// web_browser_window.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "web_browser_window.h"
#include "mshtml_addition.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "html_element_prop_dlg.h"
#include "external_link.h"
#include "data_analyze.h"
#include "webbrowser.h"
#include "pipe_line.h"
#include "work_job.h"
#include "ie_auto.h"
#include "data_scramble.pb.h"

#define WM_HTML_ELEMENTS_HILIGHT ( WM_USER + 1013 ) 

using namespace data_scrabmle; 

ULONG debug_mode = 0; 

extern ULONG hilight_delay_time; 

//DATA_STORE data_store; 
web_browser_window *g_web_browser_window; 
//HTML_LIST_ELEMENT_SCRAMBLE_INFO scramble_work_config; 

typedef struct _HTML_ACTION_WORK
{
	BOOLEAN page_loaded; 
	wstring locate_url; 
	HTML_ELEMENT_ACTION action; 
	vector<wstring> response; 
	HANDLE complete_notifier; 
	LRESULT result; 
} HTML_ACTION_WORK, *PHTML_ACTION_WORK; 

HTML_ACTION_WORK html_action_work; 

//scramble_mode scramble_work_mode = data_scrabmle::FRAME_WEB_PAGE_SCRAMBLE; 

//#define WM_WEB_PAGE_ACTIONS ( WM_USER + 1001 )
#define WM_HTML_ACTION ( WM_USER + 1002 )

ULONG g_manager_pid = 0; 

thread_manage data_scramble_worker; 

//maybe url is error.
LRESULT WINAPI get_html_element_not_found_error( LRESULT error_code )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( FALSE == html_action_work.page_loaded )
		{
			ret = ERROR_WEB_PAGE_IS_NOT_LOADED; 
		}
		else
		{
			ret = error_code; 
		}

	}while( FALSE ); 
	return ret; 
}

ULONG WINAPI data_scramble_work( PVOID param ); 

LRESULT WINAPI scramble_whole_web_page_data( IHTMLDocument2 *html_doc )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( html_doc != NULL ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI execute_web_broswer_manage_command( PVOID data, ULONG data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	wstring _response; 
	html_element_action_response response; 

	do 
	{
		ASSERT( NULL != data ); 
		ASSERT( 0 != data_size ); 

		_ret = response.ParseFromArray( data, data_size ); 
		if( false == _ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( response.response_size() == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		dbg_print( MSG_IMPORTANT, "web browser client response: %s\n", response.response(0).c_str() ); 

		if( 0 == strcmp( response.response(0).c_str(), HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A ) )
		{
			ExitProcess( *( ULONG* )data ); 
			break; 
		}
		else if( 0 == strcmp( response.response(0).c_str(), HTML_ELEMENT_ACTION_RESPONSE_CONTINUE_A ) )
		{
		}
		else
		{
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

IMPLEMENT_DYNAMIC(web_browser_window, CDialog)

web_browser_window::web_browser_window(CWnd* pParent /*=NULL*/)
	: CDialog(web_browser_window::IDD, pParent)
{
	//HRESULT hr; 

	//site = NULL; 

	output_delay_time = DEFAULT_DELAY_PROCESS_PAGE_TIME; 
	manager_pid = 0; 
	//hr = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED ); 

	//if( FAILED( hr ) )
	//{

	//}
	//else
	//{
	//	com_inited = TRUE; 
	//}

	ui_handler = new doc_host_ui_handler(); 
	element_behavior = new html_element_behavior(); 
}

web_browser_window::~web_browser_window()
{
	if( TRUE == com_inited )
	{
		CoUninitialize(); 
	}

	SAFE_RELEASE_IF( ui_handler ); 
}

void web_browser_window::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_WEB_BROWSER_CONTROL, m_WebBrowser); 
}

LRESULT web_browser_window::set_work_mode( LPCWSTR work_mode ) 
{
	WCHAR *end_text; 

	ASSERT( NULL != work_mode ); 

	debug_mode = wcstoul( work_mode, &end_text, 0 ); 

	return ERROR_SUCCESS; 
}

INT_PTR web_browser_window::create_dialog()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)

	BOOL bEnableParent = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	CWnd* pMainWnd = NULL;
	BOOL bEnableMainWnd = FALSE;
#endif
	HWND hWndParent = NULL; 

	if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd = AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
			//
			// We are hosted by non-MFC container
			// 
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{
		// create modeless dialog
		if (CreateDlgIndirect(lpDialogTemplate,
			CWnd::FromHandle(hWndParent), hInst))
		{
			//if (m_nFlags & WF_CONTINUEMODAL)
			//{
			//	// enter modal loop
			//	DWORD dwFlags = MLF_SHOWONIDLE;
			//	if (GetStyle() & DS_NOIDLEMSG)
			//		dwFlags |= MLF_NOIDLEMSG;
			//	VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			//}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
				SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		//DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OLE_SUPPORT
		if (bEnableMainWnd)
			pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	//// destroy modal window
	//DestroyWindow();
	//PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	ShowWindow( SW_HIDE ); 

	return m_nModalResult;
}

LRESULT WINAPI parse_html_actions( LPVOID data, 
								  ULONG data_size, 
								  HTML_ELEMENT_ACTION *action_out, 
								  wstring &locate_url )
{
	LRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	wstring unicode_text; 
	html_element_action action; 

	do 
	{
		//for( ; ; )
		//{
		//	Sleep( 10 ); 
		//}

		ASSERT( NULL != action_out ); 

		_ret = action.ParseFromArray( data, data_size ); 
		if( false == _ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( false == action.IsInitialized() )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//if( false == action.has_action() )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		if( action.action().length() == 0 )
		{
			action_out->action = L""; 
		}
		else
		{
			ret = utf8_to_unicode_ex( action.action().c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 			
			}

			action_out->action = unicode_text; 
		}
		
		//if( false == action.has_action() )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		if( action.xpath().length() == 0 )
		{
			if( ERROR_SUCCESS != check_no_html_element_action( action_out ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			action_out->xpath = L""; 
		}
		else
		{
			ret = utf8_to_unicode_ex( action.xpath().c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 			
			}

			action_out->xpath = unicode_text; 
		}

		if( action.param().length() == 0 )
		{
			action_out->param = L""; 
			//ret = ERROR_INVALID_PARAMETER; 
			//break; 
		}
		else
		{
			ret = utf8_to_unicode_ex( action.param().c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 			
			}

			action_out->param = unicode_text; 
		}

		if( action.url().length() == 0 )
		{
			locate_url = L""; 
		}
		else
		{
			ret = utf8_to_unicode_ex( action.url().c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 			
			}

			locate_url = unicode_text; 
		}

		if( true == action.has_input() 
			&& action.input().length() != 0 )
		{
			if( 0 != strcmp( action.action().c_str(), HTML_ELEMENT_ACTION_INPUT_A ) )
			{
				ASSERT( FALSE ); 
                action_out->outputs.clear();
                action_out->outputs.push_back(unicode_text); 
			}
			else
			{
				ret = utf8_to_unicode_ex( action.input().c_str(), unicode_text ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 			
				}

                action_out->outputs.clear();
                action_out->outputs.push_back(unicode_text);
			}
		}
		else
		{
            action_out->outputs.clear(); 
		}

		if( action.id().length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		{
			CHAR *text_end; 
			action_out->id = strtoul( action.id().c_str(), &text_end, 0 ); 
		}

	} while ( FALSE );

	return ret; 
}

//LRESULT WINAPI parse_scramble_command( LPVOID data, 
//									  ULONG data_size, 
//									  HTML_LIST_ELEMENT_SCRAMBLE_INFO *config_out )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	bool _ret; 
//	scramble_command scramble_config; 
//
//	do 
//	{
//		//for( ; ; )
//		//{
//		//	Sleep( 10 ); 
//		//}
//
//		_ret = scramble_config.ParseFromArray( data, data_size ); 
//		if( false == _ret )
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		if( false == scramble_mode_IsValid( scramble_config.mode() ) )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		switch( scramble_config.mode() )
//		{
//		case data_scrabmle::FRAME_WEB_PAGE_SCRAMBLE: 
//			{
//				ASSERT( scramble_config.frame().has_url() == true ); 
//
//				ret = utf8_to_unicode_ex( scramble_config.frame().url().c_str(), 
//					config_out->url ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//
//				ret = utf8_to_unicode_ex( scramble_config.frame().links_xpath().c_str(), 
//					config_out->link_list_xpath ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//
//				ret = utf8_to_unicode_ex( scramble_config.frame().next_xpath().c_str(), 
//					config_out->next_link_xpath ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//			}
//			break; 
//		case data_scrabmle::CONTENT_WEB_PAGE_SCRAMBLE: 
//			{
//				ULONG field_count; 
//				ULONG i; 
//				wstring text; 
//				HTML_ELEMENT_CONTENT *element_content = NULL; 
//
//				do 
//				{
//					ret = uninit_page_content_info( &config_out->page_content ); 
//					if( ret != ERROR_SUCCESS )
//					{
//					}
//
//					ret = utf8_to_unicode_ex( scramble_config.content().url().c_str(), 
//						text ); 
//					
//					if( ret != ERROR_SUCCESS )
//					{
//						break; 
//					}
//
//					config_out->links.push_back( text ); 
//
//					field_count = scramble_config.content().fields_size(); 
//
//					for( i = 0; i < field_count; i ++ )
//					{
//						element_content = new HTML_ELEMENT_CONTENT(); 
//						if( element_content == NULL )
//						{
//							break; 
//						}
//
//						ret = utf8_to_unicode_ex( scramble_config.content().fields( i ).name().c_str(), 
//							element_content->name ); 
//
//						if( ret != ERROR_SUCCESS )
//						{
//							break; 
//						}
//
//						ret = utf8_to_unicode_ex( scramble_config.content().fields( i ).xpath().c_str(), 
//							element_content->xpath ); 
//
//						if( ret != ERROR_SUCCESS )
//						{
//							break; 
//						}
//
//						config_out->page_content.element_contents.push_back( element_content ); 
//						element_content = NULL; 
//					}
//				} while ( FALSE ); 
//
//				if( element_content != NULL )
//				{
//					delete element_content; 
//				}
//			}
//			break; 
//		case data_scrabmle::CONTENT_WEB_PAGE_HILIGHT: 
//			{
//				ASSERT( scramble_config.frame().has_url() == true ); 
//
//				scramble_work_mode = data_scrabmle::CONTENT_WEB_PAGE_HILIGHT; 
//
//				ret = utf8_to_unicode_ex( scramble_config.frame().url().c_str(), 
//					config_out->url ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//
//				ret = utf8_to_unicode_ex( scramble_config.frame().links_xpath().c_str(), 
//					config_out->link_list_xpath ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//			}
//			break; 
//		default:
//			break; 
//		}
//
//	} while ( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI evaluate_worker_state()
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	ULONG wait_ret; 

	do 
	{
		wait_ret = WaitForSingleObject( data_scramble_worker.thread_handle, 0 ); 
	
		if( wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED_0 
			&& wait_ret != WAIT_FAILED )
		{
			break; 
		}

		ASSERT( wait_ret != WAIT_FAILED ); 

		do 
		{
			if(NULL == job_worker.worker)
			{
				break; 
			}

			ret = check_job_worker_exit( &job_worker ); 
			if( ret == ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		REMOTE_ERROR_DEBUG(); 
		ExitProcess( 0x000000ed ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI init_html_action_work()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		html_action_work.page_loaded = FALSE; 
		html_action_work.complete_notifier = CreateEvent( NULL, FALSE, FALSE, NULL ); 
	
		if( NULL == html_action_work.complete_notifier )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = create_work_thread( &data_scramble_worker, data_scramble_work, NULL, NULL ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	} while ( FALSE );

	return ret; 
}

ULONG WINAPI data_scramble_work( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	LRESULT __ret; 
	HRESULT hr; 
	ULONG wait_ret; 

	PVOID result_data; 
	ULONG result_data_size; 

	WCHAR pipe_name[ MAX_PATH ]; 
	pipe_ipc_point pipe_point = { 0 }; 
	BOOLEAN pipe_inited = FALSE; 

	PVOID data_in = NULL; 
	ULONG data_in_size; 

	thread_manage *thread_param; 

	ASSERT( NULL != param ); 

	thread_param = ( thread_manage* )param; 

	do 
	{
		hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, GetCurrentProcessId() ); 
		if( FAILED(hr))
		{

		}

		//for( ; ; ) Sleep( 0 ); 

		{
			ULONG wait_time; 
			wait_time = 0; 
			for( ; ; )
			{
#define WAIT_SERVER_PIPE_CREATE_TIME 1000 
#define MAX_WAIT_SERVER_PIPE_CREATE_TIME 30000 
				__ret = check_name_pipe_exist( pipe_name ); 
				if( __ret == ERROR_SUCCESS )
				{
					break; 
				}

				Sleep( WAIT_SERVER_PIPE_CREATE_TIME ); 
				wait_time += WAIT_SERVER_PIPE_CREATE_TIME; 

				if( wait_time > MAX_WAIT_SERVER_PIPE_CREATE_TIME )
				{
					break; 
				}
			}

			if( wait_time > MAX_WAIT_SERVER_PIPE_CREATE_TIME )
			{
				break; 
			}
		}

		ret = init_pipe_point( &pipe_point, pipe_name ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_ERROR, "initialize pipe(%ws) error %x\n", pipe_name, ret ); 
			break; 
		}

		pipe_inited = TRUE; 

		for( ; ; )
		{
			html_element_action_request request; 
			request.set_request( HTML_ELEMENT_REQUEST_NEXT_ACTION_A ); 

			data_in_size = request.ByteSize(); 
			data_in = malloc( data_in_size ); 
			if( NULL == data_in )
			{
				break; 
			}
			
			_ret = request.SerializeToArray( data_in, data_in_size ); 
			if( false == _ret )
			{
				break; 
			}

			ret = _exec_cmd_from_pipe( &pipe_point, 
				( BYTE* )data_in, 
				data_in_size, 
				&result_data, 
				&result_data_size ); 

			free( data_in ); 
			data_in = NULL;  
			if( ERROR_SUCCESS != ret )
			{
				dbg_print( MSG_ERROR, "execute the command on the pipe(%ws) error %x\n", pipe_name, ret ); 
				break; 
			}

			//wait_loop(); 
			if( debug_mode & WAIT_DEBUGGER_ATTACH )
			{
				wait_loop(); 
			}

			ret = parse_html_actions( result_data, 
				result_data_size, 
				&html_action_work.action, 
				html_action_work.locate_url ); 

			if( ERROR_SUCCESS != ret )
			{
				dbg_print( MSG_ERROR, "parse received data error %x\n", ret ); 
				break; 
			}

			html_action_work.response.clear(); 

			ret = ::SendMessage( g_web_browser_window->GetSafeHwnd(), 
				WM_HTML_ACTION, 
				NULL, 
				HTML_ACTION_WORK_RUN_ACTION ); 

			if( ret != ERROR_SUCCESS )
			{

			}

			ret = ERROR_SUCCESS; 

			for( ; ; )
			{
				wait_ret = WaitForSingleObject( html_action_work.complete_notifier, 3000 ); 
				if( wait_ret == WAIT_OBJECT_0 )
				{
					break; 
				}

				if( NULL != job_worker.worker )
				{
					__ret = check_all_work_done( &job_worker ); 
					if( __ret == ERROR_SUCCESS )
					{
						ret = ERROR_EXCEPTION_IN_SCRIPT_HANDLER; 
						worker_job_end( &job_worker); 
						break; 
					}

					__ret = check_job_worker_exit( &job_worker ); 
					if( __ret == ERROR_SUCCESS )
					{
						//有两种可能:1.页面没有打开，这时可以重复执行命令 2.执行命令过程中发生的异常，这时需要重新创建此命令执行进程
						ret = ERROR_EXCEPTION_IN_SCRIPT_HANDLER; 
						break; 
					}
				}
				else
				{
					ret = ERROR_EXCEPTION_IN_SCRIPT_HANDLER; 
					ASSERT( FALSE ); 
					break; 
				}
			}

			if( 0 == wcscmp( html_action_work.action.action.c_str(), 
				HTML_ELEMENT_ACTION_HILIGHT ) )
			{
				Sleep( hilight_delay_time ); 
			}

			if( ret != ERROR_SUCCESS )
			{
				//ASSERT( FALSE ); 				
				//break; 
				wstring _response; 
				wstring return_code; 
				convert_int_to_string_w( ret, return_code ); 
				
				if( html_action_work.response.size() > 0 )
				{
					_response = ( *html_action_work.response.begin() ); 

					dbg_print_w( MSG_FATAL_ERROR, 
						L"exception occur, partial response is %s\n", 
						_response.c_str() ); 
				}

				html_action_work.response.clear(); 

				_response = return_code; 
				_response += RESPONSE_TEXT_DELIM; 
				_response += L"exception occur"; 

				html_action_work.response.push_back( _response ); 
			}

			{
				PVOID ret_data; 
				ULONG ret_data_len; 

				string text; 
				html_element_action_response response; 

				do
				{
					ULONG data_size; 
					PVOID data = NULL; 

					//hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, g_manager_pid ); 
					//if( FAILED(hr))
					//{

					//}

					//ret = init_pipe_point( &pipe_point, pipe_name ); 
					//if( ret != ERROR_SUCCESS )
					//{
					//	REMOTE_ERROR_DEBUG(); 
					//	break; 
					//}

					vector<wstring>::iterator it; 
					it = html_action_work.response.begin(); 
					for( ; ; )
					{
						if( it == html_action_work.response.end() )
						{
							break; 
						}

						ret = unicode_to_utf8_ex( ( *it ).c_str(), text ); 
						if( ret != ERROR_SUCCESS )
						{
							REMOTE_ERROR_DEBUG(); 
							*response.add_response() = ""; 
						}
						else
						{
							*response.add_response() = text; 
						}

						it ++; 
					}

					{
						string id; 
						convert_int_to_string( html_action_work.action.id, id ); 
						response.set_id( id ); 
					}

					data_size = response.ByteSize(); 

					do 
					{
						data = malloc( data_size ); 
						if( NULL == data )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							break; 
						}

						if( false == response.IsInitialized() )
						{
							response.CheckInitialized(); 
							REMOTE_ERROR_DEBUG(); 
							ret = ERROR_INVALID_PARAMETER; 
							break; 
						}

						if( false == response.SerializeToArray( data, data_size ) )
						{
							REMOTE_ERROR_DEBUG(); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						ret = _exec_cmd_from_pipe( &pipe_point, 
							( BYTE* )data, 
							data_size, 
							&ret_data, 
							&ret_data_len ); 

						if( ERROR_SUCCESS != ret )
						{
							REMOTE_ERROR_DEBUG(); 
							break; 
						}

						if( ret_data != NULL 
							&& ret_data_len > 0 )
						{
							execute_web_broswer_manage_command( ret_data, ret_data_len ); 
						}
					}while( FALSE );

					if( NULL != data )
					{
						free( data ); 
					}

					if( NULL != ret_data )
					{
						free(ret_data ); 
					}
				}while( FALSE ); 

				//uninit_pipe_point( &pipe_point ); 
			}
		}
	} while ( FALSE );

	if( TRUE == pipe_inited )
	{
		uninit_pipe_point( &pipe_point ); 
		//pipe_inited = FALSE; 
	}

	if( NULL != result_data )
	{
		free( result_data ); 
	}

	if( ret != ERROR_SUCCESS )
	{
		ExitProcess( ret ); 
	}

	return ret; 
}

void web_browser_window::OnDestroy()
{
	stop_job_worker( &job_worker, 500 ); 
	CDialog::OnDestroy(); 
}

BOOL web_browser_window::OnInitDialog()
{
	LRESULT ret; 
	//ULONG cc_ret_len; 

	CDialog::OnInitDialog();

	ret = get_html_action_global_config( &hilight_delay_time, NULL, NULL ); 

	//for( ; ; )
	//{
	//	Sleep( 10 ); 
	//}

	g_web_browser_window = this; 

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	//ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	//ASSERT(IDM_ABOUTBOX < 0xF000);

	//CMenu* pSysMenu = GetSystemMenu(FALSE);
	//if (pSysMenu != NULL)
	//{
	//	CString strAboutMenu;
	//	strAboutMenu.LoadString(IDS_ABOUTBOX);
	//	if (!strAboutMenu.IsEmpty())
	//	{
	//		pSysMenu->AppendMenu(MF_SEPARATOR);
	//		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	//	}
	//}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

//#define DEFAULT_BROWSER_URL L"http://www.xheike.com/forum.php?mod=post&amp;action=newthread&amp;fid=84" 
	//L"www.baidu.com"
	// TODO: Add extra initialization here
	//CWnd *pWnd = GetDlgItem(IDC_URL); 

	//if( target_url.GetLength() == 0 )
	//{
	//	target_url = DEFAULT_BROWSER_URL; 
	//}

	//if( pWnd )
	//{
	//	pWnd->SetWindowText( target_url.GetBuffer() );
	//}

	//init_scramble_info( &scramble_info);
	m_WebBrowser.SetSilent( TRUE ); 

//#ifndef __TEST_BROWSER__
	//browser_safe_navigate_start( &m_WebBrowser, target_url.GetBuffer() ); 

	if( manager_pid != 0 )
	{
		INT32 _ret; 
		HWND parent_wnd; 
		CRect wnd_rect; 

		g_manager_pid = manager_pid; 

		parent_wnd = find_web_browser_window( manager_pid ); 

#define MAIN_BROWSER_WINDOW_CAPTION_HEIGHT 22
#define MAIN_BROWSER_WINDOW_BORDER_WIDTH 1

		_ret = ::GetWindowRect( parent_wnd, &wnd_rect ); 
		if( _ret == FALSE )
		{
			CRect sub_wnd_rect; 
			GetWindowRect( &sub_wnd_rect ); 

			embed_web_browser_window( GetSafeHwnd(), parent_wnd, CRect( MAIN_BROWSER_WINDOW_BORDER_WIDTH, MAIN_BROWSER_WINDOW_CAPTION_HEIGHT, sub_wnd_rect.Width(), sub_wnd_rect.Height() ) ); 
		}
		else
		{
			embed_web_browser_window( GetSafeHwnd(), parent_wnd, CRect( MAIN_BROWSER_WINDOW_BORDER_WIDTH, MAIN_BROWSER_WINDOW_CAPTION_HEIGHT, wnd_rect.Width(), wnd_rect.Height() ) ); 
		}
	}

	ret = start_job_worker( &job_worker ); 
	if( ret != ERROR_SUCCESS )
	{
		//break; 
	}

	ret = init_html_action_work(); 
	if( ret != ERROR_SUCCESS )
	{
		//break; 
	}

#define EVALUATE_WORKER_TIMER_ID 1001
#define EVALUATE_WORKER_ELAPSE 1000
	
	SetTimer( EVALUATE_WORKER_TIMER_ID, EVALUATE_WORKER_ELAPSE, NULL ); 

#define CHECK_WEB_PAGE_LOADING_TIMER_ID 1002
#define CHECK_WEB_PAGE_LOADING_ELAPSE 5000
	SetTimer( CHECK_WEB_PAGE_LOADING_TIMER_ID, CHECK_WEB_PAGE_LOADING_ELAPSE, NULL ); 

//#endif //__TEST_BROWSER__

	//ShowWindow(SW_HIDE); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}


#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)

void web_browser_window::OnBack() 
{
	// TODO: Add your control notification handler code here
	try
	{
		m_WebBrowser.GoBack();
	}
	catch(CException* e)
	{		
		dbg_print( MSG_ERROR, "Warning: Uncaught exception %s\n",
			__FUNCTION__ );
		DELETE_EXCEPTION(e);
	}
}

void web_browser_window::OnForward() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoForward();
}

void web_browser_window::OnStop() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.Stop();
}

void web_browser_window::OnRefresh() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.Refresh();
}

void web_browser_window::OnHome() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoHome(); 
}

#if 0
LRESULT web_browser_window::unhighlight_html_element()
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr elements; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	ULONG element_count; 
	LONG _sub_element_count; 
	_bstr_t temp_text; 
	_variant_t name; 
	_variant_t index; 
	INT32 i; 
	INT32 find_count; 

	BOOLEAN sub_element_got = FALSE; 

	do 
	{
		break; 

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_all( &elements ); 
		if( FAILED( hr ) ) 
		{
			break; 
		}

		if( NULL == elements )
		{
			break; 
		}

		//hr = elements->get_length( &element_count ); 
		//if( FAILED( hr ) )
		//{
		//	break; 
		//}

		temp_text = HTML_HIGHLIGT_ELEMENT_ID; 

		name = temp_text.GetBSTR(); 

		find_count = 0; 

		for( ; ; )
		{

			V_VT( &index ) = VT_I4;
			V_I4( &index ) = find_count; 

			find_count += 1; 

			_hr = elements->item( name, index, &sub_element_disp ); 
			if( FAILED( _hr ) )
			{
				break; 
			}

			if( NULL == sub_element_disp )
			{
				//_hr = E_UNEXPECTED; 
				break; 
			}

			do 
			{
				hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
				if( FAILED( hr )
					|| sub_elements == NULL )
				{
					break; 
				}

				sub_element_got = TRUE; 

				hr = sub_elements->get_length( &_sub_element_count ); 
				if( FAILED( hr ) ) 
				{
					break; 
				}

				for( i = 0; i < _sub_element_count; i ++ )
				{
					do 
					{
						V_VT( &name ) = VT_I4;
						V_I4( &name ) = i; 

						//comment it.
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( name, index, &sub_element_disp ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						if( sub_element_disp == NULL )
						{
							hr = E_UNEXPECTED; 
							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( SUCCEEDED( hr )
							&& sub_element != NULL )
						{
							{
								LPWSTR _temp_text; 
								hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
								if( FAILED(hr))
								{
									break; 
								}

								_temp_text = temp_text.operator wchar_t*(); 
								if( NULL == _temp_text )
								{
									hr = E_UNEXPECTED; 
									break; 
								}

								hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
								if( FAILED(hr))
								{
									break; 
								}
							}
						}
					}while( FALSE ); 
				}
			} while ( FALSE );

			if( TRUE == sub_element_got )
			{
				break; 
			}

			hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( sub_element == NULL )
			{
				hr = E_UNEXPECTED; 
				break; 
			}

			{
				LPWSTR _temp_text; 
				hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				}

				_temp_text = temp_text.operator wchar_t*(); 
				if( NULL == _temp_text )
				{
					hr = E_UNEXPECTED; 
					break; 
				}

				hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
				if( FAILED(hr))
				{
					break; 
				}
			}
		}
	}while( FALSE ); 

	return hr; 
}


LRESULT web_browser_window::highlight_html_element_by_xpath( LPCWSTR xpath, LPCWSTR url )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLDocument2Ptr doc; 
	IDispatchPtr disp; 
	HTML_ELEMENT_GROUP group; 

	ASSERT( NULL != xpath ); 

	do 
	{
		//browser_safe_navigate_start( &m_WebBrowser, url ); 
		//_browser_safe_navigate_end( &m_WebBrowser, 10000 ); 

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == doc )
		{
			break; 
		}

		ret = get_html_elements_from_xpath( xpath, 
			doc, 
			&group ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_bstr_t html_text; 
		LPCWSTR text; 
		wstring _text; 

		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
			it != group.end(); 
			it ++ )
		{
			do 
			{
				hr = ( *it )->get_outerHTML( html_text.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				} 

				text = html_text.operator wchar_t*(); 
				if( NULL == text )
				{
					break; 
				}

				_text = L"<span id=\"" HTML_HIGHLIGT_ELEMENT_ID L"\" style=\"background-color:#9fe6ac\">"; 
				_text += text; 
				_text += L"</span>"; 

				html_text = _text.c_str(); 

				//html_text = L""; 

				hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
				if( FAILED(hr))
				{
					break; 
				} 

				//hr = highlight_html_element( ( *it) ); 

			} while ( FALSE );
		}

		//Sleep( 2000 ); 

		//for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
		//	it != group.end(); 
		//	it ++ )
		//{
		//	do 
		//	{
		//		hr = ( *it )->get_innerText( html_text.GetAddress() ); 
		//		if( FAILED(hr))
		//		{
		//			break; 
		//		} 

		//		text = html_text.operator wchar_t*(); 
		//		if( NULL == text )
		//		{
		//			break; 
		//		}

		//		//html_text = L""; 

		//		hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
		//		if( FAILED(hr))
		//		{
		//			break; 
		//		} 

		//		//hr = highlight_html_element( ( *it) ); 

		//	} while ( FALSE );
		//}

	}while( FALSE );

	return ret; 
}
#endif //0

LRESULT web_browser_window::exec_html_element_action( HTML_ELEMENT_ACTION *action ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	HRESULT hr; 
	HTML_ELEMENT_INFO *element_info = NULL; 

	do 
	{
		html_doc = m_WebBrowser.GetDocument(); 
		if( NULL == html_doc ) 
		{ 
			ret = ERROR_WEB_PAGE_IS_NOT_LOADED; 
			dbg_print( MSG_FATAL_ERROR, "can not load web page, html document is null\n" ); 
			ASSERT( FALSE ); 
			break; 
		} 

		disp = m_WebBrowser.GetControlUnknown(); 

		if( NULL == disp )
		{
			ret = get_html_element_not_found_error( ERROR_ERRORS_ENCOUNTERED ); 
			ASSERT( FALSE ); 
			break; 
		}

		hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			ret = get_html_element_not_found_error( ERROR_ERRORS_ENCOUNTERED ); 
			break; 
		}

		if( NULL == web_browser )
		{
			ret = get_html_element_not_found_error( ERROR_ERRORS_ENCOUNTERED ); 
			break; 
		}
		
		do 
		{				
			element_info = new HTML_ELEMENT_INFO(); 
			if( NULL == element_info )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			element_info->action_config = *action; 

			/*****************************************************************

			*****************************************************************/
			//ret = add_html_element_config_ex(config_xml_doc, 
			//	&config ); 

			//if( ret != ERROR_SUCCESS )
			//{
			//	break; 
			//}

		}while( FALSE ); 

		do 
		{
			if( 0 == _wcsicmp( element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK )
				&& 0 == _wcsicmp( element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ))
			{
				g_web_browser_window->m_WebBrowser.switch_navigate( TRUE ); 
			}

			ret = ie_auto_execute_on_element_ex( element_info,  
				web_browser ); 
			if( ret != ERROR_SUCCESS )
			{
				ret = get_html_element_not_found_error( ret ); 
				break; 
			}

			if( 0 != element_info->elements.size() )
			{
				ret = get_html_element_not_found_error( ERROR_ERRORS_ENCOUNTERED ); 
			}

			if( 0 == _wcsicmp( element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK )
				&& 0 == _wcsicmp( element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ))
			{
				dbg_print_w( MSG_IMPORTANT, L"set location url to %s\n", g_web_browser_window->ui_handler->get_translate_url().c_str() ); 
				g_web_browser_window->m_WebBrowser.set_loading_url( g_web_browser_window->ui_handler->get_translate_url().c_str() ); 
			}

            action->outputs = element_info->action_config.outputs; 

#if 0
			do 
			{
				if( 0 != wcsicmp( element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) )
				{
					break; 
				}

				if( 0 != wcsicmp( element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT ) )
				{
					break; 
				}
			}while( FALSE ); 
#endif //0
		}while( FALSE ); 
	}while( FALSE ); 

	if( element_info != NULL )
	{
		delete element_info; 
	}
	
	return ret; 
}

typedef struct _HTML_PAGE_DATA
{
	wstring html; 
	//images
} HTML_PAGE_DATA, *PHTML_PAGE_DATA; 

HTML_PAGE_DATA page_data; 

LRESULT WINAPI get_all_html_page_content( HTML_PAGE_DATA *page_data, IHTMLDocument2* html_doc ) //IWebBrowser2 *web_browser ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementCollectionPtr element_collect; 
	
	IDispatchPtr html_element_disp; 
	IHTMLElementPtr html_sub_element; 
	_variant_t tag; 
	_variant_t id; 
	_variant_t index; 

	LONG i; 
	LONG item_count; 

	_bstr_t html_text; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != page_data ); 

		page_data->html.clear(); 

		//html_doc->get_URL( ); 
		hr = html_doc->get_all( &element_collect ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == element_collect )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
		
		tag = L"HTML"; 
		hr = element_collect->tags( tag, 
			&html_element_disp ); 

		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = html_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&element_collect ); 
		if( FAILED( hr ) )
		{
			hr = html_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&html_sub_element ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			ASSERT( NULL != html_sub_element); 
			hr = html_sub_element->get_outerHTML( html_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_text = html_text.operator wchar_t*(); 
			if( NULL == _text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			page_data->html = html_text; 
			break; 
		}

		hr = element_collect->get_length( &item_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		for( i = 0; i < item_count; i ++ )
		{
			V_VT( &id ) = VT_I4;
			V_I4( &id ) = 0; 

			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			do 
			{
				hr = element_collect->item( id, index, &html_element_disp ); 
				if( FAILED( hr ) 
					|| html_element_disp == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = html_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&html_sub_element ); 
				if( FAILED( hr ) 
					|| html_sub_element == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = html_sub_element->get_outerHTML( html_text.GetAddress() ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text = html_text.operator wchar_t*(); 
				if( NULL == _text )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				page_data->html += html_text; 
			} while ( FALSE );
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI response_output_data(HTML_ELEMENT_ACTION *action,
    vector<wstring> *response)
{
    LRESULT ret = ERROR_SUCCESS;
    string text; 

    do
    {
        ASSERT(response != NULL);
        for (vector<wstring>::iterator it = action->outputs.begin();
            it != action->outputs.end();
            it++)
        {
            do
            {
                //ret = unicode_to_utf8_ex(it->c_str(), text);
                //if (ret != ERROR_SUCCESS)
                //{
                //    break;
                //}
                response->push_back(*it);
                //output_data += L" "; 
            } while (FALSE);
        }
    } while (FALSE);

    return ret;
}

LRESULT web_browser_window::on_html_action( WPARAM wparam, LPARAM lparam) 
{
	LRESULT ret = ERROR_SUCCESS; 
    LRESULT _ret; 
	HTML_ELEMENT_ACTION *action; 
	ULONG work_type; 
	BOOLEAN action_end = TRUE; 

	do 
	{
		//for( ; ; ) Sleep( 1 ); 
		//if( 0 == wparam )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		action = &html_action_work.action; 
		work_type = ( ULONG )lparam; 

		switch( work_type )
		{
		case HTML_ACTION_WORK_RUN_ACTION:
		case HTML_ACTION_WORK_LOCATE_TO_PAGE: 
			if( HTML_ACTION_WORK_RUN_ACTION == work_type 
				&& html_action_work.locate_url.length() > 0 ) 
			{ 
				HRESULT hr; 
				wstring url; 

				url = html_action_work.locate_url.c_str(); 
				if( 0 != wcsnicmp( url.c_str(), L"http", ARRAYSIZE( L"http" ) - 1 ) ) 
				{
					if( 0 == wcsnicmp( url.c_str(), 
						HTML_NEXT_LINK_NEED_CLICK, 
						ARRAYSIZE( HTML_NEXT_LINK_NEED_CLICK ) - 1 ) )
					{
						dbg_print( MSG_FATAL_ERROR, "%s:%u clck:\n", __FUNCTION__, __LINE__ ); 
					}
					else
					{

					}
				}

				hr = locate_to_url( &m_WebBrowser, url.c_str() ); 
				if( FAILED(hr))
				{
					//break; 
				}

				action_end = FALSE; 
				break; 
			}

			do 
			{
				if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) ) 
				{
                    ret = exec_html_element_action(action);
                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }

                    if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT))
                    {
                        html_action_work.response.push_back(wstring(L""));
                    }

                    response_output_data(action, &html_action_work.response);
                }
				else if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_HILIGHT ) ) 
				{
					ret = highlight_html_element_by_xpath( action->param.c_str(), 
						action->xpath.c_str() ); 

					if( ret != ERROR_SUCCESS )
					{
					}

					//Sleep( hilight_delay_time ); 

					{
						HRESULT hr; 
						WCHAR text[ MAX_PATH ]; 

						hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", ret ); 
						if( FAILED( hr ) )
						{
						}

						html_action_work.response.push_back( text ); 
					}
				}
				else if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_INPUT ) ) 
				{
					ret = exec_html_element_action( action ); 
					if( ret != ERROR_SUCCESS )
					{
					}

					{
						HRESULT hr; 
						WCHAR text[ MAX_PATH ]; 

						hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", ret ); 
						if( FAILED( hr ) )
						{

						}

						html_action_work.response.push_back( text ); 
					}
				}
				else if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_CLICK ) ) 
				{
					HRESULT hr; 

					if( action->outputs.size() > 0 
                        && 0 != wcscmp( action->outputs[0].c_str(), L"done" ) )
					{
						ret = exec_html_element_action( action ); 
                        action->outputs.clear();
                        action->outputs.push_back(wstring(L"done")); 

						if( 0 == wcscmp( action->param.c_str(), 
							HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ) )
						{
							hr = start_worker( ( PVOID )HTML_ACTION_WORK_LOCATE_TO_PAGE ); 
							if( FAILED( hr ) )
							{
								//break; 
							}

							action_end = FALSE; 
						}
					}

					if( action_end == TRUE )
					{
						WCHAR text[ MAX_PATH ]; 
						wstring reponse; 

						hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", 0 ); 
						if( FAILED( hr ) )
						{

						}

						reponse = text; 

                        html_action_work.response.push_back(text); 

						//reponse += RESPONSE_TEXT_DELIM; 

						{
							wstring url; 

							_ret = get_web_browser_location_url( &g_web_browser_window->m_WebBrowser, url ); 
                            html_action_work.response.push_back(url);
                            //reponse = url; 
						}

					}
				}
				else if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_BACK ) ) 
				{
                    if (action->outputs.size() > 0
                        && 0 != wcscmp(action->outputs[0].c_str(), L"done"))
                    {
						HRESULT hr; 

						m_WebBrowser.switch_navigate( TRUE ); 
						m_WebBrowser.GoBack(); 

						hr = start_worker( ( PVOID )HTML_ACTION_WORK_LOCATE_TO_PAGE ); 
						if( FAILED( hr ) )
						{
							//break; 
						}

                        ret = exec_html_element_action(action);
                        action->outputs.clear();
                        action->outputs.push_back(L"done");

						action_end = FALSE; 
					}

					HRESULT hr; 
					WCHAR text[ MAX_PATH ]; 
					wstring reponse; 

					hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", 0 ); 
					if( FAILED( hr ) )
					{

					}

					reponse = text; 
					reponse += RESPONSE_TEXT_DELIM; 

					{
						wstring url; 

						ret = get_web_browser_location_url( &g_web_browser_window->m_WebBrowser, url ); 
						reponse += url; 
					}

					html_action_work.response.push_back( reponse ); 
				}
				else if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_LOCATE ) ) 
				{
					//if( 0 != wcscmp( action->output.c_str(), L"done" ) )
					//{
					//	HRESULT hr; 
					//	wstring url; 

					//	url = action->param.c_str(); 

					//	if( 0 != wcsnicmp( url.c_str(), L"http", ARRAYSIZE( L"http" ) - 1 ) ) 
					//	{
					//		//dbg_print()
					//	}

					//	hr = locate_to_url( &m_WebBrowser, url.c_str() ); 
					//	if( FAILED(hr))
					//	{
					//		//break; 
					//	}

					//	action->output = L"done"; 
					//	action_end = FALSE; 
					//	break; 
					//}

					HRESULT hr; 
					WCHAR text[ MAX_PATH ]; 
					wstring reponse; 

					hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", 0 ); 
					if( FAILED( hr ) )
					{

					}

					reponse = text; 
					reponse += RESPONSE_TEXT_DELIM; 

					{
						wstring url; 

						ret = get_web_browser_location_url( &g_web_browser_window->m_WebBrowser, url ); 
						reponse += url; 
					}

					html_action_work.response.push_back( reponse ); 
				}
				else
				{
					if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_SCRIPT ) ) 
					{
						ASSERT( FALSE ); 
					}

					ASSERT( FALSE ); 
					HRESULT hr; 
					INT32 _ret; 
					WCHAR text[ MAX_PATH ]; 

					hr = StringCchCopyW( text, 
						ARRAYSIZE( text ), 
						L"-1;unknown_action" ); 
					if( FAILED( hr ) )
					{

					}

					html_action_work.response.push_back( text ); 
					_ret = SetEvent( html_action_work.complete_notifier ); 
					if( FALSE == _ret )
					{
						ASSERT( FALSE ); 
						break; 
					}
					break; 
				} 
			}while( FALSE ); 

			if( work_type == HTML_ACTION_WORK_LOCATE_TO_PAGE 
				&& 0 == wcscmp( action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT ) )
			{
				HRESULT hr; 
				HTML_PAGE_DATA page_data; 
				IHTMLDocument2Ptr html_doc; 
				IDispatchPtr disp; 

				do 
				{
					disp = m_WebBrowser.GetDocument(); 
					hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
					if( FAILED(hr))
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					ret = get_all_html_page_content( &page_data, html_doc ); 
					if( ret != ERROR_SUCCESS )
					{
						//break; 
					}
                    
                    if (html_action_work.response.size() < 1)
                    {
                        html_action_work.response.push_back(page_data.html);
                    }
                    else
                    {
                        html_action_work.response[0 ] = page_data.html;
                    }
                } while ( FALSE );
			}

			if( TRUE == action_end )
			{
				INT32 _ret; 
				_ret = SetEvent( html_action_work.complete_notifier ); 
				if( FALSE == _ret )
				{
					ASSERT( FALSE ); 
					break; 
				}
			}
			break; 
		default:
			ASSERT( FALSE ); 
			{
				HRESULT hr; 
				INT32 _ret; 
				WCHAR text[ MAX_PATH ]; 

				hr = StringCchCopyW( text, 
					ARRAYSIZE( text ), 
					L"-1;unknown_command" ); 
				if( FAILED( hr ) )
				{

				}

				html_action_work.response.push_back( text ); 
				_ret = SetEvent( html_action_work.complete_notifier ); 
				if( FALSE == _ret )
				{
					ASSERT( FALSE ); 
					break; 
				}
			}
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}  //switch
	}while( FALSE ); 

	return ret; 
}

//LRESULT web_browser_window::on_web_page_actions( WPARAM wparam, 
//							   LPARAM lparam) 
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	// TODO: Add your control notification handler code here
//
//	switch( lparam )
//	{
//	case 0:
//		{
//			HRESULT hr; 
//			wstring url; 
//			scramble_work_config.links.clear(); 
//			scramble_work_config.next_link.clear(); 
//
//			//REMOTE_ERROR_DEBUG(); 
//
//			if( scramble_work_mode == data_scrabmle::CONTENT_WEB_PAGE_HILIGHT )
//			{
//				ret = highlight_html_element_by_xpath( scramble_work_config.link_list_xpath.c_str() ); 
//				if( ret != ERROR_SUCCESS )
//				{
//					dbg_print( MSG_FATAL_ERROR, "hilight the dest elements error %u\n", ret ); 
//					break; 
//				}
//
//				break; 
//			}
//
//			html_scramble( &m_WebBrowser, &scramble_work_config ); 
//
//			{
//				string text; 
//				scramble_response response; 
//				frame_output *frame_response = NULL; 
//
//				do 
//				{
//					response.set_mode( FRAME_WEB_PAGE_SCRAMBLE ); 
//					
//					frame_response = response.mutable_frame(); 
//
//					if( NULL == frame_response )
//					{
//						REMOTE_ERROR_DEBUG(); 
//						ret = ERROR_NOT_ENOUGH_MEMORY; 
//						break; 
//					}
//
//					if( scramble_work_config.next_link.size() != 0 )
//					{
//						ret = unicode_to_utf8_ex( ( *scramble_work_config.next_link.begin() ).c_str(), 
//							text ); 
//						if( ERROR_SUCCESS != ret )
//						{
//							REMOTE_ERROR_DEBUG(); 
//							break; 
//						}
//
//						frame_response->set_next_link( text ); 
//					}
//					else
//					{
//						frame_response->set_next_link( "" ); 
//					}
//
//					LINK_LIST_ITERATOR it; 
//
//					it = scramble_work_config.links.begin(); 
//
//					for( ; ; )
//					{
//						if( it == scramble_work_config.links.end() )
//						{
//							break; 
//						}
//
//						ret = unicode_to_utf8_ex( ( *it ).c_str(), text ); 
//						if( ERROR_SUCCESS != ret )
//						{
//							break; 
//						}
//
//						frame_response->add_link( text ); 
//
//						it ++; 
//					}
//
//					CString url; 
//					url = g_web_browser_window->m_WebBrowser.GetLocationURL(); 
//					if( url.GetLength() == 0 )
//					{
//						wstring _url; 
//						_url = g_web_browser_window->m_WebBrowser.get_loading_url(); 
//
//						ret = unicode_to_utf8_ex( _url.c_str(), text ); 
//						if( ERROR_SUCCESS != ret )
//						{
//							break; 
//						}
//					}
//					else
//					{
//						ret = unicode_to_utf8_ex( url.GetBuffer(), text ); 
//						if( ERROR_SUCCESS != ret )
//						{
//							break; 
//						}
//					}
//
//					frame_response->set_url( text ); 
//
//					{
//						LRESULT ret; 
//						ULONG message; 
//						pipe_ipc_point pipe_point; 
//						PVOID ret_data; 
//						WCHAR pipe_name[ MAX_PATH ]; 
//						ULONG ret_data_len; 
//
//						do
//						{
//							hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, g_manager_pid ); 
//							if( FAILED(hr))
//							{
//
//							}
//
//							ret = init_pipe_point( &pipe_point, pipe_name ); 
//							if( ret != ERROR_SUCCESS )
//							{
//								REMOTE_ERROR_DEBUG(); 
//								break; 
//							}
//
//							{
//								ULONG data_size; 
//								PVOID data = NULL; 
//
//								data_size = response.ByteSize(); 
//
//								do 
//								{
//									data = malloc( data_size ); 
//									if( NULL == data )
//									{
//										ret = ERROR_NOT_ENOUGH_MEMORY; 
//										break; 
//									}
//
//									if( false == response.IsInitialized() )
//									{
//										response.CheckInitialized(); 
//										REMOTE_ERROR_DEBUG(); 
//										ret = ERROR_INVALID_PARAMETER; 
//										break; 
//									}
//
//									if( false == response.SerializeToArray( data, data_size ) )
//									{
//										REMOTE_ERROR_DEBUG(); 
//										ret = ERROR_ERRORS_ENCOUNTERED; 
//										break; 
//									}
//
//									ret = _exec_cmd_from_pipe( &pipe_point, 
//										( BYTE* )data, 
//										data_size, 
//										&ret_data, 
//										&ret_data_len ); 
//
//									if( ERROR_SUCCESS != ret )
//									{
//										REMOTE_ERROR_DEBUG(); 
//										break; 
//									}
//								} while ( FALSE ); 
//							}
//
//							if( ret_data != NULL 
//								&& ret_data_len > 0 )
//							{
//								execute_web_broswer_manage_command( ret_data, ret_data_len ); 
//							}
//						}while( FALSE ); 
//
//						if( NULL != ret_data )
//						{
//							free(ret_data ); 
//						}
//					}
//				}while( FALSE ); 
//			}
//		}
//		break; 
//	case 1: 
//		{
//			HRESULT hr; 
//			string text; 
//			wstring url; 
//			scramble_response response; 
//			content_output *content = NULL; 
//
//			response.set_mode( CONTENT_WEB_PAGE_SCRAMBLE ); 
//
//			hr = scramble_web_page_content( &m_WebBrowser, 
//				&scramble_work_config.page_content ); 
//
//			if( FAILED( hr ) ) 
//			{
//			}
//
//			content = response.mutable_content(); 
//			
//			if( NULL == content )
//			{
//				REMOTE_ERROR_DEBUG(); 
//				ret = ERROR_NOT_ENOUGH_MEMORY; 
//				break; 
//			}
//
//			CString __url; 
//			__url = g_web_browser_window->m_WebBrowser.GetLocationURL(); 
//			if( __url.GetLength() == 0 )
//			{
//				url = g_web_browser_window->m_WebBrowser.get_loading_url(); 
//
//				ret = unicode_to_utf8_ex( url.c_str(), text ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//			}
//			else
//			{
//				ret = unicode_to_utf8_ex( __url.GetBuffer(), text ); 
//				if( ERROR_SUCCESS != ret )
//				{
//					break; 
//				}
//			}
//
//			//ret = unicode_to_utf8_ex( scramble_work_config.url.c_str(), text ); 
//			//if( ret != ERROR_SUCCESS )
//			//{
//			//	REMOTE_ERROR_DEBUG(); 
//			//	break; 
//			//}
//
//			content->set_url( text ); 
//
//			HTML_ELEMENT_CONTENTS_ITERATOR it; 
//			field_output *field = NULL; 
//
//			it = scramble_work_config.page_content.element_contents.begin(); 
//			for( ; ; )
//			{
//				if( it == scramble_work_config.page_content.element_contents.end() )
//				{
//					break; 
//				}
//
//				ASSERT( ( *it ) != NULL ); 
//
//				field = content->add_content(); 
//				if( NULL == field )
//				{
//					ret = ERROR_NOT_ENOUGH_MEMORY; 
//					break; 
//				}
//
//				field->set_name(""); 
//				field->set_output(""); 
//				field->add_additions(""); 
//
//				format_output_text(( *it )->text ); 
//
//				ret = unicode_to_utf8_ex( ( *it )->text.c_str(), text ); 
//				if( ret != ERROR_SUCCESS )
//				{
//					REMOTE_ERROR_DEBUG(); 
//					break; 
//				}
//
//				field->set_output( text ); 
//
//				{
//					INT32 i; 
//
//					//for( ; ; )
//					//{
//					//	Sleep( 10 ); 
//					//}
//
//					for( i = 0; i < ( *it )->additions.size(); i ++ )
//					{
//						do 
//						{
//							ret = unicode_to_utf8_ex( ( *it )->additions.at( i ).c_str(), text ); 
//							if( ret != ERROR_SUCCESS )
//							{
//								REMOTE_ERROR_DEBUG(); 
//								break; 
//							}
//
//							field->add_additions( text ); 
//						} while ( FALSE );
//					}
//				}
//
//				ret = unicode_to_utf8_ex( ( *it )->name.c_str(), text ); 
//				if( ret != ERROR_SUCCESS )
//				{
//					REMOTE_ERROR_DEBUG(); 
//					break; 
//				}
//
//				field->set_name( text ); 
//				it ++; 
//			}
//			
//			//if( false == content->has_url() 
//			//	0 == content->content_size() 
//			//	false == content->IsInitialized() )
//			//{
//			//	content->CheckInitialized(); 
//			//	REMOTE_ERROR_DEBUG(); 
//			//	ret = ERROR_INVALID_PARAMETER; 
//			//	break; 
//			//}
//
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			{
//				ULONG message; 
//				pipe_ipc_point pipe_point; 
//				PVOID ret_data; 
//				ULONG ret_data_len; 
//				PVOID data = NULL; 
//				ULONG data_size; 
//				WCHAR pipe_name[ MAX_PATH ]; 
//
//				do
//				{
//					hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, g_manager_pid ); 
//					if( FAILED(hr))
//					{
//
//					}
//
//					ret = init_pipe_point( &pipe_point, pipe_name ); 
//					if( ret != ERROR_SUCCESS )
//					{
//						break; 
//					}
//
//					message = CONTENT_WEB_PAGE_SCRAMBLE; 
//
//					data_size = response.ByteSize(); 
//					data = malloc( data_size ); 
//					if( NULL == data )
//					{
//						break; 
//					}
//
//					if( false == response.IsInitialized() )
//					{
//						response.CheckInitialized(); 
//						REMOTE_ERROR_DEBUG(); 
//						ret = ERROR_INVALID_PARAMETER; 
//						break; 
//					}
//
//					if( false == response.SerializeToArray( data, data_size ) )
//					{
//						REMOTE_ERROR_DEBUG(); 
//						ret = ERROR_ERRORS_ENCOUNTERED; 
//						break; 
//					}
//
//					ret = _exec_cmd_from_pipe( &pipe_point, 
//						( BYTE* )data, 
//						data_size, 
//						&ret_data, 
//						&ret_data_len ); 
//					
//					if( ERROR_SUCCESS != ret )
//					{
//						REMOTE_ERROR_DEBUG(); 
//						break; 
//					}
//
//					if( ret_data != NULL 
//						&& ret_data_len > 0 )
//					{
//						execute_web_broswer_manage_command( ret_data, ret_data_len ); 
//					}
//				}while( FALSE );
//
//				if( NULL != data )
//				{
//					free( data ); 
//				}
//
//				if( NULL != ret_data )
//				{
//					free(ret_data ); 
//				}
//			} 
//
//			if( scramble_work_config.links.size() == 0 )
//			{
//				if( scramble_work_config.next_link.size() > 0)
//				{
//					url = *scramble_work_config.next_link.begin(); 
//					scramble_work_config.next_link.erase( scramble_work_config.next_link.begin() ); 
//
//					if( 0 != wcsnicmp( url.c_str(), L"http:", ARRAYSIZE( L"http:" ) - 1 ) ) 
//					{
//						wstring _url; 
//
//						//for( ; ; )
//						//{
//						//	Sleep( 10 ); 
//						//}
//
//						if( 0 != wcsnicmp( url.c_str(), HTML_NEXT_LINK_NEED_CLICK, ARRAYSIZE( HTML_NEXT_LINK_NEED_CLICK ) - 1 ) )
//						{
//							break; 
//						}
//
//						_url = url.c_str() + ARRAYSIZE( HTML_NEXT_LINK_NEED_CLICK ) - 1; 
//
//						dbg_print( MSG_IMPORTANT, "clieck %ws\n", _url.c_str() ); 
//						hr = locate_to_url( &m_WebBrowser, _url.c_str() ); 
//						if( FAILED(hr))
//						{
//							//break; 
//						}
//					}
//					else
//					{
//						hr = html_scramble_ex( &m_WebBrowser, url.c_str() ); 
//						if( FAILED(hr))
//						{
//							//break; 
//						}
//					}
//				}
//				break; 
//			}
//
//			//for( ; ; )
//			{
//				if( scramble_work_config.links.size() == 0)
//				{
//					break; 
//				}
//
//				url = *scramble_work_config.links.begin(); 
//				scramble_work_config.links.erase( scramble_work_config.links.begin() ); 
//
//				hr = scramble_web_page_content_ex( &m_WebBrowser,  
//					url.c_str() ); 
//
//				if( FAILED(hr ) )
//				{
//					dbg_print( MSG_ERROR, "scramble html page error %u\n", hr ); 
//				}
//				else
//				{
//					break; 
//				}
//			}
//		}
//		break; 
//	case 2:
//		{
//			wstring url; 
//			HRESULT hr; 
//
//			//if( scramble_info.next_link.size() > 0 )
//			//{
//			//	url = *scramble_info.next_link.begin(); 
//			//	scramble_info.next_link.erase( scramble_info.next_link.begin() ); 
//			//}
//
//			if( scramble_work_config.links.size() == 0)
//			{
//				if( scramble_work_config.url.length() == 0 )
//				{
//					break; 
//				}
//
//				url = scramble_work_config.url; 
//
//				if( 0 != wcsnicmp( url.c_str(), L"http", ARRAYSIZE( L"http" ) - 1 ) ) 
//				{
//					wstring _url; 
//
//					//for( ; ; )
//					//{
//					//	Sleep( 10 ); 
//					//}
//
//					if( 0 != wcsnicmp( url.c_str(), HTML_NEXT_LINK_NEED_CLICK, ARRAYSIZE( HTML_NEXT_LINK_NEED_CLICK ) - 1 ) )
//					{
//						break; 
//					}
//
//					_url = url.c_str() + ( ARRAYSIZE( HTML_NEXT_LINK_NEED_CLICK ) - 1 ); 
//
//					dbg_print( MSG_IMPORTANT, "click %ws\n", _url.c_str() ); 
//					hr = locate_to_url( &m_WebBrowser, _url.c_str() ); 
//					if( FAILED(hr))
//					{
//						//break; 
//					}
//				}
//				else
//				{
//					hr = html_scramble_ex( &m_WebBrowser, url.c_str() ); 
//					if( FAILED(hr))
//					{
//						//break; 
//					}
//				}
//				break; 
//			}
//
//			url = *scramble_work_config.links.begin(); 
//			scramble_work_config.links.erase( scramble_work_config.links.begin() ); 
//
//			hr = scramble_web_page_content_ex( &m_WebBrowser,  
//				url.c_str() ); 
//
//
//			if( FAILED(hr ) )
//			{
//				dbg_print( MSG_ERROR, "scramble html page error %u\n", hr ); 
//			}
//			else
//			{
//				break; 
//			}
//		}
//		break; 
//	case 5:
//		{
//			//for( ; ; )
//			//{
//			//	Sleep( 10 ); 
//			//}
//
//			html_scramble_click_next( &m_WebBrowser, &scramble_work_config ); 
//		}
//	default:
//		break; 
//	}
//
//	return ERROR_SUCCESS; 
//}

void web_browser_window::OnSearch() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoSearch();
}

void web_browser_window::OnWebBrowserDownloadBegin() 
{
	// TODO: Add your control notification handler code here

	//m_Animate.Play(0,-1,-1);
}

void web_browser_window::OnWebBrowserDownloadComplete() 
{
	// TODO: Add your control notification handler code here
	//m_Animate.Stop();
	//m_Animate.Seek(0);
}

void web_browser_window::OnWebBrowserStatusTextChange(LPCTSTR Text) 
{
	// TODO: Add your control notification handler code here
	if (GetSafeHwnd())
	{
		CWnd *pWnd = GetDlgItem(IDC_STATUS_TEXT);
		if (pWnd)
			pWnd->SetWindowText(Text);
	}
}

void web_browser_window::OnWebBrowserDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	dbg_print( MSG_IMPORTANT, "the html document loaded\n" ); 

	HRESULT hr; 
	//html_element_prop_dlg dlg; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		disp = m_WebBrowser.GetDocument(); 

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) 
			|| html_doc == NULL )
		{
			break; 
		}

		//dump_html_elements_on_page( html_doc ); 
		//disable_web_page_anchor( html_doc ); 
		ret = set_host_ui_handler_on_doc( html_doc, ui_handler ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE ); 

	//if( URL->vt == VT_BSTR )
	//{
	//	_bstr_t text; 
	//	LPCWSTR _text; 

	//	do 
	//	{
	//		wstring url_loading; 

	//		text = URL->bstrVal; 
	//		if( text.length() <= 0)
	//		{
	//			m_eURL.SetWindowText( L"" ); 

	//			break; 
	//		}

	//		_text = text.operator const wchar_t*(); 
	//		ASSERT( NULL != _text ); 

	//		m_eURL.SetWindowText( _text ); 

	//		url_loading = m_WebBrowser.get_loading_url(); 

	//		if( url_loading.length() == 0 )
	//		{
	//			break; 
	//		}

	//		if( 0 == wcscmp( url_loading.c_str(), 
	//			_text ) )
	//		{
	//			m_WebBrowser.notify_status_event( TRUE ); 

	//			browser_safe_navigate_start( &m_WebBrowser, url_loading.c_str() ); 
	//		}

	//	}while( FALSE ); 

	//}


	//UpdateData( FALSE ); 
	//dbg_print( MSG_IMPORTANT, "the html document loaded\n" ); 

	if( URL->vt == VT_BSTR )
	{
		_bstr_t text; 
		LPCWSTR _text; 

		do 
		{
			wstring url_loading; 

			text = URL->bstrVal; 
			if( text.length() <= 0)
			{
				//m_eURL.SetWindowText( L"" ); 

				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text ) 
			{
				break; 
			}

			//m_eURL.SetWindowText( _text ); 

			url_loading = m_WebBrowser.get_loading_url(); 

			if( url_loading.length() == 0 )
			{
				m_WebBrowser.notify_status_event( TRUE ); 
				break; 
			}

			if( 0 == wcscmp( url_loading.c_str(), 
				_text ) )
			{
				m_WebBrowser.notify_status_event( TRUE ); 

				//browser_safe_navigate_start( &m_WebBrowser, url_loading.c_str() ); 
			}
			
		}while( FALSE ); 

		if( NULL != html_doc )
		{
			scroll_expand_html_doc( html_doc ); 
		}
	}


	//UpdateData( FALSE ); 
}

void web_browser_window::OnWebBrowserMenuBar(BOOL MenuBar)
{
	// TODO: Add your message handler code here
}

void web_browser_window::WebBrowserNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	// TODO: Add your message handler code here
	LRESULT _ret; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr _html_element; 
	wstring xpath; 
	// TODO: Add your message handler code here

	*Cancel = TRUE; 

	do 
	{
		do 
		{
			HRESULT hr  = -1;

			if( NULL == ppDisp )
			{
				ASSERT( FALSE ); 
				break; 
			}

			disp = m_WebBrowser.GetControlUnknown(); 

			if( NULL == disp )
			{
				ASSERT( FALSE ); 
				break; 
			}


			hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
			if( FAILED( hr ))
			{
				break; 
			}

			if( NULL == web_browser )
			{
				break; 
			}

			hr = web_browser->get_Application( &disp ); 
			if ( SUCCEEDED(hr) )
			{
				*ppDisp  = disp;
			}	
		} while ( FALSE );

		//if( m_WebBrowser.can_navigate() != FALSE )
		{
			dbg_print_w( MSG_IMPORTANT, L"locate to translate url %s on new window event\n", ui_handler->get_translate_url().c_str() ); 
			_ret = browser_safe_navigate_start( &m_WebBrowser, 
				ui_handler->get_translate_url().c_str() ); 
		}
#if 0
		disp = m_WebBrowser.GetDocument(); 
		//hr = web_browser->get_Document( &disp ); 
		//if( FAILED( hr ))
		//{
		//	break; 
		//}

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
		if( FAILED( hr ))
		{
			break; 
		}

		//html_doc->get_parentWindow( )
		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == _html_element )
		{
			break; 
		}

		_ret = get_active_element_xpath_ex( _html_element, &xpath, &html_element ); 

		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

		//if( xpath_dlg.get_xpath1().GetLength() != 0 )
		//{
		//	xpath_dlg.set_xpath2( CString( xpath.c_str() ) ); 
		//}
		//else
		//{
		//	xpath_dlg.set_xpath1( CString( xpath.c_str() ) ); 
		//}

		////xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
		//xpath_dlg.ShowWindow( SW_SHOW ); 
		//MessageBoxW( xpath.c_str(), NULL, 0 ); 
#endif //0
	}while( FALSE );

	//if( m_WebBrowser.can_navigate() == FALSE )
	//{
	//	*Cancel = TRUE; 
	//}
}

void web_browser_window::OnWebBrowserFullScreen(BOOL FullScreen)
{
	// TODO: Add your message handler code here
}

void web_browser_window::WebBrowserBeforeScriptExecute(LPDISPATCH pDispWindow)
{
	// TODO: Add your message handler code here
}

void web_browser_window::WebBrowserBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel)
{
	// TODO: Add your message handler code here

	LRESULT _ret; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr _html_element; 
	wstring xpath; 
	HRESULT hr; 
	// TODO: Add your message handler code here

#define beforeNavigateExternalFrameTarget 0x0001
	if( Flags->vt == VT_I4 
		&&  Flags->intVal & beforeNavigateExternalFrameTarget )
	{

	}

	do 
	{
		if( NULL == pDisp )
		{
			ASSERT( FALSE ); 
			break; 
		}

		hr = pDisp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == web_browser )
		{
			break; 
		}

		hr = web_browser->get_Document( &disp ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == _html_element )
		{
			break; 
		}

		_ret = get_active_element_xpath_ex( _html_element, &xpath, NULL, &html_element ); 

		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

		//if( xpath_dlg.get_xpath1().GetLength() != 0 )
		//{
		//	xpath_dlg.set_xpath2( CString( xpath.c_str() ) ); 
		//}
		//else
		//{
		//	xpath_dlg.set_xpath1( CString( xpath.c_str() ) ); 
		//}

		////xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
		//xpath_dlg.ShowWindow( SW_SHOW ); 
		//MessageBoxW( xpath.c_str(), NULL, 0 ); 
	}while( FALSE );

	if( m_WebBrowser.can_navigate() == FALSE )
	{
		dbg_print( MSG_IMPORTANT, "block navigate\n" ); 
		*Cancel = TRUE; 
	}
}

//void web_browser_window::OnBnClickedButtonScramble()
//{
//	LRESULT ret; 
//	HRESULT hr; 
//	wstring text; 
//	_bstr_t _text; 
//	wstring text_name; 
//	IHTMLElementPtr html_element; 
//	IDispatchPtr disp; 
//	IHTMLDocument2Ptr html_doc; 
//
//	do 
//	{
//		BOOLEAN is_input; 
//
//		hr = html_scramble_ex( &m_WebBrowser, scramble_info.url.c_str() ); 
//		if( FAILED(hr))
//		{
//			//break; 
//		}
//
//	}while( FALSE );
//
//	// TODO: Add your control notification handler code here
//}


BEGIN_MESSAGE_MAP(web_browser_window, CDialog)
	//{{AFX_MSG_MAP(CBrowser_ControlDlg)
	//ON_WM_SYSCOMMAND()
	//ON_WM_PAINT()
	//ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(ID_BACK, OnBack)
	//ON_BN_CLICKED(ID_FORWARD, OnForward)
	//ON_BN_CLICKED(ID_STOP, OnStop)
	////ON_BN_CLICKED(ID_REFRESH, OnRefresh)
	//ON_BN_CLICKED(ID_HOME, OnHome)
	//ON_MESSAGE( WM_WEB_PAGE_ACTIONS, on_web_page_actions )
	ON_MESSAGE( WM_HTML_ACTION, on_html_action )
	ON_MESSAGE( WM_HTML_ELEMENTS_HILIGHT, on_html_element_hilight)

	
	ON_WM_TIMER()
	//ON_BN_CLICKED(ID_SEARCH, OnSearch)
	//}}AFX_MSG_MAP
	//ON_BN_CLICKED(IDC_BUTTON_RECORD_SELECTED, OnBnClickedButtonRecordSelected)
	//ON_BN_CLICKED(IDC_BUTTON_RECORD_PAGE, OnBnClickedButtonRecordPage)
	//ON_BN_CLICKED(IDC_BUTTON_NEW_PAGE_GROUP, OnBnClickedButtonNewPageGroup)
	//ON_BN_CLICKED(IDC_BUTTON_CPATURE_INPUT, OnBnClickedButtonScramble)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(web_browser_window, CDialog)
//{{AFX_EVENTSINK_MAP(web_browser_window)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 106 /* DownloadBegin */, OnWebBrowserDownloadBegin, VTS_NONE)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 104 /* DownloadComplete */, OnWebBrowserDownloadComplete, VTS_NONE)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 102 /* StatusTextChange */, OnWebBrowserStatusTextChange, VTS_BSTR)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 259 /* DocumentComplete */, OnWebBrowserDocumentComplete, VTS_DISPATCH VTS_PVARIANT)
//}}AFX_EVENTSINK_MAP
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 256, web_browser_window::OnWebBrowserMenuBar, VTS_BOOL)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 251, web_browser_window::WebBrowserNewWindow2, VTS_PDISPATCH VTS_PBOOL)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 258, web_browser_window::OnWebBrowserFullScreen, VTS_BOOL)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 290, web_browser_window::WebBrowserBeforeScriptExecute, VTS_DISPATCH)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 250, web_browser_window::WebBrowserBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
ON_EVENT(web_browser_window, IDC_WEB_BROWSER_CONTROL, 271, web_browser_window::WebBrowserNavigateError, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

// web_web_browser_window message handlers

void web_browser_window::OnOK() 
{
	UpdateData(TRUE);
}

void web_browser_window::OnCancel() 
{
	PostQuitMessage( 0xcace ); 
}

void web_browser_window::OnTimer(UINT_PTR nIDEvent)
{
	LRESULT ret; 
	CDialog::OnTimer(nIDEvent);

	do
	{
#if 0
		ASSERT( NULL != site ); 
		IDispatchPtr html_doc_disp; 

		HTML_PAGE_INFO *page; 
		HTML_PAGE_INFOS_ITERATOR it; 

		//ret =_ie_auto_execute_on_site( html_doc, site ); 

		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		for( it = site->pages.begin(); it != site->pages.end(); it ++ )
		{
			do 
			{
				page = ( *it ); 


				m_WebBrowser.Navigate( page->url.c_str(), 
					NULL, 
					NULL, 
					NULL, 
					NULL );

				Sleep( 2000 ); 

				//m_WebBrowser.GetInterface(); 

				html_doc_disp = m_WebBrowser.GetDocument(); 
				if( html_doc_disp == NULL )
				{
					break; 
				}

				//ret = ie_auto_execute_on_page( page, html_doc_disp ); 

				//if( ret != ERROR_SUCCESS )
				//{
				//	//dbg_print()
				//}

			}while( FALSE ); 
		}
#endif //0
		if( CHECK_WEB_PAGE_LOADING_TIMER_ID == nIDEvent )
		{
			LRESULT _ret; 
			_ret = m_WebBrowser.check_web_page_loading_status(); 
			break; 
		}

		ret = evaluate_worker_state(); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_ERROR, "evaluate working state for web surf error 0x%0.8x\n", ret ); 
			break; 
		}
	}while( FALSE ); 

	//KillTimer( nIDEvent ); 
}

ULONG CALLBACK web_page_work( PVOID param )
{
	LRESULT ret = 0; 
	LRESULT _ret; 
	ULONG wait_ret; 

	do 
	{
		html_action_work.page_loaded = FALSE; 
		wait_ret = _browser_safe_navigate_end( &g_web_browser_window->m_WebBrowser, 
			g_web_browser_window->get_page_load_time(), 
			&html_action_work.page_loaded ); 

		Sleep( g_web_browser_window->get_output_delay_time() ); 
		ret = g_web_browser_window->SendMessageW( WM_HTML_ACTION, ( WPARAM )0, ( LPARAM )param ); 

		wait_ret = WaitForSingleObject( data_scramble_worker.thread_handle, 0 ); 
		if( wait_ret == WAIT_OBJECT_0 
			|| wait_ret == WAIT_FAILED )
		{
			_ret = check_woker_job_exist( &job_worker ); 
			if( _ret != ERROR_SUCCESS )
			{
				REMOTE_ERROR_DEBUG(); 
				ASSERT( FALSE ); 
				worker_job_end( &job_worker); 
				ret = ERROR_STOP_THREAD; 
			}
		}

	}while( FALSE );

	return ( ULONG )ret; 
}


void web_browser_window::WebBrowserNavigateError(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}

void web_browser_window::OnLButtonUp(UINT nFlags, CPoint point)
{
	//INT32 ret; 
	//POINT pt; 
	// TODO: Add your message handler code here and/or call default

	do 
	{
		//ret = GetCursorPos(&pt); 
		//if( FALSE == ret )
		//{
		//	break; 
		//}

		//ClientToScreen( &pt ); 
		//m_WebBrowser.ScreenToClient( &pt ); 

		//get_active_element_xpath()
	}while( FALSE );

	CDialog::OnLButtonUp(nFlags, point);
}

HCURSOR web_browser_window::OnQueryDragIcon()
{
	return (HCURSOR) NULL;
}

HRESULT WINAPI highlight_html_element( IHTMLElement *element )
{
	HRESULT hr = S_OK; 
	_bstr_t name; 
	_bstr_t _value; 
	_variant_t value; 
	//ULONG color; 

	ASSERT( NULL != element ); 

	//color = HIGHT_BG_COLOR; 

	do 
	{
		name = L"style"; 
		//value.vt = VT_UI4; 
		//value.uintVal = color; 

		_value = "\"background-color:#9fe6ac\""; 

		value = _value.GetBSTR(); 

		hr = element->setAttribute( name.GetBSTR(), value.GetVARIANT() ); 

	}while( FALSE );

	return hr; 
}

LRESULT web_browser_window::unhighlight_html_element()
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr elements; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	//ULONG element_count; 
	LONG _sub_element_count; 
	_bstr_t temp_text; 
	_variant_t name; 
	_variant_t index; 
	INT32 i; 
	INT32 find_count; 

	BOOLEAN sub_element_got = FALSE; 

	do 
	{
		break; 

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_all( &elements ); 
		if( FAILED( hr ) ) 
		{
			break; 
		}

		if( NULL == elements )
		{
			break; 
		}

		//hr = elements->get_length( &element_count ); 
		//if( FAILED( hr ) )
		//{
		//	break; 
		//}

		temp_text = HTML_HIGHLIGT_ELEMENT_ID; 

		name = temp_text.GetBSTR(); 

		find_count = 0; 

		for( ; ; )
		{

			V_VT( &index ) = VT_I4;
			V_I4( &index ) = find_count; 

			find_count += 1; 

			_hr = elements->item( name, index, &sub_element_disp ); 
			if( FAILED( _hr ) )
			{
				break; 
			}

			if( NULL == sub_element_disp )
			{
				//_hr = E_UNEXPECTED; 
				break; 
			}

			do 
			{
				hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
				if( FAILED( hr )
					|| sub_elements == NULL )
				{
					break; 
				}

				sub_element_got = TRUE; 

				hr = sub_elements->get_length( &_sub_element_count ); 
				if( FAILED( hr ) ) 
				{
					break; 
				}

				for( i = 0; i < _sub_element_count; i ++ )
				{
					do 
					{
						V_VT( &name ) = VT_I4;
						V_I4( &name ) = i; 

						//comment it.
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( name, index, &sub_element_disp ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						if( sub_element_disp == NULL )
						{
							hr = E_UNEXPECTED; 
							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( SUCCEEDED( hr )
							&& sub_element != NULL )
						{
							{
								LPWSTR _temp_text; 
								hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
								if( FAILED(hr))
								{
									break; 
								}

								_temp_text = temp_text.operator wchar_t*(); 
								if( NULL == _temp_text )
								{
									hr = E_UNEXPECTED; 
									break; 
								}

								hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
								if( FAILED(hr))
								{
									break; 
								}
							}
						}
					}while( FALSE ); 
				}
			} while ( FALSE );

			if( TRUE == sub_element_got )
			{
				break; 
			}

			hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( sub_element == NULL )
			{
				hr = E_UNEXPECTED; 
				break; 
			}

			{
				LPWSTR _temp_text; 
				hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				}

				_temp_text = temp_text.operator wchar_t*(); 
				if( NULL == _temp_text )
				{
					hr = E_UNEXPECTED; 
					break; 
				}

				hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
				if( FAILED(hr))
				{
					break; 
				}
			}
		}
	}while( FALSE ); 

	return hr; 
}

LRESULT web_browser_window::on_html_element_hilight( WPARAM wparam, LPARAM lparam) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR xpath; 
	xpath = ( LPCWSTR )lparam; 
	ASSERT( NULL != xpath ); 
	return highlight_html_element_by_xpath( ( LPCWSTR )wparam, xpath ); 
}

typedef struct _LOCATE_PARAM
{
	wstring xpath; 
	LONG x; 
	LONG y; 
} LOCATE_PARAM, *PLOCATE_PARAM; 

typedef vector< LOCATE_PARAM* > LOCATE_PARAMS; 

LRESULT WINAPI parse_locate_param( LPCWSTR param, LOCATE_PARAM *locate_params )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR param_begin; 
	LPWSTR param_end; 
	LPWSTR temp_text; 
	LPWSTR _param = NULL; 
	ULONG param_len; 

#define LOCATE_PARAM_EQUAL_SIGN L"=="

	do 
	{
		ASSERT( NULL != param ); 

		param_len = wcslen( param ); 
		if( param_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_param = ( LPWSTR )malloc( ( param_len + 1 ) << 1 ); 
		
		if( NULL == _param )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( _param, param, ( ( param_len + 1 ) << 1 ) ); 
		
		param_begin = _param; 

		param_end = wcsstr( param_begin, LOCATE_PARAM_EQUAL_SIGN ); 
		if( NULL == param_end )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*param_end = L'\0'; 
		locate_params->xpath = param_begin; 
		*param_end = L'='; 

		param_begin = param_end + 2; 

		param_end = wcschr( param_begin, L'x' ); 
		if( NULL == param_end )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( param_begin == param_end )
		{
			locate_params->x = 0; 
		}
		else
		{
			*param_end = L'\0'; 
			locate_params->x = wcstol( param_begin, &temp_text, 0 ); 
			*param_end = L'x'; 
		}

		param_begin = param_end + 1; 
		if( *param_begin == L'\0' )
		{
			locate_params->y = 0; 
			//ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		locate_params->y = wcstol( param_begin, &temp_text, 0 ); 

	}while( FALSE );

	if( NULL != _param )
	{
		free( _param ); 
	}

	return ret; 
}

LRESULT WINAPI parse_locate_params( LPCWSTR param, LOCATE_PARAMS *locate_params )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR param_begin; 
	LPWSTR param_end; 
	LOCATE_PARAM *locate_param; 
	LPWSTR _test = NULL; 

	do 
	{
		ASSERT( NULL != param ); 
		ASSERT( NULL != locate_params ); 

#define LOCATE_PARAM_DELIM ';'

		_test = ( LPWSTR )malloc( sizeof( WCHAR ) * ( wcslen( param ) + 1 ) ); 
		if( NULL == _test )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		wcscpy( _test, param ); 

		param_begin = _test; 
		for( ; ; )
		{
			param_end = wcschr( param_begin, LOCATE_PARAM_DELIM ); 
			if( NULL == param_end )
			{
				break; 
			}

			if( param_end == NULL )
			{
				locate_param = new LOCATE_PARAM(); 
				if( locate_param == NULL )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 

				}

				ret = parse_locate_param( param_begin, locate_param ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				locate_params->push_back( locate_param ); 
			}
		}
	}while( FALSE );

	if( NULL != _test )
	{
		free( _test ); 
	}

	return ret; 
}

LRESULT WINAPI locate_object_by_param( LOCATE_PARAM *param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != param ); 
	} while ( FALSE );

	return ret; 
}

LRESULT web_browser_window::highlight_html_element_by_xpath( LPCWSTR param, LPCWSTR xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLDocument2Ptr doc; 
	IDispatchPtr disp; 
	HTML_ELEMENT_GROUP group; 

	ASSERT( NULL != xpath ); 

	do 
	{	
		do 
		{
			LOCATE_PARAMS locate_params; 

			if( param == NULL )
			{
				break; 
			}

			ret = parse_locate_params( param, &locate_params ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			for( LOCATE_PARAMS::iterator it = locate_params.begin(); 
				it != locate_params.end(); 
				it ++ )
			{
				locate_object_by_param( ( *it ) ); 
			}
		}while( FALSE );

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == doc )
		{
			break; 
		}

		ret = get_html_elements_from_xpath( xpath, 
			doc, 
			&group, 
			FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//{
		//	CString element_text; 
		//	element_text.Format( L"共找到%u个链接", group.size() ); 
		//	SetDlgItemText( IDC_STATIC_ELEMENT_COUNT, element_text.GetBuffer() ); 
		//}

		_bstr_t html_text; 
		LPCWSTR text; 
		wstring _text; 

		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
			it != group.end(); 
			it ++ )
		{
			do 
			{
#if 1
				hr = ( *it )->get_tagName( html_text.GetAddress() ); 

				if( SUCCEEDED(hr)
					&& html_text.operator wchar_t*() != NULL 
					&& 0 != _wcsicmp( html_text.operator wchar_t*(), 
					L"a" ) )
				{
					add_html_element_behevior( ( *it ), element_behavior ); 
					break; 
				}

				hr = ( *it )->get_outerHTML( html_text.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				} 

				text = html_text.operator wchar_t*(); 
				if( NULL == text )
				{
					break; 
				}

				_text = L"<span id=\"" HTML_HIGHLIGT_ELEMENT_ID L"\" style=\"background-color:#9fe6ac\">"; 
				_text += text; 
				_text += L"</span>"; 

				html_text = _text.c_str(); 

				//html_text = L""; 

				hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
				if( FAILED(hr))
				{
					break; 
				} 

				//hr = highlight_html_element( ( *it) ); 
#endif //0
			} while ( FALSE ); 
		}

		//for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
		//	it != group.end(); 
		//	it ++ )
		//{
		//	do 
		//	{
		//		hr = ( *it )->get_innerText( html_text.GetAddress() ); 
		//		if( FAILED(hr))
		//		{
		//			break; 
		//		} 

		//		text = html_text.operator wchar_t*(); 
		//		if( NULL == text )
		//		{
		//			break; 
		//		}

		//		//html_text = L""; 

		//		hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
		//		if( FAILED(hr))
		//		{
		//			break; 
		//		} 

		//		//hr = highlight_html_element( ( *it) ); 

		//	} while ( FALSE );
		//}

	}while( FALSE );

	release_html_element_group( &group ); 

	return ret; 
}
