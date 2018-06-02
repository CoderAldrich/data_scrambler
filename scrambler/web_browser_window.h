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
#pragma once

#include "webbrowser2.h"
#include "xpath_edit_dlg.h"
#include "doc_host_ui_handler.h"
#include "html_element_behavior.h"
#include "html_ui_customize.h"
// web_browser_dlg dialog

LRESULT WINAPI response_output_data(HTML_ELEMENT_ACTION *action,
    vector<wstring> *response); 

class web_browser_window : public CDialog
{
	DECLARE_DYNAMIC(web_browser_window)

public:
	web_browser_window(CWnd* pParent = NULL);   // standard constructor
	virtual ~web_browser_window();

// Dialog Data
	enum { IDD = IDW_WEBBROWSER_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBack();
	afx_msg void OnForward();
	afx_msg void OnStop();
	afx_msg void OnRefresh();
	afx_msg void OnHome();
	//afx_msg LRESULT on_web_page_actions( WPARAM wparam, 
	//	LPARAM lparam); 

	LRESULT exec_html_element_action( HTML_ELEMENT_ACTION *action ); 

	afx_msg LRESULT on_html_action( WPARAM wparam, LPARAM lparam) ; 
	afx_msg void OnWebBrowserDownloadBegin();
	afx_msg void OnWebBrowserDownloadComplete();
	afx_msg void OnWebBrowserStatusTextChange(LPCTSTR Text);
	afx_msg void OnSearch();
	afx_msg void OnWebBrowserDocumentComplete(LPDISPATCH pDisp, VARIANT FAR* URL);

	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRecordSelected();

protected:
	CStringW config_file_name; 

public:
	void OnWebBrowserMenuBar(BOOL MenuBar);
	void WebBrowserNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel);
	void OnWebBrowserFullScreen(BOOL FullScreen);
	void WebBrowserBeforeScriptExecute(LPDISPATCH pDispWindow);
	void WebBrowserBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	HRESULT show_active_element_xpath(); 
	LRESULT on_web_page_action_notify( WPARAM wparam, 
		LPARAM lparam); 

	LRESULT unhighlight_html_element(); 
	LRESULT on_html_element_hilight( WPARAM wparam, LPARAM lparam); 
	LRESULT highlight_html_element_by_xpath( LPCWSTR param, LPCWSTR xpath ); 
	ULONG get_output_delay_time() { return output_delay_time; }
	VOID set_output_delay_time( ULONG delay_time ) { output_delay_time = delay_time; }
	ULONG get_page_load_time() { return web_page_load_time; }
	VOID set_page_load_time( ULONG time ) { web_page_load_time = time; }

public:
	void WebBrowserNavigateError(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point); 
	afx_msg void OnDestroy(); 

public:
	VOID set_target_url( LPCWSTR url )
	{
		target_url = url; 
	}

	IDispatch *get_html_document()
	{
		return m_WebBrowser.GetDocument(); 
	}

#define WAIT_DEBUGGER_ATTACH 0x00000001

	LRESULT set_work_mode( LPCWSTR work_mode ); 

	ULONG get_manager_pid()
	{
		return manager_pid; 
	}

	VOID set_manager_pid( LPCWSTR pid)
	{
		WCHAR *end_text; 
		manager_pid = wcstoul( pid, &end_text, 10 ); 
	}

	INT_PTR create_dialog(); 

protected:

	ULONG manager_pid; 
	CString target_url; 
	BOOLEAN com_inited; 
	ULONG output_delay_time; 
	ULONG web_page_load_time; 
	doc_host_ui_handler *ui_handler; 
	html_element_behavior *element_behavior; 

public:
	CWebBrowser2	m_WebBrowser;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
