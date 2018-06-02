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


// plugin_manage_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "plugin_manage_dlg.h"
#include "plugin.h"

// plugin_manage_dlg 對話方塊

IMPLEMENT_DYNAMIC(plugin_manage_dlg, CDialog)

plugin_manage_dlg::plugin_manage_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(plugin_manage_dlg::IDD, pParent)
{

}

plugin_manage_dlg::~plugin_manage_dlg()
{
}

void plugin_manage_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PLUGINS, module_infos_list);
}

BOOL plugin_manage_dlg::OnInitDialog()
{
	LRESULT ret; 
	DATA_PROCESS_PLUGINS *infos; 
	INT32 item_count; 

	CDialog::OnInitDialog();
	
	module_infos_list.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );
	module_infos_list.InsertColumn( 0, _T("路径"),LVCFMT_LEFT,300); 

	do 
	{
		ret = get_plugins( &infos ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret ); 
			break; 
		}

		for( DATA_PROCESS_PLUGINS::iterator it = infos->begin(); 
			it != infos->end(); 
			it ++ )
		{
			item_count = module_infos_list.GetItemCount(); 

			module_infos_list.InsertItem( item_count, ( *it )->path.c_str() ); 
		}

		//for( DATA_PROCESS_PLUGINS::iterator it = modules_info.begin(); 
		//	it != modules_info.end(); 
		//	it ++ )
		//{
		//	item_count = module_infos_list.GetItemCount(); 

		//	module_infos_list.InsertItem( item_count, ( *it )->path.c_str() ); 
		//}
	} while ( FALSE );

	return FALSE; 
}

BEGIN_MESSAGE_MAP(plugin_manage_dlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DEL_PLUGIN, &plugin_manage_dlg::OnBnClickedButtonDelPlugin)
	ON_BN_CLICKED(IDC_BUTTON_ADD_PLUGIN, &plugin_manage_dlg::OnBnClickedButtonAddPlugin)
	ON_BN_CLICKED(IDC_BUTTON_BROWSER, &plugin_manage_dlg::OnBnClickedButtonBrowser)
END_MESSAGE_MAP()


void plugin_manage_dlg::OnOK()
{
	CDialog::OnOK(); 
}

void plugin_manage_dlg::OnCancel()
{
    CDialog::OnCancel();
}

// plugin_manage_dlg 訊息處理常式

void plugin_manage_dlg::OnBnClickedButtonDelPlugin()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	LRESULT ret; 
	POSITION pSelectedPos = NULL;
	UINT32 nSelectedCount; 
	INT32 nSelectedPos = -1; 
	CString plugin_path; 
	DATA_PROCESS_PLUGIN_INFO info; 

	do 
	{
		nSelectedCount = module_infos_list.GetSelectedCount();
		if ( nSelectedCount < 1 )
		{
			break; 
		}

		pSelectedPos = module_infos_list.GetFirstSelectedItemPosition();
		if ( pSelectedPos == NULL )
		{
			break;
		}

		nSelectedPos = module_infos_list.GetNextSelectedItem( pSelectedPos );
		if ( nSelectedPos < 0 )
		{
			break; 
		}

		plugin_path = module_infos_list.GetItemText( nSelectedPos, 0 ); 

		info.path = plugin_path; 

		ret = del_plugin_info( &info ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
			break; 
		}

		module_infos_list.DeleteItem( nSelectedPos ); 

	}while( FALSE );

	return; 
}

void plugin_manage_dlg::OnBnClickedButtonAddPlugin()
{
	LRESULT ret; 
	CString path; 
	DATA_PROCESS_PLUGIN_INFO info; 
	INT32 item_count; 
	
	do 
	{
		GetDlgItemText(IDC_EDIT_PLUGIN_PATH, path ); 
		if( path.GetLength() == 0)
		{
			break; 
		}

		info.path = path.GetBuffer(); 

		ret = add_plugin_info( &info ); 
		if( ERROR_SUCCESS != ret )
		{
			dbg_print( MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret ); 
			break; 
		}

		item_count = module_infos_list.GetItemCount(); 

		module_infos_list.InsertItem( item_count, path.GetBuffer() ); 

		//ret = get_plugins( &infos ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	dbg_print( MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret ); 
		//	break; 
		//}

		//for( DATA_PROCESS_PLUGINS::iterator it = infos->begin(); 
		//	it != infos->end(); 
		//	it ++ )
		//{
		//	item_count = module_infos_list.GetItemCount(); 

		//	module_infos_list.InsertItem( item_count, ( *it )->path.c_str() ); 
		//}

	} while ( FALSE );

}

void plugin_manage_dlg::OnBnClickedButtonBrowser()
{
	LRESULT ret; 
	// TODO: 在此加入控制項告知處理常式程式碼
	WCHAR file_name[ MAX_PATH ]; 
	WCHAR app_path[ MAX_PATH ]; 
	ULONG cc_ret_len; 
	
	do 
	{
		*file_name = NULL; 
		ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
			break; 
		}

		ret = open_file_dlg( TRUE, L"dll", file_name, NULL, app_path, L"*.dll\0*.dll\0*.*\0*.*\0\0", NULL );
		if( ret < 0 )
		{
			dbg_print( MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
			break; 
		}

		if( *file_name == L'\0' )
		{
			break; 
		}

		SetDlgItemText( IDC_EDIT_PLUGIN_PATH, file_name ); 
	} while ( FALSE );
}
