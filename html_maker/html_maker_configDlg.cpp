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

// html_maker_configDlg.cpp : implementation file
//

#include "stdafx.h"
#include "html_maker_app.h"
#include "html_maker_configDlg.h"
//#include <mshtml.h>
//#include <comutil.h>
//#include <comdef.h>
//#include <comdefsp.h>
//#include "browser_dlg.h"
#import "msxml.tlb"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Chtml_maker_configDlg::Chtml_maker_configDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Chtml_maker_configDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Chtml_maker_configDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HTML_PAGE, html_page_list);
	DDX_Control(pDX, IDC_LIST_HTML_ELEMENT, html_element_list);
}

BEGIN_MESSAGE_MAP(Chtml_maker_configDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &Chtml_maker_configDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_ADD_HTML_INFO, &Chtml_maker_configDlg::OnBnClickedButtonAddHtmlInfo)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_HTML_INFO, &Chtml_maker_configDlg::OnBnClickedButtonDeleteHtmlInfo)
	ON_BN_CLICKED(IDC_BUTTON_PICK_UP_HTML_ELEMENTS, &Chtml_maker_configDlg::OnBnClickedButtonPickUpHtmlElements)
END_MESSAGE_MAP()


// Chtml_maker_configDlg message handlers

BOOL Chtml_maker_configDlg::OnInitDialog()
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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	//ListView_SetExtendedListViewStyle( html_page_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	html_page_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP | LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES|LVS_SHOWSELALWAYS);
	html_element_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP | LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES|LVS_SHOWSELALWAYS);

	//Ìí¼ÓÁÐ

	html_page_list.InsertColumn( 1, _T("ÍøÒ³URL"), LVCFMT_LEFT, 50);

	html_element_list.InsertColumn( 0, _T("ÍøÒ³ÔªËØ"), LVCFMT_LEFT, 50);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//void Chtml_maker_configDlg::OnSysCommand(UINT nID, LPARAM lParam)
//{
//	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
//	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
//	}
//	else
//	{
//		CDialog::OnSysCommand(nID, lParam);
//	}
//}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Chtml_maker_configDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Chtml_maker_configDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Chtml_maker_configDlg::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
}

void Chtml_maker_configDlg::OnBnClickedButtonAddHtmlInfo()
{
	// TODO: Add your control notification handler code here
}

LRESULT Chtml_maker_configDlg::get_selected_html_page( INT32 nPos, PHTML_PAGE_PROPERTIES page )
{
	LRESULT ret = ERROR_SUCCESS; 
	CString temp_text;
	
	do 
	{
		ASSERT( page != NULL ); 

		if( nPos < 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		temp_text = html_page_list.GetItemText( nPos, HTML_PAGE_URL_ITEM_INDEX ); 

		page->url = temp_text.GetBuffer(); 

		if( page->url.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE ); 

	return ret;
}

void Chtml_maker_configDlg::OnBnClickedButtonDeleteHtmlInfo()
{
	LRESULT ret; 
	INT32 _ret; 
	POSITION pSelectedPos = NULL; 
	HTML_PAGE_PROPERTIES page; 
	INT32 nSelectedCount; 
	INT32 nSelectedPos = -1; 
	CString input_text; 
	HTML_INFO_LEVEL level; 

	do 
	{
		_ret = GetDlgItemText( IDC_EDIT_URL_HTML_ELEMENT, input_text ); 
		if( FALSE == _ret )
		{
			break; 
		}

		level = get_html_info_level_from_text( input_text.GetBuffer() ); 

		switch( level )
		{
		case HTML_PAGE:
			break; 
		case HTML_ELEMENT:
			do 
			{
				nSelectedCount = html_page_list.GetSelectedCount();

				if( nSelectedCount < 1 )
				{
					break; 
				}

				pSelectedPos = html_page_list.GetFirstSelectedItemPosition();
				if ( pSelectedPos == NULL )
				{
					break;
				}

				nSelectedPos = html_page_list.GetNextSelectedItem( pSelectedPos );
				if ( nSelectedPos < 0 )
				{
					break;
				}

				ret = get_selected_html_page( nSelectedPos, &page ); 

				if( ERROR_SUCCESS != ret )
				{
					break; 
				}
			}while( FALSE ); 


			if( page.url.length() == 0 )
			{
				MessageBox( L"ÇëÑ¡ÔñÄ¿±êÍøÒ³" ); 
				break; 
			}

			//ret = add_html_element( input_text.GetBuffer(), &page ); 

			break; 
		default: 
			break; 
		}
	}while( FALSE );

}

void Chtml_maker_configDlg::OnBnClickedButtonPickUpHtmlElements()
{
	// TODO: Add your control notification handler code here

	//browser_dlg dlg; 

	//dlg.DoModal(); 

	return; 
}
