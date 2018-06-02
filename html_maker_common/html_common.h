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

#ifndef __HTML_COMMON_H__
#define __HTML_COMMON_H__

#pragma warning(push)
#pragma warning(disable:4995)
#include <vector>
#pragma warning(pop)
#include <string> 
#include <mshtml.h>
#include <mshtmhst.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#include "html_action.h"

#define MAX_DIGIT_LEN 32

#define ERROR_EXCEPTION_IN_SCRIPT_HANDLER 0xe0901001
#define ERROR_WEB_PAGE_IS_NOT_LOADED 0xe0901002

using namespace std; 

#define MAX_NUMBER_TEXT_LEN 32

#define SAFE_RELEASE_IF( __interface__ ) if( NULL != ( __interface__ ) ) __interface__->Release(); __interface__ = NULL; 

#define REMOTE_ERROR_DEBUG() //for( ; ; ) { dbg_print( MSG_FATAL_ERROR, "fatal error %s:%u\n", __FUNCTION__, __LINE__ ); Sleep( 10 ); } 

#define XML_ELEMENT_END_CHAR L'>' 

#define ERROR_GOTO_NEXT_WEB_PAGE 0xE1920001

#define INPUT_CONTENT_ELEMENT_TAG L"content"
#define INPUT_CONTENT_ELEMENT_NAME_ATTRIBUTE L"name"

#define HTML_ELEMENT_ATTRIBUTE_HREF L"href"
#define HTML_ELEMENT_ATTRIBUTE_NAME L"name" 
#define HTML_ELEMENT_ATTRIBUTE_STYLE L"style"
#define HTML_ELEMENT_ATTRIBUTE_TYPE L"type" 
#define HTML_ELEMENT_ATTRIBUTE_CLASS L"class"
#define HTML_ELEMENT_ATTRIBUTE_VALUE L"value"
#define HTML_ELEMENT_ATTRIBUTE_TITLE L"title"
#define HTML_ELEMENT_ATTRIBUTE_INITIALIZED L"initialized"
#define HTML_ELEMENT_ATTRIBUTE_BODY_EDITABLE L"contenteditable"
#define HTML_ELEMENT_ATTRIBUTE_BODY_SPELLCHECK L"spellcheck"
#define HTML_ELEMENT_ATTRIBUTE_TAB_INDEX L"tabindex"
#define HTML_ELEMENT_ATTRIBUTE_XPATH L"xpath"
#define HTML_ELEMENT_ATTRIBUTE_IN_FRAME L"in_frame"
#define HTML_ELEMENT_ATTRIBUTE_ID L"id"
#define HTML_ELEMENT_ATTRIBUTE_JUMP_TO_ID L"jump_to_id"
#define HTML_ELEMENT_ATTRIBUTE_SLEEP_TIME L"sleep_time"
#define HTML_ELEMENT_ATTRIBUTE_HTML_TEXT L"html_text"
#define HTML_ELEMENT_ATTRIBUTE_ACTION_PARAM L"param"
#define HTML_ELEMENT_ATTRIBUTE_ACTION L"action"
#define HTML_ELEMENT_ATTRIBUTE_URL L"url"

#define HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOOP_COUNT L"loop_count"
#define HTML_SCRIPT_INSTANCE_ATTRIBUTE_BEGIN_URL L"begin_url"
#define HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOCATE_URL L"locate_url"
#define HTML_SCRIPT_INSTANCE_ATTRIBUTE_FLAGS L"flags"

#define HTML_SCRIPT_ATTRIBUTE_LAST_ACTION_ID L"LAST_ACTION_ID"

#define HTML_SCRIPT_ACTION_ELEMENT_NAME L"ACTION"
#define HTML_SCRIPT_INSTANCE_ELEMENT_NAME L"INSTANCE"
#define HTML_SCRIPT_ROOT_ELEMENT_PATH L"//HTML_SCRIPT" //L"//HTML_SITES" 
#define HTML_SCRIPT_ROOT_ELEMENT_NAME L"HTML_SCRIPT" //L"//HTML_SITES" 

#define HTML_ELEMENT_ATTRIBUTE_INPUT_VALUE L"input_text"
//#define HTML_ELEMENT_INPUT_CONTENT_NAME L"content_name"

#define HTML_EVENT_ONCHANGE L"onchange"
#define HTML_EVENT_ONKEYUP L"onkeyup"
#define HTML_EVENT_ONMOUSEOVER L"onmouseover"
#define HTML_EVENT_ONCLICK L"onclick"

#define HTML_PAGE_URL L"url"

typedef enum _HTML_ELEMENT_TYPE
{
	UNKNOWN_HTML_ELEMENT = 0, 
	HTML_INPUT_ELEMENT, 
	HTML_BUTTON_ELEMENT, 
	HTML_A_ELEMENT, 
	HTML_BODY_ELEMENT, 
	HTML_DIV_ELEMENT, 
	HTML_TEXT_AREA_ELEMENT, 
	HTML_SELECT_ELEMENT, 
	MAX_HTML_ELEMENT, 
} HTML_ELEMENT_TYPE, *PHTML_ELEMENT_TYPE; 

INLINE BOOLEAN WINAPI is_input_html_element( HTML_ELEMENT_TYPE element_type )
{
	BOOLEAN ret = FALSE;	

	do 
	{
		switch( element_type )
		{
		case HTML_INPUT_ELEMENT: 
		case HTML_BODY_ELEMENT: 
		case HTML_DIV_ELEMENT: 
		case HTML_TEXT_AREA_ELEMENT: 
			break; 
		case HTML_BUTTON_ELEMENT: 
		case HTML_A_ELEMENT: 
		case HTML_SELECT_ELEMENT: 
			break; 	
		default:
			break; 
		}
	} while ( FALSE );

	return ret; 
}

#define HTML_ELEMENT_TAG_A L"a"
#define HTML_ELEMENT_TAG_INPUT L"input"
#define HTML_ELEMENT_TAG_BUTTON L"button"
#define HTML_ELEMENT_TAG_IMAGE L"img"
#define HTML_ELEMENT_TAG_BODY L"body"
#define HTML_ELEMENT_TAG_DIV L"div"
#define HTML_ELEMENT_TAG_H1 L"h1"
#define HTML_ELEMENT_TAG_H2 L"h2"
#define HTML_ELEMENT_TAG_H3 L"h3"
#define HTML_ELEMENT_TAG_TEXTAREA L"textarea"
#define HTML_ELEMENT_TAG_SELECT L"select"

typedef enum _HTML_INFO_LEVEL
{
	HTML_PAGE, 
	HTML_ELEMENT, 
	MAX_HTML_INFO_LEVEL, 
} HTML_INFO_LEVEL, *PHTML_INFO_LEVEL; 


HTML_INFO_LEVEL WINAPI get_html_info_level_from_text( LPCWSTR text ); 

wstring WINAPI variant2string( VARIANT *va ); 

//#define MAX_HTML_ELEMENT_ID 128
//#define MAX_HTML_ELEMENT_VALUE 128


//LPCWSTR html_element_tags[] = { HTML_ELEMENT_TAG_INPUT, 
//HTML_ELEMENT_TAG_BUTTON, 
//HTML_ELEMENT_TAG_A, 
//HTML_ELEMENT_TAG_BODY, 
//HTML_ELEMENT_TAG_DIV, 
//HTML_ELEMENT_TAG_TEXTAREA, 
//HTML_ELEMENT_TAG_SELECT }; 

#define INLINE __inline 
INLINE HTML_ELEMENT_TYPE get_html_element_type_from_tag( LPCWSTR text )
{
	HTML_ELEMENT_TYPE element_type = UNKNOWN_HTML_ELEMENT; 

	do 
	{
		if( _wcsicmp( HTML_ELEMENT_TAG_INPUT, text ) == 0 )
		{
			element_type = HTML_INPUT_ELEMENT; 
			break; 
		}
		else if( _wcsicmp( HTML_ELEMENT_TAG_BUTTON, text ) == 0 )
		{
			element_type = HTML_BUTTON_ELEMENT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_A, text ) == 0 )
		{
			element_type = HTML_A_ELEMENT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_BODY, text ) == 0 )
		{
			element_type = HTML_BODY_ELEMENT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_TEXTAREA, text ) == 0 )
		{
			element_type = HTML_TEXT_AREA_ELEMENT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_SELECT, text ) == 0 )
		{
			element_type = HTML_SELECT_ELEMENT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_DIV, text ) == 0 )
		{
			element_type = HTML_DIV_ELEMENT; 
			break; 
		}

		//if( wcsicmp( HTML_ELEMENT_TAG_IMAGE, text ) == 0 )
		//{
		//	element_type = HTML_IMAGE_ELEMENT; 
		//	break; 
		//}

	}while( FALSE );

	return element_type; 
}

typedef enum _HTML_ELEMENT_ACTION_TYPE
{
	HTML_ELEMENT_ACTION_TYPE_CLICK = 0, 
	HTML_ELEMENT_ACTION_TYPE_OUTPUT, 
	HTML_ELEMENT_ACTION_TYPE_INPUT, 
	HTML_ELEMENT_ACTION_TYPE_BACK, 
	HTML_ELEMENT_ACTION_TYPE_LOCATE, 
	HTML_ELEMENT_ACTION_TYPE_HOVER, 
	HTML_ELEMENT_ACTION_TYPE_SCRIPT, 
	MAX_HTML_ELEMENT_ACTION_TYPE, 
	UNKNOWN_HTML_ELEMENT_ACTION_TYPE = MAX_HTML_ELEMENT_ACTION_TYPE, 
} HTML_ELEMENT_ACTION_TYPE, *PHTML_ELEMENT_ACTION_TYPE; 
#define HTML_ELEMENT_ACTION_TYPE_JUMP ( UNKNOWN_HTML_ELEMENT_ACTION_TYPE + 10 )
#define HTML_ELEMENT_ACTION_TYPE_NONE ( UNKNOWN_HTML_ELEMENT_ACTION_TYPE + 11 )

INLINE HTML_ELEMENT_ACTION_TYPE get_html_element_default_action_info( IHTMLElement *html_element )
{
	HRESULT hr; 
	HTML_ELEMENT_ACTION_TYPE action_type = UNKNOWN_HTML_ELEMENT_ACTION_TYPE; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 

	do 
	{
		ASSERT( NULL != html_element ); 

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

		if( _wcsicmp( HTML_ELEMENT_TAG_INPUT, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}
		else if( _wcsicmp( HTML_ELEMENT_TAG_BUTTON, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_CLICK; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_A, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 

			hr = html_element->get_innerText( temp_text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				break; 
			}

			if( wcscmp( L"ÏÂÒ»Ò³", _temp_text ) == 0 )
			{
				action_type = HTML_ELEMENT_ACTION_TYPE_CLICK; 
				break; 
			}
			else if( _wcsicmp( L"next", _temp_text ) == 0 )
			{
				action_type = HTML_ELEMENT_ACTION_TYPE_CLICK; 
				break; 
			}

			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_BODY, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_TEXTAREA, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_SELECT, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_CLICK; 
			break; 
		}

		else if( _wcsicmp( HTML_ELEMENT_TAG_DIV, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}
		else if( _wcsicmp( HTML_ELEMENT_TAG_H1, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}
		else if( _wcsicmp( HTML_ELEMENT_TAG_H2, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}
		else if( _wcsicmp( HTML_ELEMENT_TAG_H3, _temp_text ) == 0 )
		{
			action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
			break; 
		}

		//if( wcsicmp( HTML_ELEMENT_TAG_IMAGE, text ) == 0 )
		//{
		//	element_type = HTML_IMAGE_ELEMENT; 
		//	break; 
		//}

	}while( FALSE );

	return action_type; 
}

//#define HTML_ELEMENT_ACTION_GROUPS( _groups_ ) HTML_ELEMENT_GROUP _groups_[ MAX_HTML_ELEMENT_ACTION ]

typedef vector< IHTMLElement* > HTML_ELEMENT_GROUP; 
typedef HTML_ELEMENT_GROUP::iterator HTML_ELEMENT_GROUP_ELEMENT_ITERATOR;  

HRESULT WINAPI release_html_element_group(HTML_ELEMENT_GROUP *group);

#include <map>
typedef std::map<wstring, wstring> HTML_ELEMENT_ATTRIBUTES; 
typedef struct _HTML_ELEMENT_INFO
{
	_HTML_ELEMENT_INFO()
	{
		info_type = UNKNOWN_HTML_ELEMENT; 
		in_frame = FALSE; 
	}

    ~_HTML_ELEMENT_INFO()
    {
        release_html_element_group(&elements); 
    }

	HTML_ELEMENT_ACTION action_config; 
	HTML_ELEMENT_GROUP elements; 
    HTML_ELEMENT_ATTRIBUTES attrs; 
	//HTML_ELEMENT_ACTIONS actions; 
	HTML_ELEMENT_TYPE info_type; 
	wstring tag; 
	wstring _class; 
	wstring id; 
	wstring href; 
	wstring name; 
	wstring value; 
	wstring xpath; 
	//wstring text; 
	wstring content; 
	wstring type; 
	wstring style; 
	wstring title; 
	
	wstring content_editable; 
	wstring spell_check; 

	wstring event_onchange; 
	wstring event_onmouseover; 
	wstring event_onclick; 

	wstring input_text; 
	wstring content_name; 

	wstring tabindex; 
	BOOLEAN in_frame; 

	ULONG sleep_time; 
	//_variant_t value; 
} HTML_ELEMENT_INFO, *PHTML_ELEMENT_INFO; 

typedef vector< HTML_ELEMENT_INFO* > HTML_ELEMENT_INFOS; 
typedef HTML_ELEMENT_INFOS::iterator HTML_ELEMENT_INFO_ITERATOR; 

//#define MAX_HTTP_URL 1024
typedef struct _HTML_PAGE_INFO
{
	wstring url; 
	HTML_ELEMENT_INFOS elements; 
	//HTML_ELEMENT_INFOS action_elements; 
	//HTML_ELEMENT_INFOS data_elements; 
} HTML_PAGE_INFO, *PHTML_PAGE_INFO; 

typedef vector< HTML_PAGE_INFO* > HTML_PAGE_INFOS; 

typedef HTML_PAGE_INFOS::iterator HTML_PAGE_INFOS_ITERATOR; 

typedef struct _HTML_WEB_SITE_INFO
{
	wstring site_url; 
	wstring site_desc; 
	HTML_PAGE_INFOS pages; 
} HTML_WEB_SITE_INFO, *PHTML_WEB_SITE_INFO; 

typedef vector< HTML_WEB_SITE_INFO* > HTML_WEB_SITE_INFOS; 
typedef HTML_WEB_SITE_INFOS::iterator HTML_WEB_SITE_INFO_ITERATOR; 

LRESULT WINAPI release_html_element_infos(HTML_ELEMENT_INFOS *infos); 

LRESULT WINAPI release_html_page_info( HTML_PAGE_INFO *page ); 

LRESULT WINAPI init_html_web_site_info( HTML_WEB_SITE_INFO *site ); 

LRESULT WINAPI release_html_web_site_info( HTML_WEB_SITE_INFO *site ); 

LRESULT WINAPI add_html_element_from_value( HTML_ELEMENT_INFO *element_info, HTML_PAGE_INFO *page_info ); 

LRESULT WINAPI add_html_page( HTML_PAGE_INFO *page_info, HTML_WEB_SITE_INFO *site ); 

LRESULT WINAPI _load_html_page_info( HTML_PAGE_INFO *html_page, LPCWSTR file_name ); 

LRESULT WINAPI load_html_site_infos( HTML_WEB_SITE_INFOS *html_site, LPCWSTR file_name ); 

LRESULT WINAPI unicode_to_utf8_ex( LPCWSTR text, string &text_out ); 

LRESULT WINAPI utf8_to_unicode_ex( LPCSTR text, wstring &text_out ); 

#define PATH_DELIM_CH L'\\'
#define PATH_DELIM_CH_A '\\'

LRESULT WINAPI get_app_path( LPWSTR FileName, 
							ULONG ccBufferLength, 
							ULONG *ccReturnLength ); 

LRESULT WINAPI get_app_path_a( LPSTR file_name, 
							  ULONG cc_buf_len, 
							  ULONG *cc_ret_len ); 

LRESULT WINAPI open_file_dlg( BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
							 LPCTSTR lpszDefExt = NULL,
							 LPCTSTR lpszFileName = NULL,
							 LPCTSTR lpszFileTitle = NULL,
							 LPCTSTR lpszInitialDir = NULL, 
							 LPCTSTR lpszFilter = NULL, 
							 HWND hWndParent = NULL, 
							 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ); 

//LRESULT WINAPI find_page_group_config( LPCWSTR file_name, 
//									  LPCWSTR domain_name, 
//									  MSXML::IXMLDOMDocument **xml_doc_out, 
//									  MSXML::IXMLDOMElement **site_element_out ); 

typedef struct _STRING_POINTER 
{
	LPWSTR _string;
	USHORT length;
} STRING_POINTER,*PSTRING_POINTER;

LRESULT WINAPI split_string( LPCWSTR szBuffer, 
							ULONG nBufferLen, 
							LPCWSTR szSplitCharSet, 
							STRING_POINTER *pMyString,
							ULONG nSize, 
							ULONG *ret_count ); 

LRESULT WINAPI get_domain_name_in_url( LPCWSTR url, wstring &domain_name ); 

LRESULT WINAPI get_file_name_from_url( LPCWSTR url, CStringW &file_name ); 

HRESULT WINAPI get_html_element_url( IHTMLElement *html_element, 
									wstring *url ); 

HRESULT WINAPI get_html_element_title( IHTMLElement *html_element, wstring *title ); 

LPCWSTR WINAPI get_html_element_tag_desc( LPCWSTR tag ); 

HRESULT WINAPI scroll_expand_html_doc( IHTMLDocument2 *html_doc ); 

LRESULT WINAPI _add_html_element( HTML_ELEMENT_INFO *element_info, 
								 HTML_ELEMENT_INFOS *element_infos ); 

INLINE LRESULT WINAPI convert_int_to_string( ULONG id, string &id_text )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	CHAR text[ MAX_NUMBER_TEXT_LEN ]; 

	do 
	{
		hr = StringCchPrintfA( text, ARRAYSIZE( text ), "%u", id ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		id_text = text; 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI convert_int_to_string_w( ULONG id, wstring &id_text )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	WCHAR text[ MAX_NUMBER_TEXT_LEN ]; 

	do 
	{
		hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", id ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		id_text = text; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_folder_name_from_url( LPCWSTR url, CStringW &file_name ); 

LRESULT WINAPI check_browser_action( HTML_ELEMENT_ACTION *action ); 
LRESULT WINAPI check_local_action( HTML_ELEMENT_ACTION *action ); 
LRESULT WINAPI check_no_html_element_action( HTML_ELEMENT_ACTION *action ); 

#endif //__HTML_COMMON_H__