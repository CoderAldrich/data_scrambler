// sql_conn_setting_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "sql_conn_setting_dlg.h"


// sql_conn_setting_dlg 對話方塊

IMPLEMENT_DYNAMIC(sql_conn_setting_dlg, CDialog)

sql_conn_setting_dlg::sql_conn_setting_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(sql_conn_setting_dlg::IDD, pParent)
{

}

sql_conn_setting_dlg::~sql_conn_setting_dlg()
{
}

void sql_conn_setting_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL sql_conn_setting_dlg::OnInitDialog()
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 _ret; 

	CDialog::OnInitDialog();

	WCHAR text[ MAX_PATH ]; 

	StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", conn_param.port ); 

	SetDlgItemText( IDC_EDIT_SQL_SERVER_IP, conn_param.ip.c_str() ); 
	SetDlgItemText( IDC_EDIT_SQL_SERVER_PORT, text ); 
	SetDlgItemText( IDC_EDIT_SQL_DATA_BASE, conn_param.data_base_name.c_str() ); 
	SetDlgItemText( IDC_EDIT_SQL_USER_NAME, conn_param.user_name.c_str() ); 
	SetDlgItemText( IDC_EDIT_SQL_PASSWORD, conn_param.password.c_str() ); 

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void sql_conn_setting_dlg::OnOK()
{
	LRESULT ret = ERROR_SUCCESS; 
	CString text; 
	LPWSTR temp_text; 

	do 
	{
		GetDlgItemText( IDC_EDIT_SQL_SERVER_IP, text ); 
		if( text.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			GetDlgItem( IDC_EDIT_SQL_SERVER_IP )->SetFocus(); 
			MessageBox( L"需要数据库服务器的IP地址" ); 
			break; 
		}

		conn_param.ip = text; 

		GetDlgItemText( IDC_EDIT_SQL_SERVER_PORT, text ); 
		if( text.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			GetDlgItem( IDC_EDIT_SQL_SERVER_PORT )->SetFocus(); 
			MessageBox( L"需要数据库服务器的端口号" ); 
			break; 
		}

		conn_param.port = ( USHORT )wcstoul( text, &temp_text, 0 ); 

		GetDlgItemText( IDC_EDIT_SQL_DATA_BASE, text ); 
		if( text.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			GetDlgItem( IDC_EDIT_SQL_SERVER_PORT )->SetFocus(); 
			MessageBox( L"需要数据库名" ); 
			break; 
		}

		conn_param.data_base_name = text; 

		GetDlgItemText( IDC_EDIT_SQL_USER_NAME, text ); 
		if( text.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			GetDlgItem( IDC_EDIT_SQL_SERVER_PORT )->SetFocus(); 
			MessageBox( L"需要数据库用户名" ); 
			break; 
		}

		conn_param.user_name = text; 

		GetDlgItemText( IDC_EDIT_SQL_PASSWORD, text ); 
		conn_param.password = text; 
	}while( FALSE );

	CDialog::OnOK(); 
}

void sql_conn_setting_dlg::OnCancel()
{
	CDialog::OnCancel(); 
}

BEGIN_MESSAGE_MAP(sql_conn_setting_dlg, CDialog)
END_MESSAGE_MAP()


// sql_conn_setting_dlg 訊息處理常式
