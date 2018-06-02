// Browser_ControlDlg.cpp : implementation file
//

#include "stdafx.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "browser_dlg.h"
#include "html_element_prop_dlg.h"
#include "external_link.h"
#include "html_actions_config.h"
#include "data_analyze.h"

using namespace MSXML; 

DATA_STORE data_store; 

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
HTML_LIST_ELEMENT_SCRAMBLE_INFO scramble_info; 

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

#define WM_WORKING ( WM_USER + 1001 )

browser_dlg *g_browser_dlg; 
ULONG CALLBACK work_thread( PVOID param )
{
	LRESULT ret = 0; 

	do 
	{
		browser_safe_navigate_end( &g_browser_dlg->m_WebBrowser ); 

		ret = g_browser_dlg->SendMessageW( WM_WORKING, 0, ( LPARAM )param ); 
		ExitThread( ( ULONG )ret ); 
	}while( FALSE );

	return ( ULONG )ret; 
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
	ON_MESSAGE( WM_WORKING, OnWorking )
	//ON_BN_CLICKED(ID_SEARCH, OnSearch)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_RECORD_SELECTED, &browser_dlg::OnBnClickedButtonRecordSelected)
	ON_BN_CLICKED(IDC_BUTTON_RECORD_PAGE, &browser_dlg::OnBnClickedButtonRecordPage)
	ON_BN_CLICKED(IDC_BUTTON_NEW_PAGE_GROUP, &browser_dlg::OnBnClickedButtonNewPageGroup)
	ON_BN_CLICKED(IDC_BUTTON_CPATURE_INPUT, &browser_dlg::OnBnClickedButtonScramble)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg message handlers

#ifdef _DEBUG
LRESULT WINAPI unit_test()
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring test_text = L"<input id=search  kw name=q type=\"text\" placeholder=\"你想找什么？\" autocomplete=\"off\" class=\"ac_input\"><BR>"; 


	format_html_text(test_text, test_text, NULL ); 

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

	g_browser_dlg = this; 
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

	//unit_test(); 
	m_WebBrowser.Navigate( m_sURL.GetBuffer(), 
		NULL, 
		NULL, 
		NULL, 
		NULL ); 

	/**************************************************************************
	必须有消息的循环才可以打开WEB窗体，解决方法：
	1.找出本质原因，是否有绕开的方法。
	2.工作使用独立线程，需要执行功能时，调用界面线程来完成。
	3.界面线程中使用定时器。
	**************************************************************************/

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
	
	init_scramble_info( &scramble_info);
	m_WebBrowser.SetSilent( TRUE ); 

	{
		DATA_STORE_PARAM param; 
		wcscpy( param.file_name, L"e:\\test_data.txt" ); 
		create_data_store( &param, &data_store ); 
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
	
	//m_WebBrowser.Navigate(m_sURL , NULL, NULL, NULL, NULL);
	browser_safe_navigate_start( &m_WebBrowser, m_sURL.GetBuffer() ); 

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

LRESULT WINAPI format_output_text( wstring &text )
{
	LRESULT ret = ERROR_SUCCESS; 

	while (TRUE)
	{
		size_t pos = text.find(L"\r\n");
		if (pos != std::wstring::npos )
		{
			text.replace(pos, 2, L"|", 0);
			//text.insert(pos, strDest);
		}
		else
		{
			break;
		}
	}

	return ret; 
}

LRESULT browser_dlg::OnWorking( WPARAM wparam, 
							LPARAM lparam) 
{
	// TODO: Add your control notification handler code here
	
	switch( lparam )
	{
	case 0:
		{
			HRESULT hr; 
			wstring url; 
			scramble_info.links.clear(); 
			scramble_info.next_link.clear(); 

			html_scramble( &m_WebBrowser, &scramble_info ); 

			//for( ; ; )
			{
				if( scramble_info.links.size() == 0)
				{
					break; 
				}

				url = *scramble_info.links.begin(); 
				scramble_info.links.erase( scramble_info.links.begin() ); 

				hr = scramble_web_page_content_ex( &m_WebBrowser,  
					url.c_str() ); 


				if( FAILED(hr ) )
				{
					dbg_print( MSG_ERROR, "scramble html page error %u\n", hr ); 
				}
				else
				{
					break; 
				}
			}
		}
		break; 
	case 1: 
		{
			LRESULT ret; 
			HRESULT hr; 
			wstring text; 
			wstring url; 

			hr = scramble_web_page_content( &m_WebBrowser, 
				&scramble_info.page_content, &text ); 
		
			format_output_text(text); 

			ret = input_one_data_block( &data_store, text.c_str() ); 
			if( ret != ERROR_SUCCESS )
			{

			}

			SetDlgItemTextW( IDC_STATIC_SCRAMBLE_DUMP, text.c_str() ); 

			if( scramble_info.links.size() == 0 )
			{
				if( scramble_info.next_link.size() > 0)
				{
					url = *scramble_info.next_link.begin(); 
					scramble_info.next_link.erase( scramble_info.next_link.begin() ); 

					hr = html_scramble_ex( &m_WebBrowser, url.c_str() ); 
					if( FAILED(hr))
					{
						//break; 
					}
				}
				break; 
			}
			
			//for( ; ; )
			{
				if( scramble_info.links.size() == 0)
				{
					break; 
				}

				url = *scramble_info.links.begin(); 
				scramble_info.links.erase( scramble_info.links.begin() ); 

				hr = scramble_web_page_content_ex( &m_WebBrowser,  
					url.c_str() ); 

				if( FAILED(hr ) )
				{
					dbg_print( MSG_ERROR, "scramble html page error %u\n", hr ); 
				}
				else
				{
					break; 
				}
			}
		}
		break; 
	default:
		break; 
	}

	return ERROR_SUCCESS; 
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

	dbg_print( MSG_IMPORTANT, "the html document loaded\n" ); 

	if( URL->vt == VT_BSTR )
	{
		_bstr_t text; 
		LPCWSTR _text; 

		do 
		{
			wstring url_loading; 

			text = URL->bstrVal; 
			if( text.length() <= 0)
			{
				m_eURL.SetWindowText( L"" ); 

				break; 
			}

			_text = text.operator const wchar_t*(); 
			ASSERT( NULL != _text ); 

			m_eURL.SetWindowText( _text ); 

			url_loading = m_WebBrowser.get_loading_url(); 

			if( url_loading.length() == 0 )
			{
				break; 
			}

			if( 0 == wcscmp( url_loading.c_str(), 
				_text ) )
			{
				m_WebBrowser.notify_status_event( TRUE ); 

				browser_safe_navigate_start( &m_WebBrowser, url_loading.c_str() ); 
			}

		}while( FALSE ); 

	}

	
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

LRESULT WINAPI add_page_config( MSXML::IXMLDOMDocument *xml_doc, 
							   LPCWSTR url, 
							   MSXML::IXMLDOMElement *site_node, 
							   MSXML::IXMLDOMElement **page_node )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	MSXML::IXMLDOMElementPtr xml_node = NULL; 

	_bstr_t temp_text; 
	//LPCWSTR _temp_text; 

	do
	{
		ASSERT( NULL != url ); 
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != site_node );
		ASSERT( NULL != page_node ); 

		//hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
		//	NULL, 
		//	CLSCTX_INPROC_SERVER, 
		//	__uuidof( MSXML::IXMLDOMDocument ), 
		//	( void** )&xml_doc ); 

		//if( hr != S_OK)
		//{
		//	wsprintfW (Msg, L"构造KEY XML:初始化XML文档对象失败, %08x", hr);
		//	ret = hr; 
		//	break; 
		//}

		//__ret = xml_doc->load( ( WCHAR* )file_name ); 

		//if( __ret != VARIANT_TRUE )
		//{
		//	MSXML::IXMLDOMParseErrorPtr spParseError;
		//	_bstr_t bstrReason;

		//	spParseError = xml_doc->parseError;

		//	bstrReason = spParseError->reason;

		//	_temp_text = bstrReason.operator wchar_t*(); 

		//	if( NULL != _temp_text )
		//	{
		//		dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
		//	}

		//	break; 		
		//}

		*page_node = NULL; 

		xml_node = xml_doc->createElement (L"page" ); 

		if( NULL == xml_node )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		site_node->raw_appendChild( xml_node, NULL );

		temp_text = url; 

		xml_node->setAttribute (L"url", temp_text);

		xml_node->AddRef(); 
		*page_node = xml_node; 
	}while( FALSE ); 

	// 释放xml文档
	//if (xml_doc != NULL)
	//{
	//	xml_doc->Release ();
	//}

	//CoUninitialize ();

	return ret; 
}

LRESULT WINAPI create_config_file( LPCWSTR config_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMProcessingInstructionPtr xml_instruction; 
	MSXML::IXMLDOMElementPtr xml_element; 
	_bstr_t text; 

	do
	{
#define HTML_CONFIG_ROOT_ELEMENT L"HTML_SITES"
		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 
		if( hr != S_OK)
		{
			log_trace_ex( MSG_IMPORTANT, "create xml document error %08x", hr);
			ret = hr; 
			break; 
		}

		// 创建Xml声明
		hr = xml_doc->raw_createProcessingInstruction( L"xml", L"version=\"1.0\" encoding=\"utf-8\"", &xml_instruction );
		
		if( S_OK != hr )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML声明失败" );
			ret = hr; 
			break; 
		}

		hr = xml_doc->raw_appendChild( xml_instruction, NULL );
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		text = HTML_CONFIG_ROOT_ELEMENT; 
		xml_element = xml_doc->createElement( text ); 
	
		if( NULL == xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = xml_doc->appendChild( xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			//break; 
		}

		{

			_variant_t save_file_name; 

			text = config_file_name; 
			save_file_name = text; 			
			hr = xml_doc->save( save_file_name ); 
		
			if( hr != S_OK )
			{
				ret = hr; 
			}
		}

	}while( FALSE ); 

	//// 释放xml声明
	//if (xml_instruction != NULL)
	//{
	//	xml_instruction.Release();
	//}

	//// 释放xml文档
	//if (xml_doc != NULL)
	//{
	//	xml_doc.Release ();
	//}

	//CoUninitialize ();

	return ret; 
}

LRESULT WINAPI add_auto_input_content( MSXML::IXMLDOMDocument *xml_doc, 
									  MSXML::IXMLDOMElement *xml_element, 
									  LPCWSTR name, 
									  LPCWSTR text )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	_bstr_t _text; 
	_bstr_t attr_name; 
	MSXML::IXMLDOMElementPtr _xml_element;
	//MSXML::IXMLDOMElementPtr _xml_element;

	do 
	{
		ASSERT( NULL != xml_doc ); 
		ASSERT( NULL != xml_element ); 
		ASSERT( NULL != name ); 
		ASSERT( NULL != text ); 

		_xml_element = xml_doc->createElement( INPUT_CONTENT_ELEMENT_TAG ); 
		if( NULL == _xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_text = text; 

		hr = _xml_element->put_text( _text.GetBSTR() ); 

		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		attr_name = INPUT_CONTENT_ELEMENT_NAME_ATTRIBUTE; 

		_text = name; 
		hr = _xml_element->setAttribute( attr_name.GetBSTR(), _text.GetBSTR() ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}

		hr = xml_element->appendChild( _xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI insert_node_by_text(MSXML::IXMLDOMDocument *xml_src_doc, MSXML::IXMLDOMNode *parent_node, MSXML::IXMLDOMElement *dest_node)
{
	LRESULT ret = ERROR_SUCCESS;
	HRESULT hr; 
	_bstr_t temp_text;
	//LPCWSTR _temp_text; 
	MSXML::IXMLDOMElementPtr xml_element; 
	MSXML::IXMLDOMNodeListPtr child_nodes; 
	MSXML::IXMLDOMNamedNodeMapPtr node_attrs; 
	MSXML::IXMLDOMNodePtr attr; 
	//LONG node_count; 
	LONG atrr_count; 
	LONG i; 

	do
	{
		ASSERT(NULL != xml_src_doc);
		ASSERT(NULL != parent_node);
		ASSERT(NULL != dest_node);

		hr = dest_node->get_nodeName( temp_text.GetAddress() );
		if (S_OK != hr)
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		hr = xml_src_doc->raw_createElement( temp_text.GetBSTR(), &xml_element ); 
		if( S_OK != hr )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//hr = dest_node->get_childNodes( &child_nodes ); 
		//if( S_OK != hr )
		//{
		//	break; 
		//}

		//hr = child_nodes->get_length( &node_count ); 
		//if( S_OK != hr )
		//{
		//	break; 
		//}

		//for( i = 0; i < node_count; i ++ )
		//{
		//	hr = child_nodes->get_item( i, &child_node ); 
		//	if( S_OK != hr )
		//	{
		//		break; 
		//	}

		//	child_node 
		//}

		hr = dest_node->get_attributes( &node_attrs ); 
		if( S_OK != hr )
		{
			//ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = node_attrs->get_length( &atrr_count ); 
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		if( atrr_count == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_variant_t attr_value; 
		for( i = 0; i < atrr_count; i ++ )
		{
			do 
			{
				hr = node_attrs->get_item( i, &attr ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = attr->get_nodeName( temp_text.GetAddress() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = attr->get_nodeValue( attr_value.GetAddress() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}

				hr = xml_element->setAttribute( temp_text.GetBSTR(), attr_value.GetVARIANT() ); 
				if( S_OK != hr )
				{
					ret = hr; 
					break; 
				}
			}while( FALSE );

		}

		hr = parent_node->appendChild( xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			break; 
		}
	} while (FALSE);

	return ret;
}

LRESULT WINAPI inert_xml_text(LPCWSTR src_xml, LPCWSTR dest_xml, LPCWSTR node_xpath, LPWSTR xml_out, ULONG cc_buf_len)
{
	LRESULT ret = ERROR_SUCCESS;
	HRESULT hr;
	MSXML::IXMLDOMDocumentPtr xml_src_doc;
	MSXML::IXMLDOMDocumentPtr xml_dest_doc;

	MSXML::IXMLDOMNodeListPtr node_list;
	MSXML::IXMLDOMNodePtr node;
	MSXML::IXMLDOMNodePtr parent_node;
	MSXML::IXMLDOMElementPtr dest_node;

	MSXML::IXMLDOMElementPtr root_element;

	_bstr_t dest_text;
	_bstr_t temp_text;
	LPCWSTR _temp_text;
	LONG node_count;

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT(NULL != src_xml);
		ASSERT(NULL != dest_xml);
		ASSERT(NULL != node_xpath);

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_src_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_dest_doc ); 

		if( hr != S_OK)
		{
			dbg_print( MSG_IMPORTANT, "构造KEY XML:初始化XML文档对象失败, %08x", hr);
			ret = hr; 
			break; 
		}

		__ret = xml_dest_doc->loadXML( ( WCHAR* )dest_xml ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_dest_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_dest_doc->get_documentElement( &dest_node ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == dest_node )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		//xml_doc->Release(); 

		__ret = xml_src_doc->loadXML( ( WCHAR* )dest_xml ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_src_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		hr = xml_src_doc->get_documentElement(&root_element);
		if (hr != S_OK)
		{
			ret = ERROR_ERRORS_ENCOUNTERED;
			break;
		}

		do
		{
			node_list = root_element->selectNodes( node_xpath );
			if( NULL != node_list )
			{
				parent_node = root_element;
				//ret = ERROR_INVALID_PARAMETER; 
				//break; 
			}

			hr = node_list->get_length(&node_count);
			if (S_OK != hr)
			{
				parent_node = root_element;
			}

			if( node_count <= 0 )
			{
				break; 
			}

			hr = node_list->get_item(node_count - 1, &node);
			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break;
			}

			hr = node->get_parentNode(&parent_node);

			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break;
			}
		} while (FALSE);

		if (parent_node == NULL)
		{
			ret = ERROR_ERRORS_ENCOUNTERED;
			break;
		}

		ret = insert_node_by_text(xml_src_doc, parent_node, dest_node);
		if (ERROR_SUCCESS != ret)
		{
			break;
		}
	} while (FALSE);

	return ret;
}

//LRESULT WINAPI add_html_element( LPCWSTR text, HTML_PAGE_PROPERTIES *page )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	do 
//	{
//	} while ( FALSE );
//
//	return ret; 
//}

LRESULT WINAPI add_html_element_config_from_text(  MSXML::IXMLDOMDocument *xml_doc, 
									   LPCWSTR url, 
									   MSXML::IXMLDOMNode *page_node, 
									   IHTMLElement *html_element ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr _xml_doc; 
	MSXML::IXMLDOMElementPtr xml_element = NULL;  // 根节点
	BOOLEAN com_inited = FALSE; 
	_bstr_t text; 
	//LPCWSTR _text; 
	//wstring domain_name; 

	do 
	{
		ASSERT( url != NULL ); 
		ASSERT( html_element != NULL ); 
		ASSERT( NULL != xml_doc ); 

		_xml_doc = xml_doc; 

		hr = html_element->get_outerText( text.GetAddress() ); 
		if( hr != S_OK )
		{
			ret = hr; 
			break; 
		}
	}while( FALSE );

	return ret; 
}


LRESULT WINAPI get_html_element_input_value( IHTMLElement *html_element, wstring &input_value, BOOLEAN &is_input )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	_bstr_t text; 
	LPCWSTR _text; 
	HTML_ELEMENT_TYPE element_type; 

	do 
	{
		ASSERT( NULL != html_element ); 

		input_value = L""; 

		hr = html_element->get_tagName( text.GetAddress() ); 
		if( S_OK != hr )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_text = text.operator const wchar_t*(); 
		if( NULL == _text )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		element_type = get_info_type( _text ); 

		is_input = FALSE; 
		switch( element_type )
		{
		case HTML_INPUT_ELEMENT: 
			{
				IHTMLInputTextElementPtr input_text_element; 
				IHTMLInputElementPtr input_element; 
				
				hr = html_element->QueryInterface( IID_IHTMLInputElement, ( PVOID* )&input_element ); 
				if( S_OK != hr )
				{
					break; 
				}

				if( NULL == input_element )
				{
					break; 
				}

				do 
				{
					VARIANT_BOOL checked; 
					hr = input_element->get_type( text.GetAddress() ); 

					if( FAILED( hr ) )
					{
						break; 
					}

					_text = text.operator const wchar_t*(); 
					if( NULL == _text )
					{
						break; 
					}

					if( 0 != wcsicmp( _text, L"checkbox" ) )
					{
						break; 
					}

					input_value = L""; 

					hr = input_element->get_checked( &checked ); 

					if( FAILED( hr ) )
					{
						break; 
					}

					if( checked == VARIANT_TRUE )
					{
						input_value = L"checked"; 
					}

				}while( FALSE );

				hr = html_element->QueryInterface( IID_IHTMLInputTextElement, ( PVOID* )&input_text_element ); 
				if( S_OK != hr )
				{
					break; 
				}

				if( NULL == input_text_element )
				{
					break; 
				}

				is_input = TRUE; 
				input_text_element->get_value( text.GetAddress() ); 

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					//ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				input_value = _text; 
			}
			break; 
		case HTML_BUTTON_ELEMENT: 
			break; 
		case HTML_A_ELEMENT: 
			break; 
		case HTML_BODY_ELEMENT: 
			{
				MSXML::IXMLDOMAttributePtr xml_attr; 

				is_input = TRUE; 
				hr = html_element->get_innerHTML( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

				input_value = _text; 
			}
			break; 
		case HTML_DIV_ELEMENT:
			{
				is_input = TRUE; 

				hr = html_element->get_innerText( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

			}
			break; 
		case HTML_TEXT_AREA_ELEMENT:
			{
				is_input = TRUE; 

				hr = html_element->get_innerText( text.GetAddress() ); 
				if( S_OK != hr )
				{
					break; 
				}

				_text = text.operator const wchar_t*(); 
				if( NULL == _text )
				{
					break; 
				}

			}
			break; 
		case HTML_SELECT_ELEMENT:
			{

			}
			break; 
		default:
			ASSERT( FALSE ); 
			log_trace_ex( MSG_IMPORTANT, "unknown HTML element: %s\n", _text ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
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

	VARIANT_BOOL __ret; 

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

#include "html_element_prop_dlg.h"

LRESULT WINAPI make_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 
	//HANDLE file_handle = INVALID_HANDLE_VALUE; 
	//WCHAR xml_template[] = L"<html_sites></html_sites>"; 
//#define MAX_TIME_TEXT 128

	//L"<?xml version=\"1.0\"?>"; 
	//L"<site url=\"%s\" >" \
	//L"<page url=\"%s\">" \
	//L"</page>" \
	//L"</site>";

	do 
	{
		ASSERT( config_file_path != NULL ); 

		ret = check_file_exist( config_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			//ret = ERROR_SUCCESS; 


			//file_handle = CreateFileW( config_file_path, 
			//	GENERIC_READ | GENERIC_WRITE, 
			//	FILE_SHARE_READ, 
			//	NULL, 
			//	OPEN_ALWAYS, 
			//	0, 
			//	NULL ); 

			//if( file_handle == INVALID_HANDLE_VALUE )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			//_ret = WriteFile( file_handle, 
			//	xml_template, 
			//	sizeof( xml_template ) - sizeof( WCHAR ), 
			//	&wrote, 
			//	NULL ); 

			//if( FALSE == _ret )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			ret = create_config_file( config_file_path ); 

		}
		else
		{
#ifdef _DEBUG
			ret = create_config_file( config_file_path ); 
#else	
			ASSERT( FALSE ); 
#endif //_DEBUG

		}

	}while( FALSE ); 

	//if( INVALID_HANDLE_VALUE != file_handle )
	//{
	//	CloseHandle( file_handle ); 
	//}

	return ret; 
}

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
	//IHTMLElementCollectionPtr html_elements; 
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

		hr = show_active_element_xpath(); 
		break; 


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


	}while( FALSE );

	return; 
}

void browser_dlg::OnMenuBarExplorer1(BOOL MenuBar)
{
	// TODO: Add your message handler code here
}

void browser_dlg::NewWindow2Explorer1(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	// TODO: Add your message handler code here
	*Cancel = TRUE; 
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

	if( m_WebBrowser.can_navigate() == FALSE )
	{
		*Cancel = TRUE; 
	}
}

void browser_dlg::OnBnClickedButtonRecordPage()
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	//HRESULT hr; 
	//ULONG _ret; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	//IHTMLElementCollectionPtr html_elements; 
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

LRESULT WINAPI find_page_group_config( LPCWSTR file_name, 
									  LPCWSTR domain_name, 
									  MSXML::IXMLDOMDocument **xml_doc_out, 
									  MSXML::IXMLDOMElement **site_element_out ); 

/****************************************************
如果使用域名做文件名，冲突率比较大
如果使用URL做文件名，冲突率更小
如果使用时间做文件，冲突最小，但不可以添加
使用URL，给页面以添加功能
****************************************************/
LRESULT WINAPI get_file_name_from_url( LPCWSTR url, CStringW &file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR file_name_char[] = { L'_', L'-', L'.', L',' }; 
	LONG cc_url_len; 
	LONG i; 
	BOOLEAN char_found = FALSE; 
	LONG j; 

	do 
	{
		file_name = L""; 

		cc_url_len = wcslen( url ); 
		for( i = 0; i < cc_url_len; i ++ )
		{
			if( url[ i ] >= 'A' && url[ i ] <= 'Z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= 'a' && url[ i ] <= 'z' )
			{
				file_name += url[ i ]; 
			}
			else if( url[ i ] >= '0' && url[ i ] <= '9' )
			{
				file_name += url[ i ]; 
			}
			else
			{
				char_found = FALSE; 
				for( j = 0; j < ARRAYSIZE( file_name_char ); j ++ )
				{
					if( file_name_char[ j ] == url[ i ] )
					{
						char_found = TRUE; 
						break; 
					}
				}

				if( char_found == TRUE )
				{
					file_name += url[ i ]; 
				}
				else
				{
					file_name += L'_'; 
				}
			}
		}
	}while( FALSE );

	return ret; 
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

HRESULT browser_dlg::show_active_element_xpath()
{
	HRESULT hr; 
	html_element_prop_dlg dlg; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		disp = m_WebBrowser.GetDocument(); 

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 

		if( FAILED( hr ) 
			|| html_doc == NULL )
		{
			break; 
		}

		dump_html_elements_on_page( html_doc ); 
		//disable_web_page_anchor( html_doc ); 

		wstring xpath; 
		get_active_element_xpath( html_doc, &xpath ); 

		dlg.set_html_element_xpath( xpath.c_str() ); 

		dlg.DoModal(); 
	} while ( FALSE );

	return hr; 
}


void browser_dlg::OnBnClickedButtonScramble()
{
	LRESULT ret; 
	HRESULT hr; 
	wstring text; 
	_bstr_t _text; 
	wstring text_name; 
	IHTMLElementPtr html_element; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		BOOLEAN is_input; 

		hr = html_scramble_ex( &m_WebBrowser, scramble_info.url.c_str() ); 
		if( FAILED(hr))
		{
			//break; 
		}

	}while( FALSE );

	// TODO: Add your control notification handler code here
}

void browser_dlg::NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}

void browser_dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	INT32 ret; 
	POINT pt; 
	// TODO: Add your message handler code here and/or call default

	do 
	{
		//ret = GetCursorPos(&pt); 
		//if( FALSE == ret )
		//{
		//	break; 
		//}

		//ClientToScreen( &pt ); 
		//m_WebBrowser.ScreenToClient( &pt ); 

		//get_active_element_xpath()
	}while( FALSE );

	CDialog::OnLButtonUp(nFlags, point);
}
