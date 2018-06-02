/*
 * 
 * Copyright (c) 2003, Michael Carruth All rights reserved.
 * This file is a part of CrashRpt library.
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

#if !defined(AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_)
#define AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_

#import "msxml.tlb"
#include "webbrowser2.h"
#include "xpath_edit_dlg.h"
#include "data_list_dlg.h"
#include "data_analyze.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonHelp();
};

class browser_dlg : public CDialog
{
// Construction
public:
	browser_dlg(CWnd* pParent = NULL);	// standard constructor
	~browser_dlg(); 

// Dialog Data
	//{{AFX_DATA(CBrowser_ControlDlg)
	enum { IDD = IDD_BROWSER_CONTROL_DIALOG };
	CEdit	m_eURL;
	CString	m_sURL;
	BOOLEAN com_inited; 
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrowser_ControlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBrowser_ControlDlg)
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
	afx_msg LRESULT OnDataScramble( WPARAM wparam, 
		LPARAM lparam); 
	afx_msg void OnHome();

	afx_msg void OnSearch();

	//DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRecordSelected();

protected:
	CStringW config_file_name; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr html_page_group; 
	MSXML::IXMLDOMElementPtr html_page; 

public:
	HRESULT show_active_element_xpath(); 
	LRESULT end_data_scramble(); 

public:
	VOID set_target_url( LPCWSTR url )
	{
		m_sURL = url; 
	}
	afx_msg void OnBnClickedButtonRecordPage();
	afx_msg void OnBnClickedButtonNewPageGroup();
	afx_msg void OnBnClickedButtonScramble();
	void NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel);

protected:
	xpath_edit_dlg xpath_dlg; 
	data_list_dlg data_dlg; 

	DATA_STORE_PARAM data_store_param; 
public:
	afx_msg void OnBnClickedBnOutputConfig();
	afx_msg void OnBnClickedBnScrambleConfig();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_)
