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

#ifndef __HTML_ELEMENT_CONFIG_H__
#define __HTML_ELEMENT_CONFIG_H__

#import "msxml.tlb"

typedef vector< IHTMLElement* > IHTML_ELEMENTS; 
typedef vector< IHTMLElement* >::iterator IHTML_ELEMENT_ITERATOR; 

LRESULT WINAPI add_html_element_config_ex(  MSXML::IXMLDOMDocument *xml_doc, 
										  HTML_ELEMENT_ACTION *element_config ); 

LRESULT WINAPI find_page_group_config( LPCWSTR file_name, 
									  LPCWSTR domain_name, 
									  MSXML::IXMLDOMDocument **xml_doc_out, 
									  MSXML::IXMLDOMElement **site_element_out ); 

LRESULT WINAPI add_page_config( MSXML::IXMLDOMDocument *xml_doc, 
							   LPCWSTR url, 
							   MSXML::IXMLDOMElement *site_node, 
							   MSXML::IXMLDOMElement **page_node ); 

LRESULT WINAPI format_html_text( wstring html_text, wstring &html_text_out, IHTMLElement *html_element = NULL ); 

LRESULT WINAPI add_html_element_config(  MSXML::IXMLDOMDocument *xml_doc, 
									   LPCWSTR url, 
									   MSXML::IXMLDOMNode *page_node, 
									   IHTMLElement *html_element ); 

LRESULT WINAPI make_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len ); 

LRESULT WINAPI get_html_element_input_value( IHTMLElement *html_element, wstring &input_value, BOOLEAN &is_input ); 

LRESULT WINAPI create_config_file( LPCWSTR config_file_name ); 

LRESULT WINAPI release_html_elements( IHTML_ELEMENTS *html_elements ); 

LRESULT WINAPI insert_node_by_text(MSXML::IXMLDOMDocument *xml_src_doc, MSXML::IXMLDOMElement *dest_node); 

#endif //__HTML_ELEMENT_CONFIG_H__