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


class welcome_dlg : public CDialog
{
	DECLARE_DYNAMIC(welcome_dlg)

public:
	welcome_dlg(CWnd* pParent = NULL);
	virtual ~welcome_dlg();

	enum { IDD = IDD_DIALOG_WELCOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

protected:
	CFont font; 
public:
	afx_msg void OnBnClickedButtonHelp();
};

INLINE LRESULT WINAPI show_welcome_info()
{
	welcome_dlg dlg; 
	dlg.DoModal(); 
	return ERROR_SUCCESS; 
}
