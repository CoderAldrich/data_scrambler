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

#ifndef __IN_PLACE_EDIT_H__
#define __IN_PLACE_EDIT_H__

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit window

class CInPlaceEdit : public CEdit
{
// Construction
public:
        CInPlaceEdit(int iItem, int iSubItem, CString sInitText);

// Attributes
public:

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CInPlaceEdit)
        public:
        virtual BOOL PreTranslateMessage(MSG* pMsg);
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CInPlaceEdit();
		void CalculateSize();

        // Generated message map functions
protected:
        //{{AFX_MSG(CInPlaceEdit)
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
private:
        int m_iItem;
        int m_iSubItem;
        CString m_sInitText;
        BOOL    m_bESC;         // To indicate whether ESC key was pressed
};

/////////////////////////////////////////////////////////////////////////////

#endif //__IN_PLACE_EDIT_H__