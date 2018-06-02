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


// xpath_compare_dlg 對話方塊

class xpath_compare_dlg : public CDialog
{
	DECLARE_DYNAMIC(xpath_compare_dlg)

public:
	xpath_compare_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~xpath_compare_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_XPATH_COMPARE };

public:

	LRESULT compare_xpath_mismatch( LPCWSTR xpath_src, LPCWSTR xpath_dest ); 
	LRESULT find_string_difference( LPCWSTR xpath_src, LPCWSTR xpath_dest ); 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	afx_msg LRESULT on_check_xpath_mismatch( WPARAM wparam, 
		LPARAM lparam); 
	virtual BOOL OnInitDialog(); 
	DECLARE_MESSAGE_MAP()
};
