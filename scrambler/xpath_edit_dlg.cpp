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

// xpath_edit_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "web_browser_window.h"
#include "mshtml_addition.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "xpath_edit_dlg.h"

// xpath_edit_dlg dialog

IMPLEMENT_DYNAMIC(xpath_edit_dlg, CDialog)

xpath_edit_dlg::xpath_edit_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(xpath_edit_dlg::IDD, pParent)
{
    parent_wnd = pParent; 
}

xpath_edit_dlg::~xpath_edit_dlg()
{
}

void xpath_edit_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(xpath_edit_dlg, CDialog)
	ON_BN_CLICKED(IDOK, &xpath_edit_dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_XOR_XPATH, &xpath_edit_dlg::OnBnClickedButtonXorXpath)
	ON_BN_CLICKED(IDC_BUTTON_HILIGHT, &xpath_edit_dlg::OnBnClickedButtonHilight)
END_MESSAGE_MAP()


// xpath_edit_dlg message handlers

void xpath_edit_dlg::OnBnClickedOk()
{
	UINT _ret; 
	CString xpath; 

    do
    {
        _ret = GetDlgItemTextW(IDC_EDIT_XPATH1, xpath);
        if (FALSE == _ret)
        {
            //break; 
        }

        m_xpath1 = xpath.GetBuffer();

        _ret = GetDlgItemTextW(IDC_EDIT_XPATH_OUTPUT, xpath);
        if (FALSE == _ret)
        {
            //break; 
        }

        m_xpath_output = xpath.GetBuffer();

        if (parent_wnd != NULL)
        {
            parent_wnd->SendMessage(WM_XPATH_CONFIG, 0, 0);
        }
	} while ( FALSE );

	ShowWindow( SW_HIDE ); 
	//OnOK();
}

BOOL xpath_edit_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	::SetDlgItemTextW( GetSafeHwnd(), IDC_EDIT_XPATH1, m_xpath1.GetBuffer() ); 
	::SetDlgItemTextW( GetSafeHwnd(), IDC_EDIT_XPATH2, m_xpath2.GetBuffer() ); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT WINAPI set_edit_text_color(CRichEditCtrl* richedit, INT32 min, INT32 max, ULONG color )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	CHARRANGE range; 
	CHARFORMAT char_format; 
	
	do 
	{
		memset( &char_format, 0, sizeof( char_format ) ); 

		richedit->GetDefaultCharFormat( char_format ); 

		char_format.cbSize = sizeof( CHARFORMAT ); 
		char_format.dwMask = CFM_COLOR;
		char_format.dwEffects &= ~CFE_AUTOCOLOR;
		char_format.crTextColor = color;  
		
		range.cpMin = min; 
		range.cpMax = max; 

		richedit->SetSel( range ); 

		_ret = richedit->SetSelectionCharFormat( char_format ); 
		if( _ret != TRUE )
		{
			ret = ERROR_SUCCESS; 
			break; 
		}

		//richedit->Invalidate(); 

		dbg_print_w( MSG_IMPORTANT, L"selected text from %u to %u color:%x\n", range.cpMin, range.cpMax, color ); 
		
#ifdef _DEBUG
		{
			//_bstr_t text; 
			//richedit->GetSel(range); 

			//richedit->GetSelTextBSTR( text.GetAddress() ); 
				//text.GetBufferSetLength( range.cpMax - range.cpMin + 1 ) ); 

			//dbg_print_w( MSG_IMPORTANT, L"selected text fomr %u to %u\n", range.cpMin, range.cpMax ); 
		}
#endif //_DEBUG

	}while( FALSE ); 

	return ret;
}

#define TEXT_COMPARE_COLOR_DELETE RGB(240, 0, 0)
LRESULT xpath_edit_dlg::find_string_difference( LPCWSTR xpath_src, LPCWSTR xpath_dest ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 basecount;
	INT32 mergecount;
	INT32 startindex=0;
	CString m_basestr; 
	CString m_mergestr; 
	CRichEditCtrl *edit; 

	do 
	{
		m_basestr = xpath_src; 
		m_mergestr = xpath_dest; 

		basecount=m_basestr.GetLength();
		mergecount=m_mergestr.GetLength();

		if(m_basestr.GetLength() < m_mergestr.GetLength())//当某一行base的字符个数小于merge的字符个数时，多出的部分直接描红
		{
			edit = ( CRichEditCtrl* )GetDlgItem( IDC_EDIT_XPATH2 ); 
			ASSERT( edit != NULL ); 
			set_edit_text_color( edit,startindex+m_basestr.GetLength(),startindex+m_mergestr.GetLength(), TEXT_COMPARE_COLOR_DELETE );

			for(INT32 m=0;m<m_basestr.GetLength();m++)//再比较base和merge相同长度部分的字符
			{
				if(m_basestr[m]!=m_mergestr[m])//相同位置的字符不同时，描红
				{
					set_edit_text_color(edit,startindex+m,startindex+m+1, TEXT_COMPARE_COLOR_DELETE);
				}
			}
		}
		else//当某一行base的字符个数不小于merge的字符个数时
		{
			edit = ( CRichEditCtrl* )GetDlgItem( IDC_EDIT_XPATH1 ); 
			ASSERT( edit != NULL ); 

			for(INT32 m=0;m<m_mergestr.GetLength();m++)//再比较base和merge相同长度部分的字符
			{
				if(m_basestr[m]!=m_mergestr[m])//相同位置的字符不同时，描红
				{
					set_edit_text_color(edit,startindex+m,startindex+m+1, TEXT_COMPARE_COLOR_DELETE);
				}
			}

			startindex+=m_mergestr.GetLength();//累计比较过的merge的字符个数

			set_edit_text_color(edit,startindex,mergecount, TEXT_COMPARE_COLOR_DELETE);//将多于的merge行数的字符全部描红
		}
	} while ( FALSE ); 
	return ret; 
}

LRESULT xpath_edit_dlg::compare_xpath_mismatch( LPCWSTR xpath_src, LPCWSTR xpath_dest )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( xpath_src != NULL ); 
		ASSERT( xpath_dest != NULL ); 

		::SetDlgItemTextW( GetSafeHwnd(), IDC_EDIT_XPATH1, xpath_src ); 
		::SetDlgItemTextW( GetSafeHwnd(), IDC_EDIT_XPATH2, xpath_dest ); 

		ret = find_string_difference( xpath_src, xpath_dest ); 
	}while( FALSE ); 

	return ret; 
}

void xpath_edit_dlg::OnBnClickedButtonXorXpath()
{
	LRESULT ret; 
	// TODO: 在此加入控制項告知處理常式程式碼
	wstring xpath1; 
	wstring xpath2; 

	xpath1 = m_xpath1.GetBuffer(); 
	xpath2 = m_xpath2.GetBuffer(); 

	ret = xor_html_element_xpath( xpath1, xpath2 ); 
	if( ERROR_SUCCESS != ret )
	{
	}

	m_xpath_output = xpath2.c_str(); 

	SetDlgItemTextW( IDC_EDIT_XPATH_OUTPUT, m_xpath_output ); 
}

#include "html_element_prop_dlg.h"
void xpath_edit_dlg::OnBnClickedButtonHilight()
{
	LRESULT ret; 
	// TODO: 在此加入控制項告知處理常式程式碼
	CWnd *parent; 
	LPCWSTR xpath; 

	do 
	{
		parent = GetParent(); 

		ASSERT( NULL != parent ); 

		if( m_xpath2.GetLength() > 0 )
		{
			xpath = m_xpath2.GetBuffer(); 
		}
		else if( m_xpath1.GetLength() > 0 )
		{
			xpath = m_xpath1.GetBuffer(); 
		}

		ret = parent->SendMessage( WM_HTML_ELEMENTS_HILIGHT, 0, ( LPARAM )xpath ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		//GetDlgItem( IDC_BUTTON_HILIGHT )->EnableWindow( FALSE ); 
	}while( FALSE );

}
