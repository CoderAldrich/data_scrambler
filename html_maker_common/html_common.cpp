/*
 * 
 * Copyright (c) 2003, Michael Carruth All rights reserved.
 * This file is a part of CrashRpt library.
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
#include <strsafe.h>
#import "msxml.tlb"
#include "html_common.h"
//#define XPATH_CREATE
#include "html_xpath.h"
#include "input_content.h"

HRESULT WINAPI scroll_expand_html_doc( IHTMLDocument2 *html_doc )
{
	HRESULT hr = S_OK; 
	IHTMLWindow2Ptr html_wnd; 

	do 
	{
		hr = html_doc->get_parentWindow ( ( IHTMLWindow2** )&html_wnd ); 
		if( FAILED( hr) )
		{
			break; 
		}

		hr = html_wnd->scrollTo(0xffff, 0xffff); 
		if( FAILED(hr))
		{
			dbg_print( MSG_FATAL_ERROR, "scroll the html window error 0x%0.8x\n", hr ); 
			break; 
		}

		hr = html_wnd->scrollTo(0, 0); 
		if( FAILED(hr))
		{
			dbg_print( MSG_FATAL_ERROR, "scroll the html window error 0x%0.8x\n", hr ); 
			break; 
		}

	}while( FALSE );

	return hr; 
}

LRESULT WINAPI utf8_to_unicode_ex( LPCSTR text, wstring &text_out )
{
	LRESULT ret; 
	ULONG cc_ret_len; 
	LPWSTR buf = NULL; 

	ASSERT( NULL != text ); 

	do 
	{
		ret = utf8_to_unicode( text, NULL, 0, &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			if( ret != ERROR_BUFFER_TOO_SMALL )
			{
				break; 
			}

			buf = ( LPWSTR )malloc( ( cc_ret_len << 1 ) ); 
			if( NULL == buf )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}
		else
		{
			ASSERT( FALSE ); 
			text_out.empty(); 
			break; 
		}

		ret = utf8_to_unicode( text, buf, cc_ret_len, &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		text_out = buf; 
	} while ( FALSE );

	if( NULL != buf )
	{
		free( buf ); 
	}

	return ret; 
}

LRESULT WINAPI unicode_to_utf8_ex( LPCWSTR text, string &text_out )
{
	LRESULT ret; 
	ULONG cc_ret_len; 
	LPSTR buf = NULL; 

	ASSERT( NULL != text ); 

	do 
	{
		ret = unicode_to_utf8( text, NULL, 0, &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			if( ret != ERROR_BUFFER_TOO_SMALL )
			{
				break; 
			}

			buf = ( LPSTR )malloc( cc_ret_len ); 
			if( NULL == buf )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}
		else
		{
			ASSERT( FALSE ); 
			text_out.empty(); 
			break; 
		}

		ret = unicode_to_utf8( text, buf, cc_ret_len, &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		text_out = buf; 
	} while ( FALSE );

	if( NULL != buf )
	{
		free( buf ); 
	}
	return ret; 
}

LRESULT WINAPI load_html_page_info( HTML_PAGE_INFO *html_page, MSXML::IXMLDOMNode *xml_node ); 

LPCWSTR WINAPI get_html_element_tag_desc( LPCWSTR tag )
{
	LPCWSTR desc = L""; 

	ASSERT( NULL != tag ); 

	do 
	{
		if( _wcsicmp( L"a", tag ) == 0 )
		{
			desc = L"超链接"; 
			break; 
		}
	}while( FALSE );

	return desc; 
}

HRESULT WINAPI get_html_element_url( IHTMLElement *html_element, 
									wstring *url )
{
	HRESULT hr = S_OK; 
	IHTMLAnchorElementPtr link_element; 
	_bstr_t text; 
	LPCWSTR temp_text; 

	do 
	{
		ASSERT( NULL != html_element ); 

		hr = html_element->QueryInterface(IID_IHTMLAnchorElement,( void** )&link_element );
		if ( hr != S_OK )
		{
			break; 
		}

		if(NULL == link_element)
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		link_element->get_href( text.GetAddress() ); 

		temp_text = text.operator wchar_t*(); 
		if( NULL == temp_text )
		{
			break; 
		}

		*url = temp_text; 
	} while ( FALSE );

	return hr; 
}

HRESULT WINAPI get_html_element_content(IHTMLElement *html_element,
    wstring *content)
{
    HRESULT hr = S_OK;
    IHTMLAnchorElementPtr link_element;
    _bstr_t text;
    LPCWSTR temp_text;

    do
    {
        ASSERT(NULL != html_element);

        hr = html_element->get_innerText(text.GetAddress());

        temp_text = text.operator wchar_t*();
        if (NULL == temp_text)
        {
            break;
        }

        *content = temp_text;
    } while (FALSE);

    return hr;
}

HRESULT WINAPI get_html_element_title( IHTMLElement *html_element, wstring *title )
{
	HRESULT hr = S_OK; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	CString text; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != title ); 

		title->empty(); 

		do 
		{
			LPCWSTR tag_desc; 
			hr = html_element->get_tagName( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				break; 
			}

			tag_desc = get_html_element_tag_desc( _temp_text ); 

			if( *tag_desc == L'\0' )
			{
				text += _temp_text; 
			}
			else
			{
				text += tag_desc; 
			}
			
			text += L" "; 
		} while ( FALSE ); 

		do 
		{
			/********************************************************
			HTML元素属性包括:
			1.ID,CLASS,NAME,TAG
			********************************************************/
			hr = html_element->get_id( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				break; 
			}

			text += L"标识号='"; 
			text += _temp_text; 

			text += L"'"; 
			text += L" "; 
		}while( FALSE ); 

		do 
		{
			/********************************************************
			HTML元素属性包括:
			1.ID,CLASS,NAME,TAG
			********************************************************/
			hr = html_element->get_className( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				break; 
			}

			text += L"样式类型='"; 
			text += _temp_text; 

			text += L"'"; 
			text += L" "; 
		} while ( FALSE ); 

		do 
		{
			_bstr_t attr_name; 
			_variant_t attr_value; 
			
			attr_name = L"name"; 

			hr = html_element->getAttribute( attr_name.GetBSTR(), 
				2, attr_value.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( attr_value.vt != VT_BSTR )
			{
				break; 
			}

			temp_text = attr_value.bstrVal; 

			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				break; 
			}

			text += L"名称='"; 
			text += _temp_text; 

			text += L"'"; 
			text += L" "; 
		} while ( FALSE ); 

		//do 
		//{
		//	//可以加入TEXT,图片
		//} while ( FALSE ); 

		*title = text; 
	} while ( FALSE ) ;

	return hr; 
}

/****************************************************
如果使用域名做文件名，冲突率比较大
如果使用URL做文件名，冲突率更小
如果使用时间做文件，冲突最小，但不可以添加
使用URL，给页面以添加功能
****************************************************/
LRESULT WINAPI get_file_name_from_url( LPCWSTR url, CStringW &file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name_char[] = { L'_', L'-', L'.', L',' }; 
	LONG cc_url_len; 
	LONG i; 
	BOOLEAN char_found = FALSE; 
	LONG j; 

	do 
	{
		file_name = L""; 

		cc_url_len = wcslen( url ); 
		for( i = 0; i < cc_url_len; i ++ )
		{
			if( url[ i ] >= 'A' && url[ i ] <= 'Z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= 'a' && url[ i ] <= 'z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= '0' && url[ i ] <= '9' )
			{
				file_name += url[ i ]; 
			}
			else
			{
				char_found = FALSE; 
				for( j = 0; j < ARRAYSIZE( file_name_char ); j ++ )
				{
					if( file_name_char[ j ] == url[ i ] )
					{
						char_found = TRUE; 
						break; 
					}
				}

				if( char_found == TRUE )
				{
					file_name += url[ i ]; 
				}
				else
				{
					file_name += L'_'; 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

wstring WINAPI variant2string( VARIANT *va )
{

	wstring text = L"";

	/*****************************************************
	VT_NULL	= 1,
	VT_I2	= 2,
	VT_I4	= 3,
	VT_R4	= 4,
	VT_R8	= 5,
	VT_CY	= 6,
	VT_DATE	= 7,
	VT_BSTR	= 8,
	VT_DISPATCH	= 9,
	VT_ERROR	= 10,
	VT_BOOL	= 11,
	VT_VARIANT	= 12,
	VT_UNKNOWN	= 13,
	VT_DECIMAL	= 14,
	VT_I1	= 16,
	VT_UI1	= 17,
	VT_UI2	= 18,
	VT_UI4	= 19,
	VT_I8	= 20,
	VT_UI8	= 21,
	VT_INT	= 22,
	VT_UINT	= 23,
	VT_VOID	= 24,
	VT_HRESULT	= 25,
	VT_PTR	= 26,
	VT_SAFEARRAY	= 27,
	VT_CARRAY	= 28,
	VT_USERDEFINED	= 29,
	VT_LPSTR	= 30,
	VT_LPWSTR	= 31,
	VT_RECORD	= 36,
	VT_INT_PTR	= 37,
	VT_UINT_PTR	= 38,
	VT_FILETIME	= 64,
	VT_BLOB	= 65,
	VT_STREAM	= 66,
	VT_STORAGE	= 67,
	VT_STREAMED_OBJECT	= 68,
	VT_STORED_OBJECT	= 69,
	VT_BLOB_OBJECT	= 70,
	VT_CF	= 71,
	VT_CLSID	= 72,
	VT_VERSIONED_STREAM	= 73,
	VT_BSTR_BLOB	= 0xfff,
	VT_VECTOR	= 0x1000,
	VT_ARRAY	= 0x2000,
	VT_BYREF	= 0x4000,
	VT_RESERVED	= 0x8000,
	VT_ILLEGAL	= 0xffff,
	VT_ILLEGALMASKED	= 
	*********************************************************/
	switch( va->vt )
	{ 
	case VT_BSTR:
		{
			_bstr_t _text; 
			_text = va->bstrVal; 
			if( _text.length() > 0 )
			{
				ASSERT( NULL != _text.operator const wchar_t*() ); 
				text = _text.operator const wchar_t*(); 
			}
		}
		break; 
	case VT_BSTR | VT_BYREF:

		{
			_bstr_t _text; 
			if( NULL == va->pbstrVal)
			{
				break; 
			}

			_text = *va->pbstrVal; 
			if( _text.length() > 0 )
			{
				ASSERT( NULL != _text.operator const wchar_t*() ); 
				text = _text.operator const wchar_t*(); 
			}
		}
		break; 

	case VT_I4:
		{
			WCHAR _text[ MAX_PATH ]; 

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%d", va->lVal ); 
			text = _text; 
		}
		break; 
	case VT_I4 | VT_BYREF:
		{
			WCHAR _text[ MAX_PATH ]; 

			if( NULL == va->plVal )
			{
				break; 
			}

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%d", *va->plVal ); 
			text = _text; 
		}
		break; 
	case VT_R8:
		{
			WCHAR _text[ MAX_PATH ]; 

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%f", va->dblVal ); 
			text = _text; 
		}
		break; 
	case VT_R8 | VT_BYREF:
		{
			WCHAR _text[ MAX_PATH ]; 

			if( NULL == va->pdblVal )
			{
				break; 
			}

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%f", *va->pdblVal ); 
			text = _text; 
		}
		break; 
	case VT_DISPATCH:
		{
			WCHAR _text[ MAX_PATH ]; 

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%p", va->pdispVal); 
			text = _text; 
		}
		break; 
	case VT_DISPATCH | VT_BYREF:
		{
			WCHAR _text[ MAX_PATH ]; 

			if( NULL == va->ppdispVal )
			{
				break; 
			}

			StringCchPrintf( _text, ARRAYSIZE( _text ), L"%p", *va->ppdispVal); 
			text = _text; 
		}
		break; 

	default:
		//ASSERT(FALSE); // unknown VARIANT type (this ASSERT is optional)
		break; 
	} /* vt */

	return text; 
}

HTML_INFO_LEVEL WINAPI get_html_info_level_from_text( LPCWSTR text )
{
	HTML_INFO_LEVEL level = MAX_HTML_INFO_LEVEL; 

	do 
	{
		if( NULL != wcsistr( text, L"http://" ) )
		{
			level = HTML_PAGE; 
			break; 
		}

		if( NULL != wcsistr( text, L"<" ) )
		{
			level = HTML_ELEMENT; 
			break; 
		}

	} while ( FALSE ); 

	return level; 
}

LRESULT WINAPI split_string( LPCWSTR szBuffer, 
							ULONG nBufferLen, 
							LPCWSTR szSplitCharSet, 
							STRING_POINTER *pMyString,
							ULONG nSize, 
							ULONG *ret_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG nCount = 0;
	ULONG nElementCount = 0; 

	while (szBuffer[nCount]!='\0' && nCount<nBufferLen)
	{
		while(wcschr(szSplitCharSet,szBuffer[nCount]))
			nCount++;

		if(nCount>=nBufferLen)
			break;

		pMyString[nElementCount]._string= ( LPWSTR )&szBuffer[nCount];
		pMyString[nElementCount].length = 0;


		while(!wcschr(szSplitCharSet,szBuffer[nCount]))
		{
			pMyString[nElementCount].length ++;
			nCount++;
			if(nCount>=nBufferLen)
				break;
		}


		nElementCount++;
		if(nElementCount>=nSize)
		{
			break; 
		}
	}

	*ret_count = nElementCount; 
	
	return ret; 
}

LRESULT WINAPI get_app_path( LPWSTR FileName, 
							ULONG ccBufferLength, 
							ULONG *ccReturnLength )
{
	INT32 Ret = ERROR_SUCCESS; 
	ULONG _Ret; 
	LPWSTR PathDelim; 

	if( FileName == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ccBufferLength == 0 )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( ccReturnLength == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	*ccReturnLength = 0; 

	_Ret = GetModuleFileName( NULL, FileName, ccBufferLength ); 
	if( _Ret >= ccBufferLength )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	else if( _Ret == 0 )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim = wcsrchr( FileName, PATH_DELIM_CH ); 

	if( PathDelim == NULL )
	{
		Ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	if( ( ULONG )( PathDelim - FileName ) + 2 > ccBufferLength )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim[ 1 ] = L'\0'; 

	*ccReturnLength = PathDelim - FileName + 1; 

_return: 
	return Ret; 
}

LRESULT WINAPI get_app_path_a( LPSTR file_name, 
							ULONG cc_buf_len, 
							ULONG *cc_ret_len )
{
	INT32 Ret = ERROR_SUCCESS; 
	ULONG _Ret; 
	LPSTR PathDelim; 

	if( file_name == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( cc_buf_len == 0 )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	if( cc_ret_len == NULL )
	{
		Ret = ERROR_INVALID_PARAMETER; 
		goto _return; 
	}

	*cc_ret_len = 0; 

	_Ret = GetModuleFileNameA( NULL, file_name, cc_buf_len ); 
	if( _Ret >= cc_buf_len )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}
	else if( _Ret == 0 )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim = strrchr( file_name, PATH_DELIM_CH_A ); 

	if( PathDelim == NULL )
	{
		Ret = ERROR_NOT_FOUND; 
		goto _return; 
	}

	if( ( ULONG )( PathDelim - file_name ) + 2 > cc_buf_len )
	{
		Ret = ERROR_ERRORS_ENCOUNTERED; 
		goto _return; 
	}

	PathDelim[ 1 ] = '\0'; 

	*cc_ret_len = PathDelim - file_name + 1; 

_return: 
	return Ret; 
}

inline bool IsOldWindows()
{
	BOOL bRet; 
	OSVERSIONINFO ovi = { 0 };

	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bRet = ::GetVersionEx(&ovi);

	return (!bRet || !((ovi.dwMajorVersion >= 5) || (ovi.dwMajorVersion == 4 && ovi.dwMinorVersion >= 90)));
}

LRESULT WINAPI open_file_dlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
							 LPCTSTR lpszDefExt, 
							 LPCTSTR lpszFileName,
							 LPCTSTR lpszFileTitle, 
							 LPCTSTR lpszInitialDir, 
							 LPCTSTR lpszFilter,
							 HWND hWndParent, 
							 DWORD flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	HWND hTmpWnd; 
	OPENFILENAME m_ofn;
	memset(&m_ofn, 0, sizeof(m_ofn)); // initialize structure to 0/NULL
	m_ofn.lStructSize = sizeof(m_ofn);
#if (_WIN32_WINNT >= 0x0500)
	// adjust struct size if running on older version of Windows
	if(IsOldWindows())
	{
		ASSERT(sizeof(m_ofn) > OPENFILENAME_SIZE_VERSION_400);   // must be
		m_ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	}
#endif //(_WIN32_WINNT >= 0x0500)

	m_ofn.lpstrFile = (LPTSTR)lpszFileName;
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrDefExt = lpszDefExt;
	m_ofn.lpstrFileTitle = (LPTSTR)lpszFileTitle;
	m_ofn.nMaxFileTitle = _MAX_FNAME; 
	m_ofn.lpstrInitialDir = lpszInitialDir; 
	//m_ofn.lpTemplateName

#ifndef _WIN32_WCE
	m_ofn.Flags = flags | OFN_EXPLORER | OFN_ENABLESIZING;
#else // CE specific
	m_ofn.Flags = flags | OFN_EXPLORER;
#endif //!_WIN32_WCE
	m_ofn.lpstrFilter = lpszFilter;

	m_ofn.hInstance = NULL;

	hTmpWnd = ::GetActiveWindow();
	m_ofn.hwndOwner = hWndParent ? hWndParent : hTmpWnd;

	if(bOpenFileDialog)
	{
		_ret = ::GetOpenFileName(&m_ofn);
	}
	else
	{
		_ret = ::GetSaveFileName(&m_ofn);
	}

	if( _ret == FALSE )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

	return ret;
}

/*************************************************************************
key html element:
<input class="btn" name="regsubmit" type="submit" value="同 意">
<a href="login.php" style="color:#c3e4f5">登录</a>
*************************************************************************/

LRESULT WINAPI release_html_element_infos(HTML_ELEMENT_INFOS *infos)
{
    LRESULT ret = ERROR_SUCCESS;
    HTML_ELEMENT_INFO_ITERATOR it;

    do
    {
        ASSERT(NULL != infos);

        for (it = infos->begin();
            it != infos->end();
            it++)
        {
            if (NULL != (*it))
            {
                delete (*it);
            }
        }

        infos->clear(); 
    } while (FALSE);

    return ret;
}

HRESULT WINAPI release_html_element_group(HTML_ELEMENT_GROUP *group)
{
    HRESULT hr = S_OK;

    do
    {
        for (HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group->begin();
            it != group->end();
            it++)
        {
            (*it)->Release();
        }

        group->clear();
    } while (FALSE);

    return hr;
}

LRESULT WINAPI release_html_page_info( HTML_PAGE_INFO *page )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_INFO_ITERATOR it; 
	ASSERT( NULL != page ); 

	do 
	{
		for( it = page->elements.begin(); 
			it != page->elements.end(); 
			it ++ )
		{
			if( NULL != ( *it ) ) 
			{
				delete ( *it ); 
			}
		}

		page->elements.clear(); 

		//for( it = page->action_elements.begin(); 
		//	it != page->action_elements.end(); 
		//	it ++ )
		//{
		//	if( NULL != ( *it ) ) 
		//	{
		//		delete ( *it ); 
		//	}
		//}

		//page->action_elements.clear(); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_html_web_site_info( HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_html_web_site_info( HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_PAGE_INFOS_ITERATOR it; 
	ASSERT( NULL != site ); 

	do 
	{
		for( it = site->pages.begin(); 
			it != site->pages.end(); 
			it ++ )
		{
			if( NULL != ( *it ) ) 
			{
				release_html_page_info( (*it) ); 
				delete ( *it ); 
			}
		}

		site->pages.clear(); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_html_web_sites_info( HTML_WEB_SITE_INFOS *sites )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_WEB_SITE_INFO_ITERATOR it; 
	ASSERT( NULL != sites ); 

	do 
	{
		for( it = sites->begin(); 
			it != sites->end(); 
			it ++ )
		{
			if( NULL != ( *it ) ) 
			{
				release_html_web_site_info( (*it) ); 
				delete ( *it ); 
			}
		}

		sites->clear(); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _add_html_element( HTML_ELEMENT_INFO *element_info, 
								HTML_ELEMENT_INFOS *element_infos )
{
	LRESULT ret = ERROR_SUCCESS; 
	size_t size; 

	do 
	{
		ASSERT( NULL != element_info ); 
		ASSERT( NULL != element_infos ); 

		size = element_infos->size(); 

		element_infos->push_back( element_info ); 

		if( element_infos->size() <= size )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI add_html_element( HTML_ELEMENT_INFO *element_info, HTML_PAGE_INFO *page_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	size_t size; 

	do 
	{
		ASSERT( NULL != element_info ); 
		ASSERT( NULL != page_info ); 

		//switch( element_info->info_type )
		//{
		//case HTML_A_ELEMENT:
		//case HTML_BUTTON_ELEMENT:
		//	{
		//		size = page_info->action_elements.size(); 

		//		page_info->action_elements.push_back( element_info ); 

		//		if( page_info->action_elements.size() <= size )
		//		{
		//			ret = ERROR_NOT_ENOUGH_MEMORY; 
		//			break; 
		//		}
		//	}
		//	break; 
		//case HTML_INPUT_ELEMENT:
		//	{
		//		//the html element of input only have 2 sub type:button, text
		//		if( 0 == wcsicmp( HTML_ELEMENT_TAG_BUTTON, element_info->type.c_str() ) )
		//		{
		//			size = page_info->data_elements.size(); 

		//			page_info->data_elements.push_back( element_info ); 

		//			if( page_info->data_elements.size() <= size )
		//			{
		//				ret = ERROR_NOT_ENOUGH_MEMORY; 
		//				break; 
		//			}
		//		}
		//		else
		//		{
		//			size = page_info->data_elements.size(); 

		//			page_info->data_elements.push_back( element_info ); 

		//			if( page_info->data_elements.size() <= size )
		//			{
		//				ret = ERROR_NOT_ENOUGH_MEMORY; 
		//				break; 
		//			}
		//		}

		//	}
		//	break; 
		//default:
		//	break; 
		//}

		//if( ERROR_SUCCESS != ret )
		//{
		//	break; 
		//}

		size = page_info->elements.size(); 

		page_info->elements.push_back( element_info ); 

		if( page_info->elements.size() <= size )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

//LRESULT WINAPI add_html_element_from_value( HTML_ELEMENT_INFO *element_info, HTML_PAGE_INFO *page_info )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HTML_ELEMENT_INFO *_element_info = NULL; 
//	do 
//	{
//		ASSERT( NULL != element_info ); 
//		ASSERT( NULL != page_info ); 
//	
//		_element_info = new HTML_ELEMENT_INFO(); 
//		if( NULL == _element_info )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			break; 
//		}
//
//		_element_info->id = element_info->id; 
//		_element_info->value = element_info->value; 
//		_element_info->type = element_info->type; 
//		
//		{
//			size_t size; 
//			size = page_info->elements.size(); 
//
//			page_info->elements.push_back( _element_info ); 
//		
//			if( page_info->elements.size() <= size )
//			{
//				ret = ERROR_NOT_ENOUGH_MEMORY; 
//				break; 
//			}
//
//			_element_info = NULL; 
//		}
//
//	}while( FALSE );
//	
//	if( NULL != _element_info )
//	{
//		delete _element_info; 
//	}
//
//	return ret; 
//}

LRESULT WINAPI add_html_site( HTML_WEB_SITE_INFO *site, 
							 HTML_WEB_SITE_INFOS *sites )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != site ); 
		ASSERT( NULL != sites ); 

		{
			size_t size; 
			size = sites->size(); 

			sites->push_back( site ); 

			if( sites->size() <= size )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI add_html_page( HTML_PAGE_INFO *page_info, HTML_WEB_SITE_INFO *site )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != page_info ); 
		ASSERT( NULL != site ); 

		{
			size_t size; 
			size = site->pages.size(); 

			site->pages.push_back( page_info ); 

			if( site->pages.size() <= size )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}

	}while( FALSE ); 

	return ret; 
}

/*********************************************************************************************************
<input class="input" type="text" size="20" maxlength="15" name="regname" id="regname">
*********************************************************************************************************/

LRESULT WINAPI load_html_button_element( HTML_ELEMENT_INFO *element_info, MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do 
		{

			hr = _xml_node->get_nodeName( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				//ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				element_info->info_type = UNKNOWN_HTML_ELEMENT; 
			}
			else
			{
				element_info->tag = _temp_text; 
				element_info->info_type = get_html_element_type_from_tag( _temp_text ); 
			}
		}while( FALSE ); 

		//do 
		//{
		//	xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_CLASS ); 
		//	if( xml_attr == NULL )
		//	{ 
		//		break; 
		//	}

		//	ASSERT( xml_attr != NULL ); 
		//	xml_node_value.Clear(); 
		//	hr = xml_attr->get_nodeValue( &xml_node_value ); 
		//	if( FAILED( hr ) )
		//	{
		//		break; 
		//	}

		//	temp_text = ( BSTR )xml_node_value.bstrVal; 

		//	_temp_text = temp_text.operator wchar_t*(); 
		//	if( NULL == _temp_text )
		//	{
		//		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	}
		//	else
		//	{
		//		element_info->_class = _temp_text; 
		//	}
		//}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TYPE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->type = _temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = _temp_text; 
			}
		}while( FALSE ); 

		if(  ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			|| ( 0 == element_info->_class.length() 
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_input_element( HTML_ELEMENT_INFO *element_info, MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//do 
		//{

		//	hr = _xml_node->get_nodeName( temp_text.GetAddress() ); 
		//	if( FAILED( hr ) )
		//	{
		//		//ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	_temp_text = temp_text.operator wchar_t*(); 
		//	if( NULL == _temp_text )
		//	{
		//		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//		element_info->info_type = UNKNOWN_HTML_ELEMENT; 
		//	}
		//	else
		//	{
		//		element_info->tag = _temp_text; 
		//		element_info->info_type = get_info_type( _temp_text ); 
		//	}
		//}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_CLASS ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->_class = _temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TYPE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->type = _temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = _temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_VALUE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->value = _temp_text; 
			}
		}while( FALSE ); 

		if(  ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			|| ( 0 == element_info->_class.length() 
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_a_element( HTML_ELEMENT_INFO *element_info, 
								   MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_HREF ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				if( *temp_text != L'#' )
				{
					element_info->href = temp_text; 
				}
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONMOUSEOVER ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onmouseover = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCLICK ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onclick = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TITLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->title = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_STYLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->style = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( _temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCHANGE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onchange = temp_text; 
			}
		}while( FALSE ); 

		if( ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_text_area_element( HTML_ELEMENT_INFO *element_info, 
									  MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		//do 
		//{
		//	xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_STYLE ); 
		//	if( xml_attr == NULL )
		//	{ 
		//		break; 
		//	}

		//	ASSERT( xml_attr != NULL ); 
		//	xml_node_value.Clear(); 
		//	hr = xml_attr->get_nodeValue( &xml_node_value ); 
		//	if( FAILED( hr ) )
		//	{
		//		break; 
		//	}

		//	_temp_text = ( BSTR )xml_node_value.bstrVal; 

		//	temp_text = _temp_text.operator wchar_t*(); 
		//	if( NULL == temp_text )
		//	{
		//		ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	}
		//	else
		//	{
		//		element_info->style = temp_text; 
		//	}
		//}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( _temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = temp_text; 
			}
		}while( FALSE ); 

		if( ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_element_common( HTML_ELEMENT_INFO *element_info, 
										MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_XPATH ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->xpath = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_SLEEP_TIME ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			//xml_node_value.vt = VT_I1; 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				//default is search all node
				element_info->sleep_time = 0; 
			}
			else
			{
#define MAX_HTML_ELEMENT_OPERATION_SLEEP_TIME 50000
				LPWSTR text_end; 
				element_info->sleep_time = wcstoul( _temp_text, &text_end, 0 ); 
				if( element_info->sleep_time > MAX_HTML_ELEMENT_OPERATION_SLEEP_TIME )
				{
					element_info->sleep_time = MAX_HTML_ELEMENT_OPERATION_SLEEP_TIME; 
				}

			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_IN_FRAME ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			//xml_node_value.vt = VT_I1; 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				//default is search all node
				element_info->in_frame = TRUE; //FALSE
			}
			else
			{
				if( *_temp_text == L'1' )
				{
					element_info->in_frame = TRUE; 
				}
				else
				{
					element_info->in_frame = FALSE; 
				}
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_NAME ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->name = _temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_ID ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->id = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_INPUT_VALUE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->input_text = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( INPUT_TEXT_NAME_ATTRIBUTE_NAME ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content_name = temp_text; 
			}
		}while( FALSE ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_body_element( HTML_ELEMENT_INFO *element_info, 
								   MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_BODY_EDITABLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content_editable = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_BODY_SPELLCHECK ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->spell_check = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TAB_INDEX ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->spell_check = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_HREF ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->href = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONMOUSEOVER ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onmouseover = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCLICK ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onclick = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TITLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->title = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_STYLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->style = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( _temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCHANGE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onchange = temp_text; 
			}
		}while( FALSE ); 

		if( ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_div_element( HTML_ELEMENT_INFO *element_info, 
									  MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 
		
		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_BODY_EDITABLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content_editable = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_BODY_SPELLCHECK ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->spell_check = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TAB_INDEX ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->spell_check = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_HREF ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->href = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONMOUSEOVER ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onmouseover = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCLICK ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onclick = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_TITLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->title = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_STYLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->style = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( _temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCHANGE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onchange = temp_text; 
			}
		}while( FALSE ); 

		if( ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI load_html_select_element( HTML_ELEMENT_INFO *element_info, MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

	do 
	{
		ASSERT( element_info != NULL ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_STYLE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->style = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			hr = _xml_node->get_text( _temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->content = temp_text; 
			}
		}while( FALSE ); 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( HTML_EVENT_ONCHANGE ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			xml_node_value.Clear(); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			_temp_text = ( BSTR )xml_node_value.bstrVal; 

			temp_text = _temp_text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				element_info->event_onchange = temp_text; 
			}
		}while( FALSE ); 

		if( ( UNKNOWN_HTML_ELEMENT == element_info->info_type 
			|| 0 == element_info->tag.length() )
			&& 0 == element_info->id.length() 
			&& 0 == element_info->name.length() 
			&& 0 == element_info->value.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_html_element_type( HTML_ELEMENT_INFO *element_info, 
									 MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	
	MSXML::IXMLDOMNodePtr _xml_node; 

	_variant_t xml_node_value; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 

	do 
	{
		ASSERT( NULL != element_info ); 
		ASSERT( NULL != xml_node ); 

		_xml_node = xml_node; 

		hr = _xml_node->get_nodeName( temp_text.GetAddress() ); 
		if( FAILED( hr ) )
		{
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_temp_text = temp_text.operator wchar_t*(); 
		if( NULL == _temp_text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			element_info->info_type = UNKNOWN_HTML_ELEMENT; 
		}
		else
		{
			element_info->tag = _temp_text; 
			element_info->info_type = get_html_element_type_from_tag( _temp_text ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_html_element( HTML_ELEMENT_INFO *element_info, 
								MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 

	MSXML::IXMLDOMNodePtr _xml_node; 

	_variant_t xml_node_value; 
	_bstr_t temp_text; 
	//LPCWSTR _temp_text; 

	do 
	{
		ASSERT( NULL != element_info ); 
		ASSERT( NULL != xml_node ); 

		_xml_node = xml_node; 

		ret = get_html_element_type( element_info, xml_node ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = load_html_element_common( element_info, xml_node ); 
		if( ret != ERROR_SUCCESS )
		{

		}

		switch( element_info->info_type )
		{
		case HTML_INPUT_ELEMENT: 
			ret = load_html_input_element( element_info, xml_node ); 
			break; 
		case HTML_BUTTON_ELEMENT: 
			ret = load_html_button_element( element_info, xml_node ); 
			break; 
		case HTML_A_ELEMENT: 
			ret = load_html_a_element( element_info, xml_node ); 
			break; 
		case HTML_BODY_ELEMENT: 
			ret = load_html_body_element( element_info, xml_node ); 
			break; 
		case HTML_DIV_ELEMENT:
			ret = load_html_div_element( element_info, xml_node ); 
			break; 
		case HTML_TEXT_AREA_ELEMENT:
			ret = load_html_text_area_element( element_info, xml_node ); 			
			break; 
		case HTML_SELECT_ELEMENT:
			ret = load_html_select_element( element_info, xml_node ); 
			break; 
		default:
			ASSERT( FALSE ); 
			log_trace_ex( MSG_IMPORTANT, "unknown HTML element: %s\n", element_info->tag.c_str() ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		{
			HRESULT hr; 
			MSXML::IXMLDOMNodePtr _xml_node; 
			_variant_t xml_node_value; 
			_bstr_t _temp_text; 
			LPCWSTR temp_text; 
			MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
			MSXML::IXMLDOMNodePtr xml_attr; 
			//HTML_ELEMENT_ACTION *action = NULL; 

			//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

			do 
			{
				ASSERT( element_info != NULL ); 
				ASSERT( xml_node != NULL ); 

				//action = new HTML_ELEMENT_ACTION(); 
				//if( NULL == action )
				//{
				//	break; 
				//}

				_xml_node = xml_node; 

				hr = _xml_node->get_attributes( &xml_attrs ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				} 

				do 
				{
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_XPATH ); 
					if( xml_attr == NULL )
					{ 
						break; 
					}

					ASSERT( xml_attr != NULL ); 
					xml_node_value.Clear(); 
					hr = xml_attr->get_nodeValue( &xml_node_value ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						element_info->action_config.xpath = temp_text; 
					}
				}while( FALSE ); 

				do 
				{
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_ACTION ); 
					if( xml_attr == NULL )
					{ 
						break; 
					}

					ASSERT( xml_attr != NULL ); 
					xml_node_value.Clear(); 
					hr = xml_attr->get_nodeValue( &xml_node_value ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						element_info->action_config.action = temp_text; 
						//action->cmd = temp_text; 
					}
				}while( FALSE ); 

				do 
				{
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_ACTION_PARAM ); //L"action_param"
					if( xml_attr == NULL )
					{ 
						break; 
					}

					ASSERT( xml_attr != NULL ); 
					xml_node_value.Clear(); 
					hr = xml_attr->get_nodeValue( &xml_node_value ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						element_info->action_config.param = temp_text; 
					}
				}while( FALSE ); 

				do 
				{
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_HTML_TEXT ); 
					if( xml_attr == NULL )
					{ 
						break; 
					}

					ASSERT( xml_attr != NULL ); 
					xml_node_value.Clear(); 
					hr = xml_attr->get_nodeValue( &xml_node_value ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						element_info->action_config.html_text = temp_text; 
					}
				}while( FALSE ); 

				do 
				{
					element_info->action_config.in_frame = FALSE; 
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_IN_FRAME ); 
					if( xml_attr == NULL )
					{ 
						break; 
					}

					ASSERT( xml_attr != NULL ); 
					xml_node_value.Clear(); 
					hr = xml_attr->get_nodeValue( &xml_node_value ); 
					if( FAILED( hr ) )
					{
						break; 
					}

					if( xml_node_value.vt != VT_I4 )
					{
						break; 
					}

					element_info->action_config.in_frame = xml_node_value.intVal; 
				}while( FALSE ); 

				//element_info->actions.push_back( action ); 
				//action = NULL; 
			}while( FALSE ); 

			//if( NULL != action)
			//{
			//	free( action ); 
			//}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI _load_html_page_info( HTML_PAGE_INFO *html_page, LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	MSXML::IXMLDOMNodePtr elements_node = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMNodePtr xml_node = NULL; 
	VARIANT_BOOL __ret; 
	//MSXML::IXMLDOMNamedNodeMapPtr xml_attrs = NULL; 
	//MSXML::IXMLDOMNodePtr xml_attr = NULL; 

	HTML_PAGE_INFO *page_info = NULL; 
	HTML_ELEMENT_INFO *element_info = NULL; 

	LONG i; 
	LONG node_count; 
	//LONG original_node_count; 
	WCHAR *temp_text; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	//_bstr_t _temp_text; 

	do 
	{
		ASSERT( NULL != html_page ); 
		ASSERT( NULL != file_name ); 

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		if( file_name == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = CoCreateInstance( __uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( PVOID* )&xml_doc ); 

		if( hr != S_OK ) 
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "CoCreateInstance error\n" ); 
			break; 
		}

		__ret = xml_doc->load( ( WCHAR* )file_name ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_doc->parseError;

			bstrReason = spParseError->reason;

			temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", temp_text ); 
			}

			break; 
		}

		xml_element = xml_doc->documentElement; 
		if( xml_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		{
			_bstr_t element_path; 
			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			elements_node = xml_element->selectSingleNode( element_path ); 
		}

		if( NULL == elements_node )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = xml_element->get_childNodes( &xml_node_list ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( xml_node_list == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		//node_count = xml_node_list->Getlength(); 

		//xml_node_list = _xml_node->selectNodes( L"*" );
		//if( xml_node_list == NULL )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	log_trace_ex( MSG_IMPORTANT, "%s: //site/page/element is not existed", __FUNCTION__ );
		//	break; 
		//}

		//hr = xml_node_list->get_length( &node_count ); 
		//if( FAILED( hr ) )
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				element_info = new HTML_ELEMENT_INFO(); 
				if( NULL == element_info )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				hr = xml_node_list->get_item( i, &xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = get_html_element( element_info, xml_node ); 
				if( ret != ERROR_SUCCESS )
				{
					delete element_info; 
					element_info = NULL; 
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ret = add_html_element( element_info, html_page ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					delete element_info; 
					element_info = NULL; 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

			}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
			tick_end = GetTickCount(); 
			dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

			//if( 0 == i 
			//	&& 0 == site_info->site_url.length() )
			//{
			//	ret = ERROR_INVALID_PARAMETER; 
			//	break; 
			//}

			//_ret = add_html_site( page_info, html_sites ); 
			//if( _ret == ERROR_SUCCESS )
			//{
			//	site_info = NULL; 
			//}
			//else
			//{
			//	ASSERT( FALSE ); 
			//}
		}

		if( node_count != html_page->elements.size() ) //+ html_page->action_elements.size(
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//if( page_info != NULL )
		//{
		//	delete page_info; 
		//}
		//if( node_count != html_page->size() - original_node_count )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

	}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
	tick_end = GetTickCount(); 
	dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

	if( NULL != page_info )
	{
		delete page_info; 
	}

	if( ret != ERROR_SUCCESS )
	{
		release_html_page_info( html_page ); 
	}

	return ret; 
}

LRESULT WINAPI load_html_page_info( HTML_PAGE_INFO *html_page, MSXML::IXMLDOMNode *xml_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	MSXML::IXMLDOMNodeListPtr xml_node_list; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	HTML_ELEMENT_INFO *element_info = NULL; 

	LONG i; 
	LONG node_count; 
	WCHAR *_temp_text; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	_bstr_t temp_text; 
	_variant_t xml_node_value; 

	do 
	{
		ASSERT( NULL != html_page ); 
		ASSERT( xml_node != NULL ); 

		_xml_node = xml_node; 

		//<?xml version="1.0" encoding="UTF-8" ?>
		// <site url="http://baidu.com" >
		//  <page url="http://baidu.com">
		//		<INPUT ="post" name="" .../>
		//		<BUTTON />
		//	</page>
		// </site>

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		//if( xml_node == NULL )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		do 
		{
			hr = _xml_node->get_attributes( &xml_attrs ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			xml_attr = xml_attrs->getNamedItem( HTML_PAGE_URL ); 
			if( xml_attr == NULL )
			{ 
				break; 
			}

			ASSERT( xml_attr != NULL ); 
			hr = xml_attr->get_nodeValue( &xml_node_value ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			temp_text = ( BSTR )xml_node_value.bstrVal; 

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
			else
			{
				html_page->url = temp_text; 
			}
		}while( FALSE ); 

		//if( 0 == html_page->url.length() )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		/*******************************************
		选取未知节点
		XPath 通配符可用来选取未知的 XML 元素。
		通配符	描述
		*	匹配任何元素节点。
		@*	匹配任何属性节点。
		node()	匹配任何类型的节点。
		实例
		
		在下面的表格中，我们列出了一些路径表达式，以及这些表达式的结果：
		路径表达式	结果
		/bookstore/*	选取 bookstore 元素的所有子元素。
		//*	选取文档中的所有元素。
		//title[@*]	选取所有带有属性的 title 元素。

		//book/title | //book/price	选取 book 元素的所有 title 和 price 元素。
		//title | //price	选取文档中的所有 title 和 price 元素。
		/bookstore/book/title | //price	选取属于 bookstore 元素的 book 元素的所有 title 元素，以及文档中所有的 price 元素。
		*******************************************/

		xml_node_list = _xml_node->selectNodes( L"*" );
		if( xml_node_list == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //site/page/element is not existed", __FUNCTION__ );
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				element_info = new HTML_ELEMENT_INFO(); 
				if( NULL == element_info )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				hr = xml_node_list->get_item( i, &_xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = get_html_element( element_info, _xml_node ); 
				if( ret != ERROR_SUCCESS )
				{
					delete element_info; 
					element_info = NULL; 
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ret = add_html_element( element_info, html_page ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					delete element_info; 
					element_info = NULL; 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

			}while( FALSE ); 

			//_xml_node->Release(); release auto
		}

		if( node_count != html_page->elements.size() ) //+ html_page->action_elements.size(
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
	tick_end = GetTickCount(); 
	dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

	if( ret != ERROR_SUCCESS )
	{
		release_html_page_info( html_page ); 
	}

	return ret; 
}

LRESULT WINAPI load_html_site_info( HTML_WEB_SITE_INFO *html_site, 
								   MSXML::IXMLDOMNode *xml_node ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HRESULT hr; 

	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMNodePtr _xml_node = NULL; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr = NULL; 

	//VARIANT_BOOL __ret; 

	HTML_PAGE_INFO *page_info = NULL; 

	_bstr_t text; 
	LPCWSTR _text; 

	LONG i; 
	LONG node_count; 
	//WCHAR *temp_text; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	//_bstr_t _temp_text; 

	do 
	{
		ASSERT( NULL != html_site ); 
		ASSERT( xml_node != NULL ); 

		//<?xml version="1.0" encoding="UTF-8" ?>
		// <site url="http://baidu.com" >
		//  <page url="http://baidu.com">
		//		<element id="post" name="" .../>
		//	</page>
		// </site>
		// <site url="http://baidu.com" >
		//  <page url="http://baidu.com">
		//		<element id="post" name="" .../>
		//	</page>
		// </site>

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		if( xml_node == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_xml_node = xml_node; 

		do 
		{
			_variant_t attr_value; 

			hr = _xml_node->get_attributes( &xml_attrs ); 
			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == xml_attrs )
			{
				break; 
			}

			text = L"url"; 
			xml_attr = xml_attrs->getNamedItem( text ); 
			if( NULL == xml_attr )
			{
				break; 
			}

			hr = xml_attr->get_nodeValue( attr_value.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			text = ( BSTR )attr_value.bstrVal; 

			_text = text.operator wchar_t*(); 
			if( NULL == _text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				html_site->site_url = _text; 
			}

			text = L"desc"; 
			xml_attr = xml_attrs->getNamedItem( text ); 
			if( NULL == xml_attr )
			{
				break; 
			}

			hr = xml_attr->get_nodeValue( attr_value.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			text = ( BSTR )attr_value.bstrVal; 

			_text = text.operator wchar_t*(); 
			if( NULL == _text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
			}
			else
			{
				html_site->site_desc = _text; 
			}
		}while( FALSE );

		xml_node_list = _xml_node->selectNodes( L"page" ); 
		if( xml_node_list == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		//node_count = xml_node_list->Getlength(); 

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				if( page_info == NULL )
				{
					page_info = new HTML_PAGE_INFO(); 
					if( NULL == page_info )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}
				}

				hr = xml_node_list->get_item( i, &_xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = load_html_page_info( page_info, _xml_node ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( 0 == i 
					&& 0 == page_info->url.length() )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				_ret = add_html_page( page_info, html_site ); 
				if( _ret == ERROR_SUCCESS )
				{
					page_info = NULL; 
				}
				else
				{
					ASSERT( FALSE ); 
				}

			}while( FALSE ); 

			//if( page_info != NULL )
			//{
			//	delete page_info; 
			//}
		}

		if( node_count != html_site->pages.size() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
	tick_end = GetTickCount(); 
	dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

	if( NULL != page_info )
	{
		delete page_info; 
	}

	if( ret != ERROR_SUCCESS )
	{
		release_html_web_site_info( html_site ); 
	}

	return ret; 
}

LRESULT WINAPI load_html_site_infos( HTML_WEB_SITE_INFOS *html_sites, LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMNodePtr xml_node = NULL; 
	VARIANT_BOOL __ret; 
	//MSXML::IXMLDOMNamedNodeMapPtr xml_attrs = NULL; 
	//MSXML::IXMLDOMNodePtr xml_attr = NULL; 

	HTML_WEB_SITE_INFO *site_info = NULL; 

	LONG i; 
	LONG node_count; 
	LONG original_node_count; 
	WCHAR *temp_text; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	//_bstr_t _temp_text; 

	do 
	{
		ASSERT( NULL != html_sites ); 
		ASSERT( file_name != NULL ); 

		original_node_count = html_sites->size(); 

		//<?xml version="1.0" encoding="UTF-8" ?>
		// <site url="http://baidu.com" >
		//  <page url="http://baidu.com">
		//		<element id="post" name="" .../>
		//	</page>
		// </site>

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		if( file_name == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = CoCreateInstance( __uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( PVOID* )&xml_doc ); 

		if( hr != S_OK ) 
		{
			dbg_print( DBG_MSG_AND_ERROR_OUT, "CoCreateInstance error\n" ); 
			break; 
		}

		__ret = xml_doc->load( ( WCHAR* )file_name ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_doc->parseError;

			bstrReason = spParseError->reason;

			temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", temp_text ); 
			}

			break; 		
		}

		xml_element = xml_doc->documentElement; 
		if( xml_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		xml_node_list = xml_element->selectNodes( L"//site" ); 
		if( xml_node_list == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: //page/element is not existed", __FUNCTION__ ); 
			break; 
		}

		//node_count = xml_node_list->Getlength(); 

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				if( site_info == NULL )
				{
					site_info = new HTML_WEB_SITE_INFO(); 
					if( NULL == site_info )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}
				}

				hr = xml_node_list->get_item( i, &xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = load_html_site_info( site_info, xml_node ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				//if( 0 == i 
				//	&& 0 == site_info->site_url.length() )
				//{
				//	ret = ERROR_INVALID_PARAMETER; 
				//	break; 
				//}

				_ret = add_html_site( site_info, html_sites ); 
				if( _ret == ERROR_SUCCESS )
				{
					site_info = NULL; 
				}
				else
				{
					ASSERT( FALSE ); 
				}

			}while( FALSE ); 

			//if( page_info != NULL )
			//{
			//	delete page_info; 
			//}
		}

		if( node_count != html_sites->size() - original_node_count )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
	tick_end = GetTickCount(); 
	dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

	if( NULL != site_info )
	{
		delete site_info; 
	}

	if( ret != ERROR_SUCCESS )
	{
		release_html_web_sites_info( html_sites ); 
	}

	return ret; 
}

/*
LRESULT WINAPI get_domain_name_in_url( LPCWSTR url, wstring &domain_name )
{
LRESULT ret = ERROR_SUCCESS; 
LPCWSTR domain_name_begin; 
LPCWSTR domain_name_end; 

do 
{
ASSERT( NULL != url ); 
//ASSERT( NULL != domain_name ); 

#define HTTP_PROTOCAL_URL_PREFIX L"http://" 
#define HTTPS_PROTOCAL_URL_PREFIX L"https://" 
#define HTTP_URL_DELIM L'/'

if( 0 != _wcsnicmp( url, 
HTTP_PROTOCAL_URL_PREFIX, 
CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ) ) )
{
if( 0 != _wcsnicmp( url, 
HTTPS_PROTOCAL_URL_PREFIX, 
CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ) ) )
{
ret = ERROR_PATH_NOT_FOUND; 
break; 
}
else
{
domain_name_begin = url + CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ); 
}
}
else
{
domain_name_begin = url + CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ); 
}

domain_name_end = wcschr( domain_name_begin, HTTP_URL_DELIM ); 

if( NULL == domain_name_end )
{
domain_name = domain_name_begin; 
}
else
{
domain_name = domain_name_begin; 
domain_name = domain_name.substr( 0, domain_name_end - domain_name_begin ); 
}
}while( FALSE );

return ret; 
}
*/
LRESULT WINAPI get_domain_name_in_url( LPCWSTR url, wstring &domain_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR domain_name_begin; 
	LPCWSTR domain_name_end; 

	do 
	{
		ASSERT( NULL != url ); 
		//ASSERT( NULL != domain_name ); 

#define HTTP_PROTOCAL_URL_PREFIX L"http://" 
#define HTTPS_PROTOCAL_URL_PREFIX L"https://" 
#define HTTP_URL_DELIM L'/'

		if( 0 != _wcsnicmp( url, 
			HTTP_PROTOCAL_URL_PREFIX, 
			CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ) ) )
		{
			if( 0 != _wcsnicmp( url, 
				HTTPS_PROTOCAL_URL_PREFIX, 
				CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ) ) )
			{
				ret = ERROR_PATH_NOT_FOUND; 
				break; 
			}
			else
			{
				domain_name_begin = url + CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ); 
			}
		}
		else
		{
			domain_name_begin = url + CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ); 
		}

		domain_name_end = wcschr( domain_name_begin, HTTP_URL_DELIM ); 

		if( NULL == domain_name_end )
		{
			domain_name = domain_name_begin; 
		}
		else
		{
            domain_name = domain_name_begin;
			domain_name = domain_name.substr( 0, domain_name_end - domain_name_begin );
		}

        ASSERT(domain_name.size() > 0); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_folder_name_from_url( LPCWSTR url, CStringW &file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name_char[] = { L'_', L'-', L'.', L',' }; 
	LONG cc_url_len; 
	LONG i; 
	BOOLEAN char_found = FALSE; 
	LONG j; 
	wstring domain_name; 

	do 
	{
		file_name = L""; 

		//ret = get_domain_name_in_url( url, domain_name ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		cc_url_len = wcslen( url ); 
		if( cc_url_len > 30 )
		{
			cc_url_len = 30; 
		}

		for( i = 0; i < cc_url_len; i ++ )
		{
			if( url[ i ] >= 'A' && url[ i ] <= 'Z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= 'a' && url[ i ] <= 'z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= '0' && url[ i ] <= '9' )
			{
				file_name += url[ i ]; 
			}
			else
			{
				char_found = FALSE; 
				for( j = 0; j < ARRAYSIZE( file_name_char ); j ++ )
				{
					if( file_name_char[ j ] == url[ i ] )
					{
						char_found = TRUE; 
						break; 
					}
				}

				if( char_found == TRUE )
				{
					file_name += url[ i ]; 
				}
				else
				{
					file_name += L'_'; 
				}
			}
		}
	}while( FALSE );

	return ret; 
}
