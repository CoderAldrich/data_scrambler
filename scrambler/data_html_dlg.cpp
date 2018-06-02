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

// FileOperatorHtmlView.cpp : 实现文件
//

#include "stdafx.h"
#include "resource.h"
#include "data_html_dlg.h"
#include "data_scrambler.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#include <comutil.h>

IMPLEMENT_DYNCREATE(data_html_dlg, dynamic_html_dlg)
#define MAX_HTML_SHOW_TEXT_LEN 13

/***************************************s
CComPtr<IHTMLElement> spElement;
spDocument->get_body(&spElement);
if (NULL == spElement)
return;

CComPtr<IHTMLBodyElement> spBodyElement;
spElement->QueryInterface(__uuidof(IHTMLBodyElement), (void**)&spBodyElement);
if (NULL == spBodyElement)
return;

spBodyElement->put_scroll(_bstr_t(_T("auto")));
*******************************************/

data_html_dlg::data_html_dlg( CWnd *pParent ) 
				: dynamic_html_dlg(data_html_dlg::IDD, data_html_dlg::IDH, pParent)
{
	EnableAutomation(); 
	columes = NULL; 
	colume_count = 0; 
}

data_html_dlg::~data_html_dlg()
{
}

void data_html_dlg::DoDataExchange(CDataExchange* pDX)
{
	dynamic_html_dlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(data_html_dlg, dynamic_html_dlg)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(data_html_dlg, dynamic_html_dlg)
    //DISP_FUNCTION(data_html_dlg,"ShowTableDescription", , VT_UI4,VTS_UI4)        //每个方法都需要在这里添加映射

END_DISPATCH_MAP()
// CFileOperatorHtmlView 诊断

#ifdef _DEBUG
void data_html_dlg::AssertValid() const
{
	dynamic_html_dlg::AssertValid();
}

void data_html_dlg::Dump(CDumpContext& dc) const
{
	dynamic_html_dlg::Dump(dc);
}
#endif //_DEBUG

void data_html_dlg::OnCancel()
{
	ShowWindow( SW_HIDE ); 
}

// CFileOperatorHtmlView 消息处理程序
BOOL data_html_dlg::OnInitDialog()
{
    //屏蔽IE的文件拖放功能
	SetHostFlags(DOCHOSTUIFLAG_FLAT_SCROLLBAR);
	//SetRegisterAsDropTarget(FALSE);

	dynamic_html_dlg::OnInitDialog(); 

    TCHAR tszExePath[MAX_PATH] ={0};
    GetModuleFileName(NULL,tszExePath,_countof(tszExePath));
    TCHAR tszUrl[256]     = {0};

	StringCbPrintf(tszUrl,sizeof(tszUrl),_T("res://%s/tab/versionctrl/tab.htm"),tszExePath);
    Navigate(tszUrl);

	//init_data_info_headers( ( LPCWSTR* )columes, colume_count ); 

	return TRUE; 
}

LRESULT data_html_dlg::insert_data_info( LPCWSTR texts[], 
										ULONG text_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
#define MAX_DATA_TEXT_LEN MAX_PATH
	WCHAR _text[ MAX_DATA_TEXT_LEN ]; 
	//LPWSTR remain_text; 
	//size_t remain_len; 
	_bstr_t data_contants; 
	IHTMLElementPtr last_row_element;
	INT32 i; 

	data_contants = "<tr>"; 

	do 
	{
		//remain_text = text; 
		//remain_len = ARRAYSIZE( text ); 

		for( i = 0; ( ULONG )i < text_count; i ++ )
		{
			hr = StringCchPrintfW( _text, 
				ARRAYSIZE( _text ), 
				L"<td>%s</td>", 
				texts[ i ] ); 

			if( FAILED(hr))
			{
				break; 
			}

			data_contants += _text; 
		}

		data_contants += "</tr>"; 

		//get_element_from_id(_T("datas"),&table_element );
		get_element_from_id(_T("last_data"),&last_row_element );

		//IHTMLElement2Ptr last_row_element2; 
		
		_bstr_t flags; 
		flags = L"beforeBegin"; 

		hr = last_row_element->insertAdjacentHTML( flags.GetBSTR(), 
			data_contants.GetBSTR() ); 

		if(hr != S_OK)
		{
			dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
		}
		else
		{
			LPCSTR contant; 
			contant = data_contants.operator char*(); 

			dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
		}

	}while( FALSE );

	//table_element.Release();
	return ret;
}

//LRESULT data_html_dlg::init_data_info_headers( LPCWSTR headers[], 
//											  ULONG header_count )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//	WCHAR text[ DATA_COLUME_TEXT_LEN ]; 
//	_bstr_t data_contants; 
//	INT32 i; 
//	IHTMLElementPtr table_element; 
//
//	do 
//	{
//		if( header_count == 0 )
//		{
//			break; 
//		}
//
//		data_contants = "<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\" id=\"table\" class=\"sortable\">"
//			"<thead>"
//			"<tr>"; 
//
//		for( i = 0; ( ULONG )i < header_count; i ++ )
//		{
//			//do 
//			{
//				hr = StringCchPrintfW( text, 
//					ARRAYSIZE( text ), 
//					L"<th>" 
//					L"<h3>%s</h3>"
//					L"</th>", 
//					headers[ i ] ); 
//
//				if( FAILED(hr))
//				{
//					dbg_print( MSG_FATAL_ERROR, "format header text error 0x%0.8x\n", hr ); 
//					break; 
//				}
//			}
//			//}while( FALSE );
//
//			data_contants += text; 
//		}
//
//		data_contants += "</tr>" 
//			"</thead>" 
//			"</table>" 
//			"<tbody id=\"datas\">" 
//			"<tr id =\"last_data\"></tr>"
//			"</tbody>"; 
//
//	}while( FALSE );
//
//	get_element_from_id( L"divTable",&table_element );
//
//	hr = table_element->put_innerHTML( data_contants.GetBSTR() ); 
//	if(hr != S_OK)
//	{
//		dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
//	}
//	else
//	{
//		LPCSTR contant; 
//		contant = data_contants.operator char*(); 
//
//		dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
//	}
//
//	table_element.Release();
//	return ret;
//}

LRESULT data_html_dlg::set_page_contents( HTML_PAGE_CONTENT *page_content ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = init_scramble_data_info_headers( page_content ); 
	}while( FALSE );

	return ret; 
}

LRESULT data_html_dlg::init_scramble_data_info_headers( HTML_PAGE_CONTENT *contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	WCHAR text[ DATA_COLUME_TEXT_LEN ]; 
	_bstr_t data_contants; 
	IHTMLElementPtr table_element; 
	HTML_ELEMENT_CONTENTS_ITERATOR it; 

	do 
	{
		if( contents == NULL )
		{
			break; 
		}

		data_contants = "<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\" id=\"table\" class=\"sortable\">"
			"<thead>"
			"<tr>"; 

		for( it = contents->element_contents.begin(); it != contents->element_contents.end(); it ++ )
		{
			//do 
			{
				hr = StringCchPrintfW( text, 
					ARRAYSIZE( text ), 
					L"<th>" 
					L"<h3>%s</h3>"
					L"</th>", 
					( *it )->name.c_str() ); 

				if( FAILED(hr))
				{
					dbg_print( MSG_FATAL_ERROR, "format header text error 0x%0.8x\n", hr ); 
					break; 
				}
			}
			//}while( FALSE );

			data_contants += text; 
		}

		data_contants += "</tr>" 
			"</thead>" 
			"</table>" 
			"<tbody id=\"datas\">" 
			"<p id =\"last_data\"></p>"
			"</tbody>"; 

	}while( FALSE );

	get_element_from_id( L"divTable",&table_element );

	hr = table_element->put_innerHTML( data_contants.GetBSTR() ); 
	if(hr != S_OK)
	{
		dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
	}
	else
	{
		LPCSTR contant; 
		contant = data_contants.operator char*(); 

		dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
	}

	table_element.Release();
	return ret;
}

LRESULT data_html_dlg::add_scrambled_data_info( HTML_PAGE_CONTENT *contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
#define MAX_DATA_TEXT_LEN MAX_PATH
	WCHAR _text[ MAX_DATA_TEXT_LEN ]; 
	//LPWSTR remain_text; 
	//size_t remain_len; 
	_bstr_t data_contants; 
	IHTMLElementPtr last_row_element;
	HTML_ELEMENT_CONTENTS_ITERATOR it; 
	//INT32 i; 

	data_contants = "<p><tr>"; 

	do 
	{
		wstring output_text; 
		for( it = contents->element_contents.begin(); it != contents->element_contents.end(); it ++ )
		{
			if( ( *it )->text.length() > MAX_HTML_SHOW_TEXT_LEN )
			{
				output_text = ( *it )->text.substr( 0, MAX_HTML_SHOW_TEXT_LEN ); 
				output_text += L"..."; 
			}
			else
			{
				output_text = ( *it )->text.c_str(); 
			}

			hr = StringCchPrintfW( _text, 
				ARRAYSIZE( _text ), 
				L"<td>%s</td>", 
				output_text.c_str() ); 

			if( FAILED(hr))
			{
				//break; 
			}

			data_contants += _text; 
		}

		data_contants += "</tr></p>"; 

		//get_element_from_id(_T("datas"),&table_element );
		get_element_from_id(_T("last_data"),&last_row_element );

		//IHTMLElement2Ptr last_row_element2; 

		_bstr_t flags; 
		flags = L"beforeBegin"; 

		hr = last_row_element->insertAdjacentHTML( flags.GetBSTR(), 
			data_contants.GetBSTR() ); 

		if(hr != S_OK)
		{
			dbg_print( MSG_FATAL_ERROR, "put the data information header error 0x%0.8x\n", hr ); 
		}
		else
		{
			LPCSTR contant; 
			contant = data_contants.operator char*(); 

			dbg_print( MSG_IMPORTANT, "data contant is: %s\n", contant == NULL ? "" : contant ); 
		}

	}while( FALSE );

	//table_element.Release();
	return ret;
}

LRESULT data_html_dlg::add_data_info( ULONG count, ... )
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

HRESULT data_html_dlg::OnGetExternal( LPDISPATCH *lppDispatch )
{
	*lppDispatch = GetIDispatch( TRUE ); 
	return S_OK;
}

VOID data_html_dlg::OnDocumentComplete( LPCTSTR lpszURL )
{
	//ShowDataContants(0);
}