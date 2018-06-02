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

#include "html_xpath.h"
#include "external_link.h"
#define DATA_COLUME_TEXT_LEN MAX_PATH 
// data_list_dlg dialog

class data_list_dlg : public CDialog
{
	DECLARE_DYNAMIC(data_list_dlg)

public:
	data_list_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~data_list_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DATAS_LIST };

public:
	LRESULT add_scrambled_data_info( HTML_ELEMENT_ACTIONS *contents ); 
	LRESULT add_data_info( ULONG count, ... ); 
	LRESULT insert_data_info( LPCWSTR texts[], ULONG text_count ); 

	LRESULT init_scramble_data_info_headers( HTML_ELEMENT_ACTIONS *contents ); 

	LRESULT set_page_contents( HTML_ELEMENT_ACTIONS *page_content ); 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(); 
	virtual void OnCancel(); 
	virtual BOOL OnInitDialog(); 
	void OnNMDblclkListContentElements(NMHDR *pNMHDR, LRESULT *pResult); 
	void OnNMClickListContentElements(NMHDR *pNMHDR, LRESULT *pResult); 

	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl data_list; 
	LPWSTR *columes; 
	ULONG colume_count; 
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};
