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

#include "stdafx.h"
#include "common_html_dlg.h"
#include "data_scrambler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define TIMER_ID	101
#define WM_APP_SHOWWINDOW	WM_APP + 100
extern Cdata_scramblerApp theApp;

// CDHTMLOverviewDlg dialog

BEGIN_MESSAGE_MAP(common_html_dlg, CDHtmlDialog)
	
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(WM_APP_SHOWWINDOW, &common_html_dlg::OnAppMsgShowWindow)
//	ON_REGISTERED_MESSAGE(WM_UPDATE_PLUGIN_INFORMATION, &CDHTMLOverviewDlg::OnUpdatePlugInInformation)

	//ON_COMMAND(ID_APP_ABOUT, &CDHTMLOverviewDlg::OnAbout)
	//ON_COMMAND(ID_CHANGE_PASSSWORD, &CDHTMLOverviewDlg::OnChangePassword)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(common_html_dlg)
END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(common_html_dlg, CDHtmlDialog)
//	DISP_FUNCTION(CDHTMLOverviewDlg, "onok", OnOKDispatch, VT_EMPTY,  VTS_BSTR VTS_BSTR)
//	DISP_FUNCTION(CDHTMLOverviewDlg, "onset", OnSetDispatch, VT_EMPTY,  VTS_BSTR VTS_BSTR )
//	DISP_FUNCTION(CDHTMLOverviewDlg, "runPlugIn", RunPlugInDispatch, VT_EMPTY, VTS_BSTR VTS_BSTR)
//	DISP_FUNCTION(CDHTMLOverviewDlg, "changePassword", ChangePassowrdDispatch, VT_EMPTY, VTS_NONE)
//	DISP_FUNCTION(CDHTMLOverviewDlg, "about", AboutDispatch, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


common_html_dlg::common_html_dlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(common_html_dlg::IDD, common_html_dlg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);



	EnableAutomation();
}

void common_html_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	//DDX_DHtml_ElementValue(pDX,_T("LINK"),m_strLnkFile);
	//DDX_DHtml_ElementValue(pDX,_T("FILENAME"),m_strFileName);
	//DDX_DHtml_ElementInnerText(pDX,_T("p4"),m_p4);
}

// CDHTMLOverviewDlg message handlers

BOOL common_html_dlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetHostFlags(DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_NO3DBORDER);
	
	m_pBrowserApp->put_RegisterAsDropTarget(VARIANT_FALSE);
	
#ifndef _DEBUG
	m_pBrowserApp->put_Silent(VARIANT_TRUE);
#endif

	SetExternalDispatch(GetIDispatch(TRUE));
	
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(m_strXmlFileName,&wfd);
	if(hFind!= INVALID_HANDLE_VALUE)
	{
		m_ftModified = wfd.ftLastWriteTime;
		FindClose(hFind);
		TCHAR tszURL[1024]={0};
		wsprintf(tszURL,_T("file:///%s"),m_strXmlFileName);
		Navigate(tszURL,NULL,NULL,NULL,NULL);
	}
	
	
	SetTimer(TIMER_ID, 1000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void common_html_dlg::OnPaint()
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
		CDHtmlDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR common_html_dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void common_html_dlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	//if (CString(szUrl).MakeLower().Find(_utility.m_strAppName.MakeLower()) > 0)	
	CComPtr<IHTMLDocument2> spdoc;

	
}

void common_html_dlg::OnSize(UINT nType, int cx, int cy)
{
	CDHtmlDialog::OnSize(nType, cx, cy);
}

void common_html_dlg::OnClose()
{
	CDHtmlDialog::OnClose();
}

void common_html_dlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 800;
	lpMMI->ptMinTrackSize.y = 560;

	CDHtmlDialog::OnGetMinMaxInfo(lpMMI);
}



LRESULT common_html_dlg::OnAppMsgShowWindow(WPARAM wParam, LPARAM lParam)
{
	SetWindowPos(NULL, 0, 0, 880, 600, SWP_NOMOVE |  SWP_SHOWWINDOW);
	CenterWindow();

	return 0;
}
HRESULT common_html_dlg::ShowContextMenu(
	DWORD dwID,
	POINT * ppt,
	IUnknown * pcmdtReserved,
	IDispatch * pdispReserved 
	)
{
	return S_OK;
}


void common_html_dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_ID)
	{
		TCHAR tszURL[256];
	
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(m_strXmlFileName,&wfd);
		if(hFind!= INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
			if(memcmp(&m_ftModified,&wfd.ftLastWriteTime,sizeof(FILETIME))!=0)
			{
				m_ftModified = wfd.ftLastWriteTime;
				wsprintf(tszURL,_T("file:///%s"),m_strXmlFileName);
				
				Navigate(tszURL,NULL,NULL,NULL,NULL);
			}
			
			
		}

		return ;
	}

	return CDHtmlDialog::OnTimer(nIDEvent);
}

