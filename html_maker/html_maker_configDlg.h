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

// html_maker_configDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#define HTML_PAGE_URL_ITEM_INDEX 0
typedef struct _HTML_PAGE_PROPERTIES
{
	wstring url; 
} HTML_PAGE_PROPERTIES, *PHTML_PAGE_PROPERTIES; 

// Chtml_maker_configDlg dialog
class Chtml_maker_configDlg : public CDialog
{
// Construction
public:
	Chtml_maker_configDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HTML_MAKER_CONFIG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonAddHtmlInfo();
	afx_msg void OnBnClickedButtonDeleteHtmlInfo(); 

public:
	LRESULT get_selected_html_page( INT32 nPos, PHTML_PAGE_PROPERTIES page ); 

protected:
	CListCtrl html_page_list;
	CListCtrl html_element_list;
public:
	afx_msg void OnBnClickedButtonPickUpHtmlElements();
};
