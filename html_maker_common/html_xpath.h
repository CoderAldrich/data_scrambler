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

#ifndef __HTML_XPATH_H__
#define __HTML_XPATH_H__

#define WM_XPATH_MISMATCH_INFO WM_USER + (0xa01)

typedef struct _HTML_WORD_INFORMATION
{
	//LONG word_begin; 
	LONG word_end; 
	LONG left_end; 
	wstring word; 
} HTML_WORD_INFORMATION, *PHTML_WORD_INFORMATION; 

typedef vector<HTML_WORD_INFORMATION*> HTML_WORDS; 
typedef vector<HTML_WORD_INFORMATION*>::iterator HTML_WORD_ITERATOR; 
typedef vector<HTML_WORD_INFORMATION*>::reverse_iterator HTML_WORD_REVERSE_ITERATOR;  

typedef enum _HTML_TEXT_STATE
{
	HTML_TEXT_BEGIN, 
	HTML_TAG_BEGIN, 
	//HTML_TAG_END, 
	HTML_ATTIBUTE_VALUE_OR_NAME_BEGIN,
	HTML_ATTIBUTE_NAME_BEGIN_OR_TAG_END, 
	//HTML_ATTIBUTE_VALUE_BEGIN, 
	//HTML_ATTIBUTE_VALUE_END, 
	HTML_ATTIBUTE_STRING_VALUE_BEGIN, 
	//HTML_ATTIBUTE_STRING_VALUE_END, 
	UNKNOWN_HTML_TEXT_STATE, 
} HTML_TEXT_STATE, *PHTML_TEXT_STATE; 

typedef enum _HTML_WORD_STATE
{
	HTML_TAG_LEFT, 
	HTML_TAG_RIGHT, 
	UNKNOWN_HTML_WORD_STATE, 
} HTML_WORD_STATE, *PHTML_WORD_STATE;  

#define XML_ELEMENT_END_CHAR L'>' 
#define XPATH_PARAM_VALUE_SIGN L'='

#include <map>
typedef map< wstring, wstring > XPATH_PARAMS; 
typedef XPATH_PARAMS::iterator XPATH_PARAM_ITERATOR; 
typedef pair< wstring, wstring > XPATH_PARAM_PAIR;

typedef vector<IHTMLDocument2*> HTML_DOCUMENT_FAMILY; 
//typedef struct _XPATH_PARAMS
//{
//	wstring tag; 
//	wstring id; 
//	wstring name; 
//} XPATH_PARAMS, *PXPATH_PARAMS; 

#define MAX_XPATH_ELEMENT_PARAMETER_COUNT 128
#define MAX_XPATH_PARAMETER_STRING_LENGTH 260
#define XPATH_PARAMETER_STRING_DELIM L"|"
#define XPATH_PARAMETER_STRING_DELIM_QUOTE L"&st"
#define XPATH_SUB_PARAMETER_STRING_DELIM L';'
#define XPATH_SUB_PARAMETER_VALUE_SIGN L'='
#define XPATH_TAG_PARAMTER_DELIM L' '
#define XPATH_TAG L"tag"

#ifdef XPATH_CREATE

//LRESULT WINAPI release_html_element_groups( HTML_ELEMENT_GROUP groups[] ); 

//LRESULT WINAPI add_html_element_to_groups( IHTMLElement *html_element, 
//										 HTML_ELEMENT_GROUP groups[ MAX_HTML_ELEMENT_ACTION ] ); 

INLINE LRESULT WINAPI add_html_element_to_group( IHTMLElement *html_element, 
										 HTML_ELEMENT_GROUP *group )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		html_element->AddRef(); 
		group->push_back( html_element ); 
	} while ( FALSE );

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
								 HTML_ELEMENT_GROUP *group ); 

LRESULT WINAPI locate_html_element_from_xpath( HTML_ELEMENT_GROUP *element_group, 
											  XPATH_PARAMS *xpaths[], 
											  ULONG xpath_count, 
											  HTML_ELEMENT_GROUP *group, 
											  LPCWSTR target_xpath, 
											  BOOLEAN analyze_trace ); 

LRESULT WINAPI set_xpath_info_ui_window( HWND window ); 
HWND WINAPI get_xpath_info_ui_window(); 

//BOOLEAN WINAPI get_xpath_info_analyze(); 
//LRESULT WINAPI set_xpath_info_analyze( BOOLEAN _switch ); 

LRESULT WINAPI xpath_params_to_xpath( XPATH_PARAMS *params[], ULONG count, wstring &xpath_out ); 

#define ERROR_XPATH_IS_FIXED 0xE1011110

LRESULT WINAPI check_and_fix_links_path( LPCWSTR xpath, wstring &xpath_out, wstring &error_msg ); 

LRESULT WINAPI print_xpath_mismatch_element( IHTMLElement *html_element, LPCWSTR xpath, BOOLEAN analyze_xpath ); 
LRESULT WINAPI prepare_xpath_location(); 

LRESULT WINAPI get_html_elements_from_xpath( LPCWSTR xpath, 
											IHTMLDocument2 *html_doc, 
											HTML_ELEMENT_GROUP groups[], 
											BOOLEAN analyze_xpath ); 

HRESULT WINAPI get_html_dom_element_text( IHTMLElement *html_element, wstring &html_text_out ); 

LRESULT WINAPI copy_html_element_group( HTML_ELEMENT_GROUP *group_src, HTML_ELEMENT_GROUP *group_dest ); 

HRESULT WINAPI get_active_element_xpath( IHTMLDocument2 *html_doc, 
										wstring *xpath ); 

LRESULT WINAPI get_active_element_xpath_ex( IHTMLElement *html_element, 
										   wstring *xpath, 
										   wstring *std_xpath, 
										   IHTMLElement **html_element_out); 

LRESULT WINAPI get_element_xpath_by_position( IHTMLDocument2 *html_doc, 
											 LPPOINT pt, 
											 wstring *xpath ); 

#define HTML_ELEMENT_TEXT_PROPERTY_NAME L"element_text"

typedef vector< IHTMLElement* > IHTML_ELEMENTS; 
typedef vector< IHTMLElement* >::iterator IHTML_ELEMENT_ITERATOR; 

#define FIX_XPATH_ERROR 0x00000001
#define HTML_ELEMENT_ORDER_VALUE_NAME L"@" 
HRESULT WINAPI compare_html_element_attributes_ex( XPATH_PARAMS *xpath, 
												  IHTMLElement *element); 
LRESULT WINAPI _get_html_element_xpath( IHTMLElement *html_element, wstring &xpath, ULONG flags ); 
INLINE LRESULT WINAPI get_html_element_xpath( IHTMLElement *html_element, wstring &xpath ) 
{
	return _get_html_element_xpath( html_element, xpath, 0 ); 
}

LRESULT WINAPI _get_html_element_xpath_ex( IHTML_ELEMENTS *html_elements, wstring &xpath, ULONG flags ); 

INLINE LRESULT WINAPI get_html_element_xpath_ex( IHTML_ELEMENTS *html_elements, wstring &xpath ) 
{
	return _get_html_element_xpath_ex( html_elements, xpath, 0 ); 
}

LRESULT WINAPI release_html_elements( IHTML_ELEMENTS *html_elements ); 

HRESULT WINAPI compare_html_element_attributes( XPATH_PARAMS *xpath, 
											   IHTMLElement *element); 

LRESULT WINAPI get_selected_html_elements(IHTMLDocument2 *html_doc, HTML_ELEMENT_GROUP *elements); 

LRESULT WINAPI get_html_element_std_xpath_ex( IHTML_ELEMENTS *html_elements, wstring &xpath ); 

#define HTML_HIGHLIGT_ELEMENT_ID L"xpath_checking" 
HRESULT WINAPI disable_web_page_anchor( IHTMLDocument2 *page ); 

HRESULT WINAPI dump_html_element( IHTMLElement *html_element ); 

HRESULT WINAPI dump_html_elements_on_page( IHTMLDocument2 *html_doc ); 

HRESULT WINAPI dump_html_elements( HTML_ELEMENT_GROUP *group ); 

HRESULT WINAPI html_element_have_child( IHTMLElement *html_element ); 

HRESULT WINAPI find_html_element_from_pos( HTML_ELEMENT_GROUP *group, POINT *pos, IHTMLElement **element ); 

HRESULT WINAPI get_html_element_from_point( IHTMLDocument2* doc, 
										   POINT *pos, 
										   IHTMLElement **element ); 


LRESULT WINAPI html_window_2_html_document(IHTMLWindow2 *window, IHTMLDocument2 **document ); 

LRESULT WINAPI html_window_2_html_web_browser(IHTMLWindow2 *window, IWebBrowser2 **web_browser); 

HRESULT WINAPI get_html_element_xpath_from_point( IHTMLDocument2* html_doc, 
												 POINT *pos, 
												 POINT *screen_pos, 
												 IHTMLElement *html_element, 
												 wstring *xpath, 
												 wstring *std_xpath, 
												 IHTMLElement **html_element_out ); 

LRESULT WINAPI _get_html_element_position( HTML_DOCUMENT_FAMILY *html_docs, 
										 IHTMLElement *html_element, 
										 RECT *rect ); 

#define TRVERSE_ALL_FRAMES 0x00000001
LRESULT WINAPI get_html_element_position_ex( IHTMLDocument2 *html_doc_parent, 
											IHTMLElement *html_element, 
											RECT *rect, 
											ULONG flags ); 

INLINE LRESULT WINAPI get_html_element_position( IHTMLDocument2 *html_doc_parent, 
												   IHTMLElement *html_element, 
												   RECT *rect )
{
	return get_html_element_position_ex( html_doc_parent, html_element, rect, 0 ); 
}

LRESULT WINAPI html_window_2_parent_html_document( HTML_DOCUMENT_FAMILY *doc_family, 
												  IHTMLWindow2 *html_window, 
												  IHTMLFrameBase2 **frame ); 

//LRESULT WINAPI get_element_on_pos_xpath_ex( IHTMLElement *html_element, 
//										   POINT *pos, 
//										   wstring *xpath ); 

#endif //XPATH_CREATE


LRESULT WINAPI escape_html_text( wstring &html_text ); 

LRESULT WINAPI quote_html_text( wstring &html_text ); 

LRESULT WINAPI get_xpath_params( LPCWSTR param, 
								ULONG cc_param_len, 
								XPATH_PARAMS *xpaths[], 
								ULONG max_param_count, 
								ULONG *param_count ); 

LRESULT WINAPI xpath2std_xpath(wstring xpath, wstring &std_xpath); 

LRESULT WINAPI parse_xpath_params( LPCWSTR param, 
								  ULONG cc_param_len, 
								  XPATH_PARAMS *params ); 

LRESULT WINAPI release_xpath_params( XPATH_PARAMS **params, ULONG params_count ); 
LRESULT WINAPI append_element_xpath( XPATH_PARAMS **xpath_params, ULONG param_count, wstring &xpath_desc ); 

LRESULT WINAPI fix_html_attribute_format( wstring &word, wstring &word_out ); 
LRESULT WINAPI fix_html_tag_end( wstring &word, HTML_WORDS *tags, HTML_WORDS *bug_tags ); 
VOID WINAPI wstring_tolower( wstring &dest_string ); 

#define _XPATH_BY_ATTRIBUTES 1
LRESULT WINAPI xor_html_element_xpath( wstring xpath_src, wstring &xpath_dest ); 
LRESULT WINAPI filter_xpath_noisy( wstring &target_xpath ); 


#define MAX_HTML_ELEMENT_TEXT_LEVEl 32 

LRESULT WINAPI get_html_element_child_text( IHTMLElement *html_element, 
										   ULONG max_level, 
										   wstring &text ); 

LRESULT WINAPI get_filtered_html_text( IHTMLElement *element, 
									  ULONG max_level, 
									  ULONG cur_level, 
									  wstring &text ); 


LRESULT WINAPI html_window_2_html_frame( IHTMLDocument2 *html_doc, 
										IHTMLWindow2 *html_window, 
										IHTMLFrameBase2 **frame ); 

LRESULT WINAPI check_html_frame_window( IHTMLElement *element, 
									   IHTMLWindow2 *html_window ); 


LRESULT WINAPI get_frames_on_html_document( IHTMLDocument2 *html_doc, HTML_DOCUMENT_FAMILY *doc_family ); 
LRESULT WINAPI get_frames_family_on_html_document( IHTMLDocument2 *html_doc, HTML_DOCUMENT_FAMILY *doc_family ); 


#define COMPARE_HTML_ELEMENT_CONTENT 0x00000001
LRESULT WINAPI _get_child_pos_in_parent( IHTMLElement *parent, 
										IHTMLElement *child, 
										LONG *index_out, 
										ULONG flags ); 

INLINE LRESULT WINAPI get_child_pos_in_parent( IHTMLElement *parent, 
										IHTMLElement *child, 
										LONG *index_out ) 
{
	return _get_child_pos_in_parent( parent, child, index_out, 0 ); 
}

LRESULT WINAPI check_parent_relation( IHTMLElement* parent, IHTMLElement* child ); 

#endif //__HTML_XPATH_H__