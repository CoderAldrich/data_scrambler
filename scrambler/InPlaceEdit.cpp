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

#include "stdafx.h"
#include "InPlaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit

CInPlaceEdit::CInPlaceEdit(int iItem, int iSubItem, CString sInitText)
:m_sInitText( sInitText )
	{
	m_iItem = iItem;
	m_iSubItem = iSubItem;
	m_bESC = FALSE;
	}

CInPlaceEdit::~CInPlaceEdit()
	{
	}


BEGIN_MESSAGE_MAP(CInPlaceEdit, CEdit)
        //{{AFX_MSG_MAP(CInPlaceEdit)
        ON_WM_KILLFOCUS()
        ON_WM_CHAR()
        ON_WM_CREATE()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit message handlers

BOOL CInPlaceEdit::PreTranslateMessage(MSG* pMsg)
	{
	if( pMsg->message == WM_KEYDOWN )
        {
		SHORT sKey = GetKeyState( VK_CONTROL);
		if(pMsg->wParam == VK_RETURN
			|| pMsg->wParam == VK_DELETE
			|| pMsg->wParam == VK_ESCAPE
			|| sKey
			)
			{
			::TranslateMessage(pMsg);
			/* Strange but true:
			If the edit control has ES_MULTILINE and ESC
			is pressed the parent is destroyed if the 
			message is dispatched.  In this 
			case the parent is the list control. */
			if( !(GetStyle() & ES_MULTILINE) || pMsg->wParam != VK_ESCAPE )
				{
				::DispatchMessage(pMsg);
				}
			return TRUE;                    // DO NOT process further
			}
        }
	
	return CEdit::PreTranslateMessage(pMsg);
	}


void CInPlaceEdit::OnKillFocus(CWnd* pNewWnd)
	{
	CEdit::OnKillFocus(pNewWnd);
	
	CString str;
	GetWindowText(str);
	
	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo;
	dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	dispinfo.hdr.idFrom = GetDlgCtrlID();
	dispinfo.hdr.code = LVN_ENDLABELEDIT;
	
	dispinfo.item.mask = LVIF_TEXT;
	dispinfo.item.iItem = m_iItem;
	dispinfo.item.iSubItem = m_iSubItem;
	dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
	dispinfo.item.cchTextMax = m_bESC ? 0 : str.GetLength();
	
	GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), 
		(LPARAM)&dispinfo );
	
	}


void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
	if( nChar == VK_ESCAPE || nChar == VK_RETURN)
        {
		if( nChar == VK_ESCAPE )
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
        }
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
	
	// Resize edit control if needed
	CalculateSize();
	}

int CInPlaceEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set the proper font
	CFont* font = GetParent()->GetFont();
	SetFont(font);
	
	SetWindowText( m_sInitText );
	SetFocus();
	CalculateSize();
	SetSel( 0, -1 );
	return 0;
	}

void CInPlaceEdit::CalculateSize()
	{
	// Get text extent
	CString str;
	
	GetWindowText( str );
	CWindowDC dc(this);
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject( pFont );
	CSize size;

	// Get client rect
	CRect rect, parentrect;
	GetClientRect( &rect );
	GetParent()->GetClientRect( &parentrect );
	
	// Transform rect to parent coordinates
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );
	
	if( !(GetStyle() & ES_MULTILINE ) )
		{
		size = dc.GetTextExtent( str );
		dc.SelectObject( pFontDC );
		size.cx += 5;                           // add some extra buffer
		}
	else
		{
		CRect thinrect( rect );  // To measure the skinniest text box
		CRect widerect( rect );  // To measure the wides text box
		widerect.right = parentrect.right;
		// Use the shortest of the two box sizes.
		int thinheight = dc.DrawText( str, &thinrect, DT_CALCRECT|DT_NOPREFIX|DT_LEFT|DT_EXPANDTABS|DT_WORDBREAK );
		int wideheight = dc.DrawText( str, &widerect, DT_CALCRECT|DT_NOPREFIX|DT_LEFT|DT_EXPANDTABS|DT_WORDBREAK );
		if( thinheight >= wideheight )
			{
			size.cy = wideheight + 5;
			size.cx = widerect.right - widerect.left + 5;
			}
		else
			{
			size.cy = thinheight + 5;
			size.cx = thinrect.right - thinrect.left + 5;
			}
		}
	
	// Check whether control needs to be resized
	// and whether there is space to grow
	int changed = 0;
	if( size.cx > rect.Width() )
		{
		if( size.cx + rect.left < parentrect.right-2 )
			rect.right = rect.left + size.cx;
		else
			rect.right = parentrect.right-2;
		changed = 1;
		}
	if( size.cy > rect.Height() )
		{
		if( size.cy + rect.top < parentrect.bottom-2 )
			rect.bottom = rect.top + size.cy;
		else
			{
			rect.bottom = parentrect.bottom-2;
			ShowScrollBar( SB_VERT );
			}
		changed = 1;
		}
	// If the size became larger rposition the window.
	if( changed )
		MoveWindow( &rect );
	}

