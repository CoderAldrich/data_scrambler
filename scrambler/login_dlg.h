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
#include "local_config.h"
#include "resource.h"
#include "hyper_link.h"

class login_dlg : public CDialog
{
	DECLARE_DYNAMIC(login_dlg)

public:
	login_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~login_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_USER_LOGIN };

public:
	LRESULT get_user_info( LOCAL_USER_INFO *user_info )
	{
		user_info->user_name = user_name.GetBuffer(); 
		user_info->password = password.GetBuffer(); 
		return ERROR_SUCCESS; 
	}

protected:
	virtual BOOL OnInitDialog(); 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual void OnOK(); 
	virtual void OnCancel(); 

	DECLARE_MESSAGE_MAP()

protected:
	CString user_name; 
	CString password; 
	CHyperLink register_link; 
	CHyperLink show_usage_link; 

public:
	afx_msg void OnStnClickedStaticUserRegister();
	afx_msg void OnStnClickedStaticUsageIntro();
};
