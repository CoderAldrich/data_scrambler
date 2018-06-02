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
#include "html_common.h"
#import "msxml.tlb"
//#import "mshtml.tlb"
#include "mshtml_addition.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "markup.h"

//static BOOLEAN analyze_xpath_locatin = FALSE; 
static HWND xpath_info_ui_window = NULL; 

#define INTEGER_TEXT_LEN 24

LRESULT WINAPI set_xpath_info_ui_window( HWND window )
{
	xpath_info_ui_window = window; 
	return ERROR_SUCCESS; 
}

HWND WINAPI get_xpath_info_ui_window()
{
	return xpath_info_ui_window; 
}

//LRESULT WINAPI set_xpath_info_analyze( BOOLEAN _switch )
//{
//	analyze_xpath_locatin = _switch; 
//	return ERROR_SUCCESS; 
//}
//
//BOOLEAN WINAPI get_xpath_info_analyze()
//{
//	return analyze_xpath_locatin; 
//}

LRESULT WINAPI get_selected_html_elements( IHTMLDocument2 *html_doc, HTML_ELEMENT_GROUP *elements )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IDispatchPtr disp; 
	IHTMLElementPtr html_element; 
	IHTMLSelectionObjectPtr html_selection; 

	_bstr_t text; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != elements ); 

		hr = html_doc->get_selection( &html_selection ); 
		if( S_OK != hr 
			|| NULL == html_selection )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = html_selection->get_type( text.GetAddress() ); 
		if( FAILED( hr ) 
			|| NULL == text.operator const wchar_t*() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

#define SELECTION_OBJECT_NONE L"None"
#define SELECTION_OBJECT_TEXT L"Text"
#define SELECTION_OBJECT_CONTROL L"Control" //???

		_text = text.operator const wchar_t*(); 

		if( _text == NULL )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		try
		{	
			INT32 i; 
			IHTMLControlRangePtr selected_elements; 
            IDispatchPtr disp; 
			LONG selected_item_count; 

            hr = html_selection->createRange(&disp);
            //html_doc->get_location( ); 
            if (S_OK != hr
                || NULL == disp)
            {
                ret = ERROR_ERRORS_ENCOUNTERED;
                break;
            }

            if (0 == _wcsicmp(_text, SELECTION_OBJECT_CONTROL))
            {
                hr = disp->QueryInterface(IID_IHTMLControlRange, (PVOID*)&selected_elements);
                if (S_OK != hr
                    || NULL == selected_elements)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }

                hr = selected_elements->
                    get_length(&selected_item_count);

                if (hr != S_OK)
                {
                    break;
                }

                for (i = 0; i < selected_item_count; i++)
                {
                    do
                    {
                        hr = selected_elements->item(i, &html_element);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (NULL == html_element)
                        {
                            hr = E_UNEXPECTED;
                            break;
                        }

                        html_element->AddRef();
                        elements->push_back(html_element);
                    } while (FALSE);

                    if (SUCCEEDED(hr))
                    {
                        break;
                    }
                }
            }
            else if (0 == _wcsicmp(_text, SELECTION_OBJECT_TEXT))
            {
                IHTMLTxtRangePtr text_range;

                hr = disp->QueryInterface(IID_IHTMLTxtRange, (PVOID*)&text_range);
                if (S_OK != hr
                    || NULL == text_range)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }

                hr = text_range->parentElement(&html_element); 
                if (S_OK != hr
                    || NULL == html_element)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }
                
                elements->push_back(html_element);

            }
		}
		catch( ... ) 
		{
			ret = ERROR_ERRORS_ENCOUNTERED; //GetExceptionCode(); 
			break; 
		}	

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI release_xpath_params( XPATH_PARAMS **params, ULONG params_count )
{
	LRESULT ret = ERROR_SUCCESS; 

	if( NULL != params )
	{
		ULONG xpath_index; 
		for( xpath_index = 0; xpath_index < params_count; xpath_index ++ )
		{
			ASSERT( params[ xpath_index ] != NULL ); 

			delete params[ xpath_index ]; 
		}
	}

	return ret; 
}


//LRESULT WINAPI release_html_element_groups( HTML_ELEMENT_GROUP groups[] )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	do
//	{
//		for( ULONG type = HTML_ELEMENT_SET_DATA; 
//			type < MAX_HTML_ELEMENT_ACTION; 
//			type ++ )
//		{
//			for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = groups[ type ].begin(); 
//				it != groups[ type ].end(); 
//				it ++ )
//			{
//				( *it )->Release(); 
//			}
//
//			groups[ type ].clear(); 
//		}
//	}while( FALSE ); 
//
//	return ret; 
//}

//LRESULT WINAPI add_html_element_to_groups( IHTMLElement *html_element, 
//										 HTML_ELEMENT_GROUP groups[ MAX_HTML_ELEMENT_ACTION ] )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//	_bstr_t html_element_tag; 
//	LPCWSTR tag_name; 
//
//	do 
//	{
//		hr = html_element->get_tagName( html_element_tag.GetAddress() ); 
//		if( FAILED( hr ) )
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		tag_name = html_element_tag.operator wchar_t*(); 
//		if( NULL == tag_name )
//		{
//			ret = ERROR_NOT_ENOUGH_MEMORY; 
//			break; 
//		}
//
//		dbg_print( MSG_IMPORTANT, "%ws\n", tag_name ); 
//
//		{
//			HTML_ELEMENT_TYPE info_type; 
//			HTML_ELEMENT_ACTION_TYPE action_type; 
//
//			info_type = get_html_element_type_from_tag( tag_name ); 
//			action_type = get_html_element_action_type( info_type ); 
//			if( action_type == UNKNOWN_HTML_ELEMENT_ACTION )
//			{
//				ret = ERROR_INVALID_PARAMETER; 
//				break; 
//			}
//
//			html_element->AddRef(); 
//			groups[ action_type ].push_back( html_element ); 
//		}
//	} while ( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI copy_html_element_group( HTML_ELEMENT_GROUP *group_src, HTML_ELEMENT_GROUP *group_dest )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_GROUP_ELEMENT_ITERATOR group_it; 

	do 
	{
		ASSERT( NULL != group_src ); 
		ASSERT( NULL != group_dest ); 

		for( ; ; )
		{
			group_it = group_src->begin(); 

			if( group_it == group_src->end() )
			{
				break; 
			}

			ret = add_html_element_to_group( ( *group_it ), group_dest ); 
			if( ret != ERROR_SUCCESS )
			{
				group_src->erase( group_it ); 
				( *group_it )->Release(); 
			}
			else
			{
				group_src->erase( group_it ); 
			}
		}
	}while( FALSE );

	return ret; 
}

/********************************************************************************
使用xpath属性来定位一个确定的HTML元素，格式为：
xpath="/body/div/input a="" b"""
查询过程:
取出xpath参数值
使用"/"来分隔出所有的元素文本，同时从元素文本中通过" "来分隔出元素的tag和parameters
从左侧第1个结点开始，进行以下处理过程:
1.从页面的根元素中取出所有子元素get_children, 而不是get_all
2.从页面的子元素中查找name属性相同(TAG相同,其它属性相同)的HTML元素集.
3.对取出的元素或元素集(尽量设置精确parameters,将这种情况最小化,消耗性能,)进行1，2步。
********************************************************************************/

LRESULT WINAPI get_html_children( IHTMLElementCollection *parent_elements, 
								 XPATH_PARAMS *xpath, 
								 HTML_ELEMENT_GROUP *group )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	XPATH_PARAM_ITERATOR param_it; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	HTML_ELEMENT_GROUP _element_group; 

	IHTMLElementPtr sub_element; 

	wstring name; 
	wstring id; 
	wstring tag; 
	LONG child_index; 

	//LPCWSTR _text; 
	wstring text; 
	_bstr_t temp_text; 

	_variant_t _name; 
	_variant_t index; 

	LONG i; 
	LONG element_count; 

	do 
	{
		ASSERT( xpath != NULL ); 
		ASSERT( parent_elements != NULL ); 

		param_it = xpath->find( L"name" ); 

		if ( param_it == xpath->end() )
		{
			name = L""; 
		}
		else
		{
			name = param_it->second.c_str(); 
		}

		param_it = xpath->find( L"id" ); 

		if ( param_it == xpath->end() )
		{
			id = L""; 
		}
		else
		{
			id = param_it->second.c_str();
		}

		param_it = xpath->find( L"tag" ); 

		if ( param_it == xpath->end() )
		{
			tag = L""; 
		}
		else
		{
			tag = param_it->second.c_str(); 
		}

		param_it = xpath->find( HTML_ELEMENT_ORDER_VALUE_NAME ); 

		if ( param_it == xpath->end() )
		{
			child_index = -1; 
		}
		else
		{
			LPWSTR text_end; 
			child_index = wcstol( param_it->second.c_str(), &text_end, 0 ); 

			ASSERT( child_index != -1 ); 
		}

		if( id.length() == 0 
			&& name.length() == 0 )
		{
			temp_text = tag.c_str(); 

			_name = temp_text; 

			hr = parent_elements->tags( _name, &sub_element_disp ); 
			if( S_OK != hr )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}
		else
		{
			if( id.length() )
			{
				temp_text = id.c_str(); 
			}
			else if( name.length() )
			{
				temp_text = name.c_str(); 
			}
			else
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_name = temp_text; 

			V_VT( &index ) = VT_EMPTY;
			V_I4( &index ) = -1; 

			hr = parent_elements->item( _name, index, &sub_element_disp ); 
			if( FAILED( hr ) 
				|| sub_element_disp == NULL )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
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

			hr = sub_elements->get_length( &element_count ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			if( id.length() == 0 
				&& name.length() == 0 )
			{ 
				if( child_index >= 0 )
				{
					if( element_count <= child_index )
					{
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					V_VT( &_name ) = VT_I4;
					V_I4( &_name ) = child_index;
					V_VT( &index ) = VT_I4;
					V_I4( &index ) = 0; 

					hr = sub_elements->item( _name, index, &sub_element_disp ); //Get an element

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
					group->push_back( sub_element ); 
					break; 
				}
			}

			if( element_count == 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			for( i = 0; i < element_count; i ++ )
			{
				V_VT( &_name ) = VT_I4;
				V_I4( &_name ) = i;
				V_VT( &index ) = VT_I4;
				V_I4( &index ) = 0; 

				hr = sub_elements->item( _name, index, &sub_element_disp ); //Get an element

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
				group->push_back( sub_element ); 
			}
		}
		else
		{
			if( id.length() == 0 
				&& name.length() == 0 )
			{
				if( 0 == child_index )
				{
					sub_element->AddRef(); 
					group->push_back( sub_element ); 
				}
				else
				{
					ASSERT( FALSE ); 
				}
			}
			else
			{
				sub_element->AddRef(); 
				group->push_back( sub_element ); 
			}
		}

	}while( FALSE );

	return ret; 
}

HRESULT WINAPI compare_html_element_attributes( XPATH_PARAMS *xpath, 
											   IHTMLElement *element)
{
	HRESULT hr = S_OK; 
	_bstr_t temp_text; 
	_variant_t value; 
	LPCWSTR _text; 

	do 
	{
		ASSERT( NULL != xpath ); 
		ASSERT( NULL != element ); 

		for( XPATH_PARAM_ITERATOR it = xpath->begin(); 
			it != xpath->end(); 
			it ++ )
		{
			do 
			{
				if( 0 == _wcsicmp( L"id", it->first.c_str() ) )
				{
					break; 
				}

				if( 0 == _wcsicmp( L"tag", it->first.c_str() ) )
				{
#ifdef _DEBUG
					if( 0 == _wcsicmp( it->first.c_str(), L"w-root" ) )
					{
						hr = hr; 
					}
#endif //_DEBUG

					break; 
				}

//#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
				if( 0 == _wcsicmp( L"class", it->first.c_str() ) )
				{
					break; 
				}
//#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

				if( 0 == _wcsicmp( HTML_ELEMENT_ORDER_VALUE_NAME, it->first.c_str() ) )
				{
					break; 
				}

				if( 0 == wcscmp( HTML_ELEMENT_TEXT_PROPERTY_NAME, 
					it->first.c_str() ) )
				{
					hr = element->get_innerText( temp_text.GetAddress() ); 
					if( FAILED( hr ) )
					{
						dbg_print( MSG_FATAL_ERROR, "this xpath of the target element have text property(%ws) but can't get the inner text from the possible element\n", 
							it->second.c_str() ); 

						if( it->second.length() != 0 )
						{
							break; 
						}

						hr = S_OK; 
					}

					_text = temp_text.operator const wchar_t*(); 
					if( NULL == _text 
						|| temp_text.length() == 0 )
					{
						if( it->second.length() != 0 )
						{
							hr = E_INVALIDARG; 
							break; 
						}
					}
					else
					{
						{
							//wstring escaped_text; 

							//escaped_text = it->second.c_str(); 

							//escape_html_text( escaped_text ); 
							if( 0 != _wcsicmp( _text, it->second.c_str() ) )
							{
								hr = E_INVALIDARG; 
								break; 
							}
							else
							{
								hr = S_OK; 
							}
						}
					}

					break; 
				}

				//_text = it->first.c_str(); 
				//if( 0 == wcscmp( _text, HTML_ELEMENT_TEXT_PROPERTY_NAME ) )
				//{
				//	hr = element->get_innerText( temp_text.GetAddress() ); 
				//	if( FAILED( hr ) )
				//	{
				//		dbg_print( MSG_FATAL_ERROR, "this xpath of the target element have text property(%ws) but can't get the inner text from the possible element\n", 
				//			it->second.c_str() ); 
				//		if( it->first.length() != 0 )
				//		{
				//			break; 
				//		}

				//		hr = S_OK; 
				//	}

				//	_text = temp_text.operator const wchar_t*(); 
				//	if( NULL == _text || temp_text.length() == 0 )
				//	{
				//		if( it->first.length() != 0 )
				//		{
				//			hr = E_INVALIDARG; 
				//			break; 
				//		}
				//	}
				//	else
				//	{
				//		if( 0 != wcsicmp( _text, it->second.c_str() ) )
				//		{
				//			hr = E_INVALIDARG; 
				//			break; 
				//		}
				//	}

				//	break; 
				//}

				//ASSERT( 0 <= it->first.length() ); 
				temp_text = it->first.c_str(); 


				//2 Returns attribute value as a BSTR. This flag does not work for event properties. 

				hr = element->getAttribute( temp_text.GetBSTR(), 2, value.GetAddress() ); 

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

				if( temp_text.length() == 0 )
				{
					if( it->second.length() == 0 )
					{
						break; 
					}

					hr = E_INVALIDARG; 
					break; 
				}
				else
				{
					_text = temp_text.operator const wchar_t*(); 
					if( _text == NULL )
					{
						ASSERT( FALSE ); 
						hr = E_INVALIDARG; 
						break; 
					}
				}

				{
					//wstring escaped_text; 

					//escaped_text = it->second.c_str(); 

					//escape_html_text( escaped_text ); 
					if( 0 != _wcsicmp( it->second.c_str(), _text ) )
					{
						hr = E_INVALIDARG; 
						break; 
					}
				}
			} while ( FALSE );
		}

	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI compare_html_element_attributes_ex( XPATH_PARAMS *xpath, 
												  IHTMLElement *element)
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	_bstr_t temp_text; 
	LPCWSTR _text; 

	wstring tag; 

	wstring id; 
	wstring name; 
	wstring _class; 
	
	do 
	{
		XPATH_PARAMS::iterator param_it; 

		ASSERT( NULL != xpath ); 
		ASSERT( NULL != element ); 

		param_it = xpath->find( L"name" ); 

		if ( param_it == xpath->end() )
		{
			name = L""; 
		}
		else
		{
			name = param_it->second.c_str(); 
		}

		param_it = xpath->find( L"id" ); 

		if ( param_it == xpath->end() )
		{
			id = L""; 
		}
		else
		{
			id = param_it->second.c_str(); 
			_hr = element->get_id( temp_text.GetAddress() ); 
			if( FAILED( _hr ) )
			{
			}
			else
			{
				_text = temp_text.operator const wchar_t*(); 
				if( _text == NULL )
				{

				}
				else
				{
					if( 0 != _wcsicmp( id.c_str(), _text ) )
					{
						//ASSERT( FALSE ); 
						hr = E_INVALIDARG;
						break; 
					}
				}
			}
		}

		param_it = xpath->find( L"tag" ); 

		if ( param_it == xpath->end() )
		{
			tag = L""; 
		}
		else
		{
			tag = param_it->second.c_str(); 
			_hr = element->get_tagName( temp_text.GetAddress() ); 
			if( FAILED( _hr ) )
			{
			}
			else
			{
				_text = temp_text.operator const wchar_t*(); 
				if( _text == NULL )
				{

				}
				else
				{
#ifdef _DEBUG
					if( 0 == _wcsicmp( tag.c_str(), L"A" ) )
					{
						hr = hr; 
					}
#endif //_DEBUG

					if( 0 != _wcsicmp( tag.c_str(), _text ) )
					{
						ASSERT( FALSE ); 
						hr = E_INVALIDARG; 
						break; 
					}
				}
			}
		}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
		param_it = xpath->find( L"class" ); 

		if ( param_it == xpath->end() )
		{
			_class = L""; 
		}
		else
		{
			_class = param_it->second.c_str(); 

			_hr = element->get_className( temp_text.GetAddress() ); 
			if( FAILED( _hr ) )
			{
			}
			else
			{
				_text = temp_text.operator const wchar_t*(); 
				if( _text == NULL )
				{

				}
				else
				{
					if( 0 != _wcsicmp( _class.c_str(), _text ) )
					{
						//ASSERT( FALSE ); 
						hr = E_INVALIDARG; 
						break; 
					}
				}
			}
		}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

#if 0
		temp_text = L"name"; 
		_hr = sub_element->getAttribute( temp_text.GetBSTR(), 2, _name.GetAddress() ); 

		if( FAILED( _hr ) )
		{
		}
		else
		{
			temp_text = _name.bstrVal; 

			_text = temp_text.operator const wchar_t*(); 
			if( _text == NULL )
			{

			}
			else
			{
				if( 0 != wcsicmp( name.c_str(), _text ) )
				{
					ASSERT( FALSE ); 
					ret = ERROR_NOT_FOUND; 
					break; 
				}
			}
		}
#else 
		hr = compare_html_element_attributes( xpath, 
			element ); 

		if( FAILED( hr) )
		{
			break; 
		}
#endif //0

	}while( FALSE );

	return hr; 
};

LRESULT WINAPI xpath_params_to_xpath( XPATH_PARAMS *params[], ULONG count, wstring &xpath_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	//ULONG param_index; 
	wstring element_xpath; 
	XPATH_PARAM_ITERATOR it; 

	do 
	{
		ASSERT( NULL != params ); 
		ASSERT( 0 < count ); 

		xpath_out.clear(); 
		ret = append_element_xpath( params, count, xpath_out ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "error %u:%s\n", __LINE__, __FUNCTION__ ); 
			break; 
		}
		//for( param_index = 0; param_index < count; param_index ++ )
		//{
		//	element_xpath = L""; 

		//	it = params[ param_index ]->find( L"tag" ); 
		//	if( it == params[ param_index ]->end() )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}
		//}
	}while( FALSE );
	
	return ret; 
}

LRESULT WINAPI check_and_fix_links_path( LPCWSTR xpath, wstring &xpath_out, wstring &error_msg )
{
	LRESULT ret = ERROR_SUCCESS; 
	XPATH_PARAMS **params = NULL; 
	ULONG params_count = 0; 
	INT32 i; 
	CString text; 
	wstring _xpath; 

	do 
	{
		ASSERT( NULL != xpath ); 
		_xpath = xpath; 

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
			text.Format( L"路径格式不正确:%s", xpath ); 
			error_msg = text.GetBuffer(); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		XPATH_PARAM_ITERATOR param_it; 
		for( i = params_count - 1; i >= 0; i -- )
		{
			param_it = params[ i ]->find( XPATH_TAG ); 

			if ( param_it != params[ i ]->end() )
			{
				if( 0 == wcscmp( L"A", (*param_it).second.c_str() ) )
				{
					break; 
				}
			}
			else
			{
				text.Format( L"路径格式不正确:%s", xpath ); 
				error_msg = text.GetBuffer(); 
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

		if( i < 2 ) //HTML|BODY|DIV
		{
			text.Format( L"路径格式不正确(请检查目标是否是超链接(A)):%s", xpath ); 
			error_msg = text.GetBuffer(); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( i != params_count - 1 )
		{
			text.Format( L"路径的目标并不是超链接(A):%s, 将修正", xpath ); 
			error_msg = text.GetBuffer(); 
			ret = xpath_params_to_xpath( params, i + 1, _xpath ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			ret = ERROR_XPATH_IS_FIXED; 
			break; 
		}

	} while ( FALSE ); 

	if( NULL != params )
	{
		if( params_count > 0 )
		{
			release_xpath_params( params, params_count ); 
		}

		free( params ); 
		params = NULL; 
	}

	xpath_out = _xpath; 
	return ret; 
}

LRESULT WINAPI prepare_xpath_location() 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	if( NULL != xpath_info_ui_window )
	{
		SendMessage( xpath_info_ui_window, WM_XPATH_MISMATCH_INFO, ( WPARAM )( PVOID )NULL, ( LPARAM )( PVOID )NULL ); 
	}

	return ret; 
}

LRESULT WINAPI print_xpath_mismatch_element( IHTMLElement *html_element, LPCWSTR xpath, BOOLEAN analyze_xpath ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring html_element_xpath; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != xpath ); 

		ret = get_html_element_xpath( html_element, html_element_xpath ); 
		if( ret != ERROR_SUCCESS )
		{

		}

		//ret = find_string_difference( html_element_xpath.c_str(), xpath ); 
		//if( ret != ERROR_SUCCESS )
		//{

		//}

		dbg_print_w( MSG_IMPORTANT, L"xpath is mismatch:\n" ); 
		dbg_print_w( MSG_IMPORTANT, L"target:%s\n", xpath ); 
		dbg_print_w( MSG_IMPORTANT, L"element xpath:%s\n", html_element_xpath.c_str() ); 

		if( NULL != xpath_info_ui_window 
			&& FALSE != analyze_xpath )
		{
			SendMessage( xpath_info_ui_window, WM_XPATH_MISMATCH_INFO, ( WPARAM )( PVOID )xpath, ( LPARAM )( PVOID )html_element_xpath.c_str() ); 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI print_xpath_match_element( IHTMLElement *html_element, LPCWSTR xpath ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring html_element_xpath; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != xpath ); 

		ret = get_html_element_xpath( html_element, html_element_xpath ); 
		if( ret != ERROR_SUCCESS )
		{

		}

		dbg_print_w( MSG_IMPORTANT, L"xpath is match:\n" ); 
		dbg_print_w( MSG_IMPORTANT, L"target:%s\n", xpath ); 
		dbg_print_w( MSG_IMPORTANT, L"element xpath:%s\n", html_element_xpath.c_str() ); 

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI locate_html_element_from_xpath( HTML_ELEMENT_GROUP *element_group, 
											  XPATH_PARAMS *xpaths[], 
											  ULONG xpath_count, 
											  HTML_ELEMENT_GROUP *group, 
											  LPCWSTR target_xpath, 
											  BOOLEAN analyze_trace )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	HTML_ELEMENT_INFO_ITERATOR it; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr elements; 
	IHTMLElementCollectionPtr sub_elements; 
	HTML_ELEMENT_GROUP _element_group; 
	HTML_ELEMENT_GROUP current_element_group; 

	IHTMLElementPtr sub_element; 
	XPATH_PARAMS *xpath; 
	ULONG xpath_index; 
	//ULONG i; 

	wstring name; 
	wstring id; 
	wstring tag; 
#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
	wstring _class; 
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

	LONG child_index; 

	LPCWSTR _text; 
	wstring text; 
	_bstr_t temp_text; 
	
	_variant_t _name; 
	_variant_t index; 

	HTML_ELEMENT_GROUP_ELEMENT_ITERATOR html_info_it; 

	BOOLEAN element_located = FALSE; 
	//wstring target_xpth; 

	do 
	{
		ASSERT( NULL != element_group ); 
		ASSERT( NULL != xpaths ); 
		ASSERT( 0 < xpath_count ); 
		ASSERT( NULL != group ); 

		/************************************************************************
		快速定位元素过滤方法过程:
		1.对元素的总范围进行限制，通过NAME和PATH缩小范围。
		1.判断是否所有的元素信息都包含ID或NAME
		2.如果包含，对所有的信息使用ITEM方法中的INDEX参数进行过滤
		3.如果有元素不包含，则通过路径进行过滤
		4.如果有元素不包含，并且路径不可以进行精确过滤，则其基础上过滤元素类型

		************************************************************************/

		if( analyze_trace != FALSE )
		{
			prepare_xpath_location(); 
		}

		ASSERT( element_group->size() > 0 ); 
		_element_group = *element_group; 

		{
			//HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 
			//for( ; ; )
			//{
			//	it = element_group.begin(); 
			//	if( it == element_group.end() )
			//	{
			//		break; 
			//	}

			//	( *it )->Release(); 
			//	element_group.erase( it ); 
			//}

			element_group->clear(); 
		}

		for( xpath_index = 0; xpath_index < xpath_count; xpath_index ++ )
		{
			XPATH_PARAMS::iterator param_it; 

			xpath = xpaths[ xpath_index ]; 
			ASSERT( xpath != NULL ); 

			param_it = xpath->find( L"name" ); 

			if ( param_it == xpath->end() )
			{
				name = L""; 
			}
			else
			{
				name = param_it->second.c_str(); 
			}

			param_it = xpath->find( L"id" ); 

			if ( param_it == xpath->end() )
			{
				id = L""; 
			}
			else
			{
				id = param_it->second.c_str();
			}

			param_it = xpath->find( L"tag" ); 

			if ( param_it == xpath->end() )
			{
				tag = L""; 
			}
			else
			{
				tag = param_it->second.c_str(); 

				//if( 0 == wcsicmp( tag.c_str(), L"a" ) )
				//{
				//	DebugBreak(); 
				//}
			}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
			param_it = xpath->find( L"class" ); 

			if ( param_it == xpath->end() )
			{
				_class = L""; 
			}
			else
			{
				_class = param_it->second.c_str(); 
				//if( 0 == wcsicmp( _class.c_str(), L"product-iWrap") )
				//{
				//	//DebugBreak(); 
				//}
			}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

			param_it = xpath->find( HTML_ELEMENT_ORDER_VALUE_NAME ); 

			if ( param_it == xpath->end() )
			{
				child_index = -1; 
			}
			else
			{
				LPWSTR text_end; 
				child_index = wcstol( param_it->second.c_str(), &text_end, 0 ); 

				ASSERT( child_index != -1 ); 
			}

			for( ; ; )
			{
				html_info_it = _element_group.begin(); 
				
				if( html_info_it == _element_group.end() )
				{
					//ret = ERROR_NOT_FOUND; 
					break; 
				}

				do 
				{
					LONG sub_element_count = 0; 
					IDispatchPtr sub_elements_disp[ 2 ]; 
					LONG sub_elements_index; 

					hr = ( *html_info_it )->get_tagName( temp_text.GetAddress() ); 
					if( FAILED( hr ) )
					{
						ret = hr; 
						break; 
					}

					_text = temp_text.operator const wchar_t*(); 
					if( NULL == _text )
					{
						break; 
					}

					if( 0 == _wcsicmp( _text, L"iframe" ) 
						|| 0 == _wcsicmp( _text, L"frame" ) )
					{
						//IHTMLIFrameElement2Ptr html_iframe; 
						IHTMLFrameBase2Ptr html_frame; 
						IHTMLWindow2Ptr html_window; 
						IHTMLDocument2Ptr html_doc; 
						//IHTMLFrameElement2Ptr html_frame; 

						hr = ( *html_info_it )->QueryInterface( IID_IHTMLFrameBase2, ( PVOID* )&html_frame ); 
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

						//hr = html_iframe->QueryInterface( IID_IHTMLWindow2, ( LPVOID* )&html_window );    
						//hr = html_frame->QueryInterface( IID_IHTMLWindow2, ( LPVOID* )&html_window ); 
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

						//hr = html_window->QueryInterface( IID_IHTMLDocument2, ( LPVOID* )&html_doc ); 
						//if( FAILED( hr ) )
						//{
						//	ret = hr; 
						//	break; 
						//}

						//hr = html_iframe->QueryInterface( IID_IHTMLDocument2, ( LPVOID* )&html_doc ); 
						//if( FAILED( hr ) )
						//{
						//	ret = hr; 
						//	break; 
						//}

						//hr = html_frame->QueryInterface( IID_IHTMLWindow2, ( LPVOID* )&html_window ); 
						//if( FAILED( hr ) )
						//{
						//	ret = hr; 
						//	break; 
						//}

						ret = html_window_2_html_document( html_window, &html_doc ); 
						if( ERROR_SUCCESS != ret )
						{
							break; 
						}

						hr = html_doc->get_all( &elements ); 
						if( FAILED( hr ) ) 
						{
							ret = hr; 
							break; 
						}

						if( NULL == elements )
						{
							ret = ERROR_NOT_FOUND; 
							break; 
						}

						hr = elements->get_length( &sub_element_count ); 
						if( FAILED( hr ) )
						{
							ret = hr; 
							break; 
						}

						if( ( xpath_index + 1 ) >= xpath_count )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							break; 
						}

						ret = get_html_children( elements, 
							xpaths[ xpath_index + 1 ], 
							&current_element_group ); 

						if( ret != ERROR_SUCCESS )
						{
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							break; 
						}

						xpath_index += 1; 
						break; 
					}
					else
					{
						hr = ( *html_info_it )->get_children( &sub_elements_disp[ 0 ] ); 
						if( FAILED( hr ) 
							|| sub_elements_disp[ 0 ] == NULL )
						{
						}

						sub_elements_disp[ 1 ] = NULL; 

						//hr = ( *html_info_it )->get_all( &sub_elements_disp[ 1 ] ); 
						//if( FAILED( hr ) 
						//	|| sub_elements_disp[ 1 ] == NULL )
						//{
						//}

						if( sub_elements_disp[ 0 ] == NULL 
							&& sub_elements_disp[ 1 ] == NULL )
						{
							ASSERT( FALSE ); 
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						for( sub_elements_index = 0; sub_elements_index < 2; sub_elements_index ++ )
						{
							do 
							{
								if( sub_elements_disp[ sub_elements_index ] == NULL )
								{
									break; 
								}

								hr = sub_elements_disp[ sub_elements_index ]->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
								if( SUCCEEDED( hr )
									&& sub_element != NULL )
								{
									ASSERT( child_index == -1 
										|| child_index == 0 ); 

									hr = sub_element->get_tagName( temp_text.GetAddress() ); 
									if( FAILED( hr ) )
									{
									}
									else
									{
										_text = temp_text.operator const wchar_t*(); 
										if( _text == NULL )
										{

										}
										else
										{
//#ifdef _DEBUG
//											if( 0 == wcsicmp( tag.c_str(), L"A" ) )
//											{
//												DebugBreak(); 
//											}
//#endif //_DEBUG

											if( 0 != _wcsicmp( tag.c_str(), _text ) )
											{
												ASSERT( FALSE ); 
												print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
												ret = ERROR_NOT_FOUND; 
												break; 
											}
											else
											{
												print_xpath_match_element( sub_element, target_xpath ); 
											}
										}
									}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
									hr = sub_element->get_className( temp_text.GetAddress() ); 
									if( FAILED( hr ) )
									{
									}
									else
									{
										_text = temp_text.operator const wchar_t*(); 
										if( _text == NULL )
										{

										}
										else
										{
											if( 0 != _wcsicmp( _class.c_str(), _text ) )
											{
												ASSERT( FALSE ); 
												print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
												ret = ERROR_NOT_FOUND; 
												break; 
											}
										}
									}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

									hr = sub_element->get_id( temp_text.GetAddress() ); 
									if( FAILED( hr ) )
									{
									}
									else
									{
										_text = temp_text.operator const wchar_t*(); 
										if( _text == NULL )
										{

										}
										else
										{
											if( 0 != _wcsicmp( id.c_str(), _text ) )
											{
												ASSERT( FALSE ); 
												print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
												ret = ERROR_NOT_FOUND; 
												break; 
											}
										}
									}

#if 0
									temp_text = L"name"; 
									hr = sub_element->getAttribute( temp_text.GetBSTR(), 2, _name.GetAddress() ); 

									if( FAILED( hr ) )
									{
									}
									else
									{
										temp_text = _name.bstrVal; 

										_text = temp_text.operator const wchar_t*(); 
										if( _text == NULL )
										{

										}
										else
										{
											if( 0 != wcsicmp( name.c_str(), _text ) )
											{
												ASSERT( FALSE ); 
												ret = ERROR_NOT_FOUND; 
												break; 
											}
										}
									}
#else 
									hr = compare_html_element_attributes( xpath, 
										sub_element ); 

									if( FAILED( hr) )
									{
										print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
										break; 
									}
#endif //0

									sub_element->AddRef(); 
									current_element_group.push_back( sub_element ); 
									break; 
								}

								hr = sub_elements_disp[ sub_elements_index ]->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&elements ); 
								if( FAILED( hr )
									&& elements == NULL )
								{
									//ret = ERROR_ERRORS_ENCOUNTERED; 
									ASSERT( FALSE ); 
									print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
									break; 
								}

								{
									hr = elements->get_length( &sub_element_count ); 

									if( FAILED( hr ) 
										|| sub_element_count == 0 )
									{
										//ret = ERROR_ERRORS_ENCOUNTERED; 
										//ASSERT( FALSE ); 
										print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
										break; 
									}
								}
							}while( FALSE ); 

							if( sub_element_count != 0 )
							{
								break; 
							}
						}
					}

					//maybe have out path element, because can't locate the elements precisely.
					if( sub_element_count == 0 )
					{
						//ASSERT( FALSE ); 
						//ret = ERROR_ERRORS_ENCOUNTERED; 
						print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
						break; 
					}

					if( id.length() == 0 
						&& name.length() == 0 )
					{
						temp_text = tag.c_str(); 

						_name = temp_text; 

						hr = elements->tags( _name, &sub_element_disp ); 
						if( S_OK != hr )
						{
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}
					}
					else
					{
						if( id.length() )
						{
							temp_text = id.c_str(); 
						}
						else if( name.length() )
						{
							temp_text = name.c_str(); 
						}
						else
						{
							ASSERT( FALSE ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}
						
						_name = temp_text.GetBSTR(); 

						//comment it.
						V_VT( &index ) = VT_EMPTY;
						V_I4( &index ) = -1; 

						//注意这里的name参数，必须是HTML元素的id或name属性,tag是不可以直接用在此参数的，只能在HTML元素取出后，再进行判断。
						hr = elements->item( _name, index, &sub_element_disp ); 
						if( FAILED( hr ) )
						{
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							dbg_print( MSG_FATAL_ERROR, "get the html element from id(name) %ws error\n", temp_text.operator const wchar_t*() ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						if( NULL == sub_element_disp )
						{
							print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
							dbg_print( MSG_FATAL_ERROR, "get the html element from id(name) %ws error\n", temp_text.operator const wchar_t*() ); 
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}
					}

					/***********************************************************
					1.查找过程:

					方式1:
					1.从XPATH的第1个元素起，一级级加载第个目录的HTML元素，如果找
					到多个元素，就将其全部加入这一级的元素组中。
					2.从上级的元素中查找全部的下一级的元素。
					重复上述过程，直到找到最后一级元素。

					方式2: 
					1.从元素的最后一级开始向外查，找到第一个有ID或NAME的元素，
					之后从这一级开始使用方法1查找一级级的子元素，直到最后一级。
					***********************************************************/
					do 
					{
						LONG _sub_element_count; 
						LONG i; 

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( SUCCEEDED( hr )
							&& sub_element != NULL )
						{
							if( id.length() == 0 
								&& name.length() == 0 )
							{
								if( child_index != -1 
									&& child_index != 0 )
								{
									ASSERT( FALSE ); 
									break; 
								}
							}

							sub_element->AddRef(); 
							current_element_group.push_back( sub_element ); 
							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
						if( FAILED( hr )
							|| sub_elements == NULL )
						{
							break; 
						}

						hr = sub_elements->get_length( &_sub_element_count ); 
						if( FAILED( hr ) ) 
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						if( id.length() == 0 
							&& name.length() == 0 )
						{
							if( child_index != -1 )
							{
								if( child_index < _sub_element_count )
								{
									V_VT( &_name ) = VT_I4;
									V_I4( &_name ) = child_index; 

									//comment it.
									V_VT( &index ) = VT_I4;
									V_I4( &index ) = 0; 

									hr = sub_elements->item( _name, index, &sub_element_disp ); 
									if( FAILED( hr ) )
									{
										ASSERT( FALSE ); 
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}

									if( sub_element_disp == NULL )
									{
										ASSERT( FALSE ); 
										ret = ERROR_ERRORS_ENCOUNTERED; 
										break; 
									}

									hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
									if( SUCCEEDED( hr )
										&& sub_element != NULL )
									{
										//if( _class.length() != 0 )
										//{
										//	hr = sub_element->get_className( temp_text.GetAddress() ); 
										//	if( FAILED( hr ) )
										//	{
										//	}
										//	else
										//	{
										//		_text = temp_text.operator const wchar_t*(); 
										//		if( _text == NULL )
										//		{

										//		}
										//		else
										//		{
										//			if( 0 != wcsicmp( _class.c_str(), _text ) )
										//			{
										//				//ASSERT( FALSE ); 
										//				//ret = ERROR_NOT_FOUND; 
										//				break; 
										//			}
										//		}
										//	}
										//}

										hr = compare_html_element_attributes_ex( xpath, sub_element ); 
										if( FAILED(hr))
										{
											//ASSERT( FALSE ); 
											print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
											break; 
										}

										sub_element->AddRef(); 
										current_element_group.push_back( sub_element ); 
									}
								}
								else
								{
									if( _element_group.size() == 0 
										&& current_element_group.size() == 0 )
									{
										ASSERT( FALSE ); 
									}

									print_xpath_mismatch_element( *html_info_it, target_xpath, analyze_trace ); 
									//ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}

								break; 
							}
						}
						else
						{
							ASSERT( 0 == _sub_element_count ); 
						}

						for( i = 0; i < _sub_element_count; i ++ )
						{
							do 
							{
								V_VT( &_name ) = VT_I4;
								V_I4( &_name ) = i; 

								//comment it.
								V_VT( &index ) = VT_I4;
								V_I4( &index ) = 0; 

								hr = sub_elements->item( _name, index, &sub_element_disp ); 
								if( FAILED( hr ) )
								{
									ASSERT( FALSE ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}

								if( sub_element_disp == NULL )
								{
									ASSERT( FALSE ); 
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}

								hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
								if( SUCCEEDED( hr )
									&& sub_element != NULL )
								{
									//if( _class.length() != 0 )
									//{
									//	hr = sub_element->get_className( temp_text.GetAddress() ); 
									//	if( FAILED( hr ) )
									//	{
									//	}
									//	else
									//	{
									//		_text = temp_text.operator const wchar_t*(); 
									//		if( _text == NULL )
									//		{

									//		}
									//		else
									//		{
									//			if( 0 != wcsicmp( _class.c_str(), _text ) )
									//			{
									//				//ASSERT( FALSE ); 
									//				//ret = ERROR_NOT_FOUND; 
									//				break; 
									//			}
									//		}
									//	}
									//}

									hr = compare_html_element_attributes_ex( xpath, sub_element ); 
									if( FAILED(hr))
									{
										print_xpath_mismatch_element( sub_element, target_xpath, analyze_trace ); 
										break; 
									}

									sub_element->AddRef(); 
									current_element_group.push_back( sub_element ); 
								}
							}while( FALSE ); 
						}
					}while( FALSE ); 
				}while( FALSE ); 

				( *html_info_it )->Release(); 
				_element_group.erase( html_info_it ); 
			}

			_element_group = current_element_group; 
			current_element_group.clear(); 
		
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//没有可以查询下一级元素的HTML元素了
			if( _element_group.size() == 0 )
			{
				//print_xpath_mismatch_element( sub_element, target_xpath ); 
				ret = ERROR_NOT_FOUND; 
				break; 
			}
		}

		if( ret == ERROR_SUCCESS )
		{
			HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 

			for( it = _element_group.begin(); it != _element_group.end(); it ++ )
			{
				add_html_element_to_group( ( *it ), group ); 
			}
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 

		for( it = _element_group.begin(); it != _element_group.end(); it ++ )
		{
			( *it )->Release(); 
		}

		ASSERT( 0 == group->size() ); 
	}

	return ret; 
}

LRESULT WINAPI fix_html_attribute_format( wstring &word, wstring &word_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring _word; 

	do 
	{
		if( word.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( word[ 0 ] != L'"' )
		{
			//INT32 i; 
			BOOLEAN no_quote_char = FALSE; 

			//for( i = 0; i < word.length(); i ++ )
			//{
			//	if( FALSE == isdigit( word[ i ] ) )
			//	{
			no_quote_char = TRUE; 
			//		break; 
			//	}
			//}

			if( TRUE == no_quote_char )
			{
				_word = L'\"'; 
				_word += word; 
				_word += L'\"'; 

				word_out = _word; 
			}
			else
			{
				word_out = word; 
			}
		}
		else
		{
			//size_t i; 
			BOOLEAN have_end_quote_char = FALSE; 

			word_out = word; 

#ifdef _DEBUG
			size_t i; 
			for( i = 1; i < word.length(); i ++ )
			{
				if( L'\"' == word.at(i ) )
				{
					have_end_quote_char = TRUE; 
					break; 
				}
			}

			if( FALSE == have_end_quote_char )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

#endif //_DEBUG
		}
	}while( FALSE );

	return ret; 
}

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

LRESULT WINAPI fix_html_tag_end( wstring &word, HTML_WORDS *tags, HTML_WORDS *bug_tags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_WORD_REVERSE_ITERATOR it; 

	do 
	{
		ASSERT( NULL != tags ); 
		ASSERT( NULL != bug_tags ); 

		for( ; ; )
		{
			it = tags->rbegin(); 

			if( it == tags->rend() )
			{
				break; 
			}

			if( 0 == _wcsicmp( ( *it )->word.c_str(), word.c_str() ) ) 
			{
				HTML_WORD_REVERSE_ITERATOR temp_it; 
				temp_it = it + 1; 
				HTML_WORD_ITERATOR temp( temp_it.base() ); 

				free( *it ); 

				tags->erase( temp ); 
				break; 
			}
			else
			{
				HTML_WORD_REVERSE_ITERATOR temp_it; 
				temp_it = it + 1; 
				HTML_WORD_ITERATOR temp( temp_it.base() ); 

				bug_tags->push_back( ( *it ) ); 
				tags->erase( temp ); 
			}
		}

	}while( FALSE );

	return ret; 
}

//LRESULT WINAPI fix_html_tag_end(wstring &word, HTML_WORDS *tags, HTML_WORDS *bug_tags)
//{
//    LRESULT ret = ERROR_SUCCESS;
//    HTML_WORD_REVERSE_ITERATOR it;
//
//    do
//    {
//        ASSERT(NULL != tags);
//        ASSERT(NULL != bug_tags);
//
//        for (; ; )
//        {
//            it = tags->rbegin();
//
//            if (it == tags->rend())
//            {
//                break;
//            }
//
//            if (0 == _wcsicmp((*it)->word.c_str(), word.c_str()))
//            {
//                HTML_WORD_REVERSE_ITERATOR temp_it;
//                temp_it = it + 1;
//                HTML_WORD_ITERATOR temp(temp_it.base());
//
//                free(*it);
//
//                tags->erase(temp);
//                break;
//            }
//            else
//            {
//                HTML_WORD_REVERSE_ITERATOR temp_it;
//                temp_it = it + 1;
//                HTML_WORD_ITERATOR temp(temp_it.base());
//
//                bug_tags->push_back((*it));
//                tags->erase(temp);
//            }
//        }
//
//    } while (FALSE);
//
//    return ret;
//}

VOID WINAPI wstring_tolower( wstring &dest_string )
{
	wstring::iterator it; 

	for( it = dest_string.begin(); it != dest_string.end(); it ++ )
	{
		if( ( *it ) >= L'A' 
			&& ( *it ) <= L'Z' )
		{
			( *it ) = ( *it ) + ( 'a' - 'A' ); 
		}
	}
}

LRESULT WINAPI parse_xpath_params( LPCWSTR param, 
								  ULONG cc_param_len, 
								  XPATH_PARAMS *params )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPWSTR _param = NULL; 
	LPWSTR param_begin; 
	LPWSTR param_end; 
	LPWSTR param_name; 
	LPWSTR param_value; 
	wstring escaped_text; 

	do 
	{
		ASSERT( NULL != param ); 
		ASSERT( NULL != params ); 

		_param = ( LPWSTR )malloc( ( cc_param_len + 1 ) << 1 ); 
		if( NULL == _param )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( _param , param, ( ( cc_param_len + 1 ) << 1 ) ); 

		param_begin = _param; 

		for( ; ; )
		{
			param_end = wcschr( param_begin, XPATH_SUB_PARAMETER_STRING_DELIM ); 
			if( NULL == param_end )
			{
				param_name = param_begin; 

				param_value = wcschr( param_begin, XPATH_SUB_PARAMETER_VALUE_SIGN ); 
				if( NULL == param_value )
				{
					break; 
				}

				*param_value = L'\0'; 
				param_value += 1; 

				escaped_text = param_value; 
				escape_html_text( escaped_text ); 

				params->insert( XPATH_PARAM_PAIR( param_name, escaped_text.c_str() ) ); 

				break; 
			}

			*param_end = L'\0'; 

			param_name = param_begin; 
			param_value = wcschr( param_begin, XPATH_SUB_PARAMETER_VALUE_SIGN ); 
			if( NULL == param_value )
			{
				break; 
			}

			*param_value = L'\0'; 
			param_value += 1; 

			{
				escaped_text = param_value; 
				escape_html_text( escaped_text ); 

				params->insert( XPATH_PARAM_PAIR( param_name, escaped_text.c_str() ) ); 
			}

			param_begin = param_end + 1; 
		}
	}while( FALSE );

	if( NULL != _param )
	{
		free( _param ); 
	}

	return ret; 
}

LRESULT WINAPI escape_html_text( wstring &html_text )
{
	LRESULT ret = ERROR_SUCCESS; 
	size_t pos; 
	do 
	{
		pos = 0; 
		while (TRUE)
		{
			size_t pos = html_text.find(XPATH_PARAMETER_STRING_DELIM_QUOTE);
			if (pos != std::wstring::npos )
			{
				html_text.replace(pos, 
					CONST_STR_LEN( XPATH_PARAMETER_STRING_DELIM_QUOTE ), 
					XPATH_PARAMETER_STRING_DELIM, 
					CONST_STR_LEN( XPATH_PARAMETER_STRING_DELIM ) );
				//text.insert(pos, strDest);
			}
			else
			{
				break;
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI quote_html_text( wstring &html_text )
{
	LRESULT ret = ERROR_SUCCESS; 
	size_t pos; 
	do 
	{
		pos = 0; 
		while (TRUE)
		{
			pos = html_text.find(XPATH_PARAMETER_STRING_DELIM, pos);
			if (pos != std::wstring::npos )
			{
				html_text.replace(pos, 
					CONST_STR_LEN( XPATH_PARAMETER_STRING_DELIM ), 
					XPATH_PARAMETER_STRING_DELIM_QUOTE, 
					CONST_STR_LEN( XPATH_PARAMETER_STRING_DELIM_QUOTE ) );
				//text.insert(pos, strDest);
			}
			else
			{
				break;
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_xpath_params( LPCWSTR param, 
								ULONG cc_param_len, 
								XPATH_PARAMS *xpaths[], 
								ULONG max_param_count, 
								ULONG *param_count )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	ULONG _ret; 
	//LPCWSTR begin; 
	//LPCWSTR end; 
	STRING_POINTER *strings = NULL; 

	WCHAR string_buffer[ MAX_XPATH_PARAMETER_STRING_LENGTH ];
	ULONG string_count; 
	ULONG i; 

	XPATH_PARAMS *params = NULL; 
	LPWSTR param_name; 
	LPWSTR param_value; 
	ULONG _param_count; 

	do 
	{
		ASSERT( NULL != param ); 
		ASSERT( NULL != xpaths ); 

		ASSERT( max_param_count <= MAX_XPATH_ELEMENT_PARAMETER_COUNT ); 

		_param_count = 0; 

		strings = ( STRING_POINTER* )malloc( sizeof( STRING_POINTER ) * max_param_count );
		if( NULL == strings )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_ret = split_string( param, 
			cc_param_len, 
			XPATH_PARAMETER_STRING_DELIM, 
			strings, 
			MAX_XPATH_ELEMENT_PARAMETER_COUNT, 
			&string_count ); 

		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

		for( i = 0; i < string_count; i ++ )
		{
			params = new XPATH_PARAMS(); 

			do 
			{
				if( NULL == params )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}

				//memset( string_buffer, 0, sizeof( string_buffer ) ); 

				if( strings[ i ].length + 1 < MAX_XPATH_PARAMETER_STRING_LENGTH )
				{
					memcpy( string_buffer, strings[ i ]._string, ( strings[ i ].length << 1 ) ); 
					string_buffer[ strings[ i ].length ] = L'\0'; 
				}
				else
				{
					memcpy( string_buffer, strings[ i ]._string, ( ( MAX_XPATH_PARAMETER_STRING_LENGTH - 1 ) << 1 ) ); 
					string_buffer[ MAX_XPATH_PARAMETER_STRING_LENGTH - 1 ] = L'\0'; 

					dbg_print( MSG_FATAL_ERROR, "copy the xpath parameter %ws error\n", strings[ i ]._string ); 
				}

				//hr = StringCchCopyW( string_buffer, 
				//	( strings[ i ].length + 1 < MAX_XPATH_PARAMETER_STRING_LENGTH ) ?  strings[ i ].length + 1: MAX_XPATH_PARAMETER_STRING_LENGTH, 
				//	strings[ i ]._string ); 

				param_name = string_buffer; 

				param_value = wcschr( param_name, XPATH_TAG_PARAMTER_DELIM ); 
				if( NULL == param_value )
				{
					params->insert( XPATH_PARAM_PAIR( XPATH_TAG, param_name ) ); 
					break; 
				}

				*param_value = L'\0'; 

				params->insert( XPATH_PARAM_PAIR( XPATH_TAG, param_name ) ); 

				ret = parse_xpath_params( param_value + 1, 
					( strings[ i ].length - ( ULONG )( ULONG_PTR )( ( param_value + 1 ) - string_buffer ) ), 
					params ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				//{
				//	*param_value = L'\0'; 
				//	params->insert( XPATH_PARAM_PAIR( param_name, param_value ) ); 


				//	param_value += 1; 

				//	params->insert( XPATH_PARAM_PAIR( param_name, param_value ) ); 

				//	param_begin += 1; 
				//}

			}while( FALSE );

			if( ret == ERROR_SUCCESS )
			{
				xpaths[ _param_count ] = params; 
				params = NULL; 
				_param_count ++; 
			}

			if( NULL != params )
			{
				delete params; 
				params = NULL; 
			}
		}

	}while( FALSE );

	*param_count = _param_count; 

	if( NULL != strings )
	{
		free( strings ); 
	}

	if( NULL != params )
	{
		delete params; 
		params = NULL; 
	}

	return ret; 
}

/**************************************************************
XPATH 由TAG+PROPERTIES组成:
其中TAG是稳定不变的，但只使用TAG选择能力太低
PROPERTY分为两种:
1.值固定度高(ID,NAME),在各种情况下都不会变化
2.值固定度低(CLASS),在不同的情况下会变化为不同的值
3.名不固定,名称是自定的,不属于HTML标准,变化性最大

**************************************************************/

/**************************************************************
针对元素属性的错误率进行综合评估，对元素量和错误数进行平衡计算，
取优化后的XPATH
**************************************************************/
typedef struct _XPATH_ERROR
{
	wstring error_desc; //或者用索引号表示
	ULONG error_count; 
} XPATH_ERROR, *PXPATH_ERROR; 

LRESULT WINAPI learning_best_xpath( IHTMLDocument2 *doc, wstring &xpath)
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != doc ); 

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI filter_xpath_noisy( wstring &target_xpath )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = xor_html_element_xpath( target_xpath, target_xpath ); 

		if( NULL != wcsstr( target_xpath.c_str(), L" class=" ) )
		{
			//DebugBreak(); 
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI append_element_xpath( XPATH_PARAMS **xpath_params, ULONG param_count, wstring &xpath_desc )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG param_index; 
	wstring element_xpath; 
	BOOLEAN have_attr; 
	INT32 mismatched_param; 
	XPATH_PARAM_ITERATOR param_it; 

	do 
	{
		for( param_index = 0; param_index < param_count; param_index ++ )
		{
			element_xpath = L""; 

			have_attr = FALSE; 
			mismatched_param = 0; 

			param_it = xpath_params[ param_index ]->find( L"tag" ); 
			if( param_it == xpath_params[ param_index ]->end() )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			element_xpath += param_it->second.c_str(); 

			for( param_it = xpath_params[ param_index ]->begin(); 
				param_it != xpath_params[ param_index ]->end(); 
				param_it ++ ) 
			{
				do 
				{
					if( 0 == _wcsicmp( param_it->first.c_str(), L"tag" ) ) 
					{
						break; 
					}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
					if( 0 == _wcsicmp( param_it->first.c_str(), L"class" ) ) 
					{
						break;
					}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

					//if( 0 == _wcsicmp( param_it->first.c_str(), HTML_ELEMENT_TEXT_PROPERTY_NAME ) )
					//{
					//	break; 
					//}

					if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					element_xpath += param_it->first.c_str(); 
					element_xpath += L"="; 

					{
						wstring escaped_text; 

						escaped_text = param_it->second.c_str(); 
						quote_html_text( escaped_text ); 

						element_xpath += escaped_text.c_str(); 
					}

					element_xpath += L";"; 
				}while( FALSE );
			}

			xpath_desc += element_xpath; 
			xpath_desc += L"|"; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI xor_html_element_xpath( wstring xpath_src, wstring &xpath_dest )
{
	LRESULT ret = ERROR_SUCCESS;  

	wstring element_xpath; 
	wstring xpath; 

	BOOLEAN have_attr; 

	XPATH_PARAMS **param_src = NULL; 
	XPATH_PARAMS **param_dest = NULL; 
	
	ULONG param_src_count = 0; 
	ULONG param_dest_count = 0; 
	ULONG param_index; 

	XPATH_PARAM_ITERATOR param_src_it; 
	XPATH_PARAM_ITERATOR param_dest_it; 
	ULONG mismatched_param; 

	do 
	{
		param_src = ( XPATH_PARAMS** )malloc( sizeof( XPATH_PARAMS* ) * MAX_XPATH_ELEMENT_PARAMETER_COUNT ); 
		if( NULL == param_src )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		param_dest = ( XPATH_PARAMS** )malloc( sizeof( XPATH_PARAMS* ) * MAX_XPATH_ELEMENT_PARAMETER_COUNT ); 
		if( NULL == param_dest )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = get_xpath_params( xpath_src.c_str(), 
			xpath_src.length(), 
			param_src, 
			MAX_XPATH_ELEMENT_PARAMETER_COUNT, 
			&param_src_count ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = get_xpath_params( xpath_dest.c_str(), 
			xpath_dest.length(), 
			param_dest, 
			MAX_XPATH_ELEMENT_PARAMETER_COUNT, 
			&param_dest_count ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		for( param_index = 0; param_index < param_src_count; param_index ++ )
		{
			element_xpath = L""; 

			if( param_index >= param_dest_count )
			{
				ret = ERROR_NOT_FOUND; 
				break; 
			}

			param_src_it = param_src[ param_index ]->find( L"tag" ); 
			if( param_src_it == param_src[ param_index ]->end() )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			param_dest_it = param_dest[ param_index ]->find( L"tag" ); 
			if( param_dest_it == param_dest[ param_index ]->end() )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( 0 != _wcsicmp( param_src_it->second.c_str(), 
				param_dest_it->second.c_str() ) )
			{
				if( param_src_count < param_dest_count )
				{
					append_element_xpath( &param_src[ param_index ], 
						param_src_count - param_index, 
						xpath ); 
				}
				else
				{
					append_element_xpath( &param_dest[ param_index ], 
						param_dest_count - param_index, 
						xpath ); 
				}

				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			element_xpath += param_src_it->second.c_str(); 

			have_attr = FALSE; 
			mismatched_param = 0; 
			for( param_src_it = param_src[ param_index ]->begin(); 
				param_src_it != param_src[ param_index ]->end(); 
				param_src_it ++ ) 
			{
				do 
				{
					if( 0 == _wcsicmp( param_src_it->first.c_str(), L"tag" ) ) 
					{
						break; 
					}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
					if( 0 == _wcsicmp( param_src_it->first.c_str(), L"class" ) ) 
					{
						break; 
					}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

					param_dest_it = param_dest[ param_index ]->find( param_src_it->first.c_str() ); 

					if( param_dest_it == param_dest[ param_index ]->end() )
					{
						mismatched_param += 1; 
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					if( 0 != _wcsicmp( param_src_it->second.c_str(), 
						param_dest_it->second.c_str() ) )
					{
						mismatched_param += 1; 
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					element_xpath += param_src_it->first.c_str(); 
					element_xpath += L"="; 
					element_xpath += param_src_it->second.c_str(); 
					element_xpath += L";"; 
				}while( FALSE );
			}

#define DEFAULT_PARAM_MISMATCH_TIME 1
			if( mismatched_param > DEFAULT_PARAM_MISMATCH_TIME )
			{
				dbg_print( MSG_FATAL_ERROR, "the parameter(%ws:%ws)[%ws] mismatch time %u is greater than %u\n", 
					xpath_src.c_str(), 
					xpath_dest.c_str(), 
					element_xpath.c_str(), 
					mismatched_param, 
					DEFAULT_PARAM_MISMATCH_TIME ); 
			
				//ret = ERROR_INVALID_PARAMETER; 
			}

			xpath += element_xpath; 
			xpath += L"|"; 
		}
		
		dbg_print( MSG_IMPORTANT, "source xpath:\n%ws\n", 
			xpath_src.c_str() ); 

		dbg_print( MSG_IMPORTANT, "dest xpath:\n%ws\n", 
			xpath_dest.c_str() ); 

		dbg_print( MSG_IMPORTANT, "(same:%s)\n", 
			_wcsicmp( xpath_src.c_str(), xpath_dest.c_str() ) == 0 ? "TRUE" : "FALSE" ); 

		dbg_print( MSG_IMPORTANT, "result xpath:\n%ws\n", 
		xpath.c_str() ); 

		xpath_dest = xpath; 

	}while( FALSE );

	if( NULL != param_src )
	{
		release_xpath_params( param_src, param_src_count ); 
		free( param_src ); 
		param_src = NULL; 
	}

	if( NULL != param_dest )
	{
		release_xpath_params( param_dest, param_dest_count ); 
		free( param_dest ); 
		param_dest = NULL; 
	}

	return ret; 
}

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
可以通过XMLELEMENT来方便生成XML
*******************************************************************************/

//#ifndef  DATA_SCRAMBLER_BROWSER
#include "Markup.h"
HRESULT WINAPI get_html_dom_element_text( IHTMLElement *html_element, wstring &html_text_out )
{
	HRESULT hr = S_OK; 
	//IHTMLDOMImplementation
	IHTMLDOMNodePtr element_node;
	//IHTMLDOMChildrenCollection2 nodes; 
	IHTMLDOMTextNodePtr element_text_node;

	IDispatchPtr disp;
	IHTMLAttributeCollectionPtr attrs;
	IDispatchPtr attr_disp;
	IHTMLDOMAttributePtr attr;
	LONG attr_count;
	wstring tag_name; 
	VARIANT attr_name;
	_bstr_t _attr_name;
	_variant_t attr_value;
	VARIANT_BOOL specified; 
	LPCWSTR _text; 
	_bstr_t text; 
	wstring temp_text; 


	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( html_text_out.length() == 0 ); 

		//nodes.()
		//element_node->get_childNodes()
		html_text_out = L""; 
		do 
		{
			_bstr_t text; 
			LPCWSTR _text; 
			hr = html_element->QueryInterface(IID_IHTMLDOMTextNode, (void**)&element_text_node); 
			if( FAILED(hr) )
			{
				break; 
			}

			if( NULL == element_text_node )
			{
				hr = E_FAIL; 
				break; 
			}

			hr = element_text_node->toString(text.GetAddress()); 
			if( FAILED(hr) )
			{
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
				hr = E_FAIL; 
				break; 
			}

			ASSERT( text.length() > 0 ); 
			
			html_text_out = _text; 

		}while( FALSE ); 

		if( SUCCEEDED(hr))
		{
			break; 
		}

		ASSERT( html_text_out.length() == 0); 

		html_text_out.empty(); 
		hr = html_element->get_tagName( text.GetAddress() ); 
		if( FAILED(hr))
		{
			break; 
		}

		_text = text.operator const wchar_t*(); 
		if( NULL == _text )
		{
			hr = E_FAIL; 
			break; 
		}

		tag_name = _text; 
		html_text_out = L"<"; 
		html_text_out += _text; 
		html_text_out += L" "; 

		hr = html_element->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&element_node ); 
		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == element_node )
		{
			hr = E_FAIL; 
			break; 
		}

		hr = element_node->get_attributes(&disp);
		if(FAILED(hr))
		{
			break; 
		}

		if( NULL == disp )
		{
			hr = E_FAIL; 
			break; 
		}

		hr = disp->QueryInterface(IID_IHTMLAttributeCollection, ( PVOID* )&attrs);
		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == attrs )
		{
			hr = E_FAIL; 
			break; 
		}
		
		hr = attrs->get_length(&attr_count);
		if( FAILED(hr))
		{
			break; 
		}

		attr_name.vt = VT_I4;

		for( INT32 i = 0; i < attr_count; i ++ )
		{
			attr_name.lVal = i;

			do 
			{
				hr= attrs->item(&attr_name, &attr_disp); 
				if( FAILED(hr) )
				{
					break; 
				}

				if( NULL == attr_disp )
				{
					hr = E_FAIL; 
					break; 
				}

				hr = attr_disp->QueryInterface(IID_IHTMLDOMAttribute, ( PVOID* )&attr);

				if( FAILED(hr)
					|| NULL == disp )
				{
					break; 
				}

				if( NULL == attr )
				{
					hr = E_FAIL; 
					break; 
				}

				hr = attr->get_specified( &specified ); 
				if( FAILED(hr))
				{
					break; 
				}

				if( VARIANT_FALSE == specified )
				{
					break; 
				}

	
				attr->get_nodeName(_attr_name.GetAddress());
				if( FAILED(hr))
				{
					break; 
				}

				attr->get_nodeValue(attr_value.GetAddress());
				if( FAILED(hr))
				{
					break; 
				}
				
				if( NULL == disp )
				{
					hr = E_FAIL; 
					break; 
				}

				_text = NULL; 

				_text = _attr_name.operator wchar_t*(); 
				if( _text == NULL )
				{
					break; 
				}

				html_text_out += _text; 
				html_text_out += L"='"; 

				//hr = filter_common_html_attr( _attr_name ); 
				//if( hr != S_OK )
				//{
				//	break; 
				//}

				//hr = ( *it )->getAttribute( attr_name, 2, attr_value.GetAddress() ); 

				//if( hr != S_OK )
				//{
				//	ret = ERROR_ERRORS_ENCOUNTERED; 
				//	break; 
				//}

				do 
				{
					_text = NULL; 

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
				}while( FALSE ); 
				if( NULL == _text )
				{
					_text = L""; 
				}

				{
					//wstring __text; 
					//__text = _text; 

					//while (TRUE)
					//{
					//	size_t pos = __text.find(L"'");
					//	if (pos != std::wstring::npos )
					//	{
					//		__text.replace(pos, 1, L" ", 0);
					//		//text.insert(pos, strDest);
					//	}
					//	else
					//	{
					//		break;
					//	}
					//}

					CString __text; 
					__text = CMarkup::EscapeText( _text, CMarkup::MNF_ESCAPEQUOTES | CMarkup::MNF_WITHREFS ); 
					html_text_out += __text.GetBuffer(); 
					html_text_out += L"' "; 
				}
			} while ( FALSE ); 
		}

		html_text_out += L" >"; 
		html_text_out += L"</"; 
		html_text_out += tag_name; 
		html_text_out += L">"; 
	}while( FALSE ); 

	return hr; 
}
//#endif //DATA_SCRAMBLER_BROWSER

/******************************************************
this order:
1.1st frame
2.second frame
******************************************************/

HRESULT WINAPI get_html_index_in_parent(IHTMLElement *html_element)
{
	HRESULT hr = S_OK; 
	IHTMLElementPtr html_parent_element; 

	IHTMLDOMNodePtr html_node; 
	IHTMLDOMNodePtr html_parent_node; 


	ASSERT( NULL != html_element); 

	do 
	{
		hr = html_element->get_parentElement( &html_parent_element ); 
		if( hr != S_OK )
		{
			break; 
		}

		if( NULL == html_parent_element )
		{
			break; 
		}

		hr = html_parent_element->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&html_parent_node ); 
		if( hr != S_OK )
		{
			break; 
		}

		if( NULL == html_parent_node )
		{
			break; 
		}

		hr = html_element->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&html_node ); 
		if( hr != S_OK )
		{
			break; 
		}

		if( NULL == html_node )
		{
			break; 
		}

		//html_parent_node->
	}while( FALSE );

	return hr; 
}

HRESULT WINAPI get_html_element_from_point( IHTMLDocument2* html_doc, 
											POINT *pos, 
											IHTMLElement **element )
{
	HRESULT hr = S_OK; 

	ASSERT( NULL != html_doc ); 
	ASSERT( NULL != pos ); 
	ASSERT( NULL != element ); 

	hr = html_doc->elementFromPoint( pos->x, pos->y, element ); 
	
	if( NULL == *element )
	{
		hr = E_FAIL; 
	}

	return hr; 
}


// Converts a IHTMLWindow2 object to a IWebBrowser2. Returns NULL in case of failure.
LRESULT WINAPI html_window_2_html_web_browser(IHTMLWindow2 *window, IWebBrowser2 **web_browser)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IServiceProviderPtr service_provider;	

	ASSERT(window != NULL);
	ASSERT( NULL != web_browser); 
	*web_browser = NULL; 

	do 
	{
		hr = window->QueryInterface( IID_IServiceProvider, ( PVOID* )&service_provider ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if(NULL == service_provider)
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}


		hr = service_provider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)web_browser );
		if ( hr != S_OK)
		{
			ret = hr; 
			break; 
		}

		if( *web_browser == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	} while ( FALSE ); 

	return ret;
}

// Converts a IHTMLWindow2 object to a IHTMLDocument2. Returns NULL in case of failure.
// It takes into account accessing the DOM across frames loaded from different domains.
LRESULT WINAPI html_window_2_html_document(IHTMLWindow2 *window, IHTMLDocument2 **document )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IWebBrowser2Ptr web_browser; 	
	IDispatchPtr disp;

	do 
	{
		ASSERT(window != NULL); 
		ASSERT( NULL != document); 

		hr = window->get_document( document);

		if ((S_OK == hr) 
			&& ( *document != NULL ))
		{
			break; 
		}

		// hRes could be E_ACCESSDENIED that means a security restriction that
		// prevents scripting across frames that loads documents from different internet domains.
		ret = html_window_2_html_web_browser(window, &web_browser);
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		// Get the document object from the IWebBrowser2 object.
		hr = web_browser->get_Document( &disp ); 
		if( FAILED( hr) )
		{
			ret = hr; 
			break; 
		}

		if( disp == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )document ); ;
		if( FAILED( hr) )
		{
			ret = hr; 
			break; 
		}

		if( *document == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	}while( FALSE );

	return ret;
}

//LRESULT WINAPI find_document_from_window( IHTMLWindow2 *window, IHTMLElement **element )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	do 
//	{
//		ASSERT( NULL != window ); 
//		ASSERT( NULL != element ); 
//			// Get the IDispatch of the main document
//			CComPtr<IDispatch> pDisp;
//			m_pBrowser->get_Document( &pDisp );
//			if( pDisp == NULL )
//				return E_FAIL;     // Get the container
//			CComQIPtr<IOleContainer> pContainer = pDisp;
//			if( pContainer == NULL )
//				return E_FAIL;     // Get an enumerator for the frames
//			CComPtr<IEnumUnknown> pEnumerator;
//			HRESULT hr = pContainer->EnumObjects( OLECONTF_EMBEDDINGS, &pEnumerator );
//			if( FAILED(hr) )
//				return E_FAIL;     // Enumerate and refresh all the frames
//			CComPtr<IUnknown> pUnk;
//			ULONG uFetched = 0;
//			for( UINT i = 0; S_OK == pEnumerator->Next(1, &pUnk, &uFetched); i++ )
//			{
//				CComQIPtr<IWebBrowser2> pSubBrowser = pUnk;
//				pUnk.Release();         if( pSubBrowser == NULL )
//					continue;         // get sub document in frame
//				CComQIPtr<IHTMLDocument2> pSubDoc;
//				pDisp.Release();
//				pSubBrowser->get_Document( &pDisp );
//				if( pDisp )
//					pSubDoc = pDisp;
//				if( pSubDoc == NULL )
//					continue;         // get selection info
//				hr = GetSelectionFromSingleFrame( pSubDoc, selText, selRect );         // 如果成功找到，则返回，否则继续尝试下一个frame
//				if( SUCCEEDED( hr ) )
//					return hr;
//			}     // 全部遍历之后仍然没有结果
//
//	}while( FALSE ); 
//	
//	return ret; 
//}


LRESULT WINAPI check_document_same( IHTMLDocument2 *_html_doc, IHTMLDocument2 *html_doc ) 
{
	HRESULT hr; 
	LRESULT ret = ERROR_INVALID_PARAMETER; 
	LRESULT _ret; 
	_bstr_t text_source; 
	_bstr_t test_dest; 

	do 
	{
		hr = _html_doc->get_URL( text_source.GetAddress() ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		html_doc->get_URL( test_dest.GetAddress() ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if( text_source.operator const char*() == NULL )
		{
			break; 
		}

		if( test_dest.operator const wchar_t*() == NULL )
		{
			break; 
		}

		if( 0 != wcscmp( text_source.operator const wchar_t*(), 
			test_dest.operator const wchar_t*() ) )
		{
			break; 
		}

		hr = _html_doc->get_fileSize( text_source.GetAddress() ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		hr = _html_doc->get_fileSize( test_dest.GetAddress() ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if( text_source.operator const wchar_t*() == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( test_dest.operator const wchar_t*() == NULL )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( 0 != wcscmp( text_source.operator const wchar_t*(), 
			test_dest.operator const wchar_t*() ) )
		{
			break; 
		}

		do 
		{
			_ret = ERROR_SUCCESS; 
			hr = _html_doc->get_fileUpdatedDate( text_source.GetAddress() ); 
			if( FAILED(hr))
			{
				break; 
			}

			hr = _html_doc->get_fileUpdatedDate( test_dest.GetAddress() ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( text_source.operator const char*() == NULL )
			{
				break; 
			}

			if( test_dest.operator const wchar_t*() == NULL )
			{
				break; 
			}

			if( 0 != wcscmp( text_source.operator const wchar_t*(), 
				test_dest.operator const wchar_t*() ) )
			{
				_ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		} while ( FALSE ); 

		if( _ret != ERROR_SUCCESS )
		{
			ret = _ret; 
			break; 
		}

		{
			IHTMLElementPtr body; 
			hr = _html_doc->get_body( &body ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( NULL == body )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = body->get_innerHTML( text_source.GetAddress() ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( NULL == text_source.operator const char*() )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = _html_doc->get_body( &body ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( NULL == body )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = body->get_innerHTML( test_dest.GetAddress() ); 
			if( FAILED(hr))
			{
				ret = hr; 
				break; 
			}

			if( NULL == test_dest.operator const wchar_t*() )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( 0 != wcscmp( test_dest.operator const wchar_t*(), 
				text_source.operator const wchar_t*() ) )
			{
				break; 
			}

			ret = ERROR_SUCCESS; 
			break; 
		}
	}while( FALSE ); 
	return ret; 
}

LRESULT WINAPI compare_html_window( IHTMLWindow2 *_html_window, 
								   IHTMLWindow2 *html_window ) 
{
	LRESULT ret = ERROR_INVALID_PARAMETER; 
	HRESULT hr; 
	IHTMLDocument2Ptr _html_doc; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		ASSERT( NULL != _html_window ); 
		ASSERT( NULL != html_window ); 

		hr = html_window_2_html_document( _html_window, &_html_doc ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if( NULL == _html_doc )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = html_window->get_document( &html_doc ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if( NULL == html_doc )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = check_document_same( _html_doc, html_doc ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}
LRESULT WINAPI check_html_frame_window( IHTMLElement *element, IHTMLWindow2 *html_window )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	HRESULT hr; 
	IHTMLFrameBase2Ptr frame_base; 
	IHTMLWindow2Ptr _html_window; 

	do 
	{
		ASSERT( NULL != element); 
		//hr = element->QueryInterface( IID_IHTMLFrameElement, ( LPVOID* )&frame_element );
		hr = element->QueryInterface( IID_IHTMLFrameBase2, ( LPVOID* )&frame_base ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		if( NULL == frame_base )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = frame_base->get_contentWindow( &_html_window ); 
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		ret = compare_html_window( _html_window, html_window ); 

		if( ret == ERROR_SUCCESS )
		{
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI html_window_2_html_frame( IHTMLDocument2 *html_doc, 
										IHTMLWindow2 *html_window, 
										IHTMLFrameBase2 **frame )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IServiceProviderPtr service_provider; 
	IWebBrowser2Ptr web_browser; 
	//IHTMLFramesCollection2Ptr frames_collection; 
	IDispatchPtr disp; 
//	LONG frame_count; 
	_variant_t index; 
	_variant_t result; 
	IHTMLFrameElementPtr frame_element;
	IHTMLFrameBase2Ptr frame_base; 
	IHTMLWindow2Ptr _html_window; 
	INT32 i; 

	do 
	{
		ASSERT( NULL != html_window ); 
		ASSERT( NULL != frame ); 
		*frame = NULL; 

		hr = html_window->QueryInterface( IID_IHTMLFrameBase, ( PVOID* )frame ); 
		if( SUCCEEDED(hr)
			&& *frame != NULL )
		{
			break; 
		}

		//hr = html_window->QueryInterface( IID_IServiceProvider, ( PVOID* )&service_provider ); 
		//if( FAILED(hr))
		//{
		//	ret = hr; 
		//	break; 
		//}

		//if(NULL == service_provider)
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//hr = service_provider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)web_browser );
		//if ( hr != S_OK)
		//{
		//	ret = hr; 
		//	break; 
		//}

		//if( *web_browser == NULL )
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		//web_browser->get_Container()

		if( NULL == html_doc )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//hr = html_doc->get_frames( &frames_collection ); 
		//if( FAILED(hr))
		//{
		//	break; 
		//}

		//if( NULL == frames_collection )
		//{
		//	break; 
		//}

		//hr = frames_collection->get_length( &frame_count ); 

		//if(FAILED(hr))
		//{
		//	ret = hr; 
		//	break; 			
		//}

		//if( frame_count == 0 )
		//{
		//	ret = ERROR_NOT_FOUND; 
		//	break; 
		//}

		IHTMLElementCollectionPtr elements; 
		IHTMLElementCollectionPtr sub_elements; 
		IHTMLElementPtr sub_element; 
		LONG element_count; 
		LONG sub_element_count; 
		_bstr_t text; 
		_variant_t id; 
		IDispatchPtr sub_element_disp; 
		INT32 j; 
		LPCWSTR frame_elemnt_tags[] = { L"IFRAME", L"FRAME" }; 

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

		for( j = 0; j < ARRAYSIZE( frame_elemnt_tags ); j ++ )
		{
			do
			{
			text = frame_elemnt_tags[ j ]; 
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

					ret = check_html_frame_window( sub_element, html_window ); 
					if( ret == ERROR_SUCCESS )
					{
						hr = sub_element->QueryInterface(IID_IHTMLFrameBase2, ( PVOID*)frame ); 
						if( FAILED(hr))
						{
							ret = hr; 
						}

						break; 
					}
				}
			}
			else
			{
				ret = check_html_frame_window( sub_element, html_window ); 
				if( ret == ERROR_SUCCESS )
				{
					hr = sub_element->QueryInterface(IID_IHTMLFrameBase2, ( PVOID*)frame ); 
					if( FAILED(hr))
					{
						ret = hr; 
					}

					break; 
				}
			}
			}while( FALSE ); 
		}
		//for( INT32 i = 0; i< frame_count; i ++ ) 
		//{ 
		//	index = i; 

		//	hr = frames_collection->item( &index, &result ); 

		//	if( FAILED( hr ) ) 
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( result.vt != VT_DISPATCH )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}

		//	if( result.pdispVal == NULL )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	disp =( LPDISPATCH )result.pdispVal; 

		//	hr = disp->QueryInterface( IID_IHTMLFrameElement, ( LPVOID* )&frame_element );
		//	//hr = disp->QueryInterface( IID_IHTMLFrameBase2, ( LPVOID* )&frame_base ); 
		//	if( FAILED( hr ) )
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( NULL == frame_base )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	hr = frame_base->get_contentWindow( &_html_window ); 
		//	if( FAILED(hr))
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( _html_window == html_window )
		//	{
		//		*frame = frame_base; 
		//		break; 
		//	}
		//}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI html_window_2_parent_html_document( HTML_DOCUMENT_FAMILY *doc_family, 
										IHTMLWindow2 *html_window, 
										IHTMLFrameBase2 **frame )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	//HRESULT hr; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		ASSERT( NULL != doc_family ); 
		ASSERT( NULL != frame ); 
		*frame = NULL; 

		if( NULL == doc_family )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//hr = html_window->QueryInterface( IID_IHTMLFrameBase, ( PVOID* )frame ); 
		//if( SUCCEEDED(hr)
		//	&& *frame != NULL )
		//{
		//	break; 
		//}

		//hr = html_window->get_document( &html_doc ); 
		//if( FAILED(hr))
		//{
		//	break; 
		//}

		//if( NULL == html_doc )
		//{
		//	ret = ERROR_ERRORS_ENCOUNTERED; 
		//	break; 
		//}

		for( HTML_DOCUMENT_FAMILY::iterator it = doc_family->begin(); 
			it != doc_family->end(); 
			it ++ )
		{
			_ret = html_window_2_html_frame( ( *it ), html_window, frame ); 
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
			}
			else
			{
				ASSERT( *frame != NULL ); 
				break; 
			}
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI get_frames_on_html_document( IHTMLDocument2 *html_doc, HTML_DOCUMENT_FAMILY *doc_family )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementCollectionPtr elements; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	IHTMLFrameBase2Ptr frame_base; 
	IHTMLDocument2Ptr frame_doc; 
	IHTMLWindow2Ptr html_window; 
	LONG element_count; 
	LONG sub_element_count; 
	_bstr_t text; 
	_variant_t id; 
	_variant_t index; 
	IDispatchPtr sub_element_disp; 
	INT32 j; 
	INT32 i; 
	LPCWSTR frame_elemnt_tags[] = { L"IFRAME", L"FRAME" }; 


	do 
	{
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != doc_family ); 

		//hr = html_doc->get_frames( &frames_collection ); 
		//if( FAILED(hr))
		//{
		//	break; 
		//}

		//if( NULL == frames_collection )
		//{
		//	break; 
		//}

		//hr = frames_collection->get_length( &frame_count ); 

		//if(FAILED(hr))
		//{
		//	ret = hr; 
		//	break; 			
		//}

		//if( frame_count == 0 )
		//{
		//	ret = ERROR_NOT_FOUND; 
		//	break; 
		//}

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

		for( j = 0; j < ARRAYSIZE( frame_elemnt_tags ); j ++ )
		{
			do
			{
				text = frame_elemnt_tags[ j ]; 
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

						hr = sub_element->QueryInterface( IID_IHTMLFrameBase2, ( LPVOID* )&frame_base ); 
						if( FAILED( hr ) )
						{
							ret = hr; 
							break; 
						}

						if( NULL == frame_base )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						hr = frame_base->get_contentWindow( &html_window ); 

						if( NULL == html_window )
						{
							ret = ERROR_NOT_FOUND; 
							break; 
						}

						ret = html_window_2_html_document( html_window, &frame_doc ); 
						if( ERROR_SUCCESS != ret )
						{
							break; 
						}

						frame_doc->AddRef(); 
						doc_family->push_back( frame_doc ); 
					}
				}
				else
				{
					hr = sub_element->QueryInterface( IID_IHTMLFrameBase2, ( LPVOID* )&frame_base ); 
					if( FAILED( hr ) )
					{
						ret = hr; 
						break; 
					}

					if( NULL == frame_base )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					hr = frame_base->get_contentWindow( &html_window ); 

					if( NULL == html_window )
					{
						ret = ERROR_NOT_FOUND; 
						break; 
					}

					ret = html_window_2_html_document( html_window, &frame_doc ); 
					if( ERROR_SUCCESS != ret )
					{
						break; 
					}

					frame_doc->AddRef(); 
					doc_family->push_back( frame_doc ); 
				}
			}while( FALSE ); 
		}

		//for( INT32 i = 0; i< frame_count; i ++ ) 
		//{ 
		//	index = i; 

		//	hr = frames_collection->item( &index, &result ); 

		//	if( FAILED( hr ) ) 
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( result.vt != VT_DISPATCH )
		//	{
		//		ret = ERROR_INVALID_PARAMETER; 
		//		break; 
		//	}

		//	if( result.pdispVal == NULL )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	disp =( LPDISPATCH )result.pdispVal; 

		//	hr = disp->QueryInterface( IID_IHTMLFrameElement, ( LPVOID* )&frame_element );
		//	//hr = disp->QueryInterface( IID_IHTMLFrameBase2, ( LPVOID* )&frame_base ); 
		//	if( FAILED( hr ) )
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( NULL == frame_base )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	hr = frame_base->get_contentWindow( &_html_window ); 
		//	if( FAILED(hr))
		//	{
		//		ret = hr; 
		//		break; 
		//	}

		//	if( _html_window == html_window )
		//	{
		//		*frame = frame_base; 
		//		break; 
		//	}
		//}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI get_frames_family_on_html_document( IHTMLDocument2 *html_doc, HTML_DOCUMENT_FAMILY *doc_family )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	HTML_DOCUMENT_FAMILY _doc_family; 
	HTML_DOCUMENT_FAMILY all_docs; 
	IHTMLDocument2 *located_doc; 

	do 
	{
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != doc_family ); 

		doc_family->push_back( html_doc ); 
		all_docs.push_back( html_doc ); 
		for( ; ; )
		{
			if( all_docs.size() == 0 )
			{
				break; 
			}

			located_doc = *all_docs.begin(); 
			all_docs.erase( all_docs.begin() ); 

			_doc_family.clear(); 
			ret = get_frames_on_html_document( located_doc, &_doc_family ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			for( HTML_DOCUMENT_FAMILY::iterator it = _doc_family.begin(); 
				it != _doc_family.end(); 
				it ++ )
			{
				ASSERT( NULL != (*it)); 
				all_docs.push_back( *it ); 
				doc_family->push_back( *it ); 
			}
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI check_compliant_mode_element( IHTMLElement *html_element )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	LPWSTR temp_text; 
	_bstr_t html_element_tag; 

	do 
	{
		ASSERT( NULL != html_element); 

		ret = ERROR_INVALID_PARAMETER; 
		break; 

		hr = html_element->get_tagName( html_element_tag.GetAddress() ); 
		if( FAILED(hr))
		{
			break; 
		}

		temp_text = html_element_tag.operator wchar_t*(); 
		if( NULL == temp_text )
		{
			break; 
		}

		if( 0 != _wcsicmp( temp_text, L"" ) )
		{
			ret = ERROR_INVALID_PARAMETER; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_html_document_doc_type()
{
	LRESULT ret = ERROR_SUCCESS; 
	//<!DOCTYPE html>
	return ret; 
}

/*
Helper function to get an element's exact position
function getPosition(el) {
var xPos = 0;
var yPos = 0;

while (el) {
if (el.tagName == "BODY") {
// deal with browser quirks with body/window/document and page scroll
var xScroll = el.scrollLeft || document.documentElement.scrollLeft;
var yScroll = el.scrollTop || document.documentElement.scrollTop;

xPos += (el.offsetLeft - xScroll + el.clientLeft);
yPos += (el.offsetTop - yScroll + el.clientTop);
} else {
// for all other non-BODY elements
xPos += (el.offsetLeft - el.scrollLeft + el.clientLeft);
yPos += (el.offsetTop - el.scrollTop + el.clientTop);
}

el = el.offsetParent;
}
return {
x: xPos,
y: yPos
};
}

// deal with the page getting resized or scrolled
window.addEventListener("scroll", updatePosition, false);
window.addEventListener("resize", updatePosition, false);

function updatePosition() {
// add your code to update the position when your browser
// is resized or scrolled
}
*/

LRESULT WINAPI get_html_element_scroll_offset( IHTMLElement *html_element, 
												  LONG *offset_x, 
												  LONG *offset_y )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElement2Ptr html_element2; 

	do 
	{
		ASSERT( NULL != html_element ); 
		ASSERT( NULL != offset_x ); 
		ASSERT( NULL != offset_y ); 

		hr = html_element->QueryInterface( &html_element2 );
		if( FAILED(hr ))
		{
			ret = hr; 
			break; 
		}

		if( NULL == html_element2 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ASSERT( html_element2 );

		//hr = html_element2->get_clientLeft( client_x );
		//if( FAILED(hr ))
		//{
		//	ret = hr; 
		//	break; 
		//}

		//hr = html_element2->get_clientTop( client_y );
		//if( FAILED(hr ))
		//{
		//	ret = hr; 
		//	break; 
		//}

		hr = html_element2->get_scrollLeft( offset_x );
		if( FAILED(hr ))
		{
			ret = hr; 
			break; 
		}

		hr = html_element2->get_scrollTop( offset_y );
		if( FAILED(hr ))
		{
			ret = hr; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI _get_html_element_position( HTML_DOCUMENT_FAMILY *html_docs, 
										 IHTMLElement *html_element, 
										 RECT *rect )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	HRESULT hr; 
	IDispatchPtr disp;
	IHTMLDocument2Ptr html_doc;
	IHTMLDocument2Ptr html_doc_parent; 
	IHTMLWindow2Ptr html_window;
	IHTMLFrameBase2Ptr html_frame;
	IHTMLElementPtr _html_element; 
	IHTMLElementPtr __html_element; 
	IHTMLElementPtr ___html_element;
	IHTMLElement2Ptr html_element2; 
	IHTMLElement2Ptr _html_element2; 
	LONG lx = 0; 
	LONG ly = 0; 
	_bstr_t text; 
	INT32 i = 0; 
	INT32 iframe_level; 

	memset( rect, 0, sizeof( *rect ) ); 

	do 
	{
		if( html_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ASSERT( NULL != html_docs ); 

		if( html_docs->size() == 0 )
		{
			break; 
		}

		_html_element = html_element; 
		
		hr = _html_element->get_offsetWidth(&rect->right);
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		hr = _html_element->get_offsetHeight(&rect->bottom);
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		iframe_level = 0; 

		for( ; ; )
		{
			if( NULL == _html_element )
			{
				ASSERT( FALSE ); 
				break; 
			}

			if( iframe_level >= ( INT32 )html_docs->size() )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			html_doc_parent = *( html_docs->rbegin() + 1 ); 

			hr = _html_element->get_offsetLeft( &lx );
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			hr = _html_element->get_offsetTop( &ly ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			rect->left += lx; 
			rect->top += ly;

			__ret = check_compliant_mode_element( _html_element ); 
			if( __ret == ERROR_SUCCESS ) 
			{
				//for ie6 or later standards-compliant mode
				_html_element->get_parentElement( &__html_element );
			}
			else
			{
				_html_element->get_offsetParent( &__html_element );
			}

			if( __html_element == NULL )
			{
				ASSERT( ly == 0 ); 
				ASSERT( lx == 0 ); 

				_bstr_t tag_name; 

				_html_element->get_tagName( tag_name.GetAddress() ); 
				if( tag_name.operator wchar_t*() != NULL )
				{
					ASSERT( 0 == _wcsicmp( tag_name.operator wchar_t*(), L"body" ) ); 
				}

				hr = _html_element->get_document( &disp ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				if( disp == NULL ) 
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = disp->QueryInterface( &html_doc ); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_doc )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( ERROR_SUCCESS == check_document_same( html_doc, html_doc_parent ) )
				{
					hr = _html_element->QueryInterface( &html_element2 );
					if( FAILED(hr ))
					{
						ret = hr; 
						break; 
					}

					if( NULL == html_element2 )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					ASSERT( html_element2 );

					do 
					{
						hr = html_element2->get_clientLeft( &lx );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						hr = html_element2->get_clientTop( &ly );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						if( lx != 0 
							|| ly != 0 )
						{
							break; 
						}

						{
							IHTMLDocument3Ptr html_doc3; 
							hr = html_doc_parent->QueryInterface( IID_IHTMLDocument3, ( PVOID* )&html_doc3 ); 
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

							hr = html_doc3->get_documentElement( &_html_element ); 
							if( FAILED(hr))
							{
								ret = hr; 
								break; 
							}

							if( _html_element == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = _html_element->QueryInterface( &html_element2 );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							if( html_element2 == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = html_element2->get_clientLeft( &lx );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							hr = html_element2->get_clientTop( &ly );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}
						}
					}while( FALSE ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					rect->left += lx; 
					rect->top += ly;

					do 
					{
						hr = html_element2->get_scrollLeft( &lx );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						hr = html_element2->get_scrollTop( &ly );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						if( lx != 0 
							|| ly != 0 )
						{
							break; 
						}

						{
							IHTMLDocument3Ptr html_doc3; 
							hr = html_doc_parent->QueryInterface( IID_IHTMLDocument3, ( PVOID* )&html_doc3 ); 
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

							hr = html_doc3->get_documentElement( &_html_element ); 
							if( FAILED(hr))
							{
								ret = hr; 
								break; 
							}

							if( _html_element == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = _html_element->QueryInterface( &html_element2 );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							if( html_element2 == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = html_element2->get_scrollLeft( &lx );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							hr = html_element2->get_scrollTop( &ly );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}
						}
					}while( FALSE ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					rect->left -= lx; 
					rect->top -= ly; 
					break; 
				}

				hr = html_doc->get_parentWindow(&html_window);
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_window )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ret = html_window_2_html_frame( html_doc_parent, html_window, &html_frame );

				if( ret != ERROR_SUCCESS ) 
				{
					break;
				}

				iframe_level += 1; 

				hr = html_frame->QueryInterface( &_html_element ); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( _html_element == NULL ) 
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break;
				}
			}
			else
			{
				_html_element = __html_element; 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( _html_element == NULL ) 
			{
				ASSERT( FALSE ); 
				break; 
			}

			hr = _html_element->QueryInterface( &html_element2 );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			if( NULL == html_element2 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = html_element2->get_clientLeft( &lx );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			hr = html_element2->get_clientTop( &ly );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			rect->left += lx; 
			rect->top += ly;

			hr = html_element2->get_scrollLeft( &lx );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			hr = html_element2->get_scrollTop( &ly );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			rect->left -= lx; 
			rect->top -= ly; 

			i ++; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		rect->right  += rect->left;
		rect->bottom += rect->top;
	}while( FALSE ); 

	return ret;
}

LRESULT WINAPI get_html_element_position_ex( IHTMLDocument2 *html_doc_parent, 
										  IHTMLElement *html_element, 
										  RECT *rect, 
										  ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT __ret; 
	HRESULT hr; 
	IDispatchPtr disp;
	HTML_DOCUMENT_FAMILY docs; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLWindow2Ptr html_window;
	IHTMLFrameBase2Ptr html_frame;
	IHTMLElementPtr _html_element; 
	IHTMLElementPtr __html_element; 
	IHTMLElementPtr ___html_element;
	IHTMLElement2Ptr html_element2; 
	IHTMLElement2Ptr _html_element2; 
	LONG lx = 0; 
	LONG ly = 0; 
	_bstr_t text; 
	INT32 i = 0; 

	do 
	{
		ASSERT( NULL != html_doc_parent ); 

		memset( rect, 0, sizeof( *rect ) ); 

		if( html_element == NULL ) 
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_html_element = html_element; 

		hr = _html_element->get_offsetWidth(&rect->right);
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		hr = _html_element->get_offsetHeight(&rect->bottom);
		if( FAILED(hr))
		{
			ret = hr; 
			break; 
		}

		if( flags == TRVERSE_ALL_FRAMES )
		{
			ret = get_frames_family_on_html_document( html_doc_parent, &docs ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}

		for( ; ; )
		{
			//if( rect->right == 0 
			//	|| rect->bottom == 0 )
			//{
			//}

			if( NULL == _html_element )
			{
				ASSERT( FALSE ); 
				break; 
			}

			hr = _html_element->get_offsetLeft( &lx );
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			hr = _html_element->get_offsetTop( &ly ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			rect->left += lx; 
			rect->top += ly;

			__ret = check_compliant_mode_element( _html_element ); 
			if( __ret == ERROR_SUCCESS ) 
			{
				//for ie6 or later standards-compliant mode
				_html_element->get_parentElement( &__html_element );
			}
			else
			{
				_html_element->get_offsetParent( &__html_element );
			}

			if( __html_element == NULL )
			{
				ASSERT( ly == 0 ); 
				ASSERT( lx == 0 ); 

				_bstr_t tag_name; 

				_html_element->get_tagName( tag_name.GetAddress() ); 
				if( tag_name.operator wchar_t*() != NULL )
				{
					ASSERT( 0 == _wcsicmp( tag_name.operator wchar_t*(), L"body" )
						||  0 == _wcsicmp( tag_name.operator wchar_t*(), L"html" ) ); 
				}

				hr = _html_element->get_document( &disp ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				if( disp == NULL ) 
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				hr = disp->QueryInterface( &html_doc ); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_doc )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( ERROR_SUCCESS == check_document_same( html_doc, html_doc_parent ) )
				{
					hr = _html_element->QueryInterface( &html_element2 );
					if( FAILED(hr ))
					{
						ret = hr; 
						break; 
					}

					if( NULL == html_element2 )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					ASSERT( html_element2 );

					do 
					{
						hr = html_element2->get_clientLeft( &lx );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						hr = html_element2->get_clientTop( &ly );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						if( lx != 0 
							|| ly != 0 )
						{
							break; 
						}

						{
							IHTMLDocument3Ptr html_doc3; 
							hr = html_doc_parent->QueryInterface( IID_IHTMLDocument3, ( PVOID* )&html_doc3 ); 
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

							hr = html_doc3->get_documentElement( &_html_element ); 
							if( FAILED(hr))
							{
								ret = hr; 
								break; 
							}

							if( _html_element == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = _html_element->QueryInterface( &html_element2 );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							if( html_element2 == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = html_element2->get_clientLeft( &lx );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							hr = html_element2->get_clientTop( &ly );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}
						}
					}while( FALSE ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					rect->left += lx; 
					rect->top += ly;

					do 
					{
						if( 0 == _wcsicmp( tag_name.operator wchar_t*(), L"html" ) )
						{
							lx = 0; 
							ly = 0; 
							break; 
						}

						hr = html_element2->get_scrollLeft( &lx );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						hr = html_element2->get_scrollTop( &ly );
						if( FAILED(hr ))
						{
							ret = hr; 
							break; 
						}

						if( lx != 0 
							|| ly != 0 )
						{
							break; 
						}

						{
							IHTMLDocument3Ptr html_doc3; 
							hr = html_doc_parent->QueryInterface( IID_IHTMLDocument3, ( PVOID* )&html_doc3 ); 
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

							hr = html_doc3->get_documentElement( &_html_element ); 
							if( FAILED(hr))
							{
								ret = hr; 
								break; 
							}

							if( _html_element == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = _html_element->QueryInterface( &html_element2 );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							if( html_element2 == NULL )
							{
								ret = ERROR_ERRORS_ENCOUNTERED; 
								break; 
							}

							hr = html_element2->get_scrollLeft( &lx );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}

							hr = html_element2->get_scrollTop( &ly );
							if( FAILED(hr ))
							{
								ret = hr; 
								break; 
							}
						}
					}while( FALSE ); 

					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					rect->left -= lx; 
					rect->top -= ly; 
					break; 
				}

				hr = html_doc->get_parentWindow(&html_window);
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_window )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				if( flags == TRVERSE_ALL_FRAMES )
				{
					ret = html_window_2_parent_html_document( &docs, html_window, &html_frame ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}
				}
				else
				{
					ret = html_window_2_html_frame( html_doc_parent, html_window, &html_frame );
				}

				if( ret != ERROR_SUCCESS ) 
				{
					break;
				}

				hr = html_frame->QueryInterface( &_html_element ); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( _html_element == NULL ) 
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break;
				}
			}
			else
			{
				_html_element = __html_element; 
			}

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( _html_element == NULL ) 
			{
				ASSERT( FALSE ); 
				break; 
			}

			hr = _html_element->QueryInterface( &html_element2 );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			if( NULL == html_element2 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = html_element2->get_clientLeft( &lx );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			hr = html_element2->get_clientTop( &ly );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			rect->left += lx; 
			rect->top += ly;

			hr = html_element2->get_scrollLeft( &lx );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			hr = html_element2->get_scrollTop( &ly );
			if( FAILED(hr ))
			{
				ret = hr; 
				break; 
			}

			rect->left -= lx; 
			rect->top -= ly; 

			i ++; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		rect->right  += rect->left;
		rect->bottom += rect->top;
	}while( FALSE ); 

	for( HTML_DOCUMENT_FAMILY::iterator it = docs.begin(); 
		it != docs.end(); 
		it ++ )
	{
		( *it )->Release(); 
	}

	docs.clear(); 

	return ret;
}

HRESULT WINAPI get_html_element_xpath_from_point( IHTMLDocument2* html_doc, 
										   POINT *pos, 
										   POINT *screen_pos, 
										   IHTMLElement *html_element, 
										   wstring *xpath, 
										   wstring *std_xpath, 
										   IHTMLElement **html_element_out )

{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IHTML_ELEMENTS html_elements; 
	IHTMLDocument2Ptr frame_parent_doc; 

	wstring _xpath; 

	_bstr_t text; 
	LPCWSTR _text; 

	BOOLEAN in_frame = FALSE; 

	do
	{
		//ASSERT(NULL != xpath );
		ASSERT(NULL != html_element); 
		ASSERT( NULL != html_element_out ); 

		*html_element_out = NULL; 

		html_element->AddRef(); 
		html_elements.push_back( html_element ); 

        html_element->AddRef();
		frame_parent_doc = html_doc; 

		for( ; ; )
		{
			do 
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
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			} while ( FALSE ); 

			if( ret != ERROR_SUCCESS )
			{
				html_element->Release(); 
				break; 
			}

			if( 0 == _wcsicmp( _text, L"iframe" ) 
				|| 0 == _wcsicmp( _text, L"frame" ) )
			{
				//IHTMLIFrameElement2 html_iframe; 
				IHTMLFrameBase2Ptr html_frame; 
				IHTMLWindow2Ptr html_window; 
				IHTMLDocument2Ptr _html_doc; 
				IHTMLElementPtr html_sub_element; 
				IHTMLElement2Ptr html_element2; 
				RECT rect; 
				//IHTMLFrameElement2Ptr html_frame; 

				in_frame = TRUE; 

				do 
				{
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

					ret = html_window_2_html_document( html_window, &_html_doc ); 
					if( ERROR_SUCCESS != ret )
					{
						break; 
					}

#if 0
					do 
					{
						IHTMLWindow3Ptr html_window3; 
						hr = html_window->QueryInterface( IID_IHTMLWindow3, ( PVOID*)&html_window3); 
						if( FAILED(hr))
						{
							ret = hr; 
							break; 
						}

						if( NULL == html_window3 )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						hr = html_window3->get_screenLeft( &rect.left ); 
						if( FAILED(hr))
						{
							ret = hr; 
							break; 
						}

						hr = html_window3->get_screenTop( &rect.top ); 
						if( FAILED(hr))
						{
							ret = hr; 
							break; 
						}
					} while ( FALSE ); 
#endif //0

					//if( ret != ERROR_SUCCESS )
					{
						//ret = ERROR_ERRORS_ENCOUNTERED; 

						ret = get_html_element_position( frame_parent_doc, html_element, &rect ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}
					}

					if( rect.left > pos->x )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					if( rect.top > pos->y )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					hr = _html_doc->elementFromPoint( pos->x - rect.left, pos->y - rect.top, &html_sub_element ); 
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

					frame_parent_doc = _html_doc; 
				}while( FALSE ); 

				html_element->Release(); 
				//html_element = NULL; 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

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
			html_element->AddRef(); 
			*html_element_out = html_element; 
			break; 
		}

		if( std_xpath != NULL )
		{
			ret = get_html_element_std_xpath_ex( &html_elements, _xpath ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			} 

			*std_xpath = _xpath; 
		}

		ret = _get_html_element_xpath_ex( &html_elements, _xpath, FIX_XPATH_ERROR ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			//break; 
		}

		ASSERT( html_elements.size() > 0 ); 
		( *( html_elements.rbegin() ) )->AddRef(); 
		*html_element_out = ( *( html_elements.rbegin() ) ); 

		*xpath = _xpath; 

		//if( std::wstring::npos != _xpath.find( HTML_HIGHLIGT_ELEMENT_ID ) )
		//{
		//	//DebugBreak(); 
		//}
	}while( FALSE ); 

	ret = release_html_elements( &html_elements ); 
	if( ERROR_SUCCESS != ret )
	{

	}

	return ret; 
}

HRESULT WINAPI _get_html_element_from_point( IHTMLDocument2* html_doc, 
										   POINT *pos, 
										   IHTMLElement **element )
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
	IHTMLElementPtr target_element; 
	HTML_ELEMENT_GROUP temp_group; 

	do 
	{
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != element ); 
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
#define DEBUG_BREAK() DebugBreak() 
				DEBUG_BREAK(); 
				break; 
			}

			hr = find_html_element_from_pos( &temp_group, pos, &target_element ); 
			if( hr == S_OK )
			{
				ASSERT( target_element != NULL ); 
				break; 
			}
		}

	} while ( FALSE ); 

	return hr; 
}

HRESULT WINAPI html_element_have_child( IHTMLElement *html_element )
{
	HRESULT hr = S_OK; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	LONG count; 

	do 
	{
		hr = html_element->get_children(&sub_element_disp); 
		if( FAILED(hr))
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
			break; 
		}

		if( NULL == sub_element_disp )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
		if( FAILED(hr))
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
			break; 
		}

		if( NULL == sub_elements )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr= sub_elements->get_length( &count ); 
		if( FAILED(hr))
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", hr ); 
			break; 
		}

		if( 0 == count )
		{
			hr = E_UNEXPECTED; 
			break; 
		}
	} while ( FALSE ); 

	return hr; 
}

HRESULT WINAPI find_html_element_from_pos( HTML_ELEMENT_GROUP *group, POINT *pos, IHTMLElement **element_out )
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	INT32 i; 
	LONG element_count; 

	HTML_ELEMENT_GROUP temp_group; 
	IHTMLElementPtr element; 

	do 
	{
		ASSERT( NULL != element_out ); 
		ASSERT( *element_out == NULL ); 

		element_count = ( LONG )group->size(); 

		for( i = 0; i < element_count; i ++ )
		{
			do 
			{
				element = group->at( i ); 
				ASSERT( NULL != element ); 

				dbg_print( MSG_IMPORTANT, "element %u", i ); 

				{
					RECT rc; 
					LONG offset; 
					IHTMLElementCollectionPtr _sub_elements; 

					_hr= element->get_offsetTop( &offset ); 
					if( FAILED(_hr))
					{
						break; 
					}

					//rect = em.OffsetRectangle;
					//p.X += rect.X - em.ScrollLeft;
					//p.Y += rect.Y - em.ScrollTop;
					//em=em.OffsetParent;
					rc.top = offset; 
					_hr= element->get_offsetLeft( &offset ); 
					if( FAILED(_hr))
					{
						break; 
					}

					rc.left = offset; 
					_hr= element->get_offsetWidth( &offset ); 
					if( FAILED(_hr))
					{
						break; 
					}

					rc.right = rc.left + offset; 

					_hr= element->get_offsetHeight( &offset ); 
					if( FAILED(_hr))
					{
						break; 
					}

					rc.bottom = rc.top + offset; 

					if( FALSE == PtInRect( &rc, *pos ) )
					{
						break; 
					}

					//element->get_offsetParent( )
				}

				{
					IHTMLElementPtr sub_element; 
					IDispatchPtr sub_element_disp; 
					IHTMLElementCollectionPtr sub_elements; 
					LONG sub_element_count; 
					_variant_t id; 
					_variant_t index; 
					LONG j; 

					_hr = element->get_children(&sub_element_disp); 
					if( FAILED(_hr))
					{
						ASSERT( *element_out == NULL ); 

						*element_out = element; 
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
						break; 
					}

					_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
					if( FAILED(_hr)
						|| NULL == sub_elements)
					{
						ASSERT( FALSE ); 
						*element_out = element; 
						dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
						break; 
					}

					_hr = sub_elements->get_length( &sub_element_count ); 
					if( FAILED(_hr))
					{
						ASSERT( FALSE ); 
						*element_out = element; 
						dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", _hr); 
						break; 
					}

					if( sub_element_count == 0 )
					{
						*element_out = element; 
						break; 
					}

					for( j = 0; j < sub_element_count; j ++ )
					{
						V_VT( &id ) = VT_I4;
						V_I4( &id ) = j;
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						_hr = sub_elements->item( id, index, &sub_element_disp ); 
						if( FAILED( _hr ) 
							|| sub_element_disp == NULL )
						{
							dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", i, _hr); 

							if( SUCCEEDED( _hr))
							{
								_hr = E_FAIL; 
							}

							break; 
						}

						_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( FAILED( _hr )
							|| NULL == sub_element )
						{
							dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", i, _hr); 

							ASSERT( FALSE ); 
							if( SUCCEEDED( _hr))
							{
								_hr = E_FAIL; 
							}

							break; 
						}

						sub_element->AddRef(); 
						temp_group.push_back( sub_element ); 
					}
				}
			}while( FALSE ); 

			if( *element_out != NULL )
			{
				break; 
			}
		} 

		release_html_element_group(group); 

		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = temp_group.begin(); it != temp_group.end(); it ++ )
		{
			group->push_back( (*it)); 
		}
	}while( FALSE );


	if( *element_out == NULL )
	{
		hr = E_FAIL; 
		//break;
	}

	return hr; 
}

HRESULT WINAPI filter_common_html_attr( LPCWSTR name )
{
	HRESULT hr = S_OK; 

	do 
	{
		ASSERT( NULL != name ); 
		
#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
		//if( 0 == wcsicmp( name, L"class" ) )
		//{
		//	break; 
		//}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

		if( 0 == _wcsicmp( name, L"name" ) )
		{
			break; 
		}

		//if( 0 == wcsicmp( name, L"id" ) )
		//{
		//	break; 
		//}

		if( 0 == _wcsicmp( name, L"lang" ) )
		{
			break; 
		}

		hr= E_INVALIDARG; 
		break; 
		if( 0 == _wcsicmp( name, L"id" ) )
		{
			break; 
		}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
		if( 0 == _wcsicmp( name, L"class" ) )
		{
			break; 
		}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

		if( 0 == _wcsicmp( name, L"bottommargin" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"leftmargin" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"rightmargin" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"topmargin" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"bottommargin" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"page-data" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( name, L"page-url" ) )
		{
			break; 
		}

		hr = S_OK; 

	}while( FALSE );

	return hr; 
}

INLINE HRESULT WINAPI need_html_element_text( LPCWSTR tag )
{
	HRESULT hr = S_OK; 

	do 
	{
		if( 0 == _wcsicmp( tag, L"A" ) )
		{
			break; 
		}

		if( 0 == _wcsicmp( tag, L"LI" ) )
		{
			break; 
		}

		hr = E_INVALIDARG; 

	} while ( FALSE );

	return hr; 
}

LRESULT WINAPI correct_xpath( LPCWSTR xpath, IHTMLDocument *doc )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		//找出多余的XPATH条目，通过高速运算，进行精简分析
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_filtered_html_text( IHTMLElement *element, 
									  ULONG max_level, 
									  ULONG cur_level, 
									  wstring &text )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT _hr; 
	wstring _text; 

	IHTMLElementPtr _parent; 
	IHTMLElementPtr sub_element; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	INT32 j; 
	LONG sub_element_count; 
	_variant_t id; 
	_variant_t index; 

#define TEMP_TEXT_LEN 128 
	WCHAR temp_text[ TEMP_TEXT_LEN ]; 
	
	do 
	{
		text = L""; 

		if( cur_level >= max_level )
		{
			ret = ERROR_INVALID_PARAMETER;
			break; 
		}

		//if( max_level > MAX_HTML_ELEMENT_TEXT_LEVEl )
		//{
		//	dbg_print( MSG_IMPORTANT, "the level of the html element to print is too high %d>%d\n", 
		//		level, 
		//		MAX_HTML_ELEMENT_TEXT_LEVEl ); 
		//}

		_hr = get_html_dom_element_text( element, _text ); 
		if( FAILED( _hr  ) )
		{
			//break; 
		}

		text += _text; 
		text += L'\n'; 

		_hr = element->get_children(&sub_element_disp); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "\nget the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
		if( FAILED(_hr)
			|| NULL == sub_elements)
		{
			dbg_print( MSG_ERROR, "\nget the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_elements->get_length( &sub_element_count ); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "\nget the item count of the collection error 0x%0.8x\n", _hr); 
			break; 
		}

		if( sub_element_count == 0 )
		{
			break; 
		}

		for( j = 0; j < sub_element_count; j ++ )
		{
			V_VT( &id ) = VT_I4;
			V_I4( &id ) = j;
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			do 
			{
				_hr = sub_elements->item( id, 
					index, 
					&sub_element_disp ); 

				if( FAILED( _hr ) 
					|| sub_element_disp == NULL )
				{
					dbg_print( MSG_ERROR, "\nget the item %d of the collection error 0x%0.8x\n", index, _hr); 

					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, 
					( PVOID* )&sub_element ); 

				if( FAILED( _hr )
					|| NULL == sub_element )
				{
					dbg_print( MSG_ERROR, "\nget the html interface of the item %d of the collection error 0x%0.8x\n", index, _hr); 

					ASSERT( FALSE ); 
					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				_hr = StringCchPrintfW( temp_text, ARRAYSIZE( temp_text ), L"\nCHILD[%u:%u]:\n", cur_level, j ); 
				text += temp_text; 
				if( FAILED(_hr ) )
				{
					break; 
				}

				if( cur_level + 1 >= max_level )
				{
					ret = ERROR_NOT_ENOUGH_QUOTA; 
					break; 
				}

				_hr = get_filtered_html_text( sub_element, max_level, cur_level + 1, _text ); 
				text += _text; 
				text += L'\n'; 

				if( FAILED( _hr ) )
				{
					break; 
				}
			}while( FALSE ); 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_html_element_child_text( IHTMLElement *html_element, 
										   ULONG max_level, 
										   wstring &text )
{
	LRESULT ret= ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != html_element ); 
		
		if( max_level > MAX_HTML_ELEMENT_TEXT_LEVEl )
		{
			dbg_print( MSG_IMPORTANT, "the level of the html element to print is too high %d>%d\n", 
				max_level, 
				MAX_HTML_ELEMENT_TEXT_LEVEl ); 
		}

		text = L""; 
		ret = get_filtered_html_text( html_element, max_level, 0, text ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI dump_html_relation( IHTMLElement *parent, IHTMLElement *child )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT _hr; 
	wstring html_text; 

	IHTMLElementPtr _parent; 

	do 
	{
		ASSERT( NULL != parent ); 
		ASSERT( NULL != child ); 

		ret = get_html_element_child_text( parent, 5, html_text); 
		if( ERROR_SUCCESS != ret )
		{
			
		}

		dbg_print_w( MSG_IMPORTANT, L"PARENT:\n%s\n", html_text.c_str() ); 

		ret = get_html_element_child_text( child, 2, html_text); 
		if( ERROR_SUCCESS != ret )
		{

		}

		dbg_print_w( MSG_IMPORTANT, L"\nCHILD:\n%s\n", html_text.c_str() ); 

		_hr = child->get_parentElement( &_parent ); 
		if( FAILED(_hr))
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		ret = get_html_element_child_text( _parent, 2, html_text); 
		if( ERROR_SUCCESS != ret )
		{

		}

		dbg_print_w( MSG_IMPORTANT, L"\nPARENT FROM CHILD:\n%s\n", html_text.c_str() ); 

	}while( FALSE ); 

	return ret; 
}

HRESULT WINAPI compare_html_element( IHTMLElement *src, IHTMLElement *dest )
{
	HRESULT hr = E_INVALIDARG; 
	_bstr_t html; 
	_bstr_t _html; 

	do 
	{
		ASSERT( NULL != src ); 
		ASSERT( NULL != dest ); 

		if( src == dest )
		{
			hr = S_OK; 
			break; 
		}

		hr = src->get_outerHTML( html.GetAddress() ); 
		if( FAILED( hr ) )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr = dest->get_outerHTML( _html.GetAddress() ); 
		if( FAILED( hr ) )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		if( html.operator wchar_t*() == NULL )
		{
			hr = E_UNEXPECTED; 
			//dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
			break; 
		}

		if( _html.operator wchar_t*() == NULL )
		{
			hr = E_UNEXPECTED; 
			//dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
			break; 
		}

		if( 0 == _wcsicmp( _html.operator wchar_t*(), html.operator wchar_t*() ) )
		{
			hr = S_OK; 
			break; 
		}
	}while( FALSE );

	return hr; 
}

LRESULT WINAPI _get_child_pos_in_parent( IHTMLElement *parent, 
									   IHTMLElement *child, 
									   LONG *index_out, 
									   ULONG flags )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	HRESULT _hr; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	LONG sub_element_count; 
	LONG i; 
	_variant_t id; 
	_variant_t index; 
	_bstr_t child_tag; 
	LPWSTR _child_tag; 

	do 
	{
		ASSERT( NULL != parent ); 
		ASSERT( NULL != child ); 
		ASSERT( NULL != index_out ); 

		*index_out = -1; 

		_hr = parent->get_children(&sub_element_disp); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
		if( FAILED(_hr)
			|| NULL == sub_elements)
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = child->get_tagName( child_tag.GetAddress() ); 
		if( FAILED( _hr ) )
		{
			break; 
		}

		_child_tag = child_tag.operator wchar_t*(); 

		if( NULL == _child_tag )
		{
			break; 
		}

		id = child_tag.GetBSTR(); 
		_hr = sub_elements->tags( id.GetVARIANT(), &sub_element_disp ); 
		if( FAILED( _hr ) )
		{
			break; 
		}

		_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
		if( FAILED(_hr)
			|| NULL == sub_elements)
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_elements->get_length( &sub_element_count ); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", _hr); 
			break; 
		}

		if( sub_element_count == 0 )
		{
			break; 
		}

		for( i = 0; i < sub_element_count; i ++ )
		{
			V_VT( &id ) = VT_I4;
			V_I4( &id ) = i;
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			do 
			{
				_hr = sub_elements->item( id, index, &sub_element_disp ); 
				if( FAILED( _hr ) 
					|| sub_element_disp == NULL )
				{
					dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", index, _hr); 

					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, 
					( PVOID* )&sub_element ); 
				if( FAILED( _hr )
					|| NULL == sub_element )
				{
					dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", index, _hr); 

					ASSERT( FALSE ); 
					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				do
				{
					_bstr_t html; 
					_bstr_t _html; 
					if( sub_element == child )
					{
						*index_out = i; 
						ret = ERROR_SUCCESS; 
					}

					if( 0 == ( flags & COMPARE_HTML_ELEMENT_CONTENT ) )
					{
						break; 
					}

					_hr = child->get_outerHTML( html.GetAddress() ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					_hr = sub_element->get_outerHTML( _html.GetAddress() ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					if( html.operator wchar_t*() == NULL )
					{
						dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
						break; 
					}

					if( _html.operator wchar_t*() == NULL )
					{
						dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
						break; 
					}

					if( 0 == _wcsicmp( _html.operator wchar_t*(), html.operator wchar_t*() ) )
					{
						ret = ERROR_SUCCESS; 
						break; 
					}
				}while( FALSE ); 
			} while ( FALSE ); 

			if( ret == ERROR_SUCCESS )
			{
				break; 
			}
		}
	} while ( FALSE ); 

	return ret; 
}

HRESULT WINAPI find_orphan_element_parent( IHTMLDocument2 *html_doc, 
										  IHTMLElement *element, 
										  IHTMLElement **parent, 
										  LONG *index_out )
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr elements; 
	IHTMLElementPtr _parent; 
	IHTMLElementPtr sub_element; 

	IHTMLElementCollectionPtr sub_elements; 
	//LONG element_count; 
	_bstr_t text; 
	_variant_t id; 
	_variant_t index; 
	LRESULT ret; 
	LONG sub_element_count;  
	INT32 i; 
	IHTMLElementPtr target_element; 
	HTML_ELEMENT_GROUP temp_group; 
	HTML_ELEMENT_GROUP _temp_group; 
	ULONG level; 
	LONG child_index; 

	do 
	{
		ASSERT( NULL != html_doc ); 
		ASSERT( NULL != element ); 
		ASSERT( NULL != parent ); 
		ASSERT( NULL != index_out ); 

		*parent = NULL; 
		*index_out = -1; 

		//hr = html_doc->get_all( &elements ); 
		//if( FAILED( hr ) ) 
		//{
		//	break; 
		//}

		//if( NULL == elements )
		//{
		//	break; 
		//}

		_parent = element; 

#define FIND_ORPHAN_PARENT_LEVEL 3
		for( i = 0; i < FIND_ORPHAN_PARENT_LEVEL; i ++ )
		{
			hr = _parent->get_parentElement( &_parent ); 
			if( FAILED( hr ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			if( NULL == _parent )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

		temp_group.push_back( _parent ); 

		level = 0; 
		for( ; ; )
		{
			if( temp_group.size() == 0 )
			{
				break; 
			}

			_parent = *temp_group.rbegin(); 
			temp_group.pop_back(); 

			ASSERT( NULL != _parent ); 

			do 
			{
				_hr = _parent->get_children(&sub_element_disp); 
				if( FAILED(_hr))
				{
					dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
					break; 
				}

				_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
				if( FAILED(_hr)
					|| NULL == sub_elements)
				{
					dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
					break; 
				}

				_hr = sub_elements->get_length( &sub_element_count ); 
				if( FAILED(_hr))
				{
					dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", _hr); 
					break; 
				}

				if( sub_element_count == 0 )
				{
					break; 
				}

				for( i = 0; i < sub_element_count; i ++ )
				{
					V_VT( &id ) = VT_I4;
					V_I4( &id ) = i;
					V_VT( &index ) = VT_I4;
					V_I4( &index ) = 0; 

					do 
					{
						_hr = sub_elements->item( id, index, &sub_element_disp ); 
						if( FAILED( _hr ) 
							|| sub_element_disp == NULL )
						{
							dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", index, _hr); 

							if( SUCCEEDED( _hr))
							{
								_hr = E_FAIL; 
							}

							break; 
						}

						_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, 
							( PVOID* )&sub_element ); 
						if( FAILED( _hr )
							|| NULL == sub_element )
						{
							dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", index, _hr); 

							ASSERT( FALSE ); 
							if( SUCCEEDED( _hr))
							{
								_hr = E_FAIL; 
							}

							break; 
						}

						ret = _get_child_pos_in_parent( sub_element, element, &child_index, COMPARE_HTML_ELEMENT_CONTENT ); 
						if( ret == ERROR_SUCCESS )
						{
							IHTMLElement *__parent; 
							dbg_print( MSG_IMPORTANT, "found orphan element parent!\n" ); 
							dump_html_relation( sub_element, element ); 

							sub_element->AddRef(); 
							*parent = sub_element; 
							*index_out = child_index; 

							do 
							{
								_hr = element->get_parentElement( &__parent ); 
								if( FAILED(_hr))
								{
									break; 
								}

								if( NULL == __parent )
								{
									break; 
								}

								ASSERT( __parent != sub_element ); 
							} while ( FALSE ); 
							break; 
						}

						if( level < FIND_ORPHAN_PARENT_LEVEL + 1 )
						{
							sub_element->AddRef(); 
							_temp_group.push_back( sub_element ); 
						}

					} while ( FALSE ); 

					if( ret == ERROR_SUCCESS )
					{
						ASSERT( *parent != NULL ); 
						break; 
					}
				}
			}while( FALSE );
			
			if( ret == ERROR_SUCCESS )
			{
				ASSERT( *parent != NULL ); 
				break; 
			}
			
			if( temp_group.size() == 0 )
			{
				HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it; 

				if( _temp_group.size() == 0 )
				{
					break; 
				}

				for( it = _temp_group.begin(); it != _temp_group.end(); it ++ )
				{
					temp_group.push_back( *it ); 
				}

				_temp_group.clear(); 

				level += 1; 
			}
		}
#if 0
		hr = element->get_tagName( text.GetAddress() ); 
		if( FAILED( hr ) )
		{
			break; 
		}

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

				do 
				{
					_hr = sub_elements->item( id, index, &sub_element_disp ); //Get an element

					if( _hr != S_OK )
					{
						break; 
					}

					if( NULL == sub_element_disp )
					{
						break; 
					}

					_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
					if( FAILED( _hr ) 
						|| sub_element == NULL )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					_hr = compare_html_element( sub_element, element ); 
					if( SUCCEEDED( _hr ) )
					{
						dbg_print( MSG_IMPORTANT, "found orphan element\n" ); 
					}
				}while( FALSE ); 
			}
		}
		else
		{
			hr = compare_html_element( sub_element, element ); 
			if( SUCCEEDED( hr ) )
			{
				dbg_print( MSG_IMPORTANT, "found orphan element\n" ); 
			}
		}
#endif //0

	}while( FALSE ); 

	release_html_element_group( &temp_group ); 

	return ret; 
}

LRESULT WINAPI check_parent_relation( IHTMLElement* parent, IHTMLElement* child ) 
{
	HRESULT _hr; 
	LRESULT ret = ERROR_NOT_FOUND; 
	IHTMLElementPtr sub_element; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	LONG sub_element_count; 
	_variant_t id; 
	_variant_t index; 
	LONG j; 

	do 
	{
		ASSERT( NULL != parent ); 
		ASSERT( NULL != child ); 

		//do
		//{
		//	IHTMLElementPtr _parent; 

		//	_hr = child->get_parentElement( &_parent ); 
		//	if( FAILED(_hr))
		//	{
		//		ret = ERROR_NOT_FOUND; 
		//		break; 
		//	}

		//	if( NULL == _parent )
		//	{
		//		ret = ERROR_NOT_FOUND; 
		//		break; 
		//	}

		//	if( parent == _parent )
		//	{
		//		break; 
		//	}

		//	{
		//		_bstr_t html; 
		//		_bstr_t _html; 

		//		_hr = parent->get_outerHTML( html.GetAddress() ); 
		//		if( FAILED( _hr ) )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			break; 
		//		}

		//		_hr = _parent->get_outerHTML( _html.GetAddress() ); 
		//		if( FAILED( _hr ) )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			break; 
		//		}

		//		if( html.operator wchar_t*() == NULL )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			//dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
		//			break; 
		//		}

		//		if( _html.operator wchar_t*() == NULL )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			//dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
		//			break; 
		//		}

		//		if( 0 != wcsicmp( _html.operator wchar_t*(), html.operator wchar_t*() ) )
		//		{
		//			//dbg_print_w( MSG_IMPORTANT, L"html element relation(to parent) is mismatch\n" ); 
		//			//dbg_print_w( MSG_IMPORTANT, "%s\n", _html.operator wchar_t*() ); 
		//			//dbg_print_w( MSG_IMPORTANT, L"%s\n", _html.operator wchar_t*() ); 
		//			ret = ERROR_NOT_FOUND; 
		//			break; 
		//		}
		//	}
		//}while( FALSE ); 

		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		//ret = ERROR_NOT_FOUND; 

		_hr = parent->get_children(&sub_element_disp); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID*)&sub_elements ); 
		if( FAILED(_hr)
			|| NULL == sub_elements)
		{
			dbg_print( MSG_ERROR, "get the child element error 0x%0.8x\n", _hr ); 
			break; 
		}

		_hr = sub_elements->get_length( &sub_element_count ); 
		if( FAILED(_hr))
		{
			dbg_print( MSG_ERROR, "get the item count of the collection error 0x%0.8x\n", _hr); 
			break; 
		}

		if( sub_element_count == 0 )
		{
			break; 
		}

		for( j = 0; j < sub_element_count; j ++ )
		{
			V_VT( &id ) = VT_I4;
			V_I4( &id ) = j;
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			do 
			{
				_hr = sub_elements->item( id, index, &sub_element_disp ); 
				if( FAILED( _hr ) 
					|| sub_element_disp == NULL )
				{
					dbg_print( MSG_ERROR, "get the item %d of the collection error 0x%0.8x\n", index, _hr); 

					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				_hr = sub_element_disp->QueryInterface( IID_IHTMLElement, 
					( PVOID* )&sub_element ); 
				if( FAILED( _hr )
					|| NULL == sub_element )
				{
					dbg_print( MSG_ERROR, "get the html interface of the item %d of the collection error 0x%0.8x\n", index, _hr); 

					ASSERT( FALSE ); 
					if( SUCCEEDED( _hr))
					{
						_hr = E_FAIL; 
					}

					break; 
				}

				{
					_bstr_t html; 
					_bstr_t _html; 
					if( child == sub_element )
					{
						ret = ERROR_SUCCESS; 
						break; 
					}

					_hr = child->get_outerHTML( html.GetAddress() ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					_hr = sub_element->get_outerHTML( _html.GetAddress() ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					if( html.operator wchar_t*() == NULL )
					{
						dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
						break; 
					}

					if( _html.operator wchar_t*() == NULL )
					{
						dbg_print( MSG_ERROR, "get the outer html error %d\n", _hr); 
						break; 
					}

					if( 0 == _wcsicmp( _html.operator wchar_t*(), html.operator wchar_t*() ) )
					{
						ret = ERROR_SUCCESS; 
						break; 
					}
				}
			} while ( FALSE ); 

			if( ret == ERROR_SUCCESS )
			{
				break; 
			}
		}
	} while ( FALSE );

	//if( ret != ERROR_SUCCESS )
	//{
	//	IHTMLDocument2Ptr doc; 
	//	IDispatchPtr disp; 

	//	do 
	//	{
	//	_hr = parent->get_document(&disp); 
	//	if( FAILED( _hr ) )
	//	{
	//		break; 
	//	}
	//
	//	_hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&doc ); 
	//	if( FAILED( _hr ) )
	//	{
	//		break; 
	//	}

	//	_hr = find_orphan_element( doc, child ); 

	//	} while ( FALSE );
	//	//dump_html_relation( parent, child ); 
	//}

	return ret; 
}

LRESULT WINAPI check_parent_relation_error(  vector< IHTMLElement* > &elements )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTMLElementPtr parent; 
	IHTMLElementPtr child; 
	INT32 bad_element; 
	vector< IHTMLElement* > bad_relate_elements; 
	LONG child_index; 

	do 
	{
		if( elements.size() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( elements.size() == 1 )
		{
			break; 
		}

		for( vector<IHTMLElement*>::iterator it = elements.begin(); 
			it != elements.end(); 
			)
		{
			if( it + 1 == elements.end() )
			{
				break; 
			}

			bad_element = 0; 

			child = ( *it ); 
			parent = ( *( it + 1 ) ); 

			do 
			{
				ret = _get_child_pos_in_parent( parent, child, &child_index, COMPARE_HTML_ELEMENT_CONTENT ); 
				if( ret == ERROR_SUCCESS )
				{
					break; 
				}

				ASSERT( child_index >= 0 ); 

				//if( it + 2 == elements.end() )
				//{
				//	bad_element = 1; 
				//	bad_relate_elements.push_back( child.GetInterfacePtr() ); 
				//	break; 
				//}

				//parent = ( *( it + 2 ) );
				//ret = check_parent_relation( parent, child ); 
				//if( ret == ERROR_SUCCESS )
				//{
				//	bad_relate_elements.push_back( *( it ) ); 
				//	break; 
				//}

				//if( it == elements.begin() )
				//{
				//	bad_element = 1; 
				//	bad_relate_elements.push_back( child.GetInterfacePtr() ); 
				//	break; 
				//}

				//child = ( *( it - 1 ) );
				//ret = check_parent_relation( parent, child ); 
				//if( ret == ERROR_SUCCESS )
				//{
				//	bad_relate_elements.push_back( *( it ) ); 
				//	break; 
				//}

				//if( ( it + 3 ) == elements.begin() )
				//{
				//	bad_element = 1; 
				//	bad_relate_elements.push_back( child.GetInterfacePtr() ); 
				//	break; 
				//}

				//parent = ( *( it + 3 ) );
				//ret = check_parent_relation( parent, child ); 
				//if( ret == ERROR_SUCCESS )
				//{
				//	bad_relate_elements.push_back( *( it ) ); 
				//	bad_relate_elements.push_back( *( it + 1 ) ); 
				//	break; 
				//}
			}while( FALSE ); 

			it += 1 + bad_element; 
		}

		for( vector<IHTMLElement*>::iterator it = bad_relate_elements.begin(); 
			it != bad_relate_elements.end(); 
			it ++ )
		{
			BOOLEAN erased = FALSE; 
			vector<IHTMLElement*>::iterator _it; 
			for( _it = elements.begin(); 
				_it != elements.end(); 
				_it ++ )
			{
				if( ( *it ) == ( *_it ) )
				{
					erased = TRUE; 
					elements.erase( _it ); 
					break; 
				}
			}

			ASSERT( erased == TRUE ); 
		}
	} while ( FALSE );

	bad_relate_elements.clear(); 

	return ret; 
}

//通过XPATH中条目在父节点中所有条目位置进行精确定位
LRESULT WINAPI _get_html_element_xpath( IHTMLElement *html_element, wstring &xpath, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementPtr _html_element; 
	IHTMLElementPtr _parent; 

	_bstr_t text; 
	LPCWSTR _text; 
	wstring element_xpath; 
	BOOLEAN have_attr; 
	vector< IHTMLElement* > xpath_elements; 
	vector< IHTMLElement* >::reverse_iterator it; 
	BOOLEAN have_text_attr; 

	LONG child_index; 
#define INTEGER_TEXT_LEN 24
	WCHAR temp_text[ INTEGER_TEXT_LEN ]; 

	do 
	{
		ASSERT( html_element != NULL ); 

		xpath = L""; 

		_html_element = html_element; 

		_html_element->AddRef(); 
		xpath_elements.push_back( _html_element ); 

		for( ; ; )
		{
			//可以进行父子，子父检证

			//dump_html_relation( parent, child ); 

			hr = _html_element->get_parentElement( &_parent ); 
			if( hr != S_OK )
			{
				//if( hr !=  )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
				}

				break; 
			}

			if( NULL == _parent )
			{
				break; 
			}

			if( flags & FIX_XPATH_ERROR )
			{
				ret = get_child_pos_in_parent( _parent, _html_element, &child_index ); 

				if( ERROR_SUCCESS != ret )
				{
					HRESULT _hr; 
					IHTMLDocument2Ptr doc; 
					IDispatchPtr disp; 
					IHTMLElementPtr parent_found; 

					//#define ORPHAN_ELEMENT_PREFIX L'~'
					//element_xpath += ORPHAN_ELEMENT_PREFIX; 

					_hr = _html_element->get_document( &disp ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					_hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&doc ); 
					if( FAILED( _hr ) )
					{
						break; 
					}

					_hr = find_orphan_element_parent( doc, _html_element, &parent_found, &child_index ); 
					if( FAILED( _hr ) )
					{
						//break; 
					}

					if( NULL == parent_found )
					{
						break; 
					}

					_parent = parent_found; 
				}

				ASSERT( child_index >= 0 ); 
			}

			_html_element = _parent; 
			_html_element->AddRef(); 
			xpath_elements.push_back( _html_element ); 
		}

		//check_parent_relation_error(xpath_elements); 
		
		for( it = xpath_elements.rbegin(); it != xpath_elements.rend(); it ++ )
		{
			have_attr = FALSE; 

			hr = ( *it )->get_tagName( text.GetAddress() ); 
			if( hr != S_OK )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_text = text.operator const wchar_t*(); 

			if( NULL == _text )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

#define _DEBUG_GET_XPATH 1
#ifdef _DEBUG_GET_XPATH
			if( 0 == _wcsicmp( _text, L"SPAN" ) )
			{
				//DebugBreak(); 
			}
#endif //_DEBUG_GET_XPATH

			element_xpath = L""; 
			element_xpath += _text; 

			if( it != xpath_elements.rbegin() 
				&& it - 1 != xpath_elements.rbegin() )
			{
				do 
				{
					ret = get_child_pos_in_parent( *( it - 1 ), *it, &child_index ); 

					if( ERROR_SUCCESS != ret )
					{
						if( flags & FIX_XPATH_ERROR )
						{
							ASSERT( FALSE ); 
						}
						break; 
					}

					ASSERT( child_index >= 0 ); 

					hr = StringCchPrintfW( temp_text, 
						ARRAYSIZE( temp_text ), 
						L"%d", 
						child_index ); 
					if( FAILED( hr ) )
					{
						//ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					//if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					element_xpath += HTML_ELEMENT_ORDER_VALUE_NAME; 
					element_xpath += L"="; 
					element_xpath += temp_text; 
					element_xpath += L";"; 
				}while( FALSE );
			}
			if( S_OK == need_html_element_text( _text ) )
			{
				have_text_attr = TRUE; 
			}
			else
			{
				have_text_attr = FALSE; 
			}

			hr = ( *it )->get_id( text.GetAddress() ); 
			if( hr != S_OK )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
			}
			else
			{
				if( FALSE == have_attr )
				{
					element_xpath += L" "; 
					have_attr = TRUE; 
				}

				element_xpath += L"id="; 
				element_xpath += _text; 
				element_xpath += L";"; 
			}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
			hr = ( *it )->get_className( text.GetAddress() ); 
			if( hr != S_OK )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
			}
			else
			{
				if( FALSE == have_attr )
				{
					element_xpath += L" "; 
					have_attr = TRUE; 
				}

				element_xpath += L"class="; 
				element_xpath += _text; 
				element_xpath += L";"; 
			}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

			if( TRUE == have_text_attr )
			{
				hr = ( *it )->get_innerText( text.GetAddress() ); 
				if( hr != S_OK )
				{
					ASSERT( FALSE ); 
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					//ASSERT( FALSE ); //go here when only embed a image.
					if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					element_xpath += HTML_ELEMENT_TEXT_PROPERTY_NAME L"="; 
					//element_xpath += L""; 
					element_xpath += L";"; 
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					//break; 
				}
				else if( text.length() == 0 )
				{
					ASSERT( FALSE ); 
					if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					element_xpath += HTML_ELEMENT_TEXT_PROPERTY_NAME L"="; 
					//element_xpath += L""; 
					element_xpath += L";"; 
				}
				else
				{
					if( FALSE == have_attr )
					{
						element_xpath += L" "; 
						have_attr = TRUE; 
					}

					{
						element_xpath += HTML_ELEMENT_TEXT_PROPERTY_NAME L"="; 
						{
							wstring _text_quote; 
							_text_quote = _text; 
							
							quote_html_text( _text_quote ); 

							element_xpath += _text_quote.c_str(); 
						}
						element_xpath += L";"; 
					}
				}
			}
			//get_html_index_in_parent

			{
				_bstr_t attr_name; 
				_variant_t attr_value; 

				//attr_name = L"name"; 

				//
				//	Default. Performs a property search that is not case-sensitive, and returns an interpolated value if the property is found.
				//	1
				//	Performs a case-sensitive property search. To find a match, the uppercase and lowercase letters in strAttributeName must exactly match those in the attribute name.
				//	2
				//	Returns attribute value as a BSTR. This flag does not work for event properties.
				//	4
				//	Returns attribute value as a fully expanded URL. Only works for URL attributes.

				//#ifdef _XPATH_BY_ATTRIBUTES
				do
				{
					IDispatchPtr disp; 
					IHTMLDOMAttributePtr attr; 
					IHTMLDOMNodePtr element_node; 
					IHTMLAttributeCollectionPtr attrs; 
					VARIANT_BOOL specified; 

					LONG attr_count; 
					_variant_t attr_name; 
					_bstr_t _attr_name; 

					LPWSTR _name; 
					wstring temp_text; 

					//IID iid = uuid("3051045d-98b5-11cf-bb82-00aa00bdce0b")
					hr = ( *it )->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&element_node); 
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

					for( INT32 i = 0; i < attr_count; i ++ )
					{
						do 
						{
							attr_name = i; 
							attrs->item( &attr_name, ( IDispatch** )&disp ); 

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

							hr = filter_common_html_attr( _name ); 
							if( hr != S_OK )
							{
								//ret = ERROR_ERRORS_ENCOUNTERED; 
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

							if( NULL == _text )
							{
							}
							else
							{
								if( FALSE == have_attr )
								{
									element_xpath += L" "; 
									have_attr = TRUE; 
								}

								element_xpath += _name; 
								element_xpath += L"="; 
								{
									wstring quote_text; 
									quote_text = _text; 
									quote_html_text( quote_text ); 
									element_xpath += quote_text.c_str(); 
								}
								element_xpath += L";"; 
							}
						} while ( FALSE ); 
					}
				}while( FALSE );
			}

			//if( FALSE == have_attr )
			//{
			//	element_xpath += L" "; 
			//}

			xpath += element_xpath; 
			xpath += L"|"; 

			( *it )->Release(); 
		}
	}while( FALSE );

	xpath_elements.clear(); 

	return ret; 
}

#define STD_XPATH_QUOTE_SIGN L"\"" 

LRESULT WINAPI get_html_element_std_xpath( IHTMLElement *html_element, wstring &xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	IHTMLElementPtr _html_element; 
	_bstr_t text; 
	LPCWSTR _text; 
	wstring element_xpath; 
	BOOLEAN have_attr; 
	vector< IHTMLElement* > xpath_elements; 
	vector< IHTMLElement* >::reverse_iterator it; 
	BOOLEAN have_text_attr; 
	LONG child_index; 

	do 
	{
		ASSERT( html_element != NULL ); 

		xpath = L""; 

		_html_element = html_element; 

		_html_element->AddRef(); 
		xpath_elements.push_back( _html_element ); 

		for( ; ; )
		{
			//have_attr = FALSE; 

			hr = _html_element->get_parentElement( &_html_element ); 
			if( hr != S_OK )
			{
				//if( hr !=  )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
				}

				break; 
			}

			if( NULL == _html_element )
			{
				break; 
			}

			_html_element->AddRef(); 
			xpath_elements.push_back( _html_element ); 
		}

		for( it = xpath_elements.rbegin(); it != xpath_elements.rend(); it ++ )
		{
			have_attr = FALSE; 

			do 
			{
				hr = ( *it )->get_tagName( text.GetAddress() ); 
				if( hr != S_OK )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text = text.operator const wchar_t*(); 

				if( NULL == _text )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

#ifndef SUPPORT_XPATH_STANDARD
				if( 0 == _wcsicmp( _text, L"A" ) )
				{
					have_text_attr = TRUE; 
				}
				else
				{
					have_text_attr = FALSE; 
				}
#endif //SUPPORT_XPATH_STANDARD

				element_xpath = L"/"; 

				{
					LPWSTR __text = NULL; 
					ULONG text_cc_len; 
					
					do 
					{
						text_cc_len = wcslen( _text ) + 1; 

						__text = ( LPWSTR )malloc( text_cc_len << 1 ); 
						if( NULL == __text )
						{
							break; 
						}

						memcpy( __text, _text, text_cc_len << 1 ); 
						element_xpath += wcslwr( __text ); 
					} while ( FALSE );
				}
				//"//INFOCROSS/InstallInfo/InstallStep[@StepName='1']/SystemUserPwd"

				hr = ( *it )->get_id( text.GetAddress() ); 
				if( hr != S_OK )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					if( it != xpath_elements.rbegin() 
						&& it - 1 != xpath_elements.rbegin() )
					{
						WCHAR temp_text[ INTEGER_TEXT_LEN ]; 

						ret = get_child_pos_in_parent( *( it - 1 ), *it, &child_index ); 

						if( ERROR_SUCCESS != ret )
						{
							break; 
						}

						ASSERT( child_index >= 0 ); 

						hr = StringCchPrintfW( temp_text, 
							ARRAYSIZE( temp_text ), 
							L"%d", 
							child_index + 1 ); 
						if( FAILED( hr ) )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						have_attr = TRUE; 
						element_xpath += L"["; 
						element_xpath += temp_text; 
						break; 
					}
				}
				else
				{
					//element_xpath += L" "; 

					if( FALSE == have_attr )
					{
						element_xpath += L"["; 
						have_attr = TRUE; 
					}

					element_xpath += L"@"; 
					element_xpath += L"id="; 
					element_xpath += STD_XPATH_QUOTE_SIGN; 
					element_xpath += _text; 
					element_xpath += STD_XPATH_QUOTE_SIGN; 
					break; 
				}

#ifdef COMPARE_HTML_ELEMENT_STYLE_CLASS
				hr = ( *it )->get_className( text.GetAddress() ); 
				if( hr != S_OK )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
				}
				else
				{
					element_xpath += L" "; 
					if( FALSE == have_attr )
					{
						element_xpath += L"["; 
						have_attr = TRUE; 
					}

					element_xpath += L"@"; 
					element_xpath += L"class="; 
					element_xpath += STD_XPATH_QUOTE_SIGN; 
					element_xpath += _text; 
					element_xpath += STD_XPATH_QUOTE_SIGN; 
				}
#endif //COMPARE_HTML_ELEMENT_STYLE_CLASS

#ifndef SUPPORT_XPATH_STANDARD
				if( TRUE == have_text_attr )
				{
					hr = ( *it )->get_innerText( text.GetAddress() ); 
					if( hr != S_OK )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					_text = text.operator const wchar_t*(); 
					if( NULL == _text )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					if( text.length() == 0 )
					{
						ASSERT( FALSE ); 
						element_xpath += L" "; 
						if( FALSE == have_attr )
						{
							element_xpath += L"["; 
							have_attr = TRUE; 
						}

						element_xpath += L"@"; 
						element_xpath += HTML_ELEMENT_TEXT_PROPERTY_NAME L"="; 
						element_xpath += STD_XPATH_QUOTE_SIGN; 
						element_xpath += STD_XPATH_QUOTE_SIGN; 
					}
					else
					{
						element_xpath += L" "; 
						if( FALSE == have_attr )
						{
							element_xpath += L"["; 
							have_attr = TRUE; 
						}

						element_xpath += L"@"; 
						element_xpath += HTML_ELEMENT_TEXT_PROPERTY_NAME L"="; 
						element_xpath += STD_XPATH_QUOTE_SIGN; 

						{
							wstring _text_quote; 
							_text_quote = _text; 

							quote_html_text( _text_quote ); 

							element_xpath += _text_quote.c_str(); 
						}

						element_xpath += STD_XPATH_QUOTE_SIGN; 
					}
				}
#endif //SUPPORT_XPATH_STANDARD

				//get_html_index_in_parent

				{
					_bstr_t attr_name; 
					_variant_t attr_value; 

					//attr_name = L"name"; 

					//
					//	Default. Performs a property search that is not case-sensitive, and returns an interpolated value if the property is found.
					//	1
					//	Performs a case-sensitive property search. To find a match, the uppercase and lowercase letters in strAttributeName must exactly match those in the attribute name.
					//	2
					//	Returns attribute value as a BSTR. This flag does not work for event properties.
					//	4
					//	Returns attribute value as a fully expanded URL. Only works for URL attributes.

					//#ifdef _XPATH_BY_ATTRIBUTES
					do
					{
						IDispatchPtr disp; 
						IHTMLDOMAttributePtr attr; 
						IHTMLDOMNodePtr element_node; 
						IHTMLAttributeCollectionPtr attrs; 
						VARIANT_BOOL specified; 

						LONG attr_count; 
						_variant_t attr_name; 
						_bstr_t _attr_name; 

						LPWSTR _name; 
						wstring temp_text; 

						//IID iid = uuid("3051045d-98b5-11cf-bb82-00aa00bdce0b")
						hr = ( *it )->QueryInterface( IID_IHTMLDOMNode, ( PVOID* )&element_node); 
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

						for( INT32 i = 0; i < attr_count; i ++ )
						{
							do 
							{
								attr_name = i; 
								attrs->item( &attr_name, ( IDispatch** )&disp ); 

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

								hr = filter_common_html_attr( _name ); 
								if( hr != S_OK )
								{
									//ret = ERROR_ERRORS_ENCOUNTERED; 
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

								if( NULL == _text )
								{
								}
								else
								{
									element_xpath += L" "; 
									if( FALSE == have_attr )
									{
										element_xpath += L"["; 
										have_attr = TRUE; 
									}

									element_xpath += L"@"; 
									element_xpath += _name; 
									element_xpath += L"="; 
									element_xpath += STD_XPATH_QUOTE_SIGN; 
									element_xpath += _text; 
									element_xpath += STD_XPATH_QUOTE_SIGN; 
								}
							} while ( FALSE ); 
						}
					}while( FALSE );
				}

			}while( FALSE );

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
			if( have_attr == TRUE )
			{
				element_xpath += L"]"; 
			}

			xpath += element_xpath; 

			( *it )->Release(); 
		}
	}while( FALSE );

	xpath_elements.clear(); 

	return ret; 
}

LRESULT WINAPI _get_html_element_xpath_ex( IHTML_ELEMENTS *html_elements, wstring &xpath, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTML_ELEMENT_ITERATOR it; 
	wstring sub_xpath; 

	do 
	{
		ASSERT( NULL != html_elements ); 

		xpath = L""; 

		for( it = html_elements->begin(); it != html_elements->end(); it ++ )
		{
			ret = _get_html_element_xpath( ( *it ), sub_xpath, flags ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			xpath += sub_xpath; 
		}

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI get_html_element_std_xpath_ex( IHTML_ELEMENTS *html_elements, wstring &xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTML_ELEMENT_ITERATOR it; 
	wstring sub_xpath; 

	do 
	{
		ASSERT( NULL != html_elements ); 

		xpath = L"/"; 

		for( it = html_elements->begin(); it != html_elements->end(); it ++ )
		{
			ret = get_html_element_std_xpath( ( *it ), sub_xpath ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			xpath += sub_xpath; 
		}

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI xpath2std_xpath(wstring xpath, wstring &std_xpath)
{
    LRESULT ret = ERROR_SUCCESS; 

    wstring element_xpath;
    wstring _xpath;

    XPATH_PARAMS **param_src = NULL;

    ULONG param_src_count = 0;
    ULONG param_index;
    BOOLEAN have_attr = FALSE; 

    XPATH_PARAM_ITERATOR param_src_it;

    do
    {
        std_xpath.clear(); 

        if (xpath.length() == 0)
        {
            ret = ERROR_INVALID_PARAMETER; 
            break; 
        }

        param_src = (XPATH_PARAMS**)malloc(sizeof(XPATH_PARAMS*) * MAX_XPATH_ELEMENT_PARAMETER_COUNT);
        if (NULL == param_src)
        {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        ret = get_xpath_params(xpath.c_str(),
            xpath.length(),
            param_src,
            MAX_XPATH_ELEMENT_PARAMETER_COUNT,
            &param_src_count);

        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        _xpath.clear(); 
        element_xpath = L"/";
        for (param_index = 0; param_index < param_src_count; param_index++)
        {
            element_xpath += L"/";

            param_src_it = param_src[param_index]->find(L"tag");
            if (param_src_it == param_src[param_index]->end())
            {
                ret = ERROR_INVALID_PARAMETER;
                break;
            }

            element_xpath += param_src_it->second; 

            do
            {
                param_src_it = param_src[param_index]->find(L"@");
                if (param_src_it != param_src[param_index]->end())
                {
                    element_xpath += L"[";
                    element_xpath += param_src_it->second.c_str();
                    element_xpath += L"]";
                    break;
                }

                have_attr = FALSE;

                for (param_src_it = param_src[param_index]->begin();
                    param_src_it != param_src[param_index]->end();
                    param_src_it++)
                {
                    if (0 == wcscmp(param_src_it->first.c_str(), L"tag"))
                    {
                        break;
                    }

                    if (0 == wcscmp(param_src_it->first.c_str(), L"@"))
                    {
                        break;
                    }

                    if (param_src_it->first.size() != 0
                        && param_src_it->second.size() != 0)
                    {
                        if (have_attr == FALSE)
                        {
                            element_xpath += L"[";
                            have_attr = TRUE;
                        }
                        else
                        {
                            element_xpath += L" and ";
                        }

                        element_xpath += L"@";
                        element_xpath += param_src_it->first.c_str();
                        element_xpath += L"=";
                        element_xpath += STD_XPATH_QUOTE_SIGN;
                        element_xpath += param_src_it->second.c_str();
                        element_xpath += STD_XPATH_QUOTE_SIGN;
                    }
                }

                if (have_attr == TRUE)
                {
                    element_xpath += L"]";
                }

                break;
            } while (FALSE);

            //element_xpath += param_src_it->second;
            //param_src_it = param_src[param_index]->find(L"id");
            //if (param_src_it != param_src[param_index]->end())
            //{
            //    element_xpath += L"[";
            //    element_xpath += L"@";
            //    element_xpath += param_src_it->first.c_str();
            //    element_xpath += L"=";
            //    element_xpath += STD_XPATH_QUOTE_SIGN;
            //    element_xpath += param_src_it->second.c_str();
            //    element_xpath += STD_XPATH_QUOTE_SIGN;
            //    element_xpath += L"]";
            //}
            //else
            //{
            //    param_src_it = param_src[param_index]->find(L"@");
            //    if (param_src_it != param_src[param_index]->end())
            //    {
            //        element_xpath += L"[";
            //        element_xpath += param_src_it->first.c_str();
            //        element_xpath += L"]";
            //    }
            //}

            _xpath += element_xpath;
            element_xpath.clear(); 
        }
        //dbg_print(MSG_IMPORTANT, "source xpath:\n%ws\n",
        //    xpath.c_str());

        dbg_print(MSG_IMPORTANT, "result xpath:\n%ws\n",
            xpath.c_str());

        std_xpath = _xpath;

    } while (FALSE);

    if (NULL != param_src)
    {
        release_xpath_params(param_src, param_src_count);
        free(param_src);
        param_src = NULL;
    }

    return ret; 
}

HRESULT WINAPI get_active_element_xpath( IHTMLDocument2 *html_doc, 
										wstring *xpath )
{
	HRESULT hr = S_OK; 
	LRESULT ret; 

	IDispatchPtr disp; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr _html_element; 

	_bstr_t text; 
	//LPCWSTR _text; 

	do 
	{
		if( html_doc == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = html_doc->get_activeElement( &_html_element ); 
		if( S_OK != hr 
			|| NULL == _html_element )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = get_active_element_xpath_ex( _html_element, xpath, NULL, &html_element ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_IMPORTANT, "add the html element to auto execution configure file error %u\n", ret ); 
		}
	}while( FALSE ); 

	return hr; 
}


LRESULT WINAPI get_element_xpath_by_position( IHTMLDocument2 *html_doc, 
											 LPPOINT pt, 
										wstring *xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IDispatchPtr disp; 
	IHTMLElementPtr html_element; 

	IHTML_ELEMENTS html_elements; 

	wstring _xpath; 

	_bstr_t text; 
	LPCWSTR _text; 

	BOOLEAN in_frame = FALSE; 
	//LPCWSTR _text; 

	do 
	{
		if( html_doc == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
		ASSERT(NULL != xpath );

		hr = html_doc->elementFromPoint( pt->x, pt->y, &html_element ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == html_element )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

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

				hr = html_doc->elementFromPoint( pt->x, pt->y, &html_sub_element ); 
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

		ret = _get_html_element_xpath_ex( &html_elements, _xpath, FIX_XPATH_ERROR ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			//break; 
		}

		*xpath = _xpath; 

		if( std::wstring::npos != _xpath.find( HTML_HIGHLIGT_ELEMENT_ID ) )
		{
			//DebugBreak(); 
		}
	}while( FALSE ); 

	ret = release_html_elements( &html_elements ); 
	if( ERROR_SUCCESS != ret )
	{

	}

	if( ret != ERROR_SUCCESS )
	{
		dbg_print( MSG_IMPORTANT, "add the html element to auto execution configure file error %u\n", ret ); 
	}

	return ret; 
} 

LRESULT WINAPI get_active_element_xpath_ex( IHTMLElement *html_element, 
										   wstring *xpath, 
										   wstring *std_xpath, 
										   IHTMLElement **html_element_out) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	IHTML_ELEMENTS html_elements; 

	wstring _xpath; 

	_bstr_t text; 
	LPCWSTR _text; 

	BOOLEAN in_frame = FALSE; 

	do
	{
		ASSERT(NULL != xpath );
		ASSERT(NULL != html_element); 
		ASSERT( NULL != html_element_out); 

		*html_element_out = NULL; 

		html_element->AddRef(); 
		html_elements.push_back( html_element ); 

		for( ; ; )
		{
			do 
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
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
			} while ( FALSE ); 

			if( ret != ERROR_SUCCESS )
			{
				html_element->Release(); 
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

				do 
				{
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
				}while( FALSE );

				html_element->Release(); 

				if( ERROR_SUCCESS != ret )
				{
					break; 
				}

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
			html_element->AddRef(); 
			*html_element_out = html_element; 
			break; 
		}

		if( std_xpath != NULL )
		{
			ret = get_html_element_std_xpath_ex( &html_elements, _xpath ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			} 

			*std_xpath = _xpath; 
		}

		ret = _get_html_element_xpath_ex( &html_elements, _xpath, FIX_XPATH_ERROR ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
			//break; 
		}

		ASSERT( html_elements.size() > 0 ); 
		( *( html_elements.rbegin() ) )->AddRef(); 
		*html_element_out = ( *( html_elements.rbegin() ) ); 

		*xpath = _xpath; 

		if( std::wstring::npos != _xpath.find( HTML_HIGHLIGT_ELEMENT_ID ) )
		{
			//DebugBreak(); 
		}
	}while( FALSE ); 

	ret = release_html_elements( &html_elements ); 
	if( ERROR_SUCCESS != ret )
	{

	}

	return ret; 
}

//LRESULT WINAPI get_element_on_pos_xpath_ex( IHTMLElement *html_element, 
//										   POINT *pos, 
//										   wstring *xpath ) 
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//
//	IHTML_ELEMENTS html_elements; 
//
//	wstring _xpath; 
//
//	_bstr_t text; 
//	LPCWSTR _text; 
//
//	BOOLEAN in_frame = FALSE; 
//
//	do
//	{
//		ASSERT(NULL != xpath );
//		ASSERT(NULL != html_element); 
//
//		html_element->AddRef(); 
//		html_elements.push_back( html_element ); 
//
//		for( ; ; )
//		{
//			hr = html_element->get_tagName( text.GetAddress() ); 
//			if( FAILED( hr ) )
//			{
//				ret = hr; 
//				break; 
//			}
//
//			_text = text.operator const wchar_t*(); 
//			if( NULL == _text )
//			{
//				break; 
//			}
//
//			if( 0 == wcsicmp( _text, L"iframe" ) 
//				|| 0 == wcsicmp( _text, L"frame" ) )
//			{
//				//IHTMLIFrameElement2 html_iframe; 
//				IHTMLFrameBase2Ptr html_frame; 
//				IHTMLWindow2Ptr html_window; 
//				IHTMLDocument2Ptr html_doc; 
//				IHTMLElementPtr html_sub_element; 
//				//IHTMLFrameElement2Ptr html_frame; 
//
//				in_frame = TRUE; 
//
//				hr = html_element->QueryInterface( IID_IHTMLFrameBase2, ( PVOID* )&html_frame ); 
//				if( FAILED( hr ) ) 
//				{
//					ret = hr; 
//					break; 
//				}
//
//				if( NULL == html_frame )
//				{
//					ret = ERROR_NOT_FOUND; 
//					break; 
//				}
//
//				hr = html_frame->get_contentWindow( &html_window ); 
//				if( FAILED( hr ) )
//				{
//					ret = hr; 
//					break; 
//				}
//
//				if( NULL == html_window )
//				{
//					ret = ERROR_NOT_FOUND; 
//					break; 
//				}
//
//				hr = html_window->get_document( &html_doc ); 
//				if( FAILED( hr ) )
//				{
//					ret = hr; 
//					break; 
//				}
//
//				hr = get_html_element_from_point( html_doc, pos, &html_sub_element ); 
//				if( FAILED( hr ) ) 
//				{
//					ret = hr; 
//					break; 
//				}
//
//				if( NULL == html_sub_element )
//				{
//					ret = ERROR_NOT_FOUND; 
//					break; 
//				}
//
//				html_element->Release(); 
//
//				html_element = html_sub_element; 
//
//				html_sub_element->AddRef(); 
//				html_elements.push_back( html_sub_element ); 
//
//				html_element->AddRef(); 
//			}
//			else
//			{
//				break; 
//			}
//		}
//
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		ret = get_html_element_xpath_ex( &html_elements, _xpath ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			dbg_print( MSG_IMPORTANT, "get html element xpath error 0x%0.8x\n", ret ); 
//			//break; 
//		}
//
//		*xpath = _xpath; 
//	}while( FALSE ); 
//
//	ret = release_html_elements( &html_elements ); 
//	if( ERROR_SUCCESS != ret )
//	{
//
//	}
//
//	return ret; 
//}

LRESULT WINAPI release_html_elements( IHTML_ELEMENTS *html_elements )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTML_ELEMENT_ITERATOR it; 

	do 
	{
		ASSERT( html_elements != NULL ); 

		for( it = html_elements->begin(); 
			it != html_elements->end(); 
			it ++ )
		{
			( *it )->Release(); 
		}

	}while( FALSE );

	html_elements->clear(); 
	return ret; 
}

/*********************************************************************

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Windows.Forms;

namespace myTest.WinFormsApp
{
    public partial class MainForm : Form
{
    public MainForm()
    {
        InitializeComponent();
    }

    private void MainForm_Load(object sender, EventArgs e)
    {
        webBrowser1.DocumentText = @"
            <html>
            <body>
            < img alt = ""0764547763 Product Details""
            src = ""http://ecx.images-amazon.com/images/I/51AK1MRIi7L._AA160_.jpg"">
        <hr / >
            <h2>Product Details< / h2>
            <ul>
            <li><b>Paperback : < / b> 648 pages< / li>
            <li><b>Publisher : < / b> Wiley; Unlimited Edition edition(October 15, 2001) < / li >
            <li><b>Language:< / b> English< / li>
            <li><b>ISBN - 10 : < / b> 0764547763 < / li >
            < / html>
            ";
    }

    private void cmdTest_Click(object sender, EventArgs e)
    {
        var processor = new WebBrowserControlXPathQueriesProcessor(webBrowser1);

        // change attributes of the first element of the list
        {
            var li = processor.GetHtmlElement("//li");
            li.innerHTML = string.Format("<span style='text-transform: uppercase;font-family:verdana;color:green;'>{0}</span>", li.innerText);
        }

        // change attributes of the second and subsequent elements of the list
        var list = processor.GetHtmlElements("//ul//li");
        int index = 1;
        foreach(var li in list)
        {
            if (index++ == 1) continue;
            li.innerHTML = string.Format("<span style='text-transform: uppercase;font-family:verdana;color:blue;'>{0}</span>", li.innerText);
        }

    }

    /// <summary>
    /// Enables IE WebBrowser control to evaluate XPath queries 
    /// by injecting http://svn.coderepos.org/share/lang/javascript/javascript-xpath/trunk/release/javascript-xpath-latest-cmp.js
    /// and to return XPath queries results to the calling C# code as strongly typed
    /// mshtml.IHTMLElement and IEnumerable<mshtml.IHTMLElement>
    /// </summary>
    public class WebBrowserControlXPathQueriesProcessor
    {
        private System.Windows.Forms.WebBrowser _webBrowser;
        public WebBrowserControlXPathQueriesProcessor(System.Windows.Forms.WebBrowser webBrowser)
        {
            _webBrowser = webBrowser;
            injectScripts();
        }

        private void injectScripts()
        {
            // Thanks to: http://stackoverflow.com/questions/7998996/how-to-inject-javascript-in-webbrowser-control

            HtmlElement head = _webBrowser.Document.GetElementsByTagName("head")[0];
            HtmlElement scriptEl = _webBrowser.Document.CreateElement("script");
            mshtml.IHTMLScriptElement element = (mshtml.IHTMLScriptElement)scriptEl.DomElement;
            element.src = "http://svn.coderepos.org/share/lang/javascript/javascript-xpath/trunk/release/javascript-xpath-latest-cmp.js";
            head.AppendChild(scriptEl);

            string javaScriptText = @"
                function GetElements(XPath) {
                var xPathRes = document.evaluate(XPath, document, null, XPathResult.ORDERED_NODE_ITERATOR_TYPE, null);
                var nextElement = xPathRes.iterateNext();
                var elements = new Object();
                var elementIndex = 1;
                while (nextElement) {
                    elements[elementIndex++] = nextElement;
                    nextElement = xPathRes.iterateNext();
                }
                elements.length = elementIndex - 1;
                return elements;
            };
            ";
                scriptEl = _webBrowser.Document.CreateElement("script");
            element = (mshtml.IHTMLScriptElement)scriptEl.DomElement;
            element.text = javaScriptText;
            head.AppendChild(scriptEl);
        }

        /// <summary>
        /// Gets Html element's mshtml.IHTMLElement object instance using XPath query
        /// </summary>
        public mshtml.IHTMLElement GetHtmlElement(string xPathQuery)
        {
            string code = string.Format("document.evaluate('{0}', document, null, XPathResult.FIRST_ORDERED_NODE_TYPE, null).singleNodeValue;", xPathQuery);
            return _webBrowser.Document.InvokeScript("eval", new object[]{ code }) as mshtml.IHTMLElement;
        }

        /// <summary>
        /// Gets Html elements' IEnumerable<mshtml.IHTMLElement> object instance using XPath query
        /// </summary>
        public IEnumerable<mshtml.IHTMLElement> GetHtmlElements(string xPathQuery)
        {
            // Thanks to: http://stackoverflow.com/questions/5278275/accessing-properties-of-javascript-objects-using-type-dynamic-in-c-sharp-4
            var comObject = _webBrowser.Document.InvokeScript("eval", new object[]{ string.Format("GetElements('{0}')", xPathQuery) });
            Type type = comObject.GetType();
            int length = (int)type.InvokeMember("length", BindingFlags.GetProperty, null, comObject, null);

            for (int i = 1; i <= length; i++)
            {
                yield return type.InvokeMember(i.ToString(), BindingFlags.GetProperty, null, comObject, null) as mshtml.IHTMLElement;
            }
        }
    }

}
}
**********************************************************************/