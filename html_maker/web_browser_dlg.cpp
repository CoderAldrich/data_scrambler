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

// web_browser_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "web_browser_dlg.h"


// web_browser_dlg dialog

IMPLEMENT_DYNAMIC(web_browser_dlg, CDialog)

web_browser_dlg::web_browser_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(web_browser_dlg::IDD, pParent)
{
	HRESULT hr; 

	site = NULL; 

	//hr = CoInitializeEx( NULL, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED ); 

	//if( FAILED( hr ) )
	//{

	//}
	//else
	//{
	//	com_inited = TRUE; 
	//}
}

web_browser_dlg::~web_browser_dlg()
{
	if( TRUE == com_inited )
	{
		CoUninitialize(); 
	}
}

#include "ie_auto.h"

void web_browser_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EXPLORER1, m_WebBrowser);
	DDX_Text(pDX, IDC_URL, target_url );
}

INT_PTR web_browser_dlg::create_dialog()
{
	// can be constructed with a resource template or InitModalIndirect
	ASSERT(m_lpszTemplateName != NULL || m_hDialogTemplate != NULL ||
		m_lpDialogTemplate != NULL);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();
	if (m_lpszTemplateName != NULL)
	{
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = ::FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		hDialogTemplate = LoadResource(hInst, hResource);
	}
	if (hDialogTemplate != NULL)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// return -1 in case of failure to load the dialog template resource
	if (lpDialogTemplate == NULL)
		return -1;

	// disable parent (before creating dialog)

	BOOL bEnableParent = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	CWnd* pMainWnd = NULL;
	BOOL bEnableMainWnd = FALSE;
#endif
	HWND hWndParent = NULL; 

	if (hWndParent && hWndParent != ::GetDesktopWindow() && ::IsWindowEnabled(hWndParent))
	{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
#ifndef _AFX_NO_OLE_SUPPORT
		pMainWnd = AfxGetMainWnd();
		if (pMainWnd && pMainWnd->IsFrameWnd() && pMainWnd->IsWindowEnabled())
		{
			//
			// We are hosted by non-MFC container
			// 
			pMainWnd->EnableWindow(FALSE);
			bEnableMainWnd = TRUE;
		}
#endif
	}

	TRY
	{
		// create modeless dialog
		if (CreateDlgIndirect(lpDialogTemplate,
			CWnd::FromHandle(hWndParent), hInst))
		{
			//if (m_nFlags & WF_CONTINUEMODAL)
			//{
			//	// enter modal loop
			//	DWORD dwFlags = MLF_SHOWONIDLE;
			//	if (GetStyle() & DS_NOIDLEMSG)
			//		dwFlags |= MLF_NOIDLEMSG;
			//	VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			//}

			// hide the window before enabling the parent, etc.
			if (m_hWnd != NULL)
				SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
				SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	CATCH_ALL(e)
	{
		//DELETE_EXCEPTION(e);
		m_nModalResult = -1;
	}
	END_CATCH_ALL

#ifndef _AFX_NO_OLE_SUPPORT
		if (bEnableMainWnd)
			pMainWnd->EnableWindow(TRUE);
#endif
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	//// destroy modal window
	//DestroyWindow();
	//PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	ShowWindow( SW_HIDE ); 

	return m_nModalResult;
}

BOOL web_browser_dlg::OnInitDialog()
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

#define DEFAULT_BROWSER_URL L"http://www.xheike.com/forum.php?mod=post&amp;action=newthread&amp;fid=84" 
	//L"www.baidu.com"
	// TODO: Add extra initialization here
	CWnd *pWnd = GetDlgItem(IDC_URL); 

	if( target_url.GetLength() == 0 )
	{
		target_url = DEFAULT_BROWSER_URL; 
	}

	if( pWnd )
	{	
		pWnd->SetWindowText( target_url.GetBuffer() );
	}

	browser_safe_navigate_start( &m_WebBrowser, 
		target_url.GetBuffer() ); 

	//m_WebBrowser.Navigate( target_url.GetBuffer(), 
	//	NULL, 
	//	NULL, 
	//	NULL, 
	//	NULL );

	//SetTimer( 101, 1000, NULL ); 
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(web_browser_dlg, CDialog)
	//ON_WM_TIMER()
END_MESSAGE_MAP()

// web_browser_dlg message handlers

void web_browser_dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnTimer(nIDEvent);


	do
	{
		LRESULT ret; 
		IDispatchPtr html_doc_disp; 

		HTML_PAGE_INFO *page; 
		HTML_PAGE_INFOS_ITERATOR it; 

		do 
		{
			ASSERT( NULL != site ); 

			for( it = site->pages.begin(); it != site->pages.end(); it ++ )
			{
				do 
				{
					page = ( *it ); 


					m_WebBrowser.Navigate( page->url.c_str(), 
						NULL, 
						NULL, 
						NULL, 
						NULL );

					Sleep( 2000 ); 

					//m_WebBrowser.GetInterface(); 

					html_doc_disp = m_WebBrowser.GetDocument(); 
					if( html_doc_disp == NULL )
					{
						break; 
					}

					//ret = ie_auto_execute_on_page( page, html_doc_disp ); 

					//if( ret != ERROR_SUCCESS )
					//{
					//	//dbg_print()
					//}

				}while( FALSE ); 
			}

		}while( FALSE ); 

		//ret =_ie_auto_execute_on_site( html_doc, site ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE ); 


	KillTimer( nIDEvent ); 
}
