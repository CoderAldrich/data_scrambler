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
#import "msxml.tlb"
#include "html_xpath.h"
#include "input_content.h"

LRESULT WINAPI add_input_text_block_info( LPCWSTR file_name, LPCWSTR text, LPCWSTR text_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMElementPtr xml_node; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr xml_root_element = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	_bstr_t temp_text; 
	_bstr_t attr_name; 
	LPCWSTR _temp_text; 
	VARIANT_BOOL __ret; 
	BOOLEAN node_found = FALSE; 

	_variant_t node_value; 
	LONG node_count; 
	LONG i; 

	do 
	{
		ASSERT( NULL != file_name ); 
		ASSERT( NULL != text ); 
		ASSERT( NULL != text_name );

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_FATAL_ERROR, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
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

		xml_root_element = xml_doc->documentElement; 
		if( xml_root_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		hr = xml_root_element->get_childNodes( &xml_node_list ); 
		if( S_OK != hr )
		{
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				hr = xml_node_list->get_item( i, &_xml_node ); 
				if( S_OK != hr )
				{
					break; 
				}

				hr = _xml_node->get_attributes( &xml_attrs ); 
				if( S_OK != hr )
				{
					break; 
				}

				temp_text = INPUT_TEXT_NAME_ATTRIBUTE_NAME; 

				xml_attr = xml_attrs->getNamedItem( temp_text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
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

				if( 0 != _wcsicmp( _temp_text, text_name ) )
				{
					break; 
				}

				node_found = TRUE; 

				temp_text = INPUT_TEXT_BLOCK_ATTRIBUTE_NAME; 

				xml_attr = xml_attrs->getNamedItem( temp_text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
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

				if( 0 == _wcsicmp( _temp_text, text ) )
				{
					break; 
				}

				temp_text = text; 
				node_value = temp_text; 
				temp_text = INPUT_TEXT_BLOCK_ATTRIBUTE_NAME; 

				hr = _xml_node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), ( PVOID* )&xml_node ); 
				if( FAILED( hr ) )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( NULL == xml_node )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = xml_node->setAttribute( temp_text, node_value ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}
			}while( FALSE );

			if( TRUE == node_found )
			{
				break; 
			}
		}

		if( TRUE == node_found )
		{
			_variant_t save_file_name; 

			text = file_name; 
			save_file_name = text; 			
			hr = xml_doc->save( save_file_name ); 

			if( hr != S_OK )
			{
				ret = hr; 
			}

			break; 
		}
		
		xml_node = xml_doc->createElement( INPUT_TEXT_CONTENT_ELEMENT_NAME ); 

		if( NULL == xml_node )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		xml_root_element->raw_appendChild( xml_node, NULL );

		temp_text = text; 
		node_value = temp_text; 
		attr_name = INPUT_TEXT_BLOCK_ATTRIBUTE_NAME; 

		hr = xml_node->setAttribute( attr_name, node_value ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		temp_text = text_name; 
		node_value = temp_text; 
		attr_name = INPUT_TEXT_NAME_ATTRIBUTE_NAME; 

		hr = xml_node->setAttribute( attr_name, node_value ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		{

			_variant_t save_file_name; 

			text = file_name; 
			save_file_name = text; 			
			hr = xml_doc->save( save_file_name ); 

			if( hr != S_OK )
			{
				ret = hr; 
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI del_input_text_block_info( LPCWSTR file_name, LPCWSTR text, LPCWSTR text_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMElementPtr xml_node; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr xml_root_element = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	_bstr_t temp_text; 
	_bstr_t attr_name; 
	LPCWSTR _temp_text; 
	VARIANT_BOOL __ret; 
	BOOLEAN node_found = FALSE; 

	_variant_t node_value; 
	LONG node_count; 
	LONG i; 

	do 
	{
		ASSERT( NULL != file_name ); 
		ASSERT( NULL != text ); 
		ASSERT( NULL != text_name );

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_FATAL_ERROR, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
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

		xml_root_element = xml_doc->documentElement; 
		if( xml_root_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		hr = xml_root_element->get_childNodes( &xml_node_list ); 
		if( S_OK != hr )
		{
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				hr = xml_node_list->get_item( i, &_xml_node ); 
				if( S_OK != hr )
				{
					break; 
				}

				hr = _xml_node->get_attributes( &xml_attrs ); 
				if( S_OK != hr )
				{
					break; 
				}

				temp_text = INPUT_TEXT_NAME_ATTRIBUTE_NAME; 

				xml_attr = xml_attrs->getNamedItem( temp_text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
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

				if( 0 != _wcsicmp( _temp_text, text_name ) )
				{
					break; 
				}

				node_found = TRUE; 

				{
					MSXML::IXMLDOMNodePtr old_element; 
					hr = xml_root_element->raw_removeChild( _xml_node, &old_element ); 
					if( FAILED( hr ) )
					{
						
					}

					break; 
				}
			}while( FALSE );

			if( TRUE == node_found )
			{
				_variant_t save_file_name; 

				text = file_name; 
				save_file_name = text; 			
				hr = xml_doc->save( save_file_name ); 

				if( hr != S_OK )
				{
					ret = hr; 
				}

				break; 
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_input_context_file_path( wstring &file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG cc_ret_len; 
	WCHAR app_path[ MAX_PATH ]; 

	do 
	{
#define HTML_INPUT_CONTENT_FILE_NAME L"input_content.dat" 
#define CONTENT_FILE_DIRECTORY L"html_content\\"

		ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONTENT_FILE_DIRECTORY ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( app_path, 
			ARRAYSIZE( app_path ), 
			HTML_INPUT_CONTENT_FILE_NAME ); 

		if( hr != S_OK )
		{
			ret = hr; 
			break; 
		}

		file_name = app_path; 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI load_input_content( LPCWSTR file_name, XPATH_PARAMS &contents )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMElementPtr xml_node; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr xml_root_element = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 

	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	_bstr_t text_name; 
	_bstr_t text_value; 
	_bstr_t temp_text; 
	LPCWSTR _temp_text; 
	LPCWSTR _text_name; 
	LPCWSTR _text_value; 

	VARIANT_BOOL __ret; 

	LONG i; 
	_variant_t node_value; 
	LONG node_count; 

	do 
	{
		ASSERT( NULL != file_name ); 

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_FATAL_ERROR, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
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

		xml_root_element = xml_doc->documentElement; 
		if( xml_root_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		hr = xml_root_element->get_childNodes( &xml_node_list ); 
		if( S_OK != hr )
		{
			break; 
		}

		hr = xml_node_list->get_length( &node_count ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				hr = xml_node_list->get_item( i, &_xml_node ); 
				if( S_OK != hr )
				{
					break; 
				}

				hr = _xml_node->get_attributes( &xml_attrs ); 
				if( S_OK != hr )
				{
					break; 
				}

				temp_text = INPUT_TEXT_NAME_ATTRIBUTE_NAME; 

				xml_attr = xml_attrs->getNamedItem( temp_text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
				if( S_OK != hr )
				{
					break; 
				}

				text_name = node_value.bstrVal; 

				temp_text = INPUT_TEXT_BLOCK_ATTRIBUTE_NAME; 

				xml_attr = xml_attrs->getNamedItem( temp_text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
				if( S_OK != hr )
				{
					break; 
				}

				text_value = node_value.bstrVal; 

				_text_value = text_value.operator const wchar_t*(); 

				if( NULL == _text_value )
				{
					break; 
				}

				_text_name = text_name.operator const wchar_t*(); 

				if( NULL == _text_name )
				{
					break; 
				}

				contents.insert( XPATH_PARAM_PAIR( _text_name, _text_value ) ); 
				//if( ret != ERROR_SUCCESS )
				//{

				//}
			}while( FALSE );
		}
	}while( FALSE ); 

	return ret; 
}


LRESULT WINAPI create_content_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMProcessingInstructionPtr xml_instruction; 
	MSXML::IXMLDOMElementPtr xml_element; 
	_bstr_t text; 

	do
	{
#define INPUT_CONTENT_ROOT_ELEMENT L"contents"
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

		text = INPUT_CONTENT_ROOT_ELEMENT; 
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

			text = file_name; 
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

LRESULT WINAPI make_content_file_exist( LPCWSTR config_file_path, ULONG cc_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 

	do 
	{
		ASSERT( config_file_path != NULL ); 

		ret = check_file_exist( config_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			ret = create_content_file( config_file_path ); 

		}
		else
		{
			//ASSERT( FALSE ); 
		}

	}while( FALSE ); 

	if( INVALID_HANDLE_VALUE != file_handle )
	{
		CloseHandle( file_handle ); 
	}

	return ret; 
}


LRESULT WINAPI add_auto_input_content( MSXML::IXMLDOMDocument *xml_doc, 
									  MSXML::IXMLDOMElement *xml_element, 
									  LPCWSTR name, 
									  LPCWSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	_bstr_t _text; 
	_bstr_t attr_name; 
	MSXML::IXMLDOMElementPtr _xml_element;
	//MSXML::IXMLDOMElementPtr _xml_element;

	do 
	{
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != xml_element ); 
		ASSERT( NULL != name ); 
		ASSERT( NULL != text ); 

		_xml_element = xml_doc->createElement( INPUT_CONTENT_ELEMENT_TAG ); 
		if( NULL == _xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_text = text; 

		hr = _xml_element->put_text( _text.GetBSTR() ); 

		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		attr_name = INPUT_CONTENT_ELEMENT_NAME_ATTRIBUTE; 

		_text = name; 
		hr = _xml_element->setAttribute( attr_name.GetBSTR(), _text.GetBSTR() ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		hr = xml_element->appendChild( _xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}
	}while( FALSE );

	return ret; 
}
