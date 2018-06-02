#pragma once
#include "D:\dev_programs\vs2008\VC\atlmfc\include\afxcmn.h"
#include "OleRichEditCtrl.h"


// scrapy_dlg 對話方塊

class scrapy_dlg : public CDialog
{
	DECLARE_DYNAMIC(scrapy_dlg)

public:
	scrapy_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~scrapy_dlg();

// 對話方塊資料
	enum { IDD = IDD_SCRAPY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();

public:
    COleRichEditCtrl edit_script;

public:
    LRESULT open_script_file(LPCWSTR file_name); 
    LRESULT set_target_html_element_info(PVOID html_element, LPCWSTR xpath, LPCWSTR std_xpath); 
    LRESULT save_script_file(); 
    afx_msg void OnBnClickedButtonCopy();
    afx_msg void OnBnClickedButtonOpen();
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
    CString script_file_name; 
public:
    afx_msg void OnBnClickedButtonSave();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
