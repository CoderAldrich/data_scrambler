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
//#import "mshtml.tlb"
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "input_content.h"
#include "external_link.h"
#include "html_event_handler.h"
#include "ie_cache.h"


HTML_LIST_ELEMENT_SCRAMBLE_INFOS scramble_infos; 

//#import <msxml3.dll>
//using namespace MSXML2;
//
//void dump_com_error(_com_error &e);
//
//int main(int argc, char* argv[])
//{
//	CoInitialize(NULL);
//	try{
//		IXMLDOMDocument2Ptr pXMLDoc = NULL;
//		HRESULT hr = pXMLDoc.CreateInstance(__uuidof(DOMDocument30));
//
//		// Set parser property settings
//		pXMLDoc->async =  VARIANT_FALSE;
//
//		// Load the sample XML file
//		hr = pXMLDoc->load("hello.xsl");
//
//		// If document does not load report the parse error 
//		if(hr!=VARIANT_TRUE)
//		{
//			IXMLDOMParseErrorPtr  pError;
//			pError = pXMLDoc->parseError;
//			_bstr_t parseError =_bstr_t("At line ")+ _bstr_t(pError->Getline())
//				+ _bstr_t("\n")+ _bstr_t(pError->Getreason());
//			MessageBox(NULL,parseError, "Parse Error",MB_OK);
//			return 0;
//		}
//		// Otherwise, build node list using SelectNodes 
//		// and returns its length as console output
//		else
//			pXMLDoc->setProperty("SelectionLanguage", "XPath");
//		// Set the selection namespace URI if the nodes
//		// you wish to select later use a namespace prefix
//		pXMLDoc->setProperty("SelectionNamespaces",
//			"xmlns:xsl='http://www.w3.org/1999/XSL/Transform'");
//		IXMLDOMElementPtr pXMLDocElement = NULL;
//		pXMLDocElement = pXMLDoc->documentElement;
//		IXMLDOMNodeListPtr pXMLDomNodeList = NULL;
//		pXMLDomNodeList = pXMLDocElement->selectNodes("//xsl:template");
//		int count = 0;
//		count = pXMLDomNodeList->length;
//		printf("The number of <xsl:template> nodes is %i.\n", count);
//	}
//	catch(_com_error &e)
//	{
//		dump_com_error(e);
//	}
//	return 0;
//}
//
//void dump_com_error(_com_error &e)
//{
//	printf("Error\n");
//	printf("\a\tCode = %08lx\n", e.Error());
//	printf("\a\tCode meaning = %s", e.ErrorMessage());
//	_bstr_t bstrSource(e.Source());
//	_bstr_t bstrDescription(e.Description());
//	printf("\a\tSource = %s\n", (LPCSTR) bstrSource);
//	printf("\a\tDescription = %s\n", (LPCSTR) bstrDescription);
//}

#include "ie_cache.h"

HRESULT WINAPI is_effective_html_element( IHTMLElement *html_element, BOOLEAN *is_valid )
{
	HRESULT hr = S_OK; 
	_bstr_t text; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != is_valid ); 

		*is_valid = TRUE; 

		hr = html_element->get_tagName( text.GetAddress() ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( 0 == text.length() )
		{
			hr = E_INVALIDARG; 
			break; 
		}

		_text = text.operator wchar_t*(); 
		if( NULL == _text )
		{
			hr = E_INVALIDARG; 
			break; 
		}

		if( 0 == _wcsicmp( L"DIV", _text ) )
		{
			*is_valid = FALSE; 
		}

	}while( FALSE );

	return hr; 
}

LRESULT CALLBACK on_cached_data_readed( PVOID data, ULONG data_size, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	HANDLE file_handle; 
	ULONG wrote_size; 

	do 
	{
		file_handle = ( HANDLE )context; 

		if( INVALID_HANDLE_VALUE == file_handle )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_ret = WriteFile( file_handle, 
			data, 
			data_size, 
			&wrote_size, 
			NULL ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( wrote_size != data_size )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
			break; 
		}
	} while ( FALSE );

	return ret; 
}

#include "md5.h"


LRESULT WINAPI save_list_scrambled( LPCWSTR file_name, 
								   vector< wstring > &url_list )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
	ULONG data_size; 
	ULONG wrote; 
	LARGE_INTEGER current_pos; 
	LARGE_INTEGER old_pos; 
	wstring text; 
	LPCWSTR data_block; 
	BOOLEAN create_new = FALSE; 

	do 
	{
        ret = create_directory_ex((LPWSTR)file_name, wcslen(file_name), 2);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

		ret = check_file_exist( file_name ); 

		if( ret != ERROR_SUCCESS )
		{
			create_new = TRUE; 
		}

		file_handle = CreateFileW( file_name, 
			FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_ALWAYS, 
			0, 
			NULL ); 

		if( INVALID_HANDLE_VALUE == file_handle ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( create_new == TRUE )
		{
			INT32 _ret; 
			ULONG data_size; 
			ULONG wrote; 
			LARGE_INTEGER current_pos; 
			LARGE_INTEGER old_pos; 
			BOOLEAN create_new = FALSE; 
			BYTE unicode_fmt_sign[] = { 0xff, 0xfe }; 

			data_size = sizeof( unicode_fmt_sign ); 

			current_pos.QuadPart = 0; 

			_ret = SetFilePointerEx( file_handle, 
				current_pos, 
				&old_pos, 
				SEEK_SET); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				unicode_fmt_sign, 
				data_size, 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		}

		for( vector<wstring>::iterator it = url_list.begin(); 
			it != url_list.end(); 
			it ++ )
		{
			do 
			{
				text = ( *it ); 
				data_size = ( ULONG )text.length() * sizeof( WCHAR ); 
				data_block = text.c_str(); 
				current_pos.QuadPart = 0; 

				_ret = SetFilePointerEx( file_handle, 
					current_pos, 
					&old_pos, 
					SEEK_END ); 

				if( FALSE == _ret )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				_ret = WriteFile( file_handle, 
					data_block, 
					data_size, 
					&wrote, 
					NULL ); 

				if( FALSE == _ret )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				_ret = WriteFile( file_handle, 
					L"\r\n", 
					sizeof( L"\r\n" ) - sizeof( WCHAR ), 
					&wrote, 
					NULL ); 

				if( FALSE == _ret )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}
			} while ( FALSE );
		}
	} while ( FALSE );

	if( INVALID_HANDLE_VALUE != file_handle )
	{
		CloseHandle( file_handle ); 
	}

	return ret; 
}

LRESULT WINAPI get_image_file_name_url( LPCWSTR url, 
									   LPWSTR file_name, 
									   ULONG cc_buf_len, 
									   ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _cc_ret_len; 
#define MD5_DIGEST_LENGTH 16
	UCHAR md5[ MD5_DIGEST_LENGTH ]; 
	WCHAR _md5_text[ MD5_DIGEST_LENGTH * 2 + 1 ]; 

	do 
	{
		ASSERT( NULL != url ); 

		//ret = get_file_name_from_url( url, file_name ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		if( NULL != cc_ret_len )
		{
			*cc_ret_len = 0; 
		}

		{
			context_md5_t ctx; 
			MD5Init( &ctx ); 
			MD5Update( &ctx, ( BYTE* )url, wcslen( url ) << 1 ); 
			MD5Final( md5, &ctx ); 
		}
		
		ret = hex2text( md5, MD5_DIGEST_LENGTH, _md5_text, ARRAYSIZE( _md5_text ) ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = get_app_path( file_name, cc_buf_len, &_cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( file_name, cc_buf_len, _md5_text ); 
		if( FAILED(hr) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( file_name, cc_buf_len, L".img" ); 
		if( FAILED(hr) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL != cc_ret_len )
		{
			*cc_ret_len = wcslen( file_name ); 
		}

	}while( FALSE );

	return ret; 
}

#define IMAGE_FILE_PATH L"images" 

LRESULT WINAPI get_file_count(LPCWSTR directory, ULONG *file_count)
{
    LRESULT ret = ERROR_SUCCESS;
    BOOL _ret;

    CString file_name_pattern; 
    HANDLE find_file_handle;
    WIN32_FIND_DATA file_data;
    INT32 _file_count = 0; 

    do
    {
        file_name_pattern = directory; 
        file_name_pattern += L"/*.*"; 

        find_file_handle = FindFirstFile(file_name_pattern.GetBuffer(), &file_data); 
                                                            
        while( find_file_handle != INVALID_HANDLE_VALUE )
        {
            if (file_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)//判断是否是文件 
            {
                _file_count++;
            }

            _ret = FindNextFile(find_file_handle, &file_data);
        
            if (_ret == FALSE)
            {
                break;
            }
        }
        
        FindClose(find_file_handle); 
        *file_count = _file_count; 
    } while (FALSE);
    
    return ret;
}

LRESULT WINAPI save_ie_cache_file( LPCWSTR url, LPCWSTR file_name_pattern)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	//CString file_name; 
	ULONG cc_ret_len; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
    WCHAR image_file_path[MAX_PATH]; 
	WCHAR file_name[ MAX_PATH ]; 
    ULONG file_count; 

	do 
	{
		//Sleep( 6000 ); 
		ret = get_image_file_name_url( url, file_name, ARRAYSIZE( file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			//break; 
		}

        ret = get_app_path(image_file_path, ARRAYSIZE(image_file_path), &cc_ret_len); 
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        hr = StringCchPrintfW(file_name, ARRAYSIZE(file_name), L"%s%s\\%s", image_file_path, IMAGE_FILE_PATH, file_name_pattern);
        if (FAILED(hr))
        {
            ret = ERROR_ERRORS_ENCOUNTERED; 
            break; 
        }

        ret = get_file_count(image_file_path, &file_count); 
        if (ret != ERROR_ERRORS_ENCOUNTERED)
        {
            break;
        }

        hr = StringCchPrintfW(file_name, ARRAYSIZE(file_name), L"%s\\%s%u", image_file_path, file_name_pattern, file_count );
        if (FAILED(hr))
        {
            ret = ERROR_ERRORS_ENCOUNTERED; 
            break;
        }

        file_handle = CreateFile( file_name, 
			FILE_GENERIC_WRITE, 
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL ); 

		if( INVALID_HANDLE_VALUE == file_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		ret = get_ie_cache_data( url, on_cached_data_readed, file_handle ); 
	} while ( FALSE );

	if( INVALID_HANDLE_VALUE != file_handle )
	{
		CloseHandle( file_handle ); 
	}

	if( ret != ERROR_SUCCESS )
	{
		DeleteFileW( file_name ); 
	}
	return ret; 
}

HRESULT WINAPI get_html_element_image_file( IHTMLElement *html_element, wstring &image_url, LPCWSTR file_name_pattern)
{
	HRESULT hr = S_OK; 
	LRESULT ret; 
	_bstr_t text; 
	LPCWSTR _text; 
	_variant_t value; 

	do 
	{
		ASSERT( NULL != html_element ); 

		hr = html_element->get_tagName( text.GetAddress() ); 
		if( FAILED(hr) )
		{
			break; 
		}

		if( text.length() == 0 )
		{
			hr = E_INVALIDARG; 
			break; 
		}

		text = L"src"; 
		hr = html_element->getAttribute( text.GetBSTR(), 2, value.GetAddress() ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( value.vt != VT_BSTR )
		{
			hr = E_INVALIDARG; 
			break; 
		}

		text = value.bstrVal; 
		if( 0 == text.length() )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		_text = text.operator wchar_t*(); 
		if( NULL == _text )
		{
			ASSERT( FALSE ); 
			break; 
		}

		image_url = _text; 

		ret = save_ie_cache_file( _text, file_name_pattern);
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return hr; 
}

HRESULT WINAPI get_html_element_image_files( IHTMLElement *html_element, vector<wstring> &image_urls, LPCWSTR file_name_pattern)
{
	HRESULT hr = S_OK; 
	HTML_ELEMENT_GROUP group; 
	HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 
	wstring image_url; 

	do 
	{
		ASSERT( NULL != html_element ); 

		hr = get_children_image_element( html_element, &group ); 
		if( FAILED(hr))
		{
			break; 
		}

		image_urls.clear(); 

		for( it = group.begin(); it != group.end(); it ++ )
		{
			do 
			{
				try
				{
					hr = get_html_element_image_file( ( *it ), image_url, file_name_pattern);
					if( FAILED(hr))
					{
						break; 
					}
				}				
				catch (...)
				{
					break; 
				}

				try
				{
					image_urls.push_back( image_url ); 
				}
				catch(...)
				{
					break; 
				}
			}while( FALSE );
		}
	} while ( FALSE ); 

	release_html_element_group( &group ); 

	return hr; 
}

/***************************************************************************
HTML爬虫第1版本：
1.打开一个网页，抓取网页里面的所有的链接。
2.将其中需要的链接打开。
3.将打开后的网页的正确的需要内容抓取出来，保存原始数据。
4.以某种形式进行循环。
5.将所有的数据以特定的格式保存下来。

***************************************************************************/
HRESULT WINAPI scramble_web_page_content_work(IHTMLDocument2 *page, 
								  HTML_PAGE_CONTENT *contents )
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	LRESULT ret; 
	IHTMLElementCollectionPtr element_collection; 
	IDispatchPtr element_disp; 
	IHTMLElementPtr element; 
	HTML_ELEMENT_GROUP group; 
	//LONG count; 
	_bstr_t text; 
	LPCWSTR _text; 

	ASSERT( nullptr != page ); 
	ASSERT( nullptr != contents ); 

	do 
	{
		//for( HTML_PAGE_CONTENTS_ITERATOR it = contents->begin(); it != contents->end(); it ++ )
		{
			for( HTML_ELEMENT_CONTENTS_ITERATOR element_it = contents->element_contents.begin(); 
				element_it != contents->element_contents.end(); 
				element_it ++ )
			{
				( *element_it )->text.clear(); 
				( *element_it )->additions.clear(); 

				ret = get_html_elements_from_xpath( ( *element_it )->xpath.c_str(), 
					page, 
					&group, 
					FALSE ); 
				
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( 0 == group.size() ); 
					break; 
				}

				if( group.size() > 1 )
				{
					dbg_print( MSG_IMPORTANT, "the text of the element of the content is too more %u %ws\n", 
						group.size(), 
						( *element_it )->xpath.c_str() ); 
				}

				//有一部分的ELEMENT是无法进行精确的定位的，因为没有ID，CLASS，NAME，TEXT标记

				for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); it != group.end(); it ++ )
				{
					do 
					{
						//hr = ( *it )->get_innerHTML( text.GetAddress() ); 
						_hr = ( *it )->get_innerText( text.GetAddress() ); 

						if( FAILED(_hr))
						{
							break; 
						}

						//for( ; ; )
						//{
						//	Sleep( 10 ); 
						//}

#ifdef _DOWNLOAD_IMAGE_IN_WEB_PAGE
						hr = get_html_element_image_files( (*it), ( *element_it )->additions ); 
						if( FAILED(hr))
						{
							break; 
						}
#endif //_DOWNLOAD_IMAGE_IN_WEB_PAGE
						//if( text.length() == 0 )
						//{
						//	break; 
						//}

						_text = text.operator const wchar_t*(); 
						if( NULL == _text )
						{
							break; 
						}

						if( it != group.begin() )
						{
							( *element_it )->text += L" "; 
						}

						( *element_it )->text += _text; 
					} while ( FALSE );
				}

				release_html_element_group( &group ); 
			}
		}

		//{
		//	ITypeInfo *type_info; 
		//	type_info->

		//	element_collection->GetTypeInfo()
		//}
	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI _scramble_web_page_content(CWebBrowser2 *browser, 
										 HTML_PAGE_CONTENT *contents )
{
	HRESULT hr = S_OK; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		html_doc = browser->GetDocument(); 

		if( NULL == html_doc )
		{
			break; 
		}

		hr = scramble_web_page_content_work( html_doc, contents ); 

	} while ( FALSE ); 

	return hr; 
}

typedef enum _WORK_TYPE
{
	WORK_SCRAMBLE_LINKS, 
	WORK_SCRAMBLE_CONTENT, 
	MAX_WORK_SCRAMBLE, 
} WORK_TYPE, *PWORK_TYPE; 

typedef struct _WORK_ITEM
{
	WORK_TYPE type; 
	PVOID contents; 
} WORK_ITEM, *PWORK_ITEM; 

HRESULT WINAPI scramble_web_page_content(CWebBrowser2 *browser, 
										 HTML_PAGE_CONTENT *page_content )
{
	HRESULT hr = S_OK; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		HTML_ELEMENT_CONTENTS_ITERATOR it = page_content->element_contents.begin(); 

		for( ; ; )
		{
			if( it == page_content->element_contents.end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 

			( *it )->text.clear(); 

			it ++; 
		}


		html_doc = browser->GetDocument(); 

		if( NULL == html_doc )
		{
			CString url; 
			url = browser->GetLocationURL(); 
			dbg_print( MSG_FATAL_ERROR, "have not loaded the web page %ws\n", url.GetBuffer() ); 
			break; 
		}

		//hr = dump_html_elements_on_page( html_doc ); 

		hr = scramble_web_page_content_work( html_doc, page_content ); 

	} while ( FALSE ); 

	return hr; 
}

HRESULT WINAPI scramble_web_page_content_ex(CWebBrowser2 *browser, 
										 LPCWSTR url )
{
	HRESULT hr = S_OK; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		hr = browser_safe_navigate_start( browser, url ); 
		if( FAILED( hr))
		{
			//break; 
		}

		start_worker( ( PVOID )1 ); 

	} while ( FALSE ); 

	return hr; 
}

/****************************************************
返回值不精确，需要完善
****************************************************/
LRESULT WINAPI get_html_elements_from_xpath( LPCWSTR xpath, 
										IHTMLDocument2 *html_doc, 
										HTML_ELEMENT_GROUP *group, 
										BOOLEAN analyze_xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementCollectionPtr element_collect = NULL; //Enumerate the HTML elements 
	IHTMLElementCollectionPtr element_sub_collect; 
	IDispatchPtr html_element_disp; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr html_sub_element; 
	//LONG sub_elements_count; 
	//BOOLEAN sub_items_found; 
	LONG i; 
	//LONG _index; 
	_bstr_t text; 
	_variant_t index; 
	_variant_t id; 
	HTML_ELEMENT_GROUP element_group; 

	do 
	{
		ASSERT( NULL != group ); 
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != xpath ); 

		if( *xpath == L'\0' )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

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

		group->clear(); 

		XPATH_PARAMS **params = NULL; 
		XPATH_PARAM_ITERATOR param_it; 
		ULONG params_count = 0; 
		LONG element_count; 
		wstring tag; 
		wstring _id; 
		wstring name; 
		LONG precise_param_index = -1; 
		BOOLEAN in_frame = FALSE; 

		do 
		{
			params = ( XPATH_PARAMS** )malloc( sizeof( XPATH_PARAMS* ) * MAX_XPATH_ELEMENT_PARAMETER_COUNT ); 
			if( NULL == params )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			ret = get_xpath_params( xpath, 
				wcslen( xpath ), 
				params, 
				MAX_XPATH_ELEMENT_PARAMETER_COUNT, 
				&params_count ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( params_count < 2 ) //HTML|BODY|DIV
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

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
							in_frame = TRUE; 
							precise_param_index = param_index; 
							break; 
						}
					}
				}

				if( precise_param_index < 0 )
				{
					precise_param_index = 1; //locate first form second element level, be not html.
				}

				if( precise_param_index >= ( LONG )params_count )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
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

				V_VT( &index ) = VT_EMPTY;
				V_I4( &index ) = -1; 

				hr = element_collect->item( id, index, &html_element_disp ); 
				if( FAILED( hr ) 
					|| html_element_disp == NULL )
				{
					LPCWSTR _text; 
					_text = text.operator wchar_t*(); 

					dbg_print( MSG_FATAL_ERROR, 
						"can't find the target html element from id(name):%ws\n", _text == NULL ? L"" : _text ); 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}

			hr = html_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&element_sub_collect ); 
			if( FAILED( hr ) 
				|| element_sub_collect == NULL )
			{
				hr = html_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&html_sub_element ); 
				if( FAILED( hr ) 
					|| html_sub_element == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
				else
				{
					do 
					{
						hr = compare_html_element_attributes_ex( params[ precise_param_index ], 
							html_sub_element ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						html_sub_element->AddRef(); 
						element_group.push_back( html_sub_element ); 
					}while( FALSE );
				}
			}
			else
			{
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

					do 
					{
						hr = compare_html_element_attributes_ex( params[ precise_param_index ], 
							html_sub_element ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						html_sub_element->AddRef(); 
						element_group.push_back( html_sub_element ); 
					}while( FALSE );
				}
			}

			if( precise_param_index + 1 >= ( LONG )params_count )
			{
				HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 

				for( ; ; )
				{
					it = element_group.begin(); 

					if( it == element_group.end() )
					{
						break; 
					}

					ret = add_html_element_to_group( ( *it ), group ); 
					if( ret != ERROR_SUCCESS )
					{
						element_group.erase( it ); 
						( *it )->Release(); 
					}
					else
					{
						element_group.erase( it ); 
					}
				}
				break; 
			}

			if( in_frame == TRUE )
			{
				ASSERT( precise_param_index >= 0 ); 
				ret = locate_html_element_from_xpath( &element_group, 
					&params[ precise_param_index ], 
					params_count - ( precise_param_index ), 
					group, 
					xpath, 
					analyze_xpath);
			}
			else
			{
				ret = locate_html_element_from_xpath( &element_group, 
					&params[ precise_param_index + 1 ], 
					params_count - ( precise_param_index + 1 ), 
					group, 
					xpath, 
					analyze_xpath ); 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		} while ( FALSE ); 

		if( NULL != params )
		{
			release_xpath_params( params, params_count ); 
			free( params ); 
			params = NULL; 
		}

	}while( FALSE );

	return ret; 
}

struct __declspec(uuid("305106e0-98b5-11cf-bb82-00aa00bdce0b"))
	/* dual interface */ IHTMLDOMNode3;

struct __declspec(uuid("305106e0-98b5-11cf-bb82-00aa00bdce0b"))
IHTMLDOMNode3 : IDispatch
{
	//
	// Property data
	//

	__declspec(property(get=Getprefix,put=Putprefix))
		_variant_t prefix;
	__declspec(property(get=GettextContent,put=PuttextContent))
		_variant_t textContent;
	__declspec(property(get=GetlocalName))
		_variant_t localName;
	__declspec(property(get=GetnamespaceURI))
		_variant_t namespaceURI;

	//
	// Wrapper methods for error-handling
	//

	void Putprefix (
		const _variant_t & p );
	_variant_t Getprefix ( );
	_variant_t GetlocalName ( );
	_variant_t GetnamespaceURI ( );
	void PuttextContent (
		const _variant_t & p );
	_variant_t GettextContent ( );
	VARIANT_BOOL isEqualNode (
	struct IHTMLDOMNode3 * otherNode );
	_variant_t lookupNamespaceURI (
		VARIANT * pvarPrefix );
	_variant_t lookupPrefix (
		VARIANT * pvarNamespaceURI );
	VARIANT_BOOL isDefaultNamespace (
		VARIANT * pvarNamespace );
	IHTMLDOMNodePtr appendChild (
	struct IHTMLDOMNode * newChild );
	IHTMLDOMNodePtr insertBefore (
	struct IHTMLDOMNode * newChild,
		const _variant_t & refChild = vtMissing );
	IHTMLDOMNodePtr removeChild (
	struct IHTMLDOMNode * oldChild );
	IHTMLDOMNodePtr replaceChild (
	struct IHTMLDOMNode * newChild,
	struct IHTMLDOMNode * oldChild );
	VARIANT_BOOL isSameNode (
	struct IHTMLDOMNode3 * otherNode );
	unsigned short compareDocumentPosition (
	struct IHTMLDOMNode * otherNode );
	VARIANT_BOOL isSupported (
		_bstr_t feature,
		const _variant_t & version );

	//
	// Raw methods provided by interface
	//

	virtual HRESULT __stdcall put_prefix (
		/*[in]*/ VARIANT p ) = 0;
	virtual HRESULT __stdcall get_prefix (
		/*[out,retval]*/ VARIANT * p ) = 0;
	virtual HRESULT __stdcall get_localName (
		/*[out,retval]*/ VARIANT * p ) = 0;
	virtual HRESULT __stdcall get_namespaceURI (
		/*[out,retval]*/ VARIANT * p ) = 0;
	virtual HRESULT __stdcall put_textContent (
		/*[in]*/ VARIANT p ) = 0;
	virtual HRESULT __stdcall get_textContent (
		/*[out,retval]*/ VARIANT * p ) = 0;
	virtual HRESULT __stdcall raw_isEqualNode (
	/*[in]*/ struct IHTMLDOMNode3 * otherNode,
		/*[out,retval]*/ VARIANT_BOOL * isEqual ) = 0;
	virtual HRESULT __stdcall raw_lookupNamespaceURI (
		/*[in]*/ VARIANT * pvarPrefix,
		/*[out,retval]*/ VARIANT * pvarNamespaceURI ) = 0;
	virtual HRESULT __stdcall raw_lookupPrefix (
		/*[in]*/ VARIANT * pvarNamespaceURI,
		/*[out,retval]*/ VARIANT * pvarPrefix ) = 0;
	virtual HRESULT __stdcall raw_isDefaultNamespace (
		/*[in]*/ VARIANT * pvarNamespace,
		/*[out,retval]*/ VARIANT_BOOL * pfDefaultNamespace ) = 0;
	virtual HRESULT __stdcall raw_appendChild (
	/*[in]*/ struct IHTMLDOMNode * newChild,
	/*[out,retval]*/ struct IHTMLDOMNode * * node ) = 0;
	virtual HRESULT __stdcall raw_insertBefore (
	/*[in]*/ struct IHTMLDOMNode * newChild,
		/*[in]*/ VARIANT refChild,
	/*[out,retval]*/ struct IHTMLDOMNode * * node ) = 0;
	virtual HRESULT __stdcall raw_removeChild (
	/*[in]*/ struct IHTMLDOMNode * oldChild,
	/*[out,retval]*/ struct IHTMLDOMNode * * node ) = 0;
	virtual HRESULT __stdcall raw_replaceChild (
	/*[in]*/ struct IHTMLDOMNode * newChild,
	/*[in]*/ struct IHTMLDOMNode * oldChild,
	/*[out,retval]*/ struct IHTMLDOMNode * * node ) = 0;
	virtual HRESULT __stdcall raw_isSameNode (
	/*[in]*/ struct IHTMLDOMNode3 * otherNode,
		/*[out,retval]*/ VARIANT_BOOL * isSame ) = 0;
	virtual HRESULT __stdcall raw_compareDocumentPosition (
	/*[in]*/ struct IHTMLDOMNode * otherNode,
		/*[out,retval]*/ unsigned short * flags ) = 0;
	virtual HRESULT __stdcall raw_isSupported (
		/*[in]*/ BSTR feature,
		/*[in]*/ VARIANT version,
		/*[out,retval]*/ VARIANT_BOOL * pfisSupported ) = 0;
};

_COM_SMARTPTR_TYPEDEF(IHTMLDOMNode3, __uuidof(IHTMLDOMNode3));
HRESULT WINAPI dump_html_element( IHTMLElement *html_element )
{
	HRESULT hr = S_OK; 
	_bstr_t text; 
	wstring element_tag; 
	wstring element_id; 
	wstring element_class; 
	string inner_html; 

	do
	{
		_variant_t _text; 
		IHTMLDOMNode3Ptr html_dom_node; 
	
		inner_html = "null"; 
		hr = html_element->QueryInterface( __uuidof( IHTMLDOMNode3 ), ( PVOID* )&html_dom_node ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( NULL != html_dom_node )
		{
			hr = E_FAIL; 
			break; 
		}

		hr = html_dom_node->get_textContent( _text.GetAddress() ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( _text.vt != VT_BSTR )
		{
			hr = E_FAIL; 
			break; 
		}

		text = _text.bstrVal; 

		inner_html = text.operator char*(); 
	}while( FALSE ); 

	if( FAILED(hr) )
	{
		hr = html_element->get_outerHTML( text.GetAddress() ); 
		if( FAILED(hr)
			|| text.length() == 0 )
		{
			inner_html = "null"; 
		}
		else
		{
			inner_html = text.operator char*(); 
		}
	}

	hr = html_element->get_tagName( text.GetAddress() ); 
	if( FAILED(hr)
		|| text.length() == 0 )
	{
		element_tag = L"null"; 
	}
	else
	{
		element_tag = text.operator wchar_t*(); 
	}

	hr = html_element->get_id( text.GetAddress() ); 
	if( FAILED(hr)
		|| text.length() == 0 )
	{
		element_id = L"null"; 
	}
	else
	{
		element_id = text.operator wchar_t*(); 
	}

	hr = html_element->get_className( text.GetAddress() ); 
	if( FAILED(hr)
		|| text.length() == 0 )
	{
		element_class = L"null"; 
	}
	else
	{
		element_class = text.operator wchar_t*(); 
	}

	//if( element_tag.length() > 0 )
	{
		dbg_print( MSG_IMPORTANT, "\ntag:%ws id:%ws class:%ws inner html:%s\n", 
			element_tag.c_str(), 
			element_id.c_str(), 
			element_class.c_str(), 
			inner_html.c_str() ); 
	}

	return hr; 
}

HRESULT WINAPI dump_html_elements( HTML_ELEMENT_GROUP *group )
{
	HRESULT hr = S_OK; 
	INT32 i; 
	LONG element_count; 

	HTML_ELEMENT_GROUP temp_group; 
	IHTMLElementPtr element; 

	do 
	{
		element_count = ( LONG )group->size(); 

		for( i = 0; i < element_count; i ++ )
		{
			do 
			{
				element = group->at( i ); 
				ASSERT( NULL != element ); 

				dbg_print( MSG_IMPORTANT, "element %u", i ); 
				dump_html_element( element ); 

				{
					IHTMLElementPtr sub_element; 
					IDispatchPtr sub_element_disp; 
					IHTMLElementCollectionPtr sub_elements; 
					LONG sub_element_count; 
					_variant_t id; 
					_variant_t index; 
					LONG j; 

					hr = element->get_children(&sub_element_disp); 
					if( FAILED(hr))
					{
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
						break; 
					}

					hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
					if( FAILED(hr)
						|| NULL == sub_elements)
					{
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
						break; 
					}

					hr = sub_elements->get_length( &sub_element_count ); 
					if( FAILED(hr))
					{
						dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", hr); 
						break; 
					}

					for( j = 0; j < sub_element_count; j ++ )
					{
						V_VT( &id ) = VT_I4;
						V_I4( &id ) = j;
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( id, index, &sub_element_disp ); 
						if( FAILED( hr ) 
							|| sub_element_disp == NULL )
						{
							dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", i, hr); 

							if( SUCCEEDED( hr))
							{
								hr = E_FAIL; 
							}

							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( FAILED( hr )
							|| NULL == sub_element )
						{
							dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", i, hr); 

							ASSERT( FALSE ); 
							if( SUCCEEDED( hr))
							{
								hr = E_FAIL; 
							}

							break; 
						}

						sub_element->AddRef(); 
						temp_group.push_back( sub_element ); 
					}
				}
			}while( FALSE ); 
		} 

		release_html_element_group(group); 
		
		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = temp_group.begin(); it != temp_group.end(); it ++ )
		{
			group->push_back( (*it)); 
		}
	}while( FALSE );

	return hr; 
}

/************************************************************************************
首先针对链家网的结构进行内容抓取:
<div class="list-wrap"><ul class="house-lst" id="house-lst" data-bl="list">
	<li data-log_index="1" data-index="0" data-id="BJCY91698184">
		<div class="pic-panel">
			<a href="http://bj.lianjia.com/ershoufang/BJCY91698184.html" target="_blank" data-el="ershoufang">
				<img class="lj-lazy" style="display: inline;" alt="南北通透 三居室  送40平米的花园 " src="http://image1.ljcdn.com/appro/group2/M00/F7/9C/rBAF7FbeRE6AP7M5AAGGbronOF8674.jpg.280x210.jpg" data-apart-layout="http://image1.ljcdn.com/approl/SalesMgr-Web/hdpic/batch/common/110000/frame/20130516/data/dl/1/43381824/60000021/60025971/60177048/488111.jpg.280x210.jpg" data-img="http://image1.ljcdn.com/appro/group2/M00/F7/9C/rBAF7FbeRE6AP7M5AAGGbronOF8674.jpg.280x210.jpg" data-original="http://image1.ljcdn.com/appro/group2/M00/F7/9C/rBAF7FbeRE6AP7M5AAGGbronOF8674.jpg.280x210.jpg">
			</a>
		</div>
	<div class="info-panel">
		<h2>
			<a title="南北通透 三居室  送40平米的花园 " href="http://bj.lianjia.com/ershoufang/BJCY91698184.html" target="_blank" data-el="ershoufang">南北通透 三居室  送40平米的花园 </a></h2><div class="col-1"><div class="where"><a class="laisuzhou" href="http://bj.lianjia.com/xiaoqu/1111027374215/" data-el="xiaoqu"><span>梵谷水郡</span></a>&nbsp;&nbsp;<span><span>3室2厅&nbsp;&nbsp;</span></span><span>132.39平米&nbsp;&nbsp;</span><span>南 北</span></div><div class="other"><div class="con"><a href="http://bj.lianjia.com/ershoufang/jiuxianqiao/" data-el="region">酒仙桥二手房</a><span>/</span>低楼层(共18层)<span>/</span>2008年建板塔结合</div></div><div class="chanquan"><div class="left agency"><div class="view-label left"><span class="taxfree"></span><span class="taxfree-ex"><span>满五年唯一</span></span><span class="unique"></span><span class="unique-ex"><span>独家</span></span><span class="haskey"></span><span class="haskey-ex"><span>有钥匙</span></span></div></div></div></div><div class="col-3"><div class="price"><span class="num">580</span>万</div><div class="price-pre">43810 元/m²</div></div><div class="col-2"><div class="square"><div><span class="num">52</span>人</div><div class="col-look">看过此房</div></div></div></div></li><li data-log_index="2" data-index="1" data-id="BJCY91730094">
	<div class="page-box house-lst-page-box" page-data='{"totalPage":100,"curPage":1}' page-url="/ershoufang/pg{page}/" comp-module="page">
	<a class="on" href="/ershoufang/pg1/" data-page="1">1
	</a>
	<a href="/ershoufang/pg2/" data-page="2">2</a>
	<a href="/ershoufang/pg3/" data-page="3">3</a>
	<span>...</span>
	<a href="/ershoufang/pg100/" data-page="100">100</a>
	<a href="/ershoufang/pg2/" data-page="2">下一页</a>
	</div>
</div>
************************************************************************************/

HRESULT WINAPI dump_html_elements_on_page( IHTMLDocument2 *html_doc )
{
	HRESULT hr = S_OK; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr elements; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr sub_element; 
	IHTMLElementCollectionPtr sub_elements; 
	LONG element_count; 
	_bstr_t text; 
	_variant_t id; 
	_variant_t index; 
	LRESULT ret; 
	LONG sub_element_count;  
	INT32 i; 
	HTML_ELEMENT_GROUP temp_group; 

	do 
	{
		ASSERT( NULL != html_doc ); 
		hr = html_doc->get_all( &elements ); 
		if( FAILED( hr ) ) 
		{
			break; 
		}

		if( NULL == elements )
		{
			break; 
		}

		hr = elements->get_length( &element_count ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		text = L"HTML"; 
		id = text.GetBSTR(); 

		hr = elements->tags( id, &sub_element_disp ); 
		if( FAILED(hr)
			|| NULL == sub_element_disp )
		{
			dbg_print( MSG_ERROR, "get html tag elements error 0x%0.8x\n", hr ); 
			break; 
		}

		hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
		if( FAILED( hr ) 
			|| sub_element == NULL )
		{

			hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
			if( FAILED( hr ) 
				|| sub_elements == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = sub_elements->get_length( &sub_element_count ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			if( sub_element_count == 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			for( i = 0; i < sub_element_count; i ++ )
			{
				V_VT( &id ) = VT_I4;
				V_I4( &id ) = i;
				V_VT( &index ) = VT_I4;
				V_I4( &index ) = 0; 

				hr = sub_elements->item( id, index, &sub_element_disp ); //Get an element

				if( hr != S_OK )
				{
					break; 
				}

				if( NULL == sub_element_disp )
				{
					break; 
				}

				hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
				if( FAILED( hr ) 
					|| sub_element == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				sub_element->AddRef(); 
				temp_group.push_back( sub_element ); 
			}
		}
		else
		{
			sub_element->AddRef(); 
			temp_group.push_back( sub_element ); 
		}

		for( ; ; )
		{
			if( temp_group.size() == 0 )
			{
				break; 
			}

			dump_html_elements( &temp_group ); 
		}
	}while( FALSE ); 

	release_html_element_group( &temp_group ); 
	return hr; 
}

HRESULT WINAPI scramble_external_links( IHTMLDocument2 *page, 
									   EXTERMAL_LINK_INFO *link_info, 
									  LINK_LIST *links, 
									  BOOLEAN analyze_xpath )
{
	HRESULT hr = S_OK; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr elements; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr sub_element; 
	IHTMLElementCollectionPtr sub_elements; 
	//LONG element_count; 
	
	_bstr_t text; 
	_bstr_t url; 
	
	_variant_t id; 
	_variant_t index; 

	do 
	{
		ASSERT( NULL != page); 
		ASSERT( NULL != links); 

//#define LIANJIA_LINK_XPATH L"HTML|BODY|DIV|DIV id=matchid;name=matchid;|DIV|DIV|UL id=house-lst;name=house-lst;|LI|DIV|H2|A|"

		html_doc = page; 

		{
			LRESULT ret; 
			LPCWSTR lianjia_link_xpath = link_info->xpath.c_str(); 
			HTML_ELEMENT_GROUP group; 

			do 
			{
#if 0
				hr = html_doc->get_all( &elements ); 
				if( FAILED( hr ) ) 
				{
					break; 
				}

				if( NULL == elements )
				{
					break; 
				}

				hr = elements->get_length( &element_count ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				text = L"matchid"; 
				id = text.GetBSTR(); 

				V_VT( &index ) = VT_I4;
				V_I4( &index ) = 0; 

				do 
				{
					//text = L"house-lst"; 

					//注意这里的name参数，必须是HTML元素的id或name属性,tag是不可以直接用在此参数的，只能在HTML元素取出后，再进行判断。
					hr = elements->item( id, index, &sub_element_disp ); 
					if( FAILED( hr ) 
						|| sub_element_disp == NULL )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					ASSERT( sub_element_disp != NULL ); 

					//LONG _sub_element_count; 
					//LONG i; 

					hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
					if( FAILED( hr )
						|| NULL == sub_element )
					{
						break; 
					}
				}while( FALSE ); 
#endif //0

				//hr = dump_html_elements_on_page( html_doc ); 

				hr = get_html_elements_from_xpath( lianjia_link_xpath, html_doc, &group, analyze_xpath ); 

				do
				{
					for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
						it != group.end(); 
						it ++ )
					{
						IHTMLElement *_html_element; 
						IHTMLAnchorElementPtr link_element; 
						LPCWSTR temp_text; 

						do 
						{
							_html_element = ( *it ); 

							hr = _html_element->QueryInterface(IID_IHTMLAnchorElement,( void** )&link_element );
							if ( hr != S_OK )
							{
								break; 
							}

							link_element->get_href( text.GetAddress() ); 

							temp_text = text.operator wchar_t*(); 
							if( NULL == temp_text )
							{
								break; 
							}

							links->push_back( temp_text ); 
							dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 
						}while( FALSE ); 

						( *it )->Release(); 
					}

					group.clear(); 

			}while( FALSE ); 
		}while( FALSE ); 
		}
	}while( FALSE );  
	return hr; 
}

HRESULT WINAPI html_scramble_click_next( CWebBrowser2 *browser, HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info )
{
	HRESULT hr = S_OK; 
	//HRESULT _hr; 

	//LINK_LIST links; 

	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	//_bstr_t _url; 

	EXTERMAL_LINK_INFO link_info; 

	HRESULT _hr; 
	HTML_PAGE_CONTENT page_content; 
	HTML_ELEMENT_CONTENT *element_content; 
	HTML_ELEMENT_GROUP group; 

	do 
	{
		ASSERT( NULL != browser ); 

		disp = browser->GetDocument(); 

		if(disp == NULL)
		{
			dbg_print( MSG_IMPORTANT, "can not get the html document from browser\n" ); 
			hr = E_FAIL; 
			break; 
		}
		else
		{
			dbg_print( MSG_IMPORTANT, "get the html document from browser successfully\n" ); 
		}

		ASSERT( NULL != disp ); 

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) )
		{
			break; 
		}

		if( html_doc == NULL )
		{
			hr = E_FAIL; 
			break; 
		}

		do
		{
			_bstr_t html_title; 
			_variant_t attr_value; 

			hr = html_doc->get_title( html_title.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == html_title.operator const wchar_t*() )
			{
				break; 
			}
		}while( FALSE ); 

		//需要对数据进行精确的定位，当前不行，可通过TEXT，ATTR<a href="/ershoufang/pg2/" data-page="2">下一页</a>

		//L"HTML|BODY|DIV|DIV id=matchid;name=matchid;|DIV|DIV|UL id=house-lst;name=house-lst;|LI|DIV|H2|A|"; 

		//hr = dump_html_elements_on_page( html_doc ); 

		hr = get_html_elements_from_xpath( scramble_info->next_link_xpath.c_str(), html_doc, &group, FALSE ); 

		do
		{
			BOOLEAN aready_clicked = FALSE; 
			for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
				it != group.end(); 
				it ++ )
			{
				IHTMLElement *_html_element; 
				IHTMLAnchorElementPtr link_element; 
				LPCWSTR temp_text; 
				_bstr_t text; 

				do 
				{
					if( aready_clicked == TRUE )
					{
						break; 
					}

					_html_element = ( *it ); 

					dump_html_element( _html_element ); 

					hr = _html_element->QueryInterface(IID_IHTMLAnchorElement,( void** )&link_element );
					if ( hr != S_OK )
					{
						break; 
					}

					link_element->get_href( text.GetAddress() ); 

					temp_text = text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						break; 
					}

					{
						if( 0 != wcsnicmp( temp_text, L"http:", ARRAYSIZE( L"http:" ) - 1 ) ) 
						{
							browser->Stop(); 
							browser->on_web_page_locate_begin(); 
							browser->set_loading_url( L"" ); 

							_html_element->click(); 
							aready_clicked = TRUE; 
							start_worker( 0 ); 
						}
						else
						{
							ASSERT( FALSE ); 
						}
					}
				}while( FALSE ); 

				( *it )->Release(); 
			}

			group.clear(); 
		}while( FALSE ); 
	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI html_scramble( CWebBrowser2 *browser, HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info )
{
	HRESULT hr = S_OK; 
	//HRESULT _hr; 

	//LINK_LIST links; 
	
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	//_bstr_t _url; 

	EXTERMAL_LINK_INFO link_info; 

	HRESULT _hr; 
	HTML_PAGE_CONTENT page_content; 
	HTML_ELEMENT_CONTENT *element_content; 
	HTML_ELEMENT_GROUP group; 

	do 
	{
		ASSERT( NULL != browser ); 

		disp = browser->GetDocument(); 

		if(disp == NULL)
		{
			dbg_print( MSG_IMPORTANT, "can not get the html document from browser\n" ); 
			hr = E_FAIL; 
			break; 
		}
		else
		{
			dbg_print( MSG_IMPORTANT, "get the html document from browser successfully\n" ); 
		}

		ASSERT( NULL != disp ); 

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) )
		{
			break; 
		}

		if( html_doc == NULL )
		{
			hr = E_FAIL; 
			break; 
		}

		do
		{
			_bstr_t html_title; 
			_variant_t attr_value; 

			hr = html_doc->get_title( html_title.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == html_title.operator const wchar_t*() )
			{
				break; 
			}
		}while( FALSE ); 

		//需要对数据进行精确的定位，当前不行，可通过TEXT，ATTR<a href="/ershoufang/pg2/" data-page="2">下一页</a>

		//L"HTML|BODY|DIV|DIV id=matchid;name=matchid;|DIV|DIV|UL id=house-lst;name=house-lst;|LI|DIV|H2|A|"; 

		//hr = dump_html_elements_on_page( html_doc ); 

		hr = get_html_elements_from_xpath( scramble_info->next_link_xpath.c_str(), html_doc, &group, FALSE ); 

		do
		{
			for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
				it != group.end(); 
				it ++ )
			{
				IHTMLElement *_html_element; 
				IHTMLAnchorElementPtr link_element; 
				LPCWSTR temp_text; 
				_bstr_t text; 

				do 
				{
					_html_element = ( *it ); 

					dump_html_element( _html_element ); 

					hr = _html_element->QueryInterface(IID_IHTMLAnchorElement,( void** )&link_element );
					if ( hr != S_OK )
					{
						break; 
					}

					link_element->get_href( text.GetAddress() ); 

					temp_text = text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						break; 
					}

					//for( ; ; )
					//{
					//	Sleep( 10 ); 
					//}

					{
						if( 0 != wcsnicmp( temp_text, L"http:", ARRAYSIZE( L"http:" ) - 1 ) ) 
						{
							CString url; 
							CString _url; 
							url = browser->GetLocationURL(); 
							_url = HTML_NEXT_LINK_NEED_CLICK; 
							_url += url.GetBuffer(); 

							scramble_info->next_link.push_back( _url.GetBuffer() ); 
							dbg_print( MSG_IMPORTANT, "text %ws\n", _url.GetBuffer() ); 
						}
						else
						{
							scramble_info->next_link.push_back( temp_text ); 
							dbg_print( MSG_IMPORTANT, "text %ws\n", temp_text ); 
						}
					}
				}while( FALSE ); 

				( *it )->Release(); 
			}

			group.clear(); 

		}while( FALSE ); 

		link_info.xpath = scramble_info->link_list_xpath.c_str(); 

		hr = scramble_external_links(html_doc, &link_info, &scramble_info->links, FALSE ); 
		if( FAILED(hr))
		{
			dbg_print( MSG_ERROR, "get the target external links from the web page error 0x%0.8x\n", hr ); 
			break; 
		}
	}while( FALSE ); 

	return hr; 
}

//ULONG CALLBACK scramble_work()
//{
//	HRESULT hr; 
//	HRESULT _hr; 
//	LINK_LIST links; 
//	LINK_LIST_ITERATOR it; 
//
//	HRESULT _hr; 
//	HTML_PAGE_CONTENT page_content; 
//	HTML_ELEMENT_CONTENT *element_content; 
//
//	element_content = new HTML_ELEMENT_CONTENT(); 
//	if( element_content == NULL )
//	{
//		break; 
//	}
//
//	element_content->xpath = L"HTML|BODY class=ie;name=ie;|DIV class=wrapper;name=wrapper;|DIV class=title-line clear;name=title-line clear;|DIV class=line01;name=line01;|"; 
//
//	page_content.element_contents.push_back( element_content ); 
//
//	element_content = new HTML_ELEMENT_CONTENT(); 
//	if( element_content == NULL )
//	{
//		break; 
//	}
//
//	element_content->xpath = L"HTML|BODY class=ie;name=ie;|DIV class=wrapper;name=wrapper;|SECTION class=panel-box panel-box-info clear;name=panel-box panel-box-info clear;|DIV class=info-box left;name=info-box left;|DIV class=house-del;name=house-del;|UL|LI class=last;name=last;|DIV class=house-uni;name=house-uni;|"; 
//	page_content.element_contents.push_back( element_content ); 
//
//	for( it = links.begin(); 
//		it != links.end(); 
//		it ++ )
//	{
//		_hr = scramble_web_page_content_ex( browser, 
//			( *it ).c_str(), 
//			&page_content ); 
//
//		if( FAILED(_hr ) )
//		{
//			dbg_print( MSG_ERROR, "scramble html page error %u\n", _hr ); 
//			hr = _hr; 
//		}
//	}
//
//	return hr; 
//}

#include "work_job.h"

HRESULT WINAPI start_worker( PVOID param )
{
	HRESULT ret; 

	ret = add_job_to_default_worker( param, web_page_work ); 
	return ret; 
}


HRESULT WINAPI html_scramble_ex( CWebBrowser2 *browser, 
							 LPCWSTR url )
{
	HRESULT hr = S_OK; 
	//HRESULT _hr; 

	LINK_LIST links; 
	LINK_LIST_ITERATOR it; 

	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	//_bstr_t _url; 

	EXTERMAL_LINK_INFO link_info; 

	do 
	{
		ASSERT( NULL != browser ); 

		browser_safe_navigate_start( browser, url ); 

		start_worker( 0 ); 

	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI html_scramble_ex2( CWebBrowser2 *browser, 
								LPCWSTR url )
{
	HRESULT hr = S_OK; 

	do 
	{
		ASSERT( NULL != browser ); 

		browser_safe_navigate_start( browser, url ); 

		start_worker( ( PVOID) 5 ); 

	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI locate_to_url( CWebBrowser2 *browser, 
							 LPCWSTR url )
{
	HRESULT hr; 

	do 
	{
		ASSERT( NULL != browser ); 

		{
			CString current_url; 
			current_url = browser->GetLocationURL(); 
			if( 0 == _wcsicmp( current_url.GetBuffer(), url ) )
			{
				ASSERT( FALSE ); 
				break; 
			}
		}

		hr = browser_safe_navigate_start( browser, url ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		hr = start_worker( ( PVOID )HTML_ACTION_WORK_LOCATE_TO_PAGE ); 

	}while( FALSE ); 

	return hr; 
}

LRESULT WINAPI init_lianjia_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content; 

#define LIANJIA_URL L"http://bj.lianjia.com/ershoufang/?utm_source=baidu&utm_medium=ppc&utm_term=C52763&utm_content=D205&utm_campaign=J3&utm_creative=1021406868&utm_network=1&utm_keyword=2502732529&utm_placement="

	do 
	{
		scramble_info->url = LIANJIA_URL; 
#define LIAN_JIA_LINK_INFO L"HTML|BODY class=ie;|DIV class=wrapper;|DIV id=matchid;class=main-box clear;|DIV class=con-box;|DIV class=list-wrap;|UL id=house-lst;class=house-lst;|LI|DIV class=info-panel;|H2|A|"
#define LINK_JIA_LINK_NEXT_INFO L"HTML|BODY class=ie;|DIV class=wrapper;|DIV id=matchid;class=main-box clear;log-mod=list;mod-id=lj-ershoufang-list;|DIV class=con-box;|DIV class=list-wrap;|DIV class=page-box house-lst-page-box;comp-module=page;|A element_text=下一页;|"

		scramble_info->next_link_xpath = LINK_JIA_LINK_NEXT_INFO; 
		scramble_info->link_list_xpath = LIAN_JIA_LINK_INFO; 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML|BODY class=ie;|DIV class=wrapper;|SECTION class=panel-box panel-box-info clear;|DIV class=info-box left;|DIV class=desc-text clear;|"; 

		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML|BODY class=ie;|DIV class=wrapper;|DIV class=title-line clear;|DIV class=line01;|H1 class=title-box left;|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_page_content_info( HTML_PAGE_CONTENT *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENTS_ITERATOR it; 
	do 
	{
		for( ; ; )
		{
			it = page_content->element_contents.begin(); 
			if( it == page_content->element_contents.end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 
			delete ( *it ); 
			page_content->element_contents.erase( it ); 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI uninit_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		for( ; ; )
		{
			HTML_ELEMENT_CONTENTS_ITERATOR it = scramble_info->page_content.element_contents.begin(); 
			if( it == scramble_info->page_content.element_contents.end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 
			delete ( *it ); 
			scramble_info->page_content.element_contents.erase( it ); 
		}

	} while ( FALSE ); 

	return ret; 
}

/*************************************************************************************
23851519294;trace-index=0;data-href=https://click.simba.taobao.com/cc_im?p=%D0%D0%B3%B5%BC%C7%C2%BC%D2%C7&s=1757136702&k=449&e=3i383X%2BoYQHHokOWjQMxOoaMKueDT6pcpOJpautRqe6gxvhS7cAiw3nRnpyyVgrEPRo8BPzmV1XzwOOSOELRcpphOdkwRs%2BxpbUgvPtkeg%2B8nB7ntLVC2IGnG%2FbEAp4wvOnMjLjlsM%2BFaJNE0yl8c03t0HquzhBSbmVCwD%2FUYkJCnDI3IS1fl6F%2F803UiC20oS1mIbktGwJ8Clo3VjypuTQnHwtjekeL9AgfCoKwnHYxts1V51%2Fk0inbh8wFy%2Bv67XshIe2YityDsgIS%2BuiXXoqOtZwuSPVsoCmRo5g3jN3VZdMwYe3NtbEw5%2FLLg5p%2ButLtSuor1W%2FlQ0JhP6Iaj8dPyM12q1owNNV1Ze0ECdupd0XAWG450MXTJUyiXRG%2FPraocAMHad%2FuBSoa2suBuwtNUXNUOgcjKQEMJHZHTTby9viEa%2BFcEjOafmXi9fYL;data-nid=523851519294;data-spm-anchor-id=a230r.1.14.1;data-spm-act-id=a230r.1.14.1.WKHsnM;|
*************************************************************************************/

/************************************************************************************
http://search.jd.com/Search?keyword=%E6%8A%95%E5%BD%B1%E4%BB%AA&enc=utf-8&suggest=6.def.0&wq=t&pvid=meajx9mi.ovrjdb
HTML|BODY|DIV id=J_searchWrap;class=w;|DIV id=J_container;class=container;|DIV id=J_main;class=g-main2;data-lazy-img-install=1;|DIV class=m-list;|DIV class=ml-wrap;|DIV id=J_goodsList;class=goods-list-v1 gl-type-1 J-goods-list;|UL class=gl-warp clearfix;data-tpl=1;|LI class=gl-item;data-sku=1000294;|DIV class=gl-i-wrap;|DIV class=p-img;|A element_text= ;onclick=searchlog(1,1000294,0,2,'','flagsClk=4194952');title=【13000+客户好评高度认可】轻松投影百寸大荧幕，3200流明高亮告别黑屋会议，通用VGA接口连接直接投影。更清晰款点这里！;target=_blank;|
************************************************************************************/

LRESULT WINAPI init_taobao_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content; 

#define TAOBAO_URL L"https://s.taobao.com/search?q=%E7%AC%94%E8%AE%B0%E6%9C%AC&imgfile=&commend=all&ssid=s5-e&search_type=item&sourceId=tb.index&spm=a21bo.7724922.8452-taobao-item.2&ie=utf8&initiative_id=tbindexz_20160327"; 
	do 

	{
		scramble_info->url = TAOBAO_URL; 

		scramble_info->next_link_xpath = L"HTML class=ks-trident7 ks-trident ks-ie7 ks-ie;lang=zh-CN;|BODY class=response-narrow ie-updator__67;|DIV id=page;class=srp-page;|DIV id=main;class=srp-main;|DIV class=grid-total;|DIV class=grid-left;|DIV id=mainsrp-pager;|DIV class=m-page g-clearfix;|DIV class=wraper;|DIV class=inner clearfix;|UL class=items;|LI class=item next;|A class=J_Ajax num icon-tag;element_text=下一页  ;|"; 
		scramble_info->link_list_xpath = L"HTML class=ks-trident7 ks-trident ks-ie7 ks-ie;lang=zh-CN;|BODY class=response-narrow ie-updator__67;|DIV id=page;class=srp-page;|DIV id=main;class=srp-main;|DIV class=grid-total;|DIV class=grid-left;|DIV id=mainsrp-itemlist;|DIV class=m-itemlist;|DIV class=grid;|DIV class=items g-clearfix;|DIV class=item  ;|DIV class=pic-box J_MouseEneterLeave J_PicBox;|DIV class=pic-box-inner;|DIV class=pic;|A class=pic-link J_ClickStat J_ItemPicA;|"; 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1000294 JD JD-1;version=140120;data-lazyload-install=1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=name;"; 

		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=tm-dou11 enableHover;|DIV id=page;|DIV id=content;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|DIV class=tm-fcs-panel;|DIV class=tm-coupon-panel;|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		scramble_info->page_content.element_contents.push_back( element_content ); 

		//element_content = new HTML_ELEMENT_CONTENT(); 
		//if( element_content == NULL )
		//{
		//	break; 
		//}

		//element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1000294 JD JD-1;version=140120;data-lazyload-install=1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=summary;|DIV id=summary-service;"; 
		//scramble_info->page_content.element_contents.push_back( element_content ); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI init_tianmao_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content; 

#define TIAN_MAO_URL L"https://list.tmall.com/search_product.htm?q=%CD%E0%D7%D3&type=p&vmarket=&spm=875.7931836.a2227oh.d100&from=mallfp..pc_1_searchbutton"  
	//L"https://list.tmall.com/search_product.htm?q=%C4%BE%B0%E5&type=p&vmarket=&spm=875.7931836.a2227oh.d100&from=mallfp..pc_1_searchbutton"; 
	//L"https://list.tmall.com/search_product.htm?q=%B1%CA%BC%C7%B1%BE&type=p&vmarket=&spm=875.7931836.a2227oh.d100&from=mallfp..pc_1_searchbutton"; 
	do 
	{
		ret = get_domain_name_in_url( TIAN_MAO_URL, scramble_info->domain ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_info->url = TIAN_MAO_URL; 
//HTML class=ks-trident7 ks-trident ks-ie7 ks-ie;|BODY|DIV id=mallPage;class=  tmall-chaoshi  page-market;|DIV id=content;|DIV class=main;|DIV class=main-wrap;|DIV id=J-listContainer;class=mainItemsList;|UL id=J_ProductList;class=product-list;|LI id=J_Product39860793860;class=product;|DIV class=productInfo;|DIV class=product-img;|A element_text=;|
		scramble_info->next_link_xpath = L"HTML class=ks-trident7 ks-trident ks-ie7 ks-ie no-touch;|BODY class=pg ie ie7 w0;|DIV class=page;|DIV id=mallPage;class= mallist tmall-3c  page-market;|DIV id=content;|DIV class=main       ;|DIV class=ui-page;|DIV class=ui-page-wrap;|B class=ui-page-num;|A class=ui-page-next;element_text=下一页>>;|"; 
		scramble_info->link_list_xpath = L"HTML class=ks-trident7 ks-trident ks-ie7 ks-ie no-touch;|BODY class=pg ie ie7 w0;|DIV class=page;|DIV id=mallPage;class= mallist tmall-3c  page-market;|DIV id=content;|DIV class=main       ;|DIV id=J_ItemList;class=view grid-nosku ;|DIV|DIV class=product-iWrap;|P class=productTitle;|A|"; 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"标题"; 
		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|DIV class=tb-detail-hd;|H1|A|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"价格"; 
		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|DIV class=tm-fcs-panel;|DL id=J_StrPriceModBox;|"; 
								 //L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=tm-dou11 enableHover;|DIV id=page;class=tm-style-detail;|DIV id=content;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|DIV class=tm-fcs-panel;"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"销量"; 
		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|UL class=tm-ind-panel;|LI class=tm-ind-item tm-ind-sellCount ;|DIV class=tm-indcon;|"; 
			//L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=tm-dou11 enableHover;|DIV id=page;class=tm-style-detail;|DIV id=content;|DIV id=detail;|DIV id=J_DetailMeta;class=tm-detail-meta tm-clear;|DIV class=tm-clear;|DIV class=tb-property;|DIV class=tb-wrap;|DIV class=tb-detail-hd;"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"品牌/参数"; 
		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=bd;|DIV class=grid-s5m0 tm-clear;|DIV class=col-main tm-clear;|DIV id=mainwrap;|DIV id=attributes;|DIV id=J_AttrList;|"; 
			//L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=bd;|DIV class=grid-s5m0 tm-clear;|DIV class=col-main tm-clear;|DIV id=mainwrap;class=main-wrap J_TRegion;|DIV id=attributes;class=attributes;|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"详情"; 
		element_content->xpath = L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=bd;|DIV class=grid-s5m0 tm-clear;|DIV class=col-main tm-clear;|DIV id=mainwrap;class=main-wrap;|DIV id=J_Detail;"; 
		//L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=bd;|DIV class=grid-s5m0 tm-clear;|DIV class=col-main tm-clear;|DIV id=mainwrap;class=main-wrap;|DIV id=attributes;class=attributes;|DIV id=J_AttrList;class=attributes-list;|DIV id=J_BrandAttr;class=tm-clear tb-hidden tm_brandAttr;|"; 
		//L"HTML class=w1190 ks-trident7 ks-trident ks-ie7 ks-ie;|BODY class=enableHover;|DIV id=page;|DIV id=content;class=tm-relate-hook;|DIV id=bd;|DIV class=grid-s5m0 tm-clear;|DIV class=col-main tm-clear;|DIV id=mainwrap;class=main-wrap J_TRegion;|DIV id=attributes;class=attributes;|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI init_jd_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content; 

#define JD_URL L"http://search.jd.com/Search?keyword=%E6%8A%95%E5%BD%B1%E4%BB%AA&enc=utf-8&suggest=6.def.0&wq=t&pvid=meajx9mi.ovrjdb"

	do 
	{
		scramble_info->url = JD_URL; 

		ret = get_domain_name_in_url( JD_URL, scramble_info->domain ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//scramble_info->url = L"click:"; 
		//scramble_info->url = JD_URL; 

		scramble_info->next_link_xpath = L"BODY|DIV id=J_searchWrap;class=w;|DIV id=J_container;class=container;|DIV id=J_main;class=g-main2;|DIV class=m-list;|DIV class=ml-wrap;|DIV class=page clearfix;|DIV id=J_bottomPage;class=p-wrap;|SPAN class=p-num;|A class=pn-next;element_text=下一页>;|"; 
		scramble_info->link_list_xpath = L"BODY|DIV class=w; id=J_searchWrap;|DIV class=container; id=J_container;|DIV class=g-main2; id=J_main;|DIV class=m-list;|DIV class=ml-wrap;|DIV id=J_goodsList;|UL class=gl-warp clearfix;|LI|DIV class=gl-i-wrap;|DIV class=p-img;|A|"; 

		//
		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"价格"; 
		element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1670321352 POP POP-1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=summary;|DIV id=summary-price;|"; 
		//L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1000294 JD JD-1;version=140120;data-lazyload-install=1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=name;"; 

		scramble_info->page_content.element_contents.push_back( element_content ); 

		//
		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}
		
		element_content->name = L"服务"; 
		element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1670321352 POP POP-1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=choose;class=clearfix p-choose-wrap;|DIV id=choose-service;|"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 

		//
		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->name = L"详情"; 
		element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1670321352 POP POP-1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|"; 

		//element_content->xpath = L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1000294 JD JD-1;version=140120;data-lazyload-install=1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=summary;|DIV id=summary-service;"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 
	
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI init_ebay_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content; 

#define EBAY_URL L"http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2050601.m570.l1313.TR0.TRC0.H0.Xnotebook.TRS0&_nkw=notebook&_sacat=0"

	do 
	{
		scramble_info->url = EBAY_URL; 

		ret = get_domain_name_in_url( EBAY_URL, scramble_info->domain ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_info->next_link_xpath = L"HTML class=no-touch;lang=en;|BODY class=sz1200 full-width LV IE IE_11 IE_11_0 gh-1199 gh-979 gh-939 gh-899 gh-flex;|DIV id=Body;|DIV id=LeftCenterBottomPanelDF;|DIV id=CenterWrapper;|DIV id=Center;class=rr_present;|DIV id=CenterPanel;class=full-width left c-std;|DIV id=CenterPanelInner;|DIV id=Results;class=results ;|DIV id=spf_content;|DIV id=rsTabs;class=rst;|DIV id=mainContent;class=t225;role=main;|DIV id=PaginationAndExpansionsContainer;|TABLE id=Pagination;class=pagn pagn-m;role=presentation;aria-describedby=pag-lbl;|TBODY|TR|TD class=pagn-next;_sp=p2045573.m1686.l1581;|A class=gspr next;|"; 
		scramble_info->link_list_xpath = L"HTML class=no-touch;lang=en;|BODY class=sz760 full-width LV IE IE_11 IE_11_0 gh-1199 gh-979 gh-939 gh-899 gh-flex;|DIV id=Body;|DIV id=LeftCenterBottomPanelDF;|DIV id=CenterWrapper;|DIV id=Center;|DIV id=CenterPanel;class=full-width left c-std;|DIV id=CenterPanelInner;|DIV id=Results;class=results ;|DIV id=spf_content;|DIV id=rsTabs;class=rst;|DIV id=mainContent;class=t225;|w-root id=w1-1;|DIV id=e1-207;class=rsw;|DIV id=ResultSetItems;class=clearfix;|UL id=ListViewInner;|LI class=sresult lvresult clearfix li shic;|H3 class=lvtitle;|A|"; 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML lang=en;|BODY class=vi-contv2  lhdr-ie- vi-hd-ops gh-1199;|DIV id=Body;class= sz940  ;|DIV id=CenterPanelDF;|DIV id=CenterPanel;class=   ebaylocale_en_US ebay_longlngsite   ;|DIV id=CenterPanelInternal;|DIV id=PicturePanel;class=pp-c;|DIV|H1 id=itemTitle;class=it-ttl;|"; 

		scramble_info->page_content.element_contents.push_back( element_content ); 

		element_content = new HTML_ELEMENT_CONTENT(); 
		if( element_content == NULL )
		{
			break; 
		}

		element_content->xpath = L"HTML lang=en;|BODY class=vi-contv2  lhdr-ie- vi-hd-ops gh-1199;|DIV id=Body;class= sz940  ;|DIV id=CenterPanelDF;|DIV id=CenterPanel;class=   ebaylocale_en_US ebay_longlngsite   ;|DIV id=CenterPanelInternal;|DIV id=LeftSummaryPanel;class=lsp-c lsp-cRight  lsp-cL300  lsp-de;|DIV id=mainContent;class=is ;|FORM name=viactiondetails;|DIV class=c-std vi-ds3cont-box-marpad;|DIV class=actPanel  vi-noborder ;|DIV class=u-cb;|"; 

		scramble_info->page_content.element_contents.push_back( element_content ); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI init_scramble_info(HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	return init_jd_scramble_info( scramble_info ); 
}

/*************************************************************************************
内核功能：
1.用户自由选择内容。

*************************************************************************************/

LRESULT WINAPI _load_scramble_info_from_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info = NULL; 

	do 
	{
		scramble_info = new HTML_LIST_ELEMENT_SCRAMBLE_INFO(); 
		if( NULL == scramble_info )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = load_scramble_info_from_file( file_name, scramble_info ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = get_domain_name_in_url( scramble_info->url.c_str(), scramble_info->domain ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_infos.push_back( scramble_info ); 
		scramble_info = NULL; 
	}while( FALSE ); 
	
	if( NULL != scramble_info )
	{
		delete scramble_info; 
	}

	return ret; 
}

LRESULT WINAPI load_scramble_infos() 
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info = NULL; 

	do 
	{
		scramble_info = new HTML_LIST_ELEMENT_SCRAMBLE_INFO(); 
		if( NULL == scramble_info )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = init_tianmao_scramble_info( scramble_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_infos.push_back( scramble_info ); 

		scramble_info = new HTML_LIST_ELEMENT_SCRAMBLE_INFO(); 
		if( NULL == scramble_info )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = init_jd_scramble_info( scramble_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_infos.push_back( scramble_info ); 

		scramble_info = new HTML_LIST_ELEMENT_SCRAMBLE_INFO(); 
		if( NULL == scramble_info )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = init_ebay_scramble_info( scramble_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		scramble_infos.push_back( scramble_info ); 

	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI uninit_scramble_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_LIST_ELEMENT_SCRAMBLE_INFOS_ITERATOR it; 

	do 
	{
		for( ; ; )
		{
			it = scramble_infos.begin(); 
			
			if( it == scramble_infos.end() )
			{
				break; 
			}

			uninit_scramble_info( *it ); 
			delete ( *it ); 
		}

	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI select_scramble_info( LPCWSTR url, 
									HTML_LIST_ELEMENT_SCRAMBLE_INFO **info )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_LIST_ELEMENT_SCRAMBLE_INFOS::reverse_iterator it; 

	do 
	{
		*info = NULL; 
		for( it = scramble_infos.rbegin(); it != scramble_infos.rend(); it ++ )
		{
			if( NULL != wcsstr( url, ( *it )->domain.c_str() ) )
			{
				*info = ( *it ); 
				break; 
			}
		}

		if( it == scramble_infos.rend() )
		{
			ret = ERROR_NOT_FOUND; 
		}

	}while(FALSE );

	return ret; 
}

LRESULT WINAPI _get_children_image_element( HTML_ELEMENT_GROUP *group, 
										   HTML_ELEMENT_GROUP *group_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	HRESULT _hr; 
	INT32 i; 
	LONG element_count; 

	HTML_ELEMENT_GROUP temp_group; 
	IHTMLElementPtr element; 
	_bstr_t html_element_tag; 
	LPCWSTR tag_name; 

	ASSERT( NULL != group ); 

	do 
	{
		if( NULL == group )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		element_count = ( LONG )group->size(); 

		for( i = 0; i < element_count; i ++ )
		{
			do 
			{
				element = group->at( i ); 
				ASSERT( NULL != element ); 

				dbg_print( MSG_IMPORTANT, "element %u", i ); 
				dump_html_element( element ); 

				do 
				{
					_hr = element->get_tagName( html_element_tag.GetAddress() ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					tag_name = html_element_tag.operator wchar_t*(); 
					if( NULL == tag_name )
					{
						break; 
					}

					if( 0 == _wcsicmp( tag_name, L"img" ) )
					{
						element->AddRef(); 
						group_out->push_back( element ); 
					}
				} while ( FALSE );

				{
					IHTMLElementPtr sub_element; 
					IDispatchPtr sub_element_disp; 
					IHTMLElementCollectionPtr sub_elements; 
					LONG sub_element_count; 
					_variant_t id; 
					_variant_t index; 
					LONG j; 

					hr = element->get_children(&sub_element_disp); 
					if( FAILED(hr))
					{
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
						break; 
					}

					hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
					if( FAILED(hr)
						|| NULL == sub_elements)
					{
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
						break; 
					}

					hr = sub_elements->get_length( &sub_element_count ); 
					if( FAILED(hr))
					{
						dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", hr); 
						break; 
					}

					for( j = 0; j < sub_element_count; j ++ )
					{
						V_VT( &id ) = VT_I4;
						V_I4( &id ) = j;
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( id, index, &sub_element_disp ); 
						if( FAILED( hr ) 
							|| sub_element_disp == NULL )
						{
							dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", i, hr); 

							if( SUCCEEDED( hr))
							{
								hr = E_FAIL; 
							}

							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( FAILED( hr )
							|| NULL == sub_element )
						{
							dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", i, hr); 

							ASSERT( FALSE ); 
							if( SUCCEEDED( hr))
							{
								hr = E_FAIL; 
							}

							break; 
						}

						do 
						{
							_hr = sub_element->get_tagName( html_element_tag.GetAddress() ); 
							if( FAILED( _hr ) )
							{
								break; 
							}

							tag_name = html_element_tag.operator wchar_t*(); 
							if( NULL == tag_name )
							{
								break; 
							}

							if( 0 == _wcsicmp( tag_name, L"img" ) )
							{
								sub_element->AddRef(); 
								group_out->push_back( sub_element ); 
							}
						} while ( FALSE );

						sub_element->AddRef(); 
						temp_group.push_back( sub_element ); 
					}
				}
			}while( FALSE ); 
		} 

		release_html_element_group(group); 

		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = temp_group.begin(); it != temp_group.end(); it ++ )
		{
			group->push_back( (*it)); 
		}
	}while( FALSE );

	return hr; 
}

HRESULT WINAPI get_children_image_element( IHTMLElement *element, HTML_ELEMENT_GROUP *group_out )
{
	HRESULT hr = S_OK; 
	HTML_ELEMENT_GROUP temp_group; 

	do 
	{
		element->AddRef(); 
		temp_group.push_back( element ); 

		for( ; ; )
		{
			if( temp_group.size() == 0 )
			{
				break; 
			}

			_get_children_image_element( &temp_group, group_out ); 
		}
	}while( FALSE ); 

	release_html_element_group( &temp_group ); 
	return hr; 
}

HRESULT WINAPI click_next_link_button( IHTMLElement *html_element )
{
	HRESULT hr = S_OK; 

	do 
	{
		ASSERT( NULL != html_element ); 
		html_element->click(); 
	}while( FALSE ); 

	return hr; 
}