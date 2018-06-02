// Browser_ControlDlg.h : header file
//
//{{AFX_INCLUDES()

#import "msxml.tlb"
#include "webbrowser2.h"
//}}AFX_INCLUDES

#if !defined(AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_)
#define AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg dialog

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

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
};

class browser_dlg : public CDialog
{
// Construction
public:
	browser_dlg(CWnd* pParent = NULL);	// standard constructor
	~browser_dlg() 
	{
		if( TRUE == com_inited )
		{
			CoUninitialize(); 
		}
	}

// Dialog Data
	//{{AFX_DATA(CBrowser_ControlDlg)
	enum { IDD = IDD_BROWSER_CONTROL_DIALOG };
	CEdit	m_eURL;
	CAnimateCtrl	m_Animate;
	CWebBrowser2	m_WebBrowser;
	CString	m_sURL;
	BOOLEAN com_inited; 
	//IXMLDOMNode *html_page_group; 
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
	afx_msg void OnBack();
	afx_msg void OnForward();
	afx_msg void OnStop();
	afx_msg void OnRefresh();
	afx_msg void OnHome();
	afx_msg LRESULT OnWorking( WPARAM wparam, 
		LPARAM lparam); 
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

protected:
	CStringW config_file_name; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr html_page_group; 
	MSXML::IXMLDOMElementPtr html_page; 

public:
	void OnMenuBarExplorer1(BOOL MenuBar);
	void NewWindow2Explorer1(LPDISPATCH* ppDisp, BOOL* Cancel);
	void OnFullScreenExplorer1(BOOL FullScreen);
	void BeforeScriptExecuteExplorer1(LPDISPATCH pDispWindow);
	void BeforeNavigate2Explorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	HRESULT show_active_element_xpath(); 

public:
	VOID set_target_url( LPCWSTR url )
	{
		m_sURL = url; 
	}
	afx_msg void OnBnClickedButtonRecordPage();
	afx_msg void OnBnClickedButtonNewPageGroup();
	afx_msg void OnBnClickedButtonScramble();
	void NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BROWSER_CONTROLDLG_H__8689CE65_69B4_4E71_A7BF_966382B8A1DE__INCLUDED_)
