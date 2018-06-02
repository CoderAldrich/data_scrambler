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

#ifndef __DATA_HTML_DLG_H__
#define __DATA_HTML_DLG_H__

#define DATA_COLUME_TEXT_LEN MAX_PATH 
#include "dynamic_html_dlg.h"
#include "html_xpath.h"
#include "external_link.h"

class data_html_dlg : public dynamic_html_dlg
{
	DECLARE_DYNCREATE(data_html_dlg)

public:
	data_html_dlg(CWnd* pParent = NULL);           // 动态创建所使用的受保护的构造函数
	virtual ~data_html_dlg();

	// Dialog Data
	enum { IDD = IDD_DIALOG_DATA_LIST, IDH = IDR_HTML_OVERVIEW_DIALOG }; 

public:
     virtual BOOL OnInitDialog();
	 virtual void OnCancel();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const; 
#endif

	LRESULT add_scrambled_data_info( HTML_PAGE_CONTENT *contents ); 
	LRESULT add_data_info( ULONG count, ... ); 
	LRESULT insert_data_info( LPCWSTR texts[], ULONG text_count ); 
	//LRESULT init_data_info_headers( LPCWSTR headers[], 
	//	ULONG header_count ); 

	LRESULT init_scramble_data_info_headers( HTML_PAGE_CONTENT *contents ); 

	LRESULT set_page_contents( HTML_PAGE_CONTENT *page_content ); 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual HRESULT OnGetExternal(
        LPDISPATCH *lppDispatch 
        );
    virtual void OnDocumentComplete(
        LPCTSTR lpszURL 
        );
	DECLARE_MESSAGE_MAP()
    DECLARE_DISPATCH_MAP()

protected:
	vector<wstring> datas; 
	LPWSTR *columes; 
	ULONG colume_count; 
	ULONG cur_row_index; 
};

#endif //__DATA_HTML_DLG_H__
