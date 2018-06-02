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

// CustomTabCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "CustomTabCtrl.h"
#include ".\customtabctrl.h"


// CCustomTabCtrl

IMPLEMENT_DYNAMIC(CCustomTabCtrl, CTabCtrl)
CCustomTabCtrl::CCustomTabCtrl()
{
}

CCustomTabCtrl::~CCustomTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CCustomTabCtrl, CTabCtrl)
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnTcnSelchange)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()



// CCustomTabCtrl 消息处理程序
void CCustomTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDraw)
{
	FillRect(lpDraw->hDC, &lpDraw->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

void CCustomTabCtrl::DrawItem(CDC * dc, LPTCITEM lpItem, int nId)
{
	bool selected = (GetCurSel() == nId);
	COLORREF bgColor = GetSysColor(COLOR_3DFACE);
	COLORREF fgColor = (selected || lpItem->dwState & TCIS_HIGHLIGHTED) ? GetSysColor(COLOR_WINDOWTEXT) : RGB(255,255,255);

	CBrush br(bgColor);
	CBrush * brOld = dc->SelectObject(&br);

	CRect rc;
	GetItemRect(nId, &rc);

	CPen pen(PS_SOLID, 1, RGB(0,0,0));
	CPen * penOld = dc->SelectObject(&pen);

	if( selected || lpItem->dwState & TCIS_HIGHLIGHTED)
	{
		rc.top -= 2;
		POINT pt[] = {
			{ rc.left + 3, rc.top },
			{ rc.right - 3, rc.top },
			{ rc.right, rc.top + 3 },
			{ rc.right, rc.bottom },
			{ rc.left, rc.bottom },
			{ rc.left, rc.top + 3 },
			{ rc.left + 3, rc.top }
		};

		dc->Polygon(pt, 6);
		rc.top += 2;
	}

	UINT nDrawFlag = DT_CENTER|DT_SINGLELINE| DT_VCENTER; //(selected ?  : DT_BOTTOM);

	if( selected )
	{
		CPen pen2(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
		dc->SelectObject(&pen2);
		dc->MoveTo(rc.left+1, rc.bottom);
		dc->LineTo(rc.right, rc.bottom);
		dc->SelectObject(&pen);	
	}
	
	dc->SetBkMode(TRANSPARENT);
	dc->SetTextColor(fgColor);
	if( selected ) rc.top -= 2;
	dc->DrawText(lpItem->pszText, &rc, nDrawFlag);

	dc->SelectObject(penOld);
	dc->SelectObject(brOld);
	pen.DeleteObject();	
	br.DeleteObject();
}

void CCustomTabCtrl::DrawItemBar(CDC * dc)
{
	if( !GetItemCount() )
		return;

    CRect rc;
	GetItemRect(0, &rc);

	int nBarHeight = GetRowCount() * rc.Height();
	int nBarTop = 2;

	GetClientRect(&rc);

	rc.top = nBarTop;
	rc.bottom = rc.top + nBarHeight;
	dc->Draw3dRect(rc, RGB(0x3a, 0x6e, 0x9e), RGB(0x4e, 0x7a, 0xa5));

	rc.InflateRect(-1,-1,-1,-1);

	GRADIENT_RECT    gRect[] = {
		{ 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }
	};

	TRIVERTEX vert[] = {
		{ rc.left, rc.top, 0x6000,0x8300,0xa300,0 },
		{ rc.left + rc.Width()/2, rc.top + rc.Height()/2, 0x8600,0xb000,0xd100,0 },
		{ rc.left + rc.Width()/2, rc.top, 0x8600,0xb000,0xd100,0 },
		{ rc.right, rc.top + rc.Height()/2, 0x6000,0x8300,0xa300,0 },

		{ rc.left, rc.top + rc.Height()/2, 0x3100,0x6000,0x8e00,0 },
		{ rc.left + rc.Width()/2, rc.bottom, 0x6500,0x9f00,0xce00,0 },
		{ rc.left + rc.Width()/2, rc.top + rc.Height()/2, 0x6500,0x9f00,0xce00,0 },
		{ rc.right, rc.bottom, 0x3100,0x6000,0x8e00,0 },
	};

	dc->GradientFill(vert, 8, &gRect, 4, GRADIENT_FILL_RECT_H); 

	GetClientRect(&rc);
	rc.top = nBarHeight + nBarTop;

	CPen pen(PS_SOLID, 1, RGB(0x4e, 0x7a, 0xa5));
	CPen * pnOld;
	CBrush br, * brOld;
	br.CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	brOld = dc->SelectObject(&br);
	pnOld = dc->SelectObject(&pen);

	dc->Rectangle(&rc);
	dc->SelectObject(pnOld);
	dc->SelectObject(&brOld);

	br.DeleteObject();
	pen.DeleteObject();
}

void CCustomTabCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
		
	CDC dcMem;
	CBitmap bmMem;
	CBitmap * pOldbm = NULL;
	CFont * pOldFont = NULL;
	CRect rc;
	GetClientRect(&rc);	

	if( dcMem.CreateCompatibleDC(&dc) )
	{
		if( bmMem.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height()) )
		{
			pOldbm = dcMem.SelectObject(&bmMem);
			pOldFont = dcMem.SelectObject(GetFont());
			CBrush br(GetSysColor(COLOR_3DFACE));
			dcMem.FillRect(&rc, &br);
		}
		else
		{
			dcMem.DeleteDC();
			return ;
		}
	}

	DrawItemBar(&dcMem);
	
    
	TCITEM Item;	
	WCHAR szText[256];
	int nCurSel = GetCurSel();

	Item.dwStateMask = TCIS_BUTTONPRESSED | TCIS_HIGHLIGHTED;
	Item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	Item.pszText = &szText[0];
	Item.cchTextMax = sizeof(szText);

	for(int i=0;i<GetItemCount();i++)
	{
		if( !GetItem(i, &Item) )
			continue;

		DrawItem(&dcMem, &Item, i);
	}

	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &dcMem, rc.left, rc.top, SRCCOPY);

	dcMem.SelectObject(pOldbm);
	dcMem.SelectObject(pOldFont);
	dcMem.DeleteDC();
}

void CCustomTabCtrl::SelectItem(int nItem)
{
	// TODO: 在此添加控件通知处理程序代码

	if( nItem < 0 || nItem >= m_arrForms.GetSize() )
		return;

	for(int i=0;i<m_arrForms.GetSize();i++)
	{
		CWnd * pWnd = m_arrForms[i];
		if( pWnd )
			pWnd->ShowWindow( SW_HIDE );
	}



	CWnd * pWnd = m_arrForms[nItem];
	if( !pWnd ) 
		return;

	CRect rc;
	GetItemRect(0, &rc);
	int nHeight = GetRowCount() * rc.Height() + 6;

	GetClientRect(&rc);

	
	pWnd->SetWindowPos(NULL, 4, nHeight, rc.Width()-8, rc.Height()-nHeight - 4, 0);
	pWnd->ShowWindow(SW_SHOW);
	pWnd->EnableWindow(TRUE);

}



void CCustomTabCtrl::SetItemForm(CWnd * pWnd, int nItem)
{
	if( nItem >= GetItemCount() || nItem >= m_arrForms.GetSize())
		return;

	m_arrForms[nItem] = pWnd;
}

LRESULT CCustomTabCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
    LRESULT ret = CTabCtrl::WindowProc(message, wParam, lParam);

	switch(message)
	{
	case TCM_INSERTITEM:
		{
			CWnd * pWnd = NULL;
			m_arrForms.InsertAt((int)wParam, pWnd, 1);
		}
		break;

	case TCM_DELETEITEM:
		m_arrForms.RemoveAt((int)wParam);
		break;

	case TCM_DELETEALLITEMS:
		m_arrForms.SetSize(0);
		break;

	case TCM_SETCURSEL:
		SelectItem((int)wParam);
		break;

	}

	return ret;
}

void CCustomTabCtrl::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	SelectItem(GetCurSel());
}

void CCustomTabCtrl::OnNcPaint()
{
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CTabCtrl::OnNcPaint()
}
