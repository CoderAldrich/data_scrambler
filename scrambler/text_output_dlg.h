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


// text_output_dlg 對話方塊

class text_output_dlg : public CDialog
{
	DECLARE_DYNAMIC(text_output_dlg)

public:
	text_output_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~text_output_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_TEXT_OUTPUT };

public:
	LRESULT set_output_text( LPCWSTR text )
	{
		CString _text; 
		GetDlgItemText( IDC_RICHEDIT_OUTPUT, _text ); 
		if( _text.GetLength() > 20000 )
		{
			_text = L""; 
		}

		_text += L"\r\n"; 
		_text += text; 

		SetDlgItemText( IDC_RICHEDIT_OUTPUT, _text ); 
		return ERROR_SUCCESS; 
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
};
