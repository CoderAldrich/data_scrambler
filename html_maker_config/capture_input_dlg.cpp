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

// apture_input_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "html_maker_config.h"
#include "capture_input_dlg.h"
#include "html_xpath.h"
#include "input_content.h"
// apture_input_dlg.cpp : implementation file
//

#define IDC_MENU_ITEM_DELETE_TEXT_BLOCK 0x1011
// capture_input_dlg dialog

XPATH_PARAMS input_contents; 


// capture_input_dlg dialog

IMPLEMENT_DYNAMIC(capture_input_dlg, CDialog)

capture_input_dlg::capture_input_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(capture_input_dlg::IDD, pParent)
{

}

capture_input_dlg::~capture_input_dlg()
{
}

void capture_input_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TEXT_BLOCKS, text_block_list);
}

BOOL capture_input_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	init_html_config_ui(); 

	SetDlgItemTextW( IDC_EDIT_TEXT, text_block.c_str() ); 
	SetDlgItemTextW( IDC_EDIT_TEXT_NAME, text_name.c_str() ); 

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(capture_input_dlg, CDialog)
	ON_BN_CLICKED(IDOK, &capture_input_dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BN_ADD_TEXT, &capture_input_dlg::OnBnClickedBnAddText)
	ON_COMMAND( IDC_MENU_ITEM_DELETE_TEXT_BLOCK, &capture_input_dlg::on_delete_text_block )
	ON_NOTIFY(NM_RCLICK, IDC_LIST_TEXT_BLOCKS, &capture_input_dlg::OnNMRClickListTextBlocks)
	ON_BN_CLICKED(IDC_BN_DELETE_TEXT, &capture_input_dlg::OnBnClickedBnDeleteText)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TEXT_BLOCKS, &capture_input_dlg::OnNMDblclkListTextBlocks)
END_MESSAGE_MAP()

void capture_input_dlg::on_delete_text_block()
{
	wstring text_name; 
	wstring text_content; 
	
	delete_text_block( text_name, text_content ); 
}

LRESULT capture_input_dlg::get_selected_text_block()
{
	LRESULT ret = ERROR_SUCCESS; 
	//CString text; 
	LPCWSTR _text_name; 
	//wstring conf_file_name; 
	//XPATH_PARAM_ITERATOR it; 

	//INT32 item_count; 
	INT32 item_index = -1; 
	POSITION pos; 

	do 
	{
		pos = text_block_list.GetFirstSelectedItemPosition();
		if (pos == NULL)
		{
			TRACE(_T("No items were selected!\n"));
		}
		else
		{
			//for( ; ; )
			{
				do 
				{
					item_index = text_block_list.GetNextSelectedItem( pos ); 

					_text_name = ( LPCWSTR )text_block_list.GetItemData( item_index ); 
					if( NULL != _text_name )
					{
						XPATH_PARAM_ITERATOR xpath_it; 

						text_name = _text_name; 

						xpath_it = input_contents.find( text_name.c_str() ); 

						if( xpath_it == input_contents.end() )
						{
							text_block = L""; 
						}
						else
						{
							text_block = xpath_it->second.c_str(); 
						}
					}
					else
					{
						text_name = L""; 
					}

					break; 
				}while( FALSE ); 
			}
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT capture_input_dlg::delete_text_block( wstring &text_name_out, wstring &text_content_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	POSITION pSelectedPos = NULL;
	UINT32 nSelectedCount; 
	INT32 nSelectedPos = -1; 
	LPCWSTR text_block; 


	do 
	{
		text_name_out = L""; 
		text_content_out = L""; 

		nSelectedCount = text_block_list.GetSelectedCount();
		if ( nSelectedCount < 1 )
		{
			break; 
		}

		pSelectedPos = text_block_list.GetFirstSelectedItemPosition();
		if ( pSelectedPos == NULL )
		{
			break;
		}

		nSelectedPos = text_block_list.GetNextSelectedItem( pSelectedPos );
		if ( nSelectedPos < 0 )
		{
			break; 
		}

		text_block = ( LPCWSTR )text_block_list.GetItemData( nSelectedPos ); 
		ASSERT( NULL != text_block ); 

		{
			XPATH_PARAM_ITERATOR text_block_it; 

			for( text_block_it = input_contents.begin(); text_block_it != input_contents.end(); text_block_it ++ )
			{
				if( 0 == wcscmp( text_block, ( *text_block_it ).first.c_str() ) )
				{
					{
						wstring conf_file_name;  

						ret = get_input_context_file_path( conf_file_name ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						ret = del_input_text_block_info( conf_file_name.c_str(), ( *text_block_it ).second.c_str(), ( *text_block_it ).first.c_str() ); 
						if( ret != ERROR_SUCCESS )
						{
							ASSERT( FALSE ); 
						}
						
						text_name_out =  ( *text_block_it ).first.c_str(); 
						text_content_out =  ( *text_block_it ).second.c_str(); 
					}

					input_contents.erase( text_block_it ); 
					break; 
				}
			}

			//ASSERT( text_block_it != input_contents.end() ); 
		}

		text_block_list.DeleteItem( nSelectedPos ); 

	}while( FALSE );

	return ret; 
}


// capture_input_dlg message handlers

void capture_input_dlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	get_selected_text_block(); 

	OnOK();
}

LRESULT capture_input_dlg::load_input_contents()
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 _ret; 

	CString text; 
	CString text_name; 
	INT32 list_item_index; 
	wstring conf_file_name; 
	XPATH_PARAM_ITERATOR it; 

	do 
	{
		input_contents.clear(); 

		ret = get_input_context_file_path( conf_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = load_input_content( conf_file_name.c_str(), input_contents ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		text_block_list.DeleteAllItems(); 

		for( it = input_contents.begin(); it != input_contents.end(); it ++ )
		{
			list_item_index = text_block_list.InsertItem( 0, ( *it ).first.c_str(), -1 ); 

			text_block_list.SetItemText(list_item_index,0,( *it ).first.c_str() ); 

			text_block_list.SetItemText(list_item_index,1,( *it ).second.c_str() ); 

			text_block_list.SetItemData( list_item_index, ( DWORD_PTR )( *it ).first.c_str() ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT capture_input_dlg::init_html_config_ui()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG colume_index = 0; 

	do
	{
		_ret = text_block_list.InsertColumn( colume_index, _T("文本名"), LVCFMT_LEFT, 120);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}
		else
		{
			colume_index ++; 
		}

		_ret = text_block_list.InsertColumn( colume_index, _T("文本内容"), LVCFMT_LEFT, 120);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}
		else
		{
			colume_index ++; 
		}

		text_block_list.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES
			|LVS_SHOWSELALWAYS); 

		ret = load_input_contents(); 
	}while( FALSE ); 

	return ret; 
}

void capture_input_dlg::OnBnClickedBnAddText()
{
	// TODO: Add your control notification handler code here
	INT32 ret; 
	CString text; 
	CString text_name; 
	wstring conf_file_name; 

	do 
	{
		ret = get_input_context_file_path( conf_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = make_content_file_exist( conf_file_name.c_str(), conf_file_name.length() ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = GetDlgItemText( IDC_EDIT_TEXT, text ); 
		if( ret <= 0 )
		{
			break; 
		}

		ret = GetDlgItemText( IDC_EDIT_TEXT_NAME, text_name ); 
		if( ret <= 0 )
		{
			break; 
		}

		ret = add_input_text_block_info( conf_file_name.c_str(), text.GetBuffer(), text_name.GetBuffer() ); 

		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = load_input_contents(); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

	} while( FALSE );
	
	return; 
}

void capture_input_dlg::OnNMRClickListTextBlocks(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LRESULT ret; 
	INT32 _ret; 
	// TODO: Add your control notification handler code here
	CMenu menu; 
	NMLISTVIEW* pMListView; 
	CPoint pt; 
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	do
	{
		*pResult = 0;

		_ret = menu.CreatePopupMenu();
		if( FALSE == _ret )
		{
			break; 
		}

		_ret = menu.AppendMenu( MF_STRING, IDC_MENU_ITEM_DELETE_TEXT_BLOCK, L"删除" );
		if( FALSE == _ret )
		{
			break; 
		}

		pMListView=(NMLISTVIEW*) pNMHDR;
		int nIndex = pMListView->iItem;
		if ( nIndex < 0 )
		{
			menu.EnableMenuItem( 0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED ); 
		}

		GetCursorPos( &pt );
		_ret = menu.TrackPopupMenu( TPM_LEFTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, this ); 
		if( FALSE == _ret )
		{
			break; 
		}

	}while( FALSE ); 
}

void capture_input_dlg::OnBnClickedBnDeleteText()
{
	// TODO: Add your control notification handler code here
	INT32 ret; 
	wstring text; 
	wstring text_name; 
	wstring conf_file_name; 

	do 
	{
		ret = get_input_context_file_path( conf_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = make_content_file_exist( conf_file_name.c_str(), conf_file_name.length() ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = delete_text_block( text_name, text ); 
		if( ret != ERROR_SUCCESS )
		{
			//break; 
		}

		if( text.length() > 0 
			&& text_name.length() > 0 )
		{
			SetDlgItemText( IDC_EDIT_TEXT, text.c_str() ); 

			SetDlgItemText( IDC_EDIT_TEXT_NAME, text_name.c_str() ); 
		}
	} while( FALSE );

	return; 
}

void capture_input_dlg::OnNMDblclkListTextBlocks(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	OnBnClickedOk(); 
}
