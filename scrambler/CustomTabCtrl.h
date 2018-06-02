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


// CCustomTabCtrl

class CCustomTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CCustomTabCtrl)

public:
	CCustomTabCtrl();
	virtual ~CCustomTabCtrl();
	void DrawItem(LPDRAWITEMSTRUCT lpDraw);
	void DrawItem(CDC * dc, LPTCITEM lpItem, int nId);
	void DrawItemBar(CDC * dc);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

private:
	typedef CArray<CWnd *, CWnd * &>	CWndArray;
	CWndArray m_arrForms;

public:
	void SetItemForm(CWnd * pWnd, int nItem);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void SelectItem(int nItem);
public:
	afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNcPaint();
};


