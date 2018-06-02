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

#ifndef __HTML_VIEW_H__
#define __HTML_VIEW_H__

#include <afxhtml.h>

class CHtmlCtrl : public CHtmlView {
public:
	CHtmlCtrl() { }
	~CHtmlCtrl() { }

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);

	// Normally, CHtmlView destroys itself in PostNcDestroy,
	// but we don't want to do that for a control since a control
	// is usually implemented as a stack object in a dialog.
	//
	virtual void PostNcDestroy() {  }

	// overrides to bypass MFC doc/view frame dependencies
	afx_msg void OnDestroy();
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT msg);

	// override to trap "app:" pseudo protocol
	virtual void OnBeforeNavigate2( LPCTSTR lpszURL,
		DWORD nFlags,
		LPCTSTR lpszTargetFrameName,
		CByteArray& baPostedData,
		LPCTSTR lpszHeaders,
		BOOL* pbCancel );

	// override to handle links to "app:mumble...". lpszWhere will be "mumble"
	virtual void OnAppCmd(LPCTSTR lpszWhere);

	DECLARE_MESSAGE_MAP();
	DECLARE_DYNAMIC(CHtmlCtrl)
};

#endif //__HTML_VIEW_H__