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

// Browser_ControlDlg.cpp : implementation file
//

#include "stdafx.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "html_maker_config.h"
#include "html_element_config.h"
#include "browser_dlg.h"

using namespace MSXML; 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg dialog

browser_dlg::browser_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(browser_dlg::IDD, pParent), 
	com_inited( FALSE )
{
	//{{AFX_DATA_INIT(CBrowser_ControlDlg)
	m_sURL = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void browser_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrowser_ControlDlg)
	DDX_Control(pDX, IDC_URL, m_eURL);
	DDX_Control(pDX, IDC_ANIMATE1, m_Animate);
	DDX_Control(pDX, IDC_EXPLORER1, m_WebBrowser);
	DDX_Text(pDX, IDC_URL, m_sURL);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(browser_dlg, CDialog)
	//{{AFX_MSG_MAP(CBrowser_ControlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BACK, OnBack)
	ON_BN_CLICKED(ID_FORWARD, OnForward)
	ON_BN_CLICKED(ID_STOP, OnStop)
	//ON_BN_CLICKED(ID_REFRESH, OnRefresh)
	ON_BN_CLICKED(ID_HOME, OnHome)
	//ON_BN_CLICKED(ID_SEARCH, OnSearch)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_RECORD_SELECTED, &browser_dlg::OnBnClickedButtonRecordSelected)
	ON_BN_CLICKED(IDC_BUTTON_RECORD_PAGE, &browser_dlg::OnBnClickedButtonRecordPage)
	ON_BN_CLICKED(IDC_BUTTON_NEW_PAGE_GROUP, &browser_dlg::OnBnClickedButtonNewPageGroup)
	ON_BN_CLICKED(IDC_BUTTON_CPATURE_INPUT, &browser_dlg::OnBnClickedButtonCpatureInput)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg message handlers

#ifdef _DEBUG
LRESULT WINAPI unit_test()
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring test_text = L"<input id=search  kw name=q type=\"text\" placeholder=\"你想找什么？\" autocomplete=\"off\" class=\"ac_input\"><BR>"; 


	format_html_text(test_text, test_text ); 

	return ret; 
}
#else
LRESULT WINAPI unit_test()
{
	LRESULT ret = ERROR_SUCCESS; 
	return ret; 
}
#endif //_DEBUG
BOOL browser_dlg::OnInitDialog()
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
	
#define DEFAULT_BROWSER_URL L"http://account.xici.net/register?refer=http%3A%2F%2Fwww.xici.net%2F"
	//L"http://www.xheike.com/forum.php?mod=forumdisplay&fid=50"; //L"http://www.xheike.com/forum.php?mod=post&amp;action=newthread&amp;fid=84"; //L"http://www.newsmth.net/nForum/#!board/NewSoftware"
	// TODO: Add extra initialization here
	CWnd *pWnd = GetDlgItem(IDC_URL); 

	if( m_sURL.GetLength() == 0 )
	{
		m_sURL = DEFAULT_BROWSER_URL; 
	}

	if( pWnd )
	{	
		pWnd->SetWindowText( m_sURL.GetBuffer() );
	}

	unit_test(); 
	m_WebBrowser.Navigate( m_sURL.GetBuffer(), 
		NULL, 
		NULL, 
		NULL, 
		NULL ); 

	m_Animate.Open(L"progress.avi");

	HRESULT hr; 
	hr = CoInitialize( NULL ); 
	
	if( FAILED( hr ) )
	{

	}
	else
	{
		com_inited = TRUE; 
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void browser_dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void browser_dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR browser_dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BEGIN_EVENTSINK_MAP(browser_dlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CBrowser_ControlDlg)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 106 /* DownloadBegin */, OnDownloadBeginExplorer1, VTS_NONE)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 104 /* DownloadComplete */, OnDownloadCompleteExplorer1, VTS_NONE)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 102 /* StatusTextChange */, OnStatusTextChangeExplorer1, VTS_BSTR)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 259 /* DocumentComplete */, OnDocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 256, browser_dlg::OnMenuBarExplorer1, VTS_BOOL)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 251, browser_dlg::NewWindow2Explorer1, VTS_PDISPATCH VTS_PBOOL)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 258, browser_dlg::OnFullScreenExplorer1, VTS_BOOL)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 290, browser_dlg::BeforeScriptExecuteExplorer1, VTS_DISPATCH)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 250, browser_dlg::BeforeNavigate2Explorer1, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(browser_dlg, IDC_EXPLORER1, 271, browser_dlg::NavigateErrorExplorer1, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void browser_dlg::OnOK() 
{
	// TODO: Add extra validation here
	
	UpdateData(true);
	m_WebBrowser.Navigate(m_sURL , NULL, NULL, NULL, NULL);
	//OR you can also do the following
 	// CString str;
 	// m_eURL.GetWindowText(str);
	// m_WebBrowser.Navigate(m_sURL , NULL, NULL, NULL, NULL);
}

void browser_dlg::OnBack() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoBack();
}

void browser_dlg::OnForward() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoForward();
}

void browser_dlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.Stop();
}


void browser_dlg::OnRefresh() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.Refresh();
}

void browser_dlg::OnHome() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoHome(); 
}


/***********************************************************************************
界面设计:
1.导航条 go home
2.back forward
***********************************************************************************/
void browser_dlg::OnSearch() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoSearch();
}

void browser_dlg::OnDownloadBeginExplorer1() 
{
	// TODO: Add your control notification handler code here
	
	m_Animate.Play(0,-1,-1);

}

void browser_dlg::OnDownloadCompleteExplorer1() 
{
	// TODO: Add your control notification handler code here
 	m_Animate.Stop();
	m_Animate.Seek(0);
 
}

void browser_dlg::OnStatusTextChangeExplorer1(LPCTSTR Text) 
{
	// TODO: Add your control notification handler code here
		if (GetSafeHwnd())
	{
		CWnd *pWnd = GetDlgItem(IDC_STATUS_TEXT);
		if (pWnd)
			pWnd->SetWindowText(Text);
	}


}

 
void browser_dlg::OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	// TODO: Add your control notification handler code here
	CString locURL;
	locURL=	m_WebBrowser.GetLocationURL();
	m_eURL.SetWindowText(locURL); //(CString(URL->bstrVal));
	//UpdateData( FALSE ); 
}

/********************************************************************************
<?xml version="1.0" encoding="UTF-8" ?>
<site url="http://tieba.baidu.com" >
<page url="http://tieba.baidu.com/p/3954573871">
<a href="#" id="quick_reply" class="btn-small btn-sub j_quick_reply">
<i class="icon-reply"></i>回复
</a>
</page>
<page url="">
</page>
<page url="">
<div id="ueditor_replace" style="width: 678px; min-height: 220px; z-index: 0;" class=" edui-body-container" contenteditable="true" input_text="auto input text to html div element, date: 2015-08-09" sleep_time="300">
<p>
<br/>
</p>
</div>
<!--<a href="#" class="ui_btn ui_btn_m j_submit poster_submit" title="Ctrl+Enter快捷发表">
<span>
<em>发 表</em>
</span>
</a>-->
</page>
</site>
********************************************************************************/

LRESULT WINAPI get_domain_name_in_url( LPCWSTR url, wstring &domain_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR domain_name_begin; 
	LPCWSTR domain_name_end; 

	do 
	{
		ASSERT( NULL != url ); 
		//ASSERT( NULL != domain_name ); 

#define HTTP_PROTOCAL_URL_PREFIX L"http://" 
#define HTTPS_PROTOCAL_URL_PREFIX L"https://" 
#define HTTP_URL_DELIM L'/'

		if( 0 != _wcsnicmp( url, 
			HTTP_PROTOCAL_URL_PREFIX, 
			CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ) ) )
		{
			if( 0 != _wcsnicmp( url, 
				HTTPS_PROTOCAL_URL_PREFIX, 
				CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ) ) )
			{
				ret = ERROR_PATH_NOT_FOUND; 
				break; 
			}
			else
			{
				domain_name_begin = url + CONST_STR_LEN( HTTPS_PROTOCAL_URL_PREFIX ); 
			}
		}
		else
		{
			domain_name_begin = url + CONST_STR_LEN( HTTP_PROTOCAL_URL_PREFIX ); 
		}

		domain_name_end = wcschr( domain_name_begin, HTTP_URL_DELIM ); 

		if( NULL == domain_name_end )
		{
			domain_name = domain_name_begin; 
		}
		else
		{
			domain_name = domain_name_begin; 
			domain_name = domain_name.substr( 0, domain_name_end - domain_name_begin ); 
		}
	}while( FALSE );

	return ret; 
}

/*********************************************************************************
问题：
对页面点击后，或过一定时间后，或经过某种事件后，需要重新加载的问题的处理：
1.加入元素的属性refresh =true表示需要重新加载 =false表示不需要
2.对不同的页面状态，加入不同的页面。
3.在页加载事件处理页面的RELOAD，自动保持页面最新。
*********************************************************************************/
/*********************************************************************************
<iframe tabindex="2" class="pt" id="e_iframe" frameborder="0" style="height: 400px;" hidefocus="">
<html><head id="editorheader"><meta http-equiv="Content-Type" content="text/html; charset=utf-8"><link href="misc.php?css=1_wysiwyg&amp;YTL" rel="stylesheet" type="text/css"><script type="text/javascript">window.onerror = function() { return true; }</script></head><body style="height: 400px;" contenteditable="true" spellcheck="false"><p>asdfasdfasdfasfd</p></body></html>
</iframe>
**********************************************************************************/

LRESULT WINAPI get_html_active_element( IHTMLDocument2* html_doc, 
									   IHTMLElement **html_element )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	_bstr_t text; 
	LPCWSTR _text; 

	wstring input_text; 
	IHTMLElementPtr _html_element; 

	//VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != html_doc ); 
		ASSERT( NULL != html_element ); 
		
		*html_element = NULL; 

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ) ) 
		{
			ret = hr; 
			break; 
		}

		if( NULL == _html_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		for( ; ; )
		{
			hr = _html_element->get_tagName( text.GetAddress() ); 
			if( FAILED( hr ) )
			{
				ret = hr; 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			if( NULL == _text )
			{
				break; 
			}

			if( 0 == wcsicmp( _text, L"iframe" ) 
				|| 0 == wcsicmp( _text, L"frame" ) )
			{
				//IHTMLIFrameElement2 html_iframe; 
				IHTMLFrameBase2Ptr html_frame; 
				IHTMLWindow2Ptr html_window; 
				IHTMLDocument2Ptr html_doc; 
				IHTMLElementPtr html_sub_element; 
				//IHTMLFrameElement2Ptr html_frame; 

				hr = _html_element->QueryInterface( IID_IHTMLFrameBase2, ( PVOID* )&html_frame ); 
				if( FAILED( hr ) ) 
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_frame )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				hr = html_frame->get_contentWindow( &html_window ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_window )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				hr = html_window->get_document( &html_doc ); 
				if( FAILED( hr ) )
				{
					ret = hr; 
					break; 
				}

				hr = html_doc->get_activeElement( &html_sub_element ); 
				if( FAILED( hr ) ) 
				{
					ret = hr; 
					break; 
				}

				if( NULL == html_sub_element )
				{
					ret = ERROR_NOT_FOUND; 
					break; 
				}

				_html_element->Release(); 

				_html_element = html_sub_element; 

				_html_element->AddRef(); 
			}
			else
			{
				_html_element->AddRef(); 
				*html_element = _html_element; 
				break; 
			}
		}
	}while( FALSE ); 

	return ret; 
}

#include "capture_input_dlg.h"
#include "input_content.h"

/***********************************************************************************
配置文件的生成过程:
1.配置文件内容单位是HTML页面
2.配置文件的默认名称为HTML SITE名称
3.HTML页面与配置文件没有绑定关系
4.所以配置文件路径需要用户进行确认
5.HTML元素配置动作单元是PAGE组
6.所以需要两个标识:
1.HTML元素配置文件名称.
2.自动化动作集合名称
***********************************************************************************/

void browser_dlg::OnBnClickedButtonRecordSelected()
{
	LRESULT ret; 
	HRESULT hr; 
	//ULONG _ret; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr html_elements; 
	IHTMLElementPtr html_element; 
	//IHTMLSelectionObject2Ptr 
	IHTMLSelectionObjectPtr html_selection; 

	_bstr_t text; 
	LPCWSTR _text; 
	CString url; 
	wstring domain_name; 

	//IWebBrowser2Ptr browser; 
	// TODO: Add your control notification handler code here

	do 
	{
		if( NULL == xml_doc )
		{
			OnBnClickedButtonNewPageGroup(); 

			if( NULL == xml_doc )
			{
				break; 
			}
		}

		//url = m_WebBrowser.GetLocationURL(); 

		//ret = get_domain_name_in_url( url.GetBuffer(), domain_name ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	domain_name = L""; 
		//}

		//_ret = GetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, config_file_path ); 
		//if( _ret == 0 )
		//{
		//	WCHAR file_name[ MAX_PATH ]; 
		//	WCHAR app_path[ MAX_PATH ]; 
		//	ULONG cc_ret_len; 

		//	ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
		//	if( ret != ERROR_SUCCESS )
		//	{
		//		break; 
		//	}

		//	//hr = html_doc->get_URL( text.GetAddress() ); 
		//	//if( S_OK != hr )
		//	//{
		//	//	break; 
		//	//}

		//	//_text = text.operator const wchar_t*(); 
		//	//if( _text == NULL )
		//	//{
		//	//	break; 
		//	//}

		//	if( url.GetLength() == 0 )
		//	{
		//		app_path[ cc_ret_len - 1 ] = L'\0'; 
		//	}
		//	else
		//	{
		//		if( domain_name.length() == 0 )
		//		{
		//			app_path[ cc_ret_len - 1 ] = L'\0'; 
		//		}

		//		//hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), domain_name.c_str() ); 
		//		//if( hr != S_OK )
		//		//{
		//		//	break; 
		//		//}
		//	}

		//	*file_name = _T( '\0' ); 

		//	//_ret = SetCurrentDirectoryW( app_path ); 
		//	//if( FALSE == _ret )
		//	//{

		//	//}

		//	ret = open_file_dlg( FALSE, L"xml", file_name, NULL, app_path, L".xml", NULL ); 
		//	if( ret < 0 )
		//	{
		//		break; 
		//	}

		//	if( *file_name != L'\0' )
		//	{
		//		SetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, file_name ); 
		//	}

		//	config_file_path = file_name; 
		//}

		//ret = make_config_file_exist( config_file_path.GetBuffer() ); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		////m_WebBrowser.GetFocus(); 
		disp = m_WebBrowser.GetDocument(); 

		if( NULL == disp )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) 
			|| html_doc == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//do
		//{
		//	_bstr_t html_title; 
		//	_variant_t attr_value; 

		//	hr = html_doc->get_title( html_title.GetAddress() ); 
		//	if( hr != S_OK )
		//	{
		//		break; 
		//	}

		//	if( NULL == html_title.operator const wchar_t*() )
		//	{
		//		break; 
		//	}

		//	ret = find_page_group_config( config_file_path.GetBuffer(), 
		//		domain_name.c_str(), 
		//		&xml_doc, 
		//		&html_page_group ); 

		//	if( ERROR_SUCCESS != ret )
		//	{
		//		break; 
		//	}

		//	ASSERT( NULL != xml_doc ); 

		//	attr_value = html_title.GetBSTR(); 

		//	hr = html_page_group->setAttribute( L"desc", attr_value ); 
		//
		//	if( S_OK != hr )
		//	{
		//		
		//	}
		//}while( FALSE ); 

		//ret = add_page_config( xml_doc, url, html_page_group, &html_page ); 
		//if( ERROR_SUCCESS != ret )
		//{
		//	break; 
		//}

		//{
		//	IHTMLFramesCollection2Ptr frames; 
		//	hr = html_doc->get_frames( &frames ); 
		//
		//	if( FAILED( hr ) )
		//	{
		//		break; 
		//	}
		//}

		hr = html_doc->get_activeElement( &html_element ); 
		if( S_OK != hr 
			|| NULL == html_element )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = add_html_element_config( xml_doc, url.GetBuffer(), html_page, html_element ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_IMPORTANT, "add the html element to auto execution configure file error %u\n", ret ); 
		}

		//{
		//	IHTMLLocationPtr location; 

		//	hr = html_doc->get_location( &location ); 
		//	if( hr != S_OK 
		//		|| NULL == location )
		//	{
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	hr = location->get_hostname( text.GetAddress() ); 
		//	if( hr != S_OK )
		//	{
		//		//ret = ERROR_ERRORS_ENCOUNTERED; 
		//		//break; 
		//	}
		//}

		do 
		{
		hr = html_doc->get_selection( &html_selection ); 
		if( S_OK != hr 
			|| NULL == html_selection )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = html_selection->get_type( text.GetAddress() ); 
		if( FAILED( hr ) 
			|| NULL == text.operator const wchar_t*() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

#define SELECTION_OBJECT_NONE L"None"
#define SELECTION_OBJECT_TEXT L"Text"
#define SELECTION_OBJECT_CONTROL L"Control" //???

		_text = text.operator const wchar_t*(); 

		if( _text == NULL )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		//{
		//	IHTMLTxtRangePtr text_range; 

		//	hr = html_selection->createRange( ( IDispatch** )&text_range ); 
		//	
		//	text_range->get_htmlText( &text );
		//}

		try
		{	
			INT32 i; 
			IHTMLControlRangePtr selected_elements; 
			LONG selected_item_count; 

			if( 0 != wcsicmp( _text, SELECTION_OBJECT_CONTROL ) )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			hr = html_selection->createRange( ( IDispatch** )&selected_elements ); 
			//html_doc->get_location( ); 

			if( S_OK != hr 
				|| NULL == selected_elements )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = selected_elements->get_length( &selected_item_count ); 

			if( hr != S_OK )
			{
				break; 
			}

			for( i = 0; i < selected_item_count; i ++ )
			{
				do 
				{
					hr = selected_elements->item( i, &html_element ); 
					if( S_OK != hr 
						|| NULL == html_element )
					{
						break; 
					}

					ret = add_html_element_config( xml_doc, url, html_page, html_element ); 
					if( ret != ERROR_SUCCESS )
					{
						dbg_print( MSG_IMPORTANT, "add the thml element to auto execution configure file error %u\n", ret ); 
					}
				}while( FALSE );
			}
			//txt_range->get_text( &text  );
		}
		catch( ... ) 
		{
			ret = ERROR_ERRORS_ENCOUNTERED; //GetExceptionCode(); 
			break; 
		}	


		//hr = html_doc->get_all( &html_elements ); 
		//if( FAILED( hr ) )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		}while( FALSE );

		{
			_variant_t save_path; 
			_bstr_t _save_path; 

			_save_path = config_file_name.GetBuffer(); 
			save_path = _save_path; 

			hr = xml_doc->raw_save( save_path ); 
		
			if( hr != S_OK )
			{
				ret = hr; 
				break; 
			}
		}

	}while( FALSE );

	//if( NULL != page_element )
	//{
	//	page_element->Release(); 		
	//}

	//if( NULL != site_element )
	//{
	//	site_element->Release(); 
	//}

	//if( NULL != xml_doc )
	//{
	//	xml_doc->Release(); 
	//}

	return; 
}

void browser_dlg::OnMenuBarExplorer1(BOOL MenuBar)
{
	// TODO: Add your message handler code here
}

void browser_dlg::NewWindow2Explorer1(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}

void browser_dlg::OnFullScreenExplorer1(BOOL FullScreen)
{
	// TODO: Add your message handler code here
}

void browser_dlg::BeforeScriptExecuteExplorer1(LPDISPATCH pDispWindow)
{
	// TODO: Add your message handler code here
}

void browser_dlg::BeforeNavigate2Explorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}

void browser_dlg::OnBnClickedButtonRecordPage()
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	//HRESULT hr; 
	//ULONG _ret; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr html_elements; 
	IHTMLElementPtr html_element; 
	CString url; 

	// TODO: Add your control notification handler code here

	do 
	{
		if( NULL == xml_doc )
		{
			break; 
		}

		url = m_WebBrowser.GetLocationURL(); 

		ret = add_page_config( xml_doc, url, html_page_group, &html_page ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
	}while( FALSE ); 

	return; 
}

void browser_dlg::OnBnClickedButtonNewPageGroup()
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	HRESULT hr; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr html_elements; 
	IHTMLElementPtr html_element; 

	_bstr_t text; 
	//LPCWSTR _text; 
	//CString config_file_path; 
	CString url; 
	wstring domain_name; 

	//IWebBrowser2Ptr browser; 
	// TODO: Add your control notification handler code here

	do 
	{
		url = m_WebBrowser.GetLocationURL(); 

		ret = get_file_name_from_url( url.GetBuffer(), config_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = get_domain_name_in_url( url.GetBuffer(), domain_name ); 
		if( ret != ERROR_SUCCESS )
		{
#ifdef _DEBUG
			domain_name = L"TEST"; 
#else
			domain_name = L""; 
#endif //_DEBUG	
		}


		//_ret = GetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, config_file_path ); 
		//if( _ret == 0 )
		{
			WCHAR file_name[ MAX_PATH ]; 
			WCHAR app_path[ MAX_PATH ]; 
			ULONG cc_ret_len; 
			SYSTEMTIME _time; 

#define CONFIG_FILE_DIRECTORY L"html_page_config\\"

			ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//hr = html_doc->get_URL( text.GetAddress() ); 
			//if( S_OK != hr )
			//{
			//	break; 
			//}

			//_text = text.operator const wchar_t*(); 
			//if( _text == NULL )
			//{
			//	break; 
			//}

			hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
			if( S_OK != hr )
			{
				break; 
			}

			//if( url.GetLength() == 0 )
			//{
			//	app_path[ cc_ret_len - 1 ] = L'\0'; 
			//}
			//else
			//{
			//	if( domain_name.length() == 0 )
			//	{
			//		app_path[ cc_ret_len - 1 ] = L'\0'; 
			//	}

			//	//hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), domain_name.c_str() ); 
			//	//if( hr != S_OK )
			//	//{
			//	//	break; 
			//	//}
			//}

			//*file_name = _T( '\0' ); 

			//_ret = SetCurrentDirectoryW( app_path ); 
			//if( FALSE == _ret )
			//{

			//}

			//ret = open_file_dlg( FALSE, L"xml", file_name, NULL, app_path, L".xml", NULL ); 
			//if( ret < 0 )
			//{
			//	break; 
			//}

			//if( *file_name != L'\0' )
			//{
			//	SetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, file_name ); 
			//}

			ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			GetLocalTime( &_time ); 

#ifdef _FILE_NAME_BY_TIME
			hr = StringCchPrintfW( file_name, 
				ARRAYSIZE( file_name ), 
				L"%s%s_%04u%02u%02u%02u%02u%02u", 
				app_path, 
				domain_name.c_str(), 
				_time.wYear, 
				_time.wMonth, 
				_time.wDay, 
				_time.wHour, 
				_time.wMinute, 
				_time.wSecond ); 
#else
			hr = StringCchPrintfW( file_name, 
				ARRAYSIZE( file_name ), 
				L"%s%s", 
				app_path, 
				config_file_name.GetBuffer() ); 
#endif //_FILE_NAME_BY_TIME

			if( hr != S_OK )
			{

			}

			config_file_name = file_name; 
		}

		ret = make_config_file_exist( config_file_name.GetBuffer(), 
			config_file_name.GetLength() ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}

		//m_WebBrowser.GetFocus(); 
		disp = m_WebBrowser.GetDocument(); 

		if( NULL == disp )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) 
			|| html_doc == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		do
		{
			_bstr_t html_title; 
			_variant_t attr_value; 

			hr = html_doc->get_title( html_title.GetAddress() ); 
			if( hr != S_OK )
			{
				break; 
			}

			if( NULL == html_title.operator const wchar_t*() )
			{
				break; 
			}

			ret = find_page_group_config( config_file_name.GetBuffer(), 
				domain_name.c_str(), 
				&xml_doc, 
				&html_page_group ); 

			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			ASSERT( NULL != xml_doc ); 
			ASSERT( NULL != html_page_group ); 

			attr_value = html_title.GetBSTR(); 

			hr = html_page_group->setAttribute( L"desc", attr_value ); 

			if( S_OK != hr )
			{

			}

			ret = add_page_config( xml_doc, url, html_page_group, &html_page ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

		}while( FALSE ); 

	}while( FALSE ); 

	return; 
}

#include "capture_input_dlg.h"
void browser_dlg::OnBnClickedButtonCpatureInput()
{
	LRESULT ret; 
	HRESULT hr; 
	capture_input_dlg dlg; 
	wstring text; 
	_bstr_t _text; 
	wstring text_name; 
	IHTMLElementPtr html_element; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		BOOLEAN is_input; 
		disp = m_WebBrowser.GetDocument(); 

		if( NULL == disp )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) 
			|| html_doc == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = get_html_active_element( html_doc, &html_element ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = get_html_element_input_value( html_element, text, is_input ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		hr = html_element->get_id( _text.GetAddress() ); 
		if( FAILED( hr ) )
		{
			//break; 
		}
		else
		{
			LPCWSTR __text; 
			__text = _text.operator const wchar_t*(); 
			if( NULL != __text )
			{
				text_name = __text; 
				dlg.set_text_name( text_name.c_str() ); 
			}
		}

		dlg.set_text_block( text.c_str() ); 

		dlg.DoModal(); 
	}while( FALSE );

	// TODO: Add your control notification handler code here
}

void browser_dlg::NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}
