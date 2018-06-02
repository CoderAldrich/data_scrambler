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
#include "mshtml_addition.h"
//#import <mshtml.tlb>
#define XPATH_CREATE
#include "html_xpath.h"
#include "input_content.h"
#include "external_link.h"
#include "html_event_handler.h"

#ifndef DATA_SCRAMBLER_BROWSER
HRESULT WINAPI disable_web_page_anchor( IHTMLDocument2 *page )
{
	HRESULT hr = S_OK; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	IHTMLEventObjPtr event_object; 

	_variant_t id; 
	_variant_t index; 

	IDispatchPtr sub_element_disp; 

	INT32 i; 
	LONG sub_element_count; 
	HtmlElementEventHandler *myaction = new HtmlElementEventHandler(); 
	VARIANT vDocDisp; 
	BOOLEAN element_handler_set = FALSE; 
	//BOOLEAN is_inserted_element; 

	do 
	{
		ASSERT( NULL != page ); 

		//dump_html_elements_on_page( page ); 

		hr = page->get_all( &sub_elements ); 
		if( FAILED(hr) 
			&& NULL == sub_elements ) 
		{ 
			break; 
		}

		hr = sub_elements->get_length( &sub_element_count ); 
		if( FAILED( hr ) )
		{
			break; 
		}

		if( sub_element_count == 0 )
		{
			hr = E_FAIL; 
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
				element_handler_set = FALSE; 
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
					break; 
				}

				//dump_html_element( sub_element ); 

				do
				{
					_bstr_t text; 

					hr = sub_element->get_tagName( text.GetAddress() ); 
					if( FAILED(hr) 
						|| text.operator const wchar_t*() == NULL )
					{
						break; 
					}

					if( 0 != _wcsicmp( text.operator const wchar_t*(), L"A" ) 
						&& 0 != _wcsicmp( text.operator const wchar_t*(), L"EM" ))
					{
						break; 
					}

					vDocDisp.vt = VT_DISPATCH; 
					vDocDisp.pdispVal = myaction; 
					hr = sub_element->put_onclick(vDocDisp); 

					if( FAILED( hr ) )
					{
						break; 
					}

					element_handler_set = TRUE; 
				}while( FALSE ); 

				if( TRUE == element_handler_set )
				{
					break; 
				}

				//is_inserted_element = FALSE; 

				//do
				//{
				//	_bstr_t _text; 
				//	LPWSTR __text; 
				//	hr = sub_element->get_id( _text.GetAddress() ); 
				//	if( FAILED(hr))
				//	{
				//		break; 
				//	}

				//	__text = _text.operator wchar_t*(); 

				//	if( NULL == __text )
				//	{
				//		break; 
				//	}

				//	if( 0 == wcsicmp( __text, HTML_HIGHLIGT_ELEMENT_ID ) )
				//	{
				//		is_inserted_element = TRUE; 
				//	}
				//}while( FALSE ); 

				//if( TRUE == is_inserted_element )
				//{
				//	break; 
				//}

				hr = html_element_have_child( sub_element ); 
				if( hr == S_OK )
				{
					break; 
				}

				vDocDisp.vt = VT_DISPATCH; 
				vDocDisp.pdispVal = myaction; 
				hr = sub_element->put_onclick(vDocDisp); 

				if( FAILED( hr ) )
				{
					break; 
				}
			}while( FALSE ); 
		}
		//element->attachEvent()
	}while( FALSE );

	if( NULL != myaction )
	{
		myaction->Release(); 
	}

	return hr; 
}

#endif //DATA_SCRAMBLER_BROWSER