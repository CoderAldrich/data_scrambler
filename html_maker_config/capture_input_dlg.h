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


// capture_input_dlg dialog

class capture_input_dlg : public CDialog
{
	DECLARE_DYNAMIC(capture_input_dlg)

public:
	capture_input_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~capture_input_dlg();

// Dialog Data
	enum { IDD = IDD_CAPTURE_INPUT_DLG };

protected:

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	VOID set_text_block( LPCWSTR text )
	{
		ASSERT( text != NULL ); 
		text_block = text; 
	}

	VOID set_text_name( LPCWSTR name )
	{
		ASSERT( name != NULL ); 
		text_name = name; 
	}

	wstring get_text_name()
	{
		return text_name; 
	}

	wstring get_text_block()
	{
		return text_block; 
	}

	LRESULT get_selected_text_block(); 
	LRESULT load_input_contents(); 
	LRESULT init_html_config_ui(); 
	void on_delete_text_block(); 
	LRESULT delete_text_block( wstring &text_name_out, wstring &text_content_out ); 

protected:
	wstring text_block; 
	wstring text_name; 

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBnAddText();
	CListCtrl text_block_list;
	afx_msg void OnNMRClickListTextBlocks(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBnDeleteText();
	afx_msg void OnNMDblclkListTextBlocks(NMHDR *pNMHDR, LRESULT *pResult);
};
