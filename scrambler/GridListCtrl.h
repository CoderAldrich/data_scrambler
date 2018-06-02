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

#if !defined(AFX_GRIDLISTCTRL_H__92CA5999_9434_11D1_88D5_444553540000__INCLUDED_)
#define AFX_GRIDLISTCTRL_H__92CA5999_9434_11D1_88D5_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl window

class CGridListCtrl : public CListCtrl
{
// Construction
public:
	CGridListCtrl();

// Attributes
public:
	// The current subitem or column number which is order based.
	int m_CurSubItem;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void MakeColumnVisible(int nCol);
	BOOL PositionControl( CWnd * pWnd, int iItem, int iSubItem );
	BOOL PrepareControl(WORD wParseStyle);
	virtual ~CGridListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridListCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	WORD m_wStyle;
	DECLARE_MESSAGE_MAP()
private:
	int IndexToOrder( int iIndex );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDLISTCTRL_H__92CA5999_9434_11D1_88D5_444553540000__INCLUDED_)
