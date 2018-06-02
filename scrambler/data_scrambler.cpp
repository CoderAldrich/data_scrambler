/*
 *
 * Copyright 2010 JiJie Shi
 *
 * This file is part of data_scrambler.
 *
 * data_scrambler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * data_scrambler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with data_scrambler.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// html_maker_config.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "html_maker_configDlg.h"
#include "html_element_prop_dlg.h"
#include "browser_dlg.h"
#include "html_script_config_dlg.h"
#include "web_browser_window.h"
#include "webbrowser.h"
#include "update_dlg.h"
#include "user_reg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(Cdata_scramblerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

#include <shlobj.h>
LRESULT WINAPI _create_link( LPCWSTR file_name, 
							LPCWSTR arguments, 
					LPCWSTR link_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hres ;
	IShellLink *psl = NULL;
	IPersistFile *ppf = NULL ;
	BOOLEAN com_inited = FALSE; 

	//WORD wsz[ MAX_PATH] ;
	hres = CoInitialize( NULL ); 
	if( FAILED( hres ) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	com_inited = TRUE; 

	hres = CoCreateInstance( CLSID_ShellLink, 
		NULL,
		CLSCTX_INPROC_SERVER, 
		IID_IShellLink,
		( void ** )&psl ) ;

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	psl->SetPath( file_name ); 
	
	if( arguments != NULL )
	{
		psl->SetArguments( arguments ); 
	}

	//psl ->SetHotkey( MAKEWORD( 'R',
	//	HOTKEYF_SHIFT | HOTKEYF_CONTROL ) ) ; 

	hres = psl->QueryInterface( IID_IPersistFile,
		(void**)&ppf ); 

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	
	//MultiByteToWideChar( CP_ACP, 
	//	0, 
	//	link_file_name, 
	//	-1, 
	//	link_file_name, 
	//	MAX_PATH ); 

	hres = ppf->Save( link_file_name, STGM_READWRITE ); 

	if( FAILED( hres) )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

_return:

	if( ppf != NULL )
	{
		ppf->Release();
	}
	
	if( psl != NULL )
	{
		psl->Release();
	}

	if( com_inited == TRUE )
	{
		CoUninitialize(); 
	}

	return ret;
}

LRESULT WINAPI notify_shell( LONG event_id, LPCWSTR dest_path )
{    
	LPWSTR tmp;

	SHChangeNotify( event_id,
		SHCNF_FLUSH | SHCNF_PATH,
		dest_path, 
		0 ); 

	tmp = ( LPWSTR )dest_path + lstrlen( dest_path ) - 1; 

	for( ; *tmp != L'\\'; tmp--); 

	*tmp = '\0'; 

	SHChangeNotify( SHCNE_UPDATEDIR | SHCNE_INTERRUPT, 
		SHCNF_FLUSH | SHCNF_PATH, 
		dest_path, 0 ); 

	return ERROR_SUCCESS; 
}

LRESULT WINAPI create_link( LPCWSTR file_name, LPCWSTR arguments, LPCWSTR link_file_name )
{
	LRESULT ret; 

	ret = _create_link( file_name, 
		arguments, 
		link_file_name ); 

	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}


	notify_shell( SHCNE_CREATE|SHCNE_INTERRUPT,  
		link_file_name ); 

_return:
	return ret; 
}

// Cdata_scramblerApp construction

Cdata_scramblerApp::Cdata_scramblerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Cdata_scramblerApp object

Cdata_scramblerApp theApp;

// Cdata_scramblerApp initialization

LRESULT WINAPI scrambler_clone_start_self()
{
#define CMD_LINE_LEN 320
#define _TMP_FILE_LEN 64
	LRESULT ret = ERROR_SUCCESS; 
	TCHAR mod_path[ MAX_PATH ]; 
	TCHAR clone_path[ MAX_PATH ]; 
	TCHAR cmd_line[ CMD_LINE_LEN ]; 
	//TCHAR tmp_file_name[ _TMP_FILE_LEN ]; 
	//HANDLE process = NULL; 
	DWORD _ret; 
	HANDLE clone_file = INVALID_HANDLE_VALUE; 
	STARTUPINFO clone_start_info; 
	PROCESS_INFORMATION clone_process_info = { 0 };
	DWORD wait_ret; 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

	_ret = GetModuleFileName( NULL, mod_path, MAX_PATH ); 
	if( _ret == 0 )
	{
		ret = GetLastError(); 
		goto _return; 
	}

	_tcscpy( clone_path, mod_path ); 

	//ret = rand_string( tmp_file_name, _TMP_FILE_LEN ); 

	//if( ret != ERROR_SUCCESS )
	//{
	//	goto _return; 
	//}

	//_ret = add_app_path_to_file_name( clone_path, MAX_PATH, tmp_file_name, FALSE ); 
	//if( _ret != ERROR_SUCCESS )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	//ret = get_tmp_file_name( clone_path, MAX_PATH ); 
	//if( ret != ERROR_SUCCESS )
	//{
	//	goto _return; 
	//}

	//_ret = add_app_path_to_file_name( clone_path, MAX_PATH, UPDATE_FILE_NAME, FALSE ); 
	//if( _ret != ERROR_SUCCESS )
	//{
	//	ret = ERROR_ERRORS_ENCOUNTERED; 
	//	goto _return; 
	//}

	//_ret = CopyFile( mod_path, clone_path, FALSE );
	//if( _ret == FALSE )
	//{
	//	ret = GetLastError(); 
	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "copy clone file %ws failed \n", clone_path ) ); 

	//}

	// Open the clone EXE using FILE_FLAG_DELETE_ON_CLOSE
	//clone_file = CreateFile( clone_path, 
	//	0, 
	//	FILE_SHARE_READ, 
	//	NULL, 
	//	OPEN_EXISTING, 
	//	FILE_FLAG_DELETE_ON_CLOSE, 
	//	NULL ); 

	//if( clone_file == INVALID_HANDLE_VALUE )
	//{
	//	ret = GetLastError(); 
	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "open the clone file failed \n" ) ); 
	//	goto _return; 
	//}

	// Spawn the clone EXE passing it our EXE's process handle
	// and the full path name to the Original EXE file.

	//process = OpenProcess( SYNCHRONIZE, 
	//	TRUE, 
	//	GetCurrentProcessId() ); 

	//if( process == NULL )
	//{
	//	ret = GetLastError(); 
	//	goto _return; 
	//}


	ZeroMemory( &clone_start_info, sizeof( clone_start_info ));
	clone_start_info.cb = sizeof( clone_start_info ); 
	_ret = _sntprintf( cmd_line, CMD_LINE_LEN, _T( "%s -u" ), clone_path ); 

	if( ( INT32 )_ret < 0 )
	{
		ret = ERROR_BUFFER_OVERFLOW; 
		goto _return; 
	}

	_ret = ( BOOL )CreateProcess( NULL, 
		cmd_line, 
		NULL, 
		NULL, 
		TRUE, 
		0, 
		NULL, 
		NULL, 
		&clone_start_info, 
		&clone_process_info ); 

	if( _ret == FALSE )
	{
		ret = GetLastError(); 
		log_trace( ( DBG_MSG_AND_ERROR_OUT, "start the process from the clone file failed \n" ) ); 
		goto _return; 
	}

	//wait_ret = WaitForSingleObject( clone_process_info.hProcess, INFINITE ); 

	//if( wait_ret != WAIT_TIMEOUT 
	//	&& wait_ret != WAIT_OBJECT_0 
	//	&& wait_ret != WAIT_ABANDONED )
	//{
	//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait clone process failed\n" ) ); 
	//	ASSERT( FALSE ); 
	//}

_return:
	//if( process != NULL )
	//{
	//	CloseHandle( process );
	//}

	if( clone_file != INVALID_HANDLE_VALUE )
	{
		CloseHandle( clone_file ); 
	}

	if( clone_process_info.hProcess != NULL )
	{
		CloseHandle( clone_process_info.hProcess );
		ASSERT( clone_process_info.hThread != NULL ); 
		CloseHandle( clone_process_info.hThread );
	}

	log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 

	return ret; 
}

#include "welcome_dlg.h"
BOOL Cdata_scramblerApp::InitInstance()
{
	AfxEnableControlContainer();

	AfxOleInit();
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("DataScrambler"));

	do
	{
		LRESULT ret; 
		CString command_line; 
		INT32 arg_count; 
		LPWSTR *args; 
		LPCWSTR url = NULL; 

#define NO_CRASH_REPORT 1
#ifndef NO_CRASH_REPORT
		ret = init_crash_rpt(); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE && "initialize the crush report error\n" ); 
			//goto _return; 
		}
#endif  //NO_CRASH_REPORT

		//auto_update(); 

#if 0 //ndef DATA_SCRAMBLER_BROWSER
		//if( 0 ==  wcsicmp( args[ 1 ], L"-t" ) ) 
		{
			browser_test_dlg dlg; 
			m_pMainWnd = &dlg; 

			INT_PTR nResponse = dlg.DoModal();
			if (nResponse == IDOK)
			{
				// TODO: Place code here to handle when the dialog is
				//  dismissed with OK
			}
			else if (nResponse == IDCANCEL)
			{
				// TODO: Place code here to handle when the dialog is
				//  dismissed with Cancel
			}
			break; 
		}
#endif //DATA_SCRAMBLER_BROWSER

		config_ie_control_version(); 

		command_line = GetCommandLineW(); 
		args = CommandLineToArgvW( command_line.GetBuffer(), &arg_count ); 

		if( arg_count > 1 ) 
		{
			if(  0 ==  _wcsicmp( args[ 1 ], L"-u" ) )
			{
				ret = auto_update(); 
				if( ret == ERROR_SUCCESS )
				{
					break; 
				}

				break; 
			}

//#ifndef DATA_SCRAMBLER_BROWSER
			//if( 0 ==  _wcsicmp( args[ 1 ], L"-t" ) ) 
			//{
			//	browser_dlg dlg; 
			//	INT_PTR nResponse; 

			//	m_pMainWnd = &dlg; 

			//	scrambler_clone_start_self(); 

			//	nResponse = dlg.DoModal();
			//	if( nResponse == IDOK )
			//	{
			//		// TODO: Place code here to handle when the dialog is
			//		//  dismissed with OK
			//	}
			//	else if( nResponse == IDCANCEL )
			//	{
			//		// TODO: Place code here to handle when the dialog is
			//		//  dismissed with Cancel
			//	}

			//	break; 
			//}
//#endif //DATA_SCRAMBLER_BROWSER

			if( 0 == _wcsicmp( args[ 1 ], L"-x" ) ) 
			{
				INT32 ret; 
				web_browser_window dlg; 
				HWND manager_wnd = NULL; 
				//INT_PTR nResponse; 

				//if( 0 == wcsnicmp( L"http", CONST_STR_LEN( L"http" ) ) )
				{
				}

				//Chtml_maker_configDlg dlg;
				//m_pMainWnd = &dlg; 

				//for ( ; ; )
				//{
				//	Sleep(1); 
				//}

				//dlg.set_target_url( url ); 
				if( arg_count > 2 )
				{
					dlg.set_manager_pid( args[ 2 ] ); 
				}
				else
				{
					ASSERT( FALSE ); 
				}

#define WAIT_PAGE_LOAD_TIME_PARAM_INDEX 4
#define PROCESS_PAGE_DELAY_TIME_PARAM_INDEX 3
#define WORK_MODE_PARAM_INDEX 5
				if( arg_count > PROCESS_PAGE_DELAY_TIME_PARAM_INDEX )
				{
					dlg.set_output_delay_time( wcstoul( args[ PROCESS_PAGE_DELAY_TIME_PARAM_INDEX ], NULL, 10 ) ); 
				}

				if( arg_count > WAIT_PAGE_LOAD_TIME_PARAM_INDEX )
				{
					dlg.set_page_load_time( wcstoul( args[ WAIT_PAGE_LOAD_TIME_PARAM_INDEX ], NULL, 10 ) ); 
				}

				if( arg_count > WORK_MODE_PARAM_INDEX )
				{
					dlg.set_work_mode( args[ WORK_MODE_PARAM_INDEX ] ); 
				}

				manager_wnd = find_web_browser_window( dlg.get_manager_pid() ); 

				if( manager_wnd == NULL )
				{
					ASSERT( FALSE ); 
					break; 
				}

				ret = dlg.Create( MAKEINTRESOURCE(dlg.IDD), manager_wnd == NULL ? NULL : CWnd::FromHandle(manager_wnd) );
				if( FALSE == ret )
				{
					ASSERT( FALSE ); 
					break; 
				}

				dlg.ShowWindow( SW_SHOW ); 

				//if (nResponse == IDOK)
				//{
				//	// TODO: Place code here to handle when the dialog is
				//	//  dismissed with OK
				//}
				//else if (nResponse == IDCANCEL)
				//{
				//	// TODO: Place code here to handle when the dialog is
				//	//  dismissed with Cancel
				//}

				MSG msg; 
				//while (GetMessage(&msg, NULL, 0, 0))
				//{
				//	if( !IsDialogMessage(hDlgModeless, &msg))
				//	{
				//		TranslateMessage(&msg);
				//		DispatchMessage( &msg);
				//	}
				//}
				// Main message loop: 
				while (GetMessage(&msg, NULL, 0, 0)) 
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg); 
				} 

				break; 
			}
		}

		{
			wstring response; 
			post_data_to_server( response, 1, L"info", L"starting" ); 
		}

		show_welcome_info(); 
		//process_current_user_access(); 

#if 0
		do
		{
			LRESULT _ret; 
			HRESULT hr; 
			WCHAR mod_path[ MAX_PATH ]; 
			_ret = GetModuleFileName( NULL, mod_path, MAX_PATH ); 
			if( _ret == 0 )
			{
				ret = GetLastError(); 
				break; 
			}

#define SCRAMBLER_CONFIG_LINK_NAME L"web_browser_tool.lnk"
#define SCRAMBLER_CONFIG_LINK_ARGUMENTS L"-t"
			WCHAR link_path[ MAX_PATH ]; 
			ULONG cc_link_path_len; 
			ret = get_app_path( link_path, ARRAYSIZE( link_path ), &cc_link_path_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			hr = StringCchCat( link_path, ARRAYSIZE( link_path ), SCRAMBLER_CONFIG_LINK_NAME ); 
			if( FAILED( hr ))
			{
				break; 
			}

			ret = create_link( mod_path, SCRAMBLER_CONFIG_LINK_ARGUMENTS, link_path ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE ); 
#endif //0

		html_script_config_dlg dlg; 
		m_pMainWnd = &dlg; 

		scrambler_clone_start_self(); 

		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}

	}while( FALSE ); 

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
