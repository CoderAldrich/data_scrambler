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
#include "D:\dev_programs\vs2008\VC\atlmfc\include\afxcmn.h"
#include "OleRichEditCtrl.h"

class http_log_dlg : public CDialog
{
	DECLARE_DYNAMIC(http_log_dlg)

public:
	http_log_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~http_log_dlg();

// 對話方塊資料
	enum { IDD = IDD_SCRAPY_EDIT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();

public:
    COleRichEditCtrl edit_log;

public:
    LRESULT open_log_file(LPCWSTR file_name); 
    LRESULT append_log(LPCWSTR text); 
    LRESULT save_log_file(); 
    LRESULT insert_text(LPCWSTR text); 
    afx_msg void OnBnClickedButtonCopy();
    afx_msg void OnBnClickedButtonOpen();

protected:
    CString script_file_name; 
public:
    afx_msg void OnBnClickedButtonSave();
};
