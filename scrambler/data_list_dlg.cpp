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

// data_list_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "data_list_dlg.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#include <comutil.h>

#define MAX_HTML_SHOW_TEXT_LEN 120
#define DATA_SCRAMBLED_COLUMN_LEN 200 
#define MAX_DATA_TEXT_LEN MAX_PATH

IMPLEMENT_DYNAMIC(data_list_dlg, CDialog)

data_list_dlg::data_list_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(data_list_dlg::IDD, pParent)
{
	columes = NULL; 
	colume_count = 0; 
}

data_list_dlg::~data_list_dlg()
{
}

void data_list_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DATAS, data_list);
}


BEGIN_MESSAGE_MAP(data_list_dlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DATAS, &data_list_dlg::OnNMDblclkListContentElements)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_DATAS, &data_list_dlg::OnNMClickListContentElements)
	ON_WM_SIZING()
END_MESSAGE_MAP()


// data_list_dlg message handlers
LRESULT data_list_dlg::insert_data_info( LPCWSTR texts[], 
										ULONG text_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	//WCHAR _text[ MAX_DATA_TEXT_LEN ]; 
	//LPWSTR remain_text; 
	//size_t remain_len; 
	ULONG item_count; 
	INT32 i; 

	do 
	{
		//remain_text = text; 
		//remain_len = ARRAYSIZE( text ); 

		item_count = data_list.GetItemCount(); 
		data_list.InsertItem( item_count, texts[ 0 ] ); 

		for( i = 1; ( ULONG )i < text_count; i ++ )
		{
			data_list.SetItemText( item_count, i, texts[ i ] ); 
		}

		//if(hr != S_OK)
		//{
		//	dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
		//}
		//else
		//{
		//	LPCSTR contant; 
		//	contant = data_contants.operator char*(); 

		//	dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
		//}

	}while( FALSE );

	//table_element.Release();
	return ret;
}

BOOL data_list_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	data_list.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT| /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );

	return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT data_list_dlg::set_page_contents( HTML_ELEMENT_ACTIONS *page_contents ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != page_contents ); 
		//if( page_contents->size() == 0 )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		//for( HTML_PAGE_CONTENTS::iterator it = page_contents->begin(); 
		//	it != page_contents->end(); 
		//	it ++ )
		//{
		//}
		ret = init_scramble_data_info_headers( page_contents ); 
	}while( FALSE );

	return ret; 
}

LRESULT data_list_dlg::init_scramble_data_info_headers( HTML_ELEMENT_ACTIONS *contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	//HRESULT hr; 
	//WCHAR text[ DATA_COLUME_TEXT_LEN ]; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	CHeaderCtrl *list_header; 
	_bstr_t data_contants; 
	BOOLEAN colume_changed = FALSE; 
	LVCOLUMN column_info; 
#define MAX_COLUMN_TEXT_LEN MAX_PATH
	WCHAR colume_text[ MAX_COLUMN_TEXT_LEN ]; 
	INT32 i; 

	do 
	{
		if( contents == NULL )
		{
			break; 
		}

		do 
		{
			list_header = data_list.GetHeaderCtrl(); 
			if( list_header == NULL )
			{
				colume_changed = TRUE; 
				break; 
			}

			if( list_header->GetItemCount() != contents->size() )
			{
				colume_changed = TRUE; 
				break; 
			}

			colume_text[ ARRAYSIZE( colume_text ) - 1 ] = L'\0'; 

			for( i = 0; i < list_header->GetItemCount(); i ++ )
			{
				it = contents->begin() + i; 

				column_info.mask = LVCF_TEXT; 
				column_info.pszText = colume_text; 
				column_info.cchTextMax = ARRAYSIZE( colume_text ); 

				_ret = data_list.GetColumn(i, 
					&column_info ); 
				if( FALSE == _ret )
				{
					colume_changed = TRUE; 
					break; 
				}

				if( colume_text[ ARRAYSIZE( colume_text ) - 1 ] != L'\0' )
				{
					ASSERT( FALSE ); 
					colume_text[ ARRAYSIZE( colume_text ) - 1 ] = L'\0'; 
				}

				if( 0 != wcscmp( column_info.pszText, 
					( *it )->name.c_str() ) )
				{
					colume_changed = TRUE; 
					break; 
				}
			}
		}while( FALSE ); 

		if( colume_changed == FALSE )
		{
			break; 
		}

		data_list.DeleteAllItems(); 

		for( ; ; )
		{
			_ret = data_list.DeleteColumn( 0 ); 
			if( FALSE == _ret )
			{
				break; 
			}

			list_header = data_list.GetHeaderCtrl(); 
			if( list_header != NULL )
			{
				list_header->DeleteItem( 0 ); 
			}
		}

		for( it = contents->begin(); it != contents->end(); it ++ )
		{
			data_list.InsertColumn(it - contents->begin(), 
				( *it )->name.c_str(),
				LVCFMT_LEFT,
				DATA_SCRAMBLED_COLUMN_LEN ); 
		}
	}while( FALSE ); 
	
	return ret;
}

const INT_PTR image_index = 2; 
LRESULT data_list_dlg::add_scrambled_data_info( HTML_ELEMENT_ACTIONS *contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	//WCHAR _text[ MAX_DATA_TEXT_LEN ]; 
	//LPWSTR remain_text; 
	//size_t remain_len; 
	IHTMLElementPtr last_row_element;
	HTML_ELEMENT_ACTIONS::iterator it; 
	CString data_output_status; 

	//INT32 i; 

	do 
	{
        wstring output_data;
		wstring output_text; 
		ULONG item_count; 
		INT32 item_index; 

		item_count = data_list.GetItemCount(); 

#define MAX_DATA_LIST_ITEM_COUNT 2500

		if( item_count > MAX_DATA_LIST_ITEM_COUNT )
		{
			data_list.DeleteItem( 0 ); 
			item_count = data_list.GetItemCount(); 
		}

		item_index = -1; 
		for( it = contents->begin(); it != contents->end(); it ++ )
		{
            cat_output_data((*it), output_data);
  
			if( it != contents->begin() 
				&& output_data.length() > MAX_HTML_SHOW_TEXT_LEN )
			{
				output_text = output_data.substr( 0, MAX_HTML_SHOW_TEXT_LEN ); 
				output_text += L"..."; 
			}
			else
			{
				output_text = output_data.c_str();
			}

			if( it == contents->begin() )
			{
				item_index = data_list.InsertItem( item_count, output_text.c_str() ); 
				if( item_index < 0 )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}
			else
			{
				ASSERT( item_index >= 0 ); 
				data_list.SetItemText( item_index, it - contents->begin(), output_text.c_str() ); 
			}

			do 
			{
				ASSERT( item_index >= 0 ); 
				data_list.SetItemData( item_index, NULL ); 
				//if( ( *it )->additions.size() > image_index )
				//{
				//	wstring image_file_name; 

				//	image_file_name = ( *( ( *it )->additions.begin() + image_index ) ); 

				//	if( image_file_name.length() == 0 )
				//	{
				//		break; 
				//	}

				//	{
				//		WCHAR *_image_file_name; 

				//		_image_file_name = ( WCHAR* )malloc( ( ( image_file_name.length() + 1 ) << 1 ) ); 
				//		if( NULL == _image_file_name)
				//		{
				//			break; 
				//		}

				//		memcpy( _image_file_name, image_file_name.c_str(), ( image_file_name.length() + 1 ) << 1 ); 
				//		data_list.SetItemData( item_index, ( ULONG_PTR )( PVOID )_image_file_name ); 
				//	}
				//}
			} while (FALSE ); 

			data_output_status.Format( L"¹²%dÌõ¼ÇÂ¼", item_count + 1 ); 
			SetWindowText( data_output_status.GetBuffer() ); 
		}

		//if(hr != S_OK)
		//{
		//	dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
		//}
		//else
		//{
		//	//LPCSTR contant; 
		//	//contant = data_contants.operator char*(); 

		//	//dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
		//}

	}while( FALSE );

	//table_element.Release();
	return ret;
}

void data_list_dlg::OnOK()
{

}

void data_list_dlg::OnCancel()
{

}

LRESULT data_list_dlg::add_data_info( ULONG count, ... )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR *texts = NULL; 
	LPWSTR text; 
	va_list va; 
	ULONG i; 

	do 
	{
		//datas.push_back( wstring( text ) ); 
		texts = ( LPWSTR* )malloc( sizeof( LPWSTR ) * count ); 
		if( NULL == texts )
		{
			break; 
		}

		va_start( va, count ); 

		for( i = 0; i < count; i ++ )
		{
			text = ( LPWSTR )va_arg( va, LPWSTR ); 
			texts[ i ] = text; 
		}

		va_end( va ); 

		ret = insert_data_info( ( LPCWSTR* )texts, count ); 
	}while( FALSE ); 

	if( NULL != texts )
	{
		free( texts ); 
	}

	return ret; 
}

void data_list_dlg::OnNMClickListContentElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	INT32 item_index; 

	*pResult = 0;


	do 
	{
		item_index = pNMItemActivate->iItem; 

		if( item_index < 0 )
		{
			break; 
		}

		if( item_index >= data_list.GetItemCount() )
		{
			ASSERT( FALSE ); 
			break; 
		}

		{
			HRESULT hr; 
			LRESULT ret; 
			WCHAR file_name[ MAX_PATH ]; 
			WCHAR *cmd_line = NULL; 
			PVOID item_data; 
			ULONG cc_ret_len; 

			item_data = ( PVOID )data_list.GetItemData( item_index ); 

			do 
			{
				#define MAX_CMD_LINE 1024
				cmd_line = ( WCHAR* )malloc( MAX_CMD_LINE << 1 ); 
				if( NULL == cmd_line )
				{
					break; 
				}

				//SHELLEXECUTEINFO sei = { 0 };
				//sei.fMask = SEE_MASK_NOCLOSEPROCESS;
				//sei.lpVerb = _T("open");
				//sei.lpFile = _T("mspaint.exe");
				//sei.hwnd = NULL;
				//sei.nShow = SW_SHOWNORMAL;
				//sei.cbSize = sizeof(SHELLEXECUTEINFO);

				//ShellExecuteEx(&sei);
				//WaitForInputIdle(sei.hProcess, 500);

				if( item_data == NULL )
				{
					break; 
				}

				ret = get_image_file_name_url( ( LPCWSTR )item_data, file_name, ARRAYSIZE( file_name ), &cc_ret_len ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				hr = StringCchCopyW( cmd_line, MAX_CMD_LINE, L"mspaint.exe " ); 
				if( FAILED(hr))
				{
					break; 
				}

				hr = StringCchCatW( cmd_line, MAX_CMD_LINE, file_name ); 
				if( FAILED(hr))
				{
					break; 
				}

				run_proc( cmd_line, NULL); 

			} while ( FALSE );

			if( NULL != cmd_line )
			{
				free( cmd_line ); 
			}
		}
	}while( FALSE ); 

	return; 
}

void data_list_dlg::OnNMDblclkListContentElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	LONG item_index; 
	CString item_text; 

	*pResult = 0;

	do 
	{
		item_index = pNMItemActivate->iItem; 

		if( item_index < 0 )
		{
			break; 
		}

		if( item_index >= data_list.GetItemCount() )
		{
			ASSERT( FALSE ); 
			break; 
		}

		item_text = data_list.GetItemText( item_index, 0 ); 

		if( *item_text == '\0' )
		{
			break; 
		}

		try
		{
			::ShellExecute(NULL, NULL, item_text, NULL, NULL, SW_SHOWNORMAL); 
		}
		catch (CMemoryException* e)
		{
			dbg_print( MSG_FATAL_ERROR, "error %s %n\n", __FUNCTION__, __LINE__ ); 
		}
		catch (CFileException* e)
		{
			dbg_print( MSG_FATAL_ERROR, "error %s %n\n", __FUNCTION__, __LINE__ ); 
		}
		catch (CException* e)
		{
			dbg_print( MSG_FATAL_ERROR, "error %s %n\n", __FUNCTION__, __LINE__ ); 
		}
	}while( FALSE ); 
}
void data_list_dlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CRect client_rect; 

	GetClientRect(&client_rect); 

	data_list.MoveWindow( &client_rect, TRUE ); 
	CDialog::OnSizing(fwSide, pRect);
}
