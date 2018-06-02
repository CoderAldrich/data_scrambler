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

#include "stdafx.h"
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include <strsafe.h>
#import "msxml.tlb"
#include "mshtml_addition.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "input_content.h"
#include "ie_auto.h"

/***********************************************************************************
使用WINDOWS IE插件来实现更全面的自动化处理功能。

***********************************************************************************/

LRESULT WINAPI find_page_group_config( LPCWSTR file_name, 
									  LPCWSTR domain_name, 
									  MSXML::IXMLDOMDocument **xml_doc_out, 
									  MSXML::IXMLDOMElement **site_element_out ); 
XPATH_PARAMS input_contents; 
CRITICAL_SECTION html_com_lock; 
LRESULT WINAPI init_ie_auto()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	wstring conf_file_name; 

	init_cs_lock( html_com_lock ); 

	do 
	{
		_ret = get_input_context_file_path( conf_file_name ); 
		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = load_input_content( conf_file_name.c_str(), input_contents ); 
		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_ie_auto()
{
	LRESULT ret = ERROR_SUCCESS; 
	
	uninit_cs_lock( html_com_lock ); 

	return ret; 
}

#include "Resource.h"
#include "web_browser_dlg.h"

//web_browser_dlg browser_dlg; 
//LRESULT WINAPI create_browser_dlg()
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	//DoModal() ) ///
//
//	do 
//	{
//		if( browser_dlg.GetSafeHwnd() != NULL )
//		{
//			break; 
//		}
//
//		if( browser_dlg.create_dialog() ) 
//		{
//
//			// hide the window before enabling the parent, etc.
//			//SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
//			//SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
//
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//		}
//	} while ( FALSE );
//	
//	return ret; 
//	//browser_dlg.Create( web_browser_dlg::IDD ); 
//}

//LRESULT WINAPI ie_auto_execute_on_site_internal( HTML_WEB_SITE_INFO *site )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr;	
//
//	do 
//	{
//		ASSERT( NULL != site ); 
//
//		browser_dlg.set_auto_html_info( site ); 
//
//		lock_cs( html_com_lock ); 
//
//		//hr = CoCreateInstance( CLSID_InternetExplorer, 
//		//	NULL, 
//		//	CLSCTX_LOCAL_SERVER, 
//		//	IID_IWebBrowser2, 
//		//	( LPVOID* ) &ie_browser ); 
//
//		browser_dlg.ShowWindow( SW_SHOW ); 
//
//		browser_dlg.OnTimer( 101 ); 
//		unlock_cs( html_com_lock ); 
//
//	}while( FALSE ); 
//
//	return ret; 
//}

LRESULT WINAPI ie_load_web_page( IWebBrowser2 *ie_browser, LPCWSTR url )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	BOOLEAN page_load_ready = FALSE; 
	ULONG wait_time = 0; 
	COleVariant url_variant; 
	//COleVariant url_variant; 
	COleVariant null;

	do 
	{
#define WAIT_WEG_PAGE_OPEN_DONE_TIME 200
#define MAX_WAIT_WEG_PAGE_OPEN_DONE_TIME 30000

		ASSERT( ie_browser != NULL ); 

		if( url != NULL 
			&& *url != L'\0' )
		{
			url_variant = url; 

			hr = ie_browser->Navigate2( url_variant, null, null, null, null ) ; //Open the URL page,

			if( FAILED( hr ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		while( !page_load_ready )  //This while loop mask sure that the page is fully loaded before we go to the next page
		{
			SHANDLE_PTR ie_window;
			READYSTATE ready_state; 

			//等待网页完全打开,退出循环
			//ie_browser->get_StatusText( html_response_statue.GetAddress() );

			//if( ( LPCWSTR )L"完毕" == html_response_statue 
			//	|| ( LPCWSTR )L"完成" == html_response_statue 
			//	|| ( LPCWSTR )L"Done" == html_response_statue )
			//{
			//	page_load_ready = 1;
			//	break;
			//}

			hr = ie_browser->get_ReadyState( &ready_state ); 

			if( SUCCEEDED( hr ) 
				&& ready_state == READYSTATE_COMPLETE) 
			{
				page_load_ready = TRUE;
				break;
			}

			//hr = ie_browser->Stop(); 

			//if( SUCCEEDED( hr ) ) 
			//{
			//	page_load_ready = TRUE; 
			//	break; 
			//} 

			Sleep( WAIT_WEG_PAGE_OPEN_DONE_TIME ); 

			wait_time += WAIT_WEG_PAGE_OPEN_DONE_TIME; 

			//新版本IE得不到HWND
			//ie_browser->get_HWND( &ie_window ); 
			//if( NULL == ie_window )
			//{
			//	page_load_ready = 1; 
			//	break; 
			//}

			if( wait_time > MAX_WAIT_WEG_PAGE_OPEN_DONE_TIME )
			{
				ret = WAIT_TIMEOUT; 
				dbg_print( MSG_FATAL_ERROR, "page %ws is not loaded completely\n", url == NULL ? L"" : url ); 
				//ASSERT( FALSE ); 
				break; 
			}
		}		
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_site( HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr;	

	_variant_t id, index;
	_bstr_t html_response_statue;

	IWebBrowser2Ptr ie_browser; 
	IDispatchPtr html_doc_disp;
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr element_collect = NULL; //Enumerate the HTML elements 
	IDispatchPtr html_element_disp; 
	HTML_PAGE_INFO *page; 
	HTML_PAGE_INFOS_ITERATOR it; 

	do 
	{
		ASSERT( NULL != site ); 

		lock_cs( html_com_lock ); 

		hr = CoCreateInstance( CLSID_InternetExplorer, 
			NULL, 
			CLSCTX_LOCAL_SERVER, 
			IID_IWebBrowser2, 
			( LPVOID* ) &ie_browser ); 

		unlock_cs( html_com_lock ); 

		if( hr != S_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 

			dbg_print( MSG_FATAL_ERROR,"CoCreateInstance fail a=%d hr = 0x%x\n", hr );  
			break; 
		}

		for( it = site->pages.begin(); it != site->pages.end(); it ++ )
		{
			do 
			{
				page = ( *it ); 


#if 1 //def HIDE_BROWSER
				ie_browser->put_Visible( TRUE ); //Comment out this line if you do not want the browser to be displayed
#endif //HIDE_BROWSER

				ret = ie_load_web_page( ie_browser, page->url.c_str() ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				hr = ie_browser->get_Document( &html_doc_disp ); //Get the underlying document object of the browser

				if( html_doc_disp == NULL )
				{
					break; 
				}

				ret = ie_auto_execute_on_page( page, html_doc_disp, ie_browser ); 

				if( ret != ERROR_SUCCESS )
				{
					//dbg_print()
				}

				//Sleep( 200 ); 
			}while( FALSE ); 
#ifdef HIDE_BROWSER
			ie_browser->put_Visible( TRUE ); 
#endif //HIDE_BROWSER
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI _ie_auto_execute_on_site( IDispatch *html_doc_disp, 
										HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr;	

	HTML_PAGE_INFO *page; 
	HTML_PAGE_INFOS_ITERATOR it; 

	do 
	{
		ASSERT( NULL != site ); 

		for( it = site->pages.begin(); it != site->pages.end(); it ++ )
		{
			do 
			{
				page = ( *it ); 

				//ret = ie_auto_execute_on_page( page, html_doc_disp, NULL ); 

				//if( ret != ERROR_SUCCESS )
				//{
				//	//dbg_print()
				//}

			}while( FALSE ); 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_input_element( IHTMLElement *html_element, 
										HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLInputElementPtr input_element;//We need to check for input elemnt on login screen
	IHTMLInputTextElementPtr input_text_element;//We need to check for input elemnt on login screen
	IHTMLInputButtonElementPtr input_button_element; 

	LPCWSTR temp_text; 
	LPCWSTR class_name; 
	BOOLEAN goto_next_page = FALSE; 
	BOOLEAN element_found = FALSE; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != element_info ); 

		_html_element = html_element; 

		//Ask for an HTMLElemnt interface
		//if( element_info->info_type != info_type ) 
		//{
		//	break; 
		//}

		if( element_info->info_type != HTML_INPUT_ELEMENT ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do 
		{
			hr = _html_element->QueryInterface( IID_IHTMLInputTextElement,( void** )&input_text_element );
			if( hr != S_OK )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			element_found = TRUE; 

			if( element_info->name.length() != 0 )
			{
				input_text_element->get_name( text.GetAddress() );

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( element_info->id.length() != 0 )
			{
				hr = _html_element->get_id( text.GetAddress() );
				if( FAILED( hr ) )
				{
					break; 
				}

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( element_info->input_text.length() == 0 )
			{
				XPATH_PARAM_ITERATOR content_it; 

				if( element_info->content_name.length() == 0 )
				{
					break; 
				}

				content_it = input_contents.find( element_info->content_name.c_str() ); 
				if ( content_it == input_contents.end() )
				{
					dbg_print( MSG_INFO, "input content %ws is removed\n", element_info->content_name.c_str() ); 
					text = L""; 
				}
				else
				{
					text = content_it->second.c_str();
				}
			}
			else
			{
				text = element_info->input_text.c_str(); 
			}

			input_text_element->put_value( text.GetBSTR() ); //Paste the User Id 
		}while( FALSE ); 

		if( TRUE == element_found )
		{
			break; 
		}

		do
		{
			hr = _html_element->QueryInterface( IID_IHTMLInputElement,( void** )&input_element ); 

			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == input_element )
			{
				break; 
			}

			//hr = input_element->get_type( text.GetAddress() ); 
			//if( FAILED( hr ) )
			//{
			//	break; 
			//}

			//temp_text = text.operator wchar_t*(); 
			//if( NULL == temp_text )
			//{
			//	break; 
			//}

			if( 0 != element_info->type.length() )
			{
				input_element->get_type( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->type.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( 0 != _wcsicmp( L"checkbox", element_info->type.c_str() ) )
			{
				break; 
			}

			element_found = TRUE; 
			if( 0 != element_info->name.length() )
			{
				input_element->get_name( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
				{
					break; 
				}
			}

			//<input class="btn" name="regsubmit" type="submit" value="同 意">
			//if( 0 != element_info->value.length() )
			//{
			//	do 
			//	{
			//		input_element->get_value( text.GetAddress() ); 

			//		if( 0 == text.length() )
			//		{
			//			break; 
			//		}

			//		temp_text = text.operator wchar_t*(); 
			//		if( NULL == temp_text )
			//		{
			//			break; 
			//		}

			//		if( 0 != wcsicmp( element_info->value.c_str(), temp_text ) )
			//		{
			//			break; 
			//		}
			//	} while ( FALSE );
			//}

			if( 0 != element_info->input_text.length() )
			{
				if( 0 == _wcsicmp( L"checked", element_info->input_text.c_str() ) )
				{
					input_element->put_checked( VARIANT_TRUE ); 
				}
				else
				{
					input_element->put_checked( VARIANT_FALSE ); 
				}
			} 
			else
			{
				input_element->put_checked( VARIANT_FALSE ); 
			}
		}while( FALSE ); 

		if( TRUE == element_found )
		{
			break; 
		}

		do 
		{
			hr = _html_element->QueryInterface( IID_IHTMLInputButtonElement,( void** )&input_button_element );
			if( hr != S_OK )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( 0 != element_info->type.length() )
			{
				input_button_element->get_type( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->type.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( element_info->name.length() != 0 )
			{
				input_button_element->get_name( text.GetAddress() );

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( element_info->id.length() != 0 )
			{
				hr = _html_element->get_id( text.GetAddress() );
				if( FAILED( hr ) )
				{
					break; 
				}

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
				{
					break; 
				}
			}

			html_element->click(); 
			goto_next_page = TRUE; 

		}while( FALSE ); 

	}while( FALSE ); 

	if( TRUE == goto_next_page )
	{
		ret = ERROR_GOTO_NEXT_WEB_PAGE; 
	}

	return ret; 
}

LRESULT WINAPI execute_on_button_element( IHTMLElement *html_element, 
										HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLButtonElementPtr button_element; 
	BOOLEAN goto_next_page = FALSE; 

	LPCWSTR temp_text; 
	LPCWSTR class_name; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != element_info ); 

		_html_element = html_element; 

		//Ask for an HTMLElemnt interface
		//if( element_info->info_type != info_type ) 
		//{
		//	break; 
		//}

		if( element_info->info_type != HTML_BUTTON_ELEMENT ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do 
		{
			button_element = NULL; 
			hr = _html_element->QueryInterface( IID_IHTMLButtonElement,( void** )&button_element );
			if( hr != S_OK )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			button_element->get_name( text.GetAddress() );

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( element_info->name.length() != 0 )
			{
				if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
				{
					break; 
				}
			}

			hr = _html_element->get_id( text.GetAddress() );
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( element_info->id.length() != 0 )
			{
				if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
				{
					break; 
				}
			}

			_html_element->click(); 
			goto_next_page = TRUE; 
		}while( FALSE ); 

	}while( FALSE ); 

	if( TRUE == goto_next_page )
	{
		ret = ERROR_GOTO_NEXT_WEB_PAGE; 
	}

	return ret; 
}

/***********************************************************************
<body contenteditable="true" spellcheck="false" style="height: 400px;"><p>WINDOWS系统行为跟踪软件，使你更深入的了解WINDOWS中程序内部工作机制BITTRACE WINDOWS系统行为跟踪软件，可以全面，清晰的展示出WINDOWS系统和各个程序的内部工作细节，包括网络，文件系统，注册表，进程/线程，系统关键数据结构。并且抓取了行为内部的数据流，可以展示向文件（注册表，网络）写入，从文件（注册表，网络）读取的数据内容，并且具有非常高效的展示UI，可以高速的显示出行为各个细节信息。让你对各个方面的行为一目了然。相对PROCMON，具有更细节的信息内容，和数据内容展示。举一个例子：1.运行bittrace 2.运行bcdedit /set {default} bootmenupolicy legacy3.可以从bittrace中通过查找/过滤设置注册表键值的行为，找到这个命令修改的注册表键值的路径。下载地址：www.simplestai.com QQ讨论群：271355765 真心的希望可以帮助大家探索WINDOWS内部的秘密。</p></body>
***********************************************************************/

LRESULT WINAPI execute_on_body_element( IHTMLElement *html_element, 
										HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLBodyElementPtr body_element; 

	IDispatchPtr _html_element_disp; 

	LPCWSTR temp_text; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		//BOOLEAN found = FALSE; 

		//if( element_info->info_type != HTML_BODY_ELEMENT ) 
		//{
		//	break; 
		//}

		_html_element = html_element; 

		hr = _html_element->QueryInterface( IID_IHTMLBodyElement,( void** )&body_element ); 

		if( hr != S_OK )
		{
			break; 
		}

		if( 0 != element_info->id.length() )
		{
			html_element->get_id( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
			{
				break; 
			}
		}

		do 
		{
			if( 0 != element_info->style.length() )
			{
				IHTMLStylePtr style; 
				hr = _html_element->get_style( &style ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				hr = style->get_cssText( text.GetAddress() ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				if( 0 != _wcsicmp( element_info->style.c_str(), temp_text ) )
				{
					break; 
				}
			}
		}while( FALSE );

		//do 
		{
			//<input class="btn" name="regsubmit" type="submit" value="同 意">
			if( 0 != element_info->content_editable.length() )
			{
				VARIANT_BOOL is_edit; 
				hr = _html_element->get_isTextEdit( &is_edit ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				if( is_edit == VARIANT_TRUE )
				{
					//element_info->content_editable
					if( 0 != _wcsicmp( element_info->content_editable.c_str(), L"true" ) )
					{
						break; 
					}
				}
				else
				{
					if( 0 != _wcsicmp( element_info->content_editable.c_str(), L"false" ) )
					{
						break; 
					}
				}
			}
		} //while ( FALSE );

		if( 0 != element_info->content.length() )
		{
			//if( found == TRUE )
			//{
			//	break; 
			//}

			hr = html_element->get_innerHTML( text.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			//html_element->get_innerText( text.GetAddress() ); 

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->content.c_str(), temp_text ) )
			{
				break; 
			}
		}
		
		{
			_bstr_t value; 
			//_variant_t value; 
			if( element_info->input_text.length() == 0 )
			{
				XPATH_PARAM_ITERATOR content_it; 

				if( element_info->content_name.length() == 0 )
				{
					break; 
				}

				content_it = input_contents.find( element_info->content_name.c_str() ); 
				if ( content_it == input_contents.end() )
				{
					dbg_print( MSG_INFO, "input content %ws is removed\n", element_info->content_name.c_str() ); 
					value = L""; 
				}
				else
				{
					value = content_it->second.c_str();
				}
			}
			else
			{
				value = element_info->input_text.c_str(); 
			}

			html_element->put_innerHTML( value.GetBSTR() ); 

			//body_element->put_text( value.GetVARIANT() ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_div_element( IHTMLElement *html_element, 
									   HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLDivElementPtr div_element; 

	//IDispatchPtr _html_element_disp; 

	LPCWSTR temp_text; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		//BOOLEAN found = FALSE; 

		//if( element_info->info_type != HTML_DIV_ELEMENT ) 
		//{
		//	break; 
		//}

		_html_element = html_element; 

		hr = _html_element->QueryInterface( IID_IHTMLDivElement,( void** )&div_element ); 

		if( hr != S_OK )
		{
			break; 
		}

		if( 0 != element_info->id.length() )
		{
			html_element->get_id( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
			{
				break; 
			}
		}

		if( 0 != element_info->style.length() )
		{
			IHTMLStylePtr style; 
			hr = _html_element->get_style( &style ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			hr = style->get_cssText( text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			//temp_text = text.operator wchar_t*(); 
			//if( NULL == temp_text )
			//{
			//	break; 
			//}

			//if( 0 != wcsicmp( element_info->style.c_str(), temp_text ) )
			//{
			//	break; 
			//}
		}

		//<input class="btn" name="regsubmit" type="submit" value="同 意">
		//if( 0 != element_info->content_editable.length() )
		//{
		//	VARIANT_BOOL is_edit; 
		//	hr = _html_element->get_isTextEdit( &is_edit ); 
		//	if( FAILED( hr ) )
		//	{
		//		break; 
		//	}

		//	if( is_edit == VARIANT_TRUE )
		//	{
		//		if( 0 != wcsicmp( element_info->value.c_str(), L"1" ) )
		//		{
		//			break; 
		//		}
		//	}
		//	else
		//	{
		//		if( 0 != wcsicmp( element_info->value.c_str(), L"0" ) )
		//		{
		//			break; 
		//		}
		//	}
		//}

		if( 0 != element_info->content.length() )
		{
			//if( found == TRUE )
			//{
			//	break; 
			//}

			html_element->get_innerText( text.GetAddress() ); 

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->content.c_str(), temp_text ) )
			{
				break; 
			}
		}

		{
			IDispatchPtr element_collect_disp; 
			IHTMLElementCollectionPtr element_collect = NULL; //Enumerate the HTML elements 
			LONG element_count; 
			IHTMLElementPtr html_sub_element; 
			IDispatchPtr html_element_disp; 
			_variant_t id, index;

			if( element_info->input_text.length() == 0 )
			{
				XPATH_PARAM_ITERATOR content_it; 

				if( element_info->content_name.length() == 0 )
				{
					break; 
				}

				content_it = input_contents.find( element_info->content_name.c_str() ); 
				if ( content_it == input_contents.end() )
				{
					dbg_print( MSG_INFO, "input content %ws is removed\n", element_info->content_name.c_str() ); 
					text = L""; 
				}
				else
				{
					text = content_it->second.c_str();
				}
			}
			else
			{
				text = element_info->input_text.c_str(); 
			}

			//html_element->put_title( text.GetBSTR() );
			html_element->get_children( &element_collect_disp ); 

			if( hr != S_OK 
				|| element_collect_disp == NULL ) 
			{
				break; 
			}

			hr = element_collect_disp->QueryInterface(IID_IHTMLElementCollection, (void**)&element_collect );
			if ( hr != S_OK )
			{
				break; 
			}

			hr = element_collect->get_length( &element_count );//Find the count of the elements

			if ( hr != S_OK 
				|| element_count == 0 )
			{
				break; 
			}

			V_VT( &id ) = VT_I4;
			V_I4( &id ) = 0;
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			hr = element_collect->item( id, index, &html_element_disp ); //Get an element

			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == html_element_disp )
			{
				break; 
			}

			//Ask for an HTMLElemnt interface
			hr = html_element_disp->QueryInterface( IID_IHTMLElement,( void** )&html_sub_element );
			if( hr != S_OK )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			html_sub_element->put_innerText( text.GetBSTR() ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_select_element( IHTMLElement *html_element, 
									   HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLSelectElementPtr select_element; 

	LPCWSTR temp_text; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		//BOOLEAN found = FALSE; 

		//if( element_info->info_type != HTML_SELECT_ELEMENT ) 
		//{
		//	break; 
		//}

		_html_element = html_element; 

		hr = _html_element->QueryInterface( IID_IHTMLSelectElement,( void** )&select_element ); 

		if( hr != S_OK )
		{
			break; 
		}

		if( 0 != element_info->name.length() )
		{
			select_element->get_name( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
			{
				break; 
			}
		}

		if( 0 != element_info->type.length() )
		{
			select_element->get_type( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->type.c_str(), temp_text ) )
			{
				break; 
			}
		}

		//<input class="btn" name="regsubmit" type="submit" value="同 意">
		if( 0 != element_info->value.length() )
		{
			hr = _html_element->get_innerText( text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->value.c_str(), temp_text ) )
			{
				break; 
			}
		}

		if( 0 != element_info->content.length() )
		{
			//if( found == TRUE )
			//{
			//	break; 
			//}

			select_element->get_value( text.GetAddress() ); 

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->content.c_str(), temp_text ) )
			{
				break; 
			}
		}

		{
			if( element_info->input_text.length() == 0 )
			{
				XPATH_PARAM_ITERATOR content_it; 

				if( element_info->content_name.length() == 0 )
				{
					break; 
				}

				content_it = input_contents.find( element_info->content_name.c_str() ); 
				if ( content_it == input_contents.end() )
				{
					dbg_print( MSG_INFO, "input content %ws is removed\n", element_info->content_name.c_str() ); 
					text = L""; 
				}
				else
				{
					text = content_it->second.c_str();
				}
			}
			else
			{
				text = element_info->input_text.c_str(); 
			}

			html_element->put_innerText( text.GetBSTR() ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_text_area_element( IHTMLElement *html_element, 
											HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	_bstr_t html_element_class; 
	IHTMLElementPtr _html_element; 
	IHTMLTextAreaElementPtr text_area_element; 

	IDispatchPtr _html_element_disp; 

	LPCWSTR temp_text; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		//BOOLEAN found = FALSE; 

		//if( element_info->info_type != HTML_TEXT_AREA_ELEMENT ) 
		//{
		//	break; 
		//}

		_html_element = html_element; 

		hr = _html_element->QueryInterface( IID_IHTMLTextAreaElement,( void** )&text_area_element ); 

		if( hr != S_OK )
		{
			break; 
		}

		if( 0 != element_info->name.length() )
		{
			text_area_element->get_name( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->name.c_str(), temp_text ) )
			{
				break; 
			}
		}

		if( 0 != element_info->type.length() )
		{
			text_area_element->get_type( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->type.c_str(), temp_text ) )
			{
				break; 
			}
		}

		//<input class="btn" name="regsubmit" type="submit" value="同 意">
		if( 0 != element_info->value.length() )
		{
			hr = _html_element->get_innerText( text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->value.c_str(), temp_text ) )
			{
				break; 
			}
		}

		if( 0 != element_info->content.length() )
		{
			//if( found == TRUE )
			//{
			//	break; 
			//}

			text_area_element->get_value( text.GetAddress() ); 

			if( 0 == text.length() )
			{
				break; 
			}

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			if( 0 != _wcsicmp( element_info->content.c_str(), temp_text ) )
			{
				break; 
			}
		}


		{
			if( element_info->input_text.length() == 0 )
			{
				XPATH_PARAM_ITERATOR content_it; 

				if( element_info->content_name.length() == 0 )
				{
					break; 
				}

				content_it = input_contents.find( element_info->content_name.c_str() ); 
				if ( content_it == input_contents.end() )
				{
					dbg_print( MSG_INFO, "input content %ws is removed\n", element_info->content_name.c_str() ); 
					text = L""; 
				}
				else
				{
					text = content_it->second.c_str();
				}
			}
			else
			{
				text = element_info->input_text.c_str(); 
			}

			html_element->put_innerText( text.GetBSTR() ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_html_element( IHTMLElement* html_element, 
									   HTML_ELEMENT_ACTION *action, 
									   HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t temp_text; 
	LPWSTR _temp_text; 
	_variant_t value; 
	wstring cmd; 

	do 
	{
		ASSERT( NULL != html_element ); 
		cmd = action->action; 
		
		if( 0 == _wcsicmp( cmd.c_str(), L"click" ) )
		{
			html_element->click(); 
		}
		else if( 0 == _wcsicmp( cmd.c_str(), L"get_data" ) )
		{

			//2 Returns attribute value as a BSTR. This flag does not work for event properties. 
			temp_text = action->input.c_str(); 

			hr = html_element->getAttribute( temp_text.GetBSTR(), 
				2, 
				value.GetAddress() ); 

			if( FAILED( hr ) )
			{
				break; 
			}

			if( value.vt != VT_BSTR )
			{
				hr = E_INVALIDARG; 
				break; 
			}

			temp_text = value.bstrVal; 

			_temp_text = ( LPWSTR )temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				action->output = L""; 
			}
			else
			{
				action->output = _temp_text; 
			}
		}
		else if( 0 == _wcsicmp( cmd.c_str(), L"set_data" ) )
		{
			_bstr_t string_value; 
			temp_text = action->input.c_str(); 

			string_value = action->output.c_str(); 

			value = string_value; 
			hr = html_element->setAttribute( temp_text.GetBSTR(), 
				value.GetAddress() ); 

			if( FAILED( hr ) )
			{
				break; 
			}
		}
		else if( 0 == _wcsicmp( cmd.c_str(), L"click" ) )
		{
			html_element->click(); 
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while (FALSE ); 

	return ret; 
}

LRESULT WINAPI execute_on_a_element( IHTMLElement *html_element, 
									HTML_ELEMENT_ACTION *action, 
									HTML_ELEMENT_INFO *element_info ) 
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	_bstr_t text;

	IHTMLElementPtr _html_element; 
	IHTMLAnchorElementPtr link_element; 
	//IHTMLLinkElement2Ptr link_element; 

	LPCWSTR temp_text; 
	//LPCWSTR class_name; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != element ); 
		ASSERT( NULL != action); 

		_html_element = html_element; 

		do 
		{
			//if( 0 != wcsicmp( class_name, HTML_ELEMENT_TYPE_A ) ) 
			//{
			//	break; 
			//}

			//<a href="#" id="quick_reply" class="btn-small btn-sub j_quick_reply">

			hr = _html_element->QueryInterface(IID_IHTMLAnchorElement,( void** )&link_element );
			if ( hr != S_OK )
			{
				break; 
			}

			_html_element->get_outerHTML( text.GetAddress() ); 

			if( 0 != element_info->content.length() )
			{
				_html_element->get_innerText( text.GetAddress() ); 
				//link_element->get_href( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 

				if( 0 != _wcsicmp( element_info->content.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( 0 != element_info->id.length() )
			{
				_html_element->get_id( text.GetAddress() ); 
				//link_element->get_href( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 

				if( 0 != _wcsicmp( element_info->id.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( 0 != element_info->_class.length() )
			{
				_html_element->get_className( text.GetAddress() ); 
				//link_element->get_href( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 

				if( 0 != _wcsicmp( element_info->_class.c_str(), temp_text ) )
				{
					break; 
				}
			}

			if( 0 != element_info->href.length() )
			{ 
				link_element->get_href( text.GetAddress() ); 

				temp_text = text.operator wchar_t*(); 
				if( NULL == temp_text )
				{
					break; 
				}

				dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 

				if( 0 != _wcsicmp( element_info->href.c_str(), temp_text ) )
				{
					break; 
				}
			}
		}while( FALSE ); 
	}while( FALSE ); 

	if( TRUE == goto_next_page )
	{
		ret = ERROR_GOTO_NEXT_WEB_PAGE; 
	}

	return ret; 
}

//LPCWSTR html_element_tags[] = { HTML_ELEMENT_TAG_INPUT, 
//HTML_ELEMENT_TAG_BUTTON, 
//HTML_ELEMENT_TAG_A, 
//HTML_ELEMENT_TAG_BODY, 
//HTML_ELEMENT_TAG_DIV, 
//HTML_ELEMENT_TAG_TEXTAREA, 
//HTML_ELEMENT_TAG_SELECT }; 

LRESULT WINAPI ie_auto_execute_on_element_common( HTML_ELEMENT_INFO *element_info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != element_info ); 

		if( element_info->sleep_time != 0 )
		{
			Sleep( element_info->sleep_time ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_element( HTML_ELEMENT_INFO *element )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	IHTMLElementPtr _html_element; 

	HTML_ELEMENT_GROUP_ELEMENT_ITERATOR group_it; 
	HTML_ELEMENT_INFO_ITERATOR it; 

	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		ASSERT( NULL != html_element ); 

		for( group_it = element->elements.begin(); 
			group_it != element->elements.end(); 
			group_it ++ )
		{
			_html_element = ( *group_it ); 

			_ret = _ie_auto_execute_on_element( _html_element, element ); 
		
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
			}
		}
	}while( FALSE );  

	return ret; 
}

LRESULT WINAPI __ie_auto_execute_on_element( IHTMLElement *element, 
											HTML_ELEMENT_ACTIONS *actions, 
											HTML_ELEMENT_INFO *element_info )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	_bstr_t html_element_tag; 
	LPCWSTR tag_name; 
	HTML_ELEMENT_ACTIONS_ITERATOR it; 

	do 
	{
		//html_element->get_innerText( )
		hr = element->get_tagName( html_element_tag.GetAddress() ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		tag_name = html_element_tag.operator wchar_t*(); 
		if( NULL == tag_name )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//{
			//HTML_ELEMENT_TYPE type; 
		//	HTML_ELEMENT_ACTION_TYPE action_type; 
		//	
		//	info_type = get_info_type( tag_name ); 
		//	action_type = get_html_element_action_type( info_type ); 
		//	if( action_type != type_index )
		//	{
		//		break; 
		//	}
		//}

		dbg_print( MSG_IMPORTANT, "%ws\n", tag_name ); 

		//if( 0 == wcsicmp( tag_name, L"a" ) )
		//{
		//	for( it = actions->begin; it != actions->end(); it ++ )
		//	{
		//		_ret = execute_on_a_element( element, ( *it ), element_info ); 
		//		if( _ret != ERROR_SUCCESS )
		//		{
		//			ret = _ret; 
		//		}
		//	}
		//}
		//else if( 0 == wcsicmp( tag_name, L"body" ) )
		//{
		//	for( it = actions->begin; it != actions->end(); it ++ )
		//	{
		//		_ret = execute_on_body_element( element, ( *it ), element_info); 
		//		if( _ret != ERROR_SUCCESS )
		//		{
		//			ret = _ret; 
		//		}
		//	}
		//}
		//else if( 0 == wcsicmp( tag_name, L"div" ) )
		//{
		//	for( it = actions->begin; it != actions->end(); it ++ )
		//	{
		//		_ret = execute_on_div_element( element, ( *it ), element_info); 
		//		if( _ret != ERROR_SUCCESS )
		//		{

		//		}
		//	}
		//}
		//else if( 0 == wcsicmp( tag_name, L"area" ) )
		//{
		//	for( it = actions->begin; it != actions->end(); it ++ )
		//	{
		//		_ret = execute_on_text_area_element( element, ( *it ), element_info ); 
		//		if( _ret != ERROR_SUCCESS )
		//		{

		//		}
		//	}
		//}
		//else if( 0 == wcsicmp( tag_name, L"select" ) )
		//{
		//	for( it = actions->begin; it != actions->end(); it ++ )
		//	{
		//		_ret = execute_on_select_element( element, ( *it ), element_info ); 
		//		if( _ret != ERROR_SUCCESS )
		//		{

		//		}
		//	}
		//}
		//else
		for( it = actions->begin; it != actions->end(); it ++ )
		{
			_ret = execute_on_html_element( element, ( *it ), element_info ); 
			if( _ret != ERROR_SUCCESS )
			{

			}
		}

		//{
		//	ASSERT( FALSE ); 
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _ie_auto_execute_on_element( IHTMLElement *html_element, 
										   HTML_ELEMENT_INFO *element )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	IHTMLElementPtr _html_element; 
	HTML_ELEMENT_ACTIONS_ITERATOR it; 
	BOOLEAN goto_next_page = FALSE; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != page ); 

		for( it = element->actions.begin(); it != element->actions.end(); it ++ )
		{
			do 
			{
				_ret = __ie_auto_execute_on_element( _html_element, 
					( *it ), 
					element ); 

				if( _ret != ERROR_SUCCESS )
				{

				}
			}while( FALSE ); 

			if( ERROR_GOTO_NEXT_WEB_PAGE == _ret )
			{
				ret = _ret; 
				break; 
			}
		} 
	}while( FALSE );  

	return ret; 
}

ULONG CALLBACK ie_auto_execute_work( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != param ); 

		ret = ie_auto_execute_on_site( ( HTML_WEB_SITE_INFO* )param ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_site_async( HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE thread_handle = NULL; 

	do 
	{
		ASSERT( site != NULL ); 

		thread_handle = ::CreateThread( NULL, 0, ie_auto_execute_work, ( PVOID )site, 0, NULL ); 
		if( NULL == thread_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	if( NULL != thread_handle )
	{
		CloseHandle( thread_handle ); 
	}

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_sites_async( HTML_WEB_SITE_INFOS *sites )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_WEB_SITE_INFO_ITERATOR it; 

	do 
	{
		ASSERT( NULL != sites ); 

		for( it = sites->begin(); it != sites->end(); it ++ )
		{
			_ret = ie_auto_execute_on_site_async( ( *it ) ); 

			if( _ret != ERROR_SUCCESS )
			{
			}
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_sites( HTML_WEB_SITE_INFOS *sites )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_WEB_SITE_INFO_ITERATOR it; 

	do 
	{
		ASSERT( NULL != sites ); 

		for( it = sites->begin(); it != sites->end(); it ++ )
		{
			_ret = ie_auto_execute_on_site( ( *it ) ); 

			if( _ret != ERROR_SUCCESS )
			{
			}
		}

	}while( FALSE );

	return ret; 
}


LRESULT WINAPI test_frame_element( IHTMLDocument2 *html_doc )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTMLFramesCollection2Ptr frames; 
	IDispatchPtr disp; 
	HRESULT hr; 
	VARIANT varindex; 
	VARIANT varresult;  
	LONG frame_count; 
	IHTMLWindow2Ptr html_wnd; 
	IHTMLDocument2Ptr _html_doc; 

	do 
	{
		ASSERT( html_doc != NULL ); 
		
		hr = html_doc->get_frames( &frames ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		hr = frames->get_length( &frame_count ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		varresult.vt = VT_DISPATCH;   
		varindex.vt = VT_I4;
		varindex.lVal = 0;

		hr = frames->item( &varindex, &varresult ); 
		if( S_OK != hr )
		{
			break; 
		}

		disp = ( LPDISPATCH )varresult.ppdispVal; 

		if(SUCCEEDED(disp->QueryInterface(IID_IHTMLWindow2, ( LPVOID* )&html_wnd ) ) )   
		{
			ret = html_window_2_html_document( html_wnd, &_html_doc ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

		}
	}while( FALSE );

	return ret; 
}

/****************************************************
返回值不精确，需要完善
****************************************************/
LRESULT WINAPI get_target_html_elements( HTML_PAGE_INFO *page, 
										IHTMLDocument2 *html_doc )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementCollectionPtr element_collect = NULL; //Enumerate the HTML elements 
	IHTMLElementCollectionPtr element_sub_collect; 
	IDispatchPtr html_element_disp; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr html_sub_element; 
	LONG sub_elements_count; 
	BOOLEAN sub_items_found; 
	LONG i; 
	//LONG _index; 
	_bstr_t text; 
	_variant_t index; 
	_variant_t id; 
	HTML_ELEMENT_INFO_ITERATOR it; 
	HTML_ELEMENT_GROUP element_group; 
	HTML_ELEMENT_GROUP group; 

	do 
	{
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != page ); 

#ifdef _DEBUG
		//test_frame_element( html_doc ); 
#endif //_DEBUG
		
		hr = html_doc->get_all( &element_collect ); 
		if( hr != S_OK 
			|| element_collect == NULL ) 
		{
			html_doc->Release();
			break; 
		}

		//hr = element_collect->get_length( &element_count );//Find the count of the elements

		//if ( hr != S_OK )
		//{
		//	html_doc->Release();
		//	element_collect->Release();
		//	break; 
		//}

		for( it = page->elements.begin(); 
			it != page->elements.end(); 
			it ++ )
		{
			if( ( ( *it )->in_frame == FALSE )
				&& ( ( *it )->id.length() > 0
				|| ( *it )->name.length() > 0 ) )
			{
				sub_items_found = FALSE; 
			
				/*****************************************************************
				高速定位元素方法：
				1.使用name,id进行定位
				2.使用全路径进行定位
				3.两者合一
				xpath="+//*[@id="quick_reply"]" 
				*****************************************************************/
				
				if( ( *it )->id.length() )
				{
					id = ( *it )->id.c_str(); 
				}
				else if( ( *it )->name.length() )
				{
					id = ( *it )->name.c_str(); 
				}

				V_VT( &index ) = VT_I4;
				V_I4( &index ) = 0; 

				do 
				{
					hr = element_collect->item( id, index, &html_element_disp ); //Get an element

					if( hr != S_OK )
					{
						ret = hr; 
						break; 
					}

					if( NULL == html_element_disp )
					{
						ret = ERROR_NOT_FOUND; 
						break; 
					}

					do 
					{
						hr = html_element_disp->QueryInterface( IID_IHTMLElementCollection, 
							( PVOID* )&element_sub_collect ); 

						if( hr != S_OK )
						{
							break; 
						}

						if( NULL == element_sub_collect )
						{
							break; 
						}

						hr = element_sub_collect->get_length( &sub_elements_count );//Find the count of the elements

						if ( hr != S_OK 
							|| sub_elements_count == 0 )
						{
							break; 
						}

						sub_items_found = TRUE; 

						for( i = 0; i < sub_elements_count; i ++ )
						{
							V_VT( &id ) = VT_I4; 
							V_I4( &id ) = i; 
							V_VT( &index ) = VT_I4; 
							V_I4( &index ) = 0; 

							hr = element_collect->item( id, 
								index, 
								&html_element_disp ); //get an element

							if( hr != S_OK )
							{
								break; 
							}

							if( NULL == html_element_disp )
							{
								break; 
							}

							//Ask for an HTMLElemnt interface
							hr = html_element_disp->QueryInterface( IID_IHTMLElement,( void** )&html_sub_element );
							if( hr != S_OK )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							ret = add_html_element_to_group( html_element, 
								&group ); 

							if( ERROR_SUCCESS != ret )
							{
								//break; 
							}
						}
					}while( FALSE ); 

					if( sub_items_found == TRUE )
					{
						break; 
					}

					do 
					{
						hr = html_element_disp->QueryInterface( IID_IHTMLElement, 
							( PVOID* )&html_sub_element ); 

						if( hr != S_OK )
						{
							break; 
						}

						if( NULL == html_sub_element )
						{
							break; 
						}

#ifdef _DEBUG
						do
						{
							LPCWSTR __tag_name; 
							_bstr_t _tag_name; 
							hr = html_sub_element->get_tagName( _tag_name.GetAddress() ); 
							if( FAILED( hr ) )
							{
								break; 		
							}

							__tag_name = _tag_name.operator const wchar_t*(); 
							if( NULL == __tag_name )
							{
								break; 
							}

							ASSERT( 0 == _wcsicmp( __tag_name, ( *it )->tag.c_str() ) ); 
						}while( FALSE ); 
#endif //_DEBUG

						ret = add_html_element_to_group( html_sub_element, 
							&group ); 

						if( ERROR_SUCCESS != ret )
						{
							//break; 
						}

					}while( FALSE ); 
				}while( FALSE ); 
			}
			else if( ( *it )->xpath.length() > 0 )
			{
				XPATH_PARAMS **params = NULL; 
				XPATH_PARAM_ITERATOR param_it; 
				ULONG params_count = 0; 
				LONG element_count; 
				wstring tag; 
				wstring _id; 
				wstring name; 
				LONG precise_param_index = -1; 

				do 
				{
					params = ( XPATH_PARAMS** )malloc( sizeof( XPATH_PARAMS* ) * MAX_XPATH_ELEMENT_PARAMETER_COUNT ); 
					if( NULL == params )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					ret = get_xpath_params( ( *it )->xpath.c_str(), 
						( *it )->xpath.length(), 
						params, 
						MAX_XPATH_ELEMENT_PARAMETER_COUNT, 
						&params_count ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( params_count >= 2 ); //HTML|BODY|DIV

					{
						LONG param_index; 

						//must handle have frame case
						for( param_index = 0; param_index < ( LONG )params_count; param_index ++ )
						//for( param_index = ( LONG )params_count - 1; param_index >= 0; param_index -- )
						{
							param_it = params[ param_index ]->find( L"id" ); 

							if( param_it != params[ param_index ]->end() )
							{
								precise_param_index = param_index; 
							}
							else
							{
								param_it = params[ param_index ]->find( L"name" ); 

								if( param_it != params[ param_index ]->end() )
								{
									precise_param_index = param_index; 
								}
							}

							param_it = params[ param_index ]->find( L"tag" ); 

							if ( param_it != params[ param_index ]->end() )
							{
								tag = param_it->second.c_str(); 
								
								if( 0 == _wcsicmp( L"IFRAME", tag.c_str() ) 
									|| 0 == _wcsicmp( L"FRAME", tag.c_str() ) )
								{
									precise_param_index = param_index; 
									break; 
								}
							}
						}

						if( precise_param_index < 0 )
						{
							precise_param_index = 1; //locate first form second element level, be not html.
						}
					
						param_it = params[ precise_param_index ]->find( L"name" ); 

						if ( param_it == params[ precise_param_index ]->end() )
						{
							name = L""; 
						}
						else
						{
							name = param_it->second.c_str(); 
						}

						param_it = params[ precise_param_index ]->find( L"id" ); 

						if ( param_it == params[ precise_param_index ]->end() )
						{
							_id = L""; 
						}
						else
						{
							_id = param_it->second.c_str();
						}

						param_it = params[ precise_param_index ]->find( L"tag" ); 

						if ( param_it == params[ precise_param_index ]->end() )
						{
							tag = L""; 
						}
						else
						{
							tag = param_it->second.c_str(); 
						}
					}

					if( _id.length() == 0 
						&& name.length() == 0 )
					{
						text = tag.c_str(); 

						id = text; 

						hr = element_collect->tags( id, &html_element_disp ); 
						if( S_OK != hr )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}
					}
					else
					{
						if( _id.length() )
						{
							text = _id.c_str(); 
						}
						else if( name.length() )
						{
							text = name.c_str(); 
						}
						else
						{
							ASSERT( FALSE ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						id = text; 

						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = element_collect->item( id, index, &html_element_disp ); 
						if( FAILED( hr ) 
							|| html_element_disp == NULL )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}
					}

					hr = html_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&html_sub_element ); 
					if( FAILED( hr ) 
						|| html_sub_element == NULL )
					{

						hr = html_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&element_sub_collect ); 
						if( FAILED( hr ) 
							|| element_sub_collect == NULL )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						hr = element_sub_collect->get_length( &element_count ); 
						if( FAILED( hr ) )
						{
							ret = hr; 
							break; 
						}

						if( element_count == 0 )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						for( i = 0; i < element_count; i ++ )
						{
							V_VT( &id ) = VT_I4;
							V_I4( &id ) = i;
							V_VT( &index ) = VT_I4;
							V_I4( &index ) = 0; 

							//!!!element_sub_collect not elements
							hr = element_sub_collect->item( id, index, &html_element_disp ); //Get an element

							if( hr != S_OK )
							{
								break; 
							}

							if( NULL == html_element_disp )
							{
								break; 
							}

							hr = html_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&html_sub_element ); 
							if( FAILED( hr ) 
								|| html_sub_element == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							html_sub_element->AddRef(); 
							element_group.push_back( html_sub_element ); 
						}
					}
					else
					{
						html_sub_element->AddRef(); 
						element_group.push_back( html_sub_element ); 
					}

					if( precise_param_index + 1 >= ( LONG )params_count )
					{
						HTML_ELEMENT_GROUP_ELEMENT_ITERATOR group_it; 

						for( ; ; )
						{
							group_it = element_group.begin(); 
							
							if( group_it == element_group.end() )
							{
								break; 
							}

							ret = add_html_element_to_group( ( *group_it ), &group ); 
							if( ret != ERROR_SUCCESS )
							{
								element_group.erase( group_it ); 
								( *group_it )->Release(); 
							}
							else
							{
								element_group.erase( group_it ); 
							}
						}
						break; 
					}

					ret = locate_html_element_from_xpath( &element_group, 
						&params[ precise_param_index + 1 ], 
						params_count - ( precise_param_index + 1 ), 
						&group, 
						analyze_xpath ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ret = release_html_element_group( &(*it)->elements ); 
					if( ret != ERROR_SUCCESS )
					{
						//break; 
					}

					ret = copy_html_element_group( &group, &( *it )->elements ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

				} while ( FALSE ); 

				if( NULL != params )
				{
					ULONG xpath_index; 
					for( xpath_index = 0; xpath_index < params_count; xpath_index ++ )
					{
						ASSERT( params[ xpath_index ] != NULL ); 

						delete params[ xpath_index ]; 
					}

					free( params ); 
					params = NULL; 
				}
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}

#if 0 
		/**********************************************************************
		item  
		Retrieves a form object or an object from an elements collection.

		tags  
		Retrieves a collection of objects that have the specified HTML tag name. 
		**********************************************************************/

		hr = html_doc->get_all( &element_collect ); 

		if( hr != S_OK 
			|| element_collect == NULL ) 
		{
			html_doc->Release();
			break; 
		}

		hr = element_collect->get_length( &element_count );//Find the count of the elements

		if ( hr != S_OK )
		{
			html_doc->Release();
			element_collect->Release();
			break; 
		}

		/*****************************************************************
		高速定位元素方法：
		1.使用name,id进行定位
		2.使用全路径进行定位
		3.两者合一
		xpath="+//*[@id="quick_reply"]" 
		*****************************************************************/

		for( i = 0; i < element_count; i++ ) //Loop through each element
		{
			V_VT( &id ) = VT_I4;
			V_I4( &id ) = i;
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			do 
			{
				hr = element_collect->item( id, index, &html_element_disp ); //Get an element

				if( hr != S_OK )
				{
					break; 
				}

				if( NULL == html_element_disp )
				{
					break; 
				}

				//Ask for an HTMLElemnt interface
				hr = html_element_disp->QueryInterface( IID_IHTMLElement,( void** )&html_element );
				if( hr != S_OK )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				////html_element->get_innerText( )
				//hr = html_element->get_tagName( html_element_tag.GetAddress() ); 
				//if( FAILED( hr ) )
				//{
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				//tag_name = html_element_tag.operator wchar_t*(); 
				//if( NULL == tag_name )
				//{
				//	break; 
				//}

				//dbg_print( MSG_IMPORTANT, "%ws\n", tag_name ); 

				//{
				//	HTML_ELEMENT_TYPE info_type; 
				//	HTML_ELEMENT_ACTION_TYPE action_type; 

				//	info_type = get_info_type( tag_name ); 
				//	action_type = get_html_element_action_type( info_type ); 
				//	if( action_type != type_index )
				//	{
				//		break; 
				//	}
				//}

				ret = add_html_element_to_group( html_element, 
					groups ); 

				if( ERROR_SUCCESS != ret )
				{
					//break; 
				}
			}while( FALSE ); 
		}

#endif //0 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI ie_auto_execute_on_page( HTML_PAGE_INFO *page, IDispatch *html_doc_disp, IWebBrowser2 *ie_window )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr;	
	//INT32 i; 
	//INT32 j; 

	//HTML_ELEMENT_GROUP group; 
	_variant_t id, index;
	_bstr_t html_response_statue;

	IWebBrowser2Ptr ie_browser; 
	IDispatchPtr _html_doc_disp;
	IHTMLElementPtr html_element; 
	IHTMLDocument2Ptr html_doc; 
	//IHTMLElementCollectionPtr element_collect = NULL; //Enumerate the HTML elements 
	//IDispatchPtr html_element_disp; 

	//LONG element_count; 

	VARIANT_BOOL pBool = FALSE; 
	BOOLEAN page_load_ready = FALSE; 

	do 
	{
		ASSERT( NULL != page ); 
		ASSERT( NULL != html_doc_disp ); 
		ASSERT( ie_window != NULL ); 

		_html_doc_disp = html_doc_disp; 
		if( page->elements.size() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = _html_doc_disp->QueryInterface( IID_IHTMLDocument2,
			(void**)&html_doc ); //Ask for an HTMLDocument2 interface

		if( FAILED( hr ) )
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		//IEnumerable<IHTMLElement> ;
		//IHTMLElement 

		ret = get_target_html_elements( page, html_doc, FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			//break; 
		}

		for( HTML_ELEMENT_INFO_ITERATOR it = page->elements.begin(); it != page->elements.end(); it ++ ) //Loop through each element
			{
				do 
				{
					ret = ie_auto_execute_on_element( ( *it ) ); 

					if( ERROR_SUCCESS != ret )
					{

					}
				}while( FALSE ); 
			
				if( ERROR_GOTO_NEXT_WEB_PAGE == ret )
				{
					break; 
				}
			}

			if( ERROR_GOTO_NEXT_WEB_PAGE == ret )
			{
				//if( ie_window == NULL )
				{
					ret = ie_load_web_page( ie_window, NULL ); 
					if( ret != ERROR_SUCCESS )
					{
						
					}

					hr = ie_window->get_Document( &html_doc_disp ); //Get the underlying document object of the browser

					if( html_doc_disp == NULL )
					{
						break; 
					}

					hr = _html_doc_disp->QueryInterface( IID_IHTMLDocument2,
						(void**)&html_doc ); //Ask for an HTMLDocument2 interface

					if( FAILED( hr ) 
						|| NULL == html_doc )
					{
						break; 
					}
				}
				//break; 
			}

			if( NULL == html_doc )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		release_html_element_groups( group ); 

	}while( FALSE ); 

	return ret; 
}



void EnumIE(void)
{
	LRESULT ret = ERROR_SUCCESS; 
	CString sURL;	//sURL为要注册的网址，如http://www.Ice.com
	BOOLEAN RegOrComment = 0; 

	do 
	{
	if(!RegOrComment)	sURL=TEXT("file://D:/冰网会员注册.htm");
	else				sURL=TEXT("file://D:/冰网门户.htm");	
	/*本地的网址有两种方式
		1】file://D:/冰网会员注册.htm			
		2】http://localhost:1909/Default.aspx			因为我做的是本地测试，所有用这个，这个端口是由VS2008调试时的网址得到的
			调试结果是哪个网址就填哪个网址，	网上找到的那些配置IIS的，都默认是80端口，也就是以http://localhost:80/Default.aspx为网址
			很明显我这里就打不开
	*/


	CComQIPtr<IWebBrowser2>spBrowser;
	HRESULT	hr = CoCreateInstance (CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID *)&spBrowser);	//打开浏览器
	if(hr!=S_OK)		break; 

	COleVariant vaURL(sURL) ;	
	COleVariant null;	

	VARIANT_BOOL pBool=TRUE;
	spBrowser->put_Visible( pBool );//TRUE显示浏览器，FALSE隐藏浏览器
	spBrowser->Navigate2(vaURL,null,null,null,null) ; //Open the URL page,打开网页


	Sleep(2000);//等待网页加载完毕
	if(!RegOrComment)	sURL=TEXT("file:///D:/冰网会员注册.htm");	//注意：这里的file后面有三个斜杠。	如果是http开头的，则不需要这个if语句
	else				sURL=TEXT("file:///D:/冰网门户.htm");	

	CComPtr<IShellWindows> spShellWin;   
	 hr=spShellWin.CoCreateInstance(CLSID_ShellWindows);   
	if (FAILED(hr))   		break;    

	long nCount=0;    //取得浏览器实例个数(Explorer和IExplorer)   
	spShellWin->get_Count(&nCount); 
	if (0==nCount)   		break;  

	for(int i=0; i<nCount; i++)   
	{  
		CComPtr<IDispatch> spDispIE;   
		hr=spShellWin->Item(CComVariant((long)i), &spDispIE);   
		if (FAILED(hr)) continue; 

		spBrowser=spDispIE; 
		if (!spBrowser) continue; 

		IDispatch*spDispDoc;   
		hr=spBrowser->get_Document(&spDispDoc);   
		if (FAILED(hr)) continue; 

		CComQIPtr<IHTMLDocument2>spDocument2 =spDispDoc;   
		if (!spDocument2) continue;       

		CString cIEUrl_Filter;  //设置URL(必须是此URL的网站才有效);
		cIEUrl_Filter=sURL; 
	
		CComBSTR IEUrl;
		spBrowser->get_LocationURL(&IEUrl);    
		CString cIEUrl_Get;     //从机器上取得的HTTP的完整的URL;
		cIEUrl_Get=IEUrl;
		cIEUrl_Get=cIEUrl_Get.Left(cIEUrl_Filter.GetLength()); //截取前面N位

//MessageBox(cIEUrl_Get+TEXT("2")+cIEUrl_Filter); 

		if (cIEUrl_Get==cIEUrl_Filter)
		{
			// 程序运行到此，已经找到了IHTMLDocument2的接口指针   

//	CComPtr<IHTMLFormElement> pForm;
//	if (SUCCEEDED(spBrowser->get_Document( &spDispDoc)))	spDocument2 = spDispDoc;
//	if(SUCCEEDED(spDispDoc->QueryInterface(IID_IHTMLFormElement,(void**)&pForm)))
//	if(SUCCEEDED(pFormElement->item(id,index, &spDispDoc)))
//	spDispDoc->QueryInterface(IID_IHTMLInputTextElement,(void**)&pInputElement);
										
			EnumForm(spDocument2,spDispDoc); //枚举所有的表单 
		}  				
	}

	}while( FALSE );

	return;  
}


void EnumFrame(IHTMLDocument2 * pIHTMLDocument2)
{
	if (!pIHTMLDocument2) return;       
	HRESULT   hr;   

	CComPtr<IHTMLFramesCollection2> spFramesCollection2;   
	pIHTMLDocument2->get_frames(&spFramesCollection2); //取得框架frame的集合   

	long nFrameCount=0;        //取得子框架个数   
	hr=spFramesCollection2->get_length(&nFrameCount);   
	if (FAILED(hr)|| 0==nFrameCount) return;   

	for(long i=0; i<nFrameCount; i++)   
	{   
		CComVariant vDispWin2; //取得子框架的自动化接口   
		hr = spFramesCollection2->item(&CComVariant(i), &vDispWin2);   
		if (FAILED(hr)) continue;       
		CComQIPtr<IHTMLWindow2>spWin2 = vDispWin2.pdispVal;   
		if (!spWin2) continue; //取得子框架的   IHTMLWindow2   接口       
		CComPtr <IHTMLDocument2> spDoc2;   
		spWin2->get_document(&spDoc2); //取得子框架的   IHTMLDocument2   接口

		EnumForm(spDoc2,spDoc2);      //递归枚举当前子框架   IHTMLDocument2   上的表单form   
	}   
}

void EnumForm(IHTMLDocument2 * pIHTMLDocument2,IDispatch*spDispDoc)
{
	CString sDbg;

	if (!pIHTMLDocument2) return; 
	EnumFrame(pIHTMLDocument2);   //递归枚举当前IHTMLDocument2上的子框架frame   

	HRESULT hr;
	USES_CONVERSION;       

	CComQIPtr<IHTMLElementCollection> spElementCollection;   
	hr=pIHTMLDocument2->get_forms(&spElementCollection); //取得表单集合   
	if (FAILED(hr))   	return;   
 
	long nFormCount=0;           //取得表单数目   
	hr=spElementCollection->get_length(&nFormCount);   
	if (FAILED(hr))   	return;   

	for(long i=0; i<nFormCount; i++)   
	{   
		IDispatch *pDisp = NULL;   //取得第i项表单   
		hr=spElementCollection->item(CComVariant(i),CComVariant(),&pDisp);   
		if(FAILED(hr)) continue;   

		CComQIPtr<IHTMLFormElement> spFormElement= pDisp;   
		pDisp->Release();   

		long nElemCount=0;         //取得表单中域的数目   
		hr=spFormElement->get_length(&nElemCount);   
		if(FAILED(hr)) continue;   

		for(long j=0; j<nElemCount; j++)   
		{  			
			CComDispatchDriver spInputElement; //取得第j项表单域   
			hr=spFormElement->item(CComVariant(j), CComVariant(), &spInputElement);   
			if(FAILED(hr)) continue;   

			CComVariant vName,vVal,vType;     //取得表单域的名称，数值，类型 
			hr=spInputElement.GetPropertyByName(TEXT("name"), &vName);   
			if(FAILED(hr)) continue;   
			hr=spInputElement.GetPropertyByName(TEXT("value"), &vVal);   
			if(FAILED(hr)) continue;   
			hr=spInputElement.GetPropertyByName(TEXT("type"), &vType);   
			if(FAILED(hr)) continue;   

			LPCTSTR lpName= vName.bstrVal ? OLE2CT(vName.bstrVal) : TEXT(""); //未知域名   
			LPCTSTR lpVal=  vVal.bstrVal  ? OLE2CT(vVal.bstrVal)  : TEXT(""); //空值，未输入   
			LPCTSTR lpType= vType.bstrVal ? OLE2CT(vType.bstrVal) : TEXT(""); //未知类型  

		//	sDbg.Format(TEXT("%s,%s,%s"),lpType,lpVal,lpName);//通过弹出对话框，可以获得每一个控件的三项指标，可以根据需要选择一个指标
		//	MessageBox(sDbg);

			EnumField(spInputElement,lpType,lpVal,lpName);					  //传递并处理表单域的类型、值、名


			if((CString)lpVal==TEXT("提交"))			break;
			if((CString)lpVal==TEXT("发表评论"))		break;

		}//表单域循环结束     
	}//表单循环结束      
}

void EnumField(CComDispatchDriver spInputElement , CString ComType , CString ComVal , CString ComName)
{
	CString csTemp;
	bool	IsWant=TRUE;//是否想要填的表单？
/*********************************************注册冰网会员*********************************************/		
	if		   (	ComName=="TextBox1"	)								csTemp=TEXT("冰灵bing魂");				//用户名
	else	if (	ComName=="TextBox2"||	ComName=="TextBox3"	)		csTemp=TEXT("zc2011");					//密码	
	else	if (	ComName=="TextBox4"	)								csTemp=TEXT("2011年2月10日 星期四");	//"出生日期"
	else	if (	ComName=="TextBox5"	)								csTemp=TEXT("lxz@qq.com");				//"Email:"
	else	if (	ComName=="DropDownList1"	)						csTemp=TEXT("计算机科学与技术");		//"专业:"
	else	if (	ComName=="TextBox7"	)								csTemp=TEXT("冰灵bing魂   1662888517@qq.com");				//ComVal=="个人简介"
	else	IsWant=FALSE;

	if(IsWant)//如果是想要填的表单，则填写
	{
		CComVariant vSetStatus(csTemp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}

	if(ComVal=="提交")
	{
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		::MessageBox( NULL, TEXT("可以看看表单是否已经填上啦"), NULL, 0 );
		pHElement->click();   
	}
/*********************************************在冰网发帖并留言*********************************************/		
	//lpName	TextBox_Daily	Button_Submit0	TextBox_Comment		Button_Submit
	//lpVal		在这里写下你的心历路程吧......		发表日志	''	发表评论

	IsWant=TRUE;
	if			(	ComName=="TextBox_Daily")			csTemp=TEXT("Hello!	快乐兔有(*^__^*)");			//写日志		
	else	if  (	ComName=="TextBox_Comment")			csTemp=TEXT("Thanks!	开心兔有(*^__^*)");		//写留言
	else	IsWant=FALSE;
		
	if(IsWant)//如果是想要填的表单，则填写
	{
		CComVariant vSetStatus(csTemp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}

	if(ComName=="Button_Submit0")	//发表日志
	{ 
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		::MessageBox( NULL, TEXT("可以看看表单是否已经填上啦"), NULL, 0 );

		//MessageBox(TEXT("可以看看日志是否已经写上啦"));
	//	pHElement->click();		//如果需要发表日志，则取消这行注释，
								//并在EnumForm()的		【if((CString)lpVal==TEXT("发表评论"))break;】这一句后面加上【if((CString)lpVal==TEXT("发表日志"))break;】

	}
	else	if (ComName=="Button_Submit")	//发表评论
	{ 
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		//MessageBox(TEXT("可以看看留言是否已经填上啦"));
		pHElement->click();  
	}	

/*********************************************填写QQ网页*********************************************/		
/*	以下代码基本上是依照下载过来的代码格式写的，冗余度很高，就不改了，等我弄懂了以后发布简单点的源码
作者的奉献精神是很值得表扬和学习的，	整个百度和谷歌上，能用且有注释的也就两三个，作者就是之一啊
网址不记得了，我只保存了本地网页，可以打开看看。作者博客的名字为【fjssharpsword的专栏】，再次感谢作者的无私奉献哦


	//邮箱账号
	if ((ComType.Find(TEXT("text"))>=0)  &&  ComVal=="请填写一个您想要申请的帐号" && ComName==TEXT(""))
	{
		Tmp=TEXT("qq");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}
	//1代表@qq.com,3代表@foxmail.com
	else	if ((ComType.Find(TEXT("select-one"))>=0)  &&ComName==TEXT("emailselect"))
	{ 
		Tmp=TEXT("3");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}
	//检测账号是否可用
	else	if ((ComType.Find(TEXT("button"))>=0)   &&ComName==TEXT(""))
	{ 
		//button的名字改为123456
		Tmp=TEXT("123456");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
		//点击按钮
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		pHElement->click();  
	}	
	//昵称
	else	if ((ComType.Find(TEXT("text"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{
		Tmp=TEXT("昵称");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}
	//年
	else	if ((ComType.Find(TEXT("select-one"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("2011");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//月
	else	if ((ComType.Find(TEXT("select-one"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("1");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}
	//日
	else	if ((ComType.Find(TEXT("select-one"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("1");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//ComVal==TEXT("1")1代表男，2代表女
	else	if ((ComType.Find(TEXT("radio"))>=0)  &&ComVal==TEXT("2")&& ComName==TEXT("gender"))
	{
		//点击选择
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		pHElement->click();  
	}	
	//password
	else	if ((ComType.Find(TEXT("password"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("password");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//repassword
	else	if ((ComType.Find(TEXT("password"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("password");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//省
	else	if ((ComType.Find(TEXT("select-one"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("省");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//市
	else	if ((ComType.Find(TEXT("select-one"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("市");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//县
	else	if ((ComType.Find(TEXT("县"))>=0) &&  ComVal==TEXT("")  &&ComName==TEXT(""))
	{ 
		Tmp=TEXT("password");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	
	//验证码
	else	if ((ComType.Find(TEXT("text"))>=0) &&  ComVal==TEXT("请输入您在下图中看到的字符，不区分大小写")  &&ComName==TEXT("verifycode"))
	{ 
		Tmp=TEXT("cfsb");
		CComVariant vSetStatus(Tmp);
		spInputElement.PutPropertyByName(TEXT("value"),&vSetStatus);
	}	

	else	if(ComType.Find(TEXT("submit"))!=-1)
	{
//		MessageBox(TEXT("SUBMIT:"));
		IHTMLElement*  pHElement;
		spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pHElement);
		pHElement->click();                
	}*/
/*


IHTMLElement* pElem;
//获得常规网页元素HTMLElement接口
hr = spInputElement->QueryInterface(IID_IHTMLElement,(void **)&pElem);
//试图获得超链接元素接口

IHTMLAnchorElement* pSubAnchor;
hr = spInputElement->QueryInterface(IID_IHTMLAnchorElement, (void **)&pSubAnchor);
if(hr == S_OK) {
BSTR bstr;
pElem->get_id(&bstr);
CString strID;
strID = bstr;
if(strID == "a_submit"){
MessageBox(TEXT("可以看看信息是否已经填上啦"));
pElem->click();
}
pSubAnchor->Release();
}


MessageBox(ComName);
IHTMLFormElement*pForm; 


	spInputElement->QueryInterface(IID_IHTMLFormElement,(void**)&pForm);
	pForm->submit();

MessageBox(ComVal);
*/ 

//MessageBox(ComName);
//MessageBox(ComVal);
//MessageBox(ComType);

}
