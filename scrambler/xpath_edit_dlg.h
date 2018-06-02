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
#ifndef __XPATH_EDIT_DLG_H__
#define __XPATH_EDIT_DLG_H__

#pragma once

#define WM_XPATH_CONFIG ( WM_USER + 101 )


// xpath_edit_dlg dialog

class xpath_edit_dlg : public CDialog
{
	DECLARE_DYNAMIC(xpath_edit_dlg)

public:
	xpath_edit_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~xpath_edit_dlg();

// Dialog Data
	enum { IDD = IDD_XPATH_EDIT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk(); 

	CString set_xpath1( CString &xpath )
	{
		CString old_xpath; 
		old_xpath = m_xpath1; 

		m_xpath1 = xpath; 
		SetDlgItemTextW( IDC_EDIT_XPATH1, m_xpath1.GetBuffer() ); 
		SetDlgItemTextW( IDC_EDIT_XPATH2, m_xpath2.GetBuffer() ); 
		return old_xpath; 
	}

	CString set_xpath2( CString &xpath )
	{
		CString old_xpath; 
		old_xpath = m_xpath2; 

		m_xpath2 = xpath; 
		SetDlgItemTextW( IDC_EDIT_XPATH1, m_xpath1.GetBuffer() ); 
		SetDlgItemTextW( IDC_EDIT_XPATH2, m_xpath2.GetBuffer() ); 
		return old_xpath; 
	}

	LRESULT find_string_difference( LPCWSTR xpath_src, LPCWSTR xpath_dest ); 

	LRESULT compare_xpath_mismatch( LPCWSTR xpath_src, LPCWSTR xpath_dest ); 

	CString set_edit_xpath( LPCWSTR xpath )
	{
		CString old_xpath; 
		CWnd *dlg_item; 

		do 
		{
			dlg_item = GetDlgItem(IDC_EDIT_XPATH1); 
			ASSERT( dlg_item != NULL ); 
			if( NULL == dlg_item )
			{
				break; 
			}

			dlg_item->GetWindowText( m_xpath1 ); 
			if( 0 == m_xpath1.GetLength() )
				//if( ::GetFocus() == dlg_item->GetSafeHwnd() )
			{
				set_xpath1( CString( xpath ) ); 
			}

			dlg_item = GetDlgItem(IDC_EDIT_XPATH2); 
			ASSERT( dlg_item != NULL ); 
			if( NULL == dlg_item )
			{
				break; 
			}

			//dlg_item->GetWindowText( m_xpath2 ); 
			//if(0 == m_xpath2.GetLength() )
			//if( ::GetFocus() == dlg_item->GetSafeHwnd() )
			{
				set_xpath2( CString( xpath ) ); 
			}
		} while ( FALSE );
	
		return old_xpath; 
	}

	

	CString get_xpath1()
	{
		return m_xpath1; 
	}

	CString get_xpath2()
	{
		return m_xpath1; 
	}

	LRESULT get_xpath_result( wstring &xpath_result )
	{
		if( m_xpath_output.GetLength() == 0 )
		{
			xpath_result = m_xpath1.GetBuffer(); 
		}
		else
		{
			xpath_result = m_xpath_output.GetBuffer(); 
		}

		return ERROR_SUCCESS; 
	}

    VOID set_parent(CWnd *parent)
    {
        parent_wnd = parent; 
    }

protected:
	CString m_xpath1; 
	CString m_xpath2; 
	CString m_xpath_output; 
    CWnd *parent_wnd; 

public:
	afx_msg void OnBnClickedButtonXorXpath();
	afx_msg void OnBnClickedButtonHilight();
};

#endif //__XPATH_EDIT_DLG_H__