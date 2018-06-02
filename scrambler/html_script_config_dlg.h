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

#import "msxml.tlb"
#include "webbrowser2.h"

#if !defined(__HTML_SCRIPT_CONFIG_DLG_H__)
#define __HTML_SCRIPT_CONFIG_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "xpath_edit_dlg.h"
#include "html_ui_customize.h"
#include "html_page_process_dlg.h"
#include "scramble_list_dlg.h"
#include "doc_host_ui_handler.h"
#include "html_element_behavior.h"
#include "behavior_handler.h"

#define WM_LOCATED_TO_URL ( WM_USER + 1016 ) 

LRESULT WINAPI config_ie_control_version(); 

class html_script_config_dlg : public CDialog
{
public:
	html_script_config_dlg(CWnd* pParent = NULL);	// standard constructor
	~html_script_config_dlg() 
	{
		if( TRUE == com_inited )
		{
			CoUninitialize(); 
		}

		SAFE_RELEASE_IF( ui_handler ); 
	}

	enum { IDD = IDD_BROWSER_TEST_DIALOG };
	CAnimateCtrl	m_Animate;
	CWebBrowser2	m_WebBrowser;
	BOOLEAN com_inited; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	VOID OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnForward();
	afx_msg void OnStop();
	afx_msg void OnRefresh();
	afx_msg void OnHome();
	afx_msg LRESULT OnWorking( WPARAM wparam, 
		LPARAM lparam); 
	LRESULT on_locate_to_url( WPARAM wparam, LPARAM lparam ); 
	afx_msg void OnDownloadBeginExplorer1();
	afx_msg void OnDownloadCompleteExplorer1();
	afx_msg void OnStatusTextChangeExplorer1(LPCTSTR Text);
	afx_msg void OnSearch();
	afx_msg void OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL);
	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRecordSelected(); 
	afx_msg void OnBack();

    LRESULT locate_to_url(LPCWSTR url); 

protected:
	CStringW config_file_name; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr html_page_group; 
	MSXML::IXMLDOMElementPtr html_page; 

public:
	void OnWebBrowserMenuBar(BOOL MenuBar);
	void WebBrowserNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel);
	void WebBrowserNewWindow3( LPDISPATCH* ppDisp, 
		BOOL *Cancel, 
		VARIANT* Flags, 
		VARIANT *UrlContext , 
		VARIANT *Url ); 

	void OnWebBrowserFullScreen(BOOL FullScreen);
	void WebBrowserBeforeScriptExecute(LPDISPATCH pDispWindow);
	void WebBrowserBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	HRESULT show_active_element_xpath(); 
	LRESULT unhighlight_html_element(); 

public:
    LRESULT get_selected_html_element(); 
    LRESULT select_html_elements(HTML_ELEMENT_GROUP *elements); 

	//VOID set_target_url( LPCWSTR url )
	//{
	//	ASSERT( NULL != url ); 
	//	SetDlgItemText(IDC_TEST_URL, url ); 
	//}

	HRESULT get_html_element_from_xpath( LPCWSTR xpath, 
		HTML_ELEMENT_GROUP *group ); 

	LRESULT check_point_in_ie_control( PPOINT pt ); 

	afx_msg void OnBnClickedButtonRecordPage();
	afx_msg void OnBnClickedButtonNewPageGroup();
	afx_msg void OnBnClickedButtonScramble();

	void NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel);
	LRESULT highlight_html_element_by_xpath( LPCWSTR xpath ); 
	HRESULT highlight_html_element_show( LPCWSTR xpath ); 
	LRESULT test_highlight_html_elements(); 
    LRESULT exec_html_element_action(HTML_ELEMENT_ACTION *action);

#define HTML_PAGE_BUTTON_CLICK_ACTION_CINTINUE 0x00000001

    LRESULT on_html_page_clicked(ULONG flags, CPoint point); //, ULONG *action);

protected:
#ifdef CONFIG_DLG_HAVE_XPATH_DLG
	xpath_edit_dlg xpath_dlg; 
#endif //CONFIG_DLG_HAVE_XPATH_DLG

	html_page_process_dlg html_process_dlg; 

public: 
	doc_host_ui_handler *ui_handler; 

	html_element_behavior *element_behavior; 

    BOOLEAN locate_url_manually; 

public:
	afx_msg void OnBnClickedButtonRunScript();
	afx_msg void OnTimer(UINT_PTR nIDEvent); 
};

#endif // !defined(__HTML_SCRIPT_CONFIG_DLG_H__)
