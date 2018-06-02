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
#include <strsafe.h>
#import "msxml.tlb"
#define XPATH_CREATE
#include "html_xpath.h"
#include "html_element_config.h"
//#include "data_scrambler.h"
#include "input_content.h"
#include "data_scramble.h"
//#include "capture_input_dlg.h"

LRESULT WINAPI save_scarmble_info( LPCWSTR file_name, 
								  HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;
	MSXML::IXMLDOMElementPtr dest_element; 
	MSXML::IXMLDOMElementPtr sub_element; 

	MSXML::IXMLDOMTextPtr xml_text; 

	_bstr_t text; 
	LPCWSTR _text; 

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != file_name ); 
		ASSERT(NULL != scramble_info); 

		ret = make_config_file_exist( file_name, wcslen( file_name ) ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		{
			_bstr_t temp_text; 
			_variant_t _file_name; 

			temp_text = file_name; 
			_file_name = temp_text.GetBSTR(); 

			__ret = xml_dest_doc->load( _file_name ); 
		}

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_text = bstrReason.operator wchar_t*(); 

			if( NULL != _text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws(line%u:%u)\n", _text, 
					spParseError->line, 
					spParseError->linepos ); 
			}

			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_element ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_element )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		//_src_xml = src_xml.operator const wchar_t*(); 
		//if( NULL == _src_xml )
		//{
		//	break; 
		//}

		do
		{
			//_variant_t attr_value; 
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_dest_doc->createAttribute( L"url" ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = scramble_info->url.c_str(); 

			xml_text = xml_dest_doc->createTextNode( text ); 
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
			//_variant_t attr_value; 
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_dest_doc->createAttribute( L"links_xpath" ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = scramble_info->link_list_xpath.c_str(); 

			xml_text = xml_dest_doc->createTextNode( text ); 
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

		{
			//_variant_t attr_value; 
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_dest_doc->createAttribute( L"next_link_xpath" ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = scramble_info->next_link_xpath.c_str(); 

			xml_text = xml_dest_doc->createTextNode( text ); 
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


		HTML_ELEMENT_CONTENTS_ITERATOR it; 
		_variant_t value; 
		_bstr_t _value; 

		it = scramble_info->page_content.element_contents.begin(); 
		for( ; ; )
		{
			if( it == scramble_info->page_content.element_contents.end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 

			sub_element = xml_dest_doc->createElement( L"element" ); 
			if( NULL == sub_element )
			{
				break; 
			}

			{
				//_variant_t attr_value; 
				MSXML::IXMLDOMAttributePtr xml_attr; 

				xml_attr = xml_dest_doc->createAttribute( L"xpath" ); 
				if( NULL == xml_attr )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				text = ( *it )->xpath.c_str(); 

				xml_text = xml_dest_doc->createTextNode( text ); 
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

				sub_element->setAttributeNode( xml_attr ); 

				_value = ( *it )->name.c_str(); 

				value = _value.GetBSTR(); 
				sub_element->setAttribute( L"name", value.GetVARIANT() );

				dest_element->appendChild( sub_element ); 
			}while( FALSE ); 

			it ++; 
		}

		{
			_bstr_t temp_text; 
			_variant_t _file_name; 

			temp_text = file_name; 
			_file_name = temp_text.GetBSTR(); 

			hr = xml_dest_doc->save( _file_name.GetVARIANT() ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI load_scramble_info_from_file( LPCWSTR file_name, HTML_LIST_ELEMENT_SCRAMBLE_INFO *scramble_info)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;
	MSXML::IXMLDOMElementPtr dest_element; 
	//MSXML::IXMLDOMNodePtr dest_node; 
	MSXML::IXMLDOMElementPtr sub_element; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	MSXML::IXMLDOMAttributePtr xml_attr; 
	MSXML::IXMLDOMNodeListPtr xml_node_list; 
	MSXML::IXMLDOMNodePtr _xml_node; 
	MSXML::IXMLDOMTextPtr xml_text; 

	LONG node_count; 
	LONG i; 
	_variant_t node_value; 
	_bstr_t text; 
	LPCWSTR _text; 

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != file_name ); 
		ASSERT(NULL != scramble_info); 

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		{
			_bstr_t temp_text; 
			_variant_t _file_name; 

			temp_text = file_name; 
			_file_name = temp_text.GetBSTR(); 

			__ret = xml_dest_doc->load( _file_name ); 
		}

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_text = bstrReason.operator wchar_t*(); 

			if( NULL != _text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws(line%u:%u)\n", _text, 
					spParseError->line, 
					spParseError->linepos ); 
			}

			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_element ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_element )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = dest_element->get_attributes( &xml_attrs ); 
		if( S_OK != hr )
		{
			break; 
		}

		text = L"url"; 
		xml_attr = xml_attrs->getNamedItem( text.GetBSTR() ); 
		if( NULL == xml_attr )
		{
			break; 
		}

		hr = xml_attr->get_nodeValue( node_value.GetAddress() ); 
		if( S_OK != hr )
		{
			break; 
		}

		if( node_value.vt != VT_BSTR )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		text = node_value.bstrVal; 
		_text = text.operator wchar_t*(); 

		if( NULL == _text )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		scramble_info->url = _text; 

		text = L"links_xpath"; 
		xml_attr = xml_attrs->getNamedItem( text.GetBSTR() ); 
		if( NULL == xml_attr )
		{
			break; 
		}

		hr = xml_attr->get_nodeValue( node_value.GetAddress() ); 
		if( S_OK != hr )
		{
			break; 
		}

		if( node_value.vt != VT_BSTR )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		text = node_value.bstrVal; 
		_text = text.operator wchar_t*(); 

		if( NULL == _text )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		scramble_info->link_list_xpath = _text; 

		text = L"next_link_xpath"; 
		xml_attr = xml_attrs->getNamedItem( text.GetBSTR() ); 
		if( NULL == xml_attr )
		{
			break; 
		}

		hr = xml_attr->get_nodeValue( node_value.GetAddress() ); 
		if( S_OK != hr )
		{
			break; 
		}

		if( node_value.vt != VT_BSTR )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		text = node_value.bstrVal; 
		_text = text.operator wchar_t*(); 

		if( NULL == _text )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		scramble_info->next_link_xpath = _text; 

		hr = dest_element->get_childNodes( &xml_node_list ); 
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

		HTML_ELEMENT_CONTENT *element_content = NULL; 
		element_content = new HTML_ELEMENT_CONTENT(); 
		if( NULL == element_content )
		{
			break; 
		}

		element_content->name = L"URL"; 
		scramble_info->page_content.element_contents.push_back( element_content ); 
		element_content = NULL; 

		for( i = 0; i < node_count; i ++ )
		{
			do 
			{
				element_content = new HTML_ELEMENT_CONTENT(); 
				if( NULL == element_content )
				{
					break; 
				}

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

				text = L"xpath"; 

				xml_attr = xml_attrs->getNamedItem( text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
				if( S_OK != hr )
				{
					break; 
				}

				text  = node_value.bstrVal; 

				_text = text.operator wchar_t*(); 

				if( NULL == _text )
				{
					break; 
				}

				element_content->xpath = _text; 

				text = L"name"; 
				xml_attr = xml_attrs->getNamedItem( text.GetBSTR() ); 
				if( NULL == xml_attr )
				{
					break; 
				}

				hr = xml_attr->get_nodeValue( &node_value ); 
				if( S_OK != hr )
				{
					break; 
				}

				text = node_value.bstrVal; 

				_text = text .operator const wchar_t*(); 

				if( NULL == _text )
				{
					break; 
				}

				element_content->name = _text; 
				scramble_info->page_content.element_contents.push_back( element_content ); 
				element_content = NULL; 

			}while( FALSE );
		
			if( NULL != element_content )
			{
				delete element_content; 
			}
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI find_page_group_config( LPCWSTR file_name, 
									  LPCWSTR domain_name, 
									  MSXML::IXMLDOMDocument **xml_doc_out, 
									  MSXML::IXMLDOMElement **site_element_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr _xml_doc; 
	MSXML::IXMLDOMProcessingInstructionPtr xml_instruction; 
	MSXML::IXMLDOMElementPtr xml_root_element = NULL; 
	MSXML::IXMLDOMNodeListPtr xml_node_list = NULL; 
	MSXML::IXMLDOMElementPtr xml_element = NULL; 
	MSXML::IXMLDOMNodePtr xml_node; 
	MSXML::IXMLDOMNodePtr xml_attr; 
	MSXML::IXMLDOMNamedNodeMapPtr xml_attrs; 
	INT32 i; 

	LONG node_count; 
	VARIANT_BOOL __ret; 

	_bstr_t temp_text; 
	LPCWSTR _temp_text; 

	_variant_t node_value; 
	BOOLEAN node_found = FALSE; 

	do
	{
		ASSERT( NULL != file_name ); 
		ASSERT( NULL != domain_name ); 
		ASSERT( NULL != xml_doc_out ); 
		ASSERT( NULL != site_element_out ); 

		*xml_doc_out = NULL; 
		*site_element_out = NULL; 

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&_xml_doc ); 
		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "create xml document error %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = _xml_doc->load( ( WCHAR* )file_name ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = _xml_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER; 
			break; 		
		}

		xml_root_element = _xml_doc->documentElement; 
		if( xml_root_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			log_trace_ex( MSG_IMPORTANT, " documentElement invalid\n" );
			break; 
		}

		//是否可以直接使用XPATH直接查询属性进行定位
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
				hr = xml_node_list->get_item( i, &xml_node ); 
				if( S_OK != hr )
				{
					break; 
				}

				hr = xml_node->get_attributes( &xml_attrs ); 
				if( S_OK != hr )
				{
					break; 
				}

				temp_text = domain_name; 

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

				if( 0 == _wcsicmp( _temp_text, domain_name ) )
				{
					hr = xml_node->QueryInterface( IID_IXMLDOMElement, ( PVOID* )&xml_element ); 
					if( hr != S_OK )
					{
						break; 
					}

					node_found = TRUE; 
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
			break; 
		}

		xml_element = _xml_doc->createElement( L"site" ); 

		if( NULL == xml_element )
		{
			log_trace_ex( MSG_IMPORTANT, "create the root element of the document error 0x%0.8x\n", hr ); ;
			ret = hr; 
			break; 
		}

		temp_text = domain_name; 
		xml_element->setAttribute( L"url", temp_text );

		xml_root_element->appendChild( xml_element );

		_xml_doc->AddRef(); 
		xml_element->AddRef(); 

		*xml_doc_out = _xml_doc; 
		*site_element_out = xml_element; 

	}while( FALSE ); 

	// 释放xml文档
	//if (_xml_doc != NULL)
	//{
	//	_xml_doc->Release ();
	//}

	return ret; 
}

LRESULT WINAPI add_page_config( MSXML::IXMLDOMDocument *xml_doc, 
							   LPCWSTR url, 
							   MSXML::IXMLDOMElement *site_node, 
							   MSXML::IXMLDOMElement **page_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	MSXML::IXMLDOMElementPtr xml_node = NULL; 

	_bstr_t temp_text; 
	//LPCWSTR _temp_text; 

	do
	{
		ASSERT( NULL != url ); 
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != site_node );
		ASSERT( NULL != page_node ); 

		//hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
		//	NULL, 
		//	CLSCTX_INPROC_SERVER, 
		//	__uuidof( MSXML::IXMLDOMDocument ), 
		//	( void** )&xml_doc ); 

		//if( hr != S_OK)
		//{
		//	wsprintfW (Msg, L"构造KEY XML:初始化XML文档对象失败, %08x", hr);
		//	ret = hr; 
		//	break; 
		//}

		//__ret = xml_doc->load( ( WCHAR* )file_name ); 

		//if( __ret != VARIANT_TRUE )
		//{
		//	MSXML::IXMLDOMParseErrorPtr spParseError;
		//	_bstr_t bstrReason;

		//	spParseError = xml_doc->parseError;

		//	bstrReason = spParseError->reason;

		//	_temp_text = bstrReason.operator wchar_t*(); 

		//	if( NULL != _temp_text )
		//	{
		//		dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
		//	}

		//	break; 		
		//}

		*page_node = NULL; 

		xml_node = xml_doc->createElement (L"page" ); 

		if( NULL == xml_node )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		site_node->raw_appendChild( xml_node, NULL );

		temp_text = url; 

		xml_node->setAttribute (L"url", temp_text);

		xml_node->AddRef(); 
		*page_node = xml_node; 
	}while( FALSE ); 

	// 释放xml文档
	//if (xml_doc != NULL)
	//{
	//	xml_doc->Release ();
	//}

	//CoUninitialize ();

	return ret; 
}

//LRESULT WINAPI insert_node_by_text(MSXML::IXMLDOMDocument *xml_src_doc, MSXML::IXMLDOMNode *parent_node, MSXML::IXMLDOMElement *dest_node)
//{
//	LRESULT ret = ERROR_SUCCESS;
//	HRESULT hr; 
//	_bstr_t temp_text;
//	//LPCWSTR _temp_text; 
//	MSXML::IXMLDOMElementPtr xml_element; 
//	MSXML::IXMLDOMNodeListPtr child_nodes; 
//	MSXML::IXMLDOMNamedNodeMapPtr node_attrs; 
//	MSXML::IXMLDOMNodePtr attr; 
//	//LONG node_count; 
//	LONG atrr_count; 
//	LONG i; 
//
//	do
//	{
//		ASSERT(NULL != xml_src_doc);
//		ASSERT(NULL != parent_node);
//		ASSERT(NULL != dest_node);
//
//		hr = dest_node->get_nodeName( temp_text.GetAddress() );
//		if (S_OK != hr)
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		hr = xml_src_doc->raw_createElement( temp_text.GetBSTR(), &xml_element ); 
//		if( S_OK != hr )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		//hr = dest_node->get_childNodes( &child_nodes ); 
//		//if( S_OK != hr )
//		//{
//		//	break; 
//		//}
//
//		//hr = child_nodes->get_length( &node_count ); 
//		//if( S_OK != hr )
//		//{
//		//	break; 
//		//}
//
//		//for( i = 0; i < node_count; i ++ )
//		//{
//		//	hr = child_nodes->get_item( i, &child_node ); 
//		//	if( S_OK != hr )
//		//	{
//		//		break; 
//		//	}
//
//		//	child_node 
//		//}
//
//		hr = dest_node->get_attributes( &node_attrs ); 
//		if( S_OK != hr )
//		{
//			//ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		hr = node_attrs->get_length( &atrr_count ); 
//		if( S_OK != hr )
//		{
//			ret = hr; 
//			break; 
//		}
//
//		if( atrr_count == 0 )
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		_variant_t attr_value; 
//		for( i = 0; i < atrr_count; i ++ )
//		{
//			do 
//			{
//				hr = node_attrs->get_item( i, &attr ); 
//				if( S_OK != hr )
//				{
//					ret = hr; 
//					break; 
//				}
//
//				hr = attr->get_nodeName( temp_text.GetAddress() ); 
//				if( S_OK != hr )
//				{
//					ret = hr; 
//					break; 
//				}
//
//				hr = attr->get_nodeValue( attr_value.GetAddress() ); 
//				if( S_OK != hr )
//				{
//					ret = hr; 
//					break; 
//				}
//
//				hr = xml_element->setAttribute( temp_text.GetBSTR(), attr_value.GetVARIANT() ); 
//				if( S_OK != hr )
//				{
//					ret = hr; 
//					break; 
//				}
//			}while( FALSE );
//
//		}
//
//		hr = parent_node->appendChild( xml_element ); 
//		if( FAILED( hr ) )
//		{
//			ret = hr; 
//			break; 
//		}
//	} while (FALSE);
//
//	return ret;
//}

LRESULT WINAPI insert_node_by_text(MSXML::IXMLDOMDocument *xml_src_doc, MSXML::IXMLDOMElement *dest_node)
{
	LRESULT ret = ERROR_SUCCESS;
	HRESULT hr; 
	_bstr_t temp_text;
	//LPCWSTR _temp_text; 
	MSXML::IXMLDOMElementPtr xml_element; 
	MSXML::IXMLDOMNodeListPtr child_nodes; 
	MSXML::IXMLDOMNamedNodeMapPtr node_attrs; 
	MSXML::IXMLDOMNodePtr attr; 
	MSXML::IXMLDOMElementPtr parent_node; 
	//LONG node_count; 
	LONG atrr_count; 
	LONG i; 

	do
	{
		ASSERT(NULL != xml_src_doc);
		//ASSERT(NULL != parent_node);
		ASSERT(NULL != dest_node);

		hr = dest_node->get_nodeName( temp_text.GetAddress() );
		if (S_OK != hr)
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = xml_src_doc->raw_createElement( temp_text.GetBSTR(), &xml_element ); 
		if( S_OK != hr )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = xml_src_doc->get_documentElement( &parent_node ); 
		if( S_OK != hr )
		{
			break; 
		}

		if( NULL == parent_node )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		//hr = dest_node->get_childNodes( &child_nodes ); 
		//if( S_OK != hr )
		//{
		//	break; 
		//}

		//hr = child_nodes->get_length( &node_count ); 
		//if( S_OK != hr )
		//{
		//	break; 
		//}

		//for( i = 0; i < node_count; i ++ )
		//{
		//	hr = child_nodes->get_item( i, &child_node ); 
		//	if( S_OK != hr )
		//	{
		//		break; 
		//	}

		//	child_node 
		//}

		hr = dest_node->get_attributes( &node_attrs ); 
		if( S_OK != hr )
		{
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = node_attrs->get_length( &atrr_count ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		if( atrr_count == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_variant_t attr_value; 
		for( i = 0; i < atrr_count; i ++ )
		{
			do 
			{
				hr = node_attrs->get_item( i, &attr ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = attr->get_nodeName( temp_text.GetAddress() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = attr->get_nodeValue( attr_value.GetAddress() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = xml_element->setAttribute( temp_text.GetBSTR(), attr_value.GetVARIANT() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}
			}while( FALSE );

		}

		hr = parent_node->appendChild( xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}
	} while (FALSE);

	return ret;
}

LRESULT WINAPI inert_xml_text(LPCWSTR src_xml, LPCWSTR dest_xml, LPCWSTR node_xpath, LPWSTR xml_out, ULONG cc_buf_len)
{
	LRESULT ret = ERROR_SUCCESS;
	HRESULT hr;
	MSXML::IXMLDOMDocumentPtr xml_src_doc;
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;

	MSXML::IXMLDOMNodeListPtr node_list;
	MSXML::IXMLDOMNodePtr node;
	MSXML::IXMLDOMNodePtr parent_node;
	MSXML::IXMLDOMElementPtr dest_node;

	MSXML::IXMLDOMElementPtr root_element;

	_bstr_t dest_text;
	_bstr_t temp_text;
	LPCWSTR _temp_text;
	LONG node_count;

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != src_xml);
		ASSERT(NULL != dest_xml);
		ASSERT(NULL != node_xpath);

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_src_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = xml_dest_doc->loadXML( ( WCHAR* )dest_xml ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_node ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_node )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		//xml_doc->Release(); 

		__ret = xml_src_doc->loadXML( ( WCHAR* )dest_xml ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_src_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_src_doc->get_documentElement(&root_element);
		if (hr != S_OK)
		{
			ret = ERROR_ERRORS_ENCOUNTERED;
			break;
		}

		do
		{
			node_list = root_element->selectNodes( node_xpath );
			if( NULL != node_list )
			{
				parent_node = root_element;
				//ret = ERROR_INVALID_PARAMETER; 
				//break; 
			}

			hr = node_list->get_length(&node_count);
			if (S_OK != hr)
			{
				parent_node = root_element;
			}

			if( node_count <= 0 )
			{
				break; 
			}

			hr = node_list->get_item(node_count - 1, &node);
			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break;
			}

			hr = node->get_parentNode(&parent_node);

			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break;
			}
		} while (FALSE);

		if (parent_node == NULL)
		{
			ret = ERROR_ERRORS_ENCOUNTERED;
			break;
		}

		ret = insert_node_by_text(xml_src_doc, dest_node);
		if (ERROR_SUCCESS != ret)
		{
			break;
		}
	} while (FALSE);

	return ret;
}

LRESULT WINAPI add_html_element_config_from_text(  MSXML::IXMLDOMDocument *xml_doc, 
												 LPCWSTR url, 
												 MSXML::IXMLDOMNode *page_node, 
												 IHTMLElement *html_element ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr _xml_doc; 
	MSXML::IXMLDOMElementPtr xml_element = NULL;  // 根节点
	BOOLEAN com_inited = FALSE; 
	_bstr_t text; 
	//LPCWSTR _text; 
	//wstring domain_name; 

	do 
	{
		ASSERT( url != NULL ); 
		ASSERT( html_element != NULL ); 
		ASSERT( NULL != xml_doc ); 

		_xml_doc = xml_doc; 

		hr = html_element->get_outerText( text.GetAddress() ); 
		if( hr != S_OK )
		{
			ret = hr; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI format_html_text( wstring html_text, wstring &html_text_out, IHTMLElement *html_element )
{
	LRESULT ret = ERROR_SUCCESS; 
	//LPCWSTR xml_node_end; 
	wstring _html_text; 
	wstring last_attr_name; 
//#define MAX_XML_TEXT_LENGTH 256
	//LPWSTR sub_string[ MAX_XML_TEXT_LENGTH ]; 

	//IHTMLDocument2 test_doc; 
	//IHTMLElement4 test_element; 

	do 
	{
		if( html_text.length() <= 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do 
		{
			wstring word; 
			HTML_TEXT_STATE text_state; 
			HTML_WORD_STATE word_state; 
			LONG attr_value_begin; 
			LONG word_begin_pos; 
			LPWSTR _param = NULL; 
			HTML_WORDS tags; 
			HTML_WORDS bug_tags; 
			HTML_WORD_INFORMATION *word_info = NULL;  
			//LPWSTR param_begin; 
			//LPWSTR param_end; 
			//LPWSTR param_name; 
			//LPWSTR param_value; 
			LONG i; 

			do 
			{
				_param = ( LPWSTR )malloc( ( html_text.length() + 1 ) << 1 ); 
				if( NULL == _param )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				memcpy( _param, html_text.c_str(), ( ( html_text.length() + 1 ) << 1 ) ); 

				text_state = HTML_TEXT_BEGIN; 

				word_begin_pos = -1; 
				i = 0; 

				for( ; ; )
				{
					if( i >= ( LONG )html_text.length() )
					{
						if( text_state != HTML_TEXT_BEGIN )
						{
							ret = ERROR_INVALID_PARAMETER; 
						}
						break; 
					}

					//if( i == 1196 )
					//{
					//	DebugBreak(); 
					//}

					switch( _param[ i ] )
					{
					case L'/':
						{
							switch( text_state )
							{
							case HTML_TAG_BEGIN: 
								ASSERT( i >= word_begin_pos ); 
								word_state = HTML_TAG_RIGHT; 
								//word_begin_pos = i + 1; 
								break; 
							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
								break; 
							case HTML_TEXT_BEGIN: 
								//<A< ERROR
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//<A a1< ERROR
							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
							default:
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
						}
						break; 
					case L'<':
						{
							switch( text_state )
							{
							case HTML_TEXT_BEGIN: 
								ASSERT( i >= word_begin_pos ); 

								text_state = HTML_TAG_BEGIN; 
								word_state = HTML_TAG_LEFT; 

								word_begin_pos = i + 1; 
								_html_text += html_text.at( i ); 
								break; 
							case HTML_TAG_BEGIN: 

								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
								if( 0 == wcsnicmp( word.c_str(), L"!--", CONST_STR_LEN( L"!--" ) ) )
								{
									LPCWSTR comment_end; 
								
									comment_end = wcsstr( html_text.c_str() + word_begin_pos, L"-->" ); 
									if( comment_end == NULL )
									{
										ret = ERROR_INVALID_PARAMETER; 
										break; 
									}

									_html_text += html_text.substr( word_begin_pos, comment_end - html_text.c_str() - word_begin_pos + 1 ); 
									word_begin_pos = comment_end - html_text.c_str() + 1; 
								
									text_state = HTML_TEXT_BEGIN; 
								}

								break; 
								//<A< ERROR
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//<A a1< ERROR
							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
							default:
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
						}
						break; 
					case L'>':
						{
							switch( text_state )
							{
							case HTML_TAG_BEGIN:  
								//<A a1=1 a2=2 > can receive the entire string in the element now. (A a1=1 a2=2)
								ASSERT( word_begin_pos >= 0 ); 
								ASSERT( i >= word_begin_pos );  

								//TAG
								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
								_html_text += word; 

								switch( word_state )
								{
								case HTML_TAG_LEFT: 
									{
										word_info = new HTML_WORD_INFORMATION(); 

										if( NULL == word_info )
										{
											ret = ERROR_NOT_ENOUGH_MEMORY; 
											break; 
										}

										word_info->word = word; 
										word_info->word_end = _html_text.length(); 
										word_info->left_end = _html_text.length() + 1; 

										tags.push_back( word_info ); 
										word_info = NULL; 
									}
									break; 

								case HTML_TAG_RIGHT:
									{
										/******************************
										<a><br><c></a>
										******************************/

										word = word.substr( 1, word.length() - 1 ); 

										ret = fix_html_tag_end( word, &tags, &bug_tags ); 
										if( ret != ERROR_SUCCESS )
										{
											dbg_print( MSG_FATAL_ERROR, "check the tag end error %u\n", ret ); 
										}
									}
									break; 
								default:
									ASSERT( FALSE ); 
									break; 
								}

								_html_text += html_text.at( i ); 

								word_begin_pos = i + 1; 
								text_state = HTML_TEXT_BEGIN; 
								break; 

							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
								ASSERT( word_begin_pos >= 0 ); 
								ASSERT( i >= word_begin_pos ); 

								//<a a=b c d >
								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
								ret = fix_html_attribute_format( word, word ); 

								_html_text += word; 

								{
									HTML_WORD_ITERATOR it; 

									it = tags.end() - 1; 

									( *it )->left_end = _html_text.length(); 
								}

								_html_text += html_text.at( i ); 

								word_begin_pos = i + 1; 
								text_state = HTML_TEXT_BEGIN; 
								break; 
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//< TEST > 
								//<TEST A="A" >

								{
									HTML_WORD_ITERATOR it; 

									it = tags.end() - 1; 

									( *it )->left_end = _html_text.length(); 

									_html_text += html_text.at( i ); 

									word_begin_pos = i + 1; 
									text_state = HTML_TEXT_BEGIN; 
								}
								break; 
							case HTML_TEXT_BEGIN: 
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							default:
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
						}
						break; 
					case L'=':
						{
							switch( text_state )
							{
							case HTML_TAG_BEGIN:  
								//<A = 
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 

							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
								//<A a1= = 
								//<A a1= 
								//log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								//ret = ERROR_INVALID_PARAMETER; 
								ASSERT( word_begin_pos >= 0 ); 
								ASSERT( i >= word_begin_pos );  

								//<A a1=a hello a2 = X
								//<A a1=a hello a2= 

								{
									ASSERT( word_begin_pos >= 0 ); 
									ASSERT( i >= word_begin_pos );  

									if( i < word_begin_pos )
									{
										ASSERT( FALSE ); 
										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}
									else if( i == word_begin_pos )
									{
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}
									else if( i - word_begin_pos )
									{
										//if( L' ' != _html_text.at( _html_text.length() - 1 ) )
										//{
										//	_html_text += L' '; 
										//}

										ASSERT( attr_value_begin >= 0 ); 

										if( word_begin_pos < attr_value_begin )
										{
											ASSERT( FALSE ); 
											ret = ERROR_ERRORS_ENCOUNTERED; 
											break; 
										}
										else if( word_begin_pos == attr_value_begin )
										{
											//<A a1= a2= X
											ASSERT( FALSE ); 
											ret = ERROR_ERRORS_ENCOUNTERED; 
											break; 
										}

										ASSERT( word_begin_pos >= attr_value_begin + 1 ); 

										word = html_text.substr( attr_value_begin, word_begin_pos - attr_value_begin - 1 ); 

										do
										{
											_bstr_t attr_name; 
											_variant_t attr_value; 
											LPCWSTR _text; 
											HRESULT hr; 
											BOOLEAN invalid_type; 

											if( html_element == NULL )
											{
												break; 
											}

											if( last_attr_name.length() == 0 )
											{
												ASSERT( FALSE ); 
												break; 
											}

											attr_name = last_attr_name.c_str(); 

											hr = html_element->getAttribute( attr_name, 2, attr_value.GetAddress() ); 

											if( hr != S_OK )
											{
												break; 
											}
										
											invalid_type = FALSE; 
											switch( attr_value.vt ) //VT_NULL
											{
											case VT_BSTR:
												{
													attr_name = attr_value.bstrVal;
												}
												break; 
											case VT_INT:
											case VT_I4:
												{
													CString text_fmt; 

													text_fmt.Format( L"%d", attr_value.intVal ); 

													attr_name = text_fmt.GetBuffer(); 
												}
												break; 
											case VT_UINT:
											case VT_UI4:
												{
													CString text_fmt; 

													text_fmt.Format( L"%u", attr_value.intVal ); 

													attr_name = text_fmt.GetBuffer(); 
												}
												break; 
											case VT_NULL:
												break; 
											default:
												invalid_type = TRUE; 
												break; 
											}
 
											if( TRUE == invalid_type )
											{
												ASSERT( FALSE ); 
												break; 
											}

											_text = attr_name.operator const wchar_t*(); 
											if( NULL == _text )
											{
												break; 
											}

											if( 0 != wcscmp( _text, word.c_str() ) )
											{
												word = _text; 
											}
										}while( FALSE ); 
										
										ret = fix_html_attribute_format( word, word ); 
										if( ret = ERROR_SUCCESS )
										{
											break; 
										}

										_html_text += word; 

										if( L' ' != _html_text.at( _html_text.length() - 1 ) )
										{
											_html_text += L' '; 
										}

										word = html_text.substr( word_begin_pos, i - word_begin_pos ); 

										wstring_tolower( word ); 
										last_attr_name = word; 

										//word = wcslwr( word.c_str() ); 

										_html_text += word; 
										_html_text += html_text.at( i ); 
										word_begin_pos = i + 1; 
										attr_value_begin = i + 1; 

										text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
									}
								}
								break; 
							case HTML_TEXT_BEGIN: 
								// = 
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//<A a1=
								{
									ASSERT( word_begin_pos >= 0 ); 
									ASSERT( i >= word_begin_pos );  

									if( L' ' != _html_text.at( _html_text.length() - 1 ) )
									{
										_html_text += L' '; 
									}

									word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
									wstring_tolower( word ); 
									last_attr_name = word; 

									_html_text += word; 
									_html_text += html_text.at( i ); 
									word_begin_pos = i + 1; 
									attr_value_begin = i + 1; 

									text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
								}
								break; 
							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
								//<A a1="=
								//log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								//ret = ERROR_INVALID_PARAMETER; 
								if( i < word_begin_pos )
								{
									ASSERT( FALSE ); 
									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}

								break; 
							default:
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
						}
						break; 
					case L' ':
						{
							switch( text_state )
							{
							case HTML_TEXT_BEGIN:  
								//   <A
								word_begin_pos = i + 1; 
								break; 

							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
								//<A a1= 
								{
									if( i < word_begin_pos )
									{
										ASSERT( FALSE ); 
										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}
									else if( i == word_begin_pos )
									{
										word_begin_pos = i + 1; 
									}
									else if( i - word_begin_pos )
									{
										ASSERT( word_begin_pos >= 0 ); 
										ASSERT( i >= word_begin_pos );  

										word_begin_pos = i + 1; 

										//word = html_text.substr( word_begin_pos, i - word_begin_pos ); 

										//ret = fix_html_attribute_format( word, word ); 
										//if( ret = ERROR_SUCCESS )
										//{
										//	break; 
										//}

										//_html_text += word; 
										//_html_text += html_text.at( i ); 

										//word_begin_pos = i + 1; 
										text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
									}
								}
								break; 
							case HTML_TAG_BEGIN: 
								if( i < word_begin_pos )
								{
									ASSERT( FALSE ); 
									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}
								else if( i == word_begin_pos )
								{
									word_begin_pos = i + 1; 
								}
								else if( i - word_begin_pos )
								{
									ASSERT( word_begin_pos >= 0 ); 
									ASSERT( i >= word_begin_pos );  

									//TAG
									word = html_text.substr( word_begin_pos, i - word_begin_pos ); 

									_html_text += word; 

									_html_text += html_text.at( i ); 

									word_begin_pos = i + 1; 
									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 

									switch( word_state )
									{
									case HTML_TAG_LEFT: 
										{
											word_info = new HTML_WORD_INFORMATION(); 
											
											if( NULL == word_info )
											{
												ret = ERROR_NOT_ENOUGH_MEMORY; 
												break; 
											}

											word_info->word = word; 
											word_info->word_end = _html_text.length(); 
											word_info->left_end = -1; 

											tags.push_back( word_info ); 
											word_info = NULL; 
										}
										break; 

									case HTML_TAG_RIGHT: 
									default:
										ASSERT( FALSE ); 
										break; 
									}
								}

								break; 
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//<A   a1=
								{
									if( i < word_begin_pos )
									{
										ASSERT( FALSE ); 
										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}
									else if( i == word_begin_pos )
									{
										word_begin_pos = i + 1; 
									}
									else if( i - word_begin_pos )
									{
										ASSERT( word_begin_pos >= 0 ); 
										ASSERT( i >= word_begin_pos );  

										_html_text += html_text.substr( word_begin_pos, i - word_begin_pos ); 

										_html_text += html_text.at( i ); 

										word_begin_pos = i + 1; 
										text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
									}
								}
								break; 
							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
								if( i < word_begin_pos )
								{
									ASSERT( FALSE ); 
									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}
								else if( i == word_begin_pos )
								{
									word_begin_pos = i + 1; 
								}
								//else if( i - word_begin_pos )
								//{
								//}
								break; 
							default:
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
							break; 
					case L'\"':
						{
							switch( text_state )
							{
							case HTML_TEXT_BEGIN:  
								//   "
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 

							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
								//<A a1= "
								_html_text += html_text.at( i ); 
								word_begin_pos = i + 1; 
								text_state = HTML_ATTIBUTE_STRING_VALUE_BEGIN; 
								break; 
							case HTML_TAG_BEGIN: 
								//<A "
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
								//<A   a1"=
								{
									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
									ret = ERROR_INVALID_PARAMETER; 
									break; 
								}
								break; 
							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
								if( i < word_begin_pos )
								{
									ASSERT( FALSE ); 
									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}
								else if( i == word_begin_pos )
								{
									ASSERT( word_begin_pos >= 0 ); 
									ASSERT( i >= word_begin_pos );  

									_html_text += html_text.at( i ); 

									word_begin_pos = i + 1; 
									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
								}
								else if( i - word_begin_pos )
								{
									ASSERT( word_begin_pos >= 0 ); 
									ASSERT( i >= word_begin_pos );  

									_html_text += html_text.substr( word_begin_pos, i - word_begin_pos ); 

									_html_text += html_text.at( i ); 

									word_begin_pos = i + 1; 
									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
								}
								break; 
							default:
								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
								ret = ERROR_INVALID_PARAMETER; 
								break; 
							}
						}
						break; 
					default:
						break; 
						}

						if( ret != ERROR_SUCCESS )
						{
							break; 
						}
					}

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					i ++; 
				}
			}while( FALSE ); 

			if( NULL != _param)
			{
				free( _param ); 
			}

			{
				HTML_WORD_ITERATOR it; 

				if( ret != ERROR_SUCCESS )
				{
					//partial fixing, that's meaningless.
					break; 
				}

				for( it = bug_tags.begin(); it != bug_tags.end(); it ++ )
				{
					if( ( *it )->left_end < 0 )
					{
						ASSERT( FALSE ); 
						break; 
					}

					ASSERT( ( *it )->left_end < ( LONG )_html_text.length() ); 

					_html_text.insert( _html_text.begin() + ( *it )->left_end, L'/' ); 
				}

				//<A a1="1"> <B b1="1"></B> the A will remain.
				for( it = tags.begin(); it != tags.end(); it ++ )
				{
					do 
					{
						if( ( *it )->left_end < 0 )
						{
							ASSERT( FALSE ); 
							break; 
						}

						ASSERT( ( *it )->left_end < ( LONG )_html_text.length() ); 

						_html_text.insert( _html_text.begin() + ( *it )->left_end, L'/' ); 
					}while( FALSE );
				}
			}
		}while( FALSE ); 

		//do 
		//{
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		break; 
		//	}

		//	if( _html_text.length() < 2 )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}

		//	xml_node_end = wcsrchr( _html_text.c_str(), XML_ELEMENT_END_CHAR ); 
		//	if( xml_node_end == NULL )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}

		//	if( *( xml_node_end - 1 ) != L'/' )
		//	{
		//		html_text_out = _html_text.substr( 0, xml_node_end - _html_text.c_str() ); 
		//		html_text_out += L"/"; 
		//		html_text_out += L">"; 
		//	}
		//	else
		//	{
		html_text_out = _html_text; 
		//	}
		//}while( FALSE ); 
	}while( FALSE );

	return ret; 
}

/****************************************************************************************
<IFRAME tabIndex="2" id="e_iframe" class="pt" hideFocus style="HEIGHT: 400px" frameBorder="0"></IFRAME/>
add third character index for the attribute value begin, 
<BODY spellcheck="false" contentEditable="true" style="BORDER-TOP: 0px; HEIGHT: 400px; BORDER-RIGHT: 0px; BORDER-BOTTOM: 0px; BORDER-LEFT: 0px"></BODY/>
****************************************************************************************/

typedef vector<HTML_WORD_INFORMATION*> HTML_WORDS; 
typedef vector<HTML_WORD_INFORMATION*>::iterator HTML_WORD_ITERATOR; 
typedef vector<HTML_WORD_INFORMATION*>::reverse_iterator HTML_WORD_REVERSE_ITERATOR; 

/************************************************************************
fix tag end process:
check the right tag whether is match top tag, if is then the format is 
regular:
<a><b></b></a>
is not then it's not regular:
<a><b></a>
then record the skipped words, appending it a '/'
and pop the word from the stack.
************************************************************************/

//VOID WINAPI wstring_tolower( wstring &dest_string )
//{
//	wstring::iterator it; 
//
//	for( it = dest_string.begin(); it != dest_string.end(); it ++ )
//	{
//		if( ( *it ) >= L'A' 
//			&& ( *it ) <= L'Z' )
//		{
//			( *it ) = ( *it ) + ( 'a' - 'A' ); 
//		}
//	}
//}

/**********************************************************************************
bug html text: 
<INPUT id="u_login_cookie CHECKED" type="checkbox" value="2" name="CookieDate"/> CHECKED???
//*[@id="u_login_cookie"]
<input type="checkbox" id="u_login_cookie" name="CookieDate" value="2">
**********************************************************************************/

/********************************************************************************
reference:
HtmlAgilityPack
JDOM
SgmlReader
XHTML 
EfTidy
Winista.Htmlparser
LibXml2
document.evaluate( “count(//a[@href])“, document, null, XPathResult.NUMBER_TYPE, null).getNumberValue();
********************************************************************************/
/*******************************************************************************
createDocumentFromUrl
documentElement
execCommand
IHTMLDocument2::elementFromPoint
GetIDsOfNames
*******************************************************************************/

//LRESULT WINAPI format_html_text( wstring html_text, wstring &html_text_out, IHTMLElement *html_element )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	//LPCWSTR xml_node_end; 
//	wstring _html_text; 
//	wstring last_attr_name; 
////#define MAX_XML_TEXT_LENGTH 256
//	//LPWSTR sub_string[ MAX_XML_TEXT_LENGTH ]; 
//
//	//IHTMLDocument2 test_doc; 
//	//IHTMLElement4 test_element; 
//
//	do 
//	{
//		if( html_text.length() <= 0 )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		do 
//		{
//			wstring word; 
//			HTML_TEXT_STATE text_state; 
//			HTML_WORD_STATE word_state; 
//			LONG attr_value_begin; 
//			LONG word_begin_pos; 
//			LPWSTR _param = NULL; 
//			HTML_WORDS tags; 
//			HTML_WORDS bug_tags; 
//			HTML_WORD_INFORMATION *word_info = NULL;  
//			//LPWSTR param_begin; 
//			//LPWSTR param_end; 
//			//LPWSTR param_name; 
//			//LPWSTR param_value; 
//			LONG i; 
//
//			do 
//			{
//				_param = ( LPWSTR )malloc( ( html_text.length() + 1 ) << 1 ); 
//				if( NULL == _param )
//				{
//					ret = ERROR_NOT_ENOUGH_MEMORY; 
//					break; 
//				}
//
//				memcpy( _param, html_text.c_str(), ( ( html_text.length() + 1 ) << 1 ) ); 
//
//				text_state = HTML_TEXT_BEGIN; 
//
//				word_begin_pos = -1; 
//				i = 0; 
//
//				for( ; ; )
//				{
//					if( i >= ( LONG )html_text.length() )
//					{
//						if( text_state != HTML_TEXT_BEGIN )
//						{
//							ret = ERROR_INVALID_PARAMETER; 
//						}
//						break; 
//					}
//
//					//if( i == 1196 )
//					//{
//					//	DebugBreak(); 
//					//}
//
//					switch( _param[ i ] )
//					{
//					case L'/':
//						{
//							switch( text_state )
//							{
//							case HTML_TAG_BEGIN: 
//								ASSERT( i >= word_begin_pos ); 
//								word_state = HTML_TAG_RIGHT; 
//								//word_begin_pos = i + 1; 
//								break; 
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//								break; 
//							case HTML_TEXT_BEGIN: 
//								//<A< ERROR
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//<A a1< ERROR
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//							default:
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//						}
//						break; 
//					case L'<':
//						{
//							switch( text_state )
//							{
//							case HTML_TEXT_BEGIN: 
//								ASSERT( i >= word_begin_pos ); 
//
//								text_state = HTML_TAG_BEGIN; 
//								word_state = HTML_TAG_LEFT; 
//
//								word_begin_pos = i + 1; 
//								_html_text += html_text.at( i ); 
//								break; 
//							case HTML_TAG_BEGIN: 
//
//								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//								if( 0 == wcsnicmp( word.c_str(), L"!--", CONST_STR_LEN( L"!--" ) ) )
//								{
//									LPCWSTR comment_end; 
//								
//									comment_end = wcsstr( html_text.c_str() + word_begin_pos, L"-->" ); 
//									if( comment_end == NULL )
//									{
//										ret = ERROR_INVALID_PARAMETER; 
//										break; 
//									}
//
//									_html_text += html_text.substr( word_begin_pos, comment_end - html_text.c_str() - word_begin_pos + 1 ); 
//									word_begin_pos = comment_end - html_text.c_str() + 1; 
//								
//									text_state = HTML_TEXT_BEGIN; 
//								}
//
//								break; 
//								//<A< ERROR
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//<A a1< ERROR
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//							default:
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//						}
//						break; 
//					case L'>':
//						{
//							switch( text_state )
//							{
//							case HTML_TAG_BEGIN:  
//								//<A a1=1 a2=2 > can receive the entire string in the element now. (A a1=1 a2=2)
//								ASSERT( word_begin_pos >= 0 ); 
//								ASSERT( i >= word_begin_pos );  
//
//								//TAG
//								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//								_html_text += word; 
//
//								switch( word_state )
//								{
//								case HTML_TAG_LEFT: 
//									{
//										word_info = new HTML_WORD_INFORMATION(); 
//
//										if( NULL == word_info )
//										{
//											ret = ERROR_NOT_ENOUGH_MEMORY; 
//											break; 
//										}
//
//										word_info->word = word; 
//										word_info->word_end = _html_text.length(); 
//										word_info->left_end = _html_text.length() + 1; 
//
//										tags.push_back( word_info ); 
//										word_info = NULL; 
//									}
//									break; 
//
//								case HTML_TAG_RIGHT:
//									{
//										/******************************
//										<a><br><c></a>
//										******************************/
//
//										word = word.substr( 1, word.length() - 1 ); 
//
//										ret = fix_html_tag_end( word, &tags, &bug_tags ); 
//										if( ret != ERROR_SUCCESS )
//										{
//											dbg_print( MSG_FATAL_ERROR, "check the tag end error %u\n", ret ); 
//										}
//									}
//									break; 
//								default:
//									ASSERT( FALSE ); 
//									break; 
//								}
//
//								_html_text += html_text.at( i ); 
//
//								word_begin_pos = i + 1; 
//								text_state = HTML_TEXT_BEGIN; 
//								break; 
//
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//								ASSERT( word_begin_pos >= 0 ); 
//								ASSERT( i >= word_begin_pos ); 
//
//								//<a a=b c d >
//								word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//								ret = fix_html_attribute_format( word, word ); 
//
//								_html_text += word; 
//
//								{
//									HTML_WORD_ITERATOR it; 
//
//									it = tags.end() - 1; 
//
//									( *it )->left_end = _html_text.length(); 
//								}
//
//								_html_text += html_text.at( i ); 
//
//								word_begin_pos = i + 1; 
//								text_state = HTML_TEXT_BEGIN; 
//								break; 
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//< TEST > 
//								//<TEST A="A" >
//
//								{
//									HTML_WORD_ITERATOR it; 
//
//									it = tags.end() - 1; 
//
//									( *it )->left_end = _html_text.length(); 
//
//									_html_text += html_text.at( i ); 
//
//									word_begin_pos = i + 1; 
//									text_state = HTML_TEXT_BEGIN; 
//								}
//								break; 
//							case HTML_TEXT_BEGIN: 
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							default:
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//						}
//						break; 
//					case L'=':
//						{
//							switch( text_state )
//							{
//							case HTML_TAG_BEGIN:  
//								//<A = 
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//								//<A a1= = 
//								//<A a1= 
//								//log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								//ret = ERROR_INVALID_PARAMETER; 
//								ASSERT( word_begin_pos >= 0 ); 
//								ASSERT( i >= word_begin_pos );  
//
//								//<A a1=a hello a2 = X
//								//<A a1=a hello a2= 
//
//								{
//									ASSERT( word_begin_pos >= 0 ); 
//									ASSERT( i >= word_begin_pos );  
//
//									if( i < word_begin_pos )
//									{
//										ASSERT( FALSE ); 
//										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//										ret = ERROR_ERRORS_ENCOUNTERED; 
//										break; 
//									}
//									else if( i == word_begin_pos )
//									{
//										ret = ERROR_ERRORS_ENCOUNTERED; 
//										break; 
//									}
//									else if( i - word_begin_pos )
//									{
//										//if( L' ' != _html_text.at( _html_text.length() - 1 ) )
//										//{
//										//	_html_text += L' '; 
//										//}
//
//										ASSERT( attr_value_begin >= 0 ); 
//
//										if( word_begin_pos < attr_value_begin )
//										{
//											ASSERT( FALSE ); 
//											ret = ERROR_ERRORS_ENCOUNTERED; 
//											break; 
//										}
//										else if( word_begin_pos == attr_value_begin )
//										{
//											//<A a1= a2= X
//											ASSERT( FALSE ); 
//											ret = ERROR_ERRORS_ENCOUNTERED; 
//											break; 
//										}
//
//										ASSERT( word_begin_pos >= attr_value_begin + 1 ); 
//
//										word = html_text.substr( attr_value_begin, word_begin_pos - attr_value_begin - 1 ); 
//
//										do
//										{
//											_bstr_t attr_name; 
//											_variant_t attr_value; 
//											LPCWSTR _text; 
//											HRESULT hr; 
//											BOOLEAN invalid_type; 
//
//											if( html_element == NULL )
//											{
//												break; 
//											}
//
//											if( last_attr_name.length() == 0 )
//											{
//												ASSERT( FALSE ); 
//												break; 
//											}
//
//											attr_name = last_attr_name.c_str(); 
//
//											hr = html_element->getAttribute( attr_name, 2, attr_value.GetAddress() ); 
//
//											if( hr != S_OK )
//											{
//												break; 
//											}
//										
//											invalid_type = FALSE; 
//											switch( attr_value.vt ) //VT_NULL
//											{
//											case VT_BSTR:
//												{
//													attr_name = attr_value.bstrVal;
//												}
//												break; 
//											case VT_INT:
//											case VT_I4:
//												{
//													CString text_fmt; 
//
//													text_fmt.Format( L"%d", attr_value.intVal ); 
//
//													attr_name = text_fmt.GetBuffer(); 
//												}
//												break; 
//											case VT_UINT:
//											case VT_UI4:
//												{
//													CString text_fmt; 
//
//													text_fmt.Format( L"%u", attr_value.intVal ); 
//
//													attr_name = text_fmt.GetBuffer(); 
//												}
//												break; 
//											case VT_NULL:
//												break; 
//											default:
//												invalid_type = TRUE; 
//												break; 
//											}
// 
//											if( TRUE == invalid_type )
//											{
//												ASSERT( FALSE ); 
//												break; 
//											}
//
//											_text = attr_name.operator const wchar_t*(); 
//											if( NULL == _text )
//											{
//												break; 
//											}
//
//											if( 0 != wcscmp( _text, word.c_str() ) )
//											{
//												word = _text; 
//											}
//										}while( FALSE ); 
//										
//										ret = fix_html_attribute_format( word, word ); 
//										if( ret = ERROR_SUCCESS )
//										{
//											break; 
//										}
//
//										_html_text += word; 
//
//										if( L' ' != _html_text.at( _html_text.length() - 1 ) )
//										{
//											_html_text += L' '; 
//										}
//
//										word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//
//										wstring_tolower( word ); 
//										last_attr_name = word; 
//
//										//word = wcslwr( word.c_str() ); 
//
//										_html_text += word; 
//										_html_text += html_text.at( i ); 
//										word_begin_pos = i + 1; 
//										attr_value_begin = i + 1; 
//
//										text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
//									}
//								}
//								break; 
//							case HTML_TEXT_BEGIN: 
//								// = 
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//<A a1=
//								{
//									ASSERT( word_begin_pos >= 0 ); 
//									ASSERT( i >= word_begin_pos );  
//
//									if( L' ' != _html_text.at( _html_text.length() - 1 ) )
//									{
//										_html_text += L' '; 
//									}
//
//									word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//									wstring_tolower( word ); 
//									last_attr_name = word; 
//
//									_html_text += word; 
//									_html_text += html_text.at( i ); 
//									word_begin_pos = i + 1; 
//									attr_value_begin = i + 1; 
//
//									text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
//								}
//								break; 
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//								//<A a1="=
//								//log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								//ret = ERROR_INVALID_PARAMETER; 
//								if( i < word_begin_pos )
//								{
//									ASSERT( FALSE ); 
//									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//									ret = ERROR_ERRORS_ENCOUNTERED; 
//									break; 
//								}
//
//								break; 
//							default:
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//						}
//						break; 
//					case L' ':
//						{
//							switch( text_state )
//							{
//							case HTML_TEXT_BEGIN:  
//								//   <A
//								word_begin_pos = i + 1; 
//								break; 
//
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//								//<A a1= 
//								{
//									if( i < word_begin_pos )
//									{
//										ASSERT( FALSE ); 
//										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//										ret = ERROR_ERRORS_ENCOUNTERED; 
//										break; 
//									}
//									else if( i == word_begin_pos )
//									{
//										word_begin_pos = i + 1; 
//									}
//									else if( i - word_begin_pos )
//									{
//										ASSERT( word_begin_pos >= 0 ); 
//										ASSERT( i >= word_begin_pos );  
//
//										word_begin_pos = i + 1; 
//
//										//word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//
//										//ret = fix_html_attribute_format( word, word ); 
//										//if( ret = ERROR_SUCCESS )
//										//{
//										//	break; 
//										//}
//
//										//_html_text += word; 
//										//_html_text += html_text.at( i ); 
//
//										//word_begin_pos = i + 1; 
//										text_state = HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN; 
//									}
//								}
//								break; 
//							case HTML_TAG_BEGIN: 
//								if( i < word_begin_pos )
//								{
//									ASSERT( FALSE ); 
//									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//									ret = ERROR_ERRORS_ENCOUNTERED; 
//									break; 
//								}
//								else if( i == word_begin_pos )
//								{
//									word_begin_pos = i + 1; 
//								}
//								else if( i - word_begin_pos )
//								{
//									ASSERT( word_begin_pos >= 0 ); 
//									ASSERT( i >= word_begin_pos );  
//
//									//TAG
//									word = html_text.substr( word_begin_pos, i - word_begin_pos ); 
//
//									_html_text += word; 
//
//									_html_text += html_text.at( i ); 
//
//									word_begin_pos = i + 1; 
//									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
//
//									switch( word_state )
//									{
//									case HTML_TAG_LEFT: 
//										{
//											word_info = new HTML_WORD_INFORMATION(); 
//											
//											if( NULL == word_info )
//											{
//												ret = ERROR_NOT_ENOUGH_MEMORY; 
//												break; 
//											}
//
//											word_info->word = word; 
//											word_info->word_end = _html_text.length(); 
//											word_info->left_end = -1; 
//
//											tags.push_back( word_info ); 
//											word_info = NULL; 
//										}
//										break; 
//
//									case HTML_TAG_RIGHT: 
//									default:
//										ASSERT( FALSE ); 
//										break; 
//									}
//								}
//
//								break; 
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//<A   a1=
//								{
//									if( i < word_begin_pos )
//									{
//										ASSERT( FALSE ); 
//										log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//										ret = ERROR_ERRORS_ENCOUNTERED; 
//										break; 
//									}
//									else if( i == word_begin_pos )
//									{
//										word_begin_pos = i + 1; 
//									}
//									else if( i - word_begin_pos )
//									{
//										ASSERT( word_begin_pos >= 0 ); 
//										ASSERT( i >= word_begin_pos );  
//
//										_html_text += html_text.substr( word_begin_pos, i - word_begin_pos ); 
//
//										_html_text += html_text.at( i ); 
//
//										word_begin_pos = i + 1; 
//										text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
//									}
//								}
//								break; 
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//								if( i < word_begin_pos )
//								{
//									ASSERT( FALSE ); 
//									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//									ret = ERROR_ERRORS_ENCOUNTERED; 
//									break; 
//								}
//								else if( i == word_begin_pos )
//								{
//									word_begin_pos = i + 1; 
//								}
//								//else if( i - word_begin_pos )
//								//{
//								//}
//								break; 
//							default:
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//							break; 
//					case L'\"':
//						{
//							switch( text_state )
//							{
//							case HTML_TEXT_BEGIN:  
//								//   "
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//
//							case HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN: 
//								//<A a1= "
//								_html_text += html_text.at( i ); 
//								word_begin_pos = i + 1; 
//								text_state = HTML_ATTIBUTE_STRING_VALUE_BEGIN; 
//								break; 
//							case HTML_TAG_BEGIN: 
//								//<A "
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							case HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END: 
//								//<A   a1"=
//								{
//									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//									ret = ERROR_INVALID_PARAMETER; 
//									break; 
//								}
//								break; 
//							case HTML_ATTIBUTE_STRING_VALUE_BEGIN: 
//								if( i < word_begin_pos )
//								{
//									ASSERT( FALSE ); 
//									log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//									ret = ERROR_ERRORS_ENCOUNTERED; 
//									break; 
//								}
//								else if( i == word_begin_pos )
//								{
//									ASSERT( word_begin_pos >= 0 ); 
//									ASSERT( i >= word_begin_pos );  
//
//									_html_text += html_text.at( i ); 
//
//									word_begin_pos = i + 1; 
//									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
//								}
//								else if( i - word_begin_pos )
//								{
//									ASSERT( word_begin_pos >= 0 ); 
//									ASSERT( i >= word_begin_pos );  
//
//									_html_text += html_text.substr( word_begin_pos, i - word_begin_pos ); 
//
//									_html_text += html_text.at( i ); 
//
//									word_begin_pos = i + 1; 
//									text_state = HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END; 
//								}
//								break; 
//							default:
//								log_trace_ex( MSG_FATAL_ERROR, "format html text error\n" ); 
//								ret = ERROR_INVALID_PARAMETER; 
//								break; 
//							}
//						}
//						break; 
//					default:
//						break; 
//						}
//
//						if( ret != ERROR_SUCCESS )
//						{
//							break; 
//						}
//					}
//
//					if( ret != ERROR_SUCCESS )
//					{
//						break; 
//					}
//
//					i ++; 
//				}
//			}while( FALSE ); 
//
//			{
//				HTML_WORD_ITERATOR it; 
//
//				if( ret != ERROR_SUCCESS )
//				{
//					//partial fixing, that's meaningless.
//					break; 
//				}
//
//				for( it = bug_tags.begin(); it != bug_tags.end(); it ++ )
//				{
//					if( ( *it )->left_end < 0 )
//					{
//						ASSERT( FALSE ); 
//						break; 
//					}
//
//					ASSERT( ( *it )->left_end < ( LONG )_html_text.length() ); 
//
//					_html_text.insert( _html_text.begin() + ( *it )->left_end, L'/' ); 
//				}
//
//				//<A a1="1"> <B b1="1"></B> the A will remain.
//				for( it = tags.begin(); it != tags.end(); it ++ )
//				{
//					do 
//					{
//						if( ( *it )->left_end < 0 )
//						{
//							ASSERT( FALSE ); 
//							break; 
//						}
//
//						ASSERT( ( *it )->left_end < ( LONG )_html_text.length() ); 
//
//						_html_text.insert( _html_text.begin() + ( *it )->left_end, L'/' ); 
//					}while( FALSE );
//				}
//			}
//		}while( FALSE ); 
//
//		//do 
//		//{
//		//	if( ret != ERROR_SUCCESS )
//		//	{
//		//		break; 
//		//	}
//
//		//	if( _html_text.length() < 2 )
//		//	{
//		//		ret = ERROR_INVALID_PARAMETER; 
//		//		break; 
//		//	}
//
//		//	xml_node_end = wcsrchr( _html_text.c_str(), XML_ELEMENT_END_CHAR ); 
//		//	if( xml_node_end == NULL )
//		//	{
//		//		ret = ERROR_INVALID_PARAMETER; 
//		//		break; 
//		//	}
//
//		//	if( *( xml_node_end - 1 ) != L'/' )
//		//	{
//		//		html_text_out = _html_text.substr( 0, xml_node_end - _html_text.c_str() ); 
//		//		html_text_out += L"/"; 
//		//		html_text_out += L">"; 
//		//	}
//		//	else
//		//	{
//		html_text_out = _html_text; 
//		//	}
//		//}while( FALSE ); 
//	}while( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI get_html_element_input_value( IHTMLElement *html_element, wstring &input_value, BOOLEAN &is_input )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	_bstr_t text; 
	LPCWSTR _text; 
	HTML_ELEMENT_TYPE element_type; 

	do 
	{
		ASSERT( NULL != html_element ); 

		input_value = L""; 

		hr = html_element->get_tagName( text.GetAddress() ); 
		if( S_OK != hr )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_text = text.operator const wchar_t*(); 
		if( NULL == _text )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		element_type = get_html_element_type_from_tag( _text ); 

		is_input = FALSE; 
		switch( element_type )
		{
		case HTML_INPUT_ELEMENT: 
			{
				IHTMLInputTextElementPtr input_text_element; 
				IHTMLInputElementPtr input_element; 

				hr = html_element->QueryInterface( IID_IHTMLInputElement, ( PVOID* )&input_element ); 
				if( S_OK != hr )
				{
					break; 
				}

				if( NULL == input_element )
				{
					break; 
				}

				do 
				{
					VARIANT_BOOL checked; 
					hr = input_element->get_type( text.GetAddress() ); 

					if( FAILED( hr ) )
					{
						break; 
					}

					_text = text.operator const wchar_t*(); 
					if( NULL == _text )
					{
						break; 
					}

					if( 0 != _wcsicmp( _text, L"checkbox" ) )
					{
						break; 
					}

					input_value = L""; 

					hr = input_element->get_checked( &checked ); 

					if( FAILED( hr ) )
					{
						break; 
					}

					if( checked == VARIANT_TRUE )
					{
						input_value = L"checked"; 
					}

				}while( FALSE );

				hr = html_element->QueryInterface( IID_IHTMLInputTextElement, ( PVOID* )&input_text_element ); 
				if( S_OK != hr )
				{
					break; 
				}

				if( NULL == input_text_element )
				{
					break; 
				}

				is_input = TRUE; 
				input_text_element->get_value( text.GetAddress() ); 

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				input_value = _text; 
			}
			break; 
		case HTML_BUTTON_ELEMENT: 
			break; 
		case HTML_A_ELEMENT: 
			break; 
		case HTML_BODY_ELEMENT: 
			{
				MSXML::IXMLDOMAttributePtr xml_attr; 

				is_input = TRUE; 
				hr = html_element->get_innerHTML( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

				input_value = _text; 
			}
			break; 
		case HTML_DIV_ELEMENT:
			{
				is_input = TRUE; 

				hr = html_element->get_innerText( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

			}
			break; 
		case HTML_TEXT_AREA_ELEMENT:
			{
				is_input = TRUE; 

				hr = html_element->get_innerText( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

			}
			break; 
		case HTML_SELECT_ELEMENT:
			{

			}
			break; 
		default:
			ASSERT( FALSE ); 
			log_trace_ex( MSG_IMPORTANT, "unknown HTML element: %s\n", _text ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI add_html_element_config(  MSXML::IXMLDOMDocument *xml_doc, 
									   LPCWSTR url, 
									   MSXML::IXMLDOMNode *page_node, 
									   IHTMLElement *html_element ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr _xml_doc; 
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;
	MSXML::IXMLDOMElementPtr dest_element; 
	MSXML::IXMLDOMTextPtr xml_text; 

	IHTML_ELEMENTS html_elements; 

	wstring xpath; 

	_bstr_t text; 
	LPCWSTR _text; 

	//_bstr_t src_xml; 
	//LPCWSTR _src_xml; 

	wstring html_text; 
	BOOLEAN in_frame = FALSE; 

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != xml_doc );
		ASSERT(NULL != url );
		ASSERT(NULL != page_node ); 
		ASSERT(NULL != html_element); 

		_xml_doc = xml_doc; 

		//get_info_type()

		html_element->AddRef(); 
		html_elements.push_back( html_element ); 

		for( ; ; )
		{
			hr = html_element->get_tagName( text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
				break; 
			}

			if( 0 == _wcsicmp( _text, L"iframe" ) 
				|| 0 == _wcsicmp( _text, L"frame" ) )
			{
				//IHTMLIFrameElement2 html_iframe; 
				IHTMLFrameBase2Ptr html_frame; 
				IHTMLWindow2Ptr html_window; 
				IHTMLDocument2Ptr html_doc; 
				IHTMLElementPtr html_sub_element; 
				//IHTMLFrameElement2Ptr html_frame; 

				in_frame = TRUE; 

				hr = html_element->QueryInterface( IID_IHTMLFrameBase2, ( PVOID* )&html_frame ); 
				if( FAILED( hr ) ) 
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_frame )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				hr = html_frame->get_contentWindow( &html_window ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_window )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				ret = html_window_2_html_document( html_window, &html_doc ); 
				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

				hr = html_doc->get_activeElement( &html_sub_element ); 
				if( FAILED( hr ) ) 
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_sub_element )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				html_element->Release(); 

				html_element = html_sub_element; 

				html_sub_element->AddRef(); 
				html_elements.push_back( html_sub_element ); 

				html_element->AddRef(); 
			}
			else
			{
				break; 
			}
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		{
			hr = html_element->get_outerHTML( text.GetAddress() ); 
			if( hr != S_OK )
			{
				ret = hr; 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
				break; 
			}
		}

		html_text = _text; 

		ret = format_html_text( html_text, html_text, html_element ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = xml_dest_doc->loadXML( ( WCHAR* )html_text.c_str() ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_text = bstrReason.operator wchar_t*(); 

			if( NULL != _text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws(line%u:%u)\n", _text, 
					spParseError->line, 
					spParseError->linepos ); 
			}

			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_element ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_element )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		//hr = xml_doc->get_xml( src_xml.GetAddress() ); 
		//if( S_OK != hr )
		//{
		//	break; 
		//}

		//_src_xml = src_xml.operator const wchar_t*(); 
		//if( NULL == _src_xml )
		//{
		//	break; 
		//}

		do
		{
			_variant_t attr_value; 
			//MSXML::IXMLDOMAttributePtr xml_attr; 
			//xml_attr = xml_dest_doc->createAttribute( L"in_frame" ); 

			attr_value = in_frame; 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_IN_FRAME, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			//_variant_t attr_value; 
			MSXML::IXMLDOMAttributePtr xml_attr; 

			ret = _get_html_element_xpath_ex( &html_elements, xpath, FIX_XPATH_ERROR ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
				//break; 
			}

			xml_attr = xml_dest_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_XPATH ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = xpath.c_str(); 

			xml_text = xml_dest_doc->createTextNode( text ); 
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
			//attr_value = text; 

			//text = L"XPATH"; 

			//hr = dest_element->setAttributeNode( text, attr_value ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 

		do 
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 
			BOOLEAN is_input; 

			ret = get_html_element_input_value( html_element, xpath, is_input ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
				break; 
			}

			do 
			{
				wstring text_block; 
				wstring text_name; 
				//capture_input_dlg dlg; 
				_bstr_t _text_block; 

				//if( FALSE == is_input )
				//{
				//	text = L""; //xpath.c_str(); 

				//	xml_attr = xml_dest_doc->createAttribute( HTML_ELEMENT_INPUT_VALUE ); 
				//	if( NULL == xml_attr )
				//	{
				//		ret = ERROR_ERRORS_ENCOUNTERED; 
				//		break; 
				//	}
				//	break; 
				//}

				//hr = html_element->get_id( text.GetAddress() ); 
				//if( FAILED( hr ) )
				//{
				//	//break; 
				//}
				//else
				//{
				//	LPCWSTR __text; 
				//	__text = text.operator const wchar_t*(); 
				//	if( NULL != __text )
				//	{
				//		text_name = __text; 
				//		dlg.set_text_name( text_name.c_str() ); 
				//	}
				//}

				//dlg.set_text_block( xpath.c_str() ); 
				//dlg.DoModal(); 

				//xpath = dlg.get_text_name(); 
				//text_block = dlg.get_text_block(); 

				text = xpath.c_str(); 

				xml_attr = xml_dest_doc->createAttribute( INPUT_TEXT_NAME_ATTRIBUTE_NAME ); 
				if( NULL == xml_attr )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text_block = text_block.c_str(); 

				html_element->put_innerText( _text_block.GetBSTR() ); 
			}while( FALSE ); 

			xml_text = xml_dest_doc->createTextNode( text ); 
			if( NULL == xml_text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			//hr = xml_text->get_text( text.GetAddress() ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			//_text = text.operator const wchar_t*(); 

			//if( _text == NULL )
			//{
			//	break; 
			//}

			xml_attr->appendChild( xml_text ); 
			//attr_value = text; 

			//text = L"XPATH"; 

			//hr = dest_element->setAttributeNode( text, attr_value ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			dest_element->setAttributeNode( xml_attr ); 
		} while ( FALSE );

		ret = insert_node_by_text(xml_doc, dest_element);
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	ret = release_html_elements( &html_elements ); 
	if( ERROR_SUCCESS != ret )
	{

	}

	return ret; 
}

LRESULT WINAPI add_html_element_config_ex(  MSXML::IXMLDOMDocument *xml_doc, 
									   HTML_ELEMENT_ACTION *element_config ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr _xml_doc; 
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;
	MSXML::IXMLDOMElementPtr dest_element; 
	MSXML::IXMLDOMTextPtr xml_text; 

	IHTML_ELEMENTS html_elements; 

	_bstr_t text; 
	LPCWSTR _text; 

	//_bstr_t src_xml; 
	//LPCWSTR _src_xml; 

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != xml_doc ); 
		ASSERT(NULL != element_config); 

		_xml_doc = xml_doc; 

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = xml_dest_doc->loadXML( ( WCHAR* )element_config->html_text.c_str() ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_text = bstrReason.operator wchar_t*(); 

			if( NULL != _text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws(line%u:%u)\n", _text, 
					spParseError->line, 
					spParseError->linepos ); 
			}

			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_element ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_element )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		//_src_xml = src_xml.operator const wchar_t*(); 
		//if( NULL == _src_xml )
		//{
		//	break; 
		//}

		do
		{
			_variant_t attr_value; 
			//MSXML::IXMLDOMAttributePtr xml_attr; 
			//xml_attr = xml_dest_doc->createAttribute( L"in_frame" ); 

			attr_value.vt = VT_I4; 
			attr_value.intVal = element_config->in_frame; 

			hr = dest_element->setAttribute( HTML_ELEMENT_ATTRIBUTE_IN_FRAME, attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 
			//MSXML::IXMLDOMAttributePtr xml_attr; 
			//xml_attr = xml_dest_doc->createAttribute( L"contant_type" ); 

			attr_value = element_config->param.c_str(); 

			hr = dest_element->setAttribute( L"contant_type", attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			_variant_t attr_value; 
			//MSXML::IXMLDOMAttributePtr xml_attr; 
			//xml_attr = xml_dest_doc->createAttribute( L"contant_type" ); 

			attr_value = element_config->action.c_str(); 

			hr = dest_element->setAttribute( L"action", attr_value ); 

			if( hr != S_OK )
			{
				break; 
			}
		}while( FALSE ); 

		do
		{
			//_variant_t attr_value; 
			MSXML::IXMLDOMAttributePtr xml_attr; 

			xml_attr = xml_dest_doc->createAttribute( HTML_ELEMENT_ATTRIBUTE_XPATH ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			text = element_config->xpath.c_str(); 

			xml_text = xml_dest_doc->createTextNode( text ); 
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
			//attr_value = text; 

			//text = L"XPATH"; 

			//hr = dest_element->setAttributeNode( text, attr_value ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			dest_element->setAttributeNode( xml_attr ); 
		}while( FALSE ); 

		do 
		{
			MSXML::IXMLDOMAttributePtr xml_attr; 
			//wstring xpath; 
			//BOOLEAN is_input; 

			//ret = get_html_element_input_value( dest_element, xpath, is_input ); 
			//if( ret != ERROR_SUCCESS )
			//{
			//	dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			//	break; 
			//}

			xml_attr = xml_dest_doc->createAttribute( L"xpath" ); 
			if( NULL == xml_attr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			do 
			{
				//wstring text_block; 
				//wstring text_name; 
				////capture_input_dlg dlg; 
				//_bstr_t _text_block; 

				//if( FALSE == is_input )
				//{
				//	text = L""; //xpath.c_str(); 

				//	xml_attr = xml_dest_doc->createAttribute( HTML_ELEMENT_INPUT_VALUE ); 
				//	if( NULL == xml_attr )
				//	{
				//		ret = ERROR_ERRORS_ENCOUNTERED; 
				//		break; 
				//	}
				//	break; 
				//}

				//hr = html_element->get_id( text.GetAddress() ); 
				//if( FAILED( hr ) )
				//{
				//	//break; 
				//}
				//else
				//{
				//	LPCWSTR __text; 
				//	__text = text.operator const wchar_t*(); 
				//	if( NULL != __text )
				//	{
				//		text_name = __text; 
				//		dlg.set_text_name( text_name.c_str() ); 
				//	}
				//}

				//dlg.set_text_block( xpath.c_str() ); 
				//dlg.DoModal(); 

				//xpath = dlg.get_text_name(); 
				//text_block = dlg.get_text_block(); 

				//text = xpath.c_str(); 

				//xml_attr = xml_dest_doc->createAttribute( L"xpath" ); 
				//if( NULL == xml_attr )
				//{
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				//_text_block = text_block.c_str(); 

				//html_element->put_innerText( _text_block.GetBSTR() ); 
			}while( FALSE ); 

			//xml_text = xml_dest_doc->createTextNode( text ); 
			//if( NULL == xml_text )
			//{
			//	ret = ERROR_ERRORS_ENCOUNTERED; 
			//	break; 
			//}

			//hr = xml_text->get_text( text.GetAddress() ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			//_text = text.operator const wchar_t*(); 

			//if( _text == NULL )
			//{
			//	break; 
			//}

			//xml_attr->appendChild( xml_text ); 
			//attr_value = text; 

			//text = L"XPATH"; 

			//hr = dest_element->setAttributeNode( text, attr_value ); 
			//if( hr != S_OK )
			//{
			//	break; 
			//}

			//dest_element->setAttributeNode( xml_attr ); 
		} while ( FALSE );

		ret = insert_node_by_text(xml_doc, dest_element);
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE );

	ret = release_html_elements( &html_elements ); 
	if( ERROR_SUCCESS != ret )
	{

	}

	return ret; 
}

LRESULT WINAPI make_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
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
			ret = ERROR_SUCCESS; 

			file_handle = CreateFileW( config_file_path, 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ, 
				NULL, 
				OPEN_ALWAYS, 
				0, 
				NULL ); 

			if( file_handle == INVALID_HANDLE_VALUE )
			{
				ret = GetLastError(); 
				break; 
			}

            CloseHandle(file_handle); 

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

			ret = create_config_file( config_file_path ); 

		}
		else
		{
#ifdef _DEBUG
			//ret = create_config_file( config_file_path ); 
#else	
			ASSERT( FALSE ); 
#endif //_DEBUG

		}

	}while( FALSE ); 

	//if( INVALID_HANDLE_VALUE != file_handle )
	//{
	//	CloseHandle( file_handle ); 
	//}

	return ret; 
}

LRESULT WINAPI create_config_file( LPCWSTR config_file_name )
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

		text = HTML_SCRIPT_ROOT_ELEMENT_NAME; 
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

        try
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
        catch (...)
        {
            ret = ERROR_ERRORS_ENCOUNTERED; 
            break; 
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

//LRESULT WINAPI release_html_elements( IHTML_ELEMENTS *html_elements )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	IHTML_ELEMENT_ITERATOR it; 
//
//	do 
//	{
//		ASSERT( html_elements != NULL ); 
//
//		for( it = html_elements->begin(); 
//			it != html_elements->end(); 
//			it ++ )
//		{
//			( *it )->Release(); 
//		}
//
//	}while( FALSE );
//
//	html_elements->clear(); 
//	return ret; 
//}

