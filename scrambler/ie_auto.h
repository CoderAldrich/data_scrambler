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
#ifndef __IE_AUTO_H__
#define __IE_AUTO_H__

#include "html_action.h"

///html/body/div[10]/div[2]/div[8]/div[2]/div[1]/div[1]/a
HRESULT WINAPI get_html_element_from_xpath_ex( LPCWSTR xpath, 
									   HTML_ELEMENT_GROUP *group, 
									   IWebBrowser2 *web_browser, 
									   BOOLEAN analzye_xpath ); 

LRESULT WINAPI ie_auto_execute_on_element_ex( HTML_ELEMENT_INFO *element_info, 
											 IWebBrowser2 *web_browser ); 

LRESULT WINAPI ie_auto_execute_on_element( HTML_ELEMENT_INFO *element ); 

LRESULT WINAPI _ie_auto_execute_on_element( IHTMLElement *html_element, 
										   HTML_ELEMENT_INFO *element ); 

LRESULT WINAPI __ie_auto_execute_on_element( IHTMLElement *element, 
											HTML_ELEMENT_ACTIONS *actions, 
											HTML_ELEMENT_INFO *element_info ); 

HRESULT WINAPI ie_auto_execute_hover_element( IHTMLElement *element); 

LRESULT WINAPI ie_auto_execute_on_page( HTML_PAGE_INFO *page, IDispatch *html_doc_disp, IWebBrowser2 *ie_window ); 

LRESULT WINAPI ie_auto_execute_on_site( HTML_WEB_SITE_INFO *site ); 

LRESULT WINAPI _ie_auto_execute_on_site( IDispatch *html_doc_disp, 
										HTML_WEB_SITE_INFO *site ); 

//LRESULT WINAPI create_browser_dlg(); 
//LRESULT WINAPI ie_auto_execute_on_site_internal( HTML_WEB_SITE_INFO *site ); 

LRESULT WINAPI ie_auto_execute_on_site_async( HTML_WEB_SITE_INFO *site );

LRESULT WINAPI ie_auto_execute_on_sites_async( HTML_WEB_SITE_INFOS *sites ); 
LRESULT WINAPI ie_auto_execute_on_sites( HTML_WEB_SITE_INFOS *sites ); 

LRESULT WINAPI init_ie_auto(); 
LRESULT WINAPI uninit_ie_auto(); 

void EnumIE(void);
// void EnumFrame(IHTMLDocument2 * pIHTMLDocument2)
void EnumFrame(IHTMLDocument2 * pIHTMLDocument2);
// void EnumForm(IHTMLDocument2 * pIHTMLDocument2)  
void EnumForm(IHTMLDocument2 * pIHTMLDocument2,IDispatch*spDispDoc);
// void EnumField(CComDispatchDriver spInputElement,CString ComType,CString ComVal,CString ComName)
void EnumField(CComDispatchDriver spInputElement , CString ComType , CString ComVal , CString ComName);

#endif //__IE_AUTO_H__