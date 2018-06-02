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
// web_browser_dlg dialog

class web_browser_dlg : public CDialog
{
	DECLARE_DYNAMIC(web_browser_dlg)

public:
	web_browser_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~web_browser_dlg();

// Dialog Data
	enum { IDD = IDW_WEBBROWSER_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog(); 

	DECLARE_MESSAGE_MAP()

public:
	VOID set_target_url( LPCWSTR url )
	{
		target_url = url; 
	}

	IDispatch *get_html_document()
	{
		return m_WebBrowser.GetDocument(); 
	}


	VOID set_auto_html_info( HTML_WEB_SITE_INFO *site )
	{
		this->site = site; 
	}

	INT_PTR create_dialog(); 
protected:
	CWebBrowser2	m_WebBrowser;
	CString target_url; 
	BOOLEAN com_inited; 
	HTML_WEB_SITE_INFO *site; 

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
