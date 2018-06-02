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
#include "resource.h"
#include "html_document.h"
#include <afxdhtml.h>


class common_html_dlg : public CDHtmlDialog
{
// Construction
public:
	common_html_dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DIALOG_DATA_LIST, IDH = IDR_HTML_OVERVIEW_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();


	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual BOOL IsExternalDispatchSafe() { return TRUE; }

	//////////////////////////////////////////////////////////////////////////
	// event
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	
	afx_msg LRESULT OnAppMsgShowWindow(WPARAM wParam, LPARAM lParam);

	

	STDMETHOD(ShowContextMenu)(
		DWORD dwID,
		POINT * ppt,
		IUnknown * pcmdtReserved,
		IDispatch * pdispReserved 
		);
	//////////////////////////////////////////////////////////////////////////
	// dispatch
	afx_msg void OnOKDispatch( LPCTSTR lpcsLink,LPCTSTR lpcsFileName);
	afx_msg void OnSetDispatch( LPCTSTR lpcsLink,LPCTSTR lpcsFileName);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()

public:
	CString m_strXmlFileName;
	FILETIME m_ftModified;
private:
	HICON m_hIcon;
	html_document m_webPage;
	
};
