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


#pragma once
#include "customtabctrl.h"
#include "afxcmn.h"
#include "html_element_prop_dlg.h"
#include "scrapy_dlg.h"
#include "http_log_dlg.h"
#include "doc_host_ui_handler.h"

// html_page_process_dlg 對話方塊

class html_page_process_dlg : public CDialog, public log_manager
{
	DECLARE_DYNAMIC(html_page_process_dlg)

public:
	html_page_process_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~html_page_process_dlg();

// 對話方塊資料
	enum { IDD = IDD_HTML_PAGE_PROCESS_DLG };

public:

    LRESULT on_html_document_complete(LPCWSTR url); 
    LRESULT set_target_html_element_info( PVOID html_element, LPCWSTR xpath, LPCWSTR std_xpath );
    LRESULT append_log(ULONG type, LPCWSTR text); 

protected:
    html_element_prop_dlg html_command_dlg;
    CCustomTabCtrl mode_tab;
	scrapy_dlg python_scrapy_dlg; 
    http_log_dlg log_dlg; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
};
