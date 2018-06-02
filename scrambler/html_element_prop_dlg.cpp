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
#include "external_link.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "input_content.h"
#include "html_element_config.h"
#include "ie_auto.h"
#include "data_scramble.pb.h"
#include "pipe_line.h"
#include "webbrowser.h"
#include "html_element_prop_dlg.h"
#include "odbc_io.h"
#include "ha_script_parse.h"
#include "locate_url_config_dlg.h"
#include "user_reg.h"
#include "diff_match_patch.h"
#include "scrapy/scrapy_support.h"
#include "plugin.h"
#include "sqlite_conn.h"
#include "web_page_browser_manage.h"

#ifdef _DEBUG
#define DEBUG_BREAK() DebugBreak() 
#else
#define DEBUG_BREAK()  
#endif //_DEBUG

#define AUTO_SAVE_SCRIPT_TIMER 1001
#define CLEAN_SAVE_STATUS_TIMER 1002

#define AUTO_SAVE_SCRIPT_TIMER_ELAPSE 30000
#define CLEAN_SAVE_STATUS_TIMER_ELAPSE 2000

#define OPENED_PAGE_LAYOUT_NODE_NAME L"被打开网页布局"

#define HTML_ACTION_DESC_FORMAT_STRING L"[%u]" //ID:%u
#define HTML_ELEMENT_LIST_TITLE_FORMAT_TEXT L"共%u个元素"

#define HTML_ELEMENT_ACTION_DUMMY_TYPE L"dummy"
#define HTML_ELEMENT_ACTION_DUMMY_TYPE_PARAM L"allocated"

#define IDC_MENU_ITEM_DELETE_TEXT_BLOCK 0x1011
#define MAX_PIPE_DATA_LEN ( 1024 * 1024 * 6 )

#define MAX_RETRY_HTML_ACTION_COUNT 2

#define ERROR_NEXT_HTML_ACTION_NOT_FOUND                  0xe1038021

//#define DEBUG_USER_ACCESS_CHECK 1

#define BUTTON_STOP_TEXT L"停止"
#define BUTTON_RUN_TEXT L"运行"

#define HTML_ELEMENT_ACTION_UI_SHOW_ID 0x00000001

#define  HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT L"输出"
#define  HTML_ELEMENT_ACTION_INPUT_UI_TEXT L"输入"
#define  HTML_ELEMENT_ACTION_CLICK_UI_TEXT L"点击"
#define  HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT L"返回"
#define  HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT L"浏览"
#define  HTML_ELEMENT_ACTION_HOVER_UI_TEXT L"鼠标移动"
#define  HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT L"脚本"

#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT L"文本"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT L"链接地址"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE_UI_TEXT L"图像"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_PROPERTY_UI_TEXT L"其它属性"

#define LOCATE_TO_NONE 0x00000000
#define LOCATE_TO_FIRST_SIBLING_ITEM 0x00000001
#define LOCATE_TO_FIRST_SUB_ITEM 0x00000002
#define LOCATE_TO_NEXT_SIBLING_ITEM 0x00000003
#define LOCATE_TO_JUMP_TO_ITEM 0x00000004
#define LOCATE_TO_PARENT_NEXT_SIBLING_ITEM 0x00000005
#define NEXT_HTML_ACTION_DELAY 20

#define MAX_HTML_SCRIPT_INSTANCE 1
HTML_SCRIPT_INFO script_info; 
HTML_SCRIPT_INSTANCE script_instances[MAX_HTML_SCRIPT_INSTANCE];

LRESULT WINAPI get_file_absolute_path(LPCWSTR file_name, LPWSTR file_path, ULONG cc_buf_len, ULONG *cc_ret_len); 
ULONG WINAPI exec_html_action_script_thread( PVOID context ); 
ULONG CALLBACK execute_html_commmand_thread( PVOID param ); 
LRESULT WINAPI exec_html_script_action( HTML_ELEMENT_ACTION *action ); 
ULONG CALLBACK exit_scirpt_work_thread( PVOID param ); 
LRESULT WINAPI get_html_element_info(IHTMLElement *element, HTML_ELEMENT_INFO *info); 
HRESULT WINAPI get_html_element_content(IHTMLElement *html_element,
    wstring *content); 

#define REMOVE_DATA_OUTPUT_ENTRY 0x00000001
LRESULT WINAPI iterate_data_output_from_source(HTML_ELEMENT_ACTION *action, wstring &data, ULONG flags)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        data.clear();
        if (action->outputs.size() == 0)
        {
            ret = ERROR_NO_MORE_ITEMS;
            break;
        }

        data = (*action->outputs.begin());
        if (flags & REMOVE_DATA_OUTPUT_ENTRY)
        {
            action->outputs.erase(action->outputs.begin());
        }
    } while (FALSE);

    return ret;
}
;
LRESULT WINAPI get_open_page_url_from_source(HTML_ELEMENT_ACTION *action, wstring &string)
{
    return iterate_data_output_from_source(action, string, REMOVE_DATA_OUTPUT_ENTRY);
}


LRESULT get_new_page_info(HTML_SCRIPT_INSTANCE *instance, HTML_PAGE_INFO &info)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        if (instance->location_url.length() == 0)
        {
            info.url = instance->begin_url;
        }
        else
        {
            info.url = instance->location_url;
        }

    } while (FALSE);

    return ret;
}

LRESULT WINAPI post_script_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_ERRORS_ENCOUNTERED; 
	ULONG flags; 

	do 
	{
		ASSERT( NULL != file_name ); 
		ret = get_script_file_flag( file_name, &flags ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		if( 0 != ( flags & HTML_ELEMENT_SCRIPT_FLAG_POSTED ) )
		{
			break; 
		}

		ret = post_data_file( file_name ); 
		if( ret == ERROR_SUCCESS )
		{
			set_script_file_flag( file_name, HTML_ELEMENT_SCRIPT_FLAG_POSTED ); 
		}
	}while(FALSE ); 

	return ret; 
}

#define DEFAULT_HTML_ELEMENT_TEXT_FIELD_NAME L"field%ID%"
LRESULT WINAPI get_default_field_name( wstring &field_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	field_name = DEFAULT_HTML_ELEMENT_TEXT_FIELD_NAME; 
	return ret; 
}


/***********************************************************
txt @=0;
输入问题：
自定义的HTML 元素无法执行输入
***********************************************************/

LRESULT WINAPI parse_script_text( LPCWSTR text, wstring &text_out ); 
LRESULT WINAPI post_process_action( HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring url; 
	WEB_BROWSER_PROCESS *web_browser_info; 

	do 
	{
		if( 0 != wcscmp( HTML_ELEMENT_ACTION_LOCATE, action->action.c_str() ) )
		{
			break; 
		}

		ret = get_action_web_browser( root, action, &web_browser_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		url = action->param; 
		if( url.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = parse_script_text( url.c_str(), url ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		web_browser_info->context.locate_url = url; 
	}while( FALSE );
	return ret; 
}

LRESULT WINAPI create_html_action_script_worker( SCRIPT_WORKER *worker )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE work_thread = NULL; 

	do
	{
		ASSERT( NULL != worker ); 

		worker->stop_work = TRUE; 
		worker->running = FALSE; 
		worker->exit = FALSE; 
		worker->action = NULL; 
		worker->worker = NULL; 

		InitializeCriticalSection( &worker->lock ); 

		worker->work_notifier = CreateEvent( NULL, FALSE, FALSE, NULL ); 
		if( NULL == worker->work_notifier )
		{
			ret = GetLastError(); 
			break; 
		}

		worker->state_notifier  = CreateEvent( NULL, TRUE, FALSE, NULL ); 
		if( NULL == worker->state_notifier  )
		{
			ret = GetLastError(); 
			break; 
		}

		work_thread = CreateThread( NULL, 0, 
			exec_html_action_script_thread, 
			worker, 
			0, 
			NULL ); 

		if( work_thread == NULL )
		{
			ret = GetLastError(); 
			break; 
		}
		
		worker->worker = work_thread; 
		work_thread = NULL; 
	}while( FALSE ); 

	if( NULL != work_thread )
	{
		CloseHandle( work_thread ); 
	}

	return ret; 
}

LRESULT WINAPI init_page_content()
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *element_content = NULL; 
	
	do 
	{
		element_content = new HTML_ELEMENT_CONTENT(); 
		if( NULL == element_content )
		{
			break; 
		}

		element_content->name = L"content"; 
		//scrambled_page_content.element_contents.push_back( element_content ); 

		element_content = NULL; 
	}while( FALSE );

	if( NULL != element_content )
	{
		delete element_content; 
	}

	return ret; 
}

LRESULT CALLBACK html_element_action_send( PVOID data, ULONG data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

LRESULT CALLBACK html_element_action_recv( PVOID *data, ULONG *data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

LRESULT CALLBACK html_element_action_free( PVOID *data, ULONG *data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}

element_action_handler action_handler; 

XPATH_PARAMS input_contents; 
MSXML::IXMLDOMDocumentPtr config_xml_doc; 
LRESULT WINAPI open_actions_config_file( LPCWSTR file_name, ULONG cc_name_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	VARIANT_BOOL __ret; 
	LPCWSTR _temp_text; 

	do 
	{
		ASSERT( NULL != file_name ); 

		ret = make_config_file_exist( file_name, cc_name_len ); 

		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&config_xml_doc ); 
		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "create xml document error %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = config_xml_doc->load( file_name ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = config_xml_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER; 
			break; 		
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI set_script_file_flag( LPCWSTR file_name, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr sub_xml_element; 
	MSXML::IXMLDOMElementPtr root_xml_element; 
	MSXML::IXMLDOMNodePtr xml_node; 

	VARIANT_BOOL __ret;  
	_variant_t save_file_name; 

	//MSXML::IXMLDOMNodeListPtr xml_node_list; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	//LONG node_count; 
	//LPWSTR __temp_text; 
	LPCWSTR _temp_text; 
	//_bstr_t temp_text; 
	_bstr_t attr_name; 
	//_variant_t node_value; 
	WCHAR text[ MAX_DIGIT_LEN ]; 

	do 
	{
		ASSERT( NULL != file_name ); 
		CString config_file; 

		if( *file_name == L'\0')
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		config_file = file_name; 

		if( config_file.GetAt( config_file.GetLength() - 1 ) == L'\\' )
		{
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

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			break; 
		}

		if( xml_doc->documentElement == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		{
			_bstr_t element_path; 
			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			xml_node = xml_doc->documentElement->selectSingleNode( element_path ); 
			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), 
				( PVOID* )&root_xml_element ); 
			if( FAILED(hr))
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		ASSERT( root_xml_element != NULL ); 

		if( NULL == root_xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		do
		{
			_variant_t attr_value; 

			hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", flags ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			attr_value = text; 

			hr = root_xml_element->setAttribute( HTML_SCRIPT_INSTANCE_ATTRIBUTE_FLAGS, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE );  

		do 
		{
			_variant_t save_file_name; 
			_bstr_t text; 

			text = file_name; 

			save_file_name = text; 
			hr = xml_doc->save( save_file_name ); 

			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		} while ( FALSE );
	}while( FALSE ); 
	return ret; 
}

LRESULT WINAPI get_script_file_flag( LPCWSTR file_name, ULONG *flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr sub_xml_element; 
	MSXML::IXMLDOMElementPtr root_xml_element; 
	MSXML::IXMLDOMNodePtr xml_node; 

	VARIANT_BOOL __ret;  
	_variant_t save_file_name; 

	//MSXML::IXMLDOMNodeListPtr xml_attrs; 
	MSXML::IXMLDOMAttributePtr xml_attr; 
	//LONG node_count; 
	LPWSTR __temp_text; 
	LPCWSTR _temp_text; 
	_bstr_t temp_text; 
	_bstr_t attr_name; 
	WCHAR text[ MAX_DIGIT_LEN ]; 

	do 
	{
		ASSERT( NULL != file_name ); 
		ASSERT( NULL != flags ); 

		CString config_file; 
		
		*flags = 0; 

		if( *file_name == L'\0')
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		config_file = file_name; 

		if( config_file.GetAt( config_file.GetLength() - 1 ) == L'\\' )
		{
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

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			break; 
		}

		if( xml_doc->documentElement == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		{
			_bstr_t element_path; 
			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			xml_node = xml_doc->documentElement->selectSingleNode( element_path ); 
			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), 
				( PVOID* )&root_xml_element ); 
			if( FAILED(hr))
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		ASSERT( root_xml_element != NULL ); 
		if( NULL == root_xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		do
		{
			_variant_t attr_value; 

			hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", flags ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			attr_value = text; 
			attr_name = HTML_SCRIPT_INSTANCE_ATTRIBUTE_FLAGS; 

			hr = root_xml_element->raw_getAttributeNode( attr_name, &xml_attr ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_attr->get_nodeValue( attr_value.GetAddress() ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( attr_value.vt != VT_BSTR )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			temp_text = attr_value.bstrVal; 
			_temp_text = temp_text.operator const wchar_t*(); 

			if( NULL == _temp_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			*flags = wcstoul( _temp_text, &__temp_text, 0 ); 
		}while( FALSE );  

		do 
		{
			_variant_t save_file_name; 
			_bstr_t text; 

			text = file_name; 

			save_file_name = text; 
			hr = xml_doc->save( save_file_name ); 

			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		} while ( FALSE );
	}while( FALSE ); 
	return ret; 
}

IMPLEMENT_DYNAMIC(html_element_prop_dlg, CDialog)

CRITICAL_SECTION config_list_lock; 
vector<HTML_ELEMENT_ACTION*> config_list; 
ULONG element_config_base_id = 0; 
//ULONG element_config_count = 0; 

html_element_prop_dlg *g_html_element_prop_dlg = NULL; 

html_element_prop_dlg::html_element_prop_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(html_element_prop_dlg::IDD, pParent), 
	element( NULL ), 
	configure_state( HTML_INSTRUCTION_CONTENT_PROCESS ), //MAX_HTML_INSTRUCTION_STATE 
	jump_from_item( NULL ), 
	cut_copy_item( NULL ), 
	exit_script_work_thread_handle( NULL ), 
	exist_work_event( NULL ), 
    instruction_chnaged(FALSE), 
    xpath_dlg(this)
{
	InitializeCriticalSection( &config_tree_lock ); 
	init_config_list(); 

	action_handler.free_data = html_element_action_free; 
	action_handler.send_data = html_element_action_send; 
	action_handler.recv_data = html_element_action_recv; 

	script_running = FALSE; 

	//init_sql_conn_param( &output_sql_connection ); 

	m_pListEdit = NULL;
}

html_element_prop_dlg::~html_element_prop_dlg()
{
	IHTMLElement *_html_element; 
	if( NULL != element )
	{
		_html_element = (IHTMLElement*)element; 
		_html_element->Release(); 
		element = NULL; 
	}

	DeleteCriticalSection( &config_tree_lock ); 
	uninit_config_list(); 
}

void html_element_prop_dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_ACTION, element_actions);
    DDX_Control(pDX, IDC_COMBO_CONTANT_TYPE, contant_type);
    //DDX_Control(pDX, IDC_LIST_ELEMENTS, tree_elements);
    DDX_Control(pDX, IDC_TREE_ELEMENTS, tree_elements);
    DDX_Control(pDX, IDC_EDIT_INPUT_CONTENT, input_content);
    DDX_Control(pDX, IDC_CHECK_LOAD_WEB_PAGE, open_new_page);
    //DDX_Control(pDX, IDC_COMBO_OUTPUT_METHOD, output_method);
    DDX_Control(pDX, IDC_EDIT_CONTENT_OUTPUT_PATH, output_content_path);
    //DDX_Control(pDX, IDC_LIST_CONTENT_ELEMENTS, contants_elements);
    DDX_Control(pDX, IDC_EDIT_OUTPUT_FIELD_NAME, field_name);
    DDX_Control(pDX, IDC_SLIDER_WEB_PAGE_LOAD_TIME, web_page_load_time);
    DDX_Control(pDX, IDC_SLIDER_PROCESS_DELAY_TIME, process_delay_time);
    DDX_Control(pDX, IDC_LIST_HTML_LOCATE_URL, locate_urls);
    DDX_Control(pDX, IDC_LIST_DATA_SET, data_set);
    DDX_Control(pDX, IDC_RICHEDIT_SCRAPY, scrapy_script_edit);
    DDX_Control(pDX, IDC_RICHEDIT2_PROPERTY_TEXT, property_edit);
}

#include "html_script_config_dlg.h"
extern html_script_config_dlg *g_html_script_config_dlg; 

HTML_ELEMENT_ACTION hilight_action; 
HANDLE thread_handle = NULL; 
LRESULT html_element_prop_dlg::on_hilight( WPARAM wparam, LPARAM lparam )
{
	HRESULT hr; 
	LPCWSTR xpath; 
	LRESULT ret = ERROR_SUCCESS; 
	LONG lx; 
	LONG ly; 

	do 
	{
		if( lparam == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		xpath = ( LPCWSTR )lparam; 
		if( *xpath == L'\0' )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

        g_html_script_config_dlg->highlight_html_element_by_xpath(xpath);
        break; 

		if( thread_handle != NULL )
		{
			ULONG wait_ret; 
			wait_ret = WaitForSingleObject( thread_handle, 0 ); 

			if( wait_ret != WAIT_OBJECT_0 
				&& wait_ret != WAIT_ABANDONED_0 
				&& wait_ret != WAIT_FAILED )
			{
				break; 
			}

			CloseHandle( thread_handle ); 
			thread_handle = NULL; 
		}

		hilight_action.id = 0; 
		hilight_action.jump_to_id = INVALID_JUMP_TO_ID; 
		hilight_action.in_frame = FALSE; 
		hilight_action.next_item = NULL; 
		//hilight_action.prev_item = NULL; 
		hilight_action.sub_item = NULL; 
		hilight_action.parent_item = NULL; 

		hilight_action.xpath = xpath; 
		hilight_action.action = HTML_ELEMENT_ACTION_HILIGHT; 

		do
		{
			IDispatchPtr disp; 
			IHTMLDocument2Ptr html_doc; 
			IHTMLElementPtr html_element; 

			disp = g_html_script_config_dlg->m_WebBrowser.GetDocument(); 

			if( NULL == disp )
			{
				break; 
			}

			hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

			if( FAILED( hr ) 
				|| html_doc == NULL )
			{
				break; 
			}

			hr = html_doc->get_body( &html_element ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( NULL == html_element )
			{
				break; 
			}

			hr = html_element->get_offsetTop( &lx );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			hr = html_element->get_offsetLeft( &ly );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			if( ly != 0 
				|| lx != 0 )
			{
				break; 
			}

			{
				IHTMLDocument3Ptr html_doc3; 
				hr = html_doc->QueryInterface( IID_IHTMLDocument3, ( PVOID* )&html_doc3 ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				if( html_doc3 == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = html_doc3->get_documentElement( &html_element ); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( html_element == NULL )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = html_element->get_offsetTop( &lx );
				if( FAILED(hr ))
				{
					ret = hr; 
					break; 
				}

				hr = html_element->get_offsetLeft( &ly );
				if( FAILED(hr ))
				{
					ret = hr; 
					break; 
				}
			}
		}while( FALSE ); 

		do
		{
			WCHAR text[ MAX_PATH ]; 
			hr = StringCchPrintfW( text, ARRAYSIZE( text ), L"HTML|BODY|==%d:%d", lx, ly ); 
			if( FAILED(hr))
			{
				break; 
			}

			hilight_action.param = text; 
		}while( FALSE ); 

		HTML_ACTION_EXECUATE_PARAM param; 
		param.action = &hilight_action; 
		
		param.locate_url = g_html_script_config_dlg->m_WebBrowser.get_loading_url(); 

		thread_handle = CreateThread( NULL, 0, execute_html_commmand_thread, ( PVOID )&param, 0, NULL ); 
		//ret = execute_html_commmand( &hilight_action ); 

		if( thread_handle == NULL )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while(FALSE );

	//if( thread_handle != NULL )
	//{
	//	CloseHandle( thread_handle ); 
	//}

	return ret; 
}

/*********************************************************************
元素XPATH不开放，给出用户方便的操作过程。否则需要用户理解XPATH概念。
*********************************************************************/
VOID html_element_prop_dlg::set_html_element_xpath(LPCWSTR xpath )
{
	HRESULT hr; 
	HTML_ELEMENT_GROUP group; 

	do 
	{
		ASSERT( NULL != xpath ); 

		this->xpath = xpath; 
		//SetDlgItemTextW( IDC_EDIT_XPATH, this->xpath.c_str() ); 
		hr = g_html_script_config_dlg->get_html_element_from_xpath( xpath, &group ); 
		if( FAILED(hr))
		{
		}

		if( group.size() == 0 )
		{
			dbg_print( MSG_FATAL_ERROR, "unrecognized xpath %ws\n", xpath ); 
		}
	}while( FALSE ); 

	release_html_element_group( &group ); 

	return; 
}

VOID html_element_prop_dlg::set_html_element( PVOID html_element )
{
	IHTMLElement *_html_element; 
	do 
	{

		if( NULL != element )
		{
			_html_element = (IHTMLElement*)element; 

			try
			{
				_html_element->Release(); 
			}
			catch (...)
			{
			}

			element = NULL; 
		}

		_html_element = (IHTMLElement* )html_element; 
		_html_element->AddRef(); 

		element = html_element; 
	} while ( FALSE );
    return; 
}

LRESULT WINAPI action_will_open_page(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;
 
    do
    {
        ASSERT(NULL != action); 
        if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT) 
            && 0 == wcscmp( action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
        {
            break; 
        }

        if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_CLICK)
            && 0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE))
        {
            break;
        }

        ret = ERROR_INVALID_PARAMETER; 
    } while (FALSE); 

    return ret;
}

LRESULT html_element_prop_dlg::get_active_element_properties( HTML_ELEMENT_PROPERTIES *properties )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	IHTMLElement *html_element; 

	LRESULT _ret; 
	wstring _field_name; 

	do 
	{
		ASSERT( NULL != properties ); 

		if( NULL == element )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		html_element = ( IHTMLElement* )element; 
		properties->field_name = L""; 

		_ret = get_html_text_field_name( html_element, element_config_base_id, _field_name ); 
		if( _ret != ERROR_SUCCESS )
		{

		}

		if( _field_name.length() == 0 )
		{
			get_default_field_name( _field_name ); 
		}

		properties->field_name = _field_name; 

	}while( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::init_element_properties( LPCWSTR std_xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElement *html_element; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	CString properities_text; 

	do 
	{
		do 
		{
			ASSERT( NULL != element ); 
			html_element = ( IHTMLElement* )element; 

            do
            {
                LRESULT _ret; 
                wstring script_text;
                _ret = action_to_scrapy(std_xpath, script_text);
                if (_ret != ERROR_SUCCESS)
                {
                    dbg_print(MSG_FATAL_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, _ret);
                    break;
                }

                scrapy_script_edit.SetWindowTextW(script_text.c_str());
                scrapy_script_edit.SetSel(0, -1);

                //python_scrapy_dlg.ShowWindow(SW_SHOW); 
                //python_scrapy_dlg.insert_text(script_text.c_str()); 
            } while (FALSE);

			properities_text += L"xpath="; 
			properities_text += std_xpath; 
			properities_text += L"\r\n"; 
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

			properities_text += L"class='"; 
			properities_text += _temp_text; 

			properities_text += L"'"; 
			properities_text += L" "; 
		} while ( FALSE ); 

		do 
		{
			HTML_ELEMENT_ACTION_TYPE action_type = get_html_element_default_action_info( html_element );

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

			element_actions.SetCurSel( ( INT32 )action_type ); 

			if( HTML_ELEMENT_ACTION_TYPE_OUTPUT == action_type )
			{
				if( 0 == _wcsicmp( _temp_text, L"a" ) )
				{
					contant_type.SetCurSel( 1 ); 
				}
				else
				{
					LRESULT _ret; 
					wstring _field_name; 
					contant_type.SetCurSel( 1 ); 

					_ret = get_html_text_field_name( html_element, element_config_base_id, _field_name ); 
					if( _ret != ERROR_SUCCESS )
					{

					}

					if( _field_name.length() == 0 )
					{
						get_default_field_name( _field_name ); 
					}

					field_name.SetWindowText( _field_name.c_str() ); 
				}
			}
			else if( HTML_ELEMENT_ACTION_TYPE_INPUT == action_type )
			{
				input_content.SetWindowText(L""); 
			}
			else if ( HTML_ELEMENT_ACTION_TYPE_SCRIPT == action_type )
			{
				input_content.SetWindowText(L""); 
			}
			else if( HTML_ELEMENT_ACTION_TYPE_CLICK == action_type )
			{
				open_new_page.SetCheck( 0 ); 
			}

			OnCbnSelendokComboAction(); 

			properities_text += L"tag='"; 
			properities_text += _temp_text; 

			properities_text += L"'"; 
			properities_text += L" "; 

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

			properities_text += L"text='"; 
			properities_text += _temp_text; 
			properities_text += L"'"; 
			properities_text += L" "; 
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

			properities_text += L"name='"; 
			properities_text += _temp_text; 

			properities_text += L"'"; 
			properities_text += L" "; 
		} while ( FALSE ); 

		SetDlgItemTextW( IDC_RICHEDIT2_PROPERTY_TEXT, properities_text.GetBuffer() ); 

		do 
		{
			SetDlgItemTextW( IDC_EDIT_XPATH, xpath.c_str() ); 

			if( TRUE == xpath_dlg.IsWindowVisible() )
			{
				xpath_dlg.set_edit_xpath( xpath.c_str() ); 
			}
		} while ( FALSE ); 

	}while( FALSE );

	return ret; 
}

/****************************************************
对相同的网页结构，同时打开多个终端，同时抓取：
1.使用多进程的方式，UI界面设计需要规划
****************************************************/

/****************************************************
HTML元素需要显示的属性:
1.XPATH
2.动作
3.属性(包括class id name)
4.列表(由于元素中还包括了行为信息,可能使用TREE更合适)
	列表实现方案:
	1.LIST VIEW
	2.TREE
	3.HTML
****************************************************/

LPCSTR test_scripts[] = { "int i = 0;", 
"i+=1;", 
"if (  i<  20)" }; 

LRESULT WINAPI test_ha_script_parse()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 
	VARIABLE_INFO variable; 

	do 
	{
		for( i = 0; i < ARRAYSIZE( test_scripts ); i ++ )
		{
			ret = parse_ha_script( test_scripts[ i ] ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}

			ret = get_script_variable( "i", &variable ); 
			if( ERROR_SUCCESS != ret )
			{
				//break; 
			}
			else
			{
				dbg_print( MSG_IMPORTANT, "i is %d\n", variable.variable.intVal ); 
			}

			ret = get_script_variable( "if", &variable ); 
			if( ERROR_SUCCESS != ret )
			{
				//break; 
			}
			else
			{
				dbg_print( MSG_IMPORTANT, "if statement return %d\n", variable.variable.intVal ); 
			}
		}
	}while( FALSE );

	return ret; 
}

#ifdef DEBUG_USER_ACCESS_CHECK 
ULONG CALLBACK check_user_access_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		for( ; ; )
		{
			do
			{
				if( NULL == g_html_element_prop_dlg
					|| NULL == g_html_element_prop_dlg->get_compare_dlg()->GetSafeHwnd() )
				{
					break; 
				}
				//ret = process_current_user_access();

				LRESULT _ret; 
				INT32 i; 
				LPCWSTR all_test_xpaths[ 7 ][2] = { 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DIV @=8;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DIV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|" }, 
					{ L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=content;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Village of No Return ;|", 
					L"HTML lang=zh-cmn-Hans;|BODY|DIV @=2;id=wrapper;|DIV @=0;id=context;|DIV @=0;|DIV @=0;|DIV @=0;|DIV @=0;|TABLE @=0;|TBODY @=0;|TR @=0;|TD @=1;|DAV @=0;|A @=0;element_text=健忘村 / The Vill of No Return ;|" } }; 

					g_html_element_prop_dlg->get_compare_dlg()->SendMessage( WM_XPATH_MISMATCH_INFO, 0, 0 ); 

					for( i = 0; i < ARRAYSIZE(all_test_xpaths); i ++ )
					{
						_ret = g_html_element_prop_dlg->get_compare_dlg()->SendMessage( WM_XPATH_MISMATCH_INFO, ( WPARAM )( PVOID )all_test_xpaths[ i ][ 0 ], ( LPARAM )( PVOID )all_test_xpaths[ i ][ 1 ] ); 
					}

			}while( FALSE ); 
			Sleep( 10000 ); 
		}
	}while( FALSE );

	return ret; 
}
#endif //DEBUG_USER_ACCESS_CHECK 

static DWORD CALLBACK EditStreamCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{

    return 0;
}

LRESULT WINAPI get_html_actions(HTML_ELEMENT_ACTION *actions_source, HTML_ELEMENT_ACTION **actions_output)
{
    LRESULT ret = ERROR_SUCCESS; 
    
    do 
    {
        //以精准的结构对HTML指令树进行排列
    } while ( FALSE );

    return ret; 
}

LRESULT WINAPI run_diff_match_patch_test(); 
BOOL html_element_prop_dlg::OnInitDialog()
{
	LRESULT ret; 
	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

//#ifdef DEBUG_USER_ACCESS_CHECK 
//	CreateThread( NULL, 
//	0, 
//	check_user_access_thread, 
//	NULL, 
//	0, 
//	NULL );  
//#endif //DEBUG_USER_ACCESS_CHECK 

	//run_diff_match_patch_test(); 

	init_html_script_info( &script_info, NULL ); 

	init_ha_script(); 

	init_sqlite(); 

#define DATA_SCRAMBLER_USAGE_TEXT  L"注意:\r\n1.右键按下移动鼠标分析网页,左键点击浏览网页.\r\n2.双击左侧列表中指令显示/修改指令细节\r\n3.右键双击左侧列表删除选中指令"
    SetDlgItemText(IDC_RICHEDIT2_PROPERTY_TEXT, DATA_SCRAMBLER_USAGE_TEXT);
    //SetDlgItemText(IDC_RICHEDIT_SCRAPY, DATA_SCRAMBLER_USAGE_TEXT); 

	//test_ha_script_parse(); 

	//{
	//	wstring test_text; 
	//	test_text = L"\"te,st,\"\"1234"; 
	//	format_csv_field_text( test_text ); 
	//}
	xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
	xpath_dlg.ShowWindow( SW_HIDE ); 

	compare_dlg.Create( MAKEINTRESOURCE( compare_dlg.IDD ), this ); 
	compare_dlg.ShowWindow( SW_HIDE ); 

	output_dlg.Create( MAKEINTRESOURCE( output_dlg.IDD ), this ); 
	output_dlg.ShowWindow( SW_HIDE ); 

    scrapy_script_edit.ModifyStyle(0, ES_WANTRETURN); 
    //python_scrapy_dlg.Create(MAKEINTRESOURCE(python_scrapy_dlg.IDD), this);
    //python_scrapy_dlg.ShowWindow(SW_HIDE);

	set_xpath_info_ui_window( compare_dlg.GetSafeHwnd() ); 

	//tree_elements.SetImageList()
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

	//用户选择针对HTML元素的行为。
	element_actions.AddString( HTML_ELEMENT_ACTION_CLICK_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_INPUT_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_HOVER_UI_TEXT ); 
	element_actions.AddString( HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT ); 

	element_actions.ModifyStyleEx( CBS_SORT, 0,  0);
	OnCbnSelendokComboAction(); 

	contant_type.AddString( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT ); 
	contant_type.AddString( HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT ); 
    contant_type.AddString(HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE_UI_TEXT); 

#define TIME_LINE_SIZE 10
#define TIME_PAGE_SIZE 100
#define TIME_TIC_COUNT 50
	web_page_load_time.SetRange( MIN_WEB_PAGE_LOAD_TIME, MAX_WEB_PAGE_LOAD_TIME, FALSE );
	process_delay_time.SetRange( MIN_PROCESS_DELAY_TIME, MAX_PROCESS_DELAY_TIME, FALSE ); 
	web_page_load_time.SetLineSize( TIME_LINE_SIZE ); 

	web_page_load_time.SetPageSize( TIME_PAGE_SIZE ); 
	process_delay_time.SetLineSize( TIME_LINE_SIZE ); 
	process_delay_time.SetPageSize( TIME_PAGE_SIZE ); 

	web_page_load_time.SetTicFreq( ( MAX_WEB_PAGE_LOAD_TIME - MIN_WEB_PAGE_LOAD_TIME ) / TIME_TIC_COUNT ); 
	process_delay_time.SetTicFreq( ( MAX_PROCESS_DELAY_TIME - MIN_PROCESS_DELAY_TIME ) / TIME_TIC_COUNT ); 

	{
		ULONG _page_load_time; 
		ULONG _process_delay_time; 

		ret = get_html_action_global_config( NULL, &_page_load_time, &_process_delay_time ); 
		
		if( _page_load_time > MAX_WEB_PAGE_LOAD_TIME)
		{
			_page_load_time = MAX_WEB_PAGE_LOAD_TIME; 
		}

		if( _page_load_time < MIN_WEB_PAGE_LOAD_TIME)
		{
			_page_load_time = MIN_WEB_PAGE_LOAD_TIME; 
		}
		web_page_load_time.SetPos( _page_load_time ); 

		if( _process_delay_time > MAX_PROCESS_DELAY_TIME)
		{
			_process_delay_time = MAX_PROCESS_DELAY_TIME; 
		}

		if( _process_delay_time < MIN_PROCESS_DELAY_TIME)
		{
			_process_delay_time = MIN_PROCESS_DELAY_TIME; 
		}
		process_delay_time.SetPos( _process_delay_time ); 

		_set_html_action_global_config( INVALID_TIME_VALUE, _page_load_time, _process_delay_time ); 
	}

	DWORD style = tree_elements.GetStyle();
	style &= ~LVS_TYPEMASK; 
	style |= TVS_HASBUTTONS | TVS_EDITLABELS | TVS_LINESATROOT | TVS_CHECKBOXES; 

	tree_elements.ModifyStyle(0, style); 

	tree_image_list.Create (IDB_BITMAP_ICONS, 16, 1, RGB (255, 255, 255));
	tree_elements.SetImageList (&tree_image_list, TVSIL_NORMAL);
	tree_elements.AddDragEventListener(this);

	//tree_elements.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT| LVS_EX_CHECKBOXES|LVS_REPORT|LVS_EX_SUBITEMIMAGES );

	//tree_elements.InsertColumn(0,_T(""),LVCFMT_LEFT,150);        //添加列标题
	//tree_elements.InsertColumn(1,_T("路径"),LVCFMT_LEFT,200);
	//tree_elements.InsertColumn(2,_T("动作"),LVCFMT_LEFT,60); 
	//tree_elements.InsertColumn(3,_T("动作参数"),LVCFMT_LEFT,100);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	g_html_element_prop_dlg = this; 

	data_dlg.Create( MAKEINTRESOURCE( data_dlg.IDD ), this ); 
	data_dlg.ShowWindow( SW_HIDE ); 

	do 
	{ 
		locate_urls.PrepareControl(0);
		locate_urls.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );

		locate_urls.InsertColumn( 0, _T("起始网页链接"),LVCFMT_LEFT,200); 
		locate_urls.InsertColumn( 1, _T("下一页跳转次数"),LVCFMT_LEFT, 150);

		data_set.PrepareControl(0);
		data_set.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );
		
		data_set.InsertColumn( 0, _T("名称"),LVCFMT_LEFT,200); 
		data_set.InsertColumn( 1, _T("XPATH"),LVCFMT_LEFT,60); 

		do 
		{
			exist_work_event = CreateEvent( NULL, FALSE, FALSE, NULL ); 
			if( NULL == exist_work_event )
			{
				break; 
			}

			exit_script_work_thread_handle = CreateThread( NULL, 
				0, 
				exit_scirpt_work_thread, 
				exist_work_event, 
				0, 
				NULL ); 
		} while ( FALSE ); 
	} while ( FALSE );

	{
		wstring old_conf_file; 
		ret = get_data_scrambler_config( old_conf_file ); 
		if( old_conf_file.length() > 0 )
		{
			SetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, old_conf_file.c_str() ); 
			OnBnClickedButtonLoad(); 
		}
	}

	{
		INT32 i; 
		for( i = 0; i < ARRAYSIZE( script_instances ); i ++ )
		{
			ret = html_script_instance_construct( &script_instances[ i ] ); 
			if( ERROR_SUCCESS != ret )
			{}

			ret = create_html_action_script_worker( &script_instances[ i ].worker ); 
			if( ret != ERROR_SUCCESS )
			{}
		}
	}

    SetTimer(AUTO_SAVE_SCRIPT_TIMER, AUTO_SAVE_SCRIPT_TIMER_ELAPSE, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//LRESULT html_element_prop_dlg::add_input_content_info( IHTMLElement *html_element )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//	ULONG item_count; 
//	wstring element_title; 
//	_bstr_t _text; 
//
//	item_count = contants_elements.GetItemCount(); 
//
//	html_element = ( IHTMLElement* )element; 
//
//	//hr = get_html_element_title( html_element, &element_title ); 
//	//if( FAILED(hr))
//	//{
//	//	//break; 
//	//}
//
//	element_title = L""; 
//	contants_elements.InsertItem(item_count, element_title.c_str() ); 
//
//	do 
//	{
//		LPWSTR __text; 
//		hr = html_element->get_innerText( _text.GetAddress() ); 
//		if( FAILED( hr ) )
//		{
//			break; 
//		}
//
//		__text = _text.operator wchar_t*(); 
//
//		if( NULL == __text )
//		{
//			break; 
//		}
//
//		contants_elements.SetItemText( item_count, 1, __text ); 
//
//		do
//		{
//			LPWSTR xpath_copy = NULL; 
//			ULONG cc_xpath_len; 
//			wstring _xpath; 
//
//			_xpath = xpath; 
//
//			ret = filter_xpath_noisy( _xpath ); 
//			if( ret != ERROR_SUCCESS )
//			{
//
//			}
//
//			cc_xpath_len = _xpath.size(); 
//
//			xpath_copy = ( LPWSTR )malloc( sizeof( WCHAR ) * ( cc_xpath_len + 1 ) ); 
//			if( NULL == xpath_copy )
//			{
//				break; 
//			}
//
//			memcpy( xpath_copy, _xpath.c_str(), sizeof( WCHAR ) * ( cc_xpath_len + 1 ) ); 
//
//			contants_elements.SetItemData( item_count, ( DWORD_PTR )( PVOID )xpath_copy ); 
//		}while( FALSE ); 
//	} while ( FALSE ); 
//
//	return ret; 
//}

BEGIN_MESSAGE_MAP(html_element_prop_dlg, CDialog)
    ON_MESSAGE( WM_ERROR_HANDLE, html_element_prop_dlg::on_error)
    ON_MESSAGE( WM_XPATH_CONFIG, html_element_prop_dlg::on_xpath_config )
	ON_MESSAGE( WM_LOCATE_TO_HTML_ACTION, html_element_prop_dlg::on_locate_to_action )
	ON_MESSAGE( WM_DATA_OUTPUT, html_element_prop_dlg::on_data_output ) 
	ON_MESSAGE( WM_UPDATE_HTML_ACTION, html_element_prop_dlg::on_update_html_action ) 
	ON_MESSAGE( WM_HTML_WORK_STOPPED, html_element_prop_dlg::on_work_stoppped)
	ON_MESSAGE( WM_HTML_ELEMENTS_HILIGHT, html_element_prop_dlg::on_hilight)
	ON_MESSAGE( WM_UPDATE_EXIT_WORK_TIME_DELAY, on_update_exit_work_time_delay )
	ON_BN_CLICKED(IDC_BUTTON_XPATH_VALIDATE, &html_element_prop_dlg::OnBnClickedButtonXpathValidate)
	ON_CBN_SELENDOK(IDC_COMBO_ACTION, &html_element_prop_dlg::OnCbnSelendokComboAction)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &html_element_prop_dlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_FILE_BROWSER, &html_element_prop_dlg::OnBnClickedButtonFileBrowser)
	ON_BN_CLICKED(IDC_BUTTON_EXECUTE, &html_element_prop_dlg::OnBnClickedButtonExecute)
	//ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &html_element_prop_dlg::OnHdnItemdblclickListElements)
	//ON_NOTIFY(NM_DBLCLK, IDC_LIST_ELEMENTS, &html_element_prop_dlg::OnNMDblclkListElements)
	ON_BN_CLICKED(IDC_BUTTON_XPATH_EDIT, &html_element_prop_dlg::OnBnClickedButtonXpathEdit)
	//ON_NOTIFY(NM_CLICK, IDC_LIST_ELEMENTS, &html_element_prop_dlg::OnNMClickListElements)
	//ON_NOTIFY(NM_RCLICK, IDC_LIST_ELEMENTS, &html_element_prop_dlg::OnNMRClickListElements)
	ON_NOTIFY(NM_CLICK, IDC_TREE_ELEMENTS, &html_element_prop_dlg::OnNMClickTreeElements)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_ELEMENTS, &html_element_prop_dlg::OnNMDblclkTreeElements)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_ELEMENTS, &html_element_prop_dlg::OnNMRClickTreeElements)
	ON_NOTIFY(NM_RDBLCLK, IDC_TREE_ELEMENTS, &html_element_prop_dlg::OnNMRDblclkTreeElements)
	ON_WM_DESTROY()
	//ON_WM_LBUTTONDOWN()
	//ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &html_element_prop_dlg::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_REPLACE, &html_element_prop_dlg::OnBnClickedButtonReplace)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_METHOD_CONFIRM, &html_element_prop_dlg::OnBnClickedButtonOutputMethodConfirm)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_HTML_LOCATE_URL, OnBeginUrlslabeleditList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_HTML_LOCATE_URL, OnEndUrllabeleditList)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_CONFIG, &html_element_prop_dlg::OnBnClickedButtonOutputConfig)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_WEB_PAGE_LOAD_TIME, &html_element_prop_dlg::OnTRBNThumbPosChangingSliderWebPageLoadTime)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_PROCESS_DELAY_TIME, &html_element_prop_dlg::OnTRBNThumbPosChangingSliderProcessDelayTime)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SAVE_HTML_SCRIPT, &html_element_prop_dlg::OnBnClickedButtonSaveHtmlScript)
	ON_BN_CLICKED(IDC_BUTTON_DATA_PROCESS, &html_element_prop_dlg::OnBnClickedButtonDataProcess)
	ON_BN_CLICKED(IDC_BUTTON_DATA_LEARNING, &html_element_prop_dlg::OnBnClickedButtonDataLearning)
	ON_BN_CLICKED(IDC_BUTTON_EXIT_SYSTEM, &html_element_prop_dlg::OnBnClickedButtonExitSystem)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL_EXIT_SYSTEM, &html_element_prop_dlg::OnBnClickedButtonCancelExitSystem)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_HTML_LOCATE_URL, &html_element_prop_dlg::OnNMDblclkListHtmlLocateUrl)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_HTML_LOCATE_URL, &html_element_prop_dlg::OnLvnDeleteitemListHtmlLocateUrl)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_HTML_LOCATE_URL, &html_element_prop_dlg::OnNMRClickListHtmlLocateUrl)
	ON_BN_CLICKED(IDC_BUTTON_PLUGINS, &html_element_prop_dlg::OnBnClickedButtonPlugins)
    ON_BN_CLICKED(IDC_BUTTON_HELP, &html_element_prop_dlg::OnBnClickedButtonHelp)
    ON_BN_CLICKED(IDC_BUTTON_XPATH_CLEAN, &html_element_prop_dlg::OnBnClickedButtonXpathClean)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_WEB_PAGE_LAYOUT, &html_element_prop_dlg::OnBnClickedButtonWebPageLayout)
END_MESSAGE_MAP()

// html_element_prop_dlg message handlers

#include "error_process_dlg.h"
LRESULT html_element_prop_dlg::on_error(WPARAM wparam, LPARAM lparam)
{
    LRESULT ret = ERROR_SUCCESS;
    LPCWSTR error_message; 
    error_process_dlg dlg; 
    
    do
    {
        error_message = (LPCWSTR)lparam; 

        dlg.set_error_message(error_message); 
        
        if (IDNO == dlg.DoModal())
        {
            ret = ERROR_ERRORS_ENCOUNTERED;
            break;
        }
    } while (FALSE);
    return ret;
}

LRESULT html_element_prop_dlg::on_xpath_config( WPARAM wparam, LPARAM lparam )
{
	receive_html_element_xpath(); 

	return ERROR_SUCCESS; 
}

LRESULT html_element_prop_dlg::clear_selection()
{
	LRESULT ret = ERROR_SUCCESS; 

	tree_elements.SelectItem( NULL ); 

	//for ( HTREEITEM hItem = tree_elements.GetRootItem(); hItem!=NULL; hItem = tree_elements.GetNextVisibleItem( hItem ) )
	//	if ( tree_elements.GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )
	//		tree_elements.SetItemState( hItem, 0, TVIS_SELECTED );

	return ret; 
}

LRESULT html_element_prop_dlg::on_work_stoppped( WPARAM wparam, LPARAM lparam )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		HTML_ACTION_OUTPUT_PARAM output_param; 

        output_param.action = NULL; 
		output_param.container = NULL; 
		output_param.context = NULL; 

		SetDlgItemText( IDC_BUTTON_EXECUTE, BUTTON_RUN_TEXT);
		GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( TRUE ); 
		script_running = FALSE; 
		on_locate_to_action( LOCATE_CURRENT_ACTION, ( LPARAM )&output_param ); 
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::on_update_html_action( WPARAM wparam, LPARAM lparam )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ACTION_OUTPUT_PARAM *output_param; 

	ASSERT( NULL != lparam ); 
	output_param = ( HTML_ACTION_OUTPUT_PARAM* )lparam; 
	if( wparam == UPDATE_ROOT_HTML_ACTION )
	{
		WCHAR file_name[ MAX_PATH ];

        ASSERT(NULL != output_param->context); 

		GetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, file_name, ARRAYSIZE( file_name ) ); 
		ret = update_html_script( file_name, 
			output_param->context->current_url.c_str() ); 
		
		if( locate_urls.GetItemCount() == 0 )
		{
			ASSERT( FALSE ); 
		}
		else
		{
			LPWSTR _locate_url; 

			do 
			{

				_locate_url = ( LPWSTR )locate_urls.GetItemData( 0 ); 
				if( NULL != _locate_url )
				{
					free( _locate_url ); 
				}

				_locate_url = ( LPWSTR )malloc( ( output_param->context->current_url.length() + 1 ) << 1 ); 
				if( NULL == _locate_url )
				{
					break; 
				}

				memcpy( _locate_url, 
					output_param->context->current_url.c_str(), 
					( ( output_param->context->current_url.length() + 1 ) << 1  ) ); 

				locate_urls.SetItemData( 0, ( DWORD_PTR )_locate_url ); 
			} while (FALSE );

		}
	}
	else
	{
		ASSERT( FALSE ); 
	}

	return ret; 
}

LRESULT WINAPI compare_html_element_action( HTML_ELEMENT_ACTION *_action, HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( _action->id != action->id )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}
	} while ( FALSE ); 
	return ret; 
}

LRESULT html_element_prop_dlg::find_html_action_item( HTML_ELEMENT_ACTION *action, HTREEITEM *item_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM item; 
	HTREEITEM sub_item; 
	HTREEITEM next_item; 
	vector< HTREEITEM > next_items; 
	HTML_ELEMENT_ACTION *_action; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != item_out ); 

		*item_out = NULL; 

		item = tree_elements.GetChildItem( TVI_ROOT ); 
		if( NULL == item )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			//ASSERT( FALSE ); 
			break; 
		}

		next_items.push_back( item ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				ret = ERROR_NOT_FOUND; 
				item = NULL; 
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 

			_action = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( item ); 
			if( ERROR_SUCCESS == compare_html_element_action( _action, action ) )
			{
				break; 
			}

			do 
			{
				next_item = tree_elements.GetNextSiblingItem( item ); 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = tree_elements.GetChildItem( item ); 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 

			} while ( FALSE ); 
		}

		*item_out = item; 
	}while( FALSE );

	return ret; 
}

#pragma optimize( "", off )
LRESULT html_element_prop_dlg::user_output_data( DATA_STORE *store, 
												   HTML_ELEMENT_ACTIONS *content)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != store ); 
		ASSERT( NULL != content ); 

#if 1
#define CHECK_USER_ACCESS_FREQUENCE 15
		if( 0 == ( script_info.get_data_time % CHECK_USER_ACCESS_FREQUENCE ) )
		{
			ULONG mode; 

#ifdef _DEBUG
			get_working_mode( &mode ); 
			if ( mode == 0 )
#endif //_DEBUG	
			{
				ret = process_current_user_access( GetSafeHwnd() ); 
				if( ret != ERROR_SUCCESS )
				{
					//break; 
				}
			}
		}
#endif //0

		DATA_INFO *datas = NULL; 

		do 
		{			
			datas = new DATA_INFO[ content->size() ]; 

			if( NULL == datas )
			{
				break; 
			}

			for( HTML_ELEMENT_ACTIONS::iterator it = content->begin(); 
				it != content->end(); 
				it ++ )
			{
				StringCchCopyW( datas[ it - content->begin() ].name, 
					ARRAYSIZE(datas[ it - content->begin() ].name), 
					( *it )->name.c_str() ); 

                {
                    wstring output_data; 

                    cat_output_data((*it), output_data);

                    datas[it - content->begin()].data = (LPVOID)output_data.c_str();
                    datas[it - content->begin()].size = output_data.size();

                    datas[it - content->begin()].index = it - content->begin();
                }
            }

			ret = call_plugin_data_handler( datas, content->size() ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while( FALSE ); 

		if( datas != NULL )
		{
			free( datas ); 
		}

		ret = output_data( store, 
			content ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::output_data_to_list(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;
    LRESULT _ret; 
    HTREEITEM _selected_item;
    HTREEITEM _sub_tree_item;
    HTREEITEM _new_sub_tree_item = NULL;

    HTREEITEM next_sub_tree_item;
    HTML_ELEMENT_ACTION *_config;
    //wstring::size_type url_begin;
    //wstring::size_type url_end;
    wstring data;
    CString item_text;
    ULONG url_count;

    do
    {
        ASSERT(NULL != action);
        //ASSERT(0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK));

        ret = find_html_action_item(action, &_selected_item);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        _sub_tree_item = tree_elements.GetChildItem(_selected_item);

        if (NULL == _sub_tree_item)
        {
            ret = ERROR_ERRORS_ENCOUNTERED;
            break;
        }

        //_sub_tree_item = tree_elements.GetNextSiblingItem( _sub_tree_item ); 

        //if( NULL == _sub_tree_item )
        //{
        //	_ret = ERROR_ERRORS_ENCOUNTERED; 
        //	break; 
        //}

        item_text = tree_elements.GetItemText(_sub_tree_item);
        ASSERT(0 <= item_text.Find(L"共"));

        _new_sub_tree_item = tree_elements.GetChildItem(_sub_tree_item);

        url_count = 0;
        _config = NULL;

#if 1
        for (vector<wstring>::iterator it = action->outputs.begin();
            it != action->outputs.end();
            it++)
        {
            data = (*it);
            url_count += 1;
#else
        url_begin = 0;
        for (; ; )
        {
            url_end = action->outputs.find(_URL_DELIM_CHAR, url_begin);
            if (url_end == string::npos)
            {
                break;
            }

            data = action->outputs.substr(url_begin, url_end - url_begin);
            url_begin = url_end + 1;
            url_count += 1;
#endif //0
            if (_new_sub_tree_item == NULL)
            {
                do
                {
                    _new_sub_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_NONE,
                        data.c_str(),
                        _sub_tree_item,
                        TVI_LAST);

                    if (_new_sub_tree_item == NULL)
                    {
                        break;
                    }

                    ret = allocate_html_element_action(&_config);
                    if (ERROR_SUCCESS != ret)
                    {
                        break;
                    }

                    ASSERT(NULL != _config);

                    _config->action = HTML_ELEMENT_ACTION_NONE;
                    _config->in_frame = FALSE;
                    _config->param = HTML_ELEMENT_ACTION_NONE_PARAM_LINK;
                    _config->xpath = L"";
                    //_config->url = (*it).c_str(); 
                    //_config->output = ( *it ).c_str(); 
                    _config->title = data.c_str();

                    ret = attach_element_config(_new_sub_tree_item, _config);

                    if (ERROR_SUCCESS != ret)
                    {
                        break;
                    }

                    _config = NULL;
                } while (FALSE);

                if (_config != NULL)
                {
                    detach_free_element_config(_new_sub_tree_item, _config);
                    _config = NULL;
                }

                _new_sub_tree_item = NULL;
            }
            else
            {
                _config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(_new_sub_tree_item);
                ASSERT(0 == wcscmp(_config->param.c_str(),
                    HTML_ELEMENT_ACTION_NONE_PARAM_LINK));

                _config->title = data.c_str();

                tree_elements.SetItemText(_new_sub_tree_item, data.c_str());
            }

            _new_sub_tree_item = tree_elements.GetNextSiblingItem(_new_sub_tree_item);
        }

        item_text.Format(HTML_ELEMENT_LIST_TITLE_FORMAT_TEXT, url_count);
        tree_elements.SetItemText(_sub_tree_item, item_text.GetBuffer());
        _config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(_sub_tree_item);
        ASSERT(NULL != _config);
        _config->title = item_text.GetBuffer();

        for (; ; )
        {
            if (_new_sub_tree_item == NULL)
            {
                break;
            }

            next_sub_tree_item = tree_elements.GetNextSiblingItem(_new_sub_tree_item);
            _ret = delete_tree_item(_new_sub_tree_item);
            _new_sub_tree_item = next_sub_tree_item;
        }
    } while (FALSE);
    return ret;
}

LRESULT html_element_prop_dlg::on_data_output( WPARAM wparam, LPARAM lparam )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ACTION_OUTPUT_PARAM *output_param; 
	HTML_ELEMENT_ACTION *html_command; 

	do 
	{
		switch( wparam )
		{
		case OUTPUT_TEXTS: 
			{
				HTML_ELEMENT_ACTIONS page_content; 
				ASSERT( NULL != lparam ); 

				output_param = (HTML_ACTION_OUTPUT_PARAM* )( PVOID )lparam; 
				html_command = output_param->container; 
				
				do 
				{
					ret = _get_container_content_info( html_command, 
						output_param->context, 
						&page_content ); 

					if( ERROR_SUCCESS != ret )
					{
						break; 
					}

					increment_script_run_time( &script_info, TRUE, FALSE ); 

					do
					{
						CONTAINER_INFO *container_info; 
						ret = get_html_content_info( html_command, &container_info ); 
						if( ret != ERROR_SUCCESS )
						{
                            dbg_print(MSG_ERROR, "can not found the page layout element(%s)\n", html_command->action.c_str() ); 
							break; 
						}

						do 
						{
							ret = check_data_store( &container_info->store ); 
							if( ret != ERROR_SUCCESS )
							{
								ASSERT( FALSE ); 
								break; 
							}

							ret = user_output_data(  &container_info->store, 
								&page_content ); 
						}while( FALSE );

						do 
						{
							LRESULT _ret; 
							HTML_ELEMENT_ACTIONS::iterator it; 

							for( it = page_content.begin(); 
								it != page_content.end(); 
								it ++ )
							{
								do 
								{
                                    _ret = output_data_to_list(*it);
                                    if (_ret != ERROR_SUCCESS)
                                    {
                                        //break;
                                    }

#if 0
									_ret = find_html_action_item( ( *it ), &tree_item ); 
									if( _ret != ERROR_SUCCESS )
									{
										break; 
									}

									_sub_tree_item = tree_elements.GetChildItem( tree_item ); 

									if( NULL == _sub_tree_item )
									{
										_ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}

									_sub_tree_item = tree_elements.GetChildItem( _sub_tree_item ); 

									if( NULL == _sub_tree_item )
									{
										_ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}

									//_sub_tree_item = tree_elements.GetChildItem( _sub_tree_item ); 
									//if( NULL == _sub_tree_item )
									//{
									//	break; 
									//}

									tree_elements.SetItemText( _sub_tree_item, ( *it )->outputs.c_str() ); 
									tree_elements.SelectItem( _sub_tree_item ); 
#endif //0
								}while( FALSE );
							}
						}while( FALSE );
						
						if( container_info->have_ui == FALSE )
						{
							break; 
						}

						ret = data_dlg.add_scrambled_data_info( &page_content ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						if( FALSE == data_dlg.IsWindowVisible() )
						{
							data_dlg.ShowWindow( SW_SHOW ); 
						}

					}while( FALSE ); 

					if( ret != ERROR_SUCCESS )
					{

					}
				}while( FALSE ); 

				release_container_content_info( &page_content ); 
			}
			break; 
		case OUTPUT_LINKS:
			{
            LRESULT _ret;
            wstring output_data;
            
            output_param = (HTML_ACTION_OUTPUT_PARAM*)(PVOID)lparam;
            html_command = output_param->action;

            if (html_command->outputs.size() > 0)
            {
                increment_script_run_time(&script_info, FALSE, TRUE);
            }

            do
            {
                CONTAINER_INFO *container_info;
                ret = get_html_content_info(html_command, &container_info);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                ret = check_data_store(&container_info->store);
                if (ret != ERROR_SUCCESS)
                {
                    ASSERT(FALSE);
                    break;
                }

                cat_output_data(html_command, output_data);

                ret = input_links(&container_info->store,
                    html_command->xpath.c_str(),
                    output_data.c_str() );

                if (ret != ERROR_SUCCESS)
                {
                    break;
                }
            } while (FALSE);

            _ret = output_data_to_list( html_command ); 
            if (_ret != ERROR_SUCCESS)
            {
                //break;
            }
        }
			break; 
        case OUTPUT_HTML_ELEMENTS:
        {
            LRESULT _ret;
            output_param = (HTML_ACTION_OUTPUT_PARAM*)(PVOID)lparam;
            html_command = output_param->action;

            if (html_command->outputs.size() > 0)
            {
                increment_script_run_time(&script_info, FALSE, TRUE);
            }

            _ret = output_data_to_list(html_command);
            if (_ret != ERROR_SUCCESS)
            {
                //break;
            }
        }
            break; 
		default:
			{
			}
		break; 
		}
	}while( FALSE ); 
	
	return ret; 
}
#pragma optimize( "", on )

INLINE LRESULT WINAPI compare_url_list_item( HTML_ELEMENT_ACTION *config, LPCWSTR url )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( 0 != wcscmp( config->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_LINK ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 != wcscmp( config->title.c_str(), url ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	} while ( FALSE );
	
	return ret; 
}

LRESULT html_element_prop_dlg::on_locate_to_action( WPARAM wparam, LPARAM lparam )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM item; 
	HTREEITEM sub_item; 
	HTREEITEM next_item; 
	vector< HTREEITEM > next_items; 
	HTML_ELEMENT_ACTION *action; 

	do 
	{
		ASSERT( NULL != lparam ); 

		switch( wparam )
		{
		case LOCATE_CURRENT_ACTION:
			{
				HTML_ACTION_OUTPUT_PARAM *output_param; 
				CString item_text; 
				output_param = ( HTML_ACTION_OUTPUT_PARAM* )lparam; 

				item = tree_elements.GetChildItem( TVI_ROOT ); 
				if( NULL == item )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					//ASSERT( FALSE ); 
					break; 
				}


				next_items.push_back( item ); 

				for( ; ; )
				{
					if( true == next_items.empty() )
					{
						break; 
					}

					item = *next_items.rbegin(); 

					next_items.pop_back(); 

					action = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( item ); 
					if( NULL == action )
					{
						ASSERT( FALSE ); 
					}
					else if( output_param->container != NULL 
						&& action->id == output_param->container->id )
					{
                        
                        CString text;

                        text = tree_elements.GetItemText(item); 

                        ASSERT(text.Find(L"网页布局") >= 0); 

                        text = action->title.c_str(); // OPENED_PAGE_LAYOUT_NODE_NAME;
                        text += L"(";
                        text += output_param->context->locate_url.c_str();
                        text += L")";

                        tree_elements.SetItemText(item, text); 

					}
                    else if(output_param->action != NULL
                        && action->id == output_param->action->id)
                    {
                        clear_selection();
                        tree_elements.SelectItem(item);
                        tree_elements.SetCheck(item, TRUE);
                    }
					else if( output_param->context != NULL 
						&& ERROR_SUCCESS == compare_url_list_item( action, output_param->context->locate_url.c_str() ) )
					{
						tree_elements.SetCheck( item, TRUE ); 
					}
					else
					{
						tree_elements.SetCheck( item, FALSE ); 
					}

					do 
					{
						next_item = tree_elements.GetNextSiblingItem( item ); 

						if( NULL == next_item )
						{
							break; 
						}

						next_items.push_back( next_item ); 
					} while ( FALSE ); 

					do 
					{
						sub_item = tree_elements.GetChildItem( item ); 

						if( NULL == sub_item )
						{
							break; 
						}

						next_items.push_back( sub_item ); 

					} while ( FALSE ); 
				}
			}
			break; 
		//case LOCATE_CURRENT_ACTION:
		//	{
		//		ELEMENT_CONFIG *action; 
		//		action = ( ELEMENT_CONFIG* )lparam; 

		//		if( action == &hilight_action )
		//		{
		//			break; 
		//		}

		//		item = tree_elements.GetChildItem( TVI_ROOT ); 
		//		if( NULL == item )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			//ASSERT( FALSE ); 
		//			break; 
		//		}


		//		next_items.push_back( item ); 

		//		for( ; ; )
		//		{
		//			if( true == next_items.empty() )
		//			{
		//				break; 
		//			}

		//			item = *next_items.rbegin(); 

		//			next_items.pop_back(); 

		//			data = ( PVOID )tree_elements.GetItemData( item ); 
		//			if( data == action )
		//			{
		//				clear_selection(); 
		//				tree_elements.SelectItem( item ); 
		//				tree_elements.SetCheck( item, TRUE ); 
		//				//break; 
		//			}
		//			else
		//			{
		//				tree_elements.SetCheck( item, FALSE ); 
		//			}

		//			do 
		//			{
		//				next_item = tree_elements.GetNextSiblingItem( item ); 

		//				if( NULL == next_item )
		//				{
		//					break; 
		//				}

		//				next_items.push_back( next_item ); 
		//			} while ( FALSE ); 

		//			do 
		//			{
		//				sub_item = tree_elements.GetChildItem( item ); 

		//				if( NULL == sub_item )
		//				{
		//					break; 
		//				}

		//				next_items.push_back( sub_item ); 

		//			} while ( FALSE ); 
		//		}
		//		break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

void html_element_prop_dlg::OnBnClickedButtonXpathValidate()
{
	LRESULT ret; 
	CString target_xpath; 
	// TODO: Add your control notification handler code here

	GetDlgItemText( IDC_EDIT_XPATH, target_xpath ); 

	//ret = g_browser_dlg->unhighlight_html_element(); 
	ret = on_hilight( 0, ( LPARAM )target_xpath.GetBuffer() ); 
		//g_html_script_config_dlg->highlight_html_element_show( target_xpath.GetBuffer() ); 
}

LRESULT html_element_prop_dlg::show_html_action_param_control( HTML_ELEMENT_ACTION_TYPE type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		switch( type )
		{
		case HTML_ELEMENT_ACTION_TYPE_CLICK:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"属性名称" ); 
			open_new_page.EnableWindow( TRUE ); 
			open_new_page.ShowWindow( SW_SHOW ); 
			contant_type.EnableWindow( FALSE ); 
			contant_type.ShowWindow( SW_HIDE ); 
			input_content.EnableWindow( FALSE ); 
			input_content.ShowWindow( SW_HIDE ); 
			field_name.EnableWindow( FALSE ); 
			field_name.ShowWindow( SW_HIDE ); 

			do
			{
				CWnd *ctrl; 
				ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 
				
				if( ctrl == NULL )
				{
					break; 
				}

				ctrl->ShowWindow( SW_HIDE ); 
				ctrl->EnableWindow( FALSE ); 
			}while( FALSE ); 

			break; 
		case HTML_ELEMENT_ACTION_TYPE_INPUT:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"输入内容" ); 
			open_new_page.EnableWindow( FALSE ); 
			open_new_page.ShowWindow( SW_HIDE ); 
			contant_type.EnableWindow( FALSE ); 
			contant_type.ShowWindow( SW_HIDE ); 
			input_content.EnableWindow( TRUE ); 
			input_content.ShowWindow( SW_SHOW ); 
			field_name.EnableWindow( FALSE ); 
			field_name.ShowWindow( SW_HIDE ); 
			do
			{
				CWnd *ctrl; 
				ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

				if( ctrl == NULL )
				{
					break; 
				}

				ctrl->ShowWindow( SW_HIDE ); 
				ctrl->EnableWindow( FALSE ); 
			}while( FALSE ); 
			break; 

		case HTML_ELEMENT_ACTION_TYPE_LOCATE:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"目标URL" ); 
			open_new_page.EnableWindow( FALSE ); 
			open_new_page.ShowWindow( SW_HIDE ); 
			contant_type.EnableWindow( FALSE ); 
			contant_type.ShowWindow( SW_HIDE ); 
			input_content.EnableWindow( TRUE ); 
			input_content.ShowWindow( SW_SHOW ); 
			field_name.EnableWindow( FALSE ); 
			field_name.ShowWindow( SW_HIDE ); 

			do
			{
				CWnd *ctrl; 
				ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

				if( ctrl == NULL )
				{
					break; 
				}

				ctrl->ShowWindow( SW_HIDE ); 
				ctrl->EnableWindow( FALSE ); 
			}while( FALSE ); 
			break; 
		case HTML_ELEMENT_ACTION_TYPE_SCRIPT:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"脚本内容" ); 
			open_new_page.EnableWindow( FALSE ); 
			open_new_page.ShowWindow( SW_HIDE ); 
			contant_type.EnableWindow( FALSE ); 
			contant_type.ShowWindow( SW_HIDE ); 
			input_content.EnableWindow( TRUE ); 
			input_content.ShowWindow( SW_SHOW ); 
			field_name.EnableWindow( FALSE ); 
			field_name.ShowWindow( SW_HIDE ); 
			do
			{
				CWnd *ctrl; 
				ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

				if( ctrl == NULL )
				{
					break; 
				}

				ctrl->ShowWindow( SW_HIDE ); 
				ctrl->EnableWindow( FALSE ); 
			}while( FALSE ); 
			break; 
		case HTML_ELEMENT_ACTION_TYPE_OUTPUT:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"输出类型" ); 
			open_new_page.EnableWindow( FALSE ); 
			open_new_page.ShowWindow( SW_HIDE ); 
			contant_type.EnableWindow( TRUE ); 
			contant_type.ShowWindow( SW_SHOW ); 
			input_content.EnableWindow( FALSE ); 
			input_content.ShowWindow( SW_HIDE ); 

			switch( contant_type.GetCurSel() )
			{
			case 1:
				{
					do
					{
						CWnd *ctrl; 
						ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

						if( ctrl == NULL )
						{
							break; 
						}

						ctrl->ShowWindow( SW_SHOW ); 
						ctrl->EnableWindow( TRUE ); 
					}while( FALSE ); 

					field_name.EnableWindow( TRUE ); 
					field_name.ShowWindow( SW_SHOW ); 
				}

				break; 
			case 0:
				do
				{
					CWnd *ctrl; 
					ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

					if( ctrl == NULL )
					{
						break; 
					}

					ctrl->ShowWindow( SW_HIDE ); 
					ctrl->EnableWindow( FALSE ); 
				}while( FALSE ); 
				field_name.EnableWindow( FALSE ); 
				field_name.ShowWindow( SW_HIDE ); 
				break; 
			default:
				do
				{
					CWnd *ctrl; 

					contant_type.SetCurSel( 1 ); 

					ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

					if( ctrl == NULL )
					{
						break; 
					}

					ctrl->ShowWindow( SW_SHOW ); 
					ctrl->EnableWindow( TRUE ); 
				}while( FALSE ); 
				field_name.EnableWindow( TRUE ); 
				field_name.ShowWindow( SW_SHOW ); 
	
				ASSERT( FALSE ); 
				break; 
			}
			break; 
		case HTML_ELEMENT_ACTION_TYPE_BACK:
			{
				CRichEditCtrl* edit; 

				SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"" ); 
				edit = ( CRichEditCtrl* )GetDlgItem( IDC_RICHEDIT2_PROPERTY_TEXT ); 
				edit->SetWindowText( L"" ); 

				open_new_page.EnableWindow( FALSE ); 
				open_new_page.ShowWindow( SW_HIDE ); 
				contant_type.EnableWindow( FALSE ); 
				contant_type.ShowWindow( SW_HIDE ); 
				input_content.EnableWindow( FALSE ); 
				input_content.ShowWindow( SW_HIDE ); 
				field_name.EnableWindow( FALSE ); 
				field_name.ShowWindow( SW_HIDE ); 
				do
				{
					CWnd *ctrl; 
					ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

					if( ctrl == NULL )
					{
						break; 
					}

					ctrl->ShowWindow( SW_HIDE ); 
					ctrl->EnableWindow( FALSE ); 
				}while( FALSE ); 
			}
			break; 
		case HTML_ELEMENT_ACTION_TYPE_HOVER:
			{
				CRichEditCtrl* edit; 

				SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"" ); 
				edit = ( CRichEditCtrl* )GetDlgItem( IDC_RICHEDIT2_PROPERTY_TEXT ); 
				edit->SetWindowText( L"" ); 

				open_new_page.EnableWindow( FALSE ); 
				open_new_page.ShowWindow( SW_HIDE ); 
				contant_type.EnableWindow( FALSE ); 
				contant_type.ShowWindow( SW_HIDE ); 
				input_content.EnableWindow( FALSE ); 
				input_content.ShowWindow( SW_HIDE ); 
				field_name.EnableWindow( FALSE ); 
				field_name.ShowWindow( SW_HIDE ); 
				do
				{
					CWnd *ctrl; 
					ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

					if( ctrl == NULL )
					{
						break; 
					}

					ctrl->ShowWindow( SW_HIDE ); 
					ctrl->EnableWindow( FALSE ); 
				}while( FALSE ); 
			}
			break; 
		case UNKNOWN_HTML_ELEMENT_ACTION_TYPE:
			SetDlgItemTextW( IDC_STATIC_ACTION_PARAMETER, L"" ); 
			open_new_page.EnableWindow( FALSE ); 
			open_new_page.ShowWindow( SW_HIDE ); 
			contant_type.EnableWindow( FALSE ); 
			contant_type.ShowWindow( SW_HIDE ); 
			input_content.EnableWindow( FALSE ); 
			input_content.ShowWindow( SW_HIDE ); 
			field_name.EnableWindow( FALSE ); 
			field_name.ShowWindow( SW_HIDE ); 
			do
			{
				CWnd *ctrl; 
				ctrl = GetDlgItem( IDC_STATIC_OUTPUT_FIELD_NAME ); 

				if( ctrl == NULL )
				{
					break; 
				}

				ctrl->ShowWindow( SW_HIDE ); 
				ctrl->EnableWindow( FALSE ); 
			}while( FALSE ); 
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

void html_element_prop_dlg::OnCbnSelendokComboAction()
{
	LRESULT ret; 
	INT32 cur_sel; 
	// TODO: Add your control notification handler code here

	cur_sel = element_actions.GetCurSel(); 
	
	if( cur_sel < 0 )
	{
		cur_sel = UNKNOWN_HTML_ELEMENT_ACTION_TYPE; 
	}

	switch( cur_sel )
	{
	case HTML_ELEMENT_ACTION_TYPE_INPUT:
		{
			input_content.SetWindowText( L"" ); 
		}
		break; 
	case HTML_ELEMENT_ACTION_TYPE_SCRIPT: 
	case HTML_ELEMENT_ACTION_TYPE_LOCATE:
		input_content.SetWindowText( L"" ); 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_OUTPUT:
		{
			HTML_ELEMENT_PROPERTIES properties; 
			contant_type.SetCurSel( 1 ); 

			ret = get_active_element_properties( &properties ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			field_name.SetWindowText(properties.field_name.c_str()); 
		}
		break; 
	default:
		break;
	}

	show_html_action_param_control( ( HTML_ELEMENT_ACTION_TYPE )cur_sel ); 
}

LRESULT html_element_prop_dlg::empty_tree_elements()
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM root_item = tree_elements.GetChildItem( TVI_ROOT );
	HTREEITEM sub_item; 

	while( root_item != NULL )
	{
		sub_item = tree_elements.GetNextSiblingItem( root_item );
		tree_elements.DeleteItem(sub_item); 
	}

	return ret; 
}

//LRESULT html_element_prop_dlg::IterateChildNodes(MSXML::IXMLDOMNodePtr  pNode, 
//											  CTreeCtrl *pTree, 
//											  HTREEITEM hParent)
//{
//	_bstr_t bstrNodeName;
//	HTREEITEM hCurrentItem;
//
//	if ( pTree )
//	{
//		if ( pNode )
//		{
//			CString strOutput;
//			bstrNodeName = pNode->nodeName;
//
//			//
//			// Find out the node type (as a string).
//			//
//			_bstr_t bstrNodeType;
//			bstrNodeType = pNode->nodeTypeString;
//			CString strType;
//			strType.Format(_T("%s"),bstrNodeType.operator TCHAR*());
//
//
//			MSXML::DOMNodeType eEnum;
//			pNode->get_nodeType(&eEnum);
//
//			int nImg = 0;
//			if ( eEnum == MSXML::NODE_TEXT )
//			{
//				_bstr_t bstrValue;
//				bstrValue  =pNode->text;
//				strOutput.Format(_T("%s"),bstrValue.operator TCHAR*());
//
//
//				nImg = 2;
//			}
//			else if ( eEnum == MSXML::NODE_COMMENT )
//			{
//				_variant_t vValue;
//				vValue = pNode->nodeValue;
//
//				if ( vValue.vt == VT_BSTR )
//					strOutput.Format(_T("%s"),V_BSTR(&vValue));
//				else
//					strOutput.Format(_T("Unknown comment type"));
//
//
//				nImg = 3;
//			}
//			else if ( eEnum == MSXML::NODE_PROCESSING_INSTRUCTION )
//			{
//				strOutput.Format(_T("%s"), bstrNodeName.operator TCHAR*());
//				nImg = 4;
//			}
//			else if ( eEnum == MSXML::NODE_ELEMENT )
//			{
//				strOutput.Format(_T("%s"), bstrNodeName.operator TCHAR*());
//				nImg = 5;
//			}
//			else
//			{
//				// 
//				// Other types, include the type name.
//				//
//				if(_tcsicmp( bstrNodeName.operator const TCHAR*(),_T("#document"))==0 && strType.CompareNoCase(_T("document"))==0)
//				{
//					strOutput.Format(_T("%04u-%02u-%02u %02u:%02u:%02u"),m_stCurrent.wYear, m_stCurrent.wMonth,m_stCurrent.wDay,m_stCurrent.wHour,m_stCurrent.wMinute,m_stCurrent.wSecond);
//				}
//				else
//				{
//					strOutput.Format(_T("%s - %s"), bstrNodeName.operator TCHAR*(),strType);
//				}
//
//			}
//
//
//			//
//			// Add it to the tree view.
//			//
//			TVINSERTSTRUCT tvItem;
//			tvItem.hParent = hParent ? hParent : TVI_ROOT;
//			tvItem.hInsertAfter = TVI_LAST;
//			tvItem.item.iImage = nImg;
//			tvItem.item.iSelectedImage = nImg;
//			tvItem.item.pszText = strOutput.GetBuffer(MAX_PATH);
//			tvItem.item.cchTextMax = MAX_PATH;
//			tvItem.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
//			hCurrentItem = pTree->InsertItem(&tvItem);
//			strOutput.ReleaseBuffer();
//
//			IterateAttibutes(pNode, pTree, hCurrentItem);
//		}
//	}
//
//	//
//	// Any child nodes of this node need displaying too.
//	//
//
//	MSXML::IXMLDOMNodePtr  pChild;
//	pChild=pNode->firstChild;
//	while( pChild )
//	{
//		IterateChildNodes(pChild,pTree,hCurrentItem);
//		pChild = pChild->nextSibling;
//
//
//	}
//
//	//
//	// Ensure after all of that, the item is expanded!
//	//
//	pTree->Expand(hCurrentItem,TVE_EXPAND);
//	return true;
//}

LRESULT WINAPI get_tree_item_text( ULONG flags,  
                                  ULONG id, 
								  LPCWSTR title, 
								  LPCWSTR text, 
								  LPCWSTR xpath, 
								  LPCWSTR content_type, 
								  wstring &item_text )
{
#define MAX_HTML_ID_TEXT 64
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR _text[ MAX_HTML_ID_TEXT]; 

	do 
	{
        ASSERT(id != INVALID_JUMP_TO_ID);

        if (HTML_ELEMENT_ACTION_UI_SHOW_ID & flags)
        {
            StringCchPrintfW(_text, ARRAYSIZE(_text), L"%d", id);

            item_text = L"[";
            item_text += _text;
            item_text += L"]";
        }
        else
        {
            item_text = L"";
        }

		item_text += title; 
		if( wcslen( text ) > 0 )
		{
			item_text += L":"; 
			item_text += text; 
		}

		if( wcslen( xpath ) > 0 )
		{
			item_text += L":"; 
			item_text += xpath; 
		}

		if( wcslen( content_type ) > 0 )
		{
			item_text += L":"; 
			item_text += content_type; 
		}
	}while( FALSE );

	return ret; 
}

//DWORD dwpos = GetMessagePos(); 
//TVHITTESTINFO ht = {0}; 
//ht.pt.x = GET_X_LPARAM(dwpos);
//ht.pt.y = GET_Y_LPARAM(dwpos);
//::MapWindowPoints(HWND_DESKTOP,pNMHDR->hwndFrom,&ht.pt,1); 
////把屏幕坐标转换成控件坐标
//TreeView_HitTest(pNMHDR->hwndFrom,&ht); 
////确定点击的是哪一项
//strOpcServerName = m_OpcServerList.GetItemText(ht.hItem); 

LRESULT html_element_prop_dlg::get_container_element( HTREEITEM item, HTREEITEM *container_item ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	HTREEITEM find_item = NULL; 
	BOOLEAN found = FALSE; 

	do 
	{
		if( NULL != container_item )
		{
			*container_item = NULL; 
		}

		if( NULL == item )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( item ); 
		if( NULL == config )
		{
			break; 
		}

		do 
		{
            if (ERROR_SUCCESS == check_page_layout_node(config))
            {
                find_item = item;
                break;
            }

			find_item = item; 
			for( ; ; )
			{
				do 
				{
					config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( find_item ); 
					if( NULL == config )
					{
						break; 
					}

					if(ERROR_SUCCESS == check_page_layout_node(config))
					{
						found = TRUE; 
						break; 
					}
				}while( FALSE ); 

				if( TRUE == found )
				{
					break; 
				}

				find_item = tree_elements.GetParentItem( find_item ); 
				if( NULL == find_item )
				{
					//ret = ERROR_NOT_FOUND; 
					break; 
				}
			}

			if( find_item != NULL )
			{
				break; 
			}

			find_item = item; 
			for( ; ; )
			{
				do 
				{
					config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( find_item ); 
					if( NULL == config )
					{
						break; 
					}

					if(ERROR_SUCCESS == check_page_layout_node(config))
					{
						found = TRUE; 
						break; 
					}
				}while( FALSE );

				if( TRUE == found )
				{
					break; 
				}

				find_item = tree_elements.GetChildItem( find_item ); 
				if( NULL == find_item )
				{
					break; 
				}
			}
		}while( FALSE );

		if( NULL != find_item )
		{
			if( NULL != container_item )
			{
				*container_item = find_item; 
			}
		}
		else
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI add_output_content( HTML_ELEMENT_ACTION *config, 
								  HTML_PAGE_CONTENT *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *content = NULL; 

	do 
	{
		ASSERT( NULL != config ); 

		if( 0 != wcscmp( HTML_ELEMENT_ACTION_OUTPUT, 
			config->action.c_str() ) 
			|| 0 != wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, 
			config->param.c_str() ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		content = new HTML_ELEMENT_CONTENT(); 
		if( NULL == content )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		content->action = config; 
		content->name = config->name; 
		content->xpath = config->xpath; 

		page_content->element_contents.push_back( content ); 
		content = NULL; 
	}while( FALSE );
	
	if( NULL != content )
	{
		delete content; 
	}

	return ret; 
}

LRESULT WINAPI remove_output_content( HTML_ELEMENT_ACTION *config, 
									 HTML_PAGE_CONTENT *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *content = NULL; 
	HTML_ELEMENT_CONTENTS_ITERATOR it; 

	do 
	{
		ASSERT( NULL != config ); 

		for( ; ; )
		{
			for( it = page_content->element_contents.begin(); 
				it != page_content->element_contents.end(); 
				it ++ )
			{
				if( ( *it )->action !=  config )
				{
					break; 
				}
			}

			if( it == page_content->element_contents.end())
			{
				break; 
			}

			delete ( *it ); 
			page_content->element_contents.erase( it ); 
		}
	}while( FALSE );

	return ret; 
}

#if 0
LRESULT WINAPI modify_output_content( ELEMENT_CONFIG *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_CONTENT *content = NULL; 
	HTML_ELEMENT_CONTENTS_ITERATOR it; 

	do 
	{
		ASSERT( NULL != config ); 

		for( it = output_content.element_contents.begin(); 
			it != output_content.element_contents.end(); 
			it ++ )
		{
			if( ( *it )->id ==  config->id )
			{
				//( *it )
			}
		}
	}while( FALSE );

	return ret; 
}
#endif //0

LRESULT WINAPI check_html_element_action( LPCWSTR ui_text )
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 

	do 
	{
		if( 0 != wcscmp( ui_text, HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT ) 
			&& 0 != wcscmp( ui_text, HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT ) 
			&& 0 != wcscmp( ui_text, HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT ) )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}
	} while ( FALSE );

	return ret; 
}

#if 0
LRESULT html_element_prop_dlg::add_html_element_action()
{
    LRESULT ret;
    LRESULT _ret;
    HRESULT hr;
    IHTMLElement *html_element;
    wstring element_title;
    HTML_ELEMENT_ACTION *config = NULL;
    HTML_ELEMENT_ACTION *parent_config;
    wstring element_text;
    CString content_type;
    ULONG action_type;
    CString _xpath;

    // TODO: Add your control notification handler code here

    do
    {
        CString action_text;
        HTREEITEM selected_item;
        HTREEITEM new_tree_item;
        HTREEITEM container_item;

        {
            LRESULT _ret;
#ifdef NO_SUPPORT_DELAY_TIME_ADJUST
            _ret = save_html_action_global_config();
            if (_ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "error\n");
            }
#endif //NO_SUPPORT_DELAY_TIME_ADJUST
        }

        if (configure_state & HTML_INSTRUCTION_SET_JUMP_TO)
        {
            MessageBox(L"请鼠标左键单击要跳转至的目标条目\n");
            break;
        }

        ret = allocate_element_config(&config);
        if (ERROR_SUCCESS != ret)
        {
            break;
        }

        ASSERT(NULL != config);

        //如果这个PATH的路径以前有，则进行替换
        selected_item = tree_elements.GetSelectedItem();
        if (selected_item == NULL)
        {
            //selected_item = tree_elements.GetRootItem(); 
        }

        ret = get_container_element(selected_item,
            &container_item);

        if (ret != ERROR_SUCCESS)
        {
            ASSERT(NULL == container_item);
        }
        else
        {
            ASSERT(NULL != container_item);
        }

        element_actions.GetWindowText(action_text);
        if (ERROR_SUCCESS == check_html_element_action(action_text.GetBuffer()))
        {
            GetDlgItemText(IDC_EDIT_TEXT_XPATH, _xpath);

            if (_xpath.GetLength() == 0)
            {
                MessageBox(L"请输入HTML元素的路径");
                free_element_config(config);
                config = NULL;
                break;
            }

            if (element == NULL)
            {
                free_element_config(config);
                config = NULL;
                break;
            }

            html_element = (IHTMLElement*)element;

            hr = get_html_element_title(html_element, &element_title);
            if (FAILED(hr))
            {
                //break; 
            }

            do
            {
                HRESULT hr;
                wstring html_text;
                IHTMLElement *html_element;

                config->html_text = L"";

                html_element = (IHTMLElement*)element;

                hr = get_html_dom_element_text(html_element, html_text);
                if (FAILED(hr))
                {
                    break;
                }

                config->html_text = html_text.c_str();
            } while (FALSE);
        }

        GetDlgItemText(IDC_COMBO_CONTANT_TYPE, content_type);

        if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT))
        {
            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID,, 
                config->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            config->title = element_title;
            config->action = HTML_ELEMENT_ACTION_OUTPUT;
            action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT;

            if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT))
            {
                config->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT;

                {
                    CString text_field_name;
                    field_name.GetWindowText(text_field_name);
                    if (text_field_name.GetLength() == 0)
                    {
                        ret = ERROR_ERRORS_ENCOUNTERED;
                        MessageBox(L"", L"请输入字段名称", 0);
                        break;
                    }

                    if (0 == text_field_name.Compare(DEFAULT_HTML_ELEMENT_TEXT_FIELD_NAME))
                    {
                        text_field_name.Format(L"field%u", config->id);
                    }

                    config->name = text_field_name.GetBuffer();
                }
            }
            else if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT))
            {
                config->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK;
            }
            //HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE_UI_TEXT
            else
            {
                ASSERT(FALSE);
            }
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_INPUT_UI_TEXT))
        {
            CString _input_content;
            _ret = get_tree_item_text(
                config->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            config->title = element_title;

            input_content.GetWindowText(_input_content);

            config->action = HTML_ELEMENT_ACTION_INPUT;
            action_type = HTML_ELEMENT_ACTION_TYPE_INPUT;
            config->param = L"value";
            config->output = _input_content;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_CLICK_UI_TEXT))
        {
            config->action = HTML_ELEMENT_ACTION_CLICK;
            action_type = HTML_ELEMENT_ACTION_TYPE_CLICK;

            _ret = get_tree_item_text(
                config->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            config->title = element_title;

            if (0 != open_new_page.GetCheck())
            {
                config->param = HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE;
            }
            else
            {
                config->param = L"";
            }
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_HOVER_UI_TEXT))
        {
            _ret = get_tree_item_text(
                config->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            config->title = element_title;
            config->action = HTML_ELEMENT_ACTION_HOVER;
            config->param = L"";

            action_type = HTML_ELEMENT_ACTION_TYPE_HOVER;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT))
        {
            CString _input_content;

            config->action = HTML_ELEMENT_ACTION_SCRIPT;

            input_content.GetWindowText(_input_content);
            config->param = _input_content;
            config->title = L"(";
            config->title += _input_content.GetBuffer();
            config->title += L")";
            config->in_frame = FALSE;

            _ret = get_tree_item_text(
                config->id,
                config->title.c_str(),
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            action_type = HTML_ELEMENT_ACTION_TYPE_SCRIPT;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT))
        {
            config->action = HTML_ELEMENT_ACTION_BACK;

            _ret = get_tree_item_text(
                config->id,
                L"",
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            config->title = L"";

            new_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_BACK,
                element_text.c_str(),
                container_item,
                TVI_LAST);

            action_type = HTML_ELEMENT_ACTION_TYPE_BACK;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT))
        {
            CString _input_content;

            config->action = HTML_ELEMENT_ACTION_LOCATE;

            input_content.GetWindowText(_input_content);
            config->param = _input_content;
            config->title = L"(";
            config->title += _input_content.GetBuffer();
            config->title += L")";
            config->in_frame = FALSE;

            _ret = get_tree_item_text(
                config->id,
                config->title.c_str(),
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            new_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_LOCATE,
                element_text.c_str(),
                container_item, TVI_LAST);

            action_type = HTML_ELEMENT_ACTION_TYPE_LOCATE;
        }
        else
        {
            ASSERT(FALSE);
            ret = ERROR_ERRORS_ENCOUNTERED;
            break;
        }

        config->xpath = _xpath;

        //if( NULL == tree_elements.GetChildItem( TVI_ROOT ) 
        //	&& ERROR_SUCCESS != check_no_html_element_action( config ) )
        {
            if (0 == locate_urls.GetItemCount())
            {
                locate_urls.InsertItem(0, g_html_script_config_dlg->m_WebBrowser.get_loading_url().c_str());
                locate_urls.SetItemText(0, 1, L"-1");
            }
            //else
            //{
            //	locate_urls.SetItemText( 0, 0, g_html_script_config_dlg->m_WebBrowser.get_loading_url().c_str() ); 
            //}
        }

        do
        {
            if (NULL != container_item
                && configure_state & HTML_INSTRUCTION_CONTENT_PROCESS)
            {
                HTML_ELEMENT_ACTION *container_action;

                do
                {
                    container_action = (HTML_ELEMENT_ACTION*)tree_elements.GetItemData(container_item);
                    if (container_action == NULL
                        || container_action->parent_item == NULL
                        || 0 != wcscmp(container_action->parent_item->action.c_str(),
                            HTML_ELEMENT_ACTION_OUTPUT)
                        || 0 != wcscmp(container_action->parent_item->param.c_str(),
                            HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK))
                    {
                        new_tree_item = insert_tree_item(action_type,
                            element_text.c_str(),
                            NULL, TVI_LAST);
                        break;
                    }

                    new_tree_item = insert_tree_item(action_type,
                        element_text.c_str(),
                        container_item,
                        TVI_LAST);

                } while (FALSE);

                break;
            }

            {
                CString properities_text;
                GetDlgItemText(IDC_RICHEDIT2_PROPERTY_TEXT, properities_text);
                config->properties = properities_text.GetBuffer();
            }

            new_tree_item = insert_tree_item(action_type,
                element_text.c_str(),
                NULL,
                TVI_LAST);
        } while (FALSE);

        if (NULL == new_tree_item)
        {
            ULONG error_code;
            error_code = GetLastError();
            dbg_print(MSG_ERROR, "insert the tree item error\n");
            free_element_config(config);
            config = NULL;
            break;
        }

        ret = attach_element_config(new_tree_item, config);
        if (ERROR_SUCCESS != ret)
        {
            break;
        }
    }
    return ret;
}
#endif //0
/*************************************************
通过WEB浏览器中页面所有选择的HTML元素来进行输出处理
**************************************************/
LRESULT WINAPI get_action_desc(HTML_ELEMENT_ACTION *action, wstring &desc )
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        desc = L""; 

        if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT))
        {
            desc = L"输出"; 
            if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT))
            {
                desc += L":文本";
            }
            else if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK))
            {
                desc += L":超链接";
            }
            else
            {
                ASSERT(FALSE); 
                break;
            }
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_CLICK))
        {
            desc = L"点击";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_CLICK))
        {
            desc = L"点击";
            if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE))
            {
                desc += L":新网页";
            }
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_INPUT))
        {
            desc = L"输入";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_BACK))
        {
            desc = L"上一页";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_JUMP))
        {
            desc = L"跳转";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_LOCATE))
        {
            desc = L"打开网址";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_HILIGHT))
        {
            desc = L"高亮";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_HOVER))
        {
            desc = L"模拟鼠标移动";
        }
        else if (0 == wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_START))
        {
            desc = L"开始";
        }
        else
        {
            assert(FALSE); 
            ret = ERROR_INVALID_PARAMETER; 
            break;
        }

    } while (FALSE);
    return ret;
}

LRESULT WINAPI get_html_element_attrs(IHTMLElement *element, HTML_ELEMENT_ATTRIBUTES &attris)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
    } while (FALSE);

    return ret;
}


LRESULT WINAPI get_html_element_href(IHTMLElement *element, wstring &url )
{
    LRESULT ret = ERROR_SUCCESS;
    HRESULT hr; 
    _bstr_t text; 
    IHTMLElement *_html_element;
    IHTMLAnchorElementPtr link_element;
    LPCWSTR temp_text;

    do
    {
        url.clear(); 

        _html_element = element; 

        hr = _html_element->QueryInterface(IID_IHTMLAnchorElement, (void**)&link_element);
        if (hr != S_OK)
        {
            ret = hr; 
            break;
        }

        link_element->get_href(text.GetAddress());

        temp_text = text.operator wchar_t*();
        if (NULL == temp_text)
        {
            break;
        }

        dbg_print(MSG_INFO, "url %ws\n", temp_text);
        url = temp_text; 
    } while (FALSE);
    return ret;
}


LRESULT WINAPI get_html_element_info(IHTMLElement *element, HTML_ELEMENT_INFO *info)
{
    LRESULT ret = ERROR_SUCCESS; 

    do 
    {
        ASSERT(NULL != element); 
        ASSERT(NULL != info); 

        ret = get_html_element_xpath(element, info->xpath); 
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret); 
            break;
        }

        ret = get_html_element_attrs(element, info->attrs); 
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
            //break;
        }

        ret = get_html_element_href(element, info->href);
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
            //break;
        }

        ret = get_html_element_content(element, &info->content);
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
            //break;
        }

    } while ( FALSE );

    return ret; 
}

LRESULT WINAPI get_internal_links(IHTMLElement *element, HTML_ELEMENT_INFOS &links)
{
    LRESULT ret = ERROR_SUCCESS; 
    LRESULT _ret; 
    HRESULT hr;

    INT32 i; 
    HTML_ELEMENT_INFO *info = NULL;
    IDispatchPtr disp;
    IHTMLElementCollectionPtr elements;
    IHTMLElementPtr _element; 
    IHTMLAnchorElementPtr link_element;
    LONG element_count;

    _variant_t id;
    _variant_t index;

    do
    {
        if (NULL == element)
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

        do
        {
            hr = element->QueryInterface(IID_IHTMLAnchorElement, (PVOID*)&link_element);
            if (SUCCEEDED(hr)
                && NULL != link_element)
            {
                do
                {
                    info = new HTML_ELEMENT_INFO();
                    if (info == NULL)
                    {
                        break;
                    }

                    _ret = get_html_element_info(element, info);
                    if (_ret != ERROR_SUCCESS)
                    {
                    }

                    links.push_back(info);
                    info = NULL;
                } while (FALSE);

                if (info != NULL)
                {
                    delete info;
                }

                break;
            }

            do
            {
                hr = element->get_all(&disp);
                if (FAILED(hr))
                {
                    ret = hr;
                    break;
                }

                if (NULL == disp)
                {
                    ret = ERROR_NOT_FOUND;
                    break;
                }

                hr = disp->QueryInterface(IID_IHTMLElementCollection, (PVOID*)&elements);
                if (FAILED(hr))
                {
                    ret = ERROR_NOT_FOUND;
                    break;
                }

                if (NULL == elements)
                {
                    ret = ERROR_NOT_FOUND;
                    break;
                }

                hr = elements->get_length(&element_count);
                if (FAILED(hr))
                {
                    ret = hr;
                    break;
                }

                for (i = 0; i < element_count; i++)
                {
                    do
                    {
                        info = NULL; 

                        V_VT(&id) = VT_I4;
                        V_I4(&id) = i;
                        V_VT(&index) = VT_I4;
                        V_I4(&index) = 0;

                        hr = elements->item(id, index, &disp);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (NULL == disp)
                        {
                            break;
                        }

                        hr = disp->QueryInterface(IID_IHTMLAnchorElement, (PVOID*)&link_element);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (NULL == link_element)
                        {
                            break;
                        }

                        hr = disp->QueryInterface(IID_IHTMLElement, (PVOID*)&_element);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (NULL == _element)
                        {
                            break;
                        }

                        info = new HTML_ELEMENT_INFO();
                        if (info == NULL)
                        {
                            break;
                        }

                        _ret = get_html_element_info(_element, info);
                        if (_ret != ERROR_SUCCESS)
                        {
                        }

                        links.push_back(info);
                        info = NULL;
                    } while (FALSE); 

                    if (NULL != info)
                    {
                        delete info;
                    }
                }
            } while (FALSE);
        } while (FALSE);
    } while (FALSE); 

    if (NULL != info)
    {
        delete info;
    }

    return ret;
}

LRESULT html_element_prop_dlg::get_html_elements_info( HTML_ELEMENT_ACTION *action, HTML_ELEMENT_INFOS &infos, ULONG flags )
{
    LRESULT ret = ERROR_SUCCESS; 
    LRESULT _ret; 
    HTML_ELEMENT_GROUP elements; 

    ASSERT(NULL != action ); 

    do
    {
        ret = get_html_elements(action->xpath.c_str(), elements, TRUE);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        if (elements.size() == 0)
        {
            dbg_print(MSG_ERROR, "can not get the link list\n");
            break;
        }

        for (HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = elements.begin(); it != elements.end(); it++)
        {
            do
            {
                HTML_ELEMENT_INFO *info = NULL;

                if (flags == INNER_LINK)
                {

                    _ret = get_internal_links((*it), infos);
                    if (_ret != ERROR_SUCCESS)
                    {
                        dbg_print(MSG_ERROR, "get html element internal links error %d\n", ret);
                        break;
                    }

                    break; 
                }

                do
                {
                    info = new HTML_ELEMENT_INFO();
                    if (info == NULL)
                    {
                        break;
                    }

                    _ret = get_html_element_info((*it), info);
                    if (_ret != ERROR_SUCCESS)
                    {
                    }

                    infos.push_back(info);
                    info = NULL;
                } while (FALSE); 

                if (info != NULL)
                {
                    delete info;
                }
            } while (FALSE);
        }
    } while (FALSE);

    return ret; 
}

LRESULT html_element_prop_dlg::_get_html_element_action_info_from_ui(HTML_ELEMENT_ACTION *action, 
    wstring &element_text, 
    ULONG &action_type)
{
    LRESULT ret = ERROR_SUCCESS;
    LRESULT _ret; 
    wstring element_title;
    //wstring element_text;
    CString content_type;
    CString action_text; 
    CString _xpath; 

    do
    {
        element_actions.GetWindowText(action_text);
        GetDlgItemText(IDC_COMBO_CONTANT_TYPE, content_type);
        GetDlgItemText(IDC_EDIT_TEXT_XPATH, _xpath);

        action_type = UNKNOWN_HTML_ELEMENT_ACTION_TYPE; 

        if (ERROR_SUCCESS == check_html_element_action(action_text.GetBuffer()))
        {
            if (_xpath.GetLength() == 0)
            {
                //if (0 != _wcsicmp(content_type.GetBuffer(),
                //    HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT)
                //    || 0 != _wcsicmp(action_text.GetBuffer(),
                //        HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT))
                //{
                //    _xpath = L"HTML|BODY"; 
                //}
                //else
                {
                    ret = ERROR_INVALID_PARAMETER;
                    MessageBox(L"请输入HTML元素的路径");
                    free_element_config(action);
                    action = NULL;
                    break;
                }
            }

            //通过界面配置，不可以直接取当前的HTML ELEMENT属性，因为不定是当前ELEMENT
            do
            {
                HRESULT hr;
                wstring html_text;
                IHTMLElement *html_element;

                if (element == NULL)
                {
                    //free_element_config(action);
                    //action = NULL;
                    break;
                }

                html_element = (IHTMLElement*)element;

                hr = get_html_element_title(html_element, &element_title);
                if (FAILED(hr))
                {
                    //break; 
                }

                action->html_text = L"";

                html_element = (IHTMLElement*)element;

                hr = get_html_dom_element_text(html_element, html_text);
                if (FAILED(hr))
                {
                    break;
                }

                action->html_text = html_text.c_str();
            } while (FALSE);
        }

        action->xpath = _xpath; 

        if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT))
        {
            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID, 
                action->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            action->title = element_title;
            action->action = HTML_ELEMENT_ACTION_OUTPUT;
            action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT;

            if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT))
            {
                action->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT;

                {
                    CString text_field_name;
                    field_name.GetWindowText(text_field_name);
                    if (text_field_name.GetLength() == 0)
                    {
                        ret = ERROR_ERRORS_ENCOUNTERED;
                        MessageBox(L"请输入字段名称", L"", 0);
                        break;
                    }

                    if (0 == text_field_name.Compare(DEFAULT_HTML_ELEMENT_TEXT_FIELD_NAME))
                    {
                        text_field_name.Format(L"field%u", action->id);
                    }

                    action->name = text_field_name.GetBuffer();
                }
            }
            else if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT))
            {
                action->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK;
            }
            else if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE_UI_TEXT))
            {
                action->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE; 

                CString text_field_name;
                field_name.GetWindowText(text_field_name);
                if (text_field_name.GetLength() == 0)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    MessageBox(L"请输入(images目录下的)图像保存目录", L"", 0);
                    break;
                }

                if (0 == text_field_name.Compare(DEFAULT_HTML_ELEMENT_TEXT_FIELD_NAME))
                {
                    text_field_name.Format(L"image%u", action->id);
                }

                action->name = text_field_name.GetBuffer();
            }
            else
            {
                ASSERT(FALSE);
            }
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_INPUT_UI_TEXT))
        {
            CString _input_content;
            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID,
                action->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            action->title = element_title;

            input_content.GetWindowText(_input_content);

            action->action = HTML_ELEMENT_ACTION_INPUT;
            action_type = HTML_ELEMENT_ACTION_TYPE_INPUT;
            action->param = L"value";
            action->outputs.clear(); 
            action->outputs.push_back( wstring(_input_content) );
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_CLICK_UI_TEXT))
        {
            action->action = HTML_ELEMENT_ACTION_CLICK;
            action_type = HTML_ELEMENT_ACTION_TYPE_CLICK;

            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID,
                action->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            action->title = element_title;

            if (0 != open_new_page.GetCheck())
            {
                action->param = HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE;
            }
            else
            {
                action->param = L"";
            }
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_HOVER_UI_TEXT))
        {
            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID, 
                action->id,
                element_title.c_str(),
                action_text.GetBuffer(),
                _xpath.GetBuffer(),
                content_type.GetBuffer(),
                element_text);

            if (ERROR_SUCCESS != _ret)
            {
            }

            action->title = element_title;
            action->action = HTML_ELEMENT_ACTION_HOVER;
            action->param = L"";

            action_type = HTML_ELEMENT_ACTION_TYPE_HOVER;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT))
        {
            CString _input_content;

            action->action = HTML_ELEMENT_ACTION_SCRIPT;

            input_content.GetWindowText(_input_content);
            action->param = _input_content;
            action->title = L"(";
            action->title += _input_content.GetBuffer();
            action->title += L")";
            action->in_frame = FALSE;

            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID, 
                action->id,
                action->title.c_str(),
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            action_type = HTML_ELEMENT_ACTION_TYPE_SCRIPT;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT))
        {
            action->action = HTML_ELEMENT_ACTION_BACK;

            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID, 
                action->id,
                L"",
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            action->title = L"";

            action_type = HTML_ELEMENT_ACTION_TYPE_BACK;
        }
        else if (0 == _wcsicmp(action_text.GetBuffer(),
            HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT))
        {
            CString _input_content;

            action->action = HTML_ELEMENT_ACTION_LOCATE;

            input_content.GetWindowText(_input_content);
            action->param = _input_content;
            action->title = L"(";
            action->title += _input_content.GetBuffer();
            action->title += L")";
            action->in_frame = FALSE;

            _ret = get_tree_item_text(
                HTML_ELEMENT_ACTION_UI_SHOW_ID,
                action->id,
                action->title.c_str(),
                action_text.GetBuffer(),
                L"",
                L"",
                element_text);

            action_type = HTML_ELEMENT_ACTION_TYPE_LOCATE;
        }
        else
        {
            ASSERT(FALSE);
            ret = ERROR_ERRORS_ENCOUNTERED;
            break;
        }
    } while (FALSE);

    return ret;
}

LRESULT get_html_elemen_info_text(ULONG action_type, 
    HTML_ELEMENT_ACTION *action, 
    HTML_ELEMENT_INFO *info, 
    wstring &text)
{
    LRESULT ret = ERROR_SUCCESS; 
    
    do 
    {
        text = info->xpath;

        switch (action_type)
        {
        case HTML_ELEMENT_ACTION_TYPE_CLICK:
            break;

        case HTML_ELEMENT_ACTION_TYPE_OUTPUT:
            if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK))
            {
                text = info->href;
            }
            else if (0 == wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT))
            {
                text = info->content;
            }
            break;

        case HTML_ELEMENT_ACTION_TYPE_INPUT:
            break;

        case HTML_ELEMENT_ACTION_TYPE_BACK:
            break;

        case HTML_ELEMENT_ACTION_TYPE_LOCATE:
            break;

        case HTML_ELEMENT_ACTION_TYPE_HOVER:
            break;

        case HTML_ELEMENT_ACTION_TYPE_SCRIPT:
            break;

        case UNKNOWN_HTML_ELEMENT_ACTION_TYPE:        
            break;

        default:
            break; 
        }
    } while ( FALSE );

    return ret; 
}

void html_element_prop_dlg::OnBnClickedButtonAdd()
{
	LRESULT ret; 
	wstring element_title; 
	HTML_ELEMENT_ACTION *config = NULL; 
	HTML_ELEMENT_ACTION *parent_config; 
	wstring element_text; 
	ULONG action_type; 

	// TODO: Add your control notification handler code here

	do
	{
		HTREEITEM selected_item; 
		HTREEITEM new_tree_item; 
		HTREEITEM container_item; 

		{
#ifdef NO_SUPPORT_DELAY_TIME_ADJUST
			_ret = save_html_action_global_config(); 
			if( _ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "error\n" ); 
			}
#endif //NO_SUPPORT_DELAY_TIME_ADJUST
		}

		if( configure_state & HTML_INSTRUCTION_SET_JUMP_TO )
		{
			MessageBox( L"请鼠标左键单击要跳转至的目标条目\n" ); 
			break; 
		}

		ret = allocate_html_element_action(&config); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ASSERT( NULL != config ); 

        if (0 == tree_elements.GetCount())
        {
            HTML_PAGE_INFO page_info;
            HTREEITEM new_tree_item;
            HTML_SCRIPT_INSTANCE instance; 

            do
            {
                ret = get_html_script_instance(0, &instance);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                get_new_page_info(&instance, page_info);
            } while (FALSE);

            //tree_elements.GetRootItem()==NULL
            ret = load_instruction_page(L"初始网页布局", L"", &page_info, TVI_ROOT, &new_tree_item);
            if (ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret);
                break;
            }

            selected_item = tree_elements.GetRootItem();
        }
        else
        {
            //如果这个PATH的路径以前有，则进行替换
            selected_item = tree_elements.GetSelectedItem();
            if (selected_item == NULL)
            {
                selected_item = tree_elements.GetRootItem();
            }
        }

		ret = get_container_element( selected_item, 
			&container_item ); 

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( NULL == container_item ); 
		}
		else
		{
			ASSERT( NULL != container_item ); 
		}

        ret = _get_html_element_action_info_from_ui(config, element_text, action_type); 
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret);
            break;
        }

		//if( NULL == tree_elements.GetChildItem( TVI_ROOT ) 
		//	&& ERROR_SUCCESS != check_no_html_element_action( config ) )
		{
			if( 0 == locate_urls.GetItemCount() )
			{
				locate_urls.InsertItem( 0, g_html_script_config_dlg->m_WebBrowser.get_loading_url().c_str() ); 
				locate_urls.SetItemText( 0, 1, L"-1" ); 
			}
			//else
			//{
			//	locate_urls.SetItemText( 0, 0, g_html_script_config_dlg->m_WebBrowser.get_loading_url().c_str() ); 
			//}
		}

		do 
		{
			if( NULL != container_item 
				&& configure_state & HTML_INSTRUCTION_CONTENT_PROCESS )
			{
				HTML_ELEMENT_ACTION *container_action; 

				do 
				{
					container_action = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( container_item ); 
					//if( container_action == NULL 
					//	|| container_action->parent_item == NULL 
					//	|| 0 != wcscmp( container_action->parent_item->action.c_str(), 
					//	HTML_ELEMENT_ACTION_OUTPUT ) 
					//	|| 0 != wcscmp( container_action->parent_item->param.c_str(), 
					//	HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) ) 
					//{
					//	new_tree_item = insert_tree_item( action_type, 
					//		element_text.c_str(), 
					//		NULL, TVI_LAST ); 
					//	break; 
					//}

					new_tree_item = insert_tree_item( action_type, 
						element_text.c_str(), 
						container_item, 
						TVI_LAST ); 

				} while ( FALSE );

				break; 
			}

			{
				CString properities_text; 
				GetDlgItemText( IDC_RICHEDIT2_PROPERTY_TEXT, properities_text ); 
				config->properties = properities_text.GetBuffer(); 
			}

			new_tree_item = insert_tree_item( action_type, 
				element_text.c_str(), 
				NULL, 
				TVI_LAST ); 
		}while( FALSE ); 

		if( NULL == new_tree_item )
		{
			ULONG error_code; 
			error_code = GetLastError(); 
			dbg_print( MSG_ERROR, "insert the tree item error\n" ); 
			free_element_config( config ); 
			config = NULL; 
			break; 
		}

		ret = attach_element_config( new_tree_item, config ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		parent_config = config; 
		config = NULL; 

#if 0
        ret = get_action_desc(parent_config, element_text);
        if (ret != ERROR_SUCCESS)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret); 
            break; 
        }

        do
        {
            ASSERT(NULL == config);

            selected_item = new_tree_item;

            new_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_NONE,
                element_text.c_str(),
                selected_item,
                TVI_LAST);

            if (NULL == new_tree_item)
            {
                break;
            }

            ret = allocate_element_config(&config);
            if (ERROR_SUCCESS != ret)
            {
                dbg_print(MSG_FATAL_ERROR, "allocate element config error\n", ret);
                break;
            }

            ASSERT(NULL != config);

            config->action = HTML_ELEMENT_ACTION_NONE;
            config->param = HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER;
            config->in_frame = FALSE;
            config->title = element_text;

            ret = attach_element_config(new_tree_item, config);
            if (ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "set the tree item data error\n");
                break;
            }

            config = NULL;

        } while (FALSE); 
#endif //0

        HTML_ELEMENT_INFOS _infos; 
        HTML_ELEMENT_INFOS *infos; 


        if (0 == wcscmp( parent_config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT)
            && 0 == wcscmp(parent_config->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
        {
            ret = get_html_elements_info(parent_config, _infos, INNER_LINK);
            if (ret != ERROR_SUCCESS)
            {
            }
        }
        else
        {
            ret = get_html_elements_info(parent_config, _infos, 0);
            if (ret != ERROR_SUCCESS)
            {
            }
        }

        infos = &_infos; 

        do
        {
            LRESULT _ret; 
            HTREEITEM _selected_item;
            HTREEITEM _new_tree_item;
            HTREEITEM _new_sub_tree_item;
            HTML_ELEMENT_ACTION *_config;
            CString __element_text;

            if (infos->size() == 0)
            {
                __element_text.Format(HTML_ELEMENT_LIST_TITLE_FORMAT_TEXT, 0);

                _selected_item = new_tree_item;
                _new_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_NONE,
                    __element_text.GetBuffer(),
                    _selected_item,
                    TVI_LAST);

                if (NULL == _new_tree_item)
                {
                    _ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }

                //tree_elements.SetItemText(_selected_item, __element_text.GetBuffer()); 
                dbg_print(MSG_ERROR, "can not get the link list\n");
                break;
            }

            __element_text.Format(HTML_ELEMENT_LIST_TITLE_FORMAT_TEXT, infos->size());

            _selected_item = new_tree_item;
            //tree_elements.SetItemText(_selected_item, __element_text.GetBuffer());
            //_new_tree_item = new_tree_item; 

            _new_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_NONE,
                __element_text.GetBuffer(),
                _selected_item,
                TVI_LAST);

            if (NULL == _new_tree_item)
            {
                _ret = ERROR_ERRORS_ENCOUNTERED;
                break;
            }

            ret = allocate_html_element_action(&_config);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }

            _config->action = HTML_ELEMENT_ACTION_NONE;
            _config->title = __element_text.GetBuffer();

            ret = attach_element_config(_new_tree_item, _config);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }

            /*****************************************************************************
            针对不同类型的命令，列出不同的信息。
            命令执行方式类似于SCRAPY
            ****************************************************************************/
            _config = NULL;
            for (HTML_ELEMENT_INFO_ITERATOR it = infos->begin(); it != infos->end(); it++)
            {
                do
                {
                    get_html_elemen_info_text(action_type, parent_config, *(it), element_text);

                    _new_sub_tree_item = insert_tree_item(HTML_ELEMENT_ACTION_TYPE_NONE,
                        element_text.c_str(),
                        _new_tree_item,
                        TVI_LAST);

                    if (NULL == _new_sub_tree_item)
                    {
                        break;
                    }

                    ret = allocate_html_element_action(&_config);
                    if (ERROR_SUCCESS != ret)
                    {
                        break;
                    }

                    ASSERT(NULL != _config);

                    _config->action = HTML_ELEMENT_ACTION_NONE;
                    _config->in_frame = FALSE;

                    if (0 == wcscmp(parent_config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT)
                        && 0 == wcscmp(parent_config->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK))
                    {
                        _config->param = HTML_ELEMENT_ACTION_NONE_PARAM_LINK;
                    }
                    else
                    {
                        _config->param = HTML_ELEMENT_ACTION_NONE_PARAM_LINK;
                    }

                    _config->xpath = (*(it))->xpath;
                    _config->outputs.clear(); 
                    _config->title = element_text; 
                    
                    ret = attach_element_config(_new_sub_tree_item, _config);

                    if (ERROR_SUCCESS != ret)
                    {
                        break;
                    }

                    _config = NULL;
                } while (FALSE);

                if (_config != NULL)
                {
                    detach_free_element_config(_new_sub_tree_item, _config);
                    _config = NULL;
                }
            }

            HTML_PAGE_INFO page;

            _ret = action_will_open_page(parent_config);

            if (_ret == ERROR_SUCCESS)
            {
                selected_item = new_tree_item;

                ret = load_instruction_page(NULL, parent_config->xpath.c_str(), &page, selected_item, &new_tree_item);
                if (ret != ERROR_SUCCESS)
                {
                    dbg_print(MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret);
                    break;
                }
            }

            if ( 0 == wcscmp( parent_config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) 
                && 0 == wcscmp(parent_config->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK )
                && (*(infos->begin()))->href.length() > 0)
            {
                if (IDYES == MessageBox(L"是否打开第1个得到的超链接?", L"", MB_YESNO))
                {
                    g_html_script_config_dlg->locate_to_url((*(infos->begin()))->href.c_str());
                }
            }

        }while (FALSE); 
		
        release_html_element_infos(&_infos); 
	}while( FALSE ); 

	do
	{
		CButton *check_box; 
		check_box = ( CButton* )GetDlgItem( IDC_CHECK_LOAD_WEB_PAGE ); 
		if( NULL == check_box )
		{
			break; 
		}

		check_box->SetCheck( 0 ); 
	}while( FALSE ); 

	if( NULL != config )
	{
		ASSERT( FALSE ); 
		//free_element_config( config ); 
	}
}

void html_element_prop_dlg::set_tip( LPCWSTR tip_text )
{
	// TODO: Add your specialized code here and/or call the base class
	ASSERT( NULL != tip_text ); 
	
	SetDlgItemTextW( IDC_STATIC_TIP, tip_text ); 
}

LRESULT WINAPI set_html_element_info( MSXML::IXMLDOMDocument *xml_doc, 
									MSXML::IXMLDOMElement *xml_element, 
									HTML_ELEMENT_ACTION *action ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMElementPtr dest_element; 
	MSXML::IXMLDOMTextPtr xml_text; 

	_bstr_t text; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != xml_element ); 
		ASSERT( NULL != action ); 

		do
		{
			dest_element = xml_element; 

			_variant_t attr_value; 
			attr_value.vt = VT_I4; 
			attr_value.intVal = action->in_frame; 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_IN_FRAME, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value = action->param.c_str(); 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_ACTION_PARAM, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value = action->action.c_str(); 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_ACTION, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value = action->title.c_str(); 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_TITLE, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value = action->name.c_str(); 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_NAME, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value.vt = VT_I4; 
			attr_value.intVal= action->id; 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_ID, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 

			attr_value.vt = VT_I4; 
			attr_value.intVal= action->jump_to_id; 

			if( action->jump_to_id != INVALID_JUMP_TO_ID )
			{
				ASSERT( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_JUMP ) ); 
			}
			else
			{
				ASSERT( 0 != wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_JUMP ) ); 
			}

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_JUMP_TO_ID, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_HTML_TEXT ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( action->html_text.length() == 0 )
			{
				text = L" "; 
			}
			else
			{
				text = action->html_text.c_str(); 
			}

			xml_text = xml_doc->createTextNode( text ); 
			if( NULL == xml_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_text->get_text( text.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			_text = text.operator const wchar_t*(); 

			if( _text == NULL )
			{
				break; 
			}

			xml_attr->appendChild( xml_text ); 

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 

		//do
		//{
		//	MSXML::IXMLDOMAttributePtr xml_attr; 

		//	xml_attr = xml_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_URL ); 
		//	if( NULL == xml_attr )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	text = action->url.c_str(); 

		//	xml_text = xml_doc->createTextNode( text ); 
		//	if( NULL == xml_text )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	hr = xml_text->get_text( text.GetAddress() ); 
		//	if( hr != S_OK )
		//	{
		//		break; 
		//	}

		//	_text = text.operator const wchar_t*(); 

		//	if( _text == NULL )
		//	{
		//		break; 
		//	}

		//	xml_attr->appendChild( xml_text ); 

		//	dest_element->setAttributeNode( xml_attr ); 
		//}while( FALSE ); 

			do
			{
				if( 0 != wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_INPUT ) )
				{
					break; 
				}

				MSXML::IXMLDOMAttributePtr xml_attr; 

				xml_attr = xml_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_INPUT_VALUE ); 
				if( NULL == xml_attr )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ASSERT( action->outputs.size() >= 0 ); 

                {
                    wstring output_data;
                    cat_output_data(action, output_data);
                    text = output_data.c_str(); 
                }

				xml_text = xml_doc->createTextNode( text ); 
				if( NULL == xml_text )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = xml_text->get_text( text.GetAddress() ); 
				if( hr != S_OK )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 

				if( _text == NULL )
				{
					break; 
				}

				xml_attr->appendChild( xml_text ); 

				dest_element->setAttributeNode( xml_attr ); 
			}while( FALSE ); 
		do
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_XPATH ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = action->xpath.c_str(); 

			xml_text = xml_doc->createTextNode( text ); 
			if( NULL == xml_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_text->get_text( text.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			_text = text.operator const wchar_t*(); 

			if( _text == NULL )
			{
				break; 
			}

			xml_attr->appendChild( xml_text ); 

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_html_scirpt_instance_info( MSXML::IXMLDOMDocument *xml_doc, 
									 MSXML::IXMLDOMElement *xml_element, 
									 HTML_SCRIPT_INSTANCE *instance_info ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMElementPtr dest_element; 
	MSXML::IXMLDOMTextPtr xml_text; 

	_bstr_t text; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != xml_element ); 
		ASSERT( NULL != instance_info ); 

		do
		{
			dest_element = xml_element; 

			_variant_t attr_value; 
			attr_value.vt = VT_I4; 
			attr_value.intVal = instance_info->loop_count; 

			hr = dest_element->setAttribute( HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOOP_COUNT, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_doc->createAttribute( HTML_SCRIPT_INSTANCE_ATTRIBUTE_BEGIN_URL); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = instance_info->begin_url.c_str(); 

			xml_text = xml_doc->createTextNode( text ); 
			if( NULL == xml_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_text->get_text( text.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			_text = text.operator const wchar_t*(); 

			if( _text == NULL )
			{
				break; 
			}

			xml_attr->appendChild( xml_text ); 

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 

		do
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_doc->createAttribute( HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOCATE_URL); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = instance_info->location_url.c_str(); 

			xml_text = xml_doc->createTextNode( text ); 
			if( NULL == xml_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_text->get_text( text.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			_text = text.operator const wchar_t*(); 

			if( _text == NULL )
			{
				break; 
			}

			xml_attr->appendChild( xml_text ); 

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 
	}while( FALSE );

	return ret; 
}
/********************************************************************
HTML元素工作流程配置内容(抓取内容):
1.起始页面，如果没有当前页面，则定位到相应的页面。
2.扫描所有的ELEMENT，对每个ELEMENT执行相应的行为。
3.得到ELEMENT的XPATH，找到当前页面中的对应的元素。
4.得到元素对应的动作，动作相关信息。对其中所有的元素执行相应的动作。
********************************************************************/

LRESULT CALLBACK save_html_element( HTML_ELEMENT_ACTION *action, 
								   MSXML::IXMLDOMDocument *doc,  
								   MSXML::IXMLDOMElement *xml_parent_element, 
								   MSXML::IXMLDOMElement **xml_element_out ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	MSXML::IXMLDOMNodePtr xml_node = NULL; 
	//MSXML::IXMLDOMNamedNodeMapPtr xml_attrs = NULL; 
	//MSXML::IXMLDOMNodePtr xml_attr = NULL; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != doc ); 
		ASSERT( NULL != xml_parent_element ); 
		ASSERT( NULL != xml_element_out ); 

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		//{
		//	_bstr_t element_path; 
		//	element_path = HTML_SCRIPT_ROOT_ELEMENT_NAME; 
		//	elements_node = xml_element->selectSingleNode( element_path ); 
		//}

		xml_doc = doc; 
		xml_element = xml_doc->createElement( HTML_SCRIPT_ACTION_ELEMENT_NAME ); 
		if( NULL == xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = set_html_element_info( xml_doc, xml_element, action ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//ret = insert_node_by_text(xml_doc, dest_element);
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		if( NULL == xml_parent_element->appendChild( xml_element ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		xml_element->AddRef(); 
		*xml_element_out = xml_element; 
	}while( FALSE ); 

#ifdef PERFORMANCE_DEBUG
	tick_end = GetTickCount(); 
	dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG

	return ret; 
}

LRESULT CALLBACK save_html_script_instance_info( vector< HTML_SCRIPT_INSTANCE*> *instances, 
												MSXML::IXMLDOMDocument *doc,  
												MSXML::IXMLDOMElement *xml_parent_element ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	do 
	{
		ASSERT( NULL != instances ); 
		ASSERT( NULL != doc ); 
		ASSERT( NULL != xml_parent_element ); 

		xml_doc = doc; 
		xml_element = xml_doc->createElement( HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
		if( NULL == xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		for( vector< HTML_SCRIPT_INSTANCE*>::iterator it = instances->begin(); 
			it != instances->end(); 
			it ++ )
		{
			ASSERT( NULL != ( *it ) ); 
			do 
			{
				ret = set_html_scirpt_instance_info( xml_doc, xml_element, ( *it ) ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( NULL == xml_parent_element->appendChild( xml_element ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			}while( FALSE );
		}
	}while( FALSE ); 

	return ret; 
}

typedef struct _HTML_ELEMENT_CONFIG_INFO
{
	HTML_ELEMENT_ACTION* config; 
	MSXML::IXMLDOMElement* xml_parent_element; 
} HTML_ELEMENT_CONFIG_INFO, *PHTML_ELEMENT_CONFIG_INFO; 

typedef struct _HTML_ELEMENT_CONFIG_INFO2
{
	MSXML::IXMLDOMNode* xml_node; 
	HTREEITEM parent; 
} HTML_ELEMENT_CONFIG_INFO2, *PHTML_ELEMENT_CONFIG_INFO2; 

LRESULT html_element_prop_dlg::save_html_script( LPCWSTR file_name, 
												vector<HTML_SCRIPT_INSTANCE*> *instances )
{
	LRESULT ret = ERROR_SUCCESS; 
	CString xpath; 
	CString action; 
	CString action_param; 
	HTML_ELEMENT_ACTION *root_config; 
	HTREEITEM root_item; 
	HTML_ELEMENT_ACTION *item; 
	HTML_ELEMENT_ACTION *sub_item; 
	HTML_ELEMENT_ACTION *next_item; 
	HTML_ELEMENT_CONFIG_INFO *config_info; 
	HTML_ELEMENT_CONFIG_INFO *new_config_info = NULL; 
	vector< HTML_ELEMENT_CONFIG_INFO* > next_items; 

	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMNodePtr xml_node; 
	MSXML::IXMLDOMElement *sub_xml_element = NULL; 

	VARIANT_BOOL __ret; 
	//MSXML::IXMLDOMNamedNodeMapPtr xml_attrs = NULL; 
	//MSXML::IXMLDOMNodePtr xml_attr = NULL; 

	LPCWSTR temp_text; 
	_variant_t save_file_name; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	do 
	{ 
		CString config_file; 

		ASSERT( NULL != file_name ); 
		config_file = file_name; 

		if( instances->size() == 0 )
		{
			//ASSERT( FALSE ); 
			//ret = ERROR_INVALID_PARAMETER; 
			//break; 
		}

		if( config_file.GetAt( config_file.GetLength() - 1 ) == L'\\' )
		{
			break; 
		}

		DeleteFileW( config_file.GetBuffer() ); 

		ret = open_actions_config_file( config_file.GetBuffer(), config_file.GetLength() ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

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

		if( xml_doc->documentElement == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}
 
		//{
		//	_bstr_t element_path; 
		//	element_path = HTML_SCRIPT_ROOT_ELEMENT_NAME; 
		//	elements_node = xml_element->selectSingleNode( element_path ); 
		//}

		//if( NULL == elements_node )
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//ASSERT( script_work.running == FALSE ); 
		root_item = tree_elements.GetChildItem( TVI_ROOT ); 
		if( root_item == NULL )
		{
			break; 
		}

		root_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( root_item ); 

		ASSERT( NULL != root_config ); 

		new_config_info = new HTML_ELEMENT_CONFIG_INFO(); 
		if( new_config_info == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		//{
		//	MSXML::IXMLDOMElement *xml_node; 
		//
		//	xml_node = xml_doc->createElement( INPUT_TEXT_CONTENT_ELEMENT_NAME ); 

		//	if( NULL == xml_node )
		//	{
		//		log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	xml_doc->documentElement->raw_appendChild( xml_node, NULL );
		//}


		{
			_bstr_t element_path; 

			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			xml_node = xml_doc->documentElement->selectSingleNode( element_path ); 

			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), 
				( PVOID* )&new_config_info->xml_parent_element ); 
			if( FAILED(hr))
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		ASSERT( new_config_info->xml_parent_element != NULL ); 

		if( NULL == new_config_info->xml_parent_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//do
		//{
		//	_variant_t attr_value; 

		//	attr_value = begin_url; 

		//	hr = new_config_info->xml_parent_element->setAttribute( HTML_SCRIPT_BEGIN_URL, attr_value ); 

		//	if( hr != S_OK )
		//	{
		//		break; 
		//	}
		//}while( FALSE ); 

		//do
		//{
		//	_variant_t attr_value; 

		//	attr_value = begin_urls; 

		//	hr = new_config_info->xml_parent_element->setAttribute( HTML_SCRIPT_LOCATION_URL, attr_value ); 

		//	if( hr != S_OK )
		//	{
		//		break; 
		//	}

		//}while( FALSE ); 

		ret = save_html_script_instance_info( instances, 
			xml_doc, 
			new_config_info->xml_parent_element ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		new_config_info->config = root_config; 
		//new_config_info->xml_parent_element->AddRef(); 
		next_items.push_back( new_config_info ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			config_info = *next_items.rbegin(); 
			next_items.pop_back(); 
			ASSERT( NULL != config_info->xml_parent_element ); 

			//_bstr_t _temp_text; 

			do 
			{
				ret = save_html_element( config_info->config, 
					xml_doc, 
					config_info->xml_parent_element, 
					&sub_xml_element ); 
				if( ret !=	ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( NULL != sub_xml_element); 

				item = config_info->config; 

				do 
				{
					sub_item = item->sub_item; 

					if( NULL == sub_item )
					{
						sub_xml_element->Release(); 
						sub_xml_element = NULL; 
						break; 
					}

					new_config_info = new HTML_ELEMENT_CONFIG_INFO(); 
					if( new_config_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					new_config_info->config = sub_item; 
					new_config_info->xml_parent_element = sub_xml_element; 
					next_items.push_back( new_config_info ); 
					sub_xml_element = NULL; 
				} while ( FALSE ); 

				do 
				{
					next_item = item->next_item; 

					if( NULL == next_item )
					{
						break; 
					}

					new_config_info = new HTML_ELEMENT_CONFIG_INFO(); 
					if( new_config_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					new_config_info->config = next_item; 
					new_config_info->xml_parent_element = config_info->xml_parent_element; 
					new_config_info->xml_parent_element->AddRef(); 
					next_items.push_back( new_config_info ); 
				} while ( FALSE ); 
			}while( FALSE ); 

			config_info->xml_parent_element->Release(); 
			delete config_info; 
		} 

		do 
		{
			_variant_t save_file_name; 
			_bstr_t text; 

			text = file_name; 

			save_file_name = text; 
			hr = xml_doc->save( save_file_name ); 
		
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

            instruction_chnaged = FALSE; 
		} while ( FALSE );

	}while( FALSE ); 

	ASSERT( next_items.empty() == true ); 
	//for( vector< HTML_ELEMENT_CONFIG_INFO* >::iterator it = next_items.begin(); 
	//	it != next_items.end(); 
	//	it ++ ) 
	//{
	//	do 
	//	{
	//		if( NULL == ( *it ) )
	//		{
	//			ASSERT( FALSE ); 
	//		}

	//		( *it )->xml_parent_element->Release(); 
	//		delete ( *( it ) ); 
	//	} while ( FALSE ); 
	//}

	next_items.clear(); 

	return ret; 
}

LRESULT html_element_prop_dlg::update_html_script( LPCWSTR file_name, 
												  LPCWSTR location_url )
{
	LRESULT ret = ERROR_SUCCESS; 
	//CString xpath; 
	//CString action; 
	//CString action_param; 
	//ELEMENT_CONFIG *root_config; 
	//HTREEITEM root_item; 
	//ELEMENT_CONFIG *item; 
	//ELEMENT_CONFIG *sub_item; 
	//ELEMENT_CONFIG *next_item; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr sub_xml_element; 
	MSXML::IXMLDOMElementPtr root_xml_element; 
	MSXML::IXMLDOMNodePtr xml_node; 

	VARIANT_BOOL __ret; 
	//MSXML::IXMLDOMNamedNodeMapPtr xml_attrs = NULL; 
	//MSXML::IXMLDOMNodePtr xml_attr = NULL; 

	_variant_t save_file_name; 

	MSXML::IXMLDOMNodeListPtr xml_node_list; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	LONG node_count; 
	//LPWSTR __temp_text; 
	LPCWSTR _temp_text; 
	_bstr_t temp_text; 
	_bstr_t attr_name; 
	_variant_t node_value; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	do 
	{ 
		CString config_file; 

		ASSERT( NULL != location_url ); 
		ASSERT( NULL != file_name ); 
		config_file = file_name; 

		if( *location_url == L'\0')
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( config_file.GetAt( config_file.GetLength() - 1 ) == L'\\' )
		{
			break; 
		}

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

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

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			break; 
		}

		if( xml_doc->documentElement == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		//ASSERT( script_work.running == TRUE ); 

		{
			_bstr_t element_path; 
			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			xml_node = xml_doc->documentElement->selectSingleNode( element_path ); 
			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), 
				( PVOID* )&root_xml_element ); 
			if( FAILED(hr))
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		ASSERT( root_xml_element != NULL ); 
		if( NULL == root_xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
		//do
		//{
		//	_variant_t attr_value; 

		//	attr_value = location_url; 

		//	hr = root_xml_element->setAttribute( HTML_SCRIPT_LOCATION_URL, attr_value ); 

		//	if( hr != S_OK )
		//	{
		//		break; 
		//	}
		//}while( FALSE ); 
		do 
		{
			hr = root_xml_element->raw_selectNodes( HTML_SCRIPT_INSTANCE_ELEMENT_NAME, &xml_node_list ); 
			if( FAILED(hr))
			{
				log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
				break; 
			}

			if( xml_node_list == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
				break; 
			}

			hr = xml_node_list->get_length( &node_count ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
				break; 
			}

			if( node_count == 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				//xml_node = xml_doc->createElement( HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 

				//if( NULL == xml_node )
				//{
				//	log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				//root_xml_element->raw_appendChild( xml_node, NULL );
				break; 
			}

			hr = xml_node_list->get_item( 0, &xml_node ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			do 
			{
				hr = xml_node->QueryInterface(  __uuidof( MSXML::IXMLDOMElement ), 
					( PVOID* )&sub_xml_element ); 

				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}
				
				if( NULL == sub_xml_element )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				temp_text = location_url; 
				node_value = temp_text; 
				attr_name = HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOCATE_URL; 

				hr = sub_xml_element->setAttribute( attr_name, node_value ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}
			}while( FALSE ); 
		}while( FALSE ); 

		do 
		{
			_variant_t save_file_name; 
			_bstr_t text; 

			text = file_name; 

			save_file_name = text; 
			hr = xml_doc->save( save_file_name ); 

			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		} while ( FALSE );
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_html_element_action_config( MSXML::IXMLDOMNode *xml_node, HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != xml_node ); 
		ASSERT( NULL != config ); 

		config->next_item = NULL; 
		config->parent_item = NULL; 
		config->sub_item = NULL; 

		{
			HRESULT hr; 
			//MSXML::IXMLDOMElementPtr _xml_element; 
			_variant_t xml_node_value; 
			_bstr_t _temp_text; 
			LPCWSTR temp_text; 
			MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
			MSXML::IXMLDOMNodePtr xml_attr; 
			//HTML_ELEMENT_ACTION *action = NULL; 

			//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

			do 
			{
				//hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), ( PVOID* )&_xml_element ); 
				//if( FAILED(hr))
				//{
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				hr = xml_node->get_attributes( &xml_attrs ); 
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
						config->xpath = temp_text; 
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
						config->title = temp_text; 
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
						config->action = temp_text; 
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
						config->param = temp_text; 
					}
				}while( FALSE ); 

				//do 
				//{
				//	xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_URL ); //L"action_param"
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
				//		config->url = temp_text; 
				//	}
				//}while( FALSE ); 

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
						config->html_text = temp_text; 
					}
				}while( FALSE ); 

				do 
				{
					config->in_frame = FALSE; 
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

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						LPWSTR end_text; 
						config->in_frame = ( BOOLEAN )wcstol( temp_text, &end_text, 0 ); 
						ASSERT( config->in_frame == FALSE 
							|| config->in_frame == TRUE ); 
					}

				}while( FALSE ); 

				do 
				{
					config->id = INVALID_JUMP_TO_ID; 
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

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						LPWSTR end_text; 
						config->id = wcstol( temp_text, &end_text, 0 ); 
					}
				}while( FALSE ); 

				do 
				{
					config->jump_to_id = INVALID_JUMP_TO_ID; 
					xml_attr = xml_attrs->getNamedItem( HTML_ELEMENT_ATTRIBUTE_JUMP_TO_ID ); 
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
						ASSERT( FALSE ); 
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						LPWSTR end_text; 
						config->jump_to_id = wcstol( temp_text, &end_text, 0 ); 
						if( config->jump_to_id != INVALID_JUMP_TO_ID )
						{
							ASSERT( 0 == wcscmp( config->action.c_str(), HTML_ELEMENT_ACTION_JUMP ) ); 
						}
						else
						{
							ASSERT( 0 != wcscmp( config->action.c_str(), HTML_ELEMENT_ACTION_JUMP ) ); 
						}
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

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
                        config->outputs.clear(); 
						config->outputs.push_back( wstring(temp_text)); 
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

					_temp_text = ( BSTR )xml_node_value.bstrVal; 

					temp_text = _temp_text.operator wchar_t*(); 
					if( NULL == temp_text )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
					}
					else
					{
						config->name = temp_text; 
					}
				}while( FALSE ); 
			}while( FALSE ); 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

LPCWSTR WINAPI get_ui_string( LPCWSTR text )
{
	LPCWSTR _text = L""; 

	if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_OUTPUT ) )
	{
		_text = HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_CLICK ) )
	{
		_text = HTML_ELEMENT_ACTION_CLICK_UI_TEXT; 
	}
	else if( 0 == wcscmp( text, HTML_ELEMENT_ACTION_BACK ) )
	{
		_text = HTML_ELEMENT_ACTION_BACKWARD_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_INPUT ) )
	{
		_text = HTML_ELEMENT_ACTION_INPUT_UI_TEXT; 
	}
	else if( 0 == wcscmp( text, HTML_ELEMENT_ACTION_LOCATE ) )
	{
		_text = HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_HOVER ) )
	{
		_text = HTML_ELEMENT_ACTION_HOVER_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_SCRIPT ) )
	{
		_text = HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
	{
		_text = HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT ) )
	{
		_text = HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK; 
	}
	else
	{
		//ASSERT( FALSE ); 
	}

	return _text; 
}

ULONG WINAPI get_action_type( LPCWSTR text )
{
	ULONG _actoin; 
	if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_OUTPUT ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_OUTPUT; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_CLICK ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_CLICK; 
	}
	else if( 0 == wcscmp( text, HTML_ELEMENT_ACTION_BACK ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_BACK; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_INPUT ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_INPUT; 
	}
	else if( 0 == wcscmp( text, HTML_ELEMENT_ACTION_LOCATE ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_LOCATE; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_HOVER ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_HOVER; 
	}
	else if( 0 == wcscmp( text , HTML_ELEMENT_ACTION_SCRIPT ) )
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_SCRIPT; 
	}
	else
	{
		_actoin = HTML_ELEMENT_ACTION_TYPE_NONE; 
		//ASSERT( FALSE ); 
	}

	return _actoin; 
}

LRESULT WINAPI get_html_element_action_text( HTML_ELEMENT_ACTION *action, wstring &element_text ) 
{
	LRESULT ret = ERROR_SUCCESS; 
    ULONG flags = 0; 

	do 
	{
		ASSERT( NULL != action ); 

        if( 0 != wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_NONE ) )
        {
            flags = HTML_ELEMENT_ACTION_UI_SHOW_ID;
        }

		ret = get_tree_item_text( 
            flags, 
            action->id, 
			action->title.c_str(), 
			get_ui_string( action->action.c_str() ), 
			action->xpath.c_str(), 
			get_ui_string( action->param.c_str() ), 
			element_text ); 

		if( INVALID_JUMP_TO_ID != action->jump_to_id )
		{
			WCHAR temp_text[ MAX_PATH ]; 
			StringCchPrintfW( temp_text, 
				ARRAYSIZE( temp_text ), 
				L"跳转至:" HTML_ACTION_DESC_FORMAT_STRING, 
				action->jump_to_id ); 

			element_text += temp_text; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::load_instruction_page( LPCWSTR prefix, 
    LPCWSTR name, 
    HTML_PAGE_INFO *page_info,
    HTREEITEM parent,
    HTREEITEM *item_out)
{
    LRESULT ret = ERROR_SUCCESS;
    HTML_ELEMENT_ACTION *config = NULL;
    wstring element_text;
    HTREEITEM new_tree_item;
    BOOL _ret;

    do
    {
        ASSERT(NULL != parent);
        ASSERT(NULL != item_out);

        *item_out = NULL; 

        ret = allocate_html_element_action(&config);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        config->action = HTML_ELEMENT_ACTION_NONE; 
        config->param = HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER; 
        config->name = name; 

        //ret = get_html_element_action_text(config,
        //    element_text);

        if (prefix == NULL)
        {
            element_text = OPENED_PAGE_LAYOUT_NODE_NAME; 
        }
        else
        {
            element_text = prefix;
        }

        config->title = element_text;

        if (page_info->url.length() > 0)
        {
            element_text += L"("; 
            element_text += page_info->url.c_str(); 
            element_text += L")";
        }

        config->title = element_text; 

        new_tree_item = _insert_tree_item(config->action.c_str(),
            element_text.c_str(),
            parent,
            TVI_LAST);

        if (NULL == new_tree_item)
        {
            break;
        }

        ret = attach_element_config(new_tree_item, config);
        if (ret != ERROR_SUCCESS)
        {
            _ret = tree_elements.DeleteItem(new_tree_item);
            if (FALSE == _ret)
            {
                ASSERT(FALSE);
                ret = ERROR_ERRORS_ENCOUNTERED;
                break;
            }

            dbg_print(MSG_FATAL_ERROR, "set the tree item data error\n");
            break;
        }

        *item_out = new_tree_item; 

    } while (FALSE);

    return ret;
}

LRESULT html_element_prop_dlg::load_html_element( 
	MSXML::IXMLDOMDocument *xml_doc, 
	MSXML::IXMLDOMNode *xml_node, 
	HTREEITEM parent, 
	HTREEITEM *item_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *action = NULL; 
	wstring element_text; 
	HTREEITEM new_tree_item; 
	BOOL _ret; 

	do 
	{
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != xml_node ); 
		ASSERT( NULL != parent ); 
		ASSERT( NULL != item_out ); 

		ret = allocate_html_element_action(&action); 
		if( ret != ERROR_SUCCESS)
		{
			break; 
		}

		ret = get_html_element_action_config( xml_node, action ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

        ret = update_base_html_element_action_id(action); 
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ret = get_html_element_action_text( action, 
			element_text ); 
		
		new_tree_item = _insert_tree_item( action->action.c_str(), 
			element_text.c_str(), 
			parent, 
			TVI_LAST ); 

		if( NULL == new_tree_item )
		{
			break; 
		}

		ret = attach_element_config( new_tree_item, action ); 
		if( ret != ERROR_SUCCESS )
		{
			_ret = tree_elements.DeleteItem( new_tree_item ); 
			if( FALSE == _ret )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}		

			dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
			break; 
		}

		*item_out = new_tree_item; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI load_html_script_instances( MSXML::IXMLDOMNode *parent_element, 
										  vector<HTML_SCRIPT_INSTANCE*> *instances )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 

	MSXML::IXMLDOMNodeListPtr xml_node_list; 
	MSXML::IXMLDOMNodePtr xml_node; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	LONG node_count; 
	LONG i; 
	LPWSTR __temp_text; 
	LPCWSTR _temp_text; 
	_bstr_t temp_text; 
	_variant_t node_value; 

	HTML_SCRIPT_INSTANCE *instance; 
	
	do 
	{
		ASSERT( NULL != instances ); 
	
		hr = parent_element->raw_selectNodes( HTML_SCRIPT_INSTANCE_ELEMENT_NAME, &xml_node_list ); 
		if( FAILED(hr))
		{
			log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
			break; 
		}

		if( xml_node_list == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, "%s: xml node %s is not existed", __FUNCTION__, HTML_SCRIPT_INSTANCE_ELEMENT_NAME ); 
			break; 
		}

		instance = NULL; 
		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				hr = xml_node_list->get_item( i, &xml_node ); 
				if( FAILED( hr ) )
				{
					break; 
				}

				if( NULL == xml_node )
				{
					break; 
				}

				hr = xml_node->get_attributes( &xml_attrs ); 
				if( S_OK != hr )
				{
					break; 
				}

				instance = new HTML_SCRIPT_INSTANCE(); 
				if( NULL == instance )
				{
					break; 
				}

				do 
				{
					temp_text = HTML_SCRIPT_INSTANCE_ATTRIBUTE_BEGIN_URL; 

					hr = xml_attrs->raw_getNamedItem( temp_text.GetBSTR(), &xml_attr); 
					if( FAILED(hr ) )
					{
						break; 
					}

					if( NULL == xml_attr )
					{
						break; 
					}

					hr = xml_attr->get_nodeValue( node_value.GetAddress()); 
					if( S_OK != hr )
					{
						break; 
					}

					temp_text = node_value.bstrVal; 

					_temp_text = temp_text.operator const wchar_t*(); 

					if( NULL == _temp_text )
					{
						break; 
					}

					instance->begin_url = _temp_text; 
				}while( FALSE );

				do 
				{
					temp_text = HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOCATE_URL; 

					hr = xml_attrs->raw_getNamedItem( temp_text.GetBSTR(), &xml_attr ); 
					if( FAILED(hr))
					{
						break; 
					}

					if( NULL == xml_attr )
					{
						break; 
					}

					hr = xml_attr->get_nodeValue( node_value.GetAddress()); 
					if( S_OK != hr )
					{
						break; 
					}

					temp_text = node_value.bstrVal; 

					_temp_text = temp_text.operator const wchar_t*(); 

					if( NULL == _temp_text )
					{
						break; 
					}

					instance->location_url = _temp_text; 
				}while( FALSE );

				do 
				{
					instance->loop_count = -1; 
					temp_text = HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOOP_COUNT; 

					hr = xml_attrs->raw_getNamedItem( temp_text.GetBSTR(), &xml_attr ); 
					if( FAILED(hr))
					{
						break; 
					}

					if( NULL == xml_attr )
					{
						break; 
					}

					hr = xml_attr->get_nodeValue( &node_value ); 
					if( S_OK != hr )
					{
						break; 
					}

					if( node_value.vt == VT_I4 )
					{
						instance->loop_count = node_value.intVal; 
					}
					else
					{
						temp_text = node_value.bstrVal; 

						_temp_text = temp_text.operator const wchar_t*(); 

						if( NULL == _temp_text )
						{
							break; 
						}

						instance->loop_count = wcstol( _temp_text, &__temp_text, 0 ); 
					}
				}while( FALSE ); 

				if( instance->begin_url.length() == 0 )
				{
					ASSERT( FALSE ); 
					break; 
				}

				if( instance->location_url.length() == 0 )
				{
					instance->location_url = instance->begin_url; 
				}

				instances->push_back( instance ); 
				instance = NULL; 
			}while( FALSE ); 

			if( NULL != instance )
			{
				delete instance; 
				instance = NULL; 
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::load_html_elements_properties( HTML_SCRIPT_INSTANCES *instances )
{
	LRESULT ret = ERROR_SUCCESS; 
	CString xpath; 
	CString action; 
	CString action_param; 

	HTREEITEM root_item; 
	HTREEITEM item; 
	HTML_ELEMENT_CONFIG_INFO2 *config_info; 
	HTML_ELEMENT_CONFIG_INFO2 *new_config_info = NULL; 
	vector< HTML_ELEMENT_CONFIG_INFO2* > next_items; 

	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc = NULL; 
	MSXML::IXMLDOMNode *xml_node; 
	MSXML::IXMLDOMNode *sub_xml_node; 
	MSXML::IXMLDOMNode *next_xml_node; 
	MSXML::IXMLDOMNodeListPtr xml_node_list; 

	VARIANT_BOOL __ret; 

	LPCWSTR temp_text; 
	WCHAR file_name[ MAX_PATH ];
	_bstr_t text; 

#ifdef PERFORMANCE_DEBUG
	ULONG tick_begin; 
	ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

	do 
	{ 

#ifdef PERFORMANCE_DEBUG
		tick_begin = GetTickCount(); 
#endif //PERFORMANCE_DEBUG

		//ASSERT( script_work.running == FALSE ); 
		root_item = tree_elements.GetChildItem( TVI_ROOT ); 
		if( root_item != NULL )
		{
			if( instruction_chnaged == TRUE 
                && IDYES == MessageBox( L"是否保存正在编辑的命令?", L"", MB_YESNO ) )
			{
				LRESULT ret; 
				HRESULT hr; 
				INT32 _ret; 
				WCHAR app_path[ MAX_PATH ]; 
				ULONG cc_ret_len; 
				//SYSTEMTIME _time; 

				do 
				{
					ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

                    _ret = SetCurrentDirectoryW(app_path);
                    if (FALSE == _ret)
                    {
                    }

					hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
					if( S_OK != hr )
					{
						break; 
					}

					ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					*file_name = _T( '\0' ); 

					{
						wstring url; 
						wstring domain_name; 
						//wstring _file_name; 
						url = g_html_script_config_dlg->m_WebBrowser.get_loading_url();

						ret = get_domain_name_in_url( url.c_str(), domain_name ); 
						if( ret != ERROR_SUCCESS )
						{
						}
						else
						{
							domain_name += L".xml"; 
							StringCchCopyW( file_name, sizeof( file_name ), domain_name.c_str() ); 
						}


						//_file_name = app_path; 
						//_file_name += L"\\"; 
						//_file_name += domain_name; 

						ret = open_file_dlg( TRUE, L"xml", file_name, NULL, app_path, L"*.xml\0*.xml\0\0", NULL );
						if( ret < 0 )
						{
							break; 
						}
					}

					if( *file_name != L'\0' )
					{
						vector<HTML_SCRIPT_INSTANCE*> instances; 

						do 
						{
							ret = get_html_script_instances( &instances ); 
							if( ret != ERROR_SUCCESS )
							{
								break; 
							}

							ret = save_html_script( file_name, &instances ); 
							if( ret != ERROR_SUCCESS )
							{
								ASSERT( FALSE ); 
								dbg_print( MSG_FATAL_ERROR | DBG_MSG_AND_ERROR_OUT | DBG_MSG_BOX_OUT, "save the action for html elements error 0x%0.8x\n", ret ); 
								//MessageBox( L"保存指令列失败" ); 
							}
							else
							{
                                SetDlgItemTextW(IDC_STATIC_SAVE_FILE_STATE, L"保存指令列成功");
                                SetTimer(CLEAN_SAVE_STATUS_TIMER, CLEAN_SAVE_STATUS_TIMER_ELAPSE, NULL);
							}

							release_html_script_instances( &instances ); 
						}while( FALSE ); 
					}
				}while( FALSE ); 
			}

			delete_tree_item_siblings( root_item ); 
		}

		GetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, file_name, ARRAYSIZE( file_name ) ); 
		if( L'\0' == *file_name )
		{
			WCHAR app_path[ MAX_PATH ]; 
			INT32 _ret; 
			ULONG cc_ret_len; 

			do 
			{
				ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

                _ret = SetCurrentDirectoryW(app_path);
                if (FALSE == _ret)
                {

                }

				hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
				if( S_OK != hr )
				{
					break; 
				}

				ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				{
					wstring url; 
					wstring domain_name; 
					//wstring _file_name; 
					url = g_html_script_config_dlg->m_WebBrowser.get_loading_url();

					ret = get_domain_name_in_url( url.c_str(), domain_name ); 
					if( ret != ERROR_SUCCESS )
					{
					}
					else
					{
						domain_name += L".xml"; 
						StringCchCopyW( file_name, sizeof( file_name ), domain_name.c_str() ); 
					}

					ret = open_file_dlg( TRUE, L"xml", file_name, NULL, app_path, L"*.xml\0*.xml\0\0", NULL );
					if( ret < 0 )
					{
						break; 
					}
				}
			}while( FALSE ); 
		}
		else if( L'.' == *file_name )
		{
			WCHAR app_path[ MAX_PATH ]; 
			INT32 _ret; 
			ULONG cc_ret_len; 

			do 
			{
				ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

                _ret = SetCurrentDirectoryW(app_path);
                if (FALSE == _ret)
                {

                }

				//hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
				//if( S_OK != hr )
				//{
				//	break; 
				//}

				//ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
				//if( ret != ERROR_SUCCESS )
				//{
				//	break; 
				//}

				hr = StringCchCatW(app_path, ARRAYSIZE(app_path), file_name); 
				if( FAILED(hr))
				{
					break; 
				}

				hr = StringCchCopyW(file_name, ARRAYSIZE(file_name), app_path); 
				if( FAILED(hr))
				{
					break; 
				}
			}while( FALSE ); 
		}

		text = file_name; 

		if( NULL == text.operator wchar_t*() )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//ret = open_actions_config_file( file_name, wcslen( file_name ) ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

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

		if( xml_doc->documentElement == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		{
			_bstr_t element_path; 

			//for( ; ; )
			//{
			//	if( NULL == xml_node->childNodes )
			//	{
			//		break; 
			//	}

			//	hr = xml_node->get_firstChild(&sub_xml_node);
			//	if( FAILED(hr) )
			//	{
			//		ret = ERROR_ERRORS_ENCOUNTERED; 
			//		break; 
			//	}

			//	if( NULL == sub_xml_node )
			//	{
			//		break; 
			//	}

			//	hr = xml_node->raw_removeChild(sub_xml_node, NULL ); 
			//	if( FAILED( hr ) )
			//	{
			//		ret = ERROR_ERRORS_ENCOUNTERED; 
			//		break; 
			//	}
			//	//ret = ERROR_INVALID_PARAMETER; 
			//	//break; 
			//}

			element_path = HTML_SCRIPT_ROOT_ELEMENT_PATH; 
			hr = xml_doc->documentElement->raw_selectSingleNode( element_path, &xml_node ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_FILE_INVALID; 
				break; 
			}

			if( NULL == xml_node )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			ret = load_html_script_instances( xml_node, instances ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			{
				HTML_SCRIPT_INSTANCE *instance = NULL; 

				do
				{
					_variant_t attr_value; 
					MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
					MSXML::IXMLDOMNodePtr xml_attr; 
					wstring begin_url; 
					wstring locate_url; 

					hr = xml_node->get_attributes( &xml_attrs ); 
					if( FAILED( hr ) )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					} 

					do 
					{
						xml_attr = xml_attrs->getNamedItem( HTML_SCRIPT_INSTANCE_ATTRIBUTE_BEGIN_URL ); 
						if( xml_attr == NULL )
						{ 
							break; 
						}

						ASSERT( xml_attr != NULL ); 
						attr_value.Clear(); 
						hr = xml_attr->get_nodeValue( &attr_value ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						text = ( BSTR )attr_value.bstrVal; 

						temp_text = text.operator wchar_t*(); 
						if( NULL == temp_text )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
						}
						else
						{
							begin_url = temp_text; 
						}
					}while( FALSE ); 

					do 
					{
						xml_attr = xml_attrs->getNamedItem( HTML_SCRIPT_INSTANCE_ATTRIBUTE_LOCATE_URL ); 
						if( xml_attr == NULL )
						{ 
							break; 
						}

						ASSERT( xml_attr != NULL ); 
						attr_value.Clear(); 
						hr = xml_attr->get_nodeValue( &attr_value ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						text = ( BSTR )attr_value.bstrVal; 

						temp_text = text.operator wchar_t*(); 
						if( NULL == temp_text )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
						}
						else
						{
							locate_url = temp_text; 
						}
					}while( FALSE ); 
					if( locate_url.length() == 0 )
					{
						locate_url = begin_url; 
						if( locate_url.length() == 0 )
						{
							break; 
						}
					}

					instance = new HTML_SCRIPT_INSTANCE(); 
					if( NULL == instance )
					{
						break; 
					}

					instance->actions = NULL; 
					instance->begin_url = begin_url; 
					instance->location_url = locate_url; 
					instance->loop_count = -1; 

					instances->push_back( instance ); 
					instance = NULL; 
				}while( FALSE ); 

				if( NULL != instance )
				{
					delete instance; 
				}
			}

			root_item = TVI_ROOT; 
			new_config_info = new HTML_ELEMENT_CONFIG_INFO2(); 
			if( new_config_info == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			do 
			{
				element_path = HTML_SCRIPT_ACTION_ELEMENT_NAME; 
				hr = xml_node->raw_selectNodes( element_path.GetBSTR(), &xml_node_list ); 
				if( FAILED(hr))
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( xml_node_list == NULL 
					|| xml_node_list->Getlength() == 0 )
				{
					ret = ERROR_INVALID_PARAMETER; 
					log_trace_ex( MSG_IMPORTANT, "%s: action script is not null", __FUNCTION__ ); 
					break; 
				}
			} while ( FALSE );

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//{
			//	MSXML::IXMLDOMElementPtr xml_element; 
			//	xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), (PVOID*)&xml_element); 
			//}

			hr = xml_node->get_firstChild( &new_config_info->xml_node ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( new_config_info->xml_node == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
			//hr = sub_xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), 
			//	( PVOID* )&new_config_info->xml_node ); 
			//if( FAILED(hr))
			//{
			//	ASSERT( FALSE ); 
			//	ret = ERROR_ERRORS_ENCOUNTERED; 
			//	break; 
			//}
		}

        //HTML_PAGE_INFO page_info;
        //HTREEITEM new_tree_item; 

        //if (instances->size() != 0)
        //{
        //    get_new_page_info(instances->at(0), page_info);
        //}
        //else
        //{

        //}
        //
        //ret = load_instruction_page(L"初始网页布局", &page_info, root_item, &new_tree_item); 
        //if (ret != ERROR_SUCCESS)
        //{
        //    dbg_print(MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret); 
        //    break;
        //}

		new_config_info->parent = root_item;
		next_items.push_back( new_config_info ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			config_info = *next_items.rbegin(); 
			next_items.pop_back(); 
			ASSERT( NULL != config_info->xml_node ); 

			do 
			{
				do 
				{
					item = NULL; 

					hr = config_info->xml_node->get_nodeName( text.GetAddress() ); 
					if(FAILED(hr))
					{
						break; 
					}

					temp_text = text.operator wchar_t*(); 

					if( temp_text == NULL )
					{
						break; 
					}

					if( 0 != wcsicmp( temp_text, HTML_SCRIPT_ACTION_ELEMENT_NAME ) )
					{
						break; 
					}

					ret = load_html_element( xml_doc, 
						config_info->xml_node, 
						config_info->parent, 
						&item ); 

					if( ret !=	ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( NULL != item); 

					do 
					{
						hr = config_info->xml_node->get_firstChild( &sub_xml_node ); 
						if( FAILED(hr))
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						if( sub_xml_node == NULL )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						new_config_info = new HTML_ELEMENT_CONFIG_INFO2(); 
						if( new_config_info == NULL )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							break; 
						}

						new_config_info->xml_node = sub_xml_node; 
						new_config_info->parent = item; 
						next_items.push_back( new_config_info ); 
					} while ( FALSE ); 
				}while( FALSE ); 

				do 
				{
					hr = config_info->xml_node->get_nextSibling( &next_xml_node ); 
					if( FAILED(hr))
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					if( next_xml_node == NULL )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					new_config_info = new HTML_ELEMENT_CONFIG_INFO2(); 
					if( new_config_info == NULL )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					new_config_info->parent = config_info->parent; 
					new_config_info->xml_node = next_xml_node; 
					new_config_info->xml_node->AddRef(); 
					next_items.push_back( new_config_info ); 

				} while ( FALSE ); 
			}while( FALSE ); 

			config_info->xml_node->Release(); 
			delete config_info; 

            //ret = resort_html_actions_id(); 
            //if (ret != ERROR_SUCCESS)
            //{
            //    break;
            //}
        } 
	}while( FALSE ); 

	ASSERT( next_items.empty() == true ); 

	next_items.clear(); 

	return ret; 
}

LRESULT html_element_prop_dlg::resort_html_actions_id()
{
    LRESULT ret = ERROR_SUCCESS;
    HTREEITEM root_item; 
    HTML_ELEMENT_ACTION *action; 

    do
    {
        root_item = tree_elements.GetChildItem(TVI_ROOT);
        if (root_item == NULL)
        {
            break;
        }

        action = ( HTML_ELEMENT_ACTION*)tree_elements.GetItemData(root_item); 
        if (action == NULL)
        {
            break; 
        }

    } while (FALSE); 

    return ret;
}

void html_element_prop_dlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	//CDialog::OnCancel();
}

/*********************************************
发送方式:
1.平面化，将所有的元素抽出，平面化为一维配置元素数据
2.单个元素按顺序逐次取出，一一发送
*********************************************/

//class html_element_action : public ::google::protobuf::Message {
//public:
//	html_element_action();
//	virtual ~html_element_action();
//
//	html_element_action(const html_element_action& from);
//
//	inline html_element_action& operator=(const html_element_action& from) {
//		CopyFrom(from);
//		return *this;
//	}
//
//	inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
//		return _unknown_fields_;
//	}
//
//	inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
//		return &_unknown_fields_;
//	}
//
//	static const ::google::protobuf::Descriptor* descriptor();
//	static const html_element_action& default_instance();
//
//	void Swap(html_element_action* other);
//
//	// implements Message ----------------------------------------------
//
//	html_element_action* New() const;
//	void CopyFrom(const ::google::protobuf::Message& from);
//	void MergeFrom(const ::google::protobuf::Message& from);
//	void CopyFrom(const html_element_action& from);
//	void MergeFrom(const html_element_action& from);
//	void Clear();
//	bool IsInitialized() const;
//
//	int ByteSize() const;
//	bool MergePartialFromCodedStream(
//		::google::protobuf::io::CodedInputStream* input);
//	void SerializeWithCachedSizes(
//		::google::protobuf::io::CodedOutputStream* output) const;
//	::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
//	int GetCachedSize() const { return _cached_size_; }
//private:
//	void SharedCtor();
//	void SharedDtor();
//	void SetCachedSize(int size) const;
//public:
//
//	::google::protobuf::Metadata GetMetadata() const;
//
//	// nested types ----------------------------------------------------
//
//	// accessors -------------------------------------------------------
//
//	// required string xpath = 1;
//	inline bool has_xpath() const;
//	inline void clear_xpath();
//	static const int kXpathFieldNumber = 1;
//	inline const ::std::string& xpath() const;
//	inline void set_xpath(const ::std::string& value);
//	inline void set_xpath(const char* value);
//	inline void set_xpath(const char* value, size_t size);
//	inline ::std::string* mutable_xpath();
//	inline ::std::string* release_xpath();
//	inline void set_allocated_xpath(::std::string* xpath);
//
//	// required string action = 2;
//	inline bool has_action() const;
//	inline void clear_action();
//	static const int kActionFieldNumber = 2;
//	inline const ::std::string& action() const;
//	inline void set_action(const ::std::string& value);
//	inline void set_action(const char* value);
//	inline void set_action(const char* value, size_t size);
//	inline ::std::string* mutable_action();
//	inline ::std::string* release_action();
//	inline void set_allocated_action(::std::string* action);
//
//	// required string param = 3;
//	inline bool has_param() const;
//	inline void clear_param();
//	static const int kParamFieldNumber = 3;
//	inline const ::std::string& param() const;
//	inline void set_param(const ::std::string& value);
//	inline void set_param(const char* value);
//	inline void set_param(const char* value, size_t size);
//	inline ::std::string* mutable_param();
//	inline ::std::string* release_param();
//	inline void set_allocated_param(::std::string* param);
//
//	// @@protoc_insertion_point(class_scope:data_scrabmle.html_element_action)
//private:
//	inline void set_has_xpath();
//	inline void clear_has_xpath();
//	inline void set_has_action();
//	inline void clear_has_action();
//	inline void set_has_param();
//	inline void clear_has_param();
//
//	::google::protobuf::UnknownFieldSet _unknown_fields_;
//
//	::std::string* xpath_;
//	::std::string* action_;
//	::std::string* param_;
//
//	mutable int _cached_size_;
//	::google::protobuf::uint32 _has_bits_[(3 + 31) / 32];
//
//	friend void  protobuf_AddDesc_data_5fscramble_2eproto();
//	friend void protobuf_AssignDesc_data_5fscramble_2eproto();
//	friend void protobuf_ShutdownFile_data_5fscramble_2eproto();
//
//	void InitAsDefaultInstance();
//	static html_element_action* default_instance_;
//};

using namespace data_scrabmle; 

LRESULT WINAPI _html_element_action_in_new_page( HTML_ACTION_CONTEXT *context )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != context ); 

		if( context->locate_url.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 == wcscmp( context->current_url.c_str(), 
			context->locate_url.c_str() ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI html_element_action_is_new_page( html_element_action *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != action ); 

		if( action->url().length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

//LRESULT WINAPI __exec_element_action( html_element_action &action, 
//									element_action_handler *handler )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	LRESULT _ret; 
//	bool __ret; 
//	HRESULT hr; 
//
//	string ansi_text; 
//
//	PVOID data_out = NULL; 
//	ULONG data_out_size; 
//
//	PVOID data_in = NULL; 
//	ULONG data_in_size; 
//
//	pipe_ipc_point point; 
//	BOOLEAN pipe_inited = FALSE; 
//	BOOLEAN pipe_connected = FALSE; 
//	WCHAR pipe_name[ MAX_PATH ]; 
//
//#define MAX_PIPE_DATA_LEN ( 1024 * 1024 * 6 )
//
//	do
//	{
//		data_out_size = action.ByteSize(); 
//		data_out = malloc( data_out_size ); 
//		if( NULL == data_out )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			break; 
//		}
//
//		if( false == action.IsInitialized() )
//		{
//			action.CheckInitialized(); 
//			REMOTE_ERROR_DEBUG(); 
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		if( false == action.SerializeToArray( data_out, data_out_size ) )
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, GetCurrentProcessId() ); 
//		if( FAILED(hr))
//		{
//
//		}
//
//		ret = init_pipe_point( &point, pipe_name ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		ret = create_name_pipe( point.pipe_name, &point.pipe ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		ret = html_element_action_is_new_page( &action ); 
//		if( ret == ERROR_SUCCESS )
//		{
//			start_webbrowser(); 
//		}
//
//		do 
//		{
//			pipe_connected = FALSE; 
//
//			ret = accept_name_pipe_client_sync( point.pipe ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			pipe_connected = TRUE; 
//
//			ret = read_pipe_sync( &point, ( CHAR* )&data_in_size, sizeof( data_in_size ) ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			if( data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN )
//			{
//				ret = ERROR_INVALID_DATA; 
//				break; 
//			}
//
//			data_in = ( CHAR* )malloc( data_in_size ); 
//			if( data_in == NULL )
//			{
//				ret = ERROR_NOT_ENOUGH_MEMORY; 
//				break; 
//			}
//
//			ret = read_pipe_sync( &point, ( CHAR* )data_in, data_in_size ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			__ret = action.ParseFromArray( data_in, data_in_size ); 
//			if( false == __ret )
//			{
//				break; 
//			}
//
//			if( false == action.has_action() )
//			{
//				break; 
//			}
//
//			//if( 0 != strcmp( action.action().c_str(), HTML_ELEMENT_ACTION_START_A ) )
//			//{
//			//	break; 
//			//}
//
//			ret = write_pipe_sync( &point, ( CHAR* )&data_out_size, sizeof( ULONG ) ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			ret = write_pipe_sync( &point, ( CHAR* )data_out, data_out_size ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//		}while( FALSE ); 
//
//		if( TRUE == pipe_connected )
//		{
//			ret = disconnect_name_pipe_client( point.pipe ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				//break; 
//			}
//		}
//
//		if( NULL != data_in )
//		{
//			free( data_in ); 
//			data_in = NULL; 
//		}
//
//		if( NULL != data_out )
//		{
//			free( data_out ); 
//			data_out = NULL; 
//		}
//
//		//ret = handler->send_data( data, data_size, &point ); 
//		//if( ret != ERROR_SUCCESS )
//		//{
//		//	break; 
//		//}
//	}while( FALSE );
//
//	if( TRUE == pipe_inited )
//	{
//		uninit_pipe_point( &point ); 
//	}
//
//	if( NULL != data_in)
//	{
//		free( data_in ); 
//	}
//	
//	//*data_out = data; 
//	return ret; 
//}

LRESULT WINAPI config_element_action( HTML_ELEMENT_ACTION *element, 
									 HTML_ACTION_CONTEXT *context, 
									 html_element_action *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
    wstring output_data;
	string utf8_text; 

	do 
	{
		ASSERT( NULL != element ); 
		ASSERT( NULL != action ); 

		if( 0 == _wcsicmp( L"none", 
			element->action.c_str() ) )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		{
			CHAR text[ 12 ]; 
			HRESULT hr; 

			hr = StringCchPrintfA( text, ARRAYSIZE( text ), "%u", element->id ); 
			if( FAILED( hr) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				ASSERT( FALSE ); 
				break; 
			}

			action->set_id( text ); 
		}

		ret = unicode_to_utf8_ex( element->action.c_str(), 
			utf8_text ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( utf8_text.length() == 0 )
		{
			ASSERT( FALSE ); 
			//ASSERT( 0 == wcsicmp( L"none", 
			//	element->param.c_str() ) )
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			dbg_print( MSG_IMPORTANT, "the action of the action is null\n" ); 
			break; 
		}

		action->set_action( utf8_text.c_str() ); 

		ret = unicode_to_utf8_ex( element->xpath.c_str(), 
			utf8_text ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( utf8_text.length() == 0 )
		{
			if( ERROR_SUCCESS != check_no_html_element_action( element ) )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				dbg_print( MSG_IMPORTANT, "the xpath of the action is null\n" ); 
				ASSERT( FALSE ); 
				break; 
			}
		}

		action->set_xpath( utf8_text.c_str() ); 

		ret = unicode_to_utf8_ex( element->param.c_str(), 
			utf8_text ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( utf8_text.length() == 0 )
		{
			dbg_print( MSG_IMPORTANT, "the parameter of the action is null\n" ); 
		}

		action->set_param( utf8_text.c_str() ); 

		utf8_text.clear(); 
		action->set_url( utf8_text.c_str() ); 

		do 
		{
			_ret = _html_element_action_in_new_page( context ); 

			if( ERROR_SUCCESS != _ret )
			{
				break; 
			}
			
			ret = unicode_to_utf8_ex( context->locate_url.c_str(), 
				utf8_text ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( utf8_text.length() == 0 )
			{
				//ret = ERROR_ERRORS_ENCOUNTERED; 
				dbg_print( MSG_IMPORTANT, "the xpath of the action is null\n" ); 
				//break; 
			}

			action->set_url( utf8_text.c_str() ); 
		} while ( FALSE );

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( 0 == wcscmp( element->action.c_str(), HTML_ELEMENT_ACTION_INPUT ) )
		{
            cat_output_data(element, output_data); 

			ret = unicode_to_utf8_ex(output_data.c_str(),
				utf8_text ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( utf8_text.length() == 0 )
			{
				//ret = ERROR_ERRORS_ENCOUNTERED; 
				dbg_print( MSG_IMPORTANT, "the xpath of the action is null\n" ); 
				//break; 
			}

			action->set_input( utf8_text.c_str() );
		}

		if( false == action->IsInitialized() )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

/**************************************************************
命令执行方式:
1.解析时执行
2.解析命令, 将命令化为一个命令队列
	工作线程等待目标浏览器进程请求命令
	查询下一条命令
	将下一条命令发送至目标浏览器
	等待目标浏览器的反馈
	将反馈进一步分解为新的命令
**************************************************************/
//LRESULT WINAPI _exec_element_action( ELEMENT_CONFIG *element, 
//								   element_action_handler *handler )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	html_element_action action_out; 
//
//	do 
//	{
//		ret = config_element_action( element, &action_out ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		ret = __exec_element_action( action_out, handler ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//	}while( FALSE ); 	
//	//*data_out = data; 
//	return ret; 
//}

#if 0 
LRESULT WINAPI _exec_browser_action( html_element_action &action_out, 
								   element_action_handler *handler )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	bool __ret; 
	HRESULT hr; 

	PVOID data_out = NULL; 
	ULONG data_out_size; 

	PVOID data_in = NULL; 
	ULONG data_in_size; 

	pipe_ipc_point point; 
	BOOLEAN pipe_inited = FALSE; 
	BOOLEAN pipe_connected = FALSE; 
	WCHAR pipe_name[ MAX_PATH ]; 

#define MAX_PIPE_DATA_LEN ( 1024 * 1024 * 6 )

	do 
	{
		data_out_size = action_out.ByteSize(); 
		data_out = malloc( data_out_size ); 
		if( NULL == data_out )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		if( false == action_out.IsInitialized() )
		{
			action_out.CheckInitialized(); 
			REMOTE_ERROR_DEBUG(); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( false == action_out.SerializeToArray( data_out, data_out_size ) )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, GetCurrentProcessId() ); 
		if( FAILED(hr))
		{

		}

		ret = init_pipe_point( &point, pipe_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = create_name_pipe( point.pipe_name, &point.pipe ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = html_element_action_is_new_page( action ); 
		if( ret == ERROR_SUCCESS )
		{
			start_webbrowser(); 
		}

		do 
		{
			pipe_connected = FALSE; 

			ret = accept_name_pipe_client_sync( point.pipe ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			pipe_connected = TRUE; 

			ret = read_pipe_sync( &point, ( CHAR* )&data_in_size, sizeof( data_in_size ) ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN )
			{
				ret = ERROR_INVALID_DATA; 
				break; 
			}

			data_in = ( CHAR* )malloc( data_in_size ); 
			if( data_in == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			ret = read_pipe_sync( &point, ( CHAR* )data_in, data_in_size ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			__ret = action_out.ParseFromArray( data_in, data_in_size ); 
			if( false == __ret )
			{
				break; 
			}

			if( false == action_out.has_action() )
			{
				break; 
			}

			if( 0 != strcmp( action_out.action().c_str(), HTML_ELEMENT_ACTION_START_A ) )
			{
				break; 
			}

			//ret = unicode_to_utf8_ex( element->action.c_str(), ansi_text ); 
			//if( ERROR_SUCCESS != ret )
			//{
			//	break; 
			//}

			ret = write_pipe_sync( &point, ( CHAR* )&data_out_size, sizeof( ULONG ) ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = write_pipe_sync( &point, ( CHAR* )data_out, data_out_size ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while( FALSE ); 

		if( TRUE == pipe_connected )
		{
			ret = disconnect_name_pipe_client( point.pipe ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}
		}

		if( NULL != data_in )
		{
			free( data_in ); 
			data_in = NULL; 
		}

		if( NULL != data_out )
		{
			free( data_out ); 
			data_out = NULL; 
		}

		//ret = handler->send_data( data, data_size, &point ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}
	}while( FALSE );

	return ret; 
}
#endif //0

LRESULT WINAPI dump_html_element_action( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != action ); 

		dbg_print( MSG_IMPORTANT, "html element action %p:\n", action ); 
		dbg_print( MSG_IMPORTANT, "xpath:%ws,", action->xpath.c_str() ); 
		dbg_print( MSG_IMPORTANT, "action:%ws, ", action->action.c_str() ); 
		dbg_print( MSG_IMPORTANT, "flag:%ws, ", action->param.c_str() ); 
		dbg_print( MSG_IMPORTANT, "html:%ws, ", action->html_text.c_str() ); 
		dbg_print( MSG_IMPORTANT, "id:%u, ", action->id ); 
		dbg_print( MSG_IMPORTANT, "jump to id:%u\n", action->jump_to_id ); 
	}while( FALSE );

	return ret; 
}

/**********************************************************************************
执行HTML元素指令方式:
1. 
**********************************************************************************/
typedef LRESULT ( CALLBACK *traverse_html_element_action_func )( HTML_ELEMENT_ACTION *action, 
													  PVOID context ); 

LRESULT WINAPI get_next_url_from_output( wstring &output, wstring &url )
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring::size_type offset; 
	
	do 
	{
		if( output.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		offset = output.find( _URL_DELIM_CHAR ); 
		if( offset == string::npos )
		{
			url = output; 
			output = L""; 
			break; 
		}

		url = output.substr( 0, offset ); 
		output = output.substr( offset + 1 ); 
	}while( FALSE );

	return ret; 
}

//LRESULT CALLBACK exec_html_action_direct( ELEMENT_CONFIG *action )
//{
//	if( NULL != action->parent_item )
//	{
//		ELEMENT_CONFIG *parent; 
//		parent = action->parent_item; 
//
//		if( 0 != wcscmp( parent->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) )
//		{
//			ASSERT( FALSE ); 
//			break; 
//		}
//
//		if( parent->output.length() == 0 )
//		{
//			dbg_print( MSG_ERROR, "can't get the html content from the xpath(%s) on url(%s)\n", 
//				parent->xpath.c_str(), 
//				parent->url.c_str() ); 
//		}
//
//		if( 0 == wcscmp( parent->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
//		{
//			ret = config_element_action( action, &_action ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//
//			ret = __exec_element_action( _action, &action_handler ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				break; 
//			}
//		}
//		else if( 0 == wcscmp( parent->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
//		{
//
//		}
//	}while( FALSE ); 
//	return ret; 
//}

LRESULT WINAPI check_action_have_response( HTML_ELEMENT_ACTION *html_command )
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 

	do 
	{
		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, html_command->action.c_str() ) )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}

        if (0 == wcscmp(HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE, html_command->param.c_str()))
        {
            ret = ERROR_SUCCESS;
            break;
        }
	}while( FALSE );

	return ret; 
}

typedef struct _RETURN_INFO
{
	LRESULT return_code; 
	wstring message; 
} RETURN_INFO, *PRETURN_INFO; 

LRESULT CALLBACK analyze_html_element_action_response( HTML_ELEMENT_ACTION *root, 
													  HTML_ELEMENT_ACTION *action, 
													  HTML_ACTION_CONTEXT *action_context, 
													  PVOID data_in, 
													  ULONG data_in_size, 
													  RETURN_INFO *return_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	bool _ret; 
	html_element_action_response response; 
	ULONG id; 
	CHAR *text_end; 
	wstring unicode_text; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != data_in ); 
		ASSERT( 0 != data_in_size ); 
		ASSERT( 0 < action_context->locate_url.length( ) ); 

		__ret = check_action_have_response( action ); 
		if( __ret != ERROR_SUCCESS )
		{
			//ASSERT( FALSE ); 
			//break; 
		}

        return_info->return_code = 0; 
        return_info->message = L""; 

		_ret = response.ParseFromArray( data_in, data_in_size ); 
		if( false == _ret )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( response.response_size() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//dbg_print( MSG_IMPORTANT, "receive the response from the web client %s\n", response.response(0).c_str() ); 

		id = strtoul( response.id().c_str(), &text_end, 0 ); 
		if( id != action->id )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, action->action.c_str() ) )
		{
            INT32 i; 
            BOOLEAN  locate_web_page = FALSE; 

            if (response.response(0).size())
            {
                action_context->current_url = action_context->locate_url;
                locate_web_page = TRUE; 
            }

			do 
			{
				if( 0 != wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, action->param.c_str() ) )
				{
					break; 
				}

				if( action_context->locate_url.length() == 0 )
				{
					break; 
				}

				if(FALSE == locate_web_page)
				{
					break; 
				}

				ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				action_context->current_page = unicode_text; 
			}while( FALSE ); 

            if (response.response_size() < 1)
            {
                //ASSERT( FALSE ); 
                ret = ERROR_INVALID_PARAMETER;
                break;
            }

            action->outputs.clear();
            for (i = 1; i < response.response_size(); i++)
            {
                ret = utf8_to_unicode_ex(response.response(i).c_str(), unicode_text);
                if (ERROR_SUCCESS != ret)
                {
                    break;
                }

                action->outputs.push_back(wstring(unicode_text));
            }
		}
		else if( 0 == wcscmp( HTML_ELEMENT_ACTION_CLICK, action->action.c_str() ) )
		{ 
			wstring url; 
			wstring return_code; 
            ULONG i; 

			ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

            return_code = unicode_text;

            if (response.response_size() > 1)
            {
                ret = utf8_to_unicode_ex(response.response(1).c_str(), unicode_text);
                if (ERROR_SUCCESS != ret)
                {
                    break;
                }

                url = unicode_text; 
            }

            action->outputs.clear(); 
            for (i = 2; i < (ULONG)response.response_size(); i++)
            {
                ret = utf8_to_unicode_ex(response.response(i).c_str(), unicode_text);
                if (ERROR_SUCCESS != ret)
                {
                    break;
                }

                action->outputs.push_back(unicode_text); 
            }

            if (0 != wcscmp(L"0", return_code.c_str()))
            {
                WCHAR *end_text;
                ret = wcstol(return_code.c_str(), &end_text, 0);
                break;
            }

			action_context->current_url = url; 

			{
				//HTML_ELEMENT_ACTION *container; 
				//ret = get_parent_page_layout_node( action, &container ); 
				//if( ret != ERROR_SUCCESS )
				//{
				//	ASSERT( FALSE ); 
				//}

				//ASSERT( NULL != container ); 

				if( 0 == wcscmp( action->param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ) )
				{
					if( 0 == wcscmp( action_context->locate_url.c_str(), url.c_str() ) )
					{
						dbg_print_w( MSG_IMPORTANT, L"url(%s) is not changed by click (reload page mode) action\n", url.c_str() ); 
					}

					action_context->locate_url = url; 
				}
			}
		}
		else if( 0 == wcscmp( HTML_ELEMENT_ACTION_BACK, action->action.c_str() ) )
		{
			wstring::size_type offset; 
			wstring url; 
			wstring return_code; 

			ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			do 
			{
				if( unicode_text.length() == 0 )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				offset = unicode_text.find( RESPONSE_TEXT_DELIM ); 
				if( offset == string::npos )
				{
					ASSERT( FALSE ); 
					return_code = unicode_text; 
					url = L""; 
					break; 
				}

				return_code = unicode_text.substr( 0, offset ); 
				url = unicode_text.substr( offset + 1 ); 
			}while( FALSE ); 

			{
				WCHAR *end_text; 
				ret = wcstol( return_code.c_str(), &end_text, 0 ); 

				return_info->return_code = ret; 
				//return_info->message = 
			}

			if( ERROR_SUCCESS != ret ) // != wcscmp( L"0", return_code.c_str() ) )
			{
				break; 
			}

			action_context->current_url = url; 

			{
				HTML_ELEMENT_ACTION *container; 
				__ret = get_parent_page_layout_node(  action, &container ); 
				if( __ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
				}

				ASSERT( NULL != container ); 

				do
				{
					WEB_BROWSER_PROCESS *process; 
					__ret = get_web_browser_process_ex( container, 
						&process, 
						GET_ROOT_ELEMENT_INFO ); 
					if( __ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( &process->context == action_context ); 
				}while( FALSE ); 

				if( 0 == wcscmp( action_context->locate_url.c_str(), url.c_str() ) )
				{
					//ASSERT( FALSE ); 
					dbg_print_w( MSG_IMPORTANT, L"url(%s) is not changed by back action\n", url.c_str() ); 
				}

				action_context->locate_url = url; 
			}
		}
		else
		{
			if( 0 != strcmp( "0", response.response(0).c_str() ) )
			{
				CHAR *end_text; 

				ret = strtol( response.response(0).c_str(), &end_text, 0 ); 
				break; 
			}

			action_context->current_url = action_context->locate_url; 
		}
	}while( FALSE );

    return_info->return_code = ret; 

	return ret; 
}

LRESULT CALLBACK _analyze_html_element_action_response( HTML_ELEMENT_ACTION *action, 
													  HTML_ACTION_CONTEXT *action_context, 
													  PVOID data_in, 
													  ULONG data_in_size, 
													  RETURN_INFO *return_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	bool _ret; 
	html_element_action_response response; 
	ULONG id; 
	CHAR *text_end; 
	wstring unicode_text; 

	do 
	{
		ASSERT( NULL != data_in ); 
		ASSERT( 0 != data_in_size ); 

		__ret = check_action_have_response( action ); 
		if( __ret != ERROR_SUCCESS )
		{
			//ASSERT( FALSE ); 
			//break; 
		}

		_ret = response.ParseFromArray( data_in, data_in_size ); 
		if( false == _ret )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( response.response_size() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		dbg_print( MSG_IMPORTANT, "receive the response from the web client %s\n", response.response(0).c_str() ); 

		id = strtoul( response.id().c_str(), &text_end, 0 ); 
		if( id != action->id )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, action->action.c_str() ) )
		{
			ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

            action->outputs.clear(); 
			action->outputs.push_back( unicode_text ); 
			action_context->current_url = action_context->locate_url; 

			do 
			{
				if( 0 != wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, action->param.c_str() ) )
				{
					break; 
				}

				if( action_context->locate_url.length() == 0 )
				{
					break; 
				}

				if( response.response_size() < 2 )
				{
					//ASSERT( FALSE ); 
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = utf8_to_unicode_ex( response.response(1).c_str(), unicode_text ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				action_context->current_page = unicode_text; 
			}while( FALSE ); 
		}
		else if( 0 == wcscmp( HTML_ELEMENT_ACTION_CLICK, action->action.c_str() ) )
		{
			wstring::size_type offset; 
			wstring url; 
			wstring return_code; 

			ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			do 
			{
				if( unicode_text.length() == 0 )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				offset = unicode_text.find( RESPONSE_TEXT_DELIM ); 
				if( offset == string::npos )
				{
					ASSERT( FALSE ); 
					return_code = unicode_text; 
					url = L""; 
					break; 
				}

				return_code = unicode_text.substr( 0, offset ); 
				url = unicode_text.substr( offset + 1 ); 
			}while( FALSE ); 

			if( 0 != wcscmp( L"0", return_code.c_str() ) )
			{
				WCHAR *end_text; 

				ret = wcstol( return_code.c_str(), &end_text, 0 ); 
				break; 
			}

			action_context->current_url = url; 

			//{
			//	ELEMENT_CONFIG *container; 
			//	ret = get_container_parent( root, action, &container, INFINITE_SEARCH_LEVEL_COUNT, NULL ); 
			//	if( ret != ERROR_SUCCESS )
			//	{
			//		ASSERT( FALSE ); 
			//	}

			//	ASSERT( NULL != container ); 

			//	if( 0 == wcscmp( action->param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ) 
			//		&& 0 == wcscmp( container->url.c_str(), url.c_str() ) )
			//	{
			//		//ASSERT( FALSE ); 
			//		dbg_print_w( MSG_IMPORTANT, L"url(%s) is not changed by click (reload page mode) action\n", url.c_str() ); 
			//	}

			//	container->url = url; 
			//}
		}
		else if( 0 == wcscmp( HTML_ELEMENT_ACTION_BACK, action->action.c_str() ) )
		{
			wstring::size_type offset; 
			wstring url; 
			wstring return_code; 

			ret = utf8_to_unicode_ex( response.response(0).c_str(), unicode_text ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			do 
			{
				if( unicode_text.length() == 0 )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				offset = unicode_text.find( RESPONSE_TEXT_DELIM ); 
				if( offset == string::npos )
				{
					ASSERT( FALSE ); 
					return_code = unicode_text; 
					url = L""; 
					break; 
				}

				return_code = unicode_text.substr( 0, offset ); 
				url = unicode_text.substr( offset + 1 ); 
			}while( FALSE ); 

			{
				WCHAR *end_text; 
				ret = wcstol( return_code.c_str(), &end_text, 0 ); 

				return_info->return_code = ret; 
				//return_info->message = 
			}

			if( ERROR_SUCCESS != ret ) // != wcscmp( L"0", return_code.c_str() ) )
			{
				break; 
			}

			action_context->current_url = url; 

			//{
			//	ELEMENT_CONFIG *container; 
			//	ret = get_container_parent( root, action, &container, INFINITE_SEARCH_LEVEL_COUNT, NULL ); 
			//	if( ret != ERROR_SUCCESS )
			//	{
			//		ASSERT( FALSE ); 
			//	}

			//	ASSERT( NULL != container ); 

			//	if( 0 == wcscmp( container->url.c_str(), url.c_str() ) )
			//	{
			//		//ASSERT( FALSE ); 
			//		dbg_print_w( MSG_IMPORTANT, L"url(%s) is not changed by click (reload page mode) action\n", url.c_str() ); 
			//	}

			//	container->url = url; 
			//}
		}
		else
		{
			if( 0 != strcmp( "0", response.response(0).c_str() ) )
			{
				CHAR *end_text; 

				ret = strtol( response.response(0).c_str(), &end_text, 0 ); 
				break; 
			}

			action_context->current_url = action_context->locate_url; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK get_html_element_action( HTML_ELEMENT_ACTION *html_command, 
										 HTML_ACTION_CONTEXT *context, 
										 PVOID data_in, 
										 ULONG data_in_size, 
										 PVOID *data_out, 
										 ULONG *data_out_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	bool _ret; 
	html_element_action_request request; 
	html_element_action action; 
	PVOID _data_out = NULL; 
	ULONG _data_out_size; 
	
	do 
	{
		ASSERT( NULL != context ); 
		ASSERT( NULL != html_command ); 
		ASSERT( NULL != data_in ); 
		ASSERT( 0 != data_in_size ); 
		ASSERT( NULL != data_out ); 
		ASSERT( NULL != data_out_size ); 

		*data_out = NULL; 
		*data_out_size = 0; 

		_ret = request.ParseFromArray( data_in, data_in_size ); 
		if( false == _ret )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( 0 == strcmp( request.request().c_str(), HTML_ELEMENT_REQUEST_NEXT_ACTION_A ) )
		{
			ret = config_element_action( html_command, 
				context, 
				&action ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			_data_out_size = action.ByteSize(); 
			if( _data_out_size == 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				ASSERT( FALSE ); 
				break; 
			}

			if( _data_out_size > MAX_PIPE_DATA_LEN )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_data_out = malloc( _data_out_size ); 
			if( NULL == _data_out )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			_ret = action.SerializeToArray( _data_out, _data_out_size ); 
			if( false == _ret )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			*data_out = _data_out; 
			_data_out = NULL; 
			*data_out_size = _data_out_size; 
		}
		else
		{
			dbg_print( MSG_FATAL_ERROR, "unknown html command request %s\n", request.request().c_str() ); 
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	if( NULL != _data_out )
	{
		free( _data_out ); 
	}

	return ret; 
}

LRESULT exec_html_elemnt_actions_in_order( HTML_ELEMENT_ACTION *root, 
											 traverse_html_element_action_func traverse, 
											 PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *item; 
	HTML_ELEMENT_ACTION *sub_item; 
	HTML_ELEMENT_ACTION *next_item; 
	HTML_ELEMENT_ACTION *parent_item; 
	vector< HTML_ELEMENT_ACTION* > next_items; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != traverse ); 

		next_items.push_back( root ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 

			traverse( item, context ); 

			do 
			{
				next_item = item->next_item; 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = item->sub_item; 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 
				parent_item = item; 
			} while ( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT traverse_html_elemnt_action_in_order( HTML_ELEMENT_ACTION *root, 
														   traverse_html_element_action_func traverse, 
														   PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *item; 
	HTML_ELEMENT_ACTION *sub_item; 
	HTML_ELEMENT_ACTION *next_item; 
	vector< HTML_ELEMENT_ACTION* > next_items; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != traverse ); 

		next_items.push_back( root ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 

			traverse( item, context ); 

			do 
			{
				next_item = item->next_item; 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = item->sub_item; 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 
			} while ( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT CALLBACK add_action_to_list( HTML_ELEMENT_ACTION *action, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION_LIST *list; 

	ASSERT( action != NULL ); 
	ASSERT( context != NULL ); 

	list = ( HTML_ELEMENT_ACTION_LIST* )context; 

	list->push_back( action ); 

	return ret; 
}

LRESULT html_element_prop_dlg::add_item_action_to_list( HTREEITEM item, 
										 PVOID data, 
										 PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION_LIST *list; 

	ASSERT( data != NULL ); 
	ASSERT( context != NULL ); 

	list = ( HTML_ELEMENT_ACTION_LIST* )context; 

	list->push_back( ( HTML_ELEMENT_ACTION *)data ); 

	return ret; 
}

LRESULT test_tree_traverse_function( HTML_ELEMENT_ACTION_LIST &list_from_tree_ctrl, 
									HTML_ELEMENT_ACTION_LIST &list_from_tree_actions ); 

LRESULT html_element_prop_dlg::expand_html_action_script( HTREEITEM item, 
														 HTML_ELEMENT_ACTION *root, 
														 HTML_ELEMENT_ACTION_LIST &actions )
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	HTML_ELEMENT_ACTION_LIST _actions; 
	
	do 
	{
		/*****************************************
		1.从上至下，逐条发送
		2.从父到子，逐条发送
		*****************************************/
		ASSERT( NULL != root ); 

		ret = traverse_html_elemnt_action_in_order( root, 
			add_action_to_list, 
			&actions ); 

		ret = traverse_tree_item_in_order( item, 
			&html_element_prop_dlg::add_item_action_to_list, 
			&_actions ); 

		ret = test_tree_traverse_function( actions, _actions ); 

		//config = root; 
		//for( ; ; )
		//{
		//	if( NULL == config )
		//	{
		//		break; 
		//	}

		//	actions.push_back( config ); 

		//	config = config->next_item; 
		//	//dbg_print( MSG_ERROR, "send element action error %d\n", ret ); 
		//}

		//HTML_ELEMENT_ACTION_LIST_ITERATOR it = actions.begin(); 

		//for( ; ; )
		//{
		//	found_sub_item = FALSE; 

		//	for( ; 
		//		it != actions.end(); 
		//		it ++ )
		//	{
		//		sub_config = ( *it )->sub_item; 

		//		if( NULL == sub_config )
		//		{
		//			break; 
		//		}

		//		if( found_sub_item == FALSE )
		//		{
		//			found_sub_item = TRUE; 
		//		}

		//		config = sub_config; 

		//		for( ; ; )
		//		{
		//			if( NULL == config )
		//			{
		//				break; 
		//			}

		//			sub_actions.push_back( config ); 

		//			//dbg_print( MSG_ERROR, "send element action error %d\n", ret ); 
		//			config = config->next_item; 
		//		}

		//		for( HTML_ELEMENT_ACTION_LIST::reverse_iterator sub_item_it = sub_actions.rbegin(); 
		//			sub_item_it != sub_actions.rend(); 
		//			sub_item_it ++ )
		//		{
		//			actions.insert( it, ( *sub_item_it ) ); 
		//		}

		//		sub_actions.clear(); 
		//	}

		//	if( FALSE == found_sub_item )
		//	{
		//		break; 
		//	}
		//}

#ifdef _DEBUG
		ULONG count = 0; 
		for( HTML_ELEMENT_ACTION_LIST_ITERATOR it = actions.begin(); 
			it != actions.end(); 
			it ++ )
		{
			dbg_print( MSG_IMPORTANT, "[No.%u]", count ); //( ULONG )( DWORD_PTR )( it - actions.begin() ) )
			ASSERT( NULL != ( *it ) ); 
			dump_html_element_action( ( *it ) ); 
			count ++; 
		}
#endif //_DEBUG

	}while( FALSE ); 

	return ret; 
}

LRESULT test_tree_traverse_function( HTML_ELEMENT_ACTION_LIST &list_from_tree_ctrl, 
									HTML_ELEMENT_ACTION_LIST &list_from_tree_actions )
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 i; 

	do 
	{
		if( list_from_tree_actions.size() != list_from_tree_ctrl.size() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		HTML_ELEMENT_ACTION_LIST_ITERATOR it_src; 
		HTML_ELEMENT_ACTION_LIST_ITERATOR it_dest; 
		
		it_src = list_from_tree_actions.begin(); 
		it_dest = list_from_tree_ctrl.begin(); 

		for( ; ; )
		{
			if( it_src == list_from_tree_actions.end() )
			{
				ASSERT( it_dest == list_from_tree_ctrl.end() ); 
				break; 
			}

			if( *it_src != *it_dest )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			it_src ++; 
			it_dest ++; 
		}
	} while ( FALSE );
	
	return ret; 
}

//LRESULT CALLBACK exec_html_action_callback( ELEMENT_CONFIG *action, PVOID context )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	element_action_handler *handler; 
//	
//	do 
//	{
//		ASSERT(NULL != action ); 
//		ASSERT(NULL != context); 
//		
//		handler = ( element_action_handler* )context; 
//		ret = _exec_element_action( action, handler ); 
//
//	}while( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI exec_html_action_script( HTML_ELEMENT_ACTION *root, 
									   element_action_handler *handler)
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	HTML_ELEMENT_ACTION *config; 
	//HTML_ELEMENT_ACTION_LIST_ITERATOR it; 
	//HTML_ELEMENT_ACTION_LIST_ITERATOR jump_to_it; 
	//BOOLEAN found_jump_to; 

	do 
	{
		//_ret = exec_html_elemnt_actions_in_order( root, exec_html_action_callback, &action_handler ); 
		//if( ERROR_SUCCESS != _ret )
		//{
		//	ret = _ret; 
		//	//break; 
		//}

		//for( it = actions.begin(); 
		//	it != actions.end(); )
		//{
		//	found_jump_to = FALSE; 

		//	config = ( *it ); 
		//	_ret = _exec_element_action( config, handler ); 
		//	if( ERROR_SUCCESS != _ret )
		//	{
		//		ret = _ret; 
		//		//break; 
		//	}

		//	if( config->jump_to_id != INVALID_JUMP_TO_ID )
		//	{
		//		for( jump_to_it = actions.begin(); jump_to_it != actions.end(); jump_to_it ++ )
		//		{
		//			if( ( *jump_to_it )->id == config->jump_to_id )
		//			{
		//				found_jump_to = TRUE; 
		//				it = jump_to_it; 
		//				break; 
		//			}
		//		}
		//	}

		//	if( found_jump_to == TRUE )
		//	{
		//		continue; 
		//	}

		//	it ++; 
		//}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_html_element( HTML_ELEMENT_INFO *element_info, 
								MSXML::IXMLDOMNode *xml_node ); 

void html_element_prop_dlg::OnDrag()
{
}

void html_element_prop_dlg::OnDragMove(POINT point )
{

}

void html_element_prop_dlg::OnDragRelease(POINT point,HTREEITEM hDropTarget)
{
	LRESULT ret; 
	SEL_ITEM_LIST drag_items; 
	HTREEITEM drag_item; 

	do 
	{
		if( NULL == hDropTarget )
		{
			break; 
		}

		drag_items = tree_elements.GetSelectedItems(); 
		if( 0 == drag_items.size() )
		{
			ASSERT( FALSE ); 
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		drag_item = *drag_items.begin(); 
		if( drag_item == hDropTarget )
		{
			break; 
		}

		ret = move_tree_item( drag_item, hDropTarget ); 
	} while ( FALSE ); 
}

void html_element_prop_dlg::OnBnClickedButtonNew()
{
	LRESULT ret; 
	HRESULT hr; 
	CString config_file; 
	//VARIANT_BOOL __ret; 
	//LPCWSTR _temp_text; 
	// TODO: Add your control notification handler code here

	do 
	{
		GetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, config_file ); 

		if( config_file.GetAt( config_file.GetLength() - 1 ) == L'\\' )
		{
			break; 
		}

		ret = open_actions_config_file( config_file.GetBuffer(), config_file.GetLength() ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		//ret = load_html_element_config(); 

		vector<HTML_ELEMENT_INFO*> elements; 
		do
		{
			MSXML::IXMLDOMElementPtr root_element; 
			MSXML::IXMLDOMNodeListPtr sub_elements; 
			MSXML::IXMLDOMNodeListPtr xml_node_list; 
			MSXML::IXMLDOMNodePtr _xml_node; 
			MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
			MSXML::IXMLDOMNodePtr xml_attr; 

			HTML_ELEMENT_INFO *element_info = NULL; 

			LONG i; 
			LONG node_count; 
			//WCHAR *_temp_text; 

			hr = config_xml_doc->get_documentElement( &root_element ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( NULL == root_element )
			{
				break; 
			}

#ifdef PERFORMANCE_DEBUG
			ULONG tick_begin; 
			ULONG tick_end; 
#endif //PERFORMANCE_DEBUG

			_bstr_t temp_text; 
			_variant_t xml_node_value; 

			hr = root_element->QueryInterface( __uuidof( MSXML::IXMLDOMNode ), ( PVOID* )&_xml_node ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( NULL == _xml_node )
			{
				break; 
			}

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

			//do 
			//{
			//	hr = _xml_node->get_attributes( &xml_attrs ); 
			//	if( FAILED( hr ) )
			//	{
			//		ret = ERROR_INVALID_PARAMETER; 
			//		break; 
			//	}

			//	xml_attr = xml_attrs->getNamedItem( HTML_PAGE_URL ); 
			//	if( xml_attr == NULL )
			//	{ 
			//		break; 
			//	}

			//	ASSERT( xml_attr != NULL ); 
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
			//		break; 
			//	}
			//	else
			//	{
			//		html_page->url = temp_text; 
			//	}
			//}while( FALSE ); 

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

			size_t size; 
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

					size = elements.size(); 

					elements.push_back( element_info ); 

					if( elements.size() <= size )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}
				}while( FALSE ); 

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					delete element_info; 
					element_info = NULL; 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				//_xml_node->Release(); release auto
			}

			if( node_count != elements.size() ) //+ html_page->action_elements.size(
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			{
				ULONG item_count; 
				vector< HTML_ELEMENT_INFO* >::iterator it; 
				wstring action; 
				wstring param; 
				//for( it = elements.begin(); it != elements.end(); it ++ )
				//{
				//	action = ( *it )->action_config.action; 

				//	item_count = tree_elements.GetItemCount(); 

				//	tree_elements.InsertItem(item_count, ( *it )->title.c_str() ); 
				//	if( 0 == wcsicmp( action.c_str(), 
				//		L"get_data" ) )
				//	{
				//		tree_elements.SetItemText( item_count, 2, L"输出" ); 
				//	}
				//	else if( 0 == wcsicmp( action.c_str(), 
				//		L"set_data" ) )
				//	{
				//		tree_elements.SetItemText( item_count, 2, L"输入" ); 
				//	}
				//	else if( 0 == wcsicmp( action.c_str(), 
				//		L"click" ) )
				//	{
				//		tree_elements.SetItemText( item_count, 2, L"点击" ); 
				//	}

				//	param = ( *it )->action_config.param.c_str(); 
				//	if( 0 == wcsicmp( param.c_str(), 
				//		L"text" ) )
				//	{
				//		tree_elements.SetItemText( item_count, 3, L"文本" ); 
				//	}
				//	else if( 0 == wcsicmp( action.c_str(), 
				//		L"link" ) )
				//	{
				//		tree_elements.SetItemText( item_count, 3, L"链接地址" ); 
				//	}
				//	else
				//	{
				//		ASSERT( FALSE ); 
				//	}

				//	tree_elements.SetItemText(item_count, 1, ( *it )->action_config.xpath.c_str() ); 
				//}
			}
#ifdef PERFORMANCE_DEBUG
			tick_end = GetTickCount(); 
			dbg_print( MSG_IMPORTANT, "time usage: %u", tick_end - tick_begin ); 
#endif //PERFORMANCE_DEBUG
		}while( FALSE ); 
	} while ( FALSE ); 

	return; 
}

void html_element_prop_dlg::OnBnClickedButtonFileBrowser()
{
	LRESULT ret; 
	HRESULT hr; 
	INT32 _ret; 
	// TODO: Add your control notification handler code here
	WCHAR file_name[ MAX_PATH ]; 
	WCHAR app_path[ MAX_PATH ]; 
	ULONG cc_ret_len; 
	//SYSTEMTIME _time; 

	do 
	{
		ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

        _ret = SetCurrentDirectoryW(app_path);
        if (FALSE == _ret)
        {

        }

		hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
		if( S_OK != hr )
		{
			break; 
		}

		ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		*file_name = _T( '\0' ); 

		{
			wstring url; 
			wstring domain_name; 
			//wstring _file_name; 
			url = g_html_script_config_dlg->m_WebBrowser.get_loading_url();

			ret = get_domain_name_in_url( url.c_str(), domain_name ); 
			if( ret != ERROR_SUCCESS )
			{
			}
			else
			{
				domain_name += L".xml"; 
				StringCchCopyW( file_name, sizeof( file_name ), domain_name.c_str() ); 
			}


			//_file_name = app_path; 
			//_file_name += L"\\"; 
			//_file_name += domain_name; 

			ret = open_file_dlg( TRUE, L"xml", file_name, NULL, app_path, L"*.xml\0*.xml\0\0", NULL );
			if( ret < 0 )
			{
				break; 
			}
		}

		if( *file_name != L'\0' )
		{
			SetDlgItemTextW( IDC_EDIT_CONFIG_FILE_NAME, file_name ); 
		}
	} while ( FALSE ); 
}

//typedef struct _HTML_ELEMENT_ACTIONS_WORK
//{
//	HTML_ELEMENT_ACTION_LIST *actions; 
//	html_element_prop_dlg *dlg; 
//} HTML_ELEMENT_ACTIONS_WORK, *PHTML_ELEMENT_ACTIONS_WORK; 

LRESULT WINAPI find_action_from_action_id( HTML_ELEMENT_ACTION *root, 
										  ULONG id, 
										  HTML_ELEMENT_ACTION **action_found)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *item; 
	HTML_ELEMENT_ACTION *sub_item; 
	HTML_ELEMENT_ACTION *next_item; 
	vector< HTML_ELEMENT_ACTION* > next_items; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != action_found ); 
		
		*action_found = NULL; 
		ASSERT( NULL == root->parent_item ); 
		//for( ; ; )
		//{
		//	if( NULL == root->parent_item )
		//	{
		//		break; 
		//	}

		//	root = root->parent_item; 
		//}

		next_items.push_back( root ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				ret = ERROR_NOT_FOUND; 
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 
			
			if( item->id == id )
			{
				*action_found = item; 
				break; 
			}

			do 
			{
				next_item = item->next_item; 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = item->sub_item; 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 
			} while ( FALSE ); 
		}

		
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( *action_found == NULL ); 
		}
	}while( FALSE );

	return ret; 
}

//LRESULT WINAPI locate_to_sub_instruction( HTML_ELEMENT_ACTION *action, HTML_ELEMENT_ACTION **action_out )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HTML_ELEMENT_ACTION *_action_next; 
//
//	do 
//	{
//		ASSERT( NULL != action ); 
//		ASSERT( NULL != action_out ); 
//
//		*action_out = NULL; 
//		
//		if( action->sub_item == NULL )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		_action_next = action->sub_item; 
//
//		ASSERT( 0 == wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
//
//		if( _action_next->sub_item == NULL )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		_action_next = _action_next->sub_item; 
//
//		ASSERT( 0 == wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
//
//		if( _action_next->next_item == NULL )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		_action_next = _action_next->next_item; 
//		ASSERT( 0 != wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
//		*action_out = _action_next; 
//	}while( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI locate_to_page_action( HTML_ELEMENT_ACTION *action, 
										  HTML_ELEMENT_ACTION **action_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_action_next; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != action_out ); 

		*action_out = NULL; 

		if( action->sub_item == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_action_next = action->sub_item; 

		ASSERT( 0 != wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 

		//if( _action_next->next_item == NULL )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		//_action_next = _action_next->next_item; 
		//ASSERT( 0 != wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
		*action_out = _action_next; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _get_container_content_info( HTML_ELEMENT_ACTION *action, 
										  HTML_ACTION_CONTEXT *context, 
										  HTML_ELEMENT_ACTIONS *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTION *container; 
	HTML_ELEMENT_ACTION *_action_next = NULL; 
	HTML_ELEMENT_ACTION *element; 
	//ULONG output_action_count; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != page_content ); 

		do 
		{
			_ret = check_page_layout_node( action ); 
			if( _ret == ERROR_SUCCESS )
			{
				if( NULL == action->sub_item )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ret = locate_to_page_action( action, 
					&_action_next ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( NULL != _action_next ); 
				break; 
			}

			if( action->parent_item == NULL )
			{
				_action_next = action; 
				break; 
			}

			ret = get_parent_page_layout_node( action, &container ); 
			if( ERROR_SUCCESS != ret )
			{
				ASSERT( NULL != _action_next ); 
				break; 
			}

			ASSERT( container != NULL ); 
			//ASSERT( container->sub_item != NULL ); 

			//if( container->sub_item == NULL )
			//{
			//	ret = ERROR_ERRORS_ENCOUNTERED; 
			//	break; 
			//}

			ret = locate_to_page_action( container, 
				&_action_next ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( NULL != _action_next ); 
		}while( FALSE ); 

		if( ERROR_SUCCESS != ret )
		{
			ASSERT( NULL == _action_next ); 
			break; 
		}

		ASSERT( NULL != _action_next ); 

		do
		{
			element = NULL; 

			element = new HTML_ELEMENT_ACTION(); 
			if( NULL == element )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			element->id = INVALID_JUMP_TO_ID; 
			element->jump_to_id = INVALID_JUMP_TO_ID; 

			element->parent_item = NULL; 
			element->sub_item = NULL; 
			element->next_item = NULL; 

			element->action = HTML_ELEMENT_ACTION_DUMMY_TYPE; 
			element->param = HTML_ELEMENT_ACTION_DUMMY_TYPE_PARAM; 
			element->name = L"url"; 

			element->action = HTML_ELEMENT_ACTION_NONE; 

			if( context != NULL )
			{
				ASSERT( context->current_url.length() > 0 ); 
                element->outputs.clear(); 
                element->outputs.push_back(context->current_url); 
			}

			page_content->push_back( element ); 
			element = NULL; 
		}while( FALSE ); 

		if( element != NULL )
		{
			delete element; 
			element = NULL; 
		}

		if( ERROR_SUCCESS != ret )
		{
			ASSERT( NULL == _action_next ); 
			break; 
		}

		do
		{
			element = new HTML_ELEMENT_ACTION(); 
			if( NULL == element )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			element->name = L"html"; 

			element->action = HTML_ELEMENT_ACTION_DUMMY_TYPE; 
			element->param = HTML_ELEMENT_ACTION_DUMMY_TYPE_PARAM; 
			
			if( context != NULL )
			{
                element->outputs.clear(); 
				element->outputs.push_back( context->current_page ); 
				ASSERT( context->current_url.length() > 0 ); 
			}
			
			//element->xpath.clear(); 

			page_content->push_back( element ); 
			element = NULL; 
		}while( FALSE ); 

		if( element != NULL )
		{
			delete element; 
			element = NULL; 
		}

		if( ERROR_SUCCESS != ret )
		{
			ASSERT( NULL == _action_next ); 
			break; 
		}

		//output_action_count = 0; 
		for( ; ; )
		{
			if( NULL == _action_next )
			{
				break; 
			}

			do 
			{
				if( 0 != wcscmp( _action_next->action.c_str(), 
					HTML_ELEMENT_ACTION_OUTPUT ) 
					|| 0 != wcscmp( _action_next->param.c_str(), 
					HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT ) )
				{
					break; 
				}

				if( _action_next->name.length() == 0 )
				{
					CString _field_name; 
					_field_name.Format( L"field%u", _action_next->id ); 

					_action_next->name = _field_name.GetBuffer(); 
				}

				page_content->push_back( _action_next ); 
			} while ( FALSE ); 

			_action_next = _action_next->next_item; 
		}

		if( 2 >= page_content->size() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_container_content_info( HTML_ELEMENT_ACTIONS *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTIONS::iterator it; 

	do 
	{
		ASSERT( NULL != page_content ); 
		for( it = page_content->begin(); 
			it != page_content->end(); 
			it ++ )
		{
			if( ( *it ) != NULL 
				&& wcscmp( ( *it )->param.c_str(), HTML_ELEMENT_ACTION_DUMMY_TYPE_PARAM ) == 0 
				&& 0 == wcscmp( HTML_ELEMENT_ACTION_DUMMY_TYPE, ( *it )->action.c_str() ) )
			{
				delete ( *it ); 
				*it  = NULL; 
			}
		}

		page_content->clear(); 
	}while( FALSE ); 

	return ret; 
}


LRESULT WINAPI get_container_content_info( HTML_ELEMENT_ACTION *action, 
										  HTML_ACTION_CONTEXT *context, 
										  HTML_ELEMENT_ACTIONS *page_content )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTION *container; 
	HTML_ELEMENT_ACTION *_action_next = NULL; 
	//ULONG output_action_count; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != page_content ); 

		do 
		{
			_ret = check_have_sub_page( action ); 
			if( _ret == ERROR_SUCCESS )
			{
				if( NULL == action->sub_item )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ret = locate_to_page_action( action->sub_item, 
					&_action_next ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( NULL != _action_next ); 
				break; 
			}

			if( action->parent_item == NULL )
			{
				_action_next = action; 
				break; 
			}

			ret = get_parent_page_layout_node( action, &container ); 
			if( ERROR_SUCCESS != ret )
			{
				ASSERT( NULL != _action_next ); 
				break; 
			}

			ASSERT( container != NULL ); 
			ASSERT( container->sub_item != NULL ); 

			if( container->sub_item == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			ret = locate_to_page_action( container->sub_item, 
				&_action_next ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( NULL != _action_next ); 
		}while( FALSE ); 

		if( ERROR_SUCCESS != ret )
		{
			ASSERT( NULL == _action_next ); 
			break; 
		}

		ASSERT( NULL != _action_next ); 

		for( ; ; )
		{
			if( NULL == _action_next )
			{
				break; 
			}

			do 
			{
				page_content->push_back( _action_next ); 
			} while ( FALSE ); 

			_action_next = _action_next->next_item; 

		}
	}while( FALSE );

	return ret; 
}

LRESULT get_all_containers( HTML_ELEMENT_ACTION *root, 
						   vector< HTML_ELEMENT_ACTION*> *containers )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *item; 
	HTML_ELEMENT_ACTION *sub_item; 
	HTML_ELEMENT_ACTION *next_item; 
	HTML_ELEMENT_ACTION *parent_item; 
	vector< HTML_ELEMENT_ACTION* > next_items; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != containers ); 

		containers->push_back( root ); 
		next_items.push_back( root ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 

			if( root != item 
				&& ERROR_SUCCESS == check_page_layout_node( item ) )
			{
				containers->push_back( item ); 
			}

			do 
			{
				next_item = item->next_item; 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = item->sub_item; 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 
				parent_item = item; 
			} while ( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_all_container_infos( HTML_ELEMENT_ACTION *root, 
									   HTML_ACTION_CONTEXT *context, 
									   CONTAINER_INFOS *page_contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ELEMENT_CONFIG_LIST containers; 
	CONTAINER_INFO *container_info; 

	do 
	{
		ret = get_all_containers( root, &containers ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		for( ELEMENT_CONFIG_LIST::iterator it = containers.begin(); 
			it != containers.end(); 
			it ++ )
		{
			do 
			{
				container_info = NULL; 
				_ret = get_html_content_info_ex( ( *it ), &container_info, GET_ROOT_ELEMENT_INFO ); 
				if( ERROR_SUCCESS != _ret )
				{
					break; 
				}

                //通过名称来表示ID
                //if (container_info->name.length() > 0)
                {
                    container_info->name = (*it)->name;
                    container_info->_using = TRUE; 
                }

				_ret = _get_container_content_info( ( *it ), 
					context, 
					&container_info->content ); 

				if( ERROR_SUCCESS != _ret )
				{
					break; 
				}

				ret = init_data_data_store_param( &container_info->store_param ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				page_contents->push_back( container_info ); 
				container_info = NULL; 
			} while ( FALSE ); 

			if( NULL != container_info )
			{
				release_container_info( container_info ); 
			}
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_next_action_from_source_action( HTML_ELEMENT_ACTION *root, 
										   HTML_ELEMENT_ACTION *action, 
										   WEB_BROWSER_PROCESS **web_browser, 
										   HTML_ELEMENT_ACTION **action_out, 
										   ULONG *relation )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTION *_action_next; 
	HTML_ELEMENT_ACTION *container; 
    HTML_ELEMENT_ACTION *source_action; 
	WEB_BROWSER_PROCESS *_web_browser; 
	WEB_BROWSER_PROCESS *sub_web_browser; 
	wstring url; 

	do 
	{
		ASSERT( NULL != root);
		ASSERT( NULL != action ); 
		ASSERT( NULL != web_browser ); 
		ASSERT( NULL != action_out ); 

		*web_browser = NULL; 
		*action_out = NULL; 

		if( NULL != relation )
		{
			*relation = LOCATE_TO_NONE; 
		}

        ret = get_parent_source_action(action, &source_action); 
        //ret = check_have_sub_page(action); 
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

		_ret = get_parent_page_layout_node( action, &container ); 
		if( ERROR_SUCCESS != _ret )
		{
			//_action_next = root->sub_item; 
			//if( _action_next != NULL )
			//{
			//	context->locate_url = container->url; 
			//	dbg_print_w( MSG_IMPORTANT, L"[script]locate to url(%s)\n", context->locate_url.c_str() ); 
			//}

			//*action_out = _action_next; 

			//if( NULL != relation )
			//{
			//	*relation = LOCATE_TO_PARENT_NEXT_SIBLING_ITEM; 
			//}

			ret = ERROR_NEXT_HTML_ACTION_NOT_FOUND; 
			break; 
		}

		ASSERT( container != NULL ); 
		ASSERT( container->sub_item != NULL ); 

		_ret = get_open_page_url_from_source(source_action, url );
		if( _ret != ERROR_SUCCESS )
		{
			ret = get_page_layout_web_browser( root, container, &_web_browser );
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			_action_next = action->next_item; 
            if (_action_next == NULL)
            {
                ret = ERROR_NO_MORE_ITEMS;
                *action_out = NULL; 
            }
            else
            {
                dbg_print_w(MSG_IMPORTANT, L"[script]locate to url(%s)\n", _web_browser->context.locate_url.c_str());
                *action_out = _action_next;
                if (NULL != relation)
                {
                    *relation = LOCATE_TO_PARENT_NEXT_SIBLING_ITEM;
                }
            }

			*web_browser = _web_browser; 

			_ret = get_action_web_browser( root, action, &_web_browser ); 
			if( _ret != ERROR_SUCCESS )
			{
                ret = _ret; 
				break; 
			}

			_web_browser->context.locate_url = L""; //L"closed";  
			break; 
		}

		ASSERT( action->parent_item != NULL ); 

		ret = get_action_web_browser( root, action, &_web_browser ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( container->sub_item == NULL )
		{
            ASSERT(FALSE); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = locate_to_page_action(container,
			&_action_next ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ASSERT( NULL != _action_next ); 

		ret = get_action_web_browser( root, _action_next, &sub_web_browser ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ASSERT( NULL != sub_web_browser ); 

		sub_web_browser->context.locate_url = url; 
		dbg_print_w( MSG_IMPORTANT, L"[script]locate to url(%s)\n", sub_web_browser->context.locate_url.c_str() ); 
		*action_out = _action_next; 

		*web_browser = sub_web_browser; 
		if( NULL != relation )
		{
			*relation = LOCATE_TO_FIRST_SIBLING_ITEM; 
		}

	}while( FALSE );

	return ret; 
}

//only container have itself url configure.
LRESULT WINAPI get_locate_page_url( HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *action, wstring &url )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *container; 
	WEB_BROWSER_PROCESS *process; 
	do 
	{
		ASSERT( NULL != action ); 

		ret = get_parent_page_layout_node( action, &container ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ASSERT( NULL != container ); 

		ret = get_web_browser_process_ex( container, &process, GET_ROOT_ELEMENT_INFO ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		url = process->context.locate_url; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_locate_page_url( HTML_ELEMENT_ACTION *root, 
								 HTML_ELEMENT_ACTION *action, 
								 wstring &url )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *container; 

	do 
	{
		ASSERT( NULL != action ); 

		ret = get_parent_page_layout_node( action, &container ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ASSERT( NULL != container ); 

		//container->url = url; 
	}while( FALSE );

	return ret; 
}

/***********************************************************************
处理当前URL的方法:
1.初始化时，使用初始化的URL
2.如果是遍历动态得到的URL列表，从中抽取URL,遍历完后，还原原始URL.
3.有一些动作需要重配置原始URL(有多种情况，需要分别处理)
***********************************************************************/

typedef enum _WEB_BROWSER_MODE
{
    WEB_BROWSER_OPEN_NEW,
    WEB_BROWSER_CLOSE,
} WEB_BROWSER_MODE, *PWEB_BROWSER_MODE;

typedef struct _NEXT_HTML_ELEMENT_INSTRUCTION_INFO
{

    HTML_ELEMENT_ACTION *next_action;
    WEB_BROWSER_MODE browser_mode;
    wstring url;
}NEXT_HTML_ELEMENT_INSTRUCTION_INFO, *PNEXT_HTML_ELEMENT_INSTRUCTION_INFO;

LRESULT WINAPI get_next_html_element_action( 
											HTML_ELEMENT_ACTION *root, 
											HTML_ELEMENT_ACTION *action, 
											WEB_BROWSER_PROCESS **web_browser, 
											HTML_ELEMENT_ACTION **action_next, 
											ULONG *relation )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTION *_action_next; 
	HTML_ELEMENT_ACTION *_sub_action; 
	WEB_BROWSER_PROCESS *_web_browser; 
	WEB_BROWSER_PROCESS *sub_web_browser; 

	do 
	{
		ASSERT( action != NULL ); 
		ASSERT( action_next != NULL ); 
		ASSERT( web_browser != NULL ); 

		_web_browser = *web_browser; 
		*web_browser = NULL; 
		if( NULL == _web_browser )
		{
			ASSERT( FALSE ); 
			ret = get_action_web_browser( root, action, &_web_browser ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		ASSERT( _web_browser != NULL ); 

		//*action_next = NULL;
		//_web_browser->context.locate_url.clear(); 

		if( NULL != relation )
		{
			*relation = LOCATE_TO_NONE; 
		}

		do 
		{
			_action_next = NULL; 
			_sub_action = NULL; 

			_ret = check_have_sub_page( action ); 
			if( _ret != ERROR_SUCCESS )	
			{
				break; 
			}
			
			_ret = locate_to_sub_page_action( action, &_sub_action ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( _sub_action == NULL ); 
				break; 
			}
			
			ASSERT( _sub_action != NULL ); 

            if (0 == wcscmp(HTML_ELEMENT_ACTION_OUTPUT,
                action->action.c_str())
                && 0 == wcscmp(HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK,
                    action->param.c_str()))
            {
                wstring url;
                _ret = get_open_page_url_from_source(action, url);
                if (_ret != ERROR_SUCCESS)
                {
                    break;
                }

                if (NULL != relation)
                {
                    *relation = LOCATE_TO_FIRST_SUB_ITEM;
                }

                ret = get_action_web_browser(root, _sub_action, &sub_web_browser);
                if (ret != ERROR_SUCCESS)
                {
                    ASSERT(FALSE);
                    break;
                }

                ASSERT(sub_web_browser != _web_browser);

                _action_next = _sub_action;
                sub_web_browser->context.locate_url = url;
                dbg_print_w(MSG_IMPORTANT, L"[script]locate to url(%s)\n",
                    sub_web_browser->context.locate_url.c_str());
                *web_browser = sub_web_browser;
            }
            else
            {
                wstring url;
                //_ret = get_next_xpath_from_output(action->output, url);
                //if (_ret != ERROR_SUCCESS)
                //{
                //    break;
                //}

                //if (NULL != relation)
                //{
                //    *relation = LOCATE_TO_FIRST_SUB_ITEM;
                //}

                //ret = get_action_web_browser(root, _sub_action, &sub_web_browser);
                //if (ret != ERROR_SUCCESS)
                //{
                //    ASSERT(FALSE);
                //    break;
                //}

                //ASSERT(sub_web_browser != _web_browser);

                //_action_next = _sub_action;
                //sub_web_browser->context.locate_url = url;
                //dbg_print_w(MSG_IMPORTANT, L"[script]locate to url(%s)\n",
                //    sub_web_browser->context.locate_url.c_str());
                //*web_browser = sub_web_browser;
                ret = ERROR_NOT_SUPPORTED; 
                break; 
            }
		}while( FALSE ); 

		if( _action_next != NULL )
		{
			ASSERT( *web_browser != NULL ); 
			break; 
		}

		if( NULL != action->next_item )
		{
			HTML_ELEMENT_ACTION *container; 

			ASSERT( action->parent_item != NULL ? 
				action->parent_item->sub_item != action->next_item : 
				TRUE ); 

			_ret = get_parent_page_layout_node( action, &container ); 

			if( _ret != ERROR_SUCCESS )
			{
				dbg_print_w( MSG_IMPORTANT, L"[script]locate to url(%s)\n", _web_browser->context.locate_url.c_str() ); 

				_action_next = action->next_item; 

				if( NULL != relation )
				{
					*relation = LOCATE_TO_NEXT_SIBLING_ITEM; 
				}
				break; 
			}

			//_ret = check_have_sub_page( container ); 
			//if( _ret != ERROR_SUCCESS )	
			//{
			//	ASSERT( FALSE ); 
			//	break; 
			//}

			_ret = locate_to_page_action( container, &_sub_action ); 
			if( _ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				ASSERT( _sub_action == NULL ); 
				break; 
			}

			dbg_print_w( MSG_IMPORTANT, L"[script]locate to url(%s)\n", _web_browser->context.locate_url.c_str() ); 

			_action_next = action->next_item; 

			ret = get_action_web_browser( root, _action_next, &sub_web_browser ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( sub_web_browser != NULL ); 
			
			if( NULL != relation )
			{
				*relation = LOCATE_TO_NEXT_SIBLING_ITEM; 
			}

			break; 
		}

		ret = get_next_action_from_source_action( root, action, web_browser, &_action_next, relation ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	do
	{
		if( _action_next == NULL )
		{
			break; 
		}

		if( 0 == wcscmp( _action_next->action.c_str(), 
			HTML_ELEMENT_ACTION_JUMP ) 
			|| INVALID_JUMP_TO_ID != _action_next->jump_to_id )
		{
			ASSERT( 0 == wcscmp( _action_next->action.c_str(), 
				HTML_ELEMENT_ACTION_JUMP ) 
				&& INVALID_JUMP_TO_ID != _action_next->jump_to_id ) ; 

			ret = find_action_from_action_id( root, _action_next->jump_to_id, &_action_next ); 
			if( ret == ERROR_SUCCESS )
			{
				if( NULL != relation )
				{
					*relation = LOCATE_TO_JUMP_TO_ITEM; 
				}

				ASSERT( NULL != _action_next ); 
				//ASSERT( 0 != _action_next->url.length() ); 

				HTML_ELEMENT_ACTION *container; 
				//can't jump to the action in the different level loop list.
				ret = get_parent_page_layout_node( action, &container ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				ret = get_page_layout_web_browser( root, container, &_web_browser ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( container != root )
				{
					HTML_ELEMENT_ACTION *sub_action; 
					ASSERT( ERROR_SUCCESS == check_page_layout_node( container ) ); 
					ret = locate_to_sub_page_action( container, &sub_action ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}
				}
				else
				{
					//_web_browser->context.locate_url = root->url; 
				}

				dbg_print_w( MSG_IMPORTANT, L"[script]locate to url(%s)\n", _web_browser->context.locate_url.c_str() ); 
				*web_browser = _web_browser; 
				break; 
			}

			//ASSERT( FALSE ); 
		}
	}while( FALSE ); 

	if( NULL != _action_next )
	{
		post_process_action( root, _action_next ); 
	}

	*action_next = _action_next; 
	
	return ret; 
}

INLINE LRESULT WINAPI worker_stopped( SCRIPT_WORKER *worker )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( TRUE != worker->stop_work )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
        else
        {
            ret = ERROR_SUCCESS; 
        }

	}while( FALSE );

	return ret; 
}

#define EXIT_WORK_IF_NEEDED( __worker__)  if( ERROR_SUCCESS == worker_stopped( ( __worker__) ) ) { break; }

LRESULT _get_script_locate_url( wstring &url )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	if( script_instances[ 0 ].location_url.length() > 0 )
		{
			url = script_instances[ 0 ].location_url.c_str(); 
		}
		else if( script_instances[ 0 ].begin_url.length() > 0 )
		{
			url = script_instances[ 0 ].begin_url; 
		}
		else
		{
			ret = g_html_element_prop_dlg->get_html_script_locate_url_by_user( url ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
			break; 
		}
	}while( FALSE );

	return ret; 
}

/********************************************************************
起动多抓取功能实例的方法:
1. 复制一个新的命令树，调用start_thml_script_work
2. 对每个实例分别创建不同的浏览器子进程族.
********************************************************************/

typedef struct _HTML_EMENET_ACTION_COPY_INFO
{
	HTML_ELEMENT_ACTION *source; 
	HTML_ELEMENT_ACTION *dest; 
} HTML_EMENET_ACTION_COPY_INFO, *PHTML_EMENET_ACTION_COPY_INFO; 

LRESULT WINAPI release_actions( vector< HTML_EMENET_ACTION_COPY_INFO*> *actions ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		for( vector< HTML_EMENET_ACTION_COPY_INFO*>::iterator it = actions->begin(); 
			it != actions->end(); 
			it ++ )
		{
			ASSERT( NULL != ( *it ) ); 
			delete ( *it ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI free_element_config_tree( HTML_ELEMENT_ACTION *action ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	vector< HTML_ELEMENT_ACTION*> actions; 
	HTML_ELEMENT_ACTION *_action; 

	do 
	{
		ASSERT( NULL != action ); 

		actions.push_back( action ); 

		for( ; ; )
		{
			do 
			{
				_action = *actions.rbegin(); 
				actions.pop_back(); 
				if( NULL == _action )
				{
					break; 
				}

				if( NULL != _action->next_item )
				{
					actions.push_back( _action->next_item ); 
				}

				if( NULL != _action->sub_item )
				{
					actions.push_back( _action->sub_item ); 
				}

				delete _action; 
			}while( FALSE ); 			
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::copy_actions( HTML_ELEMENT_ACTION *action, HTML_ELEMENT_ACTION **actions_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	vector< HTML_EMENET_ACTION_COPY_INFO*> actions; 
	HTML_EMENET_ACTION_COPY_INFO *action_copy_info; 
	HTML_EMENET_ACTION_COPY_INFO *_action_copy_info; 
	HTML_ELEMENT_ACTION *_actions; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != actions_out); 
		
		action_copy_info = new HTML_EMENET_ACTION_COPY_INFO(); 

		action_copy_info->source = action; 
		ret = allocate_html_element_action( &action_copy_info->dest ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_actions = action_copy_info->dest; 

		actions.push_back( action_copy_info ); 

		for( ; ; )
		{
			if( 0 == actions.size() )
			{
				break; 
			}

			do 
			{
				_action_copy_info = *actions.rbegin(); 
				ASSERT( NULL != _action_copy_info ); 
				actions.pop_back(); 

				ret = copy_html_element_action( _action_copy_info->dest, _action_copy_info->source ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				if( _action_copy_info->source->sub_item )
				{
					action_copy_info = new HTML_EMENET_ACTION_COPY_INFO(); 
					action_copy_info->source = _action_copy_info->source->sub_item; 
					ret = allocate_html_element_action( &action_copy_info->dest ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					action_copy_info->dest->parent_item = _action_copy_info->dest; 
					_action_copy_info->dest->sub_item = action_copy_info->dest; 
					actions.push_back( action_copy_info ); 
				}

				if( _action_copy_info->source->next_item )
				{
					action_copy_info = new HTML_EMENET_ACTION_COPY_INFO(); 
					action_copy_info->source = _action_copy_info->source->next_item; 
					ret = allocate_html_element_action( &action_copy_info->dest ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					action_copy_info->dest->parent_item = _action_copy_info->dest->parent_item; 
					_action_copy_info->dest->next_item = action_copy_info->dest; 
					actions.push_back( action_copy_info ); 
				}
			} while ( FALSE ); 
			if( NULL != _action_copy_info )
			{
				delete _action_copy_info; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			if( _actions != NULL )
			{
				release_actions( &actions ); 
				free_element_config_tree( _actions ); 
			}
			break; 
		}

		*actions_out = _actions; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI start_html_script_work( HTML_SCRIPT_INSTANCE *instance )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	wstring url; 
	ULONG wait_ret; 
	HTML_ELEMENT_ACTION *_action; 
	BOOLEAN output; 

	do 
	{
		ASSERT( instance != NULL ); 

		if( NULL == instance->actions )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( instance->worker.running != FALSE )
		{
			ASSERT( FALSE ); 
		}

		wait_ret = WaitForSingleObject( instance->worker.state_notifier, 10000 ); 
		if( WAIT_OBJECT_0 != wait_ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( instance->worker.running == TRUE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		lock_cs( instance->worker.lock ); 
		instance->worker.action = instance->actions; 
		instance->worker.stop_work = FALSE; 
		unlock_cs( instance->worker.lock ); 

		clean_ha_variables(); 

		SetEvent( instance->worker.work_notifier ); 
	}while( FALSE ); 

	return ret; 
}
; 
LRESULT html_element_prop_dlg::start_html_script_work_ex( HTML_ELEMENT_ACTION *action )
{

	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HTML_ELEMENT_ACTION *_action; 
	//HTML_SCRIPT_INSTANCES::iterator it; 
	wstring url; 
	INT32 i; 

	do 
	{
		ASSERT( NULL != action ); 

		if( 0 < script_instances[ 0 ].location_url.length() )
		{
			url = script_instances[ 0 ].location_url; 
		}
		else
		{
			url = script_instances[ 0 ].begin_url; 
		}

		if( url.length() == 0 )
		{
			ret = g_html_element_prop_dlg->get_html_script_locate_url_by_user( url ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		ret = prepare_data_output_config( action, url.c_str() ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

#if 1
		for( i = 0; i < ARRAYSIZE( script_instances ); i ++ )
		{
			do 
			{
				ret = copy_actions( action, &_action ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( script_instances[ i ].begin_url.length() == 0 
					|| script_instances[ i ].loop_count == 0 )
				{
					break; 
				}

				wstring url; 
				WEB_BROWSER_PROCESS *browser_info; 
				if( script_instances[ i ].location_url.length() == 0 )
				{
					url = script_instances[ i ].location_url; 
				}
				else
				{
					url = script_instances[ i ].begin_url; 
				}
				
				ret = get_action_web_browser( _action, _action, &browser_info ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					break; 
				}

				browser_info->context.locate_url = url; 
				script_instances[ i ].actions = _action; 

				ret = start_html_script_work( &script_instances[ i ] ); 
			}while( FALSE ); 

			script_info.get_data_time = 0; 
			script_info.get_link_time = 0; 

			break; 
		}
#endif //0

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI stop_html_script_work_ex()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	do 
	{

		for( i = 0; 
			i < ARRAYSIZE( script_instances ); 
			i ++ )
		{
			stop_html_script_work( &script_instances[ i ] ); 
		}

	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI stop_html_script_work( HTML_SCRIPT_INSTANCE *instance)
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG wait_ret; 

	do 
	{
		ASSERT( instance->worker.action == NULL ); 
		ASSERT( instance->worker.worker != NULL ); 

		if( instance->worker.stop_work != TRUE )
		{
			instance->worker.stop_work = TRUE; 
		}

		wait_ret = WaitForSingleObject( instance->worker.state_notifier, 3000 ); 
		if( WAIT_OBJECT_0 != wait_ret )
		{
			//_ret = release_all_web_browser_processes(); 
			//if( _ret != ERROR_SUCCESS )
			//{
			//	ret = _ret; 
			//}

			wait_ret = WaitForSingleObject( instance->worker.state_notifier, 1000 ); 
			if( WAIT_OBJECT_0 != wait_ret )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
			break; 
		}

		if( instance->worker.running == TRUE )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ASSERT( instance->worker.stop_work == TRUE ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI exit_html_script_worker_ex()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	do 
	{
		for( i = 0; i < ARRAYSIZE( script_instances ); i ++ )
		{
			exit_html_script_worker( &script_instances[ i ].worker ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI exit_html_script_worker( SCRIPT_WORKER *worker )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	ULONG wait_ret; 

	do 
	{
		ASSERT( worker->action == NULL ); 
		ASSERT( worker->worker != NULL ); 

		if( worker->stop_work != TRUE )
		{
			worker->stop_work = TRUE; 
		}

		if( worker->exit != TRUE )
		{
			worker->exit = TRUE; 
		}
		else
		{
			ASSERT( FALSE ); 
		}

		_ret = SetEvent( worker->work_notifier ); 
		if( FALSE == _ret )
		{

		}

		wait_ret = WaitForSingleObject( worker->state_notifier, 3000 ); 
		if( WAIT_OBJECT_0 != wait_ret )
		{
			//need lock!!!
			disconnect_all_web_browser_processes(); 

			wait_ret = WaitForSingleObject( worker->state_notifier, 3000 ); 
			if( WAIT_OBJECT_0 != wait_ret )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
			}
			//break; 
		}

		if( worker->running == TRUE )
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			//break; 
		}

		ASSERT( worker->stop_work == TRUE ); 

		wait_ret = WaitForSingleObject( worker->worker, 3000 ); 
		if( wait_ret != WAIT_OBJECT_0 )
		{
			TerminateThread( worker->worker, 0xeeeeeeee ); 
			//break; 
		}

		CloseHandle( worker->worker ); 
		CloseHandle( worker->state_notifier ); 
		CloseHandle( worker->work_notifier ); 
		DeleteCriticalSection( &worker->lock ); 

		worker->state_notifier = NULL; 
		worker->work_notifier = NULL; 
		worker->worker = NULL; 
 
		worker->exit = FALSE; 
		worker->running = FALSE; 
		worker->stop_work = TRUE; 
		worker->action = NULL; 
	}while( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::prepare_data_output_config( HTML_ELEMENT_ACTION *root, 
														  LPCWSTR url )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	vector<HTML_ELEMENT_ACTIONS*> page_contents; 
	CONTAINER_INFOS container_infos; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != url ); 

		if( L'\0' == *url )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = release_all_html_conainer_infos(); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ret = get_all_container_infos( root, NULL, &container_infos ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
		
		wstring domain_name; 
		ret = get_domain_name_in_url( url, domain_name ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ret = data_output_config( domain_name.c_str(), &container_infos ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		for( CONTAINER_INFO_ITERATOR it = container_infos.begin(); 
			it != container_infos.end(); 
			it ++ )
		{
			//ret = init_data_store( &( *it )->store_param, 
			//	&( *it )->content, 
			//	&( *it )->store ); 

			//if( ret != ERROR_SUCCESS )
			//{
			//	break; 
			//}

			ret = config_data_format( &( *it )->content, &( *it )->store ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				//break; 
			}

			if( it == container_infos.begin() )
			{
				_ret = data_dlg.set_page_contents( &( *it )->content ); 
				
				if( ERROR_SUCCESS  == _ret )
				{
					( *it )->have_ui = TRUE; 
				}
			}

			page_contents.push_back( &( *it )->content ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI on_action_execution_error( HTML_ELEMENT_ACTION *action, RETURN_INFO *return_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	wstring text; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != return_info ); 


        //return_info->return_code = 0; 
        //return_info->message = L""; 
		_ret = get_html_element_action_text( action, text ); 
		if( _ret != ERROR_SUCCESS )
		{
			//break; 
		}

        //不直接向用户提示错误码：return_info->return_code

		{
#define MAX_MESSAGE_LEN 2048
            WCHAR _text[ MAX_MESSAGE_LEN ]; 
			
            if (return_info->message.size() > 0)
            {
                StringCchPrintfW(_text, ARRAYSIZE(_text), L"指令[%s]执行错误[%s]。\r\n如果是页面加载时间不足, 请延长加载时间.\n如果是系统问题(网线断开，网卡配置等), 请修复。\r\n点击\"是\"继续或点击\"否\"退出.", //(调整参数（如网页打开时间))继续?", 
                    text.c_str(),
                    return_info->message.c_str());
            }
            else
            {
                StringCchPrintfW(_text, ARRAYSIZE(_text), L"指令[%s]执行错误。\r\n如果是页面加载时间不足, 请延长加载时间.\n如果是系统问题(网线断开，网卡配置等), 请修复。\r\n点击\"是\"继续或点击\"否\"退出.", //(调整参数（如网页打开时间))继续?", 
                    text.c_str());
            }

            ret = g_html_element_prop_dlg->SendMessage(WM_ERROR_HANDLE,
                (WPARAM)action,
                (LPARAM)(PVOID)_text);

			//if( IDNO == MessageBox( NULL, _text, NULL, MB_YESNO ) )
			//{
			//	ret = ERROR_ERRORS_ENCOUNTERED; 
			//	break; 
			//}
		}

	} while (FALSE );

	return ret; 
}

INLINE LRESULT WINAPI check_html_script_action( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 

	do 
	{
		ASSERT( NULL != action ); 
		if( 0 != wcscmp( HTML_ELEMENT_ACTION_SCRIPT, action->action.c_str() ) )
		{
			break; 
		}

		ret = ERROR_SUCCESS; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI exec_html_script_action( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 
	string script; 

	do 
	{
		ASSERT( NULL != action ); 
		if( 0 != wcscmp( HTML_ELEMENT_ACTION_SCRIPT, action->action.c_str() ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = unicode_to_utf8_ex( action->param.c_str(), script ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = parse_ha_script( ( LPSTR )script.c_str() ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI exec_local_action( HTML_ELEMENT_ACTION *action, BOOLEAN *if_return ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	VARIABLE_INFO variable; 

	do 
	{
		ASSERT( NULL != action ); 
		*if_return = TRUE; 

		ret = exec_html_script_action( action ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = get_script_variable( "if", &variable ); 
		if( ERROR_SUCCESS != _ret )
		{
			break; 
		}

		if( variable.variable.vt != VARIABLE_INT )
		{
			break; 
		}

		if( variable.variable.intVal == 1 )
		{
			*if_return = TRUE; 
		}
		else
		{
			ASSERT( variable.variable.intVal == 0 ); 
			*if_return = FALSE; 
		}

	}while( FALSE );

	return ret; 
}

#define HILIGHT_CURRENT_LOCATING_URL 0x00000001
LRESULT WINAPI hilight_execuating_action( HTML_ELEMENT_ACTION *action, 
										 WEB_BROWSER_PROCESS *web_browser_info, 
										 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ACTION_OUTPUT_PARAM output_param; 

	do 
	{
		HTML_ELEMENT_ACTION *container; 

		ASSERT( NULL != action ); 

		ret = get_parent_page_layout_node( action, &container ); 
		if( ERROR_SUCCESS != ret )
		{
			if( HILIGHT_CURRENT_LOCATING_URL == flags )
			{
				break; 
			}

			output_param.context = NULL; 
            output_param.container = NULL;;
            output_param.action = action; 
		}
		else
		{
			ASSERT( ERROR_SUCCESS == check_page_layout_node( container ) ); 
			ASSERT( web_browser_info->context.locate_url.length() > 0 ); 
			output_param.context = &web_browser_info->context; 
			output_param.container = container; 
            output_param.action = action; 
		}

		ret = g_html_element_prop_dlg->SendMessage( WM_LOCATE_TO_HTML_ACTION, LOCATE_CURRENT_ACTION, ( LPARAM )( PVOID )&output_param ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "" ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_content_action(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;
 
    do
    {
        ASSERT(NULL != action); 

        if (0 != wcscmp(action->action.c_str(),
            HTML_ELEMENT_ACTION_OUTPUT)
            || 0 != wcscmp(action->param.c_str(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT))
        {
            ret = ERROR_INVALID_PARAMETER; 
            break;
        }

    } while (FALSE);
    return ret;
}

//HTML_ELEMENT_ACTION* WINAPI get_container_last_data_action(HTML_ELEMENT_ACTION *container)
//{
//    LRESULT ret = ERROR_SUCCESS;
//    HTML_ELEMENT_ACTION *_action_next = NULL;
//    HTML_ELEMENT_ACTION *last_data_action = NULL;
//
//    do
//    {
//        ASSERT(NULL != container);
//
//        ret = check_container_element(container);
//        if (ret != ERROR_SUCCESS)
//        {
//            break;
//        }
//
//        _action_next = container->sub_item;
//
//        for (; ; )
//        {
//            if (NULL == _action_next)
//            {
//                break;
//            }
//
//            do
//            {
//
//                if (ERROR_SUCCESS != check_content_action(_action_next))
//                {
//                    break;
//                }
//
//                last_data_action = _action_next;
//            } while (FALSE);
//
//            _action_next = _action_next->next_item;
//        }
//    } while (FALSE);
//    return last_data_action;
//}

LRESULT WINAPI is_tail_action( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI ignore_script_action( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 
	
	do 
	{
		ASSERT( NULL != action ); 

		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, action->action.c_str() ) 
			&& 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, action->param.c_str() ) )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}

		if( 0 == wcscmp( HTML_ELEMENT_ACTION_CLICK, action->action.c_str() ) 
			&& 0 != wcscmp( HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE, action->param.c_str() ) )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

ULONG WINAPI exec_html_action_script_thread(PVOID context)
{
    LRESULT ret = ERROR_SUCCESS;
    LRESULT _ret;
    LRESULT io_ret;
    SCRIPT_WORKER *worker;
    HTML_ELEMENT_ACTION *root;
    HTML_ELEMENT_ACTION *action;
    HTML_ELEMENT_ACTION *action_next;
    HTML_ELEMENT_ACTION *_action;
    //HTML_ELEMENT_ACTION *root_last_action;

    HTML_ACTION_OUTPUT_PARAM output_param;
    //typedef vector< WEB_BROWSER_PROCESS* > HTML_SCRIPT_WORK_CONTEXT; 
    bool __ret;
    HRESULT hr;

    string ansi_text;

    PVOID data_out = NULL;
    ULONG data_out_size;

    PVOID data_in = NULL;
    ULONG data_in_size;

    ULONG wait_ret;
    ULONG locate_mode;

    ULONG retry_count;
    ULONG begin_time;
    ULONG end_time;

    WEB_BROWSER_PROCESS *web_browser_info;
    WEB_BROWSER_PROCESS *next_web_browser_info;

    ULONG _page_load_time = INVALID_TIME_VALUE;
    ULONG _process_delay_time = INVALID_TIME_VALUE;

    //BOOLEAN action_on_new_page; 

    do
    {
        ASSERT(NULL != context);

        begin_time = 0;
        end_time = 0;

        worker = (SCRIPT_WORKER*)context;

        for (; ; )
        {
            //release_all_web_browser_processes(); 

            worker->running = FALSE;
            SetEvent(worker->state_notifier);

            wait_ret = WaitForSingleObject(worker->work_notifier, INFINITE);
            if (wait_ret != WAIT_OBJECT_0
                && wait_ret != WAIT_ABANDONED_0)
            {
                ASSERT(FALSE);
                dbg_print(MSG_FATAL_ERROR, "");
                break;
            }

            if (TRUE == worker->exit)
            {
                break;
            }

            do
            {
                EnterCriticalSection(&worker->lock);
                root = worker->action;
                worker->action = NULL;

                if (NULL != root)
                {
                    _ret = ResetEvent(worker->state_notifier);
                    if (FALSE == _ret)
                    {
                        ASSERT(FALSE);
                    }

                    worker->running = TRUE;
                }

                LeaveCriticalSection(&worker->lock);

                if (NULL == root)
                {
                    break;
                }

                EXIT_WORK_IF_NEEDED(worker);

                if (NULL == root->sub_item)
                {
                    break;
                }

                action = root->sub_item;

                //NEXT_HTML_ELEMENT_INSTRUCTION_INFO next_action_info; 
                //ret = get_next_html_element_action(root, root, &action); 
                //if (ret != ERROR_SUCCESS)
                //{
                //    break; 
                //}

                //root_last_action = get_container_last_data_action( root ); 

                ret = get_action_web_browser(root, action, &web_browser_info);
                if (ret != ERROR_SUCCESS)
                {
                    ASSERT(FALSE);
                    break;
                }

                ASSERT(NULL != web_browser_info);
                web_browser_info->context.current_url.clear();

                do
                {
                    wstring url;

                    if (0 != wcscmp(HTML_ELEMENT_ACTION_LOCATE, action->action.c_str()))
                    {
                        //web_browser_info->context.locate_url = action->url.c_str(); 
                        break;
                    }

                    url = action->param;
                    if (url.length() == 0)
                    {
                        ret = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    ret = parse_script_text(url.c_str(), url);
                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }

                    web_browser_info->context.locate_url = url;
                } while (FALSE);

                //action_context.root = root;

                //ret = create_name_pipe( ipc_info.point.pipe_name, &ipc_info.point.pipe ); 
                //if( ret != ERROR_SUCCESS )
                //{
                //	break; 
                //}

                //EXIT_WORK_IF_NEEDED( worker ); 
                retry_count = 0;

                for (; ; )
                {
                    BOOLEAN if_return;

                    if (ERROR_SUCCESS != check_local_action(action))
                    {
                        break;
                    }

                    ASSERT(ERROR_SUCCESS == ret);

                    _ret = exec_local_action(action, &if_return);
                    if (ERROR_SUCCESS != _ret)
                    {

                    }

                    //先取下一条指令，不能进行跳转，如果IF是FALSE再取一下条指令，不能跳转
                    next_web_browser_info = web_browser_info;
                    ret = get_next_html_element_action(root,
                        action,
                        &next_web_browser_info,
                        &action_next,
                        &locate_mode);

                    action = action_next;
                    if (action == NULL)
                    {
                        break;
                    }

                    ret = get_action_web_browser(root, action, &web_browser_info);
                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }

                    if (FALSE == if_return)
                    {
                        ret = get_next_html_element_action(root,
                            action,
                            &next_web_browser_info,
                            &action_next,
                            &locate_mode);

                        action = action_next;

                        if (action == NULL)
                        {
                            break;
                        }

                        ret = get_action_web_browser(root, action, &web_browser_info);
                        if (ret != ERROR_SUCCESS)
                        {
                            break;
                        }
                    }
                }

                ret = _html_element_action_in_new_page(&web_browser_info->context);
                if (ret != ERROR_SUCCESS)
                {
                    dbg_print(MSG_FATAL_ERROR, "don't need execute actions that don't open browser\n");
                    ASSERT(FALSE);
                    break;
                }

                EXIT_WORK_IF_NEEDED(worker);

                for (; ; )
                {
                    BOOLEAN need_close_pipe = FALSE;
                    io_ret = ERROR_SUCCESS;

                    EXIT_WORK_IF_NEEDED(worker);

                    if (NULL == action)
                    {
                        break;
                    }

                    begin_time = GetTickCount();
                    end_time;

                    do
                    {
                        Sleep(NEXT_HTML_ACTION_DELAY);

                        {
                            static HTML_ELEMENT_ACTION *prev_action = NULL;

                            if (prev_action != action)
                            {
                                _ret = hilight_execuating_action(action, web_browser_info, 0);
                            }
                            else
                            {
                                _ret = hilight_execuating_action(action, web_browser_info, HILIGHT_CURRENT_LOCATING_URL);
                            }
                            prev_action = action;
                        }

                        EXIT_WORK_IF_NEEDED(worker);

                        _ret = _html_element_action_in_new_page(&web_browser_info->context);

                        if (ERROR_SUCCESS == _ret)
                        {
                            //action_on_new_page = TRUE; 

                            _ret = g_html_script_config_dlg->SendMessage(WM_LOCATED_TO_URL,
                                0,
                                (LPARAM)(PVOID)web_browser_info->context.locate_url.c_str());

                            if (_ret != ERROR_SUCCESS)
                            {
                                dbg_print(MSG_FATAL_ERROR, "");
                            }

                            ret = start_web_browser(root, action, web_browser_info);
                            if (ret != ERROR_SUCCESS)
                            {
                                action = NULL;
                                need_close_pipe = TRUE;
                                break;
                            }

                            if (_page_load_time != INVALID_TIME_VALUE)
                            {
                                _set_html_action_global_config(INVALID_TIME_VALUE,
                                    _page_load_time,
                                    INVALID_TIME_VALUE);

                                _page_load_time = INVALID_TIME_VALUE;
                            }
                        }
                        else
                        {
                            //action_on_new_page = FALSE; 
                        }

                        /*********************************************************
                        关闭PIPE只有一种情况，就是当前URL与定位URL不一致
                        对于出错了情况，也要精细的判断出URL是否匹配
                        *********************************************************/

                        if (web_browser_info->ipc_info.pipe_inited == FALSE)
                        {
                            ASSERT(FALSE);
                            action = NULL;
                            need_close_pipe = TRUE;
                            break;
                        }

                        do
                        {
                            html_element_action_response response;
                            string action_id;

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = read_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)&data_in_size, sizeof(data_in_size));
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            if (data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN)
                            {
                                ret = ERROR_INVALID_DATA;
                                break;
                            }

                            ASSERT(NULL == data_in);

                            EXIT_WORK_IF_NEEDED(worker);

                            data_in = (CHAR*)malloc(data_in_size);
                            if (data_in == NULL)
                            {
                                ret = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = read_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)data_in, data_in_size);
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            ret = get_html_element_action(action,
                                &web_browser_info->context,
                                data_in,
                                data_in_size,
                                &data_out,
                                &data_out_size);

                            if (ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            ASSERT(data_out != NULL);
                            ASSERT(data_out_size < MAX_PIPE_DATA_LEN);

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = write_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)&data_out_size, sizeof(ULONG));
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = write_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)data_out, data_out_size);
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            free(data_in);
                            data_in = NULL;

                            free(data_out);
                            data_out = NULL;

                            io_ret = read_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)&data_in_size, sizeof(data_in_size));
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            if (data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN)
                            {
                                ret = ERROR_INVALID_DATA;
                                break;
                            }

                            ASSERT(NULL == data_in);
                            data_in = (CHAR*)malloc(data_in_size);
                            if (data_in == NULL)
                            {
                                ret = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = read_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)data_in, data_in_size);
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            end_time = GetTickCount();

                            do
                            {
                                RETURN_INFO return_info;

                                /******************************************
                                检测网页重新加载的方法:
                                1.检查WEBBROWSER的COMPLETE事件
                                2.检查网页内容
                                3.检查URL(不完善)
                                ******************************************/
                                ret = analyze_html_element_action_response(root,
                                    action,
                                    &web_browser_info->context,
                                    data_in,
                                    data_in_size,
                                    &return_info);

                                //free( data_in ); 
                                //data_in = NULL; 

                                if (ret != ERROR_SUCCESS)
                                {
                                    free(data_in);
                                    data_in = NULL;

                                    /*
                                    1.如果是浏览器进程出现无法断续工作的异常，要新建进程
                                    2.如果是打开网页不完整，调整时延，重新发送命令
                                    3.如果是其它错误，判断问题的严重性，调整处理
                                    */
                                    dbg_print(MSG_FATAL_ERROR, "get command response from client error %x\n", ret);
                                    //ERROR_WEB_PAGE_IS_NOT_LOADED == ret
                                    //if( retry_count >= MAX_RETRY_HTML_ACTION_COUNT )
                                    {
                                        retry_count++;
                                        //if( ret == ERROR_EXCEPTION_IN_SCRIPT_HANDLER )
                                        {
                                            if (0 == wcscmp(web_browser_info->context.current_url.c_str(),
                                                web_browser_info->context.locate_url.c_str()))
                                            {
                                                dbg_print_w(MSG_FATAL_ERROR, L"execute script error %x, reload web page %s\n",
                                                    ret,
                                                    web_browser_info->context.locate_url.c_str());

                                                web_browser_info->context.current_url.clear();
                                            }
                                        }

                                        _ret = on_action_execution_error(action, &return_info);

                                        if (ERROR_SUCCESS == _ret)
                                        {
                                            _ret = get_html_action_global_config(NULL, &_page_load_time, &_process_delay_time);

#define WEB_PAGE_PROCESS_DELAY_INCREMENT_TIME 2000
#define WEB_PAGE_LOAD_INCREMENT_TIME 5000
#define MAX_WEB_PAGE_PROCESS_DELAY_WORK_TIME 15000 

                                            if (_process_delay_time < MAX_PROCESS_DELAY_TIME)
                                            {
                                                _set_html_action_global_config(INVALID_TIME_VALUE,
                                                    MAX_WEB_PAGE_LOAD_TIME,
                                                    _process_delay_time + WEB_PAGE_PROCESS_DELAY_INCREMENT_TIME);
                                            }
                                            else
                                            {
                                                _set_html_action_global_config(INVALID_TIME_VALUE,
                                                    MAX_WEB_PAGE_LOAD_TIME,
                                                    _process_delay_time);
                                            }

                                            if (_page_load_time < MAX_WEB_PAGE_PROCESS_DELAY_WORK_TIME)
                                            {
                                                _page_load_time += WEB_PAGE_LOAD_INCREMENT_TIME;
                                            }
                                        }
                                        else
                                        {
                                            if (ERROR_SUCCESS != ignore_script_action(action))
                                            {
                                                ret = ERROR_FAIL_SHUTDOWN;
                                                action = NULL;
                                                break;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    free(data_in);
                                    data_in = NULL;
                                }

                                EXIT_WORK_IF_NEEDED(worker);

                                convert_int_to_string(action->id, action_id);
                                response.set_id(action_id);

                                next_web_browser_info = web_browser_info;
                                ret = get_next_html_element_action(root,
                                    action,
                                    &next_web_browser_info,
                                    &action_next,
                                    &locate_mode);

                                for (; ; )
                                {
                                    BOOLEAN if_return;

                                    if (NULL == action_next)
                                    {
                                        break;
                                    }

                                    if (ERROR_SUCCESS != check_local_action(action_next))
                                    {
                                        break;
                                    }

                                    ASSERT(ERROR_SUCCESS == ret);
                                    _action = action_next;
                                    _ret = exec_local_action(_action, &if_return);
                                    if (ERROR_SUCCESS != _ret)
                                    {

                                    }

                                    if (next_web_browser_info == NULL)
                                    {
                                        //ASSERT( FALSE ); 
                                        ret = get_action_web_browser(root, action, &next_web_browser_info);
                                        if (ret != ERROR_SUCCESS)
                                        {
                                            break;
                                        }
                                    }

                                    ret = get_next_html_element_action(root,
                                        _action,
                                        &next_web_browser_info,
                                        &action_next,
                                        &locate_mode);

                                    if (action_next == NULL)
                                    {
                                        break;
                                    }

                                    if (FALSE == if_return)
                                    {
                                        _action = action_next;

                                        if (next_web_browser_info == NULL)
                                        {
                                            //ASSERT( FALSE ); 
                                            ret = get_action_web_browser(root, action, &next_web_browser_info);
                                            if (ret != ERROR_SUCCESS)
                                            {
                                                break;
                                            }
                                        }

                                        ret = get_next_html_element_action(root,
                                            _action,
                                            &next_web_browser_info,
                                            &action_next,
                                            &locate_mode);
                                    }
                                }

                                //if( 0 == wcscmp( HTML_ELEMENT_ACTION_CLICK, 
                                //	action_next->action.c_str() ) )
                                //{
                                //	DebugBreak(); 
                                //}

                                if (0 == wcscmp(HTML_ELEMENT_ACTION_OUTPUT,
                                    action->action.c_str())
                                    && 0 == wcscmp(HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK,
                                        action->param.c_str()))
                                {
                                    do
                                    {

                                        ASSERT(ERROR_SUCCESS == check_have_sub_page(action));

                                        ASSERT(ret == ERROR_SUCCESS ? web_browser_info->context.current_url.length() > 0 : TRUE);
                                        output_param.context = &web_browser_info->context;
                                        output_param.action = action;

                                        get_parent_page_layout_node(action, &output_param.container);

                                        _ret = g_html_element_prop_dlg->SendMessage(WM_DATA_OUTPUT,
                                            OUTPUT_LINKS,
                                            (LPARAM)(PVOID)&output_param);

                                        if (_ret != ERROR_SUCCESS)
                                        {
                                            dbg_print(MSG_FATAL_ERROR, "");
                                        }
                                    } while (FALSE);

                                    do
                                    {
                                        HTML_ELEMENT_ACTION *sub_action;

                                        ASSERT(ERROR_SUCCESS == check_have_sub_page(action));
                                        _ret = locate_to_sub_page_action(action, &sub_action);
                                        if (_ret != ERROR_SUCCESS)
                                        {
                                            ASSERT(FALSE);
                                        }

                                        if (action != root)
                                        {
                                            break;
                                        }

                                        if (web_browser_info->context.current_url.length() > 0)
                                        {
                                            output_param.context = &web_browser_info->context;
                                            output_param.container = NULL;
                                            output_param.action = sub_action;

                                            g_html_element_prop_dlg->SendMessage(WM_UPDATE_HTML_ACTION,
                                                UPDATE_ROOT_HTML_ACTION,
                                                (LPARAM)(PVOID)&output_param);
                                        }
                                        else
                                        {
                                            ASSERT(FALSE);
                                        }
                                    } while (FALSE);

                                    ASSERT(locate_mode != LOCATE_TO_FIRST_SIBLING_ITEM);
                                }

                                if (locate_mode == LOCATE_TO_FIRST_SIBLING_ITEM)
                                {
                                    HTML_ELEMENT_ACTION *container;
                                    do
                                    {
                                        _ret = get_parent_page_layout_node(action, &container);
                                        if (ERROR_SUCCESS != _ret)
                                        {
                                            break;
                                        }

                                        ASSERT(web_browser_info->context.current_url.length() > 0);
                                        output_param.context = &web_browser_info->context;
                                        output_param.container = container;
                                        output_param.action = action;

                                        ASSERT(ERROR_SUCCESS == check_page_layout_node(container));
                                        _ret = g_html_element_prop_dlg->SendMessage(WM_DATA_OUTPUT, OUTPUT_TEXTS, (LPARAM)(PVOID)&output_param);
                                        if (_ret != ERROR_SUCCESS)
                                        {
                                            dbg_print(MSG_FATAL_ERROR, "");
                                        }
                                    } while (FALSE);
                                }
                                else if (NULL == action_next)
                                {
                                    HTML_ELEMENT_ACTION *container;

                                    //root = action->parent_item; 
                                    _ret = get_parent_page_layout_node(action, &container);
                                    if (ERROR_SUCCESS != _ret)
                                    {
                                        break;
                                    }

                                    ASSERT(web_browser_info->context.current_url.length() > 0);
                                    output_param.context = &web_browser_info->context;
                                    output_param.container = container;
                                    output_param.action = action;

                                    _ret = g_html_element_prop_dlg->SendMessage(WM_DATA_OUTPUT, OUTPUT_TEXTS, (LPARAM)(PVOID)&output_param);
                                    if (_ret != ERROR_SUCCESS)
                                    {
                                        dbg_print(MSG_FATAL_ERROR, "");
                                    }
                                }

                                if (ret != ERROR_SUCCESS)
                                {
                                    ASSERT(action_next == NULL);
                                    break;
                                }
                                else
                                {
                                    if (action_next == NULL)
                                    {
                                        ASSERT(ERROR_SUCCESS == is_tail_action(action));
                                        ret = ERROR_ERRORS_ENCOUNTERED;
                                        break;
                                    }
                                }

                                {
                                    WEB_BROWSER_PROCESS *_next_web_browser;
                                    ret = get_action_web_browser(root, action_next, &_next_web_browser);
                                    if (ret != ERROR_SUCCESS)
                                    {
                                        ASSERT(FALSE);
                                        break;
                                    }

                                    ASSERT(next_web_browser_info == NULL
                                        || _next_web_browser == next_web_browser_info);
                                }

                                action = action_next;
                                retry_count = 0;

                                free(data_in);
                                data_in = NULL;

                                EXIT_WORK_IF_NEEDED(worker);

                            } while (FALSE);

                            //if( io_ret != ERROR_SUCCESS )
                            //{
                            //	//need_close_pipe = TRUE; 
                            //	break; 
                            //}

                            //if( ret != ERROR_SUCCESS )
                            //{
                            //	break; 
                            //}

                            if (NULL != action)
                            {
                                _ret = _html_element_action_in_new_page(&web_browser_info->context);
                                if (_ret == ERROR_SUCCESS)
                                {
                                    response.add_response(HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A);
                                    need_close_pipe = TRUE;
                                }
                                else
                                {
                                    response.add_response(HTML_ELEMENT_ACTION_RESPONSE_CONTINUE_A);
                                }
                            }
                            else
                            {
                                response.add_response(HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A);
                                need_close_pipe = TRUE;
                            }

                            data_out_size = response.ByteSize();
                            data_out = malloc(data_out_size);
                            if (NULL == data_out)
                            {
                                ret = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }

                            if (false == response.IsInitialized())
                            {
                                ret = ERROR_ERRORS_ENCOUNTERED;
                                break;
                            }

                            __ret = response.SerializeToArray(data_out, data_out_size);
                            if (false == __ret)
                            {
                                ret = ERROR_ERRORS_ENCOUNTERED;
                                break;
                            }

                            io_ret = write_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)&data_out_size, sizeof(ULONG));
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }

                            EXIT_WORK_IF_NEEDED(worker);

                            io_ret = write_pipe_sync(&web_browser_info->ipc_info.point, (CHAR*)data_out, data_out_size);
                            if (io_ret != ERROR_SUCCESS)
                            {
                                break;
                            }
                        } while (FALSE);

                        if (io_ret != ERROR_SUCCESS)
                        {
                            need_close_pipe = TRUE;
                            ret = io_ret;
                            break;
                        }

                        if (NULL != data_in)
                        {
                            free(data_in);
                            data_in = NULL;
                        }

                        if (NULL != data_out)
                        {
                            free(data_out);
                            data_out = NULL;
                        }
                    } while (FALSE);

                    if (need_close_pipe == TRUE)
                    {
                        _ret = _html_element_action_in_new_page(&web_browser_info->context);
                        if (_ret != ERROR_SUCCESS)
                        {
                            DEBUG_BREAK();
                        }

                        _release_web_browser_process(web_browser_info);
                    }

                    if (io_ret != ERROR_SUCCESS
                        || ret != ERROR_SUCCESS)
                    {
                        if (ret == ERROR_NEXT_HTML_ACTION_NOT_FOUND
                            || retry_count > MAX_RETRY_HTML_ACTION_COUNT)
                        {
                            action = NULL;
                        }
                        else
                        {
                            retry_count++;
                        }
                    }

                    if (action != NULL)
                    {
                        ret = get_action_web_browser(root, action, &web_browser_info);
                        if (ret != ERROR_SUCCESS)
                        {
                            ASSERT(FALSE);
                            break;
                        }
                    }
                    else
                    {
                        web_browser_info = NULL;
                    }
                }
            } while (FALSE);

            if (end_time == 0)
            {
                end_time = GetTickCount();
            }

            dbg_print(MSG_IMPORTANT, "command ( 1st half process ) time %u\n", end_time - begin_time);
            dbg_print(MSG_IMPORTANT, "command ( 2nd half process ) time %u\n", GetTickCount() - end_time);

            //worker->running = FALSE; 
            //SetEvent( worker->state_notifier ); 
            _ret = disconnect_all_web_browser_processes(); 
            if (_ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_ERROR, "%s %u disconnect all browser process error\n", __FUNCTION__, __LINE__); ;
            }

            _ret = g_html_element_prop_dlg->SendMessage(WM_HTML_WORK_STOPPED, 0, 0);
            if (_ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "");
            }
        }
    } while (FALSE);

    _ret = release_all_web_browser_processes();
    if (_ret != ERROR_SUCCESS)
    {

    }

    if (NULL != data_in)
    {
        free(data_in);
    }

    return (ULONG)ret;
}

#define _EXIT_WORK_IF_NEEDED( __flag__ ) 

ULONG CALLBACK execute_html_commmand_thread( PVOID param )
{
	HTML_ACTION_EXECUATE_PARAM *script_param; 
	
	script_param = ( HTML_ACTION_EXECUATE_PARAM* )param; 

	return ( ULONG )execute_html_commmand( script_param->action, script_param->locate_url.c_str() ); 
}

LRESULT WINAPI execute_html_commmand( HTML_ELEMENT_ACTION *action, LPCWSTR locate_url )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	LRESULT io_ret; 
	//ELEMENT_CONFIG *action_next; 
	HTML_ACTION_OUTPUT_PARAM output_param; 
	bool __ret; 
	HRESULT hr; 

	string ansi_text; 

	PVOID data_out = NULL; 
	ULONG data_out_size; 

	PVOID data_in = NULL; 
	ULONG data_in_size; 

	ULONG wait_ret; 
	ULONG locate_mode; 

	ULONG retry_count; 
	ULONG begin_time; 
	ULONG end_time; 

	WEB_BROWSER_PROCESS *web_browser_info; 
	//WEB_BROWSER_PROCESS *next_web_browser_info; 
	
	ASSERT( action != NULL ); 
	ASSERT( NULL != locate_url); 
	do 
	{
		retry_count = 0; 
		ret = _get_web_browser_process( action, &web_browser_info ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
		
		web_browser_info->context.locate_url = locate_url; 
		_ret = _html_element_action_in_new_page( &web_browser_info->context ); 
		if( ERROR_SUCCESS == _ret )
		{
			ret = _start_web_browser( NULL, action, web_browser_info ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		/*********************************************************
		关闭PIPE只有一种情况，就是当前URL与定位URL不一致
		对于出错了情况，也要精细的判断出URL是否匹配
		*********************************************************/

		if( web_browser_info->ipc_info.pipe_inited == FALSE )
		{
			ASSERT( FALSE ); 
			break; 
		}

		do 
		{
			html_element_action_response response; 
			string action_id; 

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = read_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )&data_in_size, sizeof( data_in_size ) ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			if( data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN )
			{
				ret = ERROR_INVALID_DATA; 
				break; 
			}

			ASSERT( NULL == data_in ); 

			_EXIT_WORK_IF_NEEDED( worker ); 

			data_in = ( CHAR* )malloc( data_in_size ); 
			if( data_in == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = read_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )data_in, data_in_size ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			ret = get_html_element_action( action, 
				&web_browser_info->context, 
				data_in, 
				data_in_size, 
				&data_out, 
				&data_out_size ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ASSERT( data_out != NULL ); 
			ASSERT( data_out_size < MAX_PIPE_DATA_LEN ); 

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = write_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )&data_out_size, sizeof( ULONG ) ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = write_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )data_out, data_out_size ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			free( data_in ); 
			data_in = NULL; 

			free( data_out ); 
			data_out = NULL; 

			io_ret = read_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )&data_in_size, sizeof( data_in_size ) ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( data_in_size == 0 || data_in_size > MAX_PIPE_DATA_LEN )
			{
				ret = ERROR_INVALID_DATA; 
				break; 
			}

			ASSERT( NULL == data_in ); 
			data_in = ( CHAR* )malloc( data_in_size ); 
			if( data_in == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = read_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )data_in, data_in_size ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			end_time = GetTickCount(); 

			do 
			{
				RETURN_INFO return_info; 

				ret = _analyze_html_element_action_response( action, 
					&web_browser_info->context, 
					data_in, 
					data_in_size, 
					&return_info ); 

				free( data_in ); 
				data_in = NULL; 

				convert_int_to_string( action->id, action_id ); 
				response.set_id( action_id ); 

				if( ret != ERROR_SUCCESS )
				{
					dbg_print( MSG_FATAL_ERROR, "get command response from client error %x\n", ret ); 
					if( retry_count > MAX_RETRY_HTML_ACTION_COUNT )
					{
						_ret = on_action_execution_error( action, &return_info ); 
						if( _ret != ERROR_SUCCESS )
						{

						}

						action = NULL; 
					}
					else
					{
						retry_count ++; 
						if( retry_count > 1 )
						{
							if( 0 == wcscmp( web_browser_info->context.current_url.c_str(), 
								web_browser_info->context.locate_url.c_str() ) )
							{
								dbg_print_w( MSG_FATAL_ERROR, L"execute script error %x, reload web page %s\n", 
									ret, 
									web_browser_info->context.locate_url.c_str() ); 

								web_browser_info->context.current_url.clear(); 
							}
						}
					}
					break; 
				}

				_EXIT_WORK_IF_NEEDED( worker ); 

#if 0
				if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, 
					action->action.c_str() ) 
					&& 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK, 
					action->param.c_str() ) )
				{
					do 
					{

						ASSERT( ERROR_SUCCESS == check_action_container( action ) ); 

						ASSERT( ret == ERROR_SUCCESS ? web_browser_info->context.current_url.length() > 0 : TRUE ); 
						output_param.context = &web_browser_info->context; 
						output_param.container = action; 

						_ret = g_html_element_prop_dlg->SendMessage( WM_DATA_OUTPUT, 
							OUTPUT_LINKS, 
							( LPARAM )( PVOID )&output_param ); 

						if( _ret != ERROR_SUCCESS )
						{
							dbg_print( MSG_FATAL_ERROR, "" ); 
						}
					}while( FALSE ); 

					do
					{
						ELEMENT_CONFIG *container; 
						ret = get_container_parent( root, action, &container, INFINITE_SEARCH_LEVEL_COUNT, NULL ); 
						if( ret != ERROR_SUCCESS )
						{
							ASSERT( FALSE ); 
						}

						if( action != root )
						{
							break; 
						}

						if( web_browser_info->context.current_url.length() > 0 )
						{
							output_param.context = &web_browser_info->context; 
							output_param.container = container; 

							g_html_element_prop_dlg->SendMessage( WM_UPDATE_HTML_ACTION, 
								UPDATE_ROOT_HTML_ACTION, 
								( LPARAM )( PVOID )&output_param ); 
						}
						else
						{
							ASSERT( FALSE ); 
						}
					}while( FALSE ); 

					ASSERT( locate_mode != LOCATE_TO_FIRST_SIBLING_ITEM ); 
				}

				if( locate_mode == LOCATE_TO_FIRST_SIBLING_ITEM )
				{
					ELEMENT_CONFIG *container; 
					do 
					{
						_ret = get_container_parent( root, action, &container, INFINITE_SEARCH_LEVEL_COUNT, NULL ); 
						if( ERROR_SUCCESS != _ret )
						{
							break; 
						}

						ASSERT( web_browser_info->context.current_url.length() > 0 ); 
						output_param.context = &web_browser_info->context; 
						output_param.container = container; 

						ASSERT( ERROR_SUCCESS == check_action_container( container ) ); 
						_ret = g_html_element_prop_dlg->SendMessage( WM_DATA_OUTPUT, OUTPUT_TEXTS, ( LPARAM )( PVOID )&output_param ); 
						if( _ret != ERROR_SUCCESS )
						{
							dbg_print( MSG_FATAL_ERROR, "" ); 
						}
					}while( FALSE );
				}
				
				if( NULL == action_next )
				{
					ASSERT( web_browser_info->context.current_url.length() > 0 ); 
					output_param.context = &web_browser_info->context; 
					output_param.container = root; 

					_ret = g_html_element_prop_dlg->SendMessage( WM_DATA_OUTPUT, OUTPUT_TEXTS, ( LPARAM )( PVOID )&output_param ); 
					if( _ret != ERROR_SUCCESS )
					{
						dbg_print( MSG_FATAL_ERROR, "" ); 
					}
				}

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( action_next == NULL ); 
					break; 
				}
				else
				{
					if( action_next == NULL )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}
				}

				{
					WEB_BROWSER_PROCESS *_next_web_browser; 
					ret = get_action_web_browser( root, action_next, &_next_web_browser ); 
					if( ret != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						break; 
					}

					ASSERT( next_web_browser_info == NULL 
						|| _next_web_browser == next_web_browser_info ); 
				}
#endif //0
				retry_count = 0; 

				free( data_in ); 
				data_in = NULL; 

				_EXIT_WORK_IF_NEEDED( worker ); 

			}while( FALSE ); 

			//if( NULL != action )
			//{
			//	_ret = _html_element_action_in_new_page( &web_browser_info->context ); 
			//	if( _ret == ERROR_SUCCESS )
			//	{
			//		response.add_response( HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A ); 	
			//	}
			//	else
			//	{
			//		response.add_response( HTML_ELEMENT_ACTION_RESPONSE_CONTINUE_A ); 
			//	}
			//}
			//else
			{
				response.add_response( HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A ); 
			}

			data_out_size = response.ByteSize(); 
			data_out = malloc( data_out_size ); 
			if( NULL == data_out )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			__ret = response.SerializeToArray(data_out, data_out_size ); 
			if( false == __ret )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			io_ret = write_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )&data_out_size, sizeof( ULONG ) ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}

			_EXIT_WORK_IF_NEEDED( worker ); 

			io_ret = write_pipe_sync( &web_browser_info->ipc_info.point, ( CHAR* )data_out, data_out_size ); 
			if( io_ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE ); 

		if( io_ret != ERROR_SUCCESS )
		{
			ret = io_ret; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			if( ret == ERROR_NEXT_HTML_ACTION_NOT_FOUND 
				|| retry_count > MAX_RETRY_HTML_ACTION_COUNT )
			{
				action = NULL; 
			}
			else
			{
				retry_count ++; 
			}
		}

		if( NULL != data_in )
		{
			free( data_in ); 
			data_in = NULL; 
		}

		if( NULL != data_out )
		{
			free( data_out ); 
			data_out = NULL; 
		}
	}while( FALSE ); 

	if( web_browser_info != NULL )
	{
		//_ret = _html_element_action_in_new_page( &web_browser_info->context ); 
		//if( _ret != ERROR_SUCCESS )
		//{
		//	DEBUG_BREAK(); 
		//}
		release_web_browser_process( web_browser_info ); 
	}

	//if( io_ret != ERROR_SUCCESS
	//	|| ret != ERROR_SUCCESS )
	//{
	//	break; 
	//}

	//if( action != NULL )
	//{
	//	ret = get_action_web_browser( NULL, action, &web_browser_info ); 
	//	if( ret != ERROR_SUCCESS )
	//	{
	//		ASSERT( FALSE ); 
	//		break; 
	//	}
	//}
	//else
	//{
	//	web_browser_info = NULL; 
	//}

	//dbg_print( MSG_IMPORTANT, "command ( 1st half process ) time %u\n", end_time - begin_time ); 
	//dbg_print( MSG_IMPORTANT, "command ( 2nd half process ) time %u\n", GetTickCount() - end_time ); 

	return ret; 
}


LRESULT WINAPI  parse_script_text( LPCWSTR text, wstring &text_out ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR variable_begin; 
	LPWSTR variable_end; 
	wstring _text; 
	wstring variable_name; 
	ULONG text_len; 
	LPWSTR text_pos; 
	LPWSTR text_dump = NULL; 

#define VARIABLE_TEXT_BEGIN L'{' 
#define VARIABLE_TEXT_END L'}' 

	do 
	{
		ASSERT( NULL != text ); 
		
		text_len = wcslen( text ); 

		text_dump = ( LPWSTR )malloc( ( text_len + 1 ) << 1 ); 
		if( text_dump == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( text_dump, text, ( ( text_len + 1 ) << 1 ) ); 

		_text = L""; 
		text_pos = text_dump; 
		for( ; ; )
		{
			variable_begin = wcschr( text_pos, VARIABLE_TEXT_BEGIN ); 
			if( NULL == variable_begin )
			{
				break; 
			}

			variable_end = wcschr( variable_begin, VARIABLE_TEXT_END ); 
			if( NULL == variable_end )
			{
				break; 
			}

			*variable_begin = L'\0'; 
			_text += text_pos; 

			*variable_end = L'\0'; 
			variable_name = variable_begin + 1; 

			{
				VARIABLE_INFO variable; 
				string _variable_name; 

				ret = unicode_to_utf8_ex( variable_name.c_str(), _variable_name ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				ret = get_script_variable( _variable_name.c_str(), &variable ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				{
					WCHAR digit_text[ MAX_DIGIT_LEN ]; 
					//LPWSTR temp_end_text; 

					switch( variable.variable.vt )
					{
					case VARIABLE_INT: 
						StringCchPrintfW( digit_text, 
							ARRAYSIZE( digit_text ), 
							L"%d", 
							variable.variable.intVal ); 
						break; 
					case VARIABLE_R8:
						StringCchPrintfW( digit_text, 
							ARRAYSIZE( digit_text ), 
							L"%f", 
							variable.variable.fltVal ); 
						break; 
					default:
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}
					
					if( ret != ERROR_SUCCESS )
					{
						break; 							
					}
					
					_text += digit_text; 
				}

				text_pos = variable_end + 1; 
			}
		}

		_text += text_pos; 

		if( ERROR_SUCCESS != ret )
		{
			text_out = _text; 
			//text_out.clear(); 
			break; 
		}

		text_out = _text; 
	}while( FALSE );

	if( NULL != text_dump )
	{
		free( text_dump ); 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::get_html_action_output( HTREEITEM selected_item, HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != config ); 

		if( 0 != wcscmp( config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do 
		{
			if( 0 != _wcsicmp( config->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT ) )
			{
				break; 
			}

			do 
			{
				HTREEITEM _tree_item; 
				HTREEITEM _sub_tree_item; 
				HTML_ELEMENT_ACTION *_config; 
				wstring content; 

				ret = get_xpath_content( config->xpath.c_str(), content, TRUE ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				output_dlg.set_output_text( content.c_str() ); 

				if( FALSE == output_dlg.IsWindowVisible())
				{
					output_dlg.ShowWindow( SW_SHOW ); 
				}

				_sub_tree_item = tree_elements.GetChildItem( selected_item ); 
				if( NULL == _sub_tree_item )
				{
					break; 
				}

				_sub_tree_item = tree_elements.GetChildItem( _sub_tree_item ); 
				if( NULL == _sub_tree_item )
				{
					break; 
				}

				_sub_tree_item = tree_elements.GetChildItem( _sub_tree_item ); 
				if( NULL == _sub_tree_item )
				{
					break; 
				}

				tree_elements.SetItemText( _sub_tree_item, content.c_str() ); 
				tree_elements.SelectItem( _sub_tree_item ); 
			}while( FALSE );  
		}while( FALSE ); 

		do 
		{
            HRESULT hr; 
            IWebBrowser2Ptr web_browser;
            IDispatchPtr disp;
            IHTMLDocument2Ptr html_doc; 

			HTREEITEM links_tree_item; 
			HTREEITEM sub_link_tree_item; 
			HTREEITEM next_sub_tree_item; 

			LRESULT _ret; 

            HTML_ELEMENT_INFO *element_info = NULL; 
            //vector<wstring> links; 
			HTREEITEM _sub_tree_item; 
			HTREEITEM _new_sub_tree_item = NULL; 
			HTML_ELEMENT_ACTION *_config; 
			wstring url; 

			do 
			{
                do
                {
                    web_browser = NULL;
                    html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument();
                    if (NULL == html_doc)
                    {
                        break;
                    }

                    disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown();

                    if (NULL == disp)
                    {
                        ASSERT(FALSE);
                        break;
                    }

                    hr = disp->QueryInterface(IID_IWebBrowser2, (PVOID*)&web_browser);
                    if (FAILED(hr))
                    {
                        break;
                    }

                    if (NULL == web_browser)
                    {
                        break;
                    }
                } while (FALSE);

				if( 0 != _wcsicmp( config->param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK ) )
				{
					break; 
				}

				_sub_tree_item = tree_elements.GetChildItem( selected_item ); 

				if( NULL == _sub_tree_item )
				{
					_ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				{
					CString item_text; 
					item_text = tree_elements.GetItemText( _sub_tree_item ); 
					ASSERT( 0 <= item_text.Find( L"个元素" ) ); 

                    _config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(selected_item);
                    ASSERT(NULL != _config);

                    element_info = new HTML_ELEMENT_INFO(); 
                    if (NULL == element_info)
                    {
                        break;
                    }

                    element_info->action_config = *_config; 

                    ret = ie_auto_execute_on_element_ex(element_info,
                        web_browser);
                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }

					//ret = get_xpath_links(config->xpath.c_str(), links, TRUE ); 
					//if( ret != ERROR_SUCCESS )
					//{
					//	break; 
					//}

					item_text.Format(HTML_ELEMENT_LIST_TITLE_FORMAT_TEXT, element_info->action_config.outputs.size() );
					tree_elements.SetItemText( _sub_tree_item, item_text.GetBuffer() ); 
					_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( _sub_tree_item ); 
					ASSERT( NULL != _config ); 
					_config->title = item_text.GetBuffer(); 
				}

                _new_sub_tree_item = tree_elements.GetChildItem(_sub_tree_item);
				_config = NULL; 
				for( ; ; )
				{
					if(element_info->action_config.outputs.size() == 0 )
					{
						break; 
					}

                    
					url = ( *element_info->action_config.outputs.rbegin()); 
                    element_info->action_config.outputs.pop_back();

					if( _new_sub_tree_item == NULL )
					{
						do 
						{
							_new_sub_tree_item = insert_tree_item( HTML_ELEMENT_ACTION_TYPE_NONE, 
								url.c_str(), 
								_sub_tree_item, 
								TVI_LAST ); 

							if( _new_sub_tree_item == NULL )
							{
								break; 
							}

							ret = allocate_html_element_action( &_config ); 
							if( ERROR_SUCCESS != ret )
							{
								break; 
							}

							ASSERT( NULL != _config ); 

							_config->action = HTML_ELEMENT_ACTION_NONE; 
							_config->in_frame = FALSE; 
							_config->param = HTML_ELEMENT_ACTION_NONE_PARAM_LINK; 
							_config->xpath = L""; 
							//_config->url = (*it).c_str(); 
							//_config->output = ( *it ).c_str(); 
							_config->title = url.c_str(); 

							ret = attach_element_config( _new_sub_tree_item, _config ); 

							if( ERROR_SUCCESS != ret )
							{
								break; 
							}

							_config = NULL; 
						}while( FALSE );

						if( _config != NULL )
						{
							detach_free_element_config( _new_sub_tree_item, _config ); 
							_config = NULL; 
						}

						_new_sub_tree_item = NULL; 
					}
					else
					{
						_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( _new_sub_tree_item ); 
						ASSERT( 0 == wcscmp( _config->param.c_str(), 
							HTML_ELEMENT_ACTION_NONE_PARAM_LINK ) ); 

						_config->title = url.c_str(); 

						tree_elements.SetItemText( _new_sub_tree_item, url.c_str() ); 
					}

					_new_sub_tree_item = tree_elements.GetNextSiblingItem( _new_sub_tree_item ); 
				}
			}while( FALSE );

			for( ; ; )
			{
				if( _new_sub_tree_item == NULL )
				{
					break; 
				}

				next_sub_tree_item = tree_elements.GetNextSiblingItem( _new_sub_tree_item ); 
				_ret = delete_tree_item( _new_sub_tree_item ); 
				_new_sub_tree_item = next_sub_tree_item; 
			}

            if (NULL != element_info)
            {
                delete element_info;
            }
		}while( FALSE ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::html_element_action_test()
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	HTML_PAGE_INFO page; 
	LONG element_count; 
	LONG i; 

	HTML_ELEMENT_INFO *element_info = NULL; 
	HTML_ELEMENT_INFOS elements; 

	HTREEITEM tree_item; 
	LONG item_count; 

	CString action; 
	CString action_param; 

	HTML_ELEMENT_ACTION *config; 

	do 
	{
		item_count = 0; 
		tree_item = tree_elements.GetFirstVisibleItem(); 
		for( ; ; )
		{
			if( tree_item == NULL )
			{
				break; 
			}

			do 
			{
				if( FALSE == tree_elements.GetCheck(tree_item))
				{
					break; 
				}

				item_count ++; 

				config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData(tree_item); 
				if( NULL == config )
				{
					ASSERT( FALSE ); 
					break; 
				}

				if( 0 == _wcsicmp( config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) )
				{
					ret = get_html_action_output( tree_item, config ); 
					break; 
				}

				element_info = new HTML_ELEMENT_INFO(); 
				if( NULL == element_info )
				{
					break; 
				}

				element_info->action_config = *config; 

				ret = _add_html_element( element_info, &elements ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					delete element_info; 
					element_info = NULL; 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				element_info = NULL; 
			}while( FALSE ); 

			tree_item = tree_elements.GetNextVisibleItem( tree_item ); 
		}

		if( element_info != NULL )
		{
			delete element_info; 
			element_info = NULL; 
		}

		if( item_count == 0 )
		{
			do 
			{
				tree_item = tree_elements.GetSelectedItem(); 
				if( NULL == tree_item )
				{
					ASSERT( FALSE ); 
					break; 
				}

				config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( tree_item ); 
				if( NULL == config )
				{
					ASSERT( FALSE ); 
					break; 
				}

				if( 0 == _wcsicmp( config->action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) )
				{
					ret = get_html_action_output( tree_item, config ); 
					break; 
				}

				element_info = new HTML_ELEMENT_INFO(); 
				if( NULL == element_info )
				{
					break; 
				}

				element_info->action_config = *config; 

				ret = _add_html_element( element_info, &elements ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					delete element_info; 
					element_info = NULL; 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
				element_info = NULL; 
			}while( FALSE );
		}

		if( element_info != NULL )
		{
			delete element_info; 
			element_info = NULL; 
		}

		do 
		{
			web_browser = NULL; 
			html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument(); 
			if( NULL == html_doc ) 
			{ 
				break; 
			} 

			disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown(); 

			if( NULL == disp )
			{
				ASSERT( FALSE ); 
				break; 
			}

			hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID* )&web_browser ); 
			if( FAILED( hr ))
			{
				break; 
			}

			if( NULL == web_browser )
			{
				break; 
			}
		}while( FALSE ); 

		for( HTML_ELEMENT_INFO_ITERATOR it = elements.begin(); it != elements.end(); it ++ ) //Loop through each element
		{
			do 
			{
				if( 0 == wcscmp( config->action.c_str(), HTML_ELEMENT_ACTION_NONE) 
					&& 0 == wcscmp( config->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_LINK) )
				{
					CString link; 
					link = config->title.c_str(); 

					browser_safe_navigate_start( &g_html_script_config_dlg->m_WebBrowser, 
						link.GetBuffer() ); 
					break; 
				}

				if( 0 == _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_BACK ) )
				{
					g_html_script_config_dlg->OnBack(); 
					break; 
				}

				if(  0 == _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_SCRIPT ) )
				{
					exec_html_script_action( &( *it )->action_config ); 
					break; 
				}

				if(  0 == _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_LOCATE ) )
				{
					wstring url; 
					url = ( *it )->action_config.param; 
					if( url.length() == 0 )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}
					
					ret = parse_script_text( url.c_str(), url ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					browser_safe_navigate_start( &g_html_script_config_dlg->m_WebBrowser, url.c_str() ); 
					break; 
				}
				if( 0 == _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK )
					&& 0 == _wcsicmp( ( *it )->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ))
				{
					g_html_script_config_dlg->m_WebBrowser.switch_navigate( TRUE ); 
				}

				if( web_browser == NULL )
				{
					break; 
				}

				ret = ie_auto_execute_on_element_ex( ( *it ), 
					web_browser ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( 0 == _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK )
					&& 0 == _wcsicmp( ( *it )->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ))
				{
					dbg_print_w( MSG_IMPORTANT, L"set location url to %s\n", g_html_script_config_dlg->ui_handler->get_translate_url().c_str() ); 

					g_html_script_config_dlg->m_WebBrowser.set_loading_url( g_html_script_config_dlg->ui_handler->get_translate_url().c_str() ); 
				}
			}while( FALSE ); 
		}
	}while( FALSE ); 

    release_html_element_infos(&elements);

	return ret; 
}

LRESULT html_element_prop_dlg::_html_element_action_test( HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	HTML_PAGE_INFO page; 
	LONG element_count; 
	LONG i; 

	HTML_ELEMENT_INFO *element_info = NULL; 
	HTML_ELEMENT_INFOS elements; 

	CString action; 
	CString action_param; 

	do 
	{
		ASSERT( NULL != config ); 

		html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument(); 
		if( NULL == html_doc ) 
		{ 
			break; 
		} 

		disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown(); 

		if( NULL == disp )
		{
			ASSERT( FALSE ); 
			break; 
		}

		hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == web_browser )
		{
			break; 
		}

		do 
		{
			element_info = new HTML_ELEMENT_INFO(); 
			if( NULL == element_info )
			{
				break; 
			}

			element_info->action_config = *config; 

			ret = _add_html_element( element_info, &elements ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				delete element_info; 
				element_info = NULL; 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			element_info = NULL; 
		}while( FALSE ); 

		if( element_info != NULL )
		{
			delete element_info; 
			element_info = NULL; 
		}

		for( HTML_ELEMENT_INFO_ITERATOR it = elements.begin(); it != elements.end(); it ++ ) //Loop through each element
		{
			do 
			{
				//if( 0 == wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK )
				//	&& 0 == wcsicmp( ( *it )->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE ))
				//{
				//	g_html_script_config_dlg->m_WebBrowser.switch_navigate( TRUE ); 
				//}

				ret = ie_auto_execute_on_element_ex( ( *it ), 
					web_browser ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				do 
				{
                    wstring output_data; 

					if( 0 != _wcsicmp( ( *it )->action_config.action.c_str(), HTML_ELEMENT_ACTION_OUTPUT ) )
					{
						break; 
					}

					if( 0 != _wcsicmp( (*it)->action_config.param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT ) )
					{
						break; 
					}

                    cat_output_data(&(*it)->action_config, output_data);
					output_dlg.set_output_text(output_data.c_str());
					if( FALSE == output_dlg.IsWindowVisible())
					{
						output_dlg.ShowWindow( SW_SHOW ); 
					}

				} while ( FALSE );
			}while( FALSE ); 
		}
	}while( FALSE ); 

    release_html_element_infos(&elements); 

	return ret; 
}

LRESULT WINAPI html_script_worker_construct( SCRIPT_WORKER *worker )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		worker->stop_work = TRUE; 
		worker->running = FALSE; 
		worker->exit = FALSE; 
		worker->action = NULL; 
		worker->worker = NULL; 
		//worker->lock = NULL; 
		worker->work_notifier = NULL; 
		worker->state_notifier = NULL; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI html_script_instance_construct( HTML_SCRIPT_INSTANCE *instance )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != instance ); 

		instance->actions = NULL; 
		instance->loop_count = -1; 

		html_script_worker_construct( &instance->worker ); 
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::get_script_instances_from_ui() 
{
	LRESULT ret = ERROR_SUCCESS; 
	LONG item_count; 
	LONG i; 
	CString text; 
	HTML_SCRIPT_INSTANCE *instance; 
	LPWSTR temp_text; 

	do 
	{
		item_count = locate_urls.GetItemCount(); 
		for( i = 0; i < item_count; )
		{
			do 
			{
				ASSERT( i < locate_urls.GetItemCount() ); 

				instance = &script_instances[ i ]; 

				ret = get_html_script_instance( i, instance ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				i ++; 
			}while( FALSE ); 
		}

		for( ; i < ARRAYSIZE( script_instances ); i ++ )
		{
			script_instances[ i ].actions = NULL; 
			script_instances[ i ].begin_url = L""; 
			script_instances[ i ].location_url = L""; 
			script_instances[ i ].loop_count = 0; 
		}
	}while( FALSE ); 

	return ret; 
}

void html_element_prop_dlg::OnBnClickedButtonExecute()
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	HRESULT hr; 
	HTREEITEM tree_item; 
	CString action; 
	CString action_param; 

	HTML_ELEMENT_ACTION *config; 

	do 
	{
#ifdef _DEBUG
		//html_element_action_test(); 
#endif //_DEBUG

		//ret = process_current_user_access(); 
		//if( ERROR_SUCCESS != ret )
		//{
		//	break; 
		//}

		{
			LRESULT _ret; 
#ifdef NO_SUPPORT_DELAY_TIME_ADJUST
			_ret = save_html_action_global_config(); 
			if( _ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "error\n" ); 
			}
#endif //NO_SUPPORT_DELAY_TIME_ADJUST
		}

		if( script_running == FALSE )
		{
			HTML_ELEMENT_ACTION *root_config; 
			HTREEITEM root_item; 

			//{
			//	LRESULT _ret; 
			//	CString file_name; 

			//	GetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, file_name ); 

			//	if( file_name.GetLength() > 0 )
			//	{
			//		_ret = set_data_scrambler_config_ex( file_name.GetBuffer() ); 
			//		if( _ret != ERROR_SUCCESS )
			//		{
			//			//break; 
			//		}
			//	}
			//}

			ret = get_script_instances_from_ui(); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//ASSERT( script_work.running == FALSE ); 
			root_item = tree_elements.GetChildItem( TVI_ROOT ); 
			if( root_item == NULL )
			{
				break; 
			}

			root_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( root_item ); 

			ASSERT( root_config ); 

			ret = start_html_script_work_ex( root_config ); 
			
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( FALSE ); 

			SetDlgItemText( IDC_BUTTON_EXECUTE, BUTTON_STOP_TEXT ); 
			script_running = TRUE; 
		}
		else
		{
			ret = stop_html_script_work_ex(); 
			if( ERROR_SUCCESS == ret )
			{
				script_running = FALSE; 
			}

			//ASSERT( script_work.running == TRUE); 
			//GetDlgItem( IDC_BUTTON_ADD )->EnableWindow( TRUE ); 
			//SetDlgItemText( IDC_BUTTON_EXECUTE, BUTTON_RUN_TEXT ); 
		}

	}while( FALSE ); 
}

//void html_element_prop_dlg::OnHdnItemdblclickListElements(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
//	// TODO: Add your control notification handler code here
//	*pResult = 0;
//}
//
//void html_element_prop_dlg::OnNMDblclkListElements(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
//	INT32 item_index; 
//
//	*pResult = 0;
//
//	do 
//	{
//		//item_index = pNMItemActivate->iItem; 
//	
//		//tree_elements.DeleteItem( item_index ); 
//
//	}while( FALSE ); 
//
//	return; 
//}

#include "xpath_edit_dlg.h"
void html_element_prop_dlg::OnBnClickedButtonXpathEdit()
{
	CString xpath; 

	GetDlgItemText( IDC_EDIT_XPATH, xpath ); 
	xpath_dlg.set_xpath1( xpath ); 

	xpath_dlg.ShowWindow( SW_SHOW ); 
}

LRESULT WINAPI init_config_list()
{
	LRESULT ret = ERROR_SUCCESS; 
	InitializeCriticalSection( &config_list_lock ); 
	element_config_base_id = 0; 
	return ret; 
}

LRESULT WINAPI uninit_config_list()
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( true == config_list.empty() ); 

	DeleteCriticalSection( &config_list_lock ); 
	return ret; 
}

LRESULT html_element_prop_dlg::on_update_exit_work_time_delay( WPARAM wparam, 
													   LPARAM lparam) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		CString text; 
		text.Format( L"%u", ( ULONG )lparam ); 
		SetDlgItemText( IDC_EDIT_SHUTDOWN_SYSTEM_DELAY, text ); 
	} while ( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::on_html_document_complete(LPCWSTR url)
{
    LRESULT ret = ERROR_SUCCESS;
    wstring domain_name;
    size_t first_dot;
    size_t second_dot;
    LPCWSTR temp_text; 
    LPWSTR _domain_name; 
    BOOLEAN domain_name_mismatch = FALSE; 
    INT32 i; 

    do
    {
        if (NULL == url)
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

        if (L'\0' == *url)
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

        ret = get_domain_name_in_url(url, domain_name);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        domain_name_mismatch = FALSE; 

        do
        {
            first_dot = domain_name.find(L'.', 0);
            if (first_dot < 0)
            {
                ret = ERROR_INVALID_PARAMETER;
                break;
            }


            second_dot = domain_name.find(L'.', first_dot);
            if (second_dot < 0)
            {
                break;
            }

            domain_name = domain_name.substr(first_dot + 1);

            for (i = 0; i < locate_urls.GetItemCount(); i++)
            {
                temp_text = (LPWSTR)(PVOID)locate_urls.GetItemData(i);
                if (NULL == temp_text)
                {
                    break;
                }

                if (*temp_text != L'\0')
                {
                    _domain_name = wcsistr(temp_text, domain_name.c_str());
                    if (NULL == _domain_name)
                    {
                        domain_name_mismatch = TRUE;
                    }
                }
                else
                {
                    ASSERT(FALSE);
                }
            }

            if (domain_name_mismatch == TRUE)
            {
                //使用专用的对话框处理这个提示
                if (MessageBox(L"当前浏览网页链接与起始网页链接不同,是否更新起始网页链接?", L"", MB_YESNO) == IDYES) //和配置文件路径?(注意可能覆盖同名文件)
                {
                    locate_urls.SetItemText(0, 0, url);
                    locate_urls.SetItemText(0, 1, L"-1");

                    //{
                    //    PVOID old_data;
                    //    old_data = (PVOID)locate_urls.GetItemData(0);
                    //    if (NULL != old_data)
                    //    {
                    //        free(old_data);
                    //    }
                    //}

                    {
                        LPWSTR _locate_url;

                        do
                        {

                            _locate_url = (LPWSTR)locate_urls.GetItemData(0);
                            if (NULL != _locate_url)
                            {
                                free(_locate_url);
                            }

                            _locate_url = (LPWSTR)malloc((wcslen(url) + 1) << 1);
                            if (NULL == _locate_url)
                            {
                                break;
                            }

                            memcpy(_locate_url,
                                url,
                                ((wcslen(url) + 1) << 1));

                            locate_urls.SetItemData(0, (DWORD_PTR)_locate_url);

                        } while (FALSE);
                    }

                }

                if (MessageBox(L"是否更新配置文件名与域名一致?(请注意配置文件以域名命名，避免覆盖同名文件)", L"", MB_YESNO) == IDYES) //
                {
                    CString file_name;
                    INT32 delim;

                    if (instruction_chnaged == TRUE
                        && IDYES == MessageBox(L"是否保存正在编辑的命令?", L"", MB_YESNO))
                    {
                        save_html_script_ex(0);
                    }

                    GetDlgItemText(IDC_EDIT_CONFIG_FILE_NAME, file_name);

                    delim = file_name.ReverseFind(PATH_DELIM_CH);
                    if (delim < 0)
                    {
                        ASSERT(FALSE);
                        break;
                    }

                    file_name = file_name.Left(delim + 1);
                    file_name += domain_name.c_str();
                    file_name += L".xml";

                    SetDlgItemText(IDC_EDIT_CONFIG_FILE_NAME, file_name.GetBuffer());
                }
            }
        } while (FALSE);        
    } while (FALSE);

    return ret;
}

LRESULT html_element_prop_dlg::attach_element_config( HTREEITEM item, 
													   HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_config = NULL; 
	BOOLEAN lock_held = FALSE; 

	do 
	{
		ASSERT( NULL != item ); 
		ASSERT( NULL != config ); 

		if( NULL == item )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		EnterCriticalSection( &config_tree_lock ); 
		lock_held = TRUE; 

		_config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData(item); 
		if( NULL != _config )
		{
			//EnterCriticalSection( &config_list_lock ); 
			//config_list.push_back( _config); 
			//LeaveCriticalSection( &config_list_lock ); 

			config->id = _config->id; 
			detach_free_element_config( item, _config ); 
		}
		//else
		//{
		//	EnterCriticalSection( &config_list_lock ); 
		//	config->id = element_config_ount; 
		//	element_config_ount ++; 
		//	config_list.push_back(config); 
		//	LeaveCriticalSection( &config_list_lock ); 
		//}

		_config = config; 
		if( FALSE == tree_elements.SetItemData( item, (DWORD_PTR)(PVOID)_config ))
		{
			dbg_print( MSG_FATAL_ERROR, "set the item %p to %p error\n", item, config ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		do
		{
			HTREEITEM parent_item; 
			HTREEITEM sub_item; 
			HTREEITEM next_sibling_item; 
			HTREEITEM prev_sibling_item; 
            HTREEITEM first_sibling_item;

            HTML_ELEMENT_ACTION *first_sibling_config = NULL;
			HTML_ELEMENT_ACTION *parent_config = NULL; 
			HTML_ELEMENT_ACTION *sub_config = NULL; 
			HTML_ELEMENT_ACTION *next_sibling_config = NULL; 
			HTML_ELEMENT_ACTION *prev_sibling_config = NULL; 

			//这部分逻辑不可以有任何一步出错，必须原子化的进行工作
			do 
			{
				_config->next_item = NULL; 
				_config->parent_item = NULL; 
				_config->sub_item = NULL; 

				parent_item = tree_elements.GetParentItem( item ); 
				if( parent_item == NULL )
				{
					break; 
				}

				parent_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( parent_item ); 
				if( parent_config == NULL )
				{
					break; 
				}

                first_sibling_item = tree_elements.GetChildItem(parent_item);
                //first_sibling_item = item; 
                //for (; ; )
                //{
                //    prev_sibling_item = tree_elements.GetPrevSiblingItem(first_sibling_item);
                //    if (NULL == prev_sibling_item)
                //    {
                //        break; 
                //    }

                //    first_sibling_item = prev_sibling_item; 
                //}

                first_sibling_config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(first_sibling_item);
                if (first_sibling_config == NULL)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED; 
                    ASSERT(FALSE); 
                    break;
                }

				parent_config->sub_item = first_sibling_config;
                ASSERT(first_sibling_config->parent_item = parent_config); 

				_config->parent_item = parent_config; 
			} while ( FALSE ); 

			do 
			{
				sub_item = tree_elements.GetChildItem( item ); 
				if( NULL == sub_item )
				{
					break; 
				}

				sub_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( sub_item ); 
				if( sub_config == NULL )
				{
					break; 
				}

				if( sub_config->parent_item != _config->parent_item )
				{
					dbg_print( MSG_FATAL_ERROR, "parent item is not match %p:%p\n", 
						sub_config->parent_item, 
						_config->parent_item ); 

					//ret = ERROR_ERRORS_ENCOUNTERED; 
					//ASSERT( FALSE ); 
				}

				_config->sub_item = sub_config; 
				sub_config->parent_item = _config; 
			} while ( FALSE );

			do 
			{
				next_sibling_item = tree_elements.GetNextSiblingItem( item ); 
				if( NULL == next_sibling_item )
				{
					break; 
				}

				next_sibling_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( next_sibling_item ); 
				if( next_sibling_config == NULL )
				{
					break; 
				}

				_config->next_item = next_sibling_config; 
			} while ( FALSE ); 

			do 
			{
				prev_sibling_item = tree_elements.GetPrevSiblingItem( item ); 
				if( NULL == prev_sibling_item )
				{
					break; 
				}

                ASSERT(prev_sibling_item != next_sibling_item); 

				prev_sibling_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( prev_sibling_item ); 
				if( prev_sibling_config == NULL )
				{
					break; 
				}

				ASSERT( prev_sibling_config != next_sibling_config ); 
				ASSERT( prev_sibling_config->next_item == NULL 
					|| prev_sibling_config->next_item == next_sibling_config ); 
				prev_sibling_config->next_item = _config; 
			} while ( FALSE ); 
		}while( FALSE ); 

		if( ERROR_SUCCESS != ret )
		{
			dbg_print( MSG_FATAL_ERROR, "attach tree item configure error %d, must restore to the original state\n", ret ); 
		}
	}while( FALSE ); 

	if( TRUE == lock_held )
	{
		EnterCriticalSection( &config_tree_lock ); 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::set_element_config( HTREEITEM item, 
													 HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_config = NULL; 
	BOOLEAN lock_held = FALSE; 

	do 
	{
		ASSERT( NULL != item ); 
		ASSERT( NULL != config ); 

		if( NULL == item )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		EnterCriticalSection( &config_tree_lock ); 
		lock_held = TRUE; 

		_config = config; 
		if( FALSE == tree_elements.SetItemData( item, (DWORD_PTR)(PVOID)_config ))
		{
			dbg_print( MSG_FATAL_ERROR, "set the item %p to %p error\n", item, config ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	}while( FALSE ); 

	if( ERROR_SUCCESS != ret )
	{
		dbg_print( MSG_FATAL_ERROR, "attach tree item configure error %d, must restore to the original state\n", ret ); 
	}

	if( TRUE == lock_held )
	{
		EnterCriticalSection( &config_tree_lock ); 
	}

	return ret; 
}

LRESULT WINAPI get_html_element_action_id( HTML_ELEMENT_ACTION *root, ULONG *id )
{

	LRESULT ret = ERROR_SUCCESS; 
	vector< HTML_ELEMENT_ACTION*> actions; 
	HTML_ELEMENT_ACTION *_action; 
	ULONG max_id = 0; 

	do 
	{
		ASSERT( NULL != root ); 
		ASSERT( NULL != id ); 

		actions.push_back( root ); 

		for( ; ; )
		{
			if( actions.size() == 0 )
			{
				break; 
			}

			do 
			{
				_action = *actions.rbegin(); 
				actions.pop_back(); 
				if( NULL == _action )
				{
					break; 
				}

				if( _action->id == INVALID_JUMP_TO_ID )
				{
					ASSERT( FALSE ); 
				}
				else if( max_id < _action->id )
				{
					max_id = _action->id; 
				}

				if( NULL != _action->next_item )
				{
					actions.push_back( _action->next_item ); 
				}

				if( NULL != _action->sub_item )
				{
					actions.push_back( _action->sub_item ); 
				}
			}while( FALSE ); 			
		}

		*id = max_id; 

	}while( FALSE ); 
	return ret; 
}

LRESULT html_element_prop_dlg::get_max_html_action_id( ULONG *id )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM item; 
	HTML_ELEMENT_ACTION *action; 

	do 
	{
		*id = 0; 
		item = tree_elements.GetChildItem( TVI_ROOT ); 
		if( NULL == item )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			//ASSERT( FALSE ); 
			break; 
		}

		action = ( HTML_ELEMENT_ACTION*)tree_elements.GetItemData( item ); 
		ASSERT( NULL != action ); 
		get_html_element_action_id( action, id ); 
	} while ( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::allocate_element_config_ex( HTML_ELEMENT_ACTION **config )
{
	LRESULT ret; 
	ULONG max_id; 
	do
	{
		get_max_html_action_id( &max_id ); 

		ret = allocate_html_element_action( config ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ASSERT( ( ( *config )->id == 0 && max_id == 0 )
			|| ( *config )->id > max_id ); 
	}while( FALSE ); 
	return ret; 
}


LRESULT html_element_prop_dlg::update_base_html_element_action_id(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        EnterCriticalSection(&config_list_lock);

        if (element_config_base_id <= action->id)
        {
            element_config_base_id = action->id + 1;
        }

        LeaveCriticalSection(&config_list_lock);
    } while (FALSE);
    return ret;
}

LRESULT html_element_prop_dlg::allocate_html_element_action( HTML_ELEMENT_ACTION **config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_config = NULL; 

	do 
	{
		ASSERT( NULL != config ); 
		
		*config = NULL; 

		ret = _allocate_element_config( &_config ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		EnterCriticalSection( &config_list_lock ); 
		_config->id = element_config_base_id; 
		element_config_base_id ++; 
		config_list.push_back( _config ); 
		LeaveCriticalSection( &config_list_lock ); 

		*config = _config; 
		_config = NULL; 
	}while( FALSE );

	if( NULL != _config )
	{
		delete _config; 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::_allocate_element_config( HTML_ELEMENT_ACTION **config )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_config = NULL; 

	do 
	{
		ASSERT( NULL != config ); 

		*config = NULL; 

		_config = new HTML_ELEMENT_ACTION(); 
		if( NULL == _config )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_config->output_data = FALSE; 
		_config->in_frame = FALSE; 
		_config->jump_to_id = INVALID_JUMP_TO_ID; 
		_config->next_item = NULL; 
		_config->parent_item = NULL; 
		_config->sub_item = NULL; 

		*config = _config; 
		_config = NULL; 
	}while( FALSE );

	if( NULL != _config )
	{
		delete _config; 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::free_element_config( HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != config ); 

		if( NULL == config )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		EnterCriticalSection( &config_list_lock ); 
		for( vector<HTML_ELEMENT_ACTION*>::iterator it = config_list.begin(); 
			it != config_list.end(); 
			it ++ )
		{
			if( ( *it ) == config )
			{
				config_list.erase( it ); 
				break; 
			}
		}
		LeaveCriticalSection( &config_list_lock ); 
		
		delete config; 

	}while( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::get_tree_item_config( HTREEITEM item, 
													 HTML_ELEMENT_ACTION **action_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *action; 

	ASSERT( NULL != item ); 
	ASSERT( NULL != action_out ); 

	action = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item ); 
	do 
	{
		if( NULL == action )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		if( 0 == wcscmp( action->action.c_str(), HTML_ELEMENT_ACTION_NONE ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		*action_out = action; 
	} while ( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::traverse_tree_item_in_order( HTREEITEM item, 
														 traverse_tree_item_func traverse, 
														 PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM sub_item; 
	HTREEITEM next_item; 
	vector< HTREEITEM > next_items; 
	PVOID data; 

	do 
	{
		ASSERT( NULL != item ); 
		ASSERT( NULL != traverse ); 

		next_items.push_back( item ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			item = *next_items.rbegin(); 

			next_items.pop_back(); 

			data = ( PVOID )tree_elements.GetItemData( item ); 
			(this->*traverse)( item, data, context ); 

			do 
			{
				next_item = tree_elements.GetNextSiblingItem( item ); 

				if( NULL == next_item )
				{
					break; 
				}

				next_items.push_back( next_item ); 
			} while ( FALSE ); 

			do 
			{
				sub_item = tree_elements.GetChildItem( item ); 

				if( NULL == sub_item )
				{
					break; 
				}

				next_items.push_back( sub_item ); 

			} while ( FALSE ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::get_tree_item_configs( HTREEITEM item, 
													 HTML_ELEMENT_ACTION_LIST &actions )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTREEITEM sub_item; 
	HTREEITEM next_item; 
	HTML_ELEMENT_ACTION *action; 
	vector< HTREEITEM > next_items; 

	do 
	{
		ASSERT( NULL != item ); 

		next_items.push_back( item ); 

		for( ; ; )
		{
			if( true == next_items.empty() )
			{
				break; 
			}

			item = *next_items.rbegin(); 
			
			next_items.pop_back(); 

			for( ; ; )
			{
				do 
				{
					ret = get_tree_item_config( item, &action ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}
					
					ASSERT( NULL != action ); 
					actions.push_back( action ); 
				} while ( FALSE );

				do 
				{
					sub_item = tree_elements.GetChildItem( item ); 

					if( NULL == sub_item )
					{
						break; 
					}

					ret = get_tree_item_config( sub_item, &action ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( NULL != action ); 
					actions.push_back( action ); 
				} while ( FALSE ); 

				do 
				{
					next_item = tree_elements.GetNextSiblingItem( item ); 

					if( NULL == next_item )
					{
						break; 
					}

					ret = get_tree_item_config( next_item, &action ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					ASSERT( NULL != action ); 
					actions.push_back( action ); 
				} while ( FALSE ); 

				if( next_item )
				{
					next_items.push_back( next_item ); 
				}

				if( sub_item )
				{
					next_items.push_back( sub_item ); 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::free_tree_item_config( HTREEITEM tree_item, PVOID data, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	
	do 
	{
		if( NULL != data )
		{
			config = ( HTML_ELEMENT_ACTION* )data; 
			free_element_config( config ); 
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::detach_element_config( HTREEITEM item, 
												   HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != config ); 
		if( NULL == config )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		EnterCriticalSection( &config_tree_lock ); 

		do
		{
			HTREEITEM parent_item; 
			HTREEITEM sub_item; 
			HTREEITEM next_sibling_item; 
			HTREEITEM prev_sibling_item; 
			HTML_ELEMENT_ACTION *parent_config = NULL; 
			HTML_ELEMENT_ACTION *sub_config = NULL; 
			HTML_ELEMENT_ACTION *next_sibling_config = NULL; 
			HTML_ELEMENT_ACTION *prev_sibling_config = NULL; 

			do 
			{
				if( item == NULL )
				{
					break; 
				}

				do 
				{
					config->next_item = NULL; 

					next_sibling_item = tree_elements.GetNextSiblingItem( item ); 
					if( next_sibling_item == NULL )
					{
						break; 
					}

					next_sibling_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( next_sibling_item ); 
					if( next_sibling_config == NULL )
					{
						break; 
					}
					
					if( next_sibling_config == config )
					{
						next_sibling_config = NULL; 
					}

				} while ( FALSE ); 

				do 
				{
					prev_sibling_item = tree_elements.GetPrevSiblingItem( item ); 
					if( NULL == prev_sibling_item )
					{
						break; 
					}

					prev_sibling_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( prev_sibling_item ); 
					ASSERT( prev_sibling_config != config ); 
					if( prev_sibling_config == NULL 
						|| prev_sibling_config == config )
					{
						break; 
					}

					if( prev_sibling_config->next_item == config ) 
					{
						prev_sibling_config->next_item = next_sibling_config; 
					}
					else
					{
						ASSERT( FALSE && "element configure tree corrupt" ); 
					}
				} while ( FALSE ); 

				do 
				{
					config->parent_item = NULL; 
					parent_item = tree_elements.GetParentItem( item ); 
					if( NULL == parent_item )
					{
						break; 
					}

					parent_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( parent_item ); 
					if( parent_config == NULL )
					{
						break; 
					}

					ASSERT( NULL != parent_config->sub_item ); 

					if( parent_config->sub_item == config )
					{
						parent_config->sub_item = next_sibling_config; 
					}
				} while ( FALSE ); 

				do 
				{
					config->sub_item = NULL; 

					sub_item = tree_elements.GetChildItem( item ); 

					if( NULL == sub_item )
					{
						break; 
					}

					ret = traverse_tree_item_in_order( sub_item, 
						&html_element_prop_dlg::free_tree_item_config, 
						NULL ); 

					if( ERROR_SUCCESS != ret )
					{
						//break; 
					}

					//sub_config = ( ELEMENT_CONFIG* )( PVOID )tree_elements.GetItemData( sub_item ); 
					//if( sub_config == NULL )
					//{
					//	break; 
					//}

					//if( sub_config->parent_item != config )
					//{
					//	dbg_print( MSG_FATAL_ERROR, "parent item is not match %p:%p\n", 
					//		sub_config->parent_item, 
					//		config->parent_item ); 

					//	ASSERT( FALSE ); 
					//}

					////ASSERT( sub_config->parent_item == config ); 
				} while ( FALSE ); 
			}while( FALSE ); 
		}while( FALSE ); 

		if( FALSE == tree_elements.SetItemData( item, (DWORD_PTR)(PVOID)NULL ) )
		{
			dbg_print( MSG_FATAL_ERROR, "set the tree item 0x%0.8x to null error\n", item ); 
		}

	}while( FALSE ); 

	LeaveCriticalSection( &config_tree_lock ); 

	return ret; 
}

HTREEITEM html_element_prop_dlg::insert_tree_item( ULONG action, 
												  LPCWSTR item_text, 
												  HTREEITEM parent_item, 
												  HTREEITEM after_item )
{
    HTREEITEM item; 
	INT32 image_index; 

	switch( action ) 
	{
	case HTML_ELEMENT_ACTION_TYPE_JUMP:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_CLICK:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_OUTPUT:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_INPUT:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_BACK:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_LOCATE:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_HOVER:
		image_index = 0; 
		break; 
	case HTML_ELEMENT_ACTION_TYPE_SCRIPT:
		image_index = 2; 
		break; 
	default:
		image_index = 0; 
		break; 
	}

	item =  tree_elements.InsertItem( item_text, image_index, image_index, parent_item, after_item ); 

    if (NULL != item)
    {
        instruction_chnaged = TRUE; 
        tree_elements.Invalidate(FALSE); 
    }

    return item; 
}

HTREEITEM html_element_prop_dlg::_insert_tree_item( LPCWSTR action, 
												 LPCWSTR item_text, 
												 HTREEITEM parent_item, 
												 HTREEITEM after_item )
{
	ULONG acton_type = get_action_type( action ); 
	return insert_tree_item( acton_type, item_text, parent_item, after_item ); 
}

LRESULT html_element_prop_dlg::add_jump_to_instruction( HTREEITEM jump_from_item, 
													   HTREEITEM jump_to_item )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	HTREEITEM container_item; 
	HTREEITEM new_item; 
	HTML_ELEMENT_ACTION *config; 
	HTML_ELEMENT_ACTION *new_element_config = NULL; 

#define MAX_JUMP_TO 64
	WCHAR text[ MAX_JUMP_TO ]; 
	
	do 
	{
		ASSERT( NULL != jump_from_item ); 
		ASSERT( NULL != jump_to_item ); 

		{
			config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( jump_to_item ); 
			if( config == NULL )
			{
				ASSERT( FALSE ); 
				//MessageBox( L"请选择")
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( 0 == wcscmp( config->action.c_str(), HTML_ELEMENT_ACTION_NONE ) )
			{
				ret = ERROR_PARAM_NO_MEANING; 
				MessageBox( L"请选择有动作的条目" ); 
				break; 
			}

			{
				WCHAR jump_to_text[ MAX_PATH ]; 

				StringCchPrintfW( jump_to_text, 
					ARRAYSIZE( jump_to_text ), 
					L"跳转至 ID:%u (%s)", 
					config->id, 
					config->html_text.c_str() ); 

				container_item = tree_elements.GetParentItem( jump_from_item ); 
				new_item = insert_tree_item( HTML_ELEMENT_ACTION_TYPE_JUMP, 
					jump_to_text, 
					container_item, 
					jump_from_item ); 
			}

			ret = allocate_html_element_action( &new_element_config ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			ASSERT( NULL != new_element_config); 

			new_element_config->action = HTML_ELEMENT_ACTION_JUMP; 
			new_element_config->param = L""; 
			new_element_config->jump_to_id = config->id; 

			ret = attach_element_config( new_item, new_element_config ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//StringCchPrintfW( text, ARRAYSIZE( text ), L"%u", item_index ); 
			//UpdateData( TRUE ); 
			new_element_config = NULL; 
		}
	}while( FALSE );

	if( NULL != new_element_config )
	{
		free_element_config( new_element_config ); 
	}

	return ret; 
}

//void html_element_prop_dlg::OnNMClickListElements(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
//	LRESULT ret; 
//	INT32 item_index; 
//	wstring xpath; 
//	wstring action; 
//	wstring action_param; 
//	// TODO: Add your control notification handler code here
//	*pResult = 0;
//
//	do 
//	{
//		item_index = pNMItemActivate->iItem; 
//
//		if( item_index < 0 )
//		{
//			break; 
//		}
//
//		//if( item_index >= tree_elements.GetItemCount() )
//		//{
//		//	ASSERT( FALSE ); 
//		//	break; 
//		//}
//
//		if( configure_state & HTML_INSTRUCTION_SET_JUMP_TO )
//		{
//			ret = add_jump_to_instruction( item_index ); 
//			if( ret != ERROR_SUCCESS )
//			{
//				//break; 
//			}
//
//			configure_state &= ~HTML_INSTRUCTION_SET_JUMP_TO ; 
//			break; 
//		}
//
//		//xpath = tree_elements.GetItemText( item_index, 1 ); 
//		//action = tree_elements.GetItemText( item_index, 2 ); 
//		//action_param  = tree_elements.GetItemText( item_index, 3 ); 
//
//		SetDlgItemText( IDC_EDIT_XPATH, xpath.c_str() ); 
//
//		//L"link"
//		if( 0 == wcsicmp( L"链接地址", action_param.c_str() ) )
//		{
//			contant_type.SetCurSel( 1 ); 
//		}
//		else if ( 0 == wcsicmp( L"文本", action_param.c_str() ) )
//		{
//			contant_type.SetCurSel( 0 ); 
//		}
//		else
//		{
//			ASSERT( FALSE ); 
//			contant_type.SetCurSel( 0 ); 
//		}
//
//		if ( 0 == wcsicmp( L"点击", action.c_str() ) )
//		{
//			element_actions.SetCurSel( 0 ); 
//			contant_type.EnableWindow( FALSE ); 
//			contant_type.ShowWindow( SW_HIDE ); 
//		}
//		else if ( 0 == wcsicmp( L"输出", action.c_str() ) )
//		{
//			element_actions.SetCurSel( 1 ); 
//			contant_type.EnableWindow( TRUE ); 
//			contant_type.ShowWindow( SW_SHOW ); 
//		}
//		else if ( 0 == wcsicmp( L"输入", action.c_str() ) )
//		{
//			ASSERT( FALSE ); 
//			element_actions.SetCurSel( 2 ); 
//			contant_type.EnableWindow( TRUE ); 
//			contant_type.ShowWindow( SW_SHOW ); 
//		}
//		else
//		{
//			ASSERT( FALSE ); 
//			element_actions.SetCurSel( 2 ); 
//			contant_type.EnableWindow( TRUE ); 
//			contant_type.ShowWindow( SW_SHOW ); 
//		}
//
//	}while( FALSE );
//}

LRESULT html_element_prop_dlg::config_action_type_from_xpath(LPCWSTR xpath)
{
    LRESULT ret = ERROR_SUCCESS;
    XPATH_PARAMS **params = NULL;
    ULONG param_count = 0;

    do
    {
        params = (XPATH_PARAMS**)malloc(sizeof(XPATH_PARAMS*) * MAX_XPATH_ELEMENT_PARAMETER_COUNT);
        if (NULL == params)
        {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


        ret = get_xpath_params(xpath,
            wcslen(xpath),
            params,
            MAX_XPATH_ELEMENT_PARAMETER_COUNT,
            &param_count);

        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        if (param_count == 0)
        {
            ret = ERROR_ERRORS_ENCOUNTERED;
            break;
        }

        XPATH_PARAM_ITERATOR param_it; 
        param_it = params[param_count - 1]->find(L"tag"); 

        if( param_it != params[param_count - 1]->end()
            && 0 == _wcsicmp(param_it->second.c_str(), L"a"))
        {
            contant_type.SetCurSel(1);
        }
        else
        {
            LRESULT _ret;
            IHTMLElement *html_element = NULL; 
            wstring _field_name;
            HTML_ELEMENT_GROUP elements; 

            contant_type.SetCurSel(1);

            ret = get_html_elements(xpath, elements, FALSE);
            if (ERROR_SUCCESS != ret)
            {
                ret = ERROR_ERRORS_ENCOUNTERED;
                break;
            }

            html_element = elements[0]; 

            if (NULL == html_element)
            {
                ret = ERROR_ERRORS_ENCOUNTERED; 
                break; 
            }

            _ret = get_html_text_field_name(html_element, element_config_base_id, _field_name);
            if (_ret != ERROR_SUCCESS)
            {
                break; 
            }

            if (_field_name.length() == 0)
            {
                get_default_field_name(_field_name);
            }

            field_name.SetWindowText(_field_name.c_str());
        }
    } while (FALSE); 

    if (NULL != params)
    {
        release_xpath_params(params, param_count);
        free(params);
        params = NULL;
    }

    return ret; 
}


LRESULT html_element_prop_dlg::get_html_elements(LPCWSTR xpath, HTML_ELEMENT_GROUP &elements, BOOLEAN analyze_xpath )
{
    LRESULT ret = ERROR_SUCCESS;
    HRESULT hr;

    IWebBrowser2Ptr web_browser;
    IDispatchPtr disp;
    IHTMLDocument2Ptr html_doc;

    CString action;
    CString action_param;

    do
    {
        html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument();
        if (NULL == html_doc)
        {
            break;
        }

        disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown();

        if (NULL == disp)
        {
            ASSERT(FALSE);
            break;
        }

        hr = disp->QueryInterface(IID_IWebBrowser2, (PVOID*)&web_browser);
        if (FAILED(hr))
        {
            break;
        }

        if (NULL == web_browser)
        {
            break;
        }

        hr = web_browser->get_Document(&disp);
        if (FAILED(hr))
        {
            break;
        }

        if (NULL == disp)
        {
            hr = E_UNEXPECTED;
            break;
        }

        hr = disp->QueryInterface(IID_IHTMLDocument2,
            (PVOID*)&html_doc);

        if (FAILED(hr))
        {
            break;
        }

        if (NULL == html_doc)
        {
            hr = E_UNEXPECTED;
            break;
        }

        ret = get_html_element_from_xpath_ex(xpath, &elements, web_browser, analyze_xpath);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }
    } while (FALSE);

    return ret;
}

LRESULT html_element_prop_dlg::get_xpath_content( LPCWSTR xpath, wstring &content, BOOLEAN analyze_xpath)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	CString action; 
	CString action_param; 
	HTML_ELEMENT_GROUP elements; 

	do 
	{
		content.clear(); 

		html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument(); 
		if( NULL == html_doc ) 
		{ 
			break; 
		} 

		disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown(); 

		if( NULL == disp )
		{
			ASSERT( FALSE ); 
			break; 
		}

		hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == web_browser )
		{
			break; 
		}

		hr = web_browser->get_Document( &disp ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == disp )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		do 
		{
			HRESULT _hr; 
			_bstr_t text; 
			LPCWSTR _text; 
			HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 
			ret = get_html_element_from_xpath_ex( xpath, &elements, web_browser, analyze_xpath); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//有一部分的ELEMENT是无法进行精确的定位的，因为没有ID，CLASS，NAME，TEXT标记

			for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = elements.begin(); it != elements.end(); it ++ )
			{
				do 
				{
					_hr = ( *it )->get_innerText( text.GetAddress() ); 

					if( FAILED(_hr))
					{
						break; 
					}

					_text = text.operator const wchar_t*(); 
					if( NULL == _text )
					{
						break; 
					}

					content += _text; 

				} while ( FALSE );
			}
		}while ( FALSE ); 
	}while( FALSE ); 
	
	release_html_element_group( &elements ); 
	return ret; 
}

LRESULT html_element_prop_dlg::get_xpath_links( LPCWSTR xpath, LINK_LIST &links, BOOLEAN analyze_xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	CString action; 
	CString action_param; 
	//HTML_ELEMENT_GROUP elements; 

	do 
	{
		html_doc = g_html_script_config_dlg->m_WebBrowser.GetDocument(); 
		if( NULL == html_doc ) 
		{ 
			break; 
		} 

		disp = g_html_script_config_dlg->m_WebBrowser.GetControlUnknown(); 

		if( NULL == disp )
		{
			ASSERT( FALSE ); 
			break; 
		}

		hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == web_browser )
		{
			break; 
		}

		hr = web_browser->get_Document( &disp ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == disp )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		do 
		{
			HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 
			//ret = get_html_element_from_xpath_ex( xpath, &elements, web_browser ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	break; 
			//}

			EXTERMAL_LINK_INFO link_info; 
			//LINK_LIST links; 

			link_info.xpath = xpath; 

			hr = scramble_external_links(html_doc, &link_info, &links, analyze_xpath ); 
			if( FAILED(hr))
			{
				dbg_print( MSG_ERROR, "get the target external links from the web page error 0x%0.8x\n", hr ); 
				break; 
			}
		}while( FALSE ); 
	}while( FALSE ); 

	//release_html_element_group( &elements ); 
	return ret; 
};

HTREEITEM html_element_prop_dlg::get_tree_hit_item()
{
	INT32 _ret; 
	POINT cursor_pos; 
	HTREEITEM selected_item; 

	_ret = GetCursorPos( &cursor_pos ); 
	if( FALSE == _ret )
	{
		ASSERT(FALSE ); 
        selected_item = NULL; 
	}
	else
	{
		tree_elements.ScreenToClient(&cursor_pos); 
		selected_item = tree_elements.HitTest( cursor_pos, NULL ); 
	}

#ifdef _DEBUG
	if( NULL != selected_item )
	{
		HTML_ELEMENT_ACTION *config; 

		config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( selected_item ); 
		if( NULL != config )
		{
			//dbg_print( MSG_IMPORTANT, "selected item %u\n", config->id ); 
		}
	}
#endif //_DEBUG

	return selected_item; 
}

LRESULT html_element_prop_dlg::edit_html_action( HTREEITEM selected_item)
{
    LRESULT ret = ERROR_SUCCESS; 

    do
    {
        HTML_ELEMENT_ACTION *config;

        if (NULL == selected_item)
        {
            //ASSERT( FALSE ); 
            break;
        }

        config = (HTML_ELEMENT_ACTION*)tree_elements.GetItemData(selected_item);
        if (NULL == config)
        {
            ASSERT(FALSE);
            break;
        }

        do
        {
            LRESULT _ret;
            wstring script_text;
            wstring std_xpath; 

            _ret = xpath2std_xpath(config->xpath, std_xpath); 
            if (_ret != ERROR_SUCCESS)
            {
                break;
            }

            _ret = action_to_scrapy(std_xpath.c_str(), script_text);
            if (_ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "%s %u error %u\n", _ret);
                break;
            }

            scrapy_script_edit.SetWindowTextW(script_text.c_str());
            scrapy_script_edit.SetSel(0, -1);

            //python_scrapy_dlg.ShowWindow(SW_SHOW); 
            //python_scrapy_dlg.insert_text(script_text.c_str()); 
        } while (FALSE);

        do
        {
            config = (HTML_ELEMENT_ACTION*)tree_elements.GetItemData(selected_item);

            if (NULL == config)
            {
                ASSERT(FALSE);
                break;
            }

            SetDlgItemText(IDC_EDIT_XPATH, config->xpath.c_str());

            if (0 == wcscmp(config->action.c_str(), HTML_ELEMENT_ACTION_NONE)
                && 0 == wcscmp(config->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_LINK))
            {
                CString link;
                link = config->title.c_str();

                g_html_script_config_dlg->locate_to_url(link.GetBuffer()); 

                break;
            }

            if (0 == wcscmp(config->action.c_str(), HTML_ELEMENT_ACTION_NONE)
                && 0 == wcscmp(config->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_TEXT))
            {
                HTML_ELEMENT_ACTION *config;
                text_output_dlg text_out;
                wstring output_data; 

                do
                {
                    config = (HTML_ELEMENT_ACTION*)tree_elements.GetItemData(selected_item);
                    if (NULL == config)
                    {
                        break;
                    }

                    cat_output_data(config, output_data); 
                    text_out.set_output_text(output_data.c_str());
                    text_out.DoModal();

                } while (FALSE);

                break;
            }

            HTML_ELEMENT_ACTION_TYPE action_type;

            if (0 == _wcsicmp(HTML_ELEMENT_ACTION_CLICK, config->action.c_str()))
            {
                action_type = HTML_ELEMENT_ACTION_TYPE_CLICK;

                if (0 == wcscmp(config->param.c_str(),
                    HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE))
                {
                    open_new_page.SetCheck(1);
                }
                else
                {
                    open_new_page.SetCheck(0);
                }

                contant_type.SetCurSel(-1);
            }
            else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_OUTPUT, config->action.c_str()))
            {
                action_type = HTML_ELEMENT_ACTION_TYPE_OUTPUT;

                do
                {
                    CWnd *ctrl;
                    ctrl = GetDlgItem(IDC_STATIC_OUTPUT_FIELD_NAME);

                    if (ctrl == NULL)
                    {
                        break;
                    }

                    ctrl->ShowWindow(SW_HIDE);
                    ctrl->EnableWindow(FALSE);
                } while (FALSE);

                //L"link"
                if (0 == _wcsicmp(HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK, config->param.c_str()))
                {
                    //可以考虑加一个LIST CONTROL对这个数据集中的所有的元素统一进行编辑
                    ASSERT(0 == config->name.length());

                    field_name.SetWindowText(L"");
                    contant_type.SetCurSel(0);
                }
                else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, config->param.c_str()))
                {
                    contant_type.SetCurSel(1);
                    field_name.SetWindowText(config->name.c_str());
                }
                else
                {
                    if (0 != _wcsicmp(HTML_ELEMENT_ACTION_NONE_PARAM_LINK, config->param.c_str())
                        && 0 != _wcsicmp(HTML_ELEMENT_ACTION_NONE_PARAM_TEXT, config->param.c_str())
                        && 0 != _wcsicmp(HTML_ELEMENT_ACTION_NONE_PARAM_TIP, config->param.c_str())
                        && 0 != _wcsicmp(HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER, config->param.c_str())
                        && 0 != config->param.length())
                    {
                        ASSERT(FALSE);
                    }

                    field_name.SetWindowText(L"");
                    contant_type.SetCurSel(1);
                }
            }
            else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_INPUT, config->action.c_str()))
            {
                wstring output_data;

                action_type = HTML_ELEMENT_ACTION_TYPE_INPUT;
                cat_output_data(config, output_data);
                input_content.SetWindowText(output_data.c_str());
            }
            else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_SCRIPT, config->action.c_str()))
            {
                action_type = HTML_ELEMENT_ACTION_TYPE_SCRIPT;

                input_content.SetWindowText(config->param.c_str());
            }
            else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_LOCATE, config->action.c_str()))
            {
                action_type = HTML_ELEMENT_ACTION_TYPE_LOCATE;
                input_content.SetWindowText(config->param.c_str());
            }
            else if (0 == _wcsicmp(HTML_ELEMENT_ACTION_NONE, config->action.c_str()))
            {
                action_type = UNKNOWN_HTML_ELEMENT_ACTION_TYPE;
            }
            else
            {
                //ASSERT( FALSE ); 
                action_type = HTML_ELEMENT_ACTION_TYPE_LOCATE;
                if (0 != _wcsicmp(HTML_ELEMENT_ACTION_NONE, config->action.c_str())
                    && 0 != _wcsicmp(HTML_ELEMENT_ACTION_JUMP, config->action.c_str())
                    && 0 != _wcsicmp(HTML_ELEMENT_ACTION_BACK, config->action.c_str())
                    && 0 != _wcsicmp(HTML_ELEMENT_ACTION_HOVER, config->action.c_str()))
                {
                    ASSERT(FALSE);
                }

                contant_type.SetCurSel(-1);
            }

            if (HTML_ELEMENT_ACTION_TYPE_LOCATE != action_type)
            {
                element_actions.SetCurSel((INT32)action_type);
            }
            else
            {
                element_actions.SetCurSel(-1);
            }

            show_html_action_param_control(action_type);
        } while (FALSE);
    } while (FALSE);
    return ret;
}

void html_element_prop_dlg::OnNMDblclkTreeElements(NMHDR *pNMHDR, LRESULT *pResult)
{
    LRESULT ret; 
    HTREEITEM selected_item; 
    *pResult = 0; 
    selected_item = get_tree_hit_item();

    ret = edit_html_action(selected_item); 
}

LRESULT html_element_prop_dlg::get_element_config_tree( HTML_ELEMENT_ACTION **root )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	HTREEITEM item; 
	//HTREEITEM sub_item; 
	//HTREEITEM first_sub_item; 
	//HTREEITEM next_sibling_item; 
	//ELEMENT_CONFIG *config;  
	//vector< HTREEITEM > sub_items; 
	//vector< HTREEITEM > items; 

	ASSERT( NULL != root ); 
	
	*root = NULL; 

	do 
	{
		item = tree_elements.GetRootItem(); 
		config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item ); 
		if( config == NULL )
		{
			break; 
		}

		*root = config; 
	}while( FALSE );

	//do 
	//{
	//	item = tree_elements.GetRootItem(); 
	//	
	//	items->push_back( item ); 

	//	for( ; ; )
	//	{
	//		item = items->pop_back(); 

	//		if( NULL == item )
	//		{
	//			break; 
	//		}

	//		first_sub_item = tree_elements.GetChildItem( item )
	//		sub_item = first_sub_item; 

	//		for( ; ; )
	//		{
	//			next_sibling_item = tree_elements.GetNextSiblingItem( sub_item ); 
	//			if( next_sibling_item == NULL )
	//			{
	//				break; 
	//			}

	//			if( next_sibling_item == first_sub_item )
	//			{
	//				break; 
	//			}

	//			do 
	//			{
	//				config = ( ELEMENT_CONFIG* )( PVOID )tree_elements.GetItemData( next_sibling_item ); 
	//				if( config == NULL )
	//				{
	//					//break; 
	//				}

	//				config_list.push_back( config ); 

	//			}while( FALSE );

	//			sub_items->push_back( next_sibling_item ); 
	//			sub_item = next_sibling_item; 
	//		}

	//		for( vector< HTREEITEM > it = sub_items->begin(); it != sub_items->end(); it ++ )
	//		{
	//			items->push_back( ( *it ) ); 
	//		}
	//	}

	//	config_list.push_back( item ); 


	//}while( FALSE );

	return ret; 
}

void html_element_prop_dlg::OnNMClickTreeElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT ret; 
	HTREEITEM selected_item; 

	*pResult = 0; 

	do 
	{
		HTML_ELEMENT_ACTION *config; 
		selected_item = get_tree_hit_item(); 
		if( NULL == selected_item )
		{
			break; 
		}

		config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( selected_item ); 
		if( NULL == config )
		{
			break; 
		}

		if( configure_state & HTML_INSTRUCTION_SET_JUMP_TO )
		{
			do 
			{
				ret = add_jump_to_instruction( jump_from_item, selected_item ); 
				if( ret == ERROR_PARAM_NO_MEANING )
				{
					break; 
				}

				configure_state &= ~HTML_INSTRUCTION_SET_JUMP_TO; 
			} while ( FALSE ); 

			break; 
		}

		{
			SetDlgItemText( IDC_RICHEDIT2_PROPERTY_TEXT, config->properties.c_str() ); 
		}
	}while( FALSE );
}

LRESULT html_element_prop_dlg::_delete_tree_item(HTREEITEM item)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 

	do 
	{
		ASSERT( NULL != item ); 

		do 
		{
			config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item ); 
			if( config == NULL )
			{
				break; 
			}

			dbg_print( MSG_IMPORTANT, "delete selected item %ws\n", config->xpath.c_str() ); 

			ret = detach_element_config( item, config ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				dbg_print( MSG_FATAL_ERROR, "detach the element configure error 0x%0.8x\n", ret ); 
				//break; 
			}

			tree_elements.SetItemData( item, (DWORD_PTR)(PVOID)NULL ); 
		}while( FALSE );

		tree_elements.DeleteItem( item ); 
        instruction_chnaged = TRUE; 

	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::delete_tree_item_siblings(HTREEITEM item)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	HTREEITEM _sibling_item; 
	HTREEITEM sibling_item; 

	do 
	{
		ASSERT( NULL != item); 

		sibling_item = item; 
        instruction_chnaged = TRUE;

		for( ; ; )
		{
			if( sibling_item == NULL )
			{
				//ASSERT ( FALSE ); 
				break; 
			}

			do
			{
				config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( sibling_item ); 
				if( config == NULL )
				{
					ASSERT( FALSE ); 
					break; 
				}

				dbg_print( MSG_IMPORTANT, "delete selected item %ws\n", config->xpath.c_str() ); 

				ret = detach_free_element_config( sibling_item, config ); 
				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
				}

				tree_elements.SetItemData( sibling_item, (DWORD_PTR)(PVOID)NULL ); 
			}while( FALSE );

			_sibling_item = tree_elements.GetNextSiblingItem(sibling_item); 
			tree_elements.DeleteItem( sibling_item ); 
			sibling_item = _sibling_item; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::delete_tree_item(HTREEITEM item)
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 

	do 
	{
		ASSERT( NULL != item); 

		config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item ); 
		if( config == NULL )
		{
			ASSERT( FALSE ); 
			break; 
		}

		dbg_print( MSG_IMPORTANT, "delete selected item %ws\n", config->xpath.c_str() ); 

		ret = detach_free_element_config( item, config ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
		}

		tree_elements.SetItemData( item, (DWORD_PTR)(PVOID)NULL ); 
		tree_elements.DeleteItem( item ); 
        instruction_chnaged = TRUE;
	}while( FALSE );

	return ret; 
}


LRESULT WINAPI config_html_element_text(HTREEITEM tree_item, HTML_ELEMENT_ACTION *src)
{
    LRESULT ret = ERROR_SUCCESS;
    do
    {
    } while (FALSE);
    return ret;
}

LRESULT WINAPI copy_html_element_action( HTML_ELEMENT_ACTION *dest, HTML_ELEMENT_ACTION *src )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != dest ); 
		ASSERT( NULL != src ); 

		dest->id = src->id; 
		dest->action = src->action; 
		dest->xpath = src->xpath; 
		dest->param = src->param; 

		dest->jump_to_id = src->jump_to_id; 

		dest->name = src->name; 
		dest->html_text = src->html_text; 
		dest->title = src->title; 
		dest->in_frame = src->in_frame; 
        dest->outputs.clear(); 
	} while ( FALSE );

	return ret; 
}

LRESULT html_element_prop_dlg::copy_tree_item( HTREEITEM item_source, HTREEITEM item_dest )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	HTML_ELEMENT_ACTION *new_config; 
	HTML_ELEMENT_ACTION *config_source; 
	LRESULT _ret; 
	wstring element_text; 
	HTREEITEM new_tree_item; 
	HTREEITEM _new_tree_item; 
	HTREEITEM container_item; 
	HTREEITEM prev_item; 
	HTREEITEM item_located; 
	map< HTREEITEM, HTREEITEM > tree_item_map; 
	map< HTREEITEM, HTREEITEM >::iterator it; 
	INT32 item_image; 
	INT32 item_selected_image; 
    ULONG new_id; 

	do 
	{
		config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item_source ); 
		if( NULL == config )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		config_source = config; 

		element_text = tree_elements.GetItemText( item_source ).GetBuffer(); 
		container_item = tree_elements.GetParentItem( item_dest ); 
		prev_item = tree_elements.GetPrevSiblingItem( item_dest ); 
		if( container_item == NULL )
		{
			container_item = TVI_ROOT; 
		}

		if( prev_item == NULL )
		{
			prev_item = TVI_FIRST; 
		}

		_ret = tree_elements.GetItemImage( item_source, item_image, item_selected_image ); 
		if( FALSE == _ret )
		{

		}

		new_tree_item = tree_elements.InsertItem( element_text.c_str(), 
			item_image, 
			item_selected_image, 
			container_item, 
			prev_item ); 

		if( NULL == new_tree_item )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_new_tree_item = new_tree_item; 
		tree_item_map.insert( pair< HTREEITEM, HTREEITEM >( item_source, new_tree_item ) ); 

		for( ; ; )
		{
			it = tree_item_map.begin(); 
			if( it == tree_item_map.end() )
			{
				break; 
			}

			item_located = tree_elements.GetChildItem( it->first ); 
			if( item_located != NULL )
			{
				do 
				{
					new_config = NULL; 
					ret = allocate_html_element_action( &new_config ); 
					if( NULL == new_config )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item_located ); 
					if( NULL == config )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					dbg_print( MSG_IMPORTANT, "copy instruction from %u\n", config->id ); 
					element_text = tree_elements.GetItemText( item_located ).GetBuffer(); 

					_ret = tree_elements.GetItemImage( item_located, item_image, item_selected_image ); 
					if( FALSE == _ret )
					{

					}

					ASSERT( it->second != NULL ); 
					new_tree_item = tree_elements.InsertItem( element_text.c_str(), 
						item_image, 
						item_selected_image, 
						it->second, 
						TVI_LAST ); 

                    if (NULL == new_tree_item)
                    {
                        ret = ERROR_ERRORS_ENCOUNTERED;
                        break;
                    }

                    new_id = new_config->id; 
					copy_html_element_action( new_config, config ); 
                    new_config->id = new_id; 

					ret = attach_element_config( new_tree_item, new_config ); 
					if( ret != ERROR_SUCCESS )
					{
						_ret = tree_elements.DeleteItem( new_tree_item ); 
						if( FALSE == _ret )
						{
							ASSERT( FALSE ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}		

						dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
						break; 
					}

					new_config = NULL; 
					tree_item_map.insert( pair< HTREEITEM, HTREEITEM>( item_located, new_tree_item ) ); 
				} while ( FALSE );

				if( NULL != new_config )
				{
					free_element_config( new_config ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}

			if( it->first != item_source )
			{
				item_located = tree_elements.GetNextSiblingItem( it->first ); 
				if( item_located != NULL )
				{
					do 
					{
						new_config = NULL; 
						ret = allocate_html_element_action( &new_config ); 
						if( NULL == new_config )
						{
							ret = ERROR_INVALID_PARAMETER; 
							break; 
						}

						config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item_located ); 
						if( NULL == config )
						{
							ret = ERROR_INVALID_PARAMETER; 
							break; 
						}

						dbg_print( MSG_IMPORTANT, "copy instruction from %u\n", config->id ); 
						element_text = tree_elements.GetItemText( item_located ).GetBuffer(); 

						container_item = tree_elements.GetParentItem( it->second ); 

                        ASSERT(NULL != container_item); 

						_ret = tree_elements.GetItemImage( item_located, item_image, item_selected_image ); 
						if( FALSE == _ret )
						{

						}

						new_tree_item = tree_elements.InsertItem( element_text.c_str(), 
							item_image, 
							item_selected_image, 
							container_item, 
                            it->second );

                        if (NULL == new_tree_item)
                        {
                            ret = ERROR_ERRORS_ENCOUNTERED; 
                            break; 
                        }

                        new_id = new_config->id; 
						copy_html_element_action( new_config, config ); 
                        new_config->id = new_id;

						ret = attach_element_config( new_tree_item, new_config ); 
						if( ret != ERROR_SUCCESS )
						{
							_ret = tree_elements.DeleteItem( new_tree_item ); 
							if( FALSE == _ret )
							{
								ASSERT( FALSE ); 
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}		

							dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
							break; 

						}

						new_config = NULL; 
						tree_item_map.insert( pair< HTREEITEM, HTREEITEM>( item_located, new_tree_item ) ); 
					} while ( FALSE ); 

					if( NULL != new_config )
					{
						free_element_config( new_config ); 
					}

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}
				}
			}

			tree_item_map.erase( it ); 
		}

		{
			do 
			{
				new_config = NULL; 
				ASSERT( NULL != config_source ); 

				ret = allocate_html_element_action( &new_config ); 
				if( NULL == new_config )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ASSERT( NULL != _new_tree_item ); 

                new_id = new_config->id;
                copy_html_element_action(new_config, config_source);
                new_config->id = new_id;

                ret = config_html_element_text(_new_tree_item, new_config); 
                if (ret != ERROR_SUCCESS)
                {
                }

				ret = attach_element_config(_new_tree_item, new_config );
				if( ret != ERROR_SUCCESS )
				{
					_ret = tree_elements.DeleteItem(_new_tree_item);
					if( FALSE == _ret )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}		

					dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
					break; 

				}

				new_config = NULL; 

				if( ERROR_SUCCESS != ret )
				{
					dbg_print( MSG_FATAL_ERROR, "attach tree item configure error %d, must restore to the original state\n", ret ); 
				}
			}while( FALSE ); 

			if( NULL != new_config )
			{
				free_element_config( new_config ); 
			}
		}

	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		_ret = delete_tree_item_siblings( _new_tree_item ); 
		if( ERROR_SUCCESS != _ret )
		{
			ASSERT( FALSE ); 
		}		

		dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::move_tree_item( HTREEITEM item_source, HTREEITEM item_dest )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *config; 
	HTML_ELEMENT_ACTION *config_source; 
	LRESULT _ret; 
	wstring element_text; 
	HTREEITEM new_tree_item; 
	HTREEITEM _new_tree_item; 
	HTREEITEM container_item; 
	HTREEITEM prev_item; 
	HTREEITEM item_located; 
	map< HTREEITEM, HTREEITEM > tree_item_map; 
	map< HTREEITEM, HTREEITEM >::iterator it; 
	INT32 item_image; 
	INT32 item_selected_image; 

	do 
	{
        ret = copy_tree_item(item_source, item_dest);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ret = delete_tree_item(item_source);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        break; 
		
        config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( item_source ); 
		if( NULL == config )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		config_source = config; 

		container_item = tree_elements.GetParentItem( item_dest ); 
        if (container_item == NULL)
        {
            container_item = TVI_ROOT;
        }

		prev_item = tree_elements.GetPrevSiblingItem( item_dest ); 

		if( prev_item == NULL )
		{
			prev_item = TVI_FIRST; 
		}

		_ret = tree_elements.GetItemImage( item_source, item_image, item_selected_image ); 
		if( FALSE == _ret )
		{
		}

        element_text = tree_elements.GetItemText(item_source).GetBuffer();

		new_tree_item = tree_elements.InsertItem( element_text.c_str(), 
			item_image, 
			item_selected_image, 
			container_item, 
			prev_item ); 

		if( NULL == new_tree_item )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_new_tree_item = new_tree_item; 
		tree_item_map.insert( pair< HTREEITEM, HTREEITEM >( item_source, new_tree_item ) ); 

		for( ; ; )
		{
			it = tree_item_map.begin(); 
			if( it == tree_item_map.end() )
			{
				break; 
			}

			item_located = tree_elements.GetChildItem( it->first ); 
			if( item_located != NULL )
			{
                do
                {
                    config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(item_located);
                    if (NULL == config)
                    {
                        ret = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    tree_elements.SetItemData(item_located, (DWORD_PTR)NULL);

                    dbg_print(MSG_IMPORTANT, "cut instruction from %u\n", config->id);
                    element_text = tree_elements.GetItemText(item_located).GetBuffer();

                    ASSERT(it->second != NULL);

                    _ret = tree_elements.GetItemImage(item_located, item_image, item_selected_image);
                    if (FALSE == _ret)
                    {
                    }

                    new_tree_item = tree_elements.InsertItem(element_text.c_str(),
                        item_image,
                        item_selected_image,
                        it->second,
                        TVI_LAST);

                    ret = set_element_config(new_tree_item, config);
                    if (ret != ERROR_SUCCESS)
                    {
                        _ret = tree_elements.DeleteItem(new_tree_item);
                        if (FALSE == _ret)
                        {
                            ASSERT(FALSE);
                            ret = ERROR_ERRORS_ENCOUNTERED;
                            break;
                        }

                        dbg_print(MSG_FATAL_ERROR, "set the tree item data error\n");
                        break;
                    }

                    config = NULL;
                    tree_item_map.insert(pair< HTREEITEM, HTREEITEM>(item_located, new_tree_item));
                } while (FALSE); 

                if (NULL != config)
                {
                    free_element_config(config); 
                    break; 
                }

                if (ret != ERROR_SUCCESS)
                {
                    break; 
                }
			}

			if( it->first != item_source )
			{
				item_located = tree_elements.GetNextSiblingItem( it->first ); 
				if( item_located != NULL )
				{
                    do
                    {
                        config = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(item_located);
                        if (NULL == config)
                        {
                            ret = ERROR_INVALID_PARAMETER;
                            break;
                        }

                        tree_elements.SetItemData(item_located, (DWORD_PTR)NULL);

                        dbg_print(MSG_IMPORTANT, "cut instruction from %u\n", config->id);
                        element_text = tree_elements.GetItemText(item_located).GetBuffer();

                        container_item = tree_elements.GetParentItem(it->second);

                        _ret = tree_elements.GetItemImage(item_located, item_image, item_selected_image);
                        if (FALSE == _ret)
                        {
                        }

                        new_tree_item = tree_elements.InsertItem(element_text.c_str(),
                            item_image,
                            item_selected_image,
                            container_item,
                            it->second);

                        ret = set_element_config(new_tree_item, config);
                        if (ret != ERROR_SUCCESS)
                        {
                            _ret = tree_elements.DeleteItem(new_tree_item);
                            if (FALSE == _ret)
                            {
                                ASSERT(FALSE);
                                ret = ERROR_ERRORS_ENCOUNTERED;
                                break;
                            }

                            dbg_print(MSG_FATAL_ERROR, "set the tree item data error\n");
                            break;
                        }

                        config = NULL;
                        tree_item_map.insert(pair< HTREEITEM, HTREEITEM>(item_located, new_tree_item));
                    } while (FALSE);

                    if (NULL != config)
                    {
                        free_element_config(config); 
                    }

                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }
                }
			}

			tree_item_map.erase( it ); 
		}

        tree_elements.SetItemData(item_source, NULL); 
		_delete_tree_item( item_source ); 

		{
			BOOLEAN lock_held = FALSE; 

			do 
			{
				ASSERT( NULL != config_source ); 

				ASSERT( NULL != _new_tree_item ); 

				EnterCriticalSection( &config_tree_lock ); 
				lock_held = TRUE; 

				do
				{
					HTREEITEM sub_item; 
					HTML_ELEMENT_ACTION *sub_config = NULL; 

					do 
					{
						sub_item = tree_elements.GetChildItem( _new_tree_item ); 
						if( NULL == sub_item )
						{
							break; 
						}

						sub_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( sub_item ); 
						if( sub_config == NULL )
						{
							break; 
						}

						if( sub_config->parent_item != config_source )
						{
							dbg_print( MSG_FATAL_ERROR, "parent item is not match %p:%p\n", 
								sub_config->parent_item, 
								config_source->parent_item ); 
							ASSERT( FALSE ); 
						}
					} while ( FALSE ); 

                    ret = attach_element_config(_new_tree_item, config_source);
                    if (ret != ERROR_SUCCESS)
                    {
                        dbg_print(MSG_FATAL_ERROR, "attach source config %p to %p error\n", _new_tree_item, config_source);
                        ret = ERROR_ERRORS_ENCOUNTERED;
                        break;
                    }

				}while( FALSE ); 

				if( ERROR_SUCCESS != ret )
				{
					dbg_print( MSG_FATAL_ERROR, "attach tree item configure error %d, must restore to the original state\n", ret ); 
				}
			}while( FALSE ); 

			if( TRUE == lock_held )
			{
				EnterCriticalSection( &config_tree_lock ); 
			}
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		_ret = delete_tree_item_siblings( _new_tree_item ); 
		if( ERROR_SUCCESS != _ret )
		{
			ASSERT( FALSE ); 
		}		

		dbg_print( MSG_FATAL_ERROR, "set the tree item data error\n" ); 
	}

	return ret; 
}

void html_element_prop_dlg::OnNMRClickTreeElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT ret; 
	CMenu menu; 
	INT32 sub_menu_item; 
	HTREEITEM selected_item; 
    POINT pt = { 0 };

	// TODO: 在此加入控制項告知處理常式程式碼
	*pResult = 0; 

    selected_item = get_tree_hit_item();

	GetCursorPos(&pt);//得到鼠标点击位置
	menu.LoadMenu(IDR_POPUP_EDIT);//菜单资源ID
	sub_menu_item = menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD |TPM_RIGHTBUTTON,pt.x,pt.y,this); //m_newListCtrl是CListCtrl对象

#ifdef _DEBUG
	do 
	{
		if( NULL == selected_item )
		{
			//ASSERT( FALSE ); 
			break; 
		}

		{
			HTML_ELEMENT_ACTION *config; 
			config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( selected_item ); 
			if( NULL != config )
			{
				dbg_print( MSG_IMPORTANT, "select action %u \n", config->id ); 
			}
		}
	}while( FALSE );
#endif //_DEBUG

	jump_from_item = NULL; 
	configure_state &= ~HTML_INSTRUCTION_SET_JUMP_TO; 

	switch( sub_menu_item )
	{
	case ID_MENU_ITEM_RUN:
		{
			ret = html_element_action_test(); 
			if( ret != ERROR_SUCCESS )
			{

			}
		}
		break; 
    case ID_MENU_ITEM_HILIGHT:
    {
        HTML_ELEMENT_ACTION *config; 

        if (NULL == selected_item)
        {
            break;
        }

        config = (HTML_ELEMENT_ACTION*)tree_elements.GetItemData(selected_item); 
        if (NULL == config)
        {
            break;
        }

        if (0 == config->xpath.length())
        {
            break; 
        }

        ret = on_hilight(0, (LPARAM)config->xpath.c_str());
        if (ret != ERROR_SUCCESS)
        {

        }
    }
        break; 
	case ID_MENU_ITEM_JUMP:
		if( NULL == selected_item )
		{
            break; 
        }

		jump_from_item = selected_item; 

		{
			HTML_ELEMENT_ACTION *config; 
			config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( jump_from_item ); 
			if( NULL != config )
			{
				dbg_print( MSG_IMPORTANT, "jump from %u\n", config->id ); 
			}
		}

		configure_state |= HTML_INSTRUCTION_SET_JUMP_TO; 
		break; 
	case ID_MENU_ITEM_DELETE: 
		do 
		{
			//selected_item = tree_elements.GetSelectedItem(); 
            if (NULL == selected_item)
            {
                break;
            }

			delete_tree_item( selected_item ); 
		}while( FALSE );
		break; 
	case ID_MENU_ITEM_CUT: 
		do 
		{
            HTREEITEM root_item;

            if (NULL == selected_item)
            {
                break;
            }

            root_item = tree_elements.GetChildItem(TVI_ROOT);
            if (root_item == NULL)
            {
                break;
            }

            if (root_item == selected_item)
            {
                MessageBox(L"根元素不可以复制");
                break;
            }

			cut_copy_item = selected_item; 
			configure_state |= HTML_INSTRUCTION_CUT; 
			configure_state &= ~( HTML_INSTRUCTION_COPY ); 
		}while( FALSE );
		break; 
	case ID_MENU_ITEM_COPY: 
		do 
		{
            HTREEITEM root_item; 
            HTML_ELEMENT_ACTION *action;

            if (NULL == selected_item)
            {
                break;
            }

            {
                action = (HTML_ELEMENT_ACTION*)(PVOID)tree_elements.GetItemData(selected_item);
                if (NULL == action)
                {
                    //break;
                }
            }

            root_item = tree_elements.GetChildItem(TVI_ROOT);
            if (root_item == NULL)
            {
                break;
            }

            if (root_item == selected_item)
            {
                MessageBox(L"根元素不可以复制"); 
                break; 
            }

			cut_copy_item = selected_item; 
			configure_state |= HTML_INSTRUCTION_COPY; 
			configure_state &= ~( HTML_INSTRUCTION_CUT ); 
		}while( FALSE );
		break; 
	case ID_MENU_ITEM_PASTE: 
		do 
		{
            HTREEITEM root_item;
            HTML_ELEMENT_ACTION *action;

			if( cut_copy_item == NULL )
			{
				MessageBox( L"先复制或剪切如下指令", NULL, 0 ); 
				break; 
			}

            if (NULL == selected_item)
            {
                break;
            }

            root_item = tree_elements.GetChildItem(TVI_ROOT);
            if (root_item == NULL)
            {
                break;
            }

            if (selected_item == root_item)
            {
                MessageBox(L"根元素不可以做为目标");
                break; 
            }

			ASSERT( ( HTML_INSTRUCTION_CUT | HTML_INSTRUCTION_COPY ) != ( configure_state & ( HTML_INSTRUCTION_CUT | HTML_INSTRUCTION_COPY ) ) ); 

			do 
			{
				if( configure_state & HTML_INSTRUCTION_COPY )
				{
					ret = copy_tree_item( cut_copy_item, selected_item ); 
					break; 
				}

				if( configure_state & HTML_INSTRUCTION_CUT )
				{
                    ret = move_tree_item(cut_copy_item, selected_item); 
                    if (ret != ERROR_SUCCESS)
                    {
                        break;
                    }

                    break; 
				}
			} while ( FALSE ); 
			cut_copy_item = NULL; 
		}while( FALSE ); 
		break; 
	case ID_MENU_ITEM_RESET:
		{
			do 
			{
				LRESULT _ret; 
				HTREEITEM next_sub_tree_item;  
				HTREEITEM tree_item; 

				tree_item = tree_elements.GetChildItem( TVI_ROOT ); 

				for( ; ; )
				{
					if( tree_item == NULL ) 
					{
						break; 
					}

					next_sub_tree_item = tree_elements.GetNextSiblingItem( tree_item ); 
					_ret = delete_tree_item( tree_item ); 
					tree_item = next_sub_tree_item; 
				}
			}while( FALSE );
		}
		break; 
	case ID_MENU_ITEM_EDIT: 
		{
        selected_item = tree_elements.GetSelectedItem();
        if (NULL == selected_item)
        {
            ASSERT(FALSE);
            break;
        }

        edit_html_action(selected_item);
        }
        break;
    default:
		//ASSERT( FALSE ); 
		break; 
	}

	//hmenu = menu.Detach();
	menu.DestroyMenu(); 
}

void html_element_prop_dlg::OnNMRDblclkTreeElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM selected_item; 
	*pResult = 0; 

	do 
	{
		HTML_ELEMENT_ACTION *config; 
		selected_item = get_tree_hit_item(); 
		if( NULL == selected_item )
		{
			break; 
		}

		config = ( HTML_ELEMENT_ACTION* )tree_elements.GetItemData( selected_item ); 
		if( NULL != config )
		{
			dbg_print( MSG_IMPORTANT, "delete selected item %ws\n", config->xpath.c_str() ); 
			detach_free_element_config( selected_item, config ); 
			tree_elements.SetItemData( selected_item, NULL ); 
		}

		tree_elements.DeleteItem( selected_item ); 
	}while( FALSE );
}

void html_element_prop_dlg::OnDestroy()
{
	set_xpath_info_ui_window( NULL ); 
	exit_html_script_worker_ex(); 

	uninit_ha_script(); 

	release_all_html_conainer_infos(); 

	uninit_sqlite(); 
	CDialog::OnDestroy(); 
	// TODO: 在此加入您的訊息處理常式程式碼
}

void html_element_prop_dlg::OnBnClickedButtonLoad()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	INT32 item_count; 
	WCHAR text[ MAX_DIGIT_LEN ]; 
	wstring url; 
	HTML_SCRIPT_INSTANCES instances; 
	HTREEITEM item; 
	HTML_ELEMENT_ACTION *action; 
	ULONG max_action_id; 

	do 
	{
		ret = load_html_elements_properties( &instances ); 
		if( ERROR_SUCCESS != ret )
		{

		}

		item = tree_elements.GetChildItem( TVI_ROOT ); 
		if( NULL == item )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			//ASSERT( FALSE ); 
			break; 
		}

		action = ( HTML_ELEMENT_ACTION*)tree_elements.GetItemData( item ); 
		ASSERT( NULL != action ); 
		//get_html_element_action_id( action, &max_action_id ); 
		//
		//element_config_base_id = max_action_id; 
		//root_item = tree_elements.GetChildItem( TVI_ROOT ); 
		//if( root_item == NULL )
		//{
		//	break; 
		//}

		//config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( root_item ); 
		//if( NULL == config )
		//{
		//	break; 
		//}

		url.clear(); 
		locate_urls.DeleteAllItems(); 

		for( HTML_SCRIPT_INSTANCES::iterator it = instances.begin(); 
			it != instances.end(); 
			it ++ )
		{
			do 
			{
				if( ( *it )->begin_url.length() == 0 )
				{
					break; 
				}

				if( url.length() == 0 )
				{
					url = ( *it )->location_url; 
					if( url.length() == 0 )
					{
						url = ( *it )->begin_url; 
					}
				}

				item_count = locate_urls.GetItemCount(); 

				locate_urls.InsertItem( item_count, ( *it )->begin_url.c_str() ); 

				if( ( *it )->location_url.length() > 0 )
				{
					LPWSTR temp_text; 

					do 
					{
						temp_text = ( LPWSTR )malloc( ( ( *it )->location_url.length() + 1 ) << 1 ); 
						if( NULL == temp_text )
						{
							break; 
						}

						memcpy( temp_text, 
							( *it )->location_url.c_str(), 
							( ( *it )->location_url.length() + 1 ) << 1  ); 

						_ret = locate_urls.SetItemData( item_count, ( DWORD_PTR )( PVOID )temp_text ); 
						if( FALSE == _ret )
						{
							break; 
						}

						temp_text = NULL; 
					}while( FALSE ); 

					if( NULL != temp_text )
					{
						free( temp_text ); 
					}
				}

				StringCchPrintfW( text, ARRAYSIZE( text ), L"%d", ( *it )->loop_count ); 
				locate_urls.SetItemText( item_count, 1, text ); 
			}while( FALSE ); 
		}

		g_html_script_config_dlg->SendMessage( WM_LOCATED_TO_URL,
			0, 
			( LPARAM )( PVOID )url.c_str() ); 

        g_html_script_config_dlg->locate_to_url(
			url.c_str() ); 

		CString file_name; 

		GetDlgItemText( IDC_EDIT_CONFIG_FILE_NAME, file_name ); 

		init_html_script_info( &script_info, file_name.GetBuffer() ); 

		if( file_name.GetLength() > 0 )
		{
			ret = set_data_scrambler_config_ex( file_name.GetBuffer() ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}
		}

	}while( FALSE ); 

	release_html_script_instances( &instances ); 

	return; 
}

LRESULT WINAPI get_html_text_field_name( IHTMLElement *html_element, 
										ULONG action_id, 
										wstring &field_name )
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 
	HRESULT hr; 
	_bstr_t text; 
	LPCWSTR temp_text; 

	do 
	{
		do 
		{
			ASSERT( html_element != NULL ); 
			html_element->get_id( text.GetAddress() ); 

			temp_text = text.operator wchar_t*(); 
			if( NULL == temp_text )
			{
				break; 
			}

			field_name = temp_text; 
			ret = ERROR_SUCCESS; 
		}while( FALSE ); 

		if( ret == ERROR_SUCCESS )
		{
			break; 
		}

		do 
		{
			_variant_t attr_value; 

			//
			//	Default. Performs a property search that is not case-sensitive, and returns an interpolated value if the property is found.
			//	1
			//	Performs a case-sensitive property search. To find a match, the uppercase and lowercase letters in strAttributeName must exactly match those in the attribute name.
			//	2
			//	Returns attribute value as a BSTR. This flag does not work for event properties.
			//	4
			//	Returns attribute value as a fully expanded URL. Only works for URL attributes.

			//#ifdef _XPATH_BY_ATTRIBUTES
			IDispatchPtr disp; 
			IHTMLDOMAttributePtr attr; 
			IHTMLDOMNodePtr element_node; 
			IHTMLAttributeCollectionPtr attrs; 
			VARIANT_BOOL specified; 

			LONG attr_count; 
			_variant_t attr_name; 
			_bstr_t _attr_name; 

			LPWSTR _name; 
			LPCWSTR _text; 
			wstring temp_text; 

			//IID iid = uuid("3051045d-98b5-11cf-bb82-00aa00bdce0b")
			hr = html_element->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&element_node); 
			if( FAILED(hr)
				|| NULL == element_node)
			{
				break; 
			}

			hr = element_node->get_attributes( &disp); 
			if( FAILED(hr)
				|| NULL == disp)
			{
				break; 
			}

			hr = disp->QueryInterface( IID_IHTMLAttributeCollection, ( PVOID* )&attrs ); 
			if( FAILED(hr)
				|| NULL == attrs)
			{
				break; 
			}

			hr =  attrs->get_length( &attr_count ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( 0 == attr_count)
			{
				break; 
			}

			do 
			{
				attr_name = L"name"; 
				hr = attrs->item( &attr_name, ( IDispatch** )&disp ); 
				if( FAILED(hr))
				{
					break; 
				}

				if( NULL == disp )
				{
					break; 
				}

				hr = disp->QueryInterface( IID_IHTMLDOMAttribute, ( PVOID* )&attr ); 
				if( FAILED(hr))
				{
					break; 
				}

				if( NULL == attr)
				{
					hr = E_FAIL; 
					break; 
				}

				hr = attr->get_specified( &specified ); 
				if( FAILED( hr ) )
				{
					dbg_print( MSG_ERROR, "get the specified attribute from the element error 0x%0.8x\n", hr ); 
					break; 
				}

				if( VARIANT_FALSE == specified)
				{
					break; 
				}

				hr = attr->get_nodeName( _attr_name.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				}

				//#endif //_XPATH_BY_ATTRIBUTES

				_text = NULL; 

				_name = _attr_name.operator wchar_t*(); 
				if( _name == NULL )
				{
					break; 
				}

				//hr = ( *it )->getAttribute( attr_name, 2, attr_value.GetAddress() ); 

				//if( hr != S_OK )
				//{
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				hr = attr->get_nodeValue( attr_value.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				}

				if( attr_value.vt != VT_BSTR )
				{
					temp_text = variant2string( &attr_value); 
					if( temp_text.length() == 0 )
					{
						break; 
					}

					_text = temp_text.c_str(); 
				}
				else
				{
					text = attr_value.bstrVal; 

					if( text.length() == 0 )
					{
						break; 
					}

					_text = text.operator wchar_t*(); 
				}

				field_name = _text; 
			} while ( FALSE ); 
		}while( FALSE ); 
	}while( FALSE );


	if( field_name.length() == 0 )
	{
		CString _field_name; 
		_field_name.Format( L"field%u", action_id ); 

		field_name = _field_name.GetBuffer(); 
	}

	return ret; 
}

LRESULT html_element_prop_dlg::get_html_element_action_info_from_ui( HTML_ELEMENT_ACTION *config )
{
	LRESULT ret = ERROR_SUCCESS; 
	//IHTMLElement *html_element; 
	//wstring element_title; 
	CString action_text; 
	CString content_type; 

	do
	{
		ASSERT( NULL != config ); 

		//html_element = ( IHTMLElement* )element; 

		//hr = get_html_element_title( html_element, &element_title ); 
		//if( FAILED(hr))
		//{
		//	//break; 
		//}

		GetDlgItemText( IDC_COMBO_CONTANT_TYPE, content_type ); 

		element_actions.GetWindowText( action_text ); 

		//config->title  = element_title; 

		//do 
		//{
		//	HRESULT hr; 
		//	wstring html_text; 
		//	IHTMLElement *html_element; 

		//	config->html_text = L"";  

		//	html_element = ( IHTMLElement* )element; 

		//	hr = get_html_dom_element_text( html_element, html_text ); 
		//	if( FAILED( hr ) )
		//	{
		//		break; 
		//	}

		//	config->html_text = html_text.c_str(); 
		//} while ( FALSE ); 

		if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_OUTPUT_UI_TEXT ) )
		{
			config->action = HTML_ELEMENT_ACTION_OUTPUT; 

			if( 0 == _wcsicmp( content_type.GetBuffer(), 
				HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_UI_TEXT ) )
			{
				CString name; 
				field_name.GetWindowText(name); 
				config->name = name.GetBuffer(); 

				if( config->name.length() == 0 )
				{
					//ASSERT( FALSE ); 
					CString _field_name; 
					_field_name.Format( L"image%u", config->id ); 

					config->name = _field_name.GetBuffer(); 
				}

				config->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT; 
			}
			else if( 0 == _wcsicmp( content_type.GetBuffer(), 
				HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_UI_TEXT ) )
			{
				config->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK; 
			}
            else if (0 == _wcsicmp(content_type.GetBuffer(),
                HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE_UI_TEXT))
            {
                CString name;
                field_name.GetWindowText(name);
                config->name = name.GetBuffer();

                if (config->name.length() == 0)
                {
                    //ASSERT( FALSE ); 
                    CString _field_name;
                    _field_name.Format(L"image%u", config->id);

                    config->name = _field_name.GetBuffer();
                }

                config->param = HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE;
            }
			else 
			{
				ASSERT( FALSE ); 
			}
		}
		else if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_INPUT_UI_TEXT ) )
		{
			CString _input_content; 
			input_content.GetWindowText(_input_content); 

			config->action = HTML_ELEMENT_ACTION_INPUT; 
			config->param = L"value"; 

            config->outputs.clear(); 
			config->outputs.push_back(wstring(_input_content.GetBuffer())); 
		}
		else if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_CLICK_UI_TEXT ) )
		{
			config->action = HTML_ELEMENT_ACTION_CLICK; 
			if( 0 != open_new_page.GetCheck() )
			{
				config->param = HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE; 
			}
			else
			{
				config->param = L""; 
			}
		}
		else if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_HOVER_UI_TEXT ) )
		{
			config->action = HTML_ELEMENT_ACTION_HOVER; 
			config->param = L""; 
		}
		else if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_SCRIPT_UI_TEXT ) )
		{
			CString _input_content; 

			config->action = HTML_ELEMENT_ACTION_SCRIPT; 
			config->param = L""; 

			input_content.GetWindowText(_input_content); 

			config->title = L"("; 
			config->title += _input_content.GetBuffer(); 
			config->title += L")";

			config->param = _input_content; 
		}
		else if( 0 == _wcsicmp( action_text.GetBuffer(), 
			HTML_ELEMENT_ACTION_LOCATE_URL_UI_TEXT ) )
		{
			CString _input_content; 

			config->action = HTML_ELEMENT_ACTION_LOCATE; 

			input_content.GetWindowText(_input_content); 
			config->param = _input_content; 
			config->title = L"("; 
			config->title += _input_content.GetBuffer(); 
			config->title += L")"; 
			break; 
		}
		//else if( 0 == wcsicmp( action_text.GetBuffer(), 
		//	HTML_ELEMENT_ACTION_JUMP ) )
		//{
		//	config->action = HTML_ELEMENT_ACTION_JUMP; 
		//}
		else 
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			//break; 
		}

		{
			CString _xpath; 
			GetDlgItemText( IDC_EDIT_TEXT_XPATH, _xpath ); 

			config->xpath = _xpath.GetBuffer(); 
		}

		//if( NULL == tree_elements.GetChildItem( TVI_ROOT ) )
		//{
		//	config->url = g_html_script_config_dlg->m_WebBrowser.get_loading_url(); 
		//}
	}while( FALSE ); 

	return ret; 
}

void html_element_prop_dlg::OnBnClickedButtonReplace()
{
	LRESULT ret; 
	INT32 _ret; 
	HTREEITEM selected_item; 
	HTML_ELEMENT_ACTION *old_config; 
	CString content_type; 
	CString action_text; 

	do 
	{
		// TODO: 在此加入控制項告知處理常式程式碼
		//_ret = GetCursorPos( &cursor_pos ); 
		//if( FALSE == _ret )
		{
			//ASSERT(FALSE ); 
			selected_item = tree_elements.GetSelectedItem(); 
		}
		//else
		//{
		//	tree_elements.ScreenToClient(&cursor_pos); 
		//	selected_item = tree_elements.HitTest( cursor_pos, NULL ); 
		//}

		if( selected_item == NULL )
		{
			break; 
		}

		old_config = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( selected_item ); 
		if( NULL == old_config )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( 0 == wcscmp( HTML_ELEMENT_ACTION_NONE, old_config->action.c_str() ) )
		{
			break; 
		}

		ret = get_html_element_action_info_from_ui( old_config ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

        instruction_chnaged = TRUE; 

		{
			wstring element_text; 
			CString _xpath; 

			element_actions.GetWindowText( action_text ); 

			GetDlgItemText( IDC_COMBO_CONTANT_TYPE, content_type ); 
			GetDlgItemText( IDC_EDIT_XPATH, _xpath ); 

			ret = get_tree_item_text( 
                HTML_ELEMENT_ACTION_UI_SHOW_ID, 
				old_config->id, 
				old_config->title.c_str(), 
				action_text.GetBuffer(), 
				_xpath.GetBuffer(), 
				content_type.GetBuffer(), 
				element_text ); 

			if( ERROR_SUCCESS != ret )
			{
			}

			_ret = tree_elements.SetItemText( selected_item, element_text.c_str() ); 
			if( FALSE == _ret )
			{
				//break; 
			}
		}
		//ret = _allocate_element_config( &config ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		//config->id = old_config->id; 
		//config->html_text = old_config->html_text; 
		//config->action = old_config->action; 
		//config->in_frame = old_config->in_frame; 
		//config->jump_to_id = old_config->jump_to_id; 
		//config->next_item = old_config->next_item; 
		//config->output = old_config->output; 
		//config->param = old_config->param; 
		//config->parent_item = old_config->parent_item; 

	}while( FALSE );

}

#include "data_output_config_dlg.h"
void html_element_prop_dlg::OnBnClickedButtonOutputMethodConfirm()
{
	//LRESULT ret; 

#if 0
	do 
	{
		ret = uninit_data_store( &data_store ); 
		if( ret != ERROR_SUCCESS )
		{

		}

		ret = create_data_store( &data_store_param, &data_store ); 
		if( ret != ERROR_SUCCESS )
		{
			MessageBox( L"创建输出对象失败, 请验证连接参数。" ); 
			break; 
		}

		data_store.table_name = data_store_param.sql_table_name; 

		ret = create_mysql_table( &data_store, 
			&scrambled_page_content ); 

		if( ret != ERROR_SUCCESS )
		{

		}

		data_dlg.set_page_contents( &scrambled_page_content ); 
		data_dlg.ShowWindow( SW_SHOW ); 

		break; 

		//lock_cs( target_scramble_info_lock ); 
		//target_scramble_info->url = m_sURL.GetBuffer(); 
		////target_scramble_info->next_link.push_back( original_url ); 
		//unlock_cs( target_scramble_info_lock ); 

#define TIMER_SCRAMBLE_CONTENT_ID 1002
		SetTimer( TIMER_SCRAMBLE_CONTENT_ID, 3000, NULL ); 
	}while( FALSE );

#endif //0
}

LRESULT html_element_prop_dlg::end_data_scramble()
{
	LRESULT ret = ERROR_SUCCESS; 

	//uninit_data_store( &data_store ); 
	//_send_data_file( &data_store_param ); 
	return ret; 
}

void html_element_prop_dlg::on_exit() 
{
	end_data_scramble(); 
	//post_scrambled_data(); 
}

void html_element_prop_dlg::OnBeginUrlslabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnBeginlabeleditList( pNMHDR, IDC_LIST_HTML_LOCATE_URL, pResult ); 
}

void html_element_prop_dlg::OnEndUrllabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnEndlabeleditList( pNMHDR, IDC_LIST_HTML_LOCATE_URL, pResult ); 
}

void html_element_prop_dlg::OnBeginlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CRect  rect( 0,0,1,1 );
	DWORD dwStyle = ES_LEFT;
	CString str = pDispInfo->item.pszText;
	int item = pDispInfo->item.iItem;
	int subitem = pDispInfo->item.iSubItem;
	
	do 
	{
		switch( control_id )
		{
		case IDC_LIST_HTML_LOCATE_URL:
		case IDC_LIST_DATA_SET:

			// Construct and create the custom multiline edit control.
			// We could just as well have used a combobox, checkbox, 
			// rich text control, etc.
			m_pListEdit = new CInPlaceEdit( item, subitem, str );
			if( NULL == m_pListEdit )
			{
				break; 
			}

			// Start with a small rectangle.  We'll change it later.
			dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL;
			m_pListEdit->Create( dwStyle, rect, &locate_urls, 103 ); 

			if( control_id == IDC_LIST_HTML_LOCATE_URL )
			{
				// Have the Grid position and size the custom edit control
				locate_urls.PositionControl( m_pListEdit, item, subitem );
			}
			else 
			{
				data_set.PositionControl( m_pListEdit, item, subitem );
			}

			// Have the edit box size itself to its content.
			m_pListEdit->CalculateSize();
			// Return TRUE so that the list control will hnadle NOT edit label itself. 
			*pResult = 1;
			break; 
		default:
			break; 
		}
	}while( FALSE );

}

void html_element_prop_dlg::OnEndlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	INT32 item = pDispInfo->item.iItem;
	INT32 subitem = pDispInfo->item.iSubItem; 

	do 
	{
		// This is coming from the grid list control notification.
		if( NULL == m_pListEdit )
		{
			break; 
		}

		if( item < 0 )
		{
			break; 
		}

		if( pDispInfo->item.pszText )
		{
			switch( control_id )
			{
			case IDC_LIST_HTML_LOCATE_URL:
			case IDC_LIST_DATA_SET:
				if( IDC_LIST_HTML_LOCATE_URL == control_id )
				{
					locate_urls.SetItemText( item, subitem, pDispInfo->item.pszText ); 
					//do 
					//{
					//	HTML_SCRIPT_INSTANCE *instance; 
					//	instance = ( HTML_SCRIPT_INSTANCE* )( PVOID )locate_urls.GetItemData( item ); 

					//	if( NULL == instance )
					//	{
					//		break; 
					//	}

					//	switch( pDispInfo->item.iSubItem )
					//	{
					//	case 0:
					//		instance->begin_url = pDispInfo->item.pszText; 
					//		break; 
					//	case 1:
					//		instance->loop_count = wcstol( pDispInfo->item.pszText, &temp_text, 0 ); 
					//		break; 
					//	default:
					//		break; 
					//	}
					//}while( FALSE ); 
				}
				else
				{
					data_set.SetItemText( item, subitem, pDispInfo->item.pszText ); 
				}
				break; 
			}
		}

		delete m_pListEdit;
		m_pListEdit = NULL;
	}while( FALSE );

	*pResult = 0;
}

LRESULT WINAPI init_html_script_info( HTML_SCRIPT_INFO *info, 
									 LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != info ); 
		info->valid = FALSE; 
		info->share = FALSE; 
		info->shared = FALSE; 
		info->get_data_time = 0; 
		info->get_link_time = 0; 
		if( NULL == file_name 
			|| *file_name == L'\0' )
		{
			info->file_name.clear(); 
		}
		else
		{
            {
                ULONG cc_ret_len;
                WCHAR __file_name[MAX_PATH];
                ret = get_file_absolute_path(file_name, __file_name, ARRAYSIZE(__file_name), &cc_ret_len);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                info->file_name = __file_name;
            }
		}

	}while( FALSE ); 
	return ret; 
}

LRESULT WINAPI share_html_script( HTML_SCRIPT_INFO *info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != info ); 
		ASSERT( info->file_name.length() > 0 ); 

		if( info->share == FALSE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( info->shared == TRUE )
		{
			break; 
		}

		if( info->get_link_time > 1 
			&& info->get_data_time > 1 )
		{
			ret = post_script_file( info->file_name.c_str() ); 
			if( ret == ERROR_SUCCESS )
			{
				info->shared = TRUE; 
			}
		}
		else
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI increment_script_run_time( HTML_SCRIPT_INFO *info, BOOLEAN data, BOOLEAN links )
{
	LRESULT ret = ERROR_SUCCESS; 
	do 
	{
		ASSERT( NULL != info ); 
		if( data == TRUE )
		{
			info->get_data_time += 1; 
		}

		if( links == TRUE )
		{
			info->get_link_time += 1; 
		}

		ret = share_html_script( info ); 
	} while (FALSE ); 

	return ret; 
}

LRESULT WINAPI set_html_script_info( HTML_SCRIPT_INFO *info, 
									BOOLEAN share )
{
	info->share = share; 
	return ERROR_SUCCESS; 
}

LRESULT html_element_prop_dlg::data_output_config( LPCWSTR name, CONTAINER_INFOS *container_infos )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT_PTR _ret; 
	data_output_config_dlg dlg; 
	//BOOLEAN to_data_center; 

	do
	{
		CString url; 
		CString data_file_name; 

		ASSERT( NULL != name ); 
        ASSERT( *name != L'\0' ); 
		ASSERT( NULL != container_infos ); 

		release_all_html_page_data_store(); 

		dlg.set_html_pages_layout( container_infos ); 

		data_file_name = name; 
		data_file_name += L".csv"; 

		for( CONTAINER_INFOS::iterator it = container_infos->begin(); 
			it != container_infos->end(); 
			it ++ )
		{
			do 
			{
				_ret = init_default_data_store_settings( &( *it )->store_param, 
					data_file_name.GetBuffer() ); 

				if( ERROR_SUCCESS != _ret )
				{
					dbg_print( MSG_FATAL_ERROR, "initialize the data store settings error %x\n", _ret ); 
					break; 
				}
			}while( FALSE ); 
		}

		_ret = dlg.DoModal(); 
		if( _ret == IDCANCEL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}


		BOOLEAN share_script; 
		dlg.get_output_config( &share_script ); 
		if( share_script == TRUE )
		{
			set_html_script_info( &script_info, share_script ); 
		}
	}while( FALSE ); 
	return ret; 
}

//HTML_ELEMENT_ACTIONS actions; 
//HTML_ELEMENT_ACTION *action; 
//
//if( NULL == selected_item )
//{
//	selected_item = tree_elements.GetSelectedItem(); 
//	if( NULL == selected_item )
//	{
//		ASSERT( FALSE ); 
//		break; 
//	}
//}
//
//action = ( HTML_ELEMENT_ACTION* )( PVOID )tree_elements.GetItemData( selected_item ); 
//ASSERT( NULL != action ); 
//if( ERROR_SUCCESS != check_action_container( action ) )
//{
//	break; 
//}
//
//ret = get_container_content_info( action, NULL, &actions ); 
//if( ret != ERROR_SUCCESS )
//{
//	break; 
//}
//
//LONG item_count; 
//LONG item_index; 
//
//data_set.EnableWindow( TRUE ); 
//data_set.ShowWindow( SW_SHOW ); 
//data_set.DeleteAllItems(); 
//
//for( HTML_ELEMENT_ACTIONS::iterator it = actions.begin(); 
//	it != actions.end(); 
//	it ++ )
//{
//	item_count = data_set.GetItemCount(); 
//
//	item_index = data_set.InsertItem( item_count, ( *it )->name.c_str() ); 
//
//	if( item_index < 0 )
//	{
//		break; 
//	}
//
//	data_set.SetItemText( item_index, 1, ( *it )->xpath.c_str() ); 
//	data_set.SetItemData( item_index, ( DWORD_PTR )( PVOID )( *it ) ); 
//}
void html_element_prop_dlg::OnBnClickedButtonOutputConfig()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	LRESULT ret; 
	wstring domain_name; 
	CString url;  

	url = g_html_script_config_dlg->m_WebBrowser.GetLocationURL(); 
	ret = get_domain_name_in_url( url.GetBuffer(), domain_name ); 
	if( ERROR_SUCCESS != ret )
	{
		ASSERT( FALSE ); 
		domain_name = L"scrambled"; 
	}

	//ret = data_output_config( domain_name.c_str() ); 
}

void html_element_prop_dlg::OnTRBNThumbPosChangingSliderWebPageLoadTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 此功能需要 Windows Vista (含) 以上版本。
	// 符號 _WIN32_WINNT 必須 >= 0x0600。
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	// TODO: 在此加入控制項告知處理常式程式碼
	*pResult = 0;
}

void html_element_prop_dlg::OnTRBNThumbPosChangingSliderProcessDelayTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 此功能需要 Windows Vista (含) 以上版本。
	// 符號 _WIN32_WINNT 必須 >= 0x0600。
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	// TODO: 在此加入控制項告知處理常式程式碼
	*pResult = 0;
}

void html_element_prop_dlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值

	if( pScrollBar == ( CScrollBar* )&web_page_load_time )
	{
		INT32 _time; 
		_time = web_page_load_time.GetPos(); 
		_set_html_action_global_config( INVALID_TIME_VALUE, ( UINT )_time, INVALID_TIME_VALUE ); 
	}
	else if( pScrollBar == ( CScrollBar* )&process_delay_time )
	{
		INT32 _time; 
		_time = process_delay_time.GetPos(); 
		_set_html_action_global_config( INVALID_TIME_VALUE, INVALID_TIME_VALUE, ( UINT )_time ); 
	}
	else
	{
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar); 
	}
}

LRESULT html_element_prop_dlg::get_html_script_locate_url_by_user( wstring &url )
{
	LRESULT ret = ERROR_SUCCESS; 
	locate_url_config_dlg dlg; 
	INT32 _ret; 
	wstring _url; 

	do 
	{
		dlg.set_locate_url( url.c_str() ); 
		_ret = dlg.DoModal();  
		if( IDCANCEL == _ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		dlg.get_locate_url( _url ); 
	
		if( 0 == _url.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		url = _url; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI release_html_script_instances( vector<HTML_SCRIPT_INSTANCE*> *instances )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != instances ); 
		for( vector< HTML_SCRIPT_INSTANCE*>::iterator it = instances->begin(); 
			it != instances->end(); 
			it ++ )
		{
			delete ( *it ); 
		}
	}while( FALSE );

	instances->clear(); 
	return ret; 
}

LRESULT html_element_prop_dlg::get_html_script_instance( INT32 i, HTML_SCRIPT_INSTANCE *instance )
{
	LRESULT ret = ERROR_SUCCESS; 
	CString text; 
	LPWSTR temp_text; 

	do
	{
		text = locate_urls.GetItemText( i, 0 ); 
		if( text.GetLength() == 0 )
		{
			break; 
		}

		instance->location_url = text.GetBuffer(); 

		text = locate_urls.GetItemText( i, 1 ); 
		if( text.GetLength() == 0 )
		{
			instance->loop_count = -1; 
		}
		else
		{
			instance->loop_count = wcstol( text.GetBuffer(), 
				&temp_text, 
				0 ); 

			if( instance->loop_count == 0 )
			{
				instance->loop_count = -1; 
			}
		}

		do 
		{
			temp_text = ( LPWSTR )( PVOID )locate_urls.GetItemData( i ); 
			if( NULL == temp_text )
			{
				instance->begin_url = instance->location_url; 
				break; 
			}

			if( *temp_text != L'\0' )
			{
				instance->begin_url = temp_text; 
				if( instance->location_url.length() == 0 )
				{
					ASSERT( FALSE ); 
					instance->location_url = instance->begin_url; 
				}
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}while( FALSE );
	}while( FALSE ); 

	return ret; 
}

LRESULT html_element_prop_dlg::get_html_script_instances(vector<HTML_SCRIPT_INSTANCE*> *instances)
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 _ret; 
	HTML_SCRIPT_INSTANCE* instance; 
	CString text; 
	INT32 locate_url_count; 
	INT32 i; 

	ASSERT( NULL != instances ); 

	instance = NULL; 
	locate_url_count = locate_urls.GetItemCount(); 
	for( i = 0; i < locate_url_count; i ++ )
	{
		do
		{
			instance = new HTML_SCRIPT_INSTANCE(); 
			if( NULL == instance )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			ret = get_html_script_instance( i, instance ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			instances->push_back( instance ); 
			instance = NULL; 
		}while( FALSE ); 

		if( instance != NULL )
		{
			delete instance; 
			instance = NULL; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_file_absolute_path(LPCWSTR file_name, LPWSTR file_path, ULONG cc_buf_len, ULONG *cc_ret_len)
{
    LRESULT ret = ERROR_SUCCESS; 
    WCHAR app_path[MAX_PATH];
    INT32 _ret;
    ULONG _cc_ret_len; 
    HRESULT hr; 

    do 
    {
        ASSERT(file_name != NULL ); 
        ASSERT(file_path != NULL); 
        ASSERT(NULL != cc_ret_len); 

        *cc_ret_len = 0; 

        ret = get_app_path(app_path, ARRAYSIZE(app_path), &_cc_ret_len);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        _ret = SetCurrentDirectoryW(app_path);
        if (FALSE == _ret)
        {

        }

        hr = StringCchCatW(app_path, ARRAYSIZE(app_path), CONFIG_FILE_DIRECTORY);
        if (S_OK != hr)
        {
            break;
        }

        ret = create_directory_ex(app_path, wcslen(app_path), 2);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        if (file_name[0] == L'.'
            && file_name[1] == L'\\')
        {
            ret = get_app_path(app_path, ARRAYSIZE(app_path), &_cc_ret_len);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }

            hr = StringCchCatW(app_path, ARRAYSIZE(app_path), file_name);
            if (FAILED(hr))
            {
                ret = ERROR_ERRORS_ENCOUNTERED;
                break;
            }

            hr = StringCchCopyW(file_path, cc_buf_len, app_path);
            if (FAILED(hr))
            {
                ret = ERROR_ERRORS_ENCOUNTERED; 
                break;
            }

            *cc_ret_len = wcslen(file_path);
        }
        else if (file_name[1] == L':')
        {
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER; 
            break; 
        }
    } while (FALSE ); 
    
    return ret; 
}

LRESULT html_element_prop_dlg::save_html_script_ex( ULONG flags )
{
    LRESULT ret;
    vector<HTML_SCRIPT_INSTANCE*> instances;
    WCHAR file_name[MAX_PATH];
    wstring url;

    do
    {
        GetDlgItemText(IDC_EDIT_CONFIG_FILE_NAME, file_name, ARRAYSIZE(file_name));
        
        do
        {
            ret = get_html_script_instances(&instances);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }

            if (0 == instances.size())
            {
                HTML_SCRIPT_INSTANCE *instance = NULL;

                url = g_html_script_config_dlg->m_WebBrowser.get_loading_url();

                ret = get_html_script_locate_url_by_user(url);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                do
                {
                    instance = new HTML_SCRIPT_INSTANCE();
                    if (NULL == instance)
                    {
                        ret = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    instance->begin_url = url;
                    instance->loop_count = -1;

                    instances.push_back(instance);
                    instance = NULL;
                } while (FALSE);
                if (NULL != instance)
                {
                    delete instance;
                }
            }
            else
            {
                ret = get_web_browser_location_url(&g_html_script_config_dlg->m_WebBrowser,
                    url);

                if (ERROR_SUCCESS != ret)
                {

                }

                if (url.length() > 0
                    && 0 != wcscmp(instances[0]->begin_url.c_str(),
                        url.c_str()))
                {
                    wstring domain_name; 
                    wstring current_domain_name; 

                    get_domain_name_in_url(url.c_str(), domain_name); 
                    get_domain_name_in_url(instances[0]->begin_url.c_str(), current_domain_name);

                    if (0 != wcscmp(domain_name.c_str(),
                        current_domain_name.c_str()))

                    if (MessageBox(L"当前浏览网页链接与起始网页链接不同,是否更新起始网页链接?", L"", MB_YESNO) == IDYES)
                    {
                        if (0 == locate_urls.GetItemCount())
                        {
                            locate_urls.InsertItem(0, url.c_str());
                            locate_urls.SetItemText(0, 1, L"-1");
                        }
                        else
                        {
                            locate_urls.SetItemText(0, 0, url.c_str());
                            locate_urls.SetItemText(0, 1, L"-1");
                        }

                        {
                            LPWSTR _locate_url;

                            do
                            {

                                _locate_url = (LPWSTR)locate_urls.GetItemData(0);
                                if (NULL != _locate_url)
                                {
                                    free(_locate_url);
                                }

                                _locate_url = (LPWSTR)malloc((url.size() + 1) << 1);
                                if (NULL == _locate_url)
                                {
                                    break;
                                }

                                memcpy(_locate_url,
                                    url.c_str(),
                                    ( ( url.size() +  1 ) << 1));

                                locate_urls.SetItemData(0, (DWORD_PTR)_locate_url);

                            } while (FALSE);
                        }

                        instances[0]->begin_url = url;
                        instances[0]->location_url = url;
                    }
                }
            }

            {
                ULONG cc_ret_len;
                WCHAR __file_name[ MAX_PATH ]; 
                ret = get_file_absolute_path(file_name, __file_name, ARRAYSIZE(__file_name), &cc_ret_len);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                ret = save_html_script(__file_name, &instances);
            }

            if (ret != ERROR_SUCCESS)
            {
                ASSERT(FALSE);
                dbg_print(MSG_FATAL_ERROR | DBG_MSG_AND_ERROR_OUT | DBG_MSG_BOX_OUT, "save the action for html elements error 0x%0.8x\n", ret);
                //MessageBox( L"保存指令列失败" ); 
            }
            else
            {
                CString _file_name;
                GetDlgItemText(IDC_EDIT_CONFIG_FILE_NAME, _file_name);

                if (_file_name.GetLength() > 0)
                {
                    ret = set_data_scrambler_config_ex(_file_name.GetBuffer());
                    if (ret != ERROR_SUCCESS)
                    {
                        //break; 
                    }
                }

                if (flags == SHOW_UI)
                {
                    MessageBox(L"保存指令列成功");
                }
                else
                {
                    SetDlgItemTextW( IDC_STATIC_SAVE_FILE_STATE, L"保存指令列成功");
                    SetTimer(CLEAN_SAVE_STATUS_TIMER, CLEAN_SAVE_STATUS_TIMER_ELAPSE, NULL);
                }
            }
        } while (FALSE);

        release_html_script_instances(&instances);
    } while (FALSE);
    return ret;
}

void html_element_prop_dlg::OnBnClickedButtonSaveHtmlScript()
{
    save_html_script_ex(SHOW_UI); 
}

void html_element_prop_dlg::OnBnClickedButtonDataProcess()
{

#define MAX_CMD_LINE 1024
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR module_file_name[ MAX_PATH ]; 
	WCHAR cmd_line[ MAX_CMD_LINE ]; 
	HRESULT hr; 
	ULONG cc_ret_len; 

	do
	{
		ret = get_app_path( module_file_name, ARRAYSIZE( module_file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

#define DATA_PROCESS_IMGE_FILE_NAME L"data_process.exe"

		hr = StringCchCatW( module_file_name, ARRAYSIZE( module_file_name ), DATA_PROCESS_IMGE_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		*cmd_line = L'\"'; 
		cmd_line[ 1 ] = L'\0'; 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), module_file_name ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#if 0
		#define QUOTE_SIGN L"\"" 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L" " ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), QUOTE_SIGN ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), 
			data_store_param.output_path ); 
		
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
#endif //0
		ret = run_proc( cmd_line, NULL ); 
	}while( FALSE ); 
}

//sudo sed -i "s|EXTRA_ARGS='|EXTRA_ARGS='--registry-mirror=http://fecbba41.m.daocloud.io |g" /var/lib/boot2docker/profile  
void html_element_prop_dlg::OnBnClickedButtonDataLearning()
{
#define MAX_CMD_LINE 1024
	LRESULT ret = ERROR_SUCCESS;  
	WCHAR module_file_name[ MAX_PATH ]; 
	WCHAR cmd_line[ MAX_CMD_LINE ]; 
	HRESULT hr; 
	ULONG cc_ret_len; 

	do
	{
		ret = get_app_path( module_file_name, ARRAYSIZE( module_file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

#define DATA_LEARNING_IMGE_FILE_NAME L"data_learning.exe"

		hr = StringCchCatW( module_file_name, ARRAYSIZE( module_file_name ), DATA_LEARNING_IMGE_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		*cmd_line = L'\"'; 
		cmd_line[ 1 ] = L'\0'; 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), module_file_name ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#define QUOTE_SIGN L"\"" 

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), L" " ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#if 0
		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), QUOTE_SIGN ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = StringCchCatW( cmd_line, ARRAYSIZE( cmd_line ), 
			data_store_param.output_path ); 

		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
#endif //0

		ret = run_proc( cmd_line, NULL ); 
	}while( FALSE ); 
}


BOOL SetPrivilege(
				  HANDLE hToken,          // access token handle
				  LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
				  BOOL bEnablePrivilege   // to enable or disable privilege
				  ) ;

#include <PowrProf.h>
LRESULT WINAPI suspend_system()
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE hToken;
	INT32 _ret; 
	do 
	{
		OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken);

		_ret = SetPrivilege( hToken, SE_SHUTDOWN_NAME, TRUE );

		//printf("bRet=SetPrivilege(\n  hToken==0x%X,\n  lpszPrivilege==SE_SHUTDOWN_NAME,\n  bEnablePrivilege==TRUE\n);\nbRet==%d;\n",hToken,dRet);
		//printf("\n");

		_ret = SetSuspendState( TRUE, TRUE, TRUE ); 
		if( _ret == FALSE )
		{
			break; 
		}
		//printf("bRet=SetSuspendState(\n  Hibernate==%d,\n  ForceCritical==%d,\n  DisableWakeEvent==%d\n);\nbRet==%d;",b1,b2,b3,bRet);
	} while ( FALSE );

	return ret;
}


BOOL SetPrivilege(
				  HANDLE hToken,          // access token handle
				  LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
				  BOOL bEnablePrivilege   // to enable or disable privilege
				  ) 
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if ( !LookupPrivilegeValue( 
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid ) )        // receives LUID of privilege
	{
		//printf("LookupPrivilegeValue error: %u\n", GetLastError() ); 
		return FALSE; 
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if ( !AdjustTokenPrivileges(
		hToken, 
		FALSE, 
		&tp, 
		sizeof(TOKEN_PRIVILEGES), 
		(PTOKEN_PRIVILEGES) NULL, 
		(PDWORD) NULL) )
	{ 
		//printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
		return FALSE; 
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		//printf("The token does not have the specified privilege. \n");
		return FALSE;
	} 

	return TRUE;
}

ULONG shutdown_delay_time = 0xffffffff; 
ULONG CALLBACK exit_scirpt_work_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG delay_time = 0; 
	ULONG original_delay_time = 0; 
	HANDLE work_event; 
	ULONG wait_ret; 

	do 
	{
		if( param == NULL )
		{
			ASSERT( FALSE ); 
			break; 
		}

		for( ; ; )
		{
			work_event = ( HANDLE )param; 

			wait_ret = WaitForSingleObject( work_event, INFINITE ); 
			if( wait_ret != WAIT_OBJECT_0 
				&& wait_ret != WAIT_ABANDONED_0 
				&& wait_ret != WAIT_FAILED )
			{
				break; 
			}

			ASSERT( shutdown_delay_time != 0xffffffff); 

			for( ; ; )
			{
				if( shutdown_delay_time == 0xffffffff )
				{
					break; 
				}

				if( original_delay_time != shutdown_delay_time )
				{
					original_delay_time = shutdown_delay_time; 
					delay_time = original_delay_time; 
				}

				if( delay_time == 0 )
				{
					suspend_system(); 
					break; 
				}

				delay_time -= 1; 

				Sleep( 1000 ); 
				if( NULL != g_html_element_prop_dlg)
				{
					g_html_element_prop_dlg->SendMessage( WM_UPDATE_EXIT_WORK_TIME_DELAY, 0, delay_time ); 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

#pragma comment( lib, "PowrProf.lib" ) 

void html_element_prop_dlg::OnBnClickedButtonExitSystem()
{
	CString text; 
	ULONG seconds; 
	LPWSTR end_text; 
	// TODO: 在此加入控制項告知處理常式程式碼

	do 
	{
		if( exit_script_work_thread_handle == NULL )
		{
			ASSERT( FALSE ); 
			break; 
		}

		GetDlgItemText( IDC_EDIT_SHUTDOWN_SYSTEM_DELAY, text ); 

		if( 0 == text.GetLength() )
		{
			break; 
		}

		seconds = wcstoul( text.GetBuffer(), &end_text, 0 ); 

#define MAX_SHUTDOWN_SYSTEM_DELAY_TIME ( ULONG )( 3600 * 24 )
		if( seconds > MAX_SHUTDOWN_SYSTEM_DELAY_TIME )
		{
			seconds = MAX_SHUTDOWN_SYSTEM_DELAY_TIME; 
		}

		shutdown_delay_time = seconds; 
		if( FALSE == SetEvent( exist_work_event ) )
		{
			ASSERT( FALSE ); 
		}

	} while ( FALSE );
}

void html_element_prop_dlg::OnBnClickedButtonCancelExitSystem()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	do 
	{
		if( exit_script_work_thread_handle == NULL )
		{
			ASSERT( FALSE ); 
			break; 
		}

		shutdown_delay_time = 0xffffffff;  
		//if( FALSE == SetEvent( exist_work_event ) )
		//{
		//	ASSERT( FALSE ); 
		//}
	} while ( FALSE );

}

LRESULT WINAPI init_script_data_stores( HTML_ELEMENT_ACTION *root )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		/**************************************************
		分析记录数据的所有的页面的结构，针对每一种页面结构，
		都建立一个数据IO结构
		**************************************************/
	}while( FALSE );

	return ret; 
}
void html_element_prop_dlg::OnNMDblclkListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	//INT32 item_counmt; 
	//wstring url; 
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	//*pResult = 0;

	//if( pNMItemActivate->iItem < 0 )
	//{
	//	item_counmt = locate_urls.GetItemCount(); 

	//	url = g_html_script_config_dlg->m_WebBrowser.get_loading_url(); 
	//	locate_urls.InsertItem( item_counmt, url.c_str() ); 
	//}
	//else
	//{
	//}
}

void html_element_prop_dlg::OnLvnDeleteitemListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	PVOID data; 

	*pResult = 0;

	do 
	{
		if( pNMLV->iItem < 0 )
		{
			break; 
		}

		data = ( PVOID )locate_urls.GetItemData( pNMLV->iItem ); 
		if( NULL == data )
		{
			break; 
		}

		free( data ); 

		locate_urls.SetItemData( pNMLV->iItem, NULL ); 
	}while( FALSE ); 
}

void html_element_prop_dlg::OnNMRClickListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT ret; 
	INT32 _ret; 
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CMenu menu; 
	INT32 sub_menu_item; 
	POINT pt = {0};
	INT32 item_index; 

	*pResult = 0;

	do 
	{
		_ret = menu.LoadMenu(IDR_POPUP_INSTANCE_EDIT); 
		if( FALSE == _ret )
		{
			break; 
		}

		GetCursorPos( &pt ); 
		sub_menu_item = menu.GetSubMenu(0)->TrackPopupMenu( TPM_LEFTALIGN|TPM_RETURNCMD |TPM_RIGHTBUTTON, pt.x, pt.y, this ); //m_newListCtrl是CListCtrl对象

		switch( sub_menu_item )
		{
		case ID_MENU_ITEM_UPDATE_URL:
			{
				wstring url; 

				ret = get_web_browser_location_url( &g_html_script_config_dlg->m_WebBrowser, 
					url ); 

				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				if( 0 == locate_urls.GetItemCount() )
				{
					item_index = locate_urls.InsertItem( 0, url.c_str() ); 
					if( 0 > item_index )
					{
						break; 
					}
				}
				else
				{
					locate_urls.SetItemText( 0, 0, url.c_str() ); 
				}

				{
					PVOID old_data; 
					old_data = ( PVOID )locate_urls.GetItemData( 0 ); 
					if( NULL != old_data )
					{
						free( old_data ); 
					}
				}
				
				locate_urls.SetItemData( 0, ( DWORD_PTR )NULL ); 
			}
			break; 
		default:
			ASSERT( FALSE ); 
			break; 
		}
		
	}while( FALSE );
}

#include "plugin_manage_dlg.h"
void html_element_prop_dlg::OnBnClickedButtonPlugins()
{
	plugin_manage_dlg dlg; 

	dlg.DoModal(); 
}


#include "human_help_dlg.h"
void html_element_prop_dlg::OnBnClickedButtonHelp()
{
    human_help_dlg dlg; 
    dlg.DoModal(); 
    // TODO: 在此添加控件通知处理程序代码
}


void html_element_prop_dlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if( nIDEvent == AUTO_SAVE_SCRIPT_TIMER )
    { 
        if (FALSE != instruction_chnaged)
        {
            save_html_script_ex(0);
        }
    }
    else if (nIDEvent == CLEAN_SAVE_STATUS_TIMER)
    {
        SetDlgItemTextW(IDC_STATIC_SAVE_FILE_STATE, L""); 
    }
    __super::OnTimer(nIDEvent);
}


void html_element_prop_dlg::OnBnClickedButtonXpathClean()
{
    // TODO: 在此添加控件通知处理程序代码
    SetDlgItemTextW(IDC_EDIT_XPATH, L"HTML|BODY|"); 
}


void html_element_prop_dlg::OnBnClickedButtonWebPageLayout()
{
    // TODO: 在此添加控件通知处理程序代码
}
