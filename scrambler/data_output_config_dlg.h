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
#include "data_analyze.h"
#include "GridListCtrl.h"
#include "InPlaceEdit.h"
#include "afxcmn.h"

class data_output_config_dlg : public CDialog
{
	DECLARE_DYNAMIC(data_output_config_dlg)

public:
	data_output_config_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~data_output_config_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_OUTPUT_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(); 
	virtual BOOL OnInitDialog(); 

	DECLARE_MESSAGE_MAP()

public:

	LRESULT set_html_pages_layout( CONTAINER_INFOS *pages ); 

	LRESULT get_output_config( BOOLEAN *share_script ) 
	{
		LRESULT ret = ERROR_SUCCESS; 
		
		*share_script = share_script_file; 
		return ret; 
	}

	LRESULT add_page_info_to_list( CONTAINER_INFO *page ); 
	LRESULT load_data_set_info( CONTAINER_INFO *container_info ); 

	CString sql_conn_string; 
	BOOLEAN to_data_center; 
	BOOLEAN output_link_list; 
	BOOLEAN share_script_file; 
	CComboBox output_type;
	CONTAINER_INFOS *container_infos; 
	CInPlaceEdit *m_pListEdit; 
	CGridListCtrl data_sets;
	CGridListCtrl data_set;

	afx_msg void OnBnClickedButtonSelectPath();
	afx_msg void OnBnClickedButtonDonate();

	afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult);
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult); 
	
	afx_msg void OnBeginlabeleditDataSetList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDataSetList(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBeginlabeleditDataSetsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDataSetsList(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnNMDblclkListHtmlDataSets(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSqlConnSetting();
	afx_msg void OnBnClickedButtonSqlConnCheck();
	afx_msg void OnNMClickListHtmlDataSets(NMHDR *pNMHDR, LRESULT *pResult);
};
