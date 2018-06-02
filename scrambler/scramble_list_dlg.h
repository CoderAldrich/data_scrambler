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
#include "GridListCtrl.h"
#include "InPlaceEdit.h"

// scramble_list_dlg dialog

typedef enum _LIST_SCRAMBLE_CONFIG_STEP
{
	STEP_LIST_LINKS, 
	STEP_NEXT_LINK, 
	STEP_CONTENTS, 
	INVALID_LIST_SCRAMBLE_STEP, 
} LIST_SCRAMBLE_CONFIG_STEP, *PLIST_SCRAMBLE_CONFIG_STEP; 

class scramble_list_dlg : public CDialog
{
	DECLARE_DYNAMIC(scramble_list_dlg)

public:
	scramble_list_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~scramble_list_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LIST_SCRAMBLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonCheckListLinks();
	afx_msg void OnBnClickedButtonCheckNextLink();
	afx_msg void OnBnClickedButtonCheckContents();
	afx_msg void OnBnClickedButtonListLinkOk();
	afx_msg void OnBnClickedButtonNextLinkOk(); 

protected:
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();

public:
	LRESULT on_html_element_active( PVOID element, 
		LPCWSTR xpath ); 
	afx_msg void OnBnClickedButtonRecord();
	LRESULT save_scrambled_list(); 

protected:
	LRESULT change_scramble_state( LIST_SCRAMBLE_CONFIG_STEP step ); 
	void OnTimer(UINT_PTR nIDEvent); 

protected:
	ULONG scramble_config_step; 
	CGridListCtrl contants_elements;
	CInPlaceEdit *m_pListEdit; 

public:
	afx_msg void OnNMDblclkListContentElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListContentElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonContentsReturn();
	afx_msg void OnBnClickedButtonNextLinkReturn(); 
	afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
};
