// update_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_scrambler.h"
// update_dlg message handlers
#include "common_func.h"
#include "update.h"
#include "update_dlg.h"

// update_dlg dialog

IMPLEMENT_DYNAMIC(update_dlg, CDialog)

update_dlg::update_dlg(CWnd* pParent /*=NULL*/)
: CDialog(update_dlg::IDD, pParent)
{
	can_update = FALSE; 
	updating_file_name = _T(""); 
	update_file_count = 0; 
	updated_file_count = 0; 
	update_handle = NULL; 
	update_reponse = NULL; 
}

update_dlg::~update_dlg()
{
}

void update_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_FILE_UPDATE, update_progress);
	DDX_Control(pDX, IDC_PROGRESS_WHOLE_UPDATE, whole_update_progress);
}

BOOL update_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	//UpdateData(FALSE);

	Init(); 

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(update_dlg, CDialog)
END_MESSAGE_MAP()

LRESULT CALLBACK updating_status_notify( updating_info *info, PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	update_dlg *dlg; 
	LPWSTR _update_url = NULL; 
	LPWSTR _file_name = NULL; 
	update_tip_info *tip_info; 
	file_download_tip *download_tip; 

	ASSERT( info != NULL ); 
	ASSERT( param != NULL ); 

	dlg = ( update_dlg* )param; 

	switch( info->update_status )
	{
	case UPDATE_PREPARE: 
		tip_info = ( update_tip_info* )info->context; 
		ASSERT( tip_info->url != NULL ); 
		if( *tip_info->url == '\0' )
		{
			log_trace( ( MSG_ERROR, "the update url is invalid\n" ) ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_update_url = StringConvertor::AnsiToWide( tip_info->url ); 
		if( _update_url == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		dlg->on_check_update( _update_url ); 
		break; 
	case UPDATE_FILE_DOWNLOAD_TIP: 

		tip_info = ( update_tip_info* )info->context; 

		ASSERT( tip_info->context != NULL ); 

		download_tip = ( file_download_tip* )tip_info->context; 

		if( *download_tip->file_name== '\0' )
		{
			log_trace( ( MSG_ERROR, "the update file is invalid\n" ) ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_file_name = StringConvertor::AnsiToWide( ( LPCSTR )download_tip->file_name ); 
		if( _file_name == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		dlg->on_file_download_tip( _file_name, download_tip->file_size, download_tip->file_downloaded ); 
		break; 
	case UPDATE_NO_NEED: 
		tip_info = ( update_tip_info* )info->context;  

		ret = dlg->on_no_need_update( tip_info );  
		break; 
	case UPDATE_RUNNING:
		tip_info = ( update_tip_info* )info->context;  

		ret = dlg->on_update_starting( tip_info );  
		break; 
	case UPDATE_FILE_STARTING: 
		//tip_info = ( update_tip_info* )info->context; 
		break; 
	case UPDATE_FILE_END:
		tip_info = ( update_tip_info* )info->context; 

		if( tip_info->context == NULL 
			|| *( CHAR* )tip_info->context == '\0' )
		{
			ret = ERROR_INVALID_PARAMETER; 
			goto _return; 
		}

		_file_name = StringConvertor::AnsiToWide( ( LPCSTR )tip_info->context ); 
		if( _file_name == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			goto _return; 
		}

		ret = dlg->on_file_updated( _file_name ); 
		break; 
	case UPDATE_FAILED:
		tip_info = ( update_tip_info* )info->context; 
		dlg->on_update_failed( tip_info ); 
		break; 
	case UPDATE_SUCCESSFUL:
		//tip_info = ( update_tip_info* )info->context; 
		dlg->on_update_successful(); 
		break; 
	default: 
		ASSERT( FALSE && "invalid updating status\n"); 
		break; 
	}

_return:
	if( _update_url != NULL )
	{
		StringConvertor::StringFree( _update_url ); 
	}
	if( _file_name != NULL )
	{
		StringConvertor::StringFree( _file_name ); 
	}

	return ret; 
}

LRESULT WINAPI auto_update()
{
	LRESULT ret = ERROR_SUCCESS; 

	LPWSTR update_path = NULL; 
	WCHAR _app_file_path[ MAX_PATH ]; 
	ULONG cc_path_len; 
	//PVOID update_handle; 
	LPCTSTR tmp_text; 
	update_dlg dlg; 

	do 
	{
		update_path = ( LPWSTR )malloc( _MAX_URL_LEN << 1 ); 
		//ret = init_crash_rpt(); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	log_trace( ( MSG_ERROR, "initialize the crash report failed" ) ); 

		//	goto _return; 
		//}

		get_app_path( _app_file_path, ARRAYSIZE( _app_file_path ), &cc_path_len ); 
		log_trace( ( MSG_INFO, "update target path is %ws \n", _app_file_path ) ); 

		if( *_app_file_path == L'\0' )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			goto _return;; 
		}

		//ret = check_app_instance( UPDATE_INSTANCE ); ; 
		//if( ret == ERROR_SUCCESS )
		//{
		//	goto _return; 
		//}

		//ret = get_update_path( update_path, _MAX_URL_LEN ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	goto _return; 
		//}

#define DATA_SCRAMBLER_UPDATE_URL L"http://www.simplestai.com/webkits/update/update.php?action=auto"  
		StringCchCopyW( update_path, _MAX_URL_LEN, DATA_SCRAMBLER_UPDATE_URL ); 

		dlg.set_update_url( update_path ); 
		dlg.set_target_path( _app_file_path ); 

		//tmp_text = _T("更新比特安全"); 

		dlg.Create( MAKEINTRESOURCE( dlg.IDD ), NULL ); 


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

		goto _return; 
	}while( FALSE );
_return: 
	if( NULL != update_path )
	{
		free( update_path ); 
	}

	ret = ERROR_ERRORS_ENCOUNTERED; 
	return ret; 
}