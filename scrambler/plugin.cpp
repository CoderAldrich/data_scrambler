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
#include "data_scrambler.h"
#include "data_list_dlg.h"
#import "msxml.tlb"
#include "plugin.h"

//#define DYNAMIC_RELEASE_PLUGINS 1

DATA_PROCESS_PLUGINS modules_info; 

LRESULT WINAPI get_plugins( DATA_PROCESS_PLUGINS **modules )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != modules ); 
		*modules = &modules_info; 
	
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI init_plugin( DATA_PROCESS_PLUGIN *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != info ); 
		info->module = NULL; 
		info->data_handler = NULL; 
		info->path.clear(); 

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI uninit_plugin( DATA_PROCESS_PLUGIN *info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != info ); 
		if(info->module != NULL )
		{
			FreeLibrary( info->module ); 
			info->module = NULL; 
		}

		info->path.clear(); 
		info->data_handler = NULL; 

	} while ( FALSE );

	return ret; 
}


LRESULT WINAPI unload_all_plugins()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	DATA_PROCESS_PLUGINS::iterator it; 

	do 
	{
		for( ; ; )
		{
			it = modules_info.begin(); 
			if( it == modules_info.end() )
			{
				break; 
			}

			_ret = uninit_plugin( *it ); 
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
				break; 
			}

			delete (*it); 
			modules_info.erase(it); 
		}
	} while (FALSE );

	return ret; 
}

LRESULT WINAPI check_plugin_valid( DATA_PROCESS_PLUGIN *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		if( info->module == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( info->data_handler == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	} while ( FALSE );

	return ret; 
}
LRESULT WINAPI load_plugin( DATA_PROCESS_PLUGIN *info)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( 0 == info->path.length() )
		{
			dbg_print( MSG_FATAL_ERROR, "%s %u error\n", __FUNCTION__, __LINE__ ); 

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( info->module == NULL ); 

		info->module = LoadLibraryExW( info->path.c_str(), NULL, 0 ); 
		if( NULL == info->module )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			dbg_print( MSG_FATAL_ERROR, "%s %u error\n", __FUNCTION__, __LINE__ ); 
			break; 
		}

//#define PLUGIN_DATA_HANDLER_NAME "collected_data_process"
//		typedef INT32 ( WINAPI* collected_data_process_func )( PVOID data, ULONG data_size ); 
		info->data_handler = ( collected_data_process_func )GetProcAddress( info->module, PLUGIN_DATA_HANDLER_NAME ); 
		if( NULL == info->data_handler )
		{

		}

		ret = check_plugin_valid(info); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print_w( MSG_FATAL_ERROR, L"plugin %s is invalid(path:%s:data handler %p\n", 
				info->path.c_str(), 
				info->module, 
				info->data_handler ); 
			break; 
		}
	} while ( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( NULL != info->module )
		{
			FreeLibrary( info->module ); 
			info->module = NULL; 
			info->data_handler = NULL; 
		}
	}

	return ret; 
}

LRESULT WINAPI get_module_info( MSXML::IXMLDOMNode *xml_node, DATA_PROCESS_PLUGIN *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	_variant_t xml_node_value; 
	_bstr_t _temp_text; 
	LPCWSTR temp_text; 
	HRESULT hr; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMNodePtr xml_attr; 

	do 
	{
		ASSERT( NULL != xml_node ); 
		ASSERT( NULL != info ); 

		_xml_node = xml_node; 
		//<select name="question" onchange="showcustomquest(this.value)" style="width:130px">

		hr = _xml_node->get_attributes( &xml_attrs ); 
		if( FAILED( hr ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		} 

		do 
		{
			xml_attr = xml_attrs->getNamedItem( DATA_SCRAMBLE_PLUGIN_ATTRIBUTE_PATH ); 
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
				info->path = temp_text; 
			}
		}while( FALSE ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI create_xml_file( LPCWSTR config_file_name, LPCWSTR root_element )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMProcessingInstructionPtr xml_instruction; 
	MSXML::IXMLDOMElementPtr xml_element; 
	_bstr_t text; 

	do
	{
		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 
		if( hr != S_OK)
		{
			log_trace_ex( MSG_IMPORTANT, "create xml document error %08x", hr);
			ret = hr; 
			break; 
		}

		// 创建Xml声明
		hr = xml_doc->raw_createProcessingInstruction( L"xml", L"version=\"1.0\" encoding=\"utf-8\"", &xml_instruction );

		if( S_OK != hr )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML声明失败" );
			ret = hr; 
			break; 
		}

		hr = xml_doc->raw_appendChild( xml_instruction, NULL );
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		text = root_element; 
		xml_element = xml_doc->createElement( text ); 

		if( NULL == xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = xml_doc->appendChild( xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			//break; 
		}

		{

			_variant_t save_file_name; 

			text = config_file_name; 
			save_file_name = text; 			
			hr = xml_doc->save( save_file_name ); 

			if( hr != S_OK )
			{
				ret = hr; 
			}
		}

	}while( FALSE ); 

	//// 释放xml声明
	//if (xml_instruction != NULL)
	//{
	//	xml_instruction.Release();
	//}

	//// 释放xml文档
	//if (xml_doc != NULL)
	//{
	//	xml_doc.Release ();
	//}

	//CoUninitialize ();

	return ret; 
}

LRESULT WINAPI make_plugin_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 
	//HANDLE file_handle = INVALID_HANDLE_VALUE; 
	//WCHAR xml_template[] = L"<html_sites></html_sites>"; 
	//#define MAX_TIME_TEXT 128

	//L"<?xml version=\"1.0\"?>"; 
	//L"<site url=\"%s\" >" \
	//L"<page url=\"%s\">" \
	//L"</page>" \
	//L"</site>";

	do 
	{
		ASSERT( config_file_path != NULL ); 

		ret = check_file_exist( config_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			//ret = ERROR_SUCCESS; 


			//file_handle = CreateFileW( config_file_path, 
			//	GENERIC_READ | GENERIC_WRITE, 
			//	FILE_SHARE_READ, 
			//	NULL, 
			//	OPEN_ALWAYS, 
			//	0, 
			//	NULL ); 

			//if( file_handle == INVALID_HANDLE_VALUE )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			//_ret = WriteFile( file_handle, 
			//	xml_template, 
			//	sizeof( xml_template ) - sizeof( WCHAR ), 
			//	&wrote, 
			//	NULL ); 

			//if( FALSE == _ret )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			ret = create_xml_file( config_file_path, DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_NAME ); 

		}
		else
		{
//#ifdef _DEBUG
//			ret = create_xml_file( config_file_path, DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_NAME); 
//#else	
//			ASSERT( FALSE ); 
//#endif //_DEBUG

		}

	}while( FALSE ); 

	//if( INVALID_HANDLE_VALUE != file_handle )
	//{
	//	CloseHandle( file_handle ); 
	//}

	return ret; 
}

LRESULT WINAPI load_all_plugins()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr sub_xml_element; 
	MSXML::IXMLDOMElementPtr root_xml_element; 
	MSXML::IXMLDOMNodeListPtr xml_node_list; 

	VARIANT_BOOL __ret;  

	//MSXML::IXMLDOMNodeListPtr xml_attrs; 
	MSXML::IXMLDOMAttributePtr xml_attr; 

	//LPWSTR __temp_text; 
	LPCWSTR _temp_text; 
	_bstr_t temp_text; 
	_bstr_t attr_name; 
	WCHAR file_name[ MAX_PATH ]; 
	ULONG cc_ret_len; 
	CString config_file; 

	do 
	{
		ret = get_app_path( file_name, ARRAYSIZE(file_name), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		break; 

		config_file = file_name; 
		config_file += DATA_SCRAMBLE_PLUGINS_CONFIG_FILE; 

		ret = make_plugin_config_file_exist( config_file.GetBuffer(), config_file.GetLength() ); 
		if( ret != ERROR_SUCCESS )
		{
			//break; 
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

		__ret = xml_doc->load( ( WCHAR* )config_file.GetBuffer() ); 

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
			LONG node_count; 
			LONG i; 
			size_t size; 
			MSXML::IXMLDOMNodePtr _xml_node; 
			DATA_PROCESS_PLUGIN *module_info; 
			vector< DATA_PROCESS_PLUGIN* >::iterator it; 

			element_path = DATA_SCRAMBLE_PLUGIN_ELEMENT_PATH; 

			hr = xml_doc->documentElement->raw_selectNodes( element_path.GetBSTR(), &xml_node_list ); 
			if( FAILED(hr))
			{
				log_trace_ex( MSG_IMPORTANT, "%s: is not existed", __FUNCTION__ ); //HTML_SCRAMBLE_PLUGIN_ELEMENT_PATH );
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
					module_info = new DATA_PROCESS_PLUGIN(); 
					if( NULL == module_info )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					init_plugin( module_info ); 

					hr = xml_node_list->get_item( i, &_xml_node ); 
					if( FAILED( hr ) )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					ret = get_module_info( _xml_node, module_info ); 
					if( ret != ERROR_SUCCESS )
					{
						delete module_info; 
						module_info = NULL; 
						//ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					_ret = load_plugin( module_info ); 
					if(_ret != ERROR_SUCCESS )
					{
						dbg_print_w( MSG_FATAL_ERROR, L"load plugin %s error 0x%0.8x\n", ( *it )->path.c_str(), _ret ); 
						ret = _ret; 
						break; 
					}

					size = modules_info.size(); 

					modules_info.push_back( module_info ); 
					module_info = NULL; 
					if( modules_info.size() <= size )
					{
						ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}
				}while( FALSE ); 

				if( ret != ERROR_SUCCESS )
				{
					ASSERT( FALSE ); 
					if( NULL != module_info )
					{
						delete module_info; 
						module_info = NULL; 
					}
				}
				//_xml_node->Release(); release auto
			}

			//for( it = modules_info.begin(); 
			//	it != modules_info.end(); 
			//	it ++ )
			//{
			//	_ret = load_plugin( ( *it ) ); 
			//	if(_ret != ERROR_SUCCESS )
			//	{
			//		dbg_print_w( MSG_FATAL_ERROR, L"load plugin %s error 0x%0.8x\n", ( *it )->path.c_str(), _ret ); 
			//		ret = _ret; 
			//		//break; 
			//	}
			//}
		}
	}while( FALSE ); 
	return ret; 
}

LRESULT WINAPI call_plugin_data_handler( DATA_INFO datas[], ULONG count )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	DATA_PROCESS_PLUGINS::iterator it; 
	
	do 
	{
		if( datas == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( count == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		for( it = modules_info.begin(); 
			it != modules_info.end(); 
			it ++ )
		{
			do 
			{
				//ASSERT( NULL != ( *it )->data_handler); 
				ASSERT( NULL != (*it)->module ); 

				if( NULL == ( *it )->data_handler )
				{
					break; 
				}

				try
				{	_ret = ( *it )->data_handler( datas, count ); 
					if(_ret != ERROR_SUCCESS )
					{
						ret = ret; 
						break; 
					}
				}
				catch( ... )
				{
					dbg_print( MSG_FATAL_ERROR, "%s %u error\n", __FUNCTION__, __LINE__); 
				}
			} while ( FALSE ); 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI _add_plugin_info(DATA_PROCESS_PLUGIN_INFO* info); 

LRESULT WINAPI add_plugin_info(DATA_PROCESS_PLUGIN_INFO* info) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	DATA_PROCESS_PLUGINS::iterator it; 
	DATA_PROCESS_PLUGIN *plugin = NULL; 
	size_t size; 

	do 
	{
		ASSERT( NULL != info ); 
		ret = _add_plugin_info( info ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
		}

		for( it = modules_info.begin(); 
			it != modules_info.end(); 
			it ++ )
		{
			if( 0 == wcscmp( ( *it )->path.c_str(), 
				info->path.c_str() ) )
			{
				break; 
			}
		}

		if( it != modules_info.end() )
		{
			break; 
		}

		//if(_ret != ERROR_SUCCESS )
		//{
		//	dbg_print_w( MSG_FATAL_ERROR, L"load plugin %s error 0x%0.8x\n", ( *it )->path.c_str(), _ret ); 
		//	ret = _ret; 
		//	//break; 
		//}

		plugin = new DATA_PROCESS_PLUGIN(); 
		if( NULL == plugin )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		init_plugin( plugin ); 

		plugin->path = info->path.c_str(); 
		ret = load_plugin( plugin ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print_w( MSG_FATAL_ERROR, L"initialize plugin(%s) error %d\n", info->path.c_str(), ret ); 
			break; 
		}

		size = modules_info.size(); 

		modules_info.push_back( plugin ); 
		plugin = NULL; 
		if( modules_info.size() <= size )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}
	} while ( FALSE ); 

	if( ret != ERROR_SUCCESS )
	{
		ASSERT( FALSE ); 
		if( NULL != plugin )
		{
			delete plugin; 
			plugin = NULL; 
		}
	}

	return ret; 
}

LRESULT WINAPI _add_plugin_info(DATA_PROCESS_PLUGIN_INFO* info) 
{
		LRESULT ret = ERROR_SUCCESS; 
		HRESULT hr; 

		MSXML::IXMLDOMDocumentPtr xml_doc; 
		MSXML::IXMLDOMElementPtr sub_xml_element; 
		MSXML::IXMLDOMElementPtr root_xml_element; 
		MSXML::IXMLDOMNodePtr xml_node; 
		MSXML::IXMLDOMNodePtr _xml_node; 
		MSXML::IXMLDOMNodeListPtr xml_node_list; 
		BOOLEAN info_found = FALSE; 

		VARIANT_BOOL __ret;  
		_variant_t save_file_name; 

		//MSXML::IXMLDOMNodeListPtr xml_node_list; 
		MSXML::IXMLDOMNodePtr xml_attr; 
		//LONG node_count; 
		//LPWSTR __temp_text; 
		LPCWSTR _temp_text; 
		//_bstr_t temp_text; 
		_bstr_t attr_name; 
		WCHAR file_name[ MAX_PATH ]; 
		CString config_file; 
		ULONG cc_ret_len; 

		do 
		{
			ret = get_app_path( file_name, ARRAYSIZE(file_name), &cc_ret_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			config_file = file_name; 
			config_file += DATA_SCRAMBLE_PLUGINS_CONFIG_FILE; 

			//ret = make_plugin_config_file_exist(config_file.GetBuffer(), config_file.GetLength() ); 
			//if( ret != 0 )
			//{
			//	//break; 
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

			__ret = xml_doc->load( ( WCHAR* )config_file.GetBuffer() ); 

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
				element_path = DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_PATH; 
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

			hr = root_xml_element->raw_selectNodes( DATA_SCRAMBLE_PLUGIN_ELEMENT_NAME, &xml_node_list ); 
			if( FAILED(hr))
			{
				log_trace_ex( MSG_IMPORTANT, "%s: is not existed", __FUNCTION__ ); //HTML_SCRAMBLE_PLUGIN_ELEMENT_PATH );
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( xml_node_list == NULL )
			{
				ret = ERROR_INVALID_PARAMETER; 
				log_trace_ex( MSG_IMPORTANT, "%s: action script is not null", __FUNCTION__ ); 
				break; 
			}

			LONG node_count; 
			LONG i; 
			DATA_PROCESS_PLUGIN plugin; 

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

					hr = xml_node_list->get_item( i, &xml_node ); 
					if( FAILED( hr ) )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					ret = get_module_info( xml_node, &plugin ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					if( 0 == wcscmp( plugin.path.c_str(), 
						info->path.c_str() ) )
					{
						_xml_node = xml_node; 
						info_found = TRUE; 
						break; 
					}
				}while( FALSE ); 

				if( ret != ERROR_SUCCESS )
				{
				}
				//_xml_node->Release(); release auto
			}

			do
			{
				MSXML::IXMLDOMNodePtr _xml_node; 
				_variant_t attr_value; 
				_bstr_t _attr_value; 

				if( info_found == TRUE )
				{
					ASSERT( _xml_node != NULL ); 

					//attr_name = DATA_SCRAMBLE_PLUGIN_ATTRIBUTE_PATH; 
					//_attr_value = info->path.c_str(); 
					//attr_value = _attr_value; 

					//_xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ),  ( PVOID* )&sub_xml_element ); 

					//hr = sub_xml_element->setAttribute( attr_name, attr_value ); 

					//if( hr != S_OK )
					//{
					//	break; 
					//}
					break; 
				}

				sub_xml_element = xml_doc->createElement( DATA_SCRAMBLE_PLUGIN_ELEMENT_NAME ); 
				if( NULL == sub_xml_element )
				{
					ret = hr; 
					break; 
				}

				hr = root_xml_element->raw_appendChild( sub_xml_element, NULL ); 
				if( hr != S_OK )
				{
					break; 
				}

				attr_name = DATA_SCRAMBLE_PLUGIN_ATTRIBUTE_PATH; 
				_attr_value = info->path.c_str(); 
				attr_value = _attr_value; 

				hr = sub_xml_element->setAttribute( attr_name, attr_value ); 

				if( hr != S_OK )
				{
					break; 
				}

				//modules_info.push_back()
			}while( FALSE );  

			do 
			{
				_variant_t save_file_name; 
				_bstr_t text; 

				text = config_file.GetBuffer(); 

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

LRESULT WINAPI _del_plugin_info(DATA_PROCESS_PLUGIN_INFO* info) ; 
LRESULT WINAPI del_plugin_info(DATA_PROCESS_PLUGIN_INFO* info)
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	DATA_PROCESS_PLUGINS::iterator it; 
	DATA_PROCESS_PLUGIN *plugin; 

	do 
	{
		ASSERT( NULL != info ); 
		ret = _del_plugin_info( info ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
		}

		for( it = modules_info.begin(); 
			it != modules_info.end(); 
			it ++ )
		{
			if( 0 == wcscmp( ( *it )->path.c_str(), 
				info->path.c_str() ) )
			{
				plugin = ( *it ); 
				modules_info.erase( it ); 

#ifdef DYNAMIC_RELEASE_PLUGINS
				_ret = uninit_plugin( plugin ); 
				if( _ret != ERROR_SUCCESS )
				{
					ret = _ret; 
					break; 
				}

				delete plugin; 
#endif //DYNAMIC_RELEASE_PLUGINS
				break; 
			}
		}
	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI _del_plugin_info(DATA_PROCESS_PLUGIN_INFO* info) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMElementPtr root_xml_element; 
	MSXML::IXMLDOMNodePtr xml_node; 
	MSXML::IXMLDOMNodeListPtr xml_node_list; 
	BOOLEAN info_found = FALSE; 

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
	WCHAR file_name[ MAX_PATH ]; 
	CString config_file; 
	ULONG cc_ret_len; 

	do 
	{
		ret = get_app_path( file_name, ARRAYSIZE(file_name), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		config_file = file_name; 
		config_file += DATA_SCRAMBLE_PLUGINS_CONFIG_FILE; 

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

		__ret = xml_doc->load( ( WCHAR* )config_file.GetBuffer() ); 

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
			element_path = DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_PATH; 
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

		hr = root_xml_element->raw_selectNodes( DATA_SCRAMBLE_PLUGIN_ELEMENT_NAME, &xml_node_list ); 
		if( FAILED(hr))
		{
			log_trace_ex( MSG_IMPORTANT, "%s: is not existed", __FUNCTION__ ); //HTML_SCRAMBLE_PLUGIN_ELEMENT_PATH );
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

		LONG node_count; 
		LONG i; 
		DATA_PROCESS_PLUGIN plugin; 

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

				hr = xml_node_list->get_item( i, &xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				ret = get_module_info( xml_node, &plugin ); 
				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				if( 0 == wcscmp( plugin.path.c_str(), 
					info->path.c_str() ) )
				{
					//hr = xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), &sub_xml_element ); 
					//if( FAILED(hr))
					//{
					//	dbg_print( MSG_ERROR, "%s %u error %d\n", __FUNCTION__, __LINE__, ret ); 
					//	break; 
					//}

					_xml_node = xml_node; 

					info_found = TRUE; 
					break; 
				}
			}while( FALSE ); 

			if( ret != ERROR_SUCCESS )
			{
			}
			//_xml_node->Release(); release auto
		}

		do
		{
			_variant_t attr_value; 

			if( info_found == FALSE )
			{
				break; 
			}

			ASSERT( NULL != _xml_node ); 
			hr = root_xml_element->removeChild( _xml_node ); 
			if( hr != S_OK )
			{
				break; 
			}

		}while( FALSE );  

		do 
		{
			_variant_t save_file_name; 
			_bstr_t text; 

			text = config_file.GetBuffer(); 

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

