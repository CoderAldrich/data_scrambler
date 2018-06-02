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
#include "afxcmn.h"


// error_process_dlg 对话框

class error_process_dlg : public CDialog
{
	DECLARE_DYNAMIC(error_process_dlg)

public:
	error_process_dlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~error_process_dlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ERROR_PROCESS };
#endif

public:
    LRESULT set_error_message(LPCWSTR text); 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

protected:
    wstring error_message; 
public:
    CRichEditCtrl edit_error_process;
    afx_msg void OnBnClickedYes();
    afx_msg void OnBnClickedNo();
};
