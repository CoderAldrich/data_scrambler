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


// plugin_manage_dlg 對話方塊

class plugin_manage_dlg : public CDialog
{
	DECLARE_DYNAMIC(plugin_manage_dlg)

public:
	plugin_manage_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~plugin_manage_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_PLUGIN_MANAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual BOOL OnInitDialog(); 
	virtual void OnOK(); 
    virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDelPlugin();
	afx_msg void OnBnClickedButtonAddPlugin();
	afx_msg void OnBnClickedButtonBrowser();
	CListCtrl module_infos_list;
};
