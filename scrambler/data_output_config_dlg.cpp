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

// data_output_config_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "data_output_config_dlg.h"
#include "mysql_conn.h"
#include "sqlite_conn.h"

// data_output_config_dlg dialog

IMPLEMENT_DYNAMIC(data_output_config_dlg, CDialog)

data_output_config_dlg::data_output_config_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(data_output_config_dlg::IDD, pParent)
{
	to_data_center = FALSE; 
	output_link_list = FALSE; 
	share_script_file = FALSE; 
	m_pListEdit = NULL;
}

data_output_config_dlg::~data_output_config_dlg()
{
	if( m_pListEdit != NULL )
	{
		delete m_pListEdit; 
		m_pListEdit = NULL; 
	} 
}

void data_output_config_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_COMBO1, output_type);
	DDX_Control(pDX, IDC_LIST_HTML_DATA_SETS, data_sets);
	DDX_Control(pDX, IDC_LIST_HTML_DATA_SET, data_set);
	//DDX_Control(pDX, IDC_LIST_HTML_LOCATE_URL, locate_urls);
}


BEGIN_MESSAGE_MAP(data_output_config_dlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &data_output_config_dlg::OnBnClickedButtonSelectPath)
	ON_BN_CLICKED(IDC_BUTTON_DONATE, &data_output_config_dlg::OnBnClickedButtonDonate)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HTML_DATA_SETS, &data_output_config_dlg::OnNMDblclkListHtmlDataSets)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_HTML_DATA_SET, OnBeginlabeleditDataSetList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_HTML_DATA_SET, OnEndlabeleditDataSetList)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_HTML_DATA_SETS, OnBeginlabeleditDataSetsList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_HTML_DATA_SETS, OnEndlabeleditDataSetsList)
	//ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_HTML_LOCATE_URL, OnBeginlabeleditList)
	//ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_HTML_LOCATE_URL, OnEndlabeleditList)
	ON_BN_CLICKED(IDC_BUTTON_SQL_CONN_SETTING, &data_output_config_dlg::OnBnClickedButtonSqlConnSetting)
	ON_BN_CLICKED(IDC_BUTTON_SQL_CONN_CHECK, &data_output_config_dlg::OnBnClickedButtonSqlConnCheck)
	ON_NOTIFY(NM_CLICK, IDC_LIST_HTML_DATA_SETS, &data_output_config_dlg::OnNMClickListHtmlDataSets)
END_MESSAGE_MAP()

void data_output_config_dlg::OnEndlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	INT32 item; 
	INT32 subitem; 

	item = pDispInfo->item.iItem;
	subitem = pDispInfo->item.iSubItem;

	switch( control_id )
	{
	case IDC_LIST_HTML_DATA_SET:
		// This is coming from the grid list control notification.
		if( m_pListEdit )
		{
			HTML_ELEMENT_ACTION *content; 
			if( pDispInfo->item.pszText )
			{
				data_set.SetItemText( item, subitem, pDispInfo->item.pszText );

				content = ( HTML_ELEMENT_ACTION* )( PVOID )data_set.GetItemData( item ); 
				switch( subitem )
				{
				case 0:
					content->name = pDispInfo->item.pszText; 
					break; 
					//case 1:
					//	content->action->xpath = pDispInfo->item.pszText; 
					//	break; 
				default:
					break; 
				}
			}
		}
		break; 
	case IDC_LIST_HTML_DATA_SETS:
		// This is coming from the grid list control notification.
		if( m_pListEdit )
		{
			CONTAINER_INFO *container; 
			if( pDispInfo->item.pszText )
			{
				data_sets.SetItemText( item, subitem, pDispInfo->item.pszText );

				container = ( CONTAINER_INFO* )( PVOID )data_sets.GetItemData( item ); 
				//switch( subitem )
				//{
				//case 0:
				//	container->xpath = pDispInfo->item.pszText; 
				//	break; 
				//default:
				//	break; 
				//}
			}
		}
	default:
		break; 
	}
		
	if( NULL != m_pListEdit )
	{
		delete m_pListEdit;
		m_pListEdit = NULL;
	}

	*pResult = 0;
}

void data_output_config_dlg::OnBeginlabeleditDataSetList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnBeginlabeleditList( pNMHDR, IDC_LIST_HTML_DATA_SET, pResult ); 
}

void data_output_config_dlg::OnEndlabeleditDataSetList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnEndlabeleditList( pNMHDR, IDC_LIST_HTML_DATA_SET, pResult ); 
}

void data_output_config_dlg::OnBeginlabeleditDataSetsList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnBeginlabeleditList( pNMHDR, IDC_LIST_HTML_DATA_SETS, pResult ); 
}

void data_output_config_dlg::OnEndlabeleditDataSetsList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnEndlabeleditList( pNMHDR, IDC_LIST_HTML_DATA_SETS, pResult ); 
}
/*
CREATE TABLE user_post_info(
id INT PRIMARY KEY AUTO_INCREMENT ,
user_name VARCHAR( 64 ) ,
ip VARCHAR( 64 ) NOT NULL ,
info TEXT,
access_count INT,
access_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
*/

void data_output_config_dlg::OnBeginlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CString str = pDispInfo->item.pszText;
	INT32 item = pDispInfo->item.iItem;
	INT32 subitem = pDispInfo->item.iSubItem; 
	CRect  rect( 0,0,1,1 );
	DWORD dwStyle = ES_LEFT;
	
	do 
	{
		// Construct and create the custom multiline edit control.
		// We could just as well have used a combobox, checkbox, 
		// rich text control, etc.
		m_pListEdit = new CInPlaceEdit( item, subitem, str );
		if(NULL == m_pListEdit )
		{
			break; 
		}

		// Start with a small rectangle.  We'll change it later.
		dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL;

		switch( control_id )
		{
		case IDC_LIST_HTML_DATA_SET:
			m_pListEdit->Create( dwStyle, rect, &data_set, 103 );
			// Have the Grid position and size the custom edit control
			data_set.PositionControl( m_pListEdit, item, subitem );
			break; 
		case IDC_LIST_HTML_DATA_SETS:
			m_pListEdit->Create( dwStyle, rect, &data_sets, 103 );
			data_sets.PositionControl( m_pListEdit, item, subitem ); 
			break; 
		}

		// Have the edit box size itself to its content.
		m_pListEdit->CalculateSize(); 
		// Return TRUE so that the list control will hnadle NOT edit label itself. 
		*pResult = 1;
	}while( FALSE ); 
}

BOOL data_output_config_dlg::OnInitDialog()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	CDialog::OnInitDialog();

#if 0
	{
		CButton *control; 
		control = ( CButton* )GetDlgItem( IDC_CHECK_OUTPUT_TO_DATA_CENTER ); 
		ASSERT( control ); 
		control->SetCheck( 1 ); 

		control = ( CButton* )GetDlgItem( IDC_CHECK_SHARE_SCRIPT_TO_CENTER ); 
		control->SetCheck( 1 ); 
	}
#endif //0

#define DEFAULT_SQL_CONNECTION_STRING L"SERVER=127.0.0.1;PORT=3306;DATABASE=data;USER=root;PASSWORD=;"
	
	{
		wstring conn_string; 

		get_data_scrambler_conn_string( conn_string ); 
		
		if( conn_string.length() == 0 )
		{
			conn_string = DEFAULT_SQL_CONNECTION_STRING;  
		}

		SetDlgItemText( IDC_EDIT_SQL_CONN_STRING, conn_string.c_str() ); 
	}

	INT32 colume_index = 0; 
	data_sets.PrepareControl(0);
	data_sets.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );

	_ret = data_sets.InsertColumn( colume_index, _T("数据集合元素XPATH"), LVCFMT_LEFT, 130);
	if( _ret < 0 )
	{
		SAFE_SET_ERROR_CODE( ret ); 
	}
	else
	{
		colume_index ++; 
	}

	_ret = data_sets.InsertColumn( colume_index, _T("CSV输出文件路径"), LVCFMT_LEFT, 120);
	if( _ret < 0 )
	{
		SAFE_SET_ERROR_CODE( ret ); 
	}
	else
	{
		colume_index ++; 
	}

	_ret = data_sets.InsertColumn( colume_index, _T("输出数据库表名"), LVCFMT_LEFT, 120);
	if( _ret < 0 )
	{
		SAFE_SET_ERROR_CODE( ret ); 
	}
	else
	{
		colume_index ++; 
	}

	data_set.PrepareControl(0);
	data_set.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );

	colume_index = 0; 
	_ret = data_set.InsertColumn( colume_index, _T("名称"), LVCFMT_LEFT, 120);
	if( _ret < 0 )
	{
		SAFE_SET_ERROR_CODE( ret ); 
	}
	else
	{
		colume_index ++; 
	}

	_ret = data_set.InsertColumn( colume_index, _T("xpath"), LVCFMT_LEFT, 120);
	if( _ret < 0 )
	{
		SAFE_SET_ERROR_CODE( ret ); 
	}
	else
	{
		colume_index ++; 
	}

	CONTAINER_INFOS::iterator it; 

	do 
	{
		ASSERT( NULL != container_infos ); 
		for( it = container_infos->begin(); it != container_infos->end(); it ++ )
		{
			add_page_info_to_list( ( *it ) ); 
		}

		if( 0 < data_sets.GetItemCount() )
		{
			CONTAINER_INFO *container_info; 
			container_info = ( CONTAINER_INFO* )data_sets.GetItemData( 0 ); 
			if( NULL == container_info )
			{
				break; 
			}

			ret = load_data_set_info( container_info ); 
		}
	}while( FALSE ); 

	//output_type.InsertString( 0, L"csv格式文件" ); 
	//output_type.AddString( L"txt格式文件" ); 
	//output_type.InsertString( 1, L"数据库" ); 
	//output_type.SetCurSel( 0 ); 

	UpdateData(TRUE); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

VOID data_output_config_dlg::OnOK()
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 cur_sel; 
	INT32 _ret; 
	INT32 item_count; 
	INT32 i; 
	CString table_name; 
	CString csv_file_name; 
	CONTAINER_INFO *container_info; 
	wstring error_string; 

	do 
	{
		GetDlgItemText( IDC_EDIT_SQL_CONN_STRING, sql_conn_string ); 
		set_data_scrambler_conn_string( sql_conn_string.GetBuffer() ); 

		{
			CButton *control; 
			control = ( CButton* )GetDlgItem( IDC_CHECK_OUTPUT_TO_DATA_CENTER ); 

			ASSERT( control ); 
			if( control->GetCheck() )
			{
				to_data_center = TRUE; 
			}
			else
			{
				to_data_center = FALSE; 
			}

			control = ( CButton* )GetDlgItem( IDC_CHECK_OUTPUT_LINK_LIST ); 

			ASSERT( control ); 
			if( control->GetCheck() )
			{
				output_link_list = TRUE; 
			}
			else
			{
				output_link_list = FALSE; 
			}

			control = ( CButton* )GetDlgItem( IDC_CHECK_SHARE_SCRIPT_TO_CENTER ); 

			ASSERT( control ); 
			if( control->GetCheck() )
			{
				share_script_file = TRUE; 
			}
			else
			{
				share_script_file = FALSE; 
			}
		}

		//cur_sel = output_type.GetCurSel(); 
		//if( cur_sel < 0 )
		//{
		//	break; 
		//}

		item_count = data_sets.GetItemCount(); 
		for( i = 0; i < item_count; i ++ )
		{
			csv_file_name = data_sets.GetItemText( i, 1 ); 
			table_name = data_sets.GetItemText( i, 2 );

			if( csv_file_name.GetLength() == 0 
				&& table_name.GetLength() == 0 )
			{
				CString data_sets_name; 
				CString message; 
				ret = ERROR_INVALID_PARAMETER; 

				data_sets_name = data_sets.GetItemText( i, 0 ); 

				message.Format( L"请对%s配置至少一种输出方式:文本输出或MYSQL数据库", data_sets_name.GetBuffer() ); 
				MessageBox( message.GetBuffer() ); 
				break; 
			}

			container_info = ( CONTAINER_INFO* )data_sets.GetItemData( i ); 
			if( NULL == container_info )
			{
				break; 
			}

			{
				CString conn_string; 

				_ret = get_sql_connection_info_from_string( &container_info->store_param.sql_connection, sql_conn_string.GetBuffer() ); 
				if( ERROR_SUCCESS != _ret )
				{

				}
			}

			container_info->store_param.output_link_list = output_link_list; 
			container_info->store_param.to_data_center = to_data_center; 
			container_info->store_param.type = DATA_OUTPUT_CSV_FILE; 
			container_info->store_param.output_path = csv_file_name.GetBuffer(); 
			container_info->store_param.sql_table_name = table_name.GetBuffer(); 

			do 
			{
				CString message; 

				_ret = create_data_store( &container_info->store_param, 
					&container_info->store, 
					error_string ); 
				
				if( _ret != ERROR_SUCCESS )
				{
					message.Format( L"第%u个数据集", i ); 

					message += L"创建输出对象失败:"; 
					message += error_string.c_str(); 

					dbg_print_w( MSG_ERROR, L"%s", message.GetBuffer() ); 
				}

				if( ( FALSE == container_info->store.have_db_conn 
					|| container_info->store.table_name.empty() == 0 )
					&& container_info->store.data_file == INVALID_HANDLE_VALUE )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 

					MessageBox( message.GetBuffer() ); 
					break; 
				}
			}while( FALSE ); 

			if( ERROR_SUCCESS !=  ret )
			{
				break; 
			}
		}

		if( ERROR_SUCCESS !=  ret )
		{
			break; 
		}
	}while( FALSE );

	if( ret == ERROR_SUCCESS )
	{
		CDialog::OnOK(); 
	}

	return; 
}
// data_output_config_dlg message handlers

void data_output_config_dlg::OnBnClickedButtonSelectPath()
{
	LRESULT ret; 
	WCHAR file_name[ MAX_PATH ]; 
	WCHAR app_path[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		app_path[ cc_ret_len - 1 ] = L'\0'; 

		*file_name = _T( '\0' ); 

		//_ret = SetCurrentDirectoryW( app_path ); 
		//if( FALSE == _ret )
		//{

		//}

		ret = open_file_dlg( TRUE, NULL, file_name, L"*.*\0*.*\0\0", app_path );
		if( ret < 0 )
		{
			break; 
		}

		if( *file_name != L'\0' )
		{
			LPWSTR file_component; 

			file_component = wcsrchr( file_name, L'\\' ); 
			//if( file_component != NULL )
			//{
			//	SetDlgItemTextW( IDC_EDIT_OUTPUT_TABLE_NAME, file_component ); 
			//}
		}

	}while( FALSE ); 
}

void data_output_config_dlg::OnBnClickedButtonDonate()
{
	// TODO: Add your control notification handler code here
#define DONATE_URL L"http://www.simplestai.com/webkits/index.php" 
	::ShellExecute(NULL, NULL, DONATE_URL, NULL, NULL, SW_SHOWNORMAL);
}

LRESULT data_output_config_dlg::add_page_info_to_list( CONTAINER_INFO *page )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LONG item_count; 
	LONG item_index; 

	do 
	{
		ASSERT( NULL != page ); 
		item_count = data_sets.GetItemCount(); 

		item_index = data_sets.InsertItem( item_count, page->name.c_str() ); 
		if( item_index < 0 )
		{
			break; 
		}

		if( page->store_param.output_path.length() > 0 )
		{
			CString _default_path; 
			LPWSTR file_component; 
			INT32 file_component_end; 

			_default_path = page->store_param.output_path.c_str(); 
			_ret = data_sets.SetItemText( item_index, 1, _default_path.GetBuffer() ); 
			if( _ret == FALSE )
			{
				//break; 
			}

			file_component_end = _default_path.Find(L".csv", 0 ); 

			if( file_component_end >= 0 )
			{
				_default_path = _default_path.Left( file_component_end ); 
			}

			file_component = wcsrchr( _default_path.GetBuffer(), L'\\' ); 
			if( file_component != NULL )
			{
                ASSERT(*(file_component + 1) != L'\0'); 
				_ret = data_sets.SetItemText( item_index, 2, file_component + 1 ); 
				if( _ret == FALSE )
				{
					//break; 
				}
			}
		}

		data_sets.SetItemData( item_index, ( DWORD_PTR )( PVOID )page ); 

	}while( FALSE );

	return ret; 
}

LRESULT data_output_config_dlg::set_html_pages_layout( CONTAINER_INFOS *containers )
{
	LRESULT ret = ERROR_SUCCESS; 

	container_infos = containers; 
	return ret; 
}

/*********************************************
对同一颗指令树，使用多个工作程序/纯种来同时执行，
可以使用复制多颗指令树的方式。
*********************************************/

LRESULT data_output_config_dlg::load_data_set_info( CONTAINER_INFO *container_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 item_index; 
	INT32 item_count; 
	HTML_ELEMENT_ACTIONS::iterator it; 

	do 
	{
		data_set.DeleteAllItems(); 

		for( it = container_info->content.begin(); 
			it != container_info->content.end(); 
			it ++ )
		{
			item_count = data_set.GetItemCount(); 

			item_index = data_set.InsertItem( item_count, ( *it )->name.c_str() ); 

			if( item_index < 0 )
			{
				break; 
			}

			data_set.SetItemText( item_index, 1, ( *it )->xpath.c_str() ); 
			data_set.SetItemData( item_index, ( DWORD_PTR )( PVOID )( *it ) ); 
		}
	}while( FALSE );
	return ret; 
}

void data_output_config_dlg::OnNMClickListHtmlDataSets(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT ret; ; 
	INT32 hit_item; 
	CONTAINER_INFO *container_info; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	do 
	{
		*pResult = 0;

		//_ret = GetCursorPos( &pos ); 
		//if( FALSE == _ret )
		//{
		//	break; 
		//}

		//ScreenToClient( &pos ); 

		//hit_item = data_sets.HitTest( pos, NULL ); 
		//if( hit_item < 0 )
		//{
		//	break; 
		//}

		if( 0 > pNMItemActivate->iItem )
		{
			break; 
		}

		hit_item = pNMItemActivate->iItem; 
		container_info = ( CONTAINER_INFO* )data_sets.GetItemData( hit_item ); 
		if( NULL == container_info )
		{
			break; 
		}

		ret = load_data_set_info( container_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	} while ( FALSE );
}

#include "sql_conn_setting_dlg.h"

void data_output_config_dlg::OnBnClickedButtonSqlConnSetting()
{
	LRESULT ret; 
	sql_conn_setting_dlg dlg; 
	SQL_CONNECTION_PARAMW *conn_param; 

	do 
	{
		SQL_CONNECTION_PARAM _conn_param; 
		GetDlgItemText( IDC_EDIT_SQL_CONN_STRING, sql_conn_string ); 
		
		ret = get_sql_connection_info_from_string( &_conn_param, 
			sql_conn_string.GetBuffer() ); 

		dlg.set_sql_conn_param( &_conn_param ); 

		if( IDOK != dlg.DoModal() )
		{
			break; 
		}

		dlg.get_sql_conn_param( &conn_param ); 

		{
			CString conn_string; 
			conn_string.Format( L"SERVER=%s;PORT=%u;DATABASE=%s;USER=%s;PASSWORD=%s;", 
				conn_param->ip.c_str(), 
				conn_param->port, 
				conn_param->data_base_name.c_str(), 
				conn_param->user_name.c_str(), 
				conn_param->password.c_str() ); 

			SetDlgItemText( IDC_EDIT_SQL_CONN_STRING, conn_string.GetBuffer() ); 
		}
	}while( FALSE );
}

void data_output_config_dlg::OnBnClickedButtonSqlConnCheck()
{
	LRESULT ret = ERROR_SUCCESS; 
	CString text; 
	SQL_CONNECTION_PARAM param; 
	string error_string; 
	
	GetDlgItemText(IDC_EDIT_SQL_CONN_STRING, text ); 
	
	do 
	{
		ret = get_sql_connection_info_from_string( &param, 
			text.GetBuffer() ); 
		
		ret = check_mysql_db_conn( param.ip.c_str(), 
			param.port, 
			param.user_name.c_str(), 
			param.password.c_str(), 
			param.data_base_name.c_str(), 
			error_string ); 

		if( ERROR_SUCCESS != ret )
		{
			wstring message; 
			wstring text; 

			message = L"连接数据库失败:"; 
			utf8_to_unicode_ex( error_string.c_str(), text ); 

			message += text; 
			MessageBox( message.c_str() ); ; 
			break; 
		}

		MessageBox( L"连接数据库成功" ); 
	} while ( FALSE );
}

void data_output_config_dlg::OnNMDblclkListHtmlDataSets(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此加入控制項告知處理常式程式碼
	*pResult = 0;

}
