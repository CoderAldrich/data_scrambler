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


// locate_url_config_dlg 對話方塊

class locate_url_config_dlg : public CDialog
{
	DECLARE_DYNAMIC(locate_url_config_dlg)

public:
	locate_url_config_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~locate_url_config_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_LOCATE_URL };

public:
	VOID get_locate_url( wstring &url ) 
	{
		url = locate_url.c_str(); 
	}

	VOID set_locate_url( LPCWSTR url )
	{
		locate_url = url; 
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual void OnOK()
	{
		CString _url; 
		GetDlgItemText( IDC_EDIT_LOCATE_URL, _url ); 
		locate_url = _url; 
		CDialog::OnOK(); 
	}

	virtual BOOL OnInitDialog(); 

	DECLARE_MESSAGE_MAP()

protected:
	wstring locate_url; 
};
