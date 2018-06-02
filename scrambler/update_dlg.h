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
#pragma once

#include "update.h"
#include "util.h"
#include "afxcmn.h"
#include "afxwin.h"
// update_dlg dialog

LRESULT CALLBACK updating_status_notify( updating_info *info, PVOID param ); 

class update_dlg : public CDialog
{
	DECLARE_DYNAMIC(update_dlg)

public:
	update_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~update_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK()
	{
		can_update = TRUE; 
		ASSERT( update_reponse != NULL ); 
		SetEvent( update_reponse ); 
	}

	virtual void OnCancel()
	{
		can_update = FALSE; 
		PostQuitMessage( 0xcecd ); 
		ExitProcess( 0 ); 
		//CDialog::OnCancel(); 
	}

	DECLARE_MESSAGE_MAP()

public:
	VOID set_update_url( LPCWSTR url )
	{
		update_url = url; 
	}

	VOID set_target_path( LPCWSTR target )
	{
		INT32 index; 
		ASSERT( target != NULL ); 

		//target_path = _T(""); 
		target_path = target; 
		index = target_path.ReverseFind( _T( '\\' ) ); 
		if( index < 0 )
		{
			goto _return; 
		}

		ASSERT( index < target_path.GetLength() ); 

		if( index < target_path.GetLength() - 1 )
		{
			target_path.SetAt( index + 1, _T( '\0' ) ); 
		}

_return:
		return;
	}

	VOID on_check_update( LPCWSTR url )
	{
		update_url = url; 
	}

	VOID on_file_download_tip( LPCWSTR file_name, ULONG file_size, ULONG downloaded )
	{
		LPCTSTR tmp_text; 

#define FILE_DOWNLOAD_TIP_FMT _T( "文件 已下载 %d/%d" )
		CString download_tip; 

		if( downloaded == 0 )
		{
		}

		tmp_text = FILE_DOWNLOAD_TIP_FMT; 

		download_tip.Format( tmp_text, 
			//file_name, 
			downloaded, 
			file_size ); 

		SetDlgItemText( IDC_STATIC_FILE_UPDATE_TIP, download_tip.GetBuffer() ); 

		update_progress.SetRange32( 0, file_size ); 
		update_progress.SetStep(1); 

		update_progress.SetPos( downloaded ); 
	}

	VOID on_update_failed( update_tip_info *tip_info)
	{
		//wait_update_end( update_handle ); 

		LPCTSTR tmp_text; 

		ASSERT( tip_info != NULL ); 

		if( tip_info->err_msg == NULL 
			&& *tip_info->err_msg == L'\0' )
		{
			ASSERT( FALSE ); 

			tmp_text = L"更新失败"; 
			tip_info->err_msg = tmp_text; 
		}

		//SetDlgItemText( IDC_STATIC_WHOLE_UPDATE_TIP, tmp_text ); 

		if( IsWindowVisible() == FALSE )
		{
			PostQuitMessage( 0xce00 ); 
			ExitProcess( 0xce00 ); 
			//CloseWindow(); 
			goto _return; 
		}
 
		//m_pCloseBtn->SetEnabled( true ); 
		//tip_text->SetText( tip_info->err_msg ); //请检查因特网连接.
_return:
		return; 
	}

#define UPDATE_DRV_EVENT_NAME _T( "bitsafe_drv_update" )
	LRESULT update_drv_files()
	{
		LRESULT ret = ERROR_SUCCESS; 
		CString exe_file_name; 
		HANDLE update_event = NULL; 
		ULONG wait_ret; 

		exe_file_name = _T( "\"" ); 

		exe_file_name += target_path; 

		if( exe_file_name.GetAt( exe_file_name.GetLength() - 1 ) != _T( '\\' ) )
		{
			exe_file_name += _T( "\\" ); 
		}

		exe_file_name += _T( "bitsafev2.exe\" " ) _T( "/updated" ); 

		update_event = CreateEvent( NULL, TRUE, FALSE, UPDATE_DRV_EVENT_NAME ); 
		if( update_event == NULL )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		ret = run_proc_sync( exe_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			goto _return; 
		}

		wait_ret = WaitForSingleObject( update_event, INFINITE ); 
		if( wait_ret != WAIT_OBJECT_0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

_return:
		if( update_event != NULL )
		{
			CloseHandle( update_event ); 
		}

		return ret; 
	}

	VOID on_update_successful()
	{
		LRESULT ret; 
		LPCTSTR tmp_text; 

		//wait_update_end( update_handle ); 
		//m_pCloseBtn->SetEnabled( true ); 

		tmp_text = _T( "更新成功" ); 

		//tip_text->SetText( tmp_text ); 

		ret = update_drv_files(); 

		if( ret != ERROR_SUCCESS )
		{
				ASSERT( FALSE ); 

				tmp_text = _T( "旧版本没有完成全部卸载,重新运行程序(新版本)时,要先执行卸载(如出现安装提示,先不要执行安装),在保证卸载全部完成后,再重新安装并运行程序。" ); 

				MessageBox( tmp_text ); 
				goto _return; 
		}

_return:
		return; 
	}

	LRESULT on_file_updated( LPCWSTR file_name )
	{
		LRESULT ret = ERROR_SUCCESS;
		LPCTSTR tmp_text; 

		CString _file_name; 
		updating_file_name = file_name; 
		file_name = wcsrchr( file_name, L'\\' ); 
		if( file_name == NULL )
		{
			file_name = L""; 
		}

		file_name += 1; 

		tmp_text = L"正在更新第%d个文件%s\n"; 

		_file_name.Format( tmp_text, updated_file_count + 1, file_name ); 
		SetDlgItemText( IDC_STATIC_UPDATE_TIP, _file_name.GetBuffer() ); 

		whole_update_progress.SetPos( updated_file_count ++ );

		return ret; 
	}

	ULONG get_update_file_num()
	{
		return update_file_count; 
	}

	LRESULT on_no_need_update( update_tip_info *info )
	{
		LRESULT ret = ERROR_SUCCESS; 
		CString tip; 
		LPWSTR version = NULL; 
		LPCTSTR tmp_text; 

		version = StringConvertor::Utf8ToWide( info->version ); 

		//DBG_BP(); 

#ifdef _DEBUG
		tmp_text = _T( "已经是最新的版本 %s, 不需要进行更新" ); 

		tip.Format( tmp_text, version );

		SetDlgItemText( IDC_STATIC_UPDATE_TIP, tip.GetBuffer() ); 

		ShowWindow( SW_SHOW ); 
#else
		//DestroyWindow(); 
		//PostQuitMessage( 0 ); 
		//Close(); 
#endif //_DEBUG

		//DestroyWindow(); 
		PostQuitMessage( 0 ); 
		ExitProcess( 0 ); 
_return:
		if( version != NULL )
		{
			StringConvertor::StringFree( version ); 
		}

		return ret; 
	}

	LRESULT on_update_starting( update_tip_info *info )
	{
		LRESULT ret = ERROR_SUCCESS; 
		LPWSTR version = NULL; 
		LPWSTR msg = NULL; 
		CString tip; 
		LPCTSTR tmp_text; 
		CWnd *control; 

		ASSERT( info != NULL ); 

		update_file_count = info->count; 

		version = StringConvertor::Utf8ToWide( info->version ); 
		if( version == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		msg = StringConvertor::Utf8ToWide( info->msg ); 
		if( msg == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		tmp_text = _T( "检测到更新的版本 %s (%s), 是否进行下载?(将下载至程序所在目录,请关闭程序,覆盖程序原文件)" ); 

		tip.Format( tmp_text, version, msg );

		ShowWindow( SW_SHOW ); 

		SetDlgItemText( IDC_STATIC_UPDATE_TIP, tip.GetBuffer() ); 

		control = GetDlgItem( IDOK ); 
		ASSERT( NULL != control );
		control->EnableWindow( TRUE ); 

		control = GetDlgItem( IDCANCEL ); 
		if( NULL != control )
		{
			control->EnableWindow( TRUE ); 
		}

		WaitForSingleObject( update_reponse, INFINITE ); 

		if( can_update == TRUE )
		{
			control = GetDlgItem( IDOK ); 
			ASSERT( NULL != control );
			control->EnableWindow( FALSE ); 

			control = GetDlgItem( IDCANCEL ); 
			if( NULL != control )
			{
				control->EnableWindow( FALSE ); 
			}

			ret = ERROR_SUCCESS; 

			whole_update_progress.SetRange32( 0, update_file_count - 1 ); 
			whole_update_progress.SetStep(1); 

			tmp_text = _T( "开始更新,共有%d个文件" ); 

			tip.Format( tmp_text, update_file_count ); 
			SetDlgItemText( IDC_STATIC_WHOLE_UPDATE_TIP, tip.GetBuffer() ); 
		}
		else
		{
			PostQuitMessage( 0 ); 
			ExitProcess( 0 ); 
		}

_return:
		if( version != NULL )
		{
			StringConvertor::StringFree( version ); 
		}

		if( msg != NULL )
		{
			StringConvertor::StringFree( msg ); 
		}

		return ret; 
	}

	VOID inc_updated_file_num()
	{
		updated_file_count ++; 
	}

	LPCWSTR get_update_url()
	{
		return ( LPCWSTR )update_url.GetBuffer(); 
	}

	void Init() 
	{
		LRESULT ret = ERROR_SUCCESS; 
		CHAR *_update_url = NULL;
		CHAR *_target_path = NULL; 
		CWnd *control; 

		ShowWindow( SW_HIDE ); 

		control = GetDlgItem( IDOK ); 
		ASSERT( NULL != control );
		control->EnableWindow( FALSE ); 

		control = GetDlgItem( IDCANCEL ); 
		if( NULL != control )
		{
			control->EnableWindow( TRUE ); 
		}

		if( update_url.IsEmpty() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_update_url = StringConvertor::WideToAnsi( update_url.GetBuffer() ); 
		if( _update_url == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		_target_path = StringConvertor::WideToAnsi( target_path.GetBuffer() ); 
		if( _target_path == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		update_handle = start_auto_update( _update_url, _target_path, updating_status_notify, ( PVOID )this ); 
		if( update_handle == NULL )
		{
			log_trace( ( MSG_ERROR, "start update thread failed \n" ) ); 

			ret = ERROR_ERRORS_ENCOUNTERED; 
			goto _return; 
		}

		update_reponse = CreateEvent( NULL, FALSE, FALSE, NULL ); 

		if( update_reponse == NULL )
		{
			log_trace( ( MSG_ERROR, "create response event failed 0x%0.8x \n", GetLastError() ) ); 

			ret = GetLastError(); 
			goto _return; 
		}

_return:
		if( _update_url != NULL )
		{
			StringConvertor::StringFree( _update_url ); 
		}

		if( _target_path != NULL )
		{
			StringConvertor::StringFree( _target_path ); 
		}

		if( ret != ERROR_SUCCESS )
		{

			CloseWindow(); 
		}

		return; 
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;

		if( update_handle != NULL )
		{
			CloseHandle( update_handle ); 
			update_handle = NULL; 
		}

		if( update_reponse != NULL )
		{
			CloseHandle( update_reponse ); 
			update_reponse = NULL; 
		}

		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE; 

		CString file_name; 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		PostQuitMessage( 0 ); 
		ExitProcess( 0 ); 
		return 0;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CRect rcWork = oMonitor.rcWork;
		rcWork.OffsetRect(-rcWork.left, -rcWork.top);

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x	= rcWork.left;
		lpMMI->ptMaxPosition.y	= rcWork.top;
		lpMMI->ptMaxSize.x		= rcWork.right;
		lpMMI->ptMaxSize.y		= rcWork.bottom;

		bHandled = FALSE;
		return 0;
	}

public:

private:
	CString update_url; 
	CString target_path; 
	CString updating_file_name; 
	ULONG update_file_count; 
	ULONG updated_file_count; 
	PVOID update_handle; 
	HANDLE update_reponse; 
	INT32 can_update; 
public:
	CProgressCtrl update_progress;
	CProgressCtrl whole_update_progress;
}; 

LRESULT WINAPI auto_update(); 