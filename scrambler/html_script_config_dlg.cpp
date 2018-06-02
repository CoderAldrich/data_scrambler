// Browser_ControlDlg.cpp : implementation file
//

#include "stdafx.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include <exdispid.h>
#include "mshtml_addition.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "html_script_config_dlg.h"
#include "html_element_prop_dlg.h"
#include "external_link.h"
#include "html_element_config.h"
#include "data_analyze.h"
#include "common_html_dlg.h"
#include "data_scramble.pb.h"
#include "webbrowser.h"
#include "ie_hook.h"
#include "ie_auto.h"
#include "plugin.h"
#include "behavior_handler.h"
#include "sqlite_conn.h"
//#include "event_handler.h"

using namespace data_scrabmle; 
//#include "data_html_dlg.h"

//#include "xpath_edit_dlg.h"

thread_manage service_thread = { 0 }; 

using namespace MSXML; 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOLEAN stop_scramble = FALSE; 

//9999 (0x270F) 
//Internet Explorer 9. Webpages are displayed in IE9 Standards mode, regardless of the !DOCTYPE directive.
//
//9000 (0x2328) 
//Internet Explorer 9. Webpages containing standards-based !DOCTYPE directives are displayed in IE9 mode.
//
//8888 (0x22B8) 
//Webpages are displayed in IE8 Standards mode, regardless of the !DOCTYPE directive.
//
//8000 (0x1F40) 
//Webpages containing standards-based !DOCTYPE directives are displayed in IE8 mode.
//
//7000 (0x1B58) 
//Webpages containing standards-based !DOCTYPE directives are displayed in IE7 Standards mode.

LRESULT WINAPI config_ie_control_version()
{
	LRESULT ret = ERROR_SUCCESS; 
	CRegKey regKey;
	ULONG compatible_version; 
	LONG lResult;

	do 
	{
#define IE_EMULATION_CONFIG_WOW32_KEY L"SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION" 
#define IE_EMULATION_CONFIG_KEY L"SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION" 
#define DATA_SCRAMBLER_IE_COMPATIBLE_VERSION 11000

		do 
		{
			lResult = regKey.Open(HKEY_LOCAL_MACHINE, IE_EMULATION_CONFIG_KEY, KEY_WRITE | KEY_READ );

			if(lResult!=ERROR_SUCCESS)
			{    
				ret = lResult; 
				break; 
			}

			//regKey.DeleteValue(L"data_scrambler.exe"); 
			//break; 

			LONG result = regKey.QueryDWORDValue(L"data_scrambler.exe", compatible_version); 
			if(result != ERROR_SUCCESS)
			{
				if( compatible_version != 0 )
				{
					ASSERT( FALSE ); 
					compatible_version = 0; 
				}
			}

			if( compatible_version != DATA_SCRAMBLER_IE_COMPATIBLE_VERSION )
			{
				LONG result = regKey.SetDWORDValue(L"data_scrambler.exe", DATA_SCRAMBLER_IE_COMPATIBLE_VERSION); 
				if(result != ERROR_SUCCESS)
				{
					ret = result; 
				}
			}
			regKey.Close();  
		}while( FALSE );

		do 
		{

			lResult = regKey.Open(HKEY_LOCAL_MACHINE, IE_EMULATION_CONFIG_KEY, KEY_WRITE | KEY_READ | KEY_WOW64_64KEY );

			if(lResult!=ERROR_SUCCESS)
			{    
				ret = lResult; 
				break; 
			}

			//regKey.DeleteValue(L"data_scrambler.exe"); 
			//break; 

			LONG result = regKey.QueryDWORDValue(L"data_scrambler.exe", compatible_version); 
			if(result != ERROR_SUCCESS)
			{
				if( compatible_version != 0 )
				{
					ASSERT( FALSE ); 
					compatible_version = 0; 
				}
			}

			if( compatible_version != DATA_SCRAMBLER_IE_COMPATIBLE_VERSION )
			{
				LONG result = regKey.SetDWORDValue(L"data_scrambler.exe", DATA_SCRAMBLER_IE_COMPATIBLE_VERSION); 
				if(result != ERROR_SUCCESS)
				{
					ret = result; 
				}
			}
			regKey.Close();  
		}while( FALSE );

	}while( FALSE ); 

	return ret; 
}
//CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
//{
//	//{{AFX_DATA_INIT(CAboutDlg)
//	//}}AFX_DATA_INIT
//}
//
//void CAboutDlg::DoDataExchange(CDataExchange* pDX)
//{
//	CDialog::DoDataExchange(pDX);
//	//{{AFX_DATA_MAP(CAboutDlg)
//	//}}AFX_DATA_MAP
//}
//
//BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//	//{{AFX_MSG_MAP(CAboutDlg)
//		// No message handlers
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg dialog
HTML_LIST_ELEMENT_SCRAMBLE_INFO test_scramble_info; 

//XXXXSink *document_event_handler; 
html_script_config_dlg::html_script_config_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(html_script_config_dlg::IDD, pParent), 
	com_inited( FALSE ), 
    locate_url_manually( FALSE )
{
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1); 
	ui_handler = new doc_host_ui_handler(); 
	 
	//behavior_handler = new html_behavior_handler(); 
	
	element_behavior = new html_element_behavior(); 
	//document_event_handler = new XXXXSink(); 
}

LRESULT html_script_config_dlg::check_point_in_ie_control( PPOINT pt )
{
	LRESULT ret = ERROR_NOT_FOUND; 
	CWnd *ie_control; 
	RECT wnd_rect; 

	do 
	{
		ASSERT( NULL != pt ); 

		if( TRUE == html_process_dlg.IsWindowVisible() )
		{
			html_process_dlg.GetWindowRect( &wnd_rect ); 
			if( PtInRect( &wnd_rect, *pt ) )
			{
				break; 
			}
		}

#if CONFIG_DLG_HAVE_XPATH_DLG
		if( TRUE == xpath_dlg.IsWindowVisible() )
		{
			xpath_dlg.GetWindowRect( &wnd_rect ); 
			if( PtInRect( &wnd_rect, *pt ) )
			{
				break; 
			}
		}
#endif //CONFIG_DLG_HAVE_XPATH_DLG

		//if( TRUE == sramble_list_dlg.IsWindowVisible() )
		//{
		//	sramble_list_dlg.GetWindowRect( &wnd_rect ); 
		//	if( PtInRect( &wnd_rect, *pt ) )
		//	{
		//		break; 
		//	}
		//}

		ie_control = GetDlgItem( IDC_WEB_BROWSER_CONTROL ); 
		if( ie_control == NULL )
		{
			break; 
		}

		ie_control->GetWindowRect( &wnd_rect ); 

		if( FALSE == PtInRect( &wnd_rect, *pt ) )
		{
			break; 
		}

		ret = ERROR_SUCCESS; 

	} while ( FALSE ); 

	return ret; 
}

void html_script_config_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(html_script_config_dlg)
	DDX_Control(pDX, IDC_ANIMATE1, m_Animate);
	DDX_Control(pDX, IDC_WEB_BROWSER_CONTROL, m_WebBrowser);
	//}}AFX_DATA_MAP
}

LRESULT html_script_config_dlg::get_selected_html_element()
{
    LRESULT ret = ERROR_SUCCESS; 
    HRESULT hr; 
    IHTMLDocument2Ptr html_doc;
    IDispatchPtr disp;
    HTML_ELEMENT_GROUP elements; 

    do
    {
        disp = m_WebBrowser.GetDocument();

        if (NULL == disp)
        {
            break;
        }

        hr = disp->QueryInterface(IID_IHTMLDocument2, (PVOID*)&html_doc);

        if (FAILED(hr)
            || html_doc == NULL)
        {
            break;
        }

        ret = get_selected_html_elements(html_doc, &elements);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }
    } while (FALSE); 
    
    return ret; 
}

LRESULT html_script_config_dlg::select_html_elements( HTML_ELEMENT_GROUP *elements )
{
    LRESULT ret = ERROR_SUCCESS;
    HRESULT hr;
    IHTMLDocument2Ptr html_doc;
    IDispatchPtr disp;
    ULONG  element_add_type;

    //IWebBrowser2Ptr web_browser; 
    //IHTMLWindow2 *html_window; 
    //IHTMLSelectionObjectPtr selection; 

    do
    {
        //html_window->
        disp = m_WebBrowser.GetDocument();
        if (NULL == disp)
        {
            break;
        }

        //disp = m_WebBrowser.GetControlUnknown();

        //if (NULL == disp)
        //{
        //    ASSERT(FALSE);
        //    break;
        //}

        //hr = disp->QueryInterface(IID_IWebBrowser2, (PVOID*)&web_browser);
        //if (FAILED(hr))
        //{
        //    break;
        //}

        //if (NULL == web_browser)
        //{
        //    break;
        //}

        //hr = web_browser->get_Document(&disp);
        //if (FAILED(hr))
        //{
        //    break;
        //}

        //if (NULL == disp)
        //{
        //    hr = E_UNEXPECTED;
        //    break;
        //}

        hr = disp->QueryInterface(IID_IHTMLDocument2,
            (PVOID*)&html_doc);

        if (FAILED(hr))
        {
            break;
        }

        if (NULL == html_doc)
        {
            hr = E_UNEXPECTED;
            break;
        }

        //hr = html_doc->get_parentWindow(&html_window); 
        //if (FAILED(hr))
        //{
        //    break;
        //}

        //hr = html_doc->get_selection(&selection); 
        //if (FAILED(hr))
        //{
        //    break;
        //}

        //IHTMLWindow3Ptr _html_window;
        //_html_window->selection
        //if (NULL == selection)
        //{
        //    break;
        //}

        //if (html_doc->) 
        //{     // all browsers but IE
        //    var selection = window.getSelection();
        //    var rangeToSelect = document.createRange();
        //    rangeToSelect.selectNodeContents(elemToSelect);
        //    //console.log(rangeToSelect);
        //    selection.removeAllRanges();
        //    selection.addRange(rangeToSelect);
        //}
        //else 
        {
            //HTML_ELEMENT_GROUP _elements; 
            IHTMLElementPtr _element;
            IHTMLElement2Ptr _body;
            IHTMLBodyElementPtr body;
            IDispatchPtr disp;
            IHTMLTxtRangePtr txt_range;
            IHTMLControlRangePtr control_range;
            IHTMLControlElementPtr control_element;


            do
            {
                html_doc->get_body(&_element);
                if (FAILED(hr))
                {
                    break;
                }

                if (NULL == _element)
                {
                    break;
                }

                hr = _element->QueryInterface(IID_IHTMLElement2, (void**)&_body);
                if (FAILED(hr))
                {
                    break;
                }

                if (NULL == _body)
                {
                    break;
                }

                hr = _element->QueryInterface(IID_IHTMLBodyElement, (void**)&body);
                if (FAILED(hr))
                {
                    break;
                }

                if (NULL == body)
                {
                    break;
                }

                hr = body->createTextRange(&txt_range);
                if (FAILED(hr))
                {
                    ret = hr;
                    break;
                }

                if (NULL == txt_range)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }

                _body->createControlRange(&disp);
                if (FAILED(hr))
                {
                    ret = hr;
                    break;
                }

                if (NULL == disp)
                {
                    ret = ERROR_ERRORS_ENCOUNTERED;
                    break;
                }

                hr = disp->QueryInterface(IID_IHTMLControlRange, (void**)&control_range);
                if (FAILED(hr))
                {
                    break;
                }

                for (HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = elements->begin();
                    it != elements->end();
                    it++)
                {
                    element_add_type = 0;
                    do
                    {
                        hr = (*it)->QueryInterface(IID_IHTMLControlElement, (void**)&control_element);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if (NULL == control_element)
                        {
                            break;
                        }

                        control_range->add(control_element);
                        element_add_type = 1;
                    } while (FALSE);

                    if (element_add_type != 1)
                    {
                        hr = txt_range->moveToElementText((*it));
                        if (FAILED(hr))
                        {
                            break;
                        }

                        element_add_type = 2; 
                        break;
                    }
                }

                if (element_add_type == 2)
                {
                    txt_range->select();
                }
                else if (element_add_type == 1)
                {
                    control_range->select();
                }
            } while (FALSE);
        }
    } while (FALSE);

    return ret;
}

#define WM_WEB_PAGE_ACTIONS ( WM_USER + 1001 )

#include "html_event_handler.h"
html_script_config_dlg *g_html_script_config_dlg; 
static wstring hilight_xpath; 
static wstring hilight_url; 

HRESULT STDMETHODCALLTYPE HtmlElementEventHandler::Invoke( /* [in] */ DISPID dispIdMember, /* [in] */ REFIID riid, /* [in] */ LCID lcid, /* [in] */ WORD wFlags, /* [out][in] */ DISPPARAMS *pDispParams, /* [out] */ VARIANT *pVarResult, /* [out] */ EXCEPINFO *pExcepInfo, /* [out] */ UINT *puArgErr)
{ 
	INT32 i; 
	wstring text; 

	dbg_print( MSG_IMPORTANT, "enter %s dispatch id is 0x%0.8x\n", __FUNCTION__, dispIdMember); 

	for( i = 0; ( UINT32 )i < pDispParams->cArgs; i ++ )
	{
		text = variant2string( &pDispParams->rgvarg[ i ] ); 
		dbg_print( MSG_IMPORTANT, "dispatch parameter(%u): %ws", i, text.c_str() ); 
	}

	//if(dispIdMember == HTML_ELEMENT_EVENT_HANDLER_ID) 
	{ 
		//g_browser_dlg->m_WebBrowser.switch_navigate( FALSE ); 
		//g_web_script_config_dlg->OnLButtonUp( 0, CPoint(0,0)); 
		//MessageBox(0,L"Hello World",L"Hello",0); 
		//place your code here 
	} 

	return S_OK; 
} 

//ULONG CALLBACK work_test_thread( PVOID param )
//{
//	LRESULT ret = 0; 
//
//	do 
//	{
//		browser_safe_navigate_end( &g_browser_dlg->m_WebBrowser ); 
//
//		ret = g_browser_dlg->SendMessageW( WM_WORKING, 0, ( LPARAM )param ); 
//		ExitThread( ( ULONG )ret ); 
//	}while( FALSE );
//
//	return ( ULONG )ret; 
//}

BEGIN_MESSAGE_MAP(html_script_config_dlg, CDialog)
	//{{AFX_MSG_MAP(CBrowser_ControlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_DESTROY() 
	ON_WM_TIMER() 
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BACK, OnBack)
	ON_BN_CLICKED(ID_FORWARD, OnForward)
	ON_BN_CLICKED(ID_STOP, OnStop)
	ON_BN_CLICKED(ID_HOME, OnHome)

	ON_MESSAGE( WM_WEB_PAGE_ACTIONS, OnWorking )	
	ON_MESSAGE( WM_LOCATED_TO_URL, on_locate_to_url )	

	ON_BN_CLICKED(IDC_BUTTON_RECORD_SELECTED, &html_script_config_dlg::OnBnClickedButtonRecordSelected)
	ON_BN_CLICKED(IDC_BUTTON_RECORD_PAGE, &html_script_config_dlg::OnBnClickedButtonRecordPage)
	ON_BN_CLICKED(IDC_BUTTON_NEW_PAGE_GROUP, &html_script_config_dlg::OnBnClickedButtonNewPageGroup)
	ON_BN_CLICKED(IDC_BUTTON_CPATURE_INPUT, &html_script_config_dlg::OnBnClickedButtonScramble)

	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_RUN_SCRIPT, &html_script_config_dlg::OnBnClickedButtonRunScript)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowser_ControlDlg message handlers

extern thread_manage service_thread; 
ULONG WINAPI web_browser_service( PVOID param ); 

LRESULT CALLBACK test_analyze_scramble_data( PVOID data, ULONG data_size, PVOID *ret_data, ULONG *ret_data_size )
{
	LRESULT ret = ERROR_SUCCESS; 
	wstring unicode_text; 
	string ansi_text; 
	ULONG message; 
	scramble_response response; 

	do 
	{
		if( data_size < sizeof( ULONG ))
		{
			ret = ERROR_INVALID_DATA; 
			break; 
		}

		if( NULL == ret_data )
		{
			ret = ERROR_INVALID_PARAMETER; 
		}

		if( NULL == ret_data_size )
		{
			ret = ERROR_INVALID_PARAMETER; 
		}

		*ret_data = NULL; 
		*ret_data_size = 0; 

		if( TRUE == stop_scramble )
		{
			*ret_data = malloc( sizeof( ULONG ) ); 
			if( *ret_data != NULL )
			{
				*( ULONG* )( *ret_data ) = COMMNAD_EXIT_WORK; 
				*ret_data_size = sizeof( ULONG ); 
			}

			break; 
		}

		message = *( ULONG* )data; 
		data = ( PBYTE )data + sizeof( ULONG ); 
		data_size -= sizeof( ULONG ); 

		switch( message )
		{
		case REQUEST_SCRAMBLE_CONFIG: 
			{
				wstring url; 
				//LRESULT _ret; 
				scramble_command scramble_config; 
				content_scramble *content; 
				frame_scramble *frame; 
				field_scramble *field; 
				PVOID data = NULL; 
				ULONG data_size; 
				HTML_ELEMENT_CONTENTS_ITERATOR it; 

				do 
				{
					if( hilight_xpath.length() > 0 )
					{
						frame = scramble_config.mutable_frame(); 
						if( NULL == frame )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							break; 
						}

						scramble_config.set_mode( CONTENT_WEB_PAGE_HILIGHT ); 

						ret = unicode_to_utf8_ex( hilight_xpath.c_str(), ansi_text ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						frame->set_links_xpath( ansi_text.c_str() ); 

						ret = unicode_to_utf8_ex( hilight_url.c_str(), ansi_text ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						frame->set_url( ansi_text ); 
						frame->set_next_xpath( "" ); 

						{
							PVOID _data; 
							ULONG _data_size; 

							do 
							{
								_data_size = scramble_config.ByteSize(); 

								_data = malloc( _data_size ); 
								if( NULL == _data )
								{
									ret = ERROR_NOT_ENOUGH_MEMORY; 
									break; 
								}

								if( false == scramble_config.IsInitialized() )
								{
									scramble_config.CheckInitialized(); 
									REMOTE_ERROR_DEBUG(); 
									ret = ERROR_INVALID_PARAMETER; 
									break; 
								}

								if( false == scramble_config.SerializeToArray( _data, _data_size ) )
								{
									ret = ERROR_ERRORS_ENCOUNTERED; 
									break; 
								}

								*ret_data = _data; 
								*ret_data_size = _data_size; 
								_data = NULL; 
							} while ( FALSE );

							if( NULL != _data )
							{
								free( _data ); 
							}
						}
					}
				}while( FALSE ); 
			}
			break; 
		default:
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

ULONG WINAPI web_browser_service( PVOID param ) 
{
	LRESULT ret = ERROR_SUCCESS;
	HRESULT hr; 
	DWORD wait_ret;
	ULONG ret_len; 
	thread_manage *thread_param; 
	PVOID _param;
	pipe_ipc_point point = { 0 }; 

	CHAR *data = NULL; 
	ULONG data_len; 
	CHAR *ret_data = NULL; ; 
	ULONG ret_data_len; 
	BOOLEAN pipe_inited = FALSE; 
	BOOLEAN pipe_connected; 
	WCHAR pipe_name[ MAX_PATH ]; 

	ANALYZE_CLIENT_DATA_FUNC analyze_func; 

	ASSERT( NULL != param ); 

	thread_param = ( thread_manage* )param; 
	ASSERT( thread_param->param != NULL ); 
	analyze_func = ( ANALYZE_CLIENT_DATA_FUNC )thread_param->param; 

	ASSERT( NULL != analyze_func ); 

	log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) );

	hr = StringCchPrintfW( pipe_name, ARRAYSIZE( pipe_name ), DATA_SCRAMBLE_PIPE_POINT_NAME, GetCurrentProcessId() ); 
	if( FAILED(hr))
	{

	}

	ret = init_pipe_point( &point, pipe_name ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	ret = create_name_pipe( point.pipe_name, &point.pipe ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	pipe_inited = TRUE; 

#define MAX_PIPE_DATA_LEN ( 1024 * 1024 * 6 )

	//ret_data = ( CHAR* )malloc( MAX_PIPE_DATA_LEN ); 
	//if( ret_data == NULL )
	//{
	//	ret = ERROR_NOT_ENOUGH_MEMORY; 
	//	goto _return; 
	//}

	for( ; ; )
	{
		//#define CHECK_UI_LOG_EXIST_TIME_SPAN 30
		//wait_ret = wait_event( thread_param->notify, CHECK_UI_LOG_EXIST_TIME_SPAN ); 
		//if( wait_ret != WAIT_TIMEOUT 
		//	&& wait_ret != WAIT_OBJECT_0 
		//	&& wait_ret != WAIT_ABANDONED )
		//{
		//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "wait syncronous event failed will exit\n" ) ); 
		//	break; 
		//}

		if( thread_param->stop_run == TRUE )
		{
			goto _return; 
		}

		do 
		{
			pipe_connected = FALSE; 

			ret = accept_name_pipe_client_sync( point.pipe ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			pipe_connected = TRUE; 

			if( thread_param->stop_run == TRUE )
			{
				break; 
			}

			ret = read_pipe_sync( &point, ( CHAR* )&data_len, sizeof( data_len ) ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			if( data_len == 0 || data_len > MAX_PIPE_DATA_LEN )
			{
				ret = ERROR_INVALID_DATA; 
				break; 
			}

			data = ( CHAR* )malloc( data_len ); 
			if( data == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				goto _return; 
			}

			//realloc_if_need( &data, )
			ret = read_pipe_sync( &point, data, data_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = analyze_func( data, data_len, ( PVOID* )&ret_data, &ret_data_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = write_pipe_sync( &point, ( CHAR* )&ret_data_len, sizeof( ULONG ) ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = write_pipe_sync( &point, ret_data, ret_data_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

		}while( FALSE ); 

		if( TRUE == pipe_connected )
		{
			ret = disconnect_name_pipe_client( point.pipe ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}
		}

		if( NULL != data )
		{
			free( data ); 
			data = NULL; 
		}

		if( NULL != ret_data )
		{
			free( ret_data ); 
			ret_data = NULL; 
		}

		Sleep( 0 ); 
	}

	log_trace( ( MSG_INFO, "leave %s \n", __FUNCTION__ ) ); 
_return:
	if( NULL != ret_data )
	{
		free( ret_data ); 
	}

	if( NULL != data )
	{
		free( data ); 
	}

	if( TRUE == pipe_inited )
	{
		uninit_pipe_point( &point ); 
	}

	return ret; 
}

LRESULT WINAPI start_test_web_browser_service()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = _create_work_thread( &service_thread, 
			web_browser_service, 
			NULL, 
			NULL, 
			( PVOID )test_analyze_scramble_data ); 

		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI stop_test_web_browser_service()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = stop_work_thread( &service_thread ); 

	}while( FALSE ); 

	return ret;  
}

VOID html_script_config_dlg::OnDestroy()
{
	LRESULT ret; 

	ret = reset_hook( HOOK_INDEX_MOUSE ); 
	if( ret != ERROR_SUCCESS )
	{

	}

	ret = unload_all_plugins(); 
	if( ret != ERROR_SUCCESS )
	{

	}
}

//#include <Oleacc.h>

LRESULT WINAPI check_ie_window( HWND wnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR text[ MAX_PATH ]; 
	ULONG iPID = 0;
	HANDLE hModule; 
	LONG iWNDID; 
	MODULEENTRY32W* minfo = NULL;

	do
	{
		ASSERT( NULL != wnd ); 

		//dbg_print( MSG_IMPORTANT, "windows %x information:\n", wnd ); 
		::GetClassName(wnd, text,ARRAYSIZE(text));

#define IE_WINDOW_CLASS_NAME L"Internet Explorer_Server" 

		if( 0 == wcscmp( IE_WINDOW_CLASS_NAME, text ) )
		{
			break; 
		}

		//dbg_print( MSG_IMPORTANT, "window class name: %ws\n", text ); 
		
		::GetWindowText(wnd, text,ARRAYSIZE(text));
		//dbg_print( MSG_IMPORTANT, "window text: %ws\n", text ); 

		iWNDID = GetWindowLong(wnd,GWL_ID);
		
		//dbg_print( MSG_IMPORTANT, "window id: %u\n", iWNDID ); 

		GetWindowThreadProcessId(wnd,&iPID);
		//dbg_print( MSG_IMPORTANT, "window process id: %u\n", iPID ); 

		ret = ERROR_NOT_FOUND; 

		minfo = ( MODULEENTRY32W* )malloc( sizeof( MODULEENTRY32W ) ); 
		if( NULL == minfo )
		{
			break; 
		}

		minfo->dwSize = sizeof( MODULEENTRY32 ); 
		
		hModule = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, iPID ); 
		if( NULL == hModule )
		{
			break; 
		}

		Module32First( hModule, minfo );
		//dbg_print( MSG_IMPORTANT, "window executable file: %ws\n", minfo->szExePath ); 

	}while( FALSE ); 

	if( NULL != minfo ) 
	{
		delete minfo;
	}

	return ret; 
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)   
{
	LRESULT ret = ERROR_SUCCESS; 
    static BOOLEAN button_down = FALSE;

	//dbg_print( MSG_IMPORTANT, "%s:%u\n", __FUNCTION__, __LINE__ ); 

	do 
	{
		if (nCode < 0)  // do not process the message 
		{
			ret = CallNextHookEx(myhookdata[HOOK_INDEX_MOUSE].hhook, nCode,   
				wParam, lParam); 

			break; 
		}


		//dbg_print( MSG_IMPORTANT, "hook message 0x%0.8x %u\n", wParam, wParam);


        //wParam == WM_LBUTTONUP
        //|| wParam == WM_LBUTTONDOWN
        //    || wParam == WM_RBUTTONDOWN )

        if (wParam == WM_RBUTTONUP)
        {
            button_down = FALSE; 
        }
        else if (wParam == WM_RBUTTONDOWN
            || wParam == WM_MOUSEMOVE)
        {

            if (wParam == WM_RBUTTONDOWN)
            {
                button_down = TRUE;
            }

            //dbg_print( MSG_IMPORTANT, "l button up\n" ); 
			MOUSEHOOKSTRUCTEX *mouse_hook; 
			static POINT prev_point = { 0, 0 }; 
            //static BOOLEAN prev_call_next = TRUE; 
            //BOOLEAN call_next = TRUE; 

            if (button_down == FALSE)
            {
                break;
            }

			do 
			{
                
				mouse_hook = ( MOUSEHOOKSTRUCTEX* )lParam; 

				//{
				//	HRESULT hr  = E_FAIL; 
				//	IWebBrowser2Ptr web_browser; 
				//	IDispatchPtr disp; 
				//HWND _browser_wnd = NULL; 

				//	disp = g_web_script_config_dlg->m_WebBrowser.GetControlUnknown(); 

				//	if( NULL == disp )
				//	{
				//		ASSERT( FALSE ); 
				//		break; 
				//	}


				//	hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
				//	if( FAILED( hr ))
				//	{
				//		break; 
				//	}

				//	if( NULL == web_browser )
				//	{
				//		break; 
				//	}

				//	web_browser->get_HWND( (SHANDLE_PTR*)&_browser_wnd ); 
				//	if ( SUCCEEDED(hr) )
				//	{
				//		if( _browser_wnd != mouse_hook->hwnd )
				//		{
				//			break; 
				//		}
				//	} 
				//}

				//if( ie_control->GetSafeHwnd() != ::GetFocus() )
				//{
				//	break; 
				//}

				//if( FALSE == ie_control->IsTopParentActive() )
				//{
				//	break; 
				//}

				ret = check_ie_window( mouse_hook->hwnd ); 
				if( ret != ERROR_SUCCESS )
				{
                    prev_point.x = -65535;
                    prev_point.y = -65535;
                    //prev_call_next = TRUE;
                    break;
				}

				if( prev_point.x == mouse_hook->pt.x 
					&& prev_point.y == mouse_hook->pt.y )
				{
                    //call_next = prev_call_next;
					break; 
				}
 
				//{
				//	HRESULT hr; 
				//	CComPtr< IHTMLDocument2 > sp_doc;
				//	hr = ObjectFromLresult( ( LRESULT )( ULONG_PTR )mouse_hook->hwnd, IID_IHTMLDocument, 0, (void**)&sp_doc );
				//	if ( FAILED(hr) )
				//	{
				//		break; 
				//	}
				//}

				if( ERROR_SUCCESS == g_html_script_config_dlg->check_point_in_ie_control( &mouse_hook->pt ) )
				{
                    //ULONG work_mode; 
                    g_html_script_config_dlg->on_html_page_clicked(0, CPoint(mouse_hook->pt)); // , &work_mode);
                
                    //if (work_mode == 0)
                    //{
                    //    call_next = FALSE;
                    //}
                    //else
                    //{
                    //    if (wParam == WM_RBUTTONUP)
                    //    {
                    //        wParam = WM_LBUTTONUP;
                    //    }
                    //    if (wParam == WM_RBUTTONDOWN)
                    //    {
                    //        wParam = WM_LBUTTONDOWN;
                    //    }
                    //}

                    prev_point = mouse_hook->pt;
                }
				else
				{
                    prev_point.x = -65535;
                    prev_point.y = -65535;
				}

                //prev_call_next = call_next;
            } while ( FALSE );

            //if (call_next == TRUE)
            //{
            //    ret = CallNextHookEx(myhookdata[HOOK_INDEX_MOUSE].hhook, nCode, wParam,
            //        lParam);
            //}
		}
        
        //else if ( wParam != WM_LBUTTONDBLCLK )
        {
            ret = CallNextHookEx(myhookdata[HOOK_INDEX_MOUSE].hhook, nCode, wParam,
                lParam);
        }

	} while ( FALSE );

	return ret; 
}   

BOOL html_script_config_dlg::OnInitDialog()
{
	LRESULT ret; 
	CDialog::OnInitDialog();

	g_html_script_config_dlg = this; 
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
	//L"https://detail.tmall.com/item.htm?spm=a220m.1000858.1000725.1.9DFix1&id=528293783211&skuId=3145083402457&areaId=110000&cat_id=2&rn=9c867e3154a13b2b6dfcc05919e1d077&standard=1&user_id=268451883&is_b=1"

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
	
	init_scramble_info( &test_scramble_info);
	m_WebBrowser.SetSilent( TRUE ); 

	{
		INT32 _ret; 
		_ret = SetPropW( GetSafeHwnd(), DATA_SCRAMBLER_MAIN_WINDOW_PROP_NAME, ( HANDLE )1 ); 

		ASSERT( TRUE == _ret ); 
	}

#ifdef CONFIG_DLG_HAVE_XPATH_DLG
	xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
	xpath_dlg.ShowWindow( SW_HIDE ); 
#endif //CONFIG_DLG_HAVE_XPATH_DLG

	//test_browser_dlg.Create( MAKEINTRESOURCE( test_browser_dlg.IDD ), this ); 

	//CRect sub_wnd_rect; 
	//test_browser_dlg.GetWindowRect( &sub_wnd_rect ); 
	//test_browser_dlg.MoveWindow( 0, 40, sub_wnd_rect.Width(), sub_wnd_rect.Height(), FALSE ); 
	//test_browser_dlg.ShowWindow( SW_HIDE ); 

	//data_dlg.Create( MAKEINTRESOURCE( data_dlg.IDD ), this ); 
	//data_dlg.ShowWindow( SW_HIDE ); 

	html_process_dlg.Create( MAKEINTRESOURCE( html_process_dlg.IDD ), this ); 
	html_process_dlg.ShowWindow( SW_SHOW ); // 
	//prop_dlg.CenterWindow(); 

	do
	{
		CRect wnd_rect; 
		CRect child_wnd_rect; 
		CRect _child_wnd_rect; 

		GetWindowRect( &wnd_rect ); 

		html_process_dlg.GetWindowRect( &child_wnd_rect ); 

		_child_wnd_rect.left = wnd_rect.left + 
			( ( wnd_rect.Width() - child_wnd_rect.Width() ) / 2 ); 
		_child_wnd_rect.right = _child_wnd_rect.left + child_wnd_rect.Width(); 

		_child_wnd_rect.top = wnd_rect.top + 
			( ( wnd_rect.Height() - child_wnd_rect.Height() ) / 2 ); 
		_child_wnd_rect.bottom = _child_wnd_rect.top + child_wnd_rect.Height(); 

		html_process_dlg.MoveWindow( &_child_wnd_rect, TRUE ); 
	}while( FALSE ); 

//#define DEFAULT_BROWSER_URL L"https://www.amazon.cn/s/ref=nb_sb_noss_1?__mk_zh_CN=%E4%BA%9A%E9%A9%AC%E9%80%8A%E7%BD%91%E7%AB%99&url=search-alias%3Daps&field-keywords=%E5%B9%B3%E6%9D%BF%E7%94%B5%E8%84%91"; 
//	//L"http://search.jd.com/Search?keyword=%E6%8A%95%E5%BD%B1%E4%BB%AA&enc=utf-8&suggest=6.def.0&wq=t&pvid=meajx9mi.ovrjdb"
//	//L"https://list.tmall.com/search_product.htm?q=%E6%99%BA%E8%83%BD%E6%89%8B%E6%9C%BA&click_id=%E6%99%BA%E8%83%BD%E6%89%8B%E6%9C%BA&from=mallfp..pc_1.1_hq&spm=875.7931836.a1z5h.2.1ZdM3E"
//	//L"http://www.xheike.com/forum.php?mod=forumdisplay&fid=50"; //L"http://www.xheike.com/forum.php?mod=post&amp;action=newthread&amp;fid=84"; //L"http://www.newsmth.net/nForum/#!board/NewSoftware"
//	//L"e:/test.htm"
//	// TODO: Add extra initialization here
//	CWnd *pWnd = GetDlgItem(IDC_TEST_URL); 
//
//	if( m_sURL.GetLength() == 0 )
//	{
//		m_sURL = DEFAULT_BROWSER_URL; 
//	}
//
//	if( pWnd )
//	{	
//		pWnd->SetWindowText( m_sURL.GetBuffer() );
//	}
//
//	browser_safe_navigate_start( &m_WebBrowser, m_sURL.GetBuffer() ); 

	//m_WebBrowser.Navigate( m_sURL.GetBuffer(), 
	//	NULL, 
	//	NULL, 
	//	NULL, 
	//	NULL ); 

	//ret = start_test_web_browser_service(); 
	//if( ERROR_SUCCESS != ret )
	//{
	//	//break; 
	//}

	ret = load_all_plugins(); 
	if( ret != ERROR_SUCCESS )
	{

	}

	init_hook_config(); 
	ret = set_hook( HOOK_INDEX_MOUSE ); 
	if( ret != ERROR_SUCCESS )
	{

	} 

#define CHECK_WEB_PAGE_LOADING_TIMER_ID 1002
#define CHECK_WEB_PAGE_LOADING_ELAPSE 5000

	SetTimer( CHECK_WEB_PAGE_LOADING_TIMER_ID, CHECK_WEB_PAGE_LOADING_ELAPSE, NULL ); 

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void html_script_config_dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		//CAboutDlg dlgAbout;
		//dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void html_script_config_dlg::OnPaint() 
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
HCURSOR html_script_config_dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BEGIN_EVENTSINK_MAP(html_script_config_dlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CBrowser_ControlDlg)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 106 /* DownloadBegin */, OnDownloadBeginExplorer1, VTS_NONE)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 104 /* DownloadComplete */, OnDownloadCompleteExplorer1, VTS_NONE)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 102 /* StatusTextChange */, OnStatusTextChangeExplorer1, VTS_BSTR)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 259 /* DocumentComplete */, OnDocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 256, html_script_config_dlg::OnWebBrowserMenuBar, VTS_BOOL)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_NEWWINDOW2, html_script_config_dlg::WebBrowserNewWindow2, VTS_PDISPATCH VTS_PBOOL)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_NEWWINDOW3, html_script_config_dlg::WebBrowserNewWindow3, VTS_PDISPATCH VTS_PBOOL)
	//ON_EVENT(browser_test_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_FRAMEBEFORENAVIGATE, browser_test_dlg::OnWebBrowserMenuBar, VTS_BOOL)
	//ON_EVENT(browser_test_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_FRAMENAVIGATECOMPLETE, browser_test_dlg::WebBrowserNewWindow2, VTS_PDISPATCH VTS_PBOOL)
	//ON_EVENT(browser_test_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_FRAMENEWWINDOW, browser_test_dlg::WebBrowserNewWindow3, VTS_PDISPATCH VTS_PBOOL)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 258, html_script_config_dlg::OnWebBrowserFullScreen, VTS_BOOL)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 290, html_script_config_dlg::WebBrowserBeforeScriptExecute, VTS_DISPATCH)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, DISPID_BEFORENAVIGATE2, html_script_config_dlg::WebBrowserBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(html_script_config_dlg, IDC_WEB_BROWSER_CONTROL, 271, html_script_config_dlg::NavigateErrorExplorer1, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

#include "update_dlg.h"
void html_script_config_dlg::OnOK() 
{
	// TODO: Add extra validation here
	
	UpdateData(TRUE); 

	//update_dlg dlg; 

	//dlg.DoModal(); 

	//data_dlg.set_page_contents( &scramble_info.page_content ); 
	//data_dlg.ShowWindow( SW_SHOW ); 

	//RECT client_rect; 
	//GetClientRect( &client_rect ); 
	//dlg.MoveWindow( &client_rect, TRUE ); 
	//dlg.ShowWindow( SW_SHOW ); 

	//m_WebBrowser.Navigate(m_sURL , NULL, NULL, NULL, NULL); 

	CString	url;
	
    locate_url_manually = TRUE; 
    GetDlgItemText( IDC_TEST_URL, url ); 
	browser_safe_navigate_start( &m_WebBrowser, url.GetBuffer() ); 

	//OR you can also do the following
 	// CString str;
 	// m_eURL.GetWindowText(str);
	// m_WebBrowser.Navigate(m_sURL , NULL, NULL, NULL, NULL);
}

#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)
void html_script_config_dlg::OnBack() 
{
	try
	{
		m_WebBrowser.on_web_page_locate_begin(); 
		m_WebBrowser.GoBack(); 
	}
	catch(CException* e)
	{		
		dbg_print( MSG_ERROR, "Warning: Uncaught exception %s\n",
			__FUNCTION__ );
		DELETE_EXCEPTION(e);
	}
}

void html_script_config_dlg::OnForward() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.on_web_page_locate_begin(); 
	try
	{
		m_WebBrowser.GoForward();
	}
	catch (CException* e)
	{
		UINT help_conext; 
		CString error_msg; 

		do 
		{
			if( e == NULL )
			{
				break; 
			}

			e->GetErrorMessage( error_msg.GetBufferSetLength( MAX_PATH ), MAX_PATH, &help_conext ); 
			dbg_print_w( MSG_ERROR, L"ie forward error:%s\n", error_msg.GetBuffer() ); 
		} while (FALSE );

	}
}

void html_script_config_dlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	//m_WebBrowser.switch_navigate( TRUE ); 
	m_WebBrowser.Stop();
}


void html_script_config_dlg::OnRefresh() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.Refresh();
}

void html_script_config_dlg::OnHome() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoHome(); 
}

//LRESULT WINAPI format_output_text( wstring &text )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//
//	while (TRUE)
//	{
//		size_t pos = text.find(L"\r\n");
//		if (pos != std::wstring::npos )
//		{
//			text.replace(pos, 2, L"|", 0);
//			//text.insert(pos, strDest);
//		}
//		else
//		{
//			break;
//		}
//	}
//
//	return ret; 
//}

LRESULT html_script_config_dlg::locate_to_url(LPCWSTR url)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        ASSERT(NULL != url); 
        SetDlgItemText(IDC_TEST_URL, url);
        browser_safe_navigate_start(&m_WebBrowser, url); 
    } while (FALSE);

    return ret;
}

LRESULT html_script_config_dlg::on_locate_to_url( WPARAM wparam, LPARAM lparam )
{
	LRESULT ret = ERROR_SUCCESS; 
	LPCWSTR url; 

	do 
	{
		url = ( LPCWSTR )lparam; 
		//SetDlgItemText( IDC_TEST_URL, url ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT html_script_config_dlg::OnWorking( WPARAM wparam, 
							LPARAM lparam) 
{
	// TODO: Add your control notification handler code here
	
	switch( lparam )
	{
	case 0:
		{
			HRESULT hr; 
			wstring url; 
			test_scramble_info.links.clear(); 
			test_scramble_info.next_link.clear(); 

			html_scramble( &m_WebBrowser, &test_scramble_info ); 

			//for( ; ; )
			{
				if( test_scramble_info.links.size() == 0)
				{
					break; 
				}

				url = *test_scramble_info.links.begin(); 
				test_scramble_info.links.erase( test_scramble_info.links.begin() ); 

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
			HRESULT hr; 
			wstring url; 

			hr = scramble_web_page_content( &m_WebBrowser, 
				&test_scramble_info.page_content ); 
		
			//format_output_text(text); 

			//data_dlg.add_scrambled_data_info( &scramble_info.page_content ); 

			//SetDlgItemTextW( IDC_STATIC_SCRAMBLE_DUMP, text.c_str() ); 

			if( test_scramble_info.links.size() == 0 )
			{
				if( test_scramble_info.next_link.size() > 0)
				{
					url = *test_scramble_info.next_link.begin(); 
					test_scramble_info.next_link.erase( test_scramble_info.next_link.begin() ); 

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
				if( test_scramble_info.links.size() == 0)
				{
					break; 
				}

				url = *test_scramble_info.links.begin(); 
				test_scramble_info.links.erase( test_scramble_info.links.begin() ); 

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
void html_script_config_dlg::OnSearch() 
{
	// TODO: Add your control notification handler code here
	m_WebBrowser.GoSearch();
}

void html_script_config_dlg::OnDownloadBeginExplorer1() 
{
	// TODO: Add your control notification handler code here
	
	m_Animate.Play(0,-1,-1);

}

void html_script_config_dlg::OnDownloadCompleteExplorer1() 
{
	// TODO: Add your control notification handler code here
 	m_Animate.Stop();
	m_Animate.Seek(0);
 
}

void html_script_config_dlg::OnStatusTextChangeExplorer1(LPCTSTR Text) 
{
	// TODO: Add your control notification handler code here
		if (GetSafeHwnd())
	{
		CWnd *pWnd = GetDlgItem(IDC_STATUS_TEXT);
		if (pWnd)
			pWnd->SetWindowText(Text);
	}


}

 
void html_script_config_dlg::OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	// TODO: Add your control notification handler code here
	LRESULT ret; 
	HRESULT hr; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 

	do 
	{
		//dbg_print( MSG_IMPORTANT, "the html document loaded\n" ); 

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

		//dump_html_elements_on_page( html_doc ); 
		//disable_web_page_anchor( html_doc ); 
		{
			ULONG cookie; 
			set_event_handler_on_doc( html_doc, element_behavior, &cookie ); 
		}

		//test_highlight_html_elements(); 

		ret = set_host_ui_handler_on_doc( html_doc, ui_handler ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

	}while( FALSE ); 

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
				//SetDlgItemText( IDC_TEST_URL, ( L"" ) ); 
				break; 
			}

			_text = text.operator const wchar_t*(); 
			ASSERT( NULL != _text ); 

			//SetDlgItemText( IDC_TEST_URL, _text ); 

			url_loading = m_WebBrowser.get_loading_url(); 

			dbg_print_w( MSG_IMPORTANT, L"translate url is %s\n", ui_handler->get_translate_url().c_str() ); 

			if( url_loading.length() == 0 )
			{
				break; 
			}

			if( 0 == wcscmp( url_loading.c_str(), 
				_text ) )
			{
				m_WebBrowser.notify_status_event( TRUE ); 
				m_WebBrowser.close_navigate_function(); 

                if (locate_url_manually == TRUE)
                {
                    html_process_dlg.on_html_document_complete(url_loading.c_str());
                    locate_url_manually = FALSE; 
                }

				//browser_safe_navigate_start( &m_WebBrowser, url_loading.c_str() ); 
				
				if( NULL != html_doc )
				{
					scroll_expand_html_doc( html_doc ); 
				}
			}
		}while( FALSE ); 
	}

	UpdateData( FALSE ); 
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

		xml_node = xml_doc->createElement( L"page" ); 

		if( NULL == xml_node )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		site_node->raw_appendChild( xml_node, NULL );

		temp_text = url; 

		xml_node->setAttribute( L"url", temp_text);

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

HRESULT html_script_config_dlg::highlight_html_element_show( LPCWSTR xpath )
{
	HRESULT hr = S_OK; 
	CWnd *ctrl_wnd; 

	do
	{
		UpdateData(TRUE); 

		//hilight_xpath = xpath; 
		//hilight_url = m_sURL.GetBuffer(); 

		//ctrl_wnd = GetDlgItem( IDC_WEB_BROWSER_CONTROL ); 
		//
		//if( ctrl_wnd == NULL )
		//{
		//	ASSERT( FALSE ); 
		//	break; 
		//}

		////ctrl_wnd->ShowWindow( SW_HIDE ); 

		//_start_webbrowser( 0 ); 
	}while( FALSE ); 

	return hr; 
}

LRESULT html_script_config_dlg::test_highlight_html_elements()
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr elements; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	//ULONG element_count; 
	LONG _sub_element_count; 
	_bstr_t temp_text; 
	_variant_t name; 
	_variant_t index; 
	INT32 i; 
	INT32 find_count; 

	BOOLEAN sub_element_got = FALSE; 

	do 
	{
		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_all( &elements ); 
		if( FAILED( hr ) ) 
		{
			break; 
		}

		if( NULL == elements )
		{
			break; 
		}

		temp_text = HTML_HIGHLIGT_ELEMENT_ID; 

		find_count = 0; 

		for( ; ; )
		{

			V_VT( &index ) = VT_I4;
			V_I4( &index ) = 0; 

			V_VT( &name ) = VT_I4;
			V_I4( &name ) = find_count; 

			find_count += 1; 

			_hr = elements->item( name, index, &sub_element_disp ); 
			if( FAILED( _hr ) )
			{
				break; 
			}

			if( NULL == sub_element_disp )
			{
				//_hr = E_UNEXPECTED; 
				break; 
			}

			do 
			{
				hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
				if( FAILED( hr )
					|| sub_elements == NULL )
				{
					break; 
				}

				sub_element_got = TRUE; 

				hr = sub_elements->get_length( &_sub_element_count ); 
				if( FAILED( hr ) ) 
				{
					break; 
				}

				for( i = 0; i < _sub_element_count; i ++ )
				{
					do 
					{
						V_VT( &name ) = VT_I4;
						V_I4( &name ) = i; 

						//comment it.
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( name, index, &sub_element_disp ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						if( sub_element_disp == NULL )
						{
							hr = E_UNEXPECTED; 
							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( SUCCEEDED( hr )
							&& sub_element != NULL )
						{
							add_html_element_behevior( sub_element, element_behavior ); 
						}
					}while( FALSE ); 
				}
			} while ( FALSE );

			if( TRUE == sub_element_got )
			{
				break; 
			}

			hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( sub_element == NULL )
			{
				hr = E_UNEXPECTED; 
				break; 
			}

			{	
				add_html_element_behevior( sub_element, element_behavior ); 
			}
		}
	}while( FALSE ); 

	return hr; 
}

HRESULT WINAPI highlight_html_element( IHTMLElement *element )
{
	HRESULT hr = S_OK; 
	_bstr_t name; 
	_bstr_t _value; 
	_variant_t value; 
	ULONG color; 

	ASSERT( NULL != element ); 

	color = HIGHT_BG_COLOR; 
	
	do 
	{
		name = L"style"; 
		//value.vt = VT_UI4; 
		//value.uintVal = color; 

		_value = "\"background-color:#9fe6ac\""; 

		value = _value.GetBSTR(); 

		hr = element->setAttribute( name.GetBSTR(), value.GetVARIANT() ); 

	}while( FALSE );

	return hr; 
}

LRESULT html_script_config_dlg::unhighlight_html_element()
{
	HRESULT hr = S_OK; 
	HRESULT _hr; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementCollectionPtr elements; 
	IDispatchPtr disp; 
	IDispatchPtr sub_element_disp; 
	IHTMLElementCollectionPtr sub_elements; 
	IHTMLElementPtr sub_element; 
	//ULONG element_count; 
	LONG _sub_element_count; 
	_bstr_t temp_text; 
	_variant_t name; 
	_variant_t index; 
	INT32 i; 
	INT32 find_count; 

	BOOLEAN sub_element_got = FALSE; 

	do 
	{
		break; 

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&html_doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_all( &elements ); 
		if( FAILED( hr ) ) 
		{
			break; 
		}

		if( NULL == elements )
		{
			break; 
		}

		//hr = elements->get_length( &element_count ); 
		//if( FAILED( hr ) )
		//{
		//	break; 
		//}

		temp_text = HTML_HIGHLIGT_ELEMENT_ID; 

		name = temp_text.GetBSTR(); 

		find_count = 0; 

		for( ; ; )
		{
			
			V_VT( &index ) = VT_I4;
			V_I4( &index ) = find_count; 

			find_count += 1; 

			_hr = elements->item( name, index, &sub_element_disp ); 
			if( FAILED( _hr ) )
			{
				break; 
			}

			if( NULL == sub_element_disp )
			{
				//_hr = E_UNEXPECTED; 
				break; 
			}

			do 
			{
				hr = sub_element_disp->QueryInterface( IID_IHTMLElementCollection, ( PVOID* )&sub_elements ); 
				if( FAILED( hr )
					|| sub_elements == NULL )
				{
					break; 
				}

				sub_element_got = TRUE; 

				hr = sub_elements->get_length( &_sub_element_count ); 
				if( FAILED( hr ) ) 
				{
					break; 
				}

				for( i = 0; i < _sub_element_count; i ++ )
				{
					do 
					{
						V_VT( &name ) = VT_I4;
						V_I4( &name ) = i; 

						//comment it.
						V_VT( &index ) = VT_I4;
						V_I4( &index ) = 0; 

						hr = sub_elements->item( name, index, &sub_element_disp ); 
						if( FAILED( hr ) )
						{
							break; 
						}

						if( sub_element_disp == NULL )
						{
							hr = E_UNEXPECTED; 
							break; 
						}

						hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
						if( SUCCEEDED( hr )
							&& sub_element != NULL )
						{
							{
								LPWSTR _temp_text; 
								hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
								if( FAILED(hr))
								{
									break; 
								}

								_temp_text = temp_text.operator wchar_t*(); 
								if( NULL == _temp_text )
								{
									hr = E_UNEXPECTED; 
									break; 
								}

								hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
								if( FAILED(hr))
								{
									break; 
								}
							}
						}
					}while( FALSE ); 
				}
			} while ( FALSE );

			if( TRUE == sub_element_got )
			{
				break; 
			}

			hr = sub_element_disp->QueryInterface( IID_IHTMLElement, ( PVOID* )&sub_element ); 
			if( FAILED( hr ) )
			{
				break; 
			}

			if( sub_element == NULL )
			{
				hr = E_UNEXPECTED; 
				break; 
			}

			{
				LPWSTR _temp_text; 
				hr = sub_element->get_innerHTML( temp_text.GetAddress() ); 
				if( FAILED(hr))
				{
					break; 
				}

				_temp_text = temp_text.operator wchar_t*(); 
				if( NULL == _temp_text )
				{
					hr = E_UNEXPECTED; 
					break; 
				}

				hr = sub_element->put_outerHTML( temp_text.GetBSTR() ); 
				if( FAILED(hr))
				{
					break; 
				}
			}
		}
	}while( FALSE ); 

	return hr; 
}

HRESULT html_script_config_dlg::get_html_element_from_xpath( LPCWSTR xpath, 
													  HTML_ELEMENT_GROUP *group )
{
	HRESULT hr = S_OK; 
	LRESULT ret; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr doc; 

	do 
	{
		ASSERT( NULL != group ); 

		disp = m_WebBrowser.GetDocument(); 
		if( NULL == disp )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, 
			( PVOID* )&doc ); 

		if( FAILED(hr))
		{
			break; 
		}

		if( NULL == doc )
		{
			hr = E_UNEXPECTED; 
			break; 
		}

		ret = get_html_elements_from_xpath( xpath, 
			doc, 
			group, 
			FALSE ); 
		if( ret != ERROR_SUCCESS )
		{
			hr = E_UNEXPECTED; 
			break; 
		}
	} while ( FALSE );

	return hr; 
}

LRESULT html_script_config_dlg::highlight_html_element_by_xpath( LPCWSTR xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	IHTMLDocument2Ptr doc; 
	IDispatchPtr disp; 
    HTML_ELEMENT_GROUP elements; 

	ASSERT( NULL != xpath ); 

	UpdateData(TRUE); 

	do 
	{
		wstring url; 
		//GetDlgItemText( IDC_TEST_URL, url ); 
		
        ret = get_web_browser_location_url(&m_WebBrowser,
            url);

        if (ERROR_SUCCESS != ret)
        {
            //break;
        }

		hilight_xpath = xpath; 
		hilight_url = url.c_str(); 

        ret = get_html_element_from_xpath(xpath, &elements); 
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ret = select_html_elements(&elements); 
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

    }while( FALSE );

    release_html_element_group(&elements); 
	return ret; 
}

//LRESULT browser_test_dlg::get_html_element_by_xpath( LPCWSTR xpath )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//	IHTMLDocument2Ptr doc; 
//	IDispatchPtr disp; 
//	HTML_ELEMENT_GROUP group; 
//
//	ASSERT( NULL != xpath ); 
//
//	do 
//	{
//		break; 
//
//		disp = m_WebBrowser.GetDocument(); 
//		if( NULL == disp )
//		{
//			break; 
//		}
//
//		hr = disp->QueryInterface( IID_IHTMLDocument2, 
//			( PVOID* )&doc ); 
//
//		if( FAILED(hr))
//		{
//			break; 
//		}
//
//		if( NULL == doc )
//		{
//			break; 
//		}
//
//		ret = get_html_elements_from_xpath( xpath, 
//			doc, 
//			&group ); 
//
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		_bstr_t html_text; 
//		LPCWSTR text; 
//		wstring _text; 
//
//		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
//			it != group.end(); 
//			it ++ )
//		{
//			do 
//			{
//				hr = ( *it )->get_outerHTML( html_text.GetAddress() ); 
//				if( FAILED(hr))
//				{
//					break; 
//				} 
//
//				text = html_text.operator wchar_t*(); 
//				if( NULL == text )
//				{
//					break; 
//				}
//
//				_text = L"<span id=\"" HTML_HIGHLIGT_ELEMENT_ID L"\" style=\"background-color:#9fe6ac\">"; 
//				_text += text; 
//				_text += L"</span>"; 
//
//				html_text = _text.c_str(); 
//
//				//html_text = L""; 
//
//				hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
//				if( FAILED(hr))
//				{
//					break; 
//				} 
//
//				//hr = highlight_html_element( ( *it) ); 
//
//			} while ( FALSE );
//		}
//
//		Sleep( 2000 ); 
//
//		for( HTML_ELEMENT_GROUP_ELEMENT_ITERATOR it = group.begin(); 
//			it != group.end(); 
//			it ++ )
//		{
//			do 
//			{
//				hr = ( *it )->get_innerText( html_text.GetAddress() ); 
//				if( FAILED(hr))
//				{
//					break; 
//				} 
//
//				text = html_text.operator wchar_t*(); 
//				if( NULL == text )
//				{
//					break; 
//				}
//
//				//html_text = L""; 
//
//				hr = ( *it )->put_outerHTML( html_text.GetBSTR() ); 
//				if( FAILED(hr))
//				{
//					break; 
//				} 
//
//				//hr = highlight_html_element( ( *it) ); 
//
//			} while ( FALSE );
//		}
//
//	}while( FALSE );
//
//	return ret; 
//}

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

		ret = insert_node_by_text(xml_src_doc, dest_node);
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

		element_type = get_html_element_type_from_tag( _text ); 

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

					if( 0 != _wcsicmp( _text, L"checkbox" ) )
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

			if( 0 == _wcsicmp( _text, L"iframe" ) 
				|| 0 == _wcsicmp( _text, L"frame" ) )
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

				ret = html_window_2_html_document( html_window, &html_doc ); 
				if( ERROR_SUCCESS != ret )
				{
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

void html_script_config_dlg::OnBnClickedButtonRecordSelected()
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

void html_script_config_dlg::OnWebBrowserMenuBar(BOOL MenuBar)
{
	// TODO: Add your message handler code here
}

void html_script_config_dlg::WebBrowserNewWindow3( LPDISPATCH* ppDisp, 
											BOOL *Cancel, 
											VARIANT* Flags, 
											VARIANT *UrlContext , 
											VARIANT *Url ) 
{

}

//HRESULT hr;
//CComPtr<IWebBrowser2> pWebBrowser2;
//CComPtr<IHTMLWindow2> spWnd2;
//CComPtr<IServiceProvider>spServiceProv;
//hr=m_spHtmlDoc->get_parentWindow ((IHTMLWindow2**)&spWnd2);   
//if(SUCCEEDED(hr))
//{
//	hr=spWnd2->QueryInterface (IID_IServiceProvider,(void**)&spServiceProv);
//	if(SUCCEEDED(hr))
//	{
//		hr = spServiceProv->QueryService(SID_SWebBrowserApp,IID_IWebBrowser2,(void**)&pWebBrowser2);
//		if(SUCCEEDED(hr))
//		{
//			return pWebBrowser2;   
//		}   
//	}
//}

void html_script_config_dlg::WebBrowserNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	// TODO: Add your message handler code here
	LRESULT _ret; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementPtr html_element; 
	IHTMLElementPtr _html_element; 
	wstring xpath; 
	HRESULT hr; 
	// TODO: Add your message handler code here

	*Cancel = TRUE; 

	do 
	{
		//unhighlight_html_element(); 

		do 
		{
			HRESULT hr  = -1;

			if( NULL == ppDisp )
			{
				ASSERT( FALSE ); 
				break; 
			}

			disp = m_WebBrowser.GetControlUnknown(); 

			if( NULL == disp )
			{
				ASSERT( FALSE ); 
				break; 
			}


			hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
			if( FAILED( hr ))
			{
				break; 
			}

			if( NULL == web_browser )
			{
				break; 
			}

			hr = web_browser->get_Application( &disp ); 
			if( FAILED(hr) )
			{
				break; 
			}

			if( NULL == disp )
			{
				break; 
			}

			*ppDisp  = disp;
		} while ( FALSE );

		//if( m_WebBrowser.can_navigate() != FALSE )
		{
			dbg_print_w( MSG_IMPORTANT, L"locate to translate url %s on new window event\n", ui_handler->get_translate_url().c_str() ); 
			_ret = browser_safe_navigate_start( &m_WebBrowser, 
				ui_handler->get_translate_url().c_str() ); 
		}
#if 0
		disp = m_WebBrowser.GetDocument(); 
		//hr = web_browser->get_Document( &disp ); 
		//if( FAILED( hr ))
		//{
		//	break; 
		//}

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
		if( FAILED( hr ))
		{
			break; 
		}

		//html_doc->get_parentWindow( )
		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == _html_element )
		{
			break; 
		}

		_ret = get_active_element_xpath_ex( _html_element, &xpath, &html_element ); 

		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

#ifdef CONFIG_DLG_HAVE_XPATH_DLG
		if( xpath_dlg.get_xpath1().GetLength() != 0 )
		{
			xpath_dlg.set_xpath2( CString( xpath.c_str() ) ); 
		}
		else
		{
			xpath_dlg.set_xpath1( CString( xpath.c_str() ) ); 
		}
#endif //CONFIG_DLG_HAVE_XPATH_DLG

		//xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
		//xpath_dlg.ShowWindow( SW_SHOW ); 
		//prop_dlg.set_html_element( html_element ); 
		//prop_dlg.set_html_element_xpath( xpath.c_str() ); 

		//prop_dlg.init_element_properties(); 

		//prop_dlg.ShowWindow( SW_SHOW ); 

		//sramble_list_dlg.on_html_element_active( ( PVOID )html_element, xpath.c_str() ); 

		//MessageBoxW( xpath.c_str(), NULL, 0 ); 
#endif //0
	}while( FALSE );

	//if( m_WebBrowser.can_navigate() == FALSE )
	//{
	//	*Cancel = TRUE; 
	//}
}

void html_script_config_dlg::OnWebBrowserFullScreen(BOOL FullScreen)
{
	// TODO: Add your message handler code here
}

void html_script_config_dlg::WebBrowserBeforeScriptExecute(LPDISPATCH pDispWindow)
{
	// TODO: Add your message handler code here
}

void html_script_config_dlg::WebBrowserBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel)
{
	LRESULT _ret; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementPtr _html_element; 
	IHTMLElementPtr html_element; 
	wstring xpath; 
	HRESULT hr; 
	// TODO: Add your message handler code here

#define beforeNavigateExternalFrameTarget 0x0001
	if( Flags->vt == VT_I4 
		&&  Flags->intVal & beforeNavigateExternalFrameTarget )
	{

	}

	do 
	{
		//unhighlight_html_element(); 

		if( NULL == pDisp )
		{
			ASSERT( FALSE ); 
			break; 
		}

		hr = pDisp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == web_browser )
		{
			break; 
		}

		hr = web_browser->get_Document( &disp ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == html_doc )
		{
			break; 
		}

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ))
		{
			break; 
		}

		if( NULL == _html_element )
		{
			break; 
		}

		_ret = get_active_element_xpath_ex( _html_element, &xpath, NULL, &html_element ); 
	
		if( _ret != ERROR_SUCCESS )
		{
			break; 
		}

#ifdef CONFIG_DLG_HAVE_XPATH_DLG
		if( xpath_dlg.get_xpath1().GetLength() != 0 )
		{
			xpath_dlg.set_xpath2( CString( xpath.c_str() ) ); 
		}
		else
		{
			xpath_dlg.set_xpath1( CString( xpath.c_str() ) ); 
		}
#endif //CONFIG_DLG_HAVE_XPATH_DLG

		//xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
		//xpath_dlg.ShowWindow( SW_SHOW ); 

		//prop_dlg.set_html_element( html_element ); 
		//prop_dlg.set_html_element_xpath( xpath.c_str() ); 
		//prop_dlg.init_element_properties(); 
		//prop_dlg.ShowWindow( SW_SHOW ); 

		//sramble_list_dlg.on_html_element_active( ( PVOID )html_element, xpath.c_str() ); 

		//MessageBoxW( xpath.c_str(), NULL, 0 ); 
	}while( FALSE );

	if( m_WebBrowser.can_navigate() == FALSE )
	{
		*Cancel = TRUE; 
	}
	else
	{
		//m_WebBrowser.set_loading_url( URL ); 
	}
}

void html_script_config_dlg::OnBnClickedButtonRecordPage()
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

void html_script_config_dlg::OnBnClickedButtonNewPageGroup()
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

			//ret = open_file_dlg( FALSE, L"xml", file_name, NULL, app_path, L"*.xml\0*.xml\0\0", NULL ); 
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

HRESULT html_script_config_dlg::show_active_element_xpath()
{
	HRESULT hr; 
	//html_element_prop_dlg dlg; 
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


		//dump_html_elements_on_page( html_doc ); 
		//disable_web_page_anchor( html_doc ); 

		//wstring xpath; 
		//get_active_element_xpath( html_doc, &xpath ); 

		//dlg.set_html_element_xpath( xpath.c_str() ); 
		//dlg.DoModal(); 
	} while ( FALSE );

	return hr; 
}

void html_script_config_dlg::OnBnClickedButtonScramble()
{
    LRESULT ret; 
	HRESULT hr; 
	wstring text; 
	_bstr_t _text; 
	wstring text_name; 
	IHTMLElementPtr html_element; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	wstring url; 

	do 
	{
		//GetDlgItemText( IDC_TEST_URL, url ); 

        ret = get_web_browser_location_url(&m_WebBrowser,
            url);

        if (ERROR_SUCCESS != ret)
        {
            break; 
        }

		test_scramble_info.url = url.c_str(); 
		hr = html_scramble_ex( &m_WebBrowser, test_scramble_info.url.c_str() ); 
		if( FAILED(hr))
		{
			//break; 
		}

	}while( FALSE );

	// TODO: Add your control notification handler code here
}

void html_script_config_dlg::NavigateErrorExplorer1(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Frame, VARIANT* StatusCode, BOOL* Cancel)
{
	// TODO: Add your message handler code here
}

/************************************************************************
plan:加入对选中的多个元素的同时分析，加入功能
*************************************************************************/
LRESULT html_script_config_dlg::on_html_page_clicked(ULONG flags, CPoint point)
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 __ret; 
	IWebBrowser2Ptr web_browser; 
	IDispatchPtr disp; 
	IHTMLDocument2Ptr html_doc; 
	IHTMLElementPtr _html_element; 
	IHTMLElementPtr html_element; 
    HTML_ELEMENT_GROUP elements;

	wstring xpath; 
	wstring std_xpath; 
	HRESULT hr; 
	POINT pos; 

    static IHTMLElement *old_element = NULL;

	do 
	{
		//ret = GetCursorPos(&pt); 
		//if( FALSE == ret )
		//{
		//	break; 
		//}

		//ClientToScreen( &pt ); 
		//m_WebBrowser.ScreenToClient( &pt ); 

		// TODO: Add your message handler code here

		//unhighlight_html_element(); 

        //ASSERT(NULL != action); 

        //*action = 0; 
		
        disp = m_WebBrowser.GetDocument(); 
		//hr = web_browser->get_Document( &disp ); 
		//if( FAILED( hr ))
		//{
		//	break; 
		//}

		if( NULL == disp )
		{
			break; 
		}

		hr = disp->QueryInterface( IID_IHTMLDocument2, ( PVOID* )&html_doc ); 
		if( FAILED( hr ))
		{
			break; 
		}

		//html_doc->get_parentWindow( )
		if( NULL == html_doc )
		{
			break; 
		}

		//do
		//{
		//	HWND web_browser_wnd; 
		//	SHANDLE_PTR _web_browser_wnd; 
		//	IDispatchPtr disp; 
		//	IWebBrowser2Ptr web_browser; 
		//	
		//	//web_browser_wnd = ( HWND )m_WebBrowser.GetHwnd(); 
		//	//if( NULL == web_browser_wnd )
		//	//{
		//	//	hr = E_UNEXPECTED; 
		//	//	break; 
		//	//}

		//	//disp = m_WebBrowser.GetControlUnknown(); 

		//	//if( NULL == disp )
		//	//{
		//	//	ASSERT( FALSE ); 
		//	//	break; 
		//	//}


		//	//hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
		//	//if( FAILED( hr ))
		//	//{
		//	//	break; 
		//	//}

		//	//hr = web_browser->ClientToWindow( &_web_browser_wnd ); 
		//	//if( FAILED(hr))
		//	//{
		//	//	break; 
		//	//}

		//	//if( NULL == web_browser_wnd )
		//	//{
		//	//	break; 
		//	//}


		pos.x = point.x; 
		pos.y = point.y; 

			//__ret = GetCursorPos( &pos ); 
			//if( FALSE == __ret )
			//{
			//	hr = E_UNEXPECTED; 
		//	break; 
		//}

        //if (message == WM_RBUTTONUP 
        //    || message == WM_RBUTTONDOWN)
        //{
        //    *action = HTML_PAGE_BUTTON_CLICK_ACTION_CINTINUE; 
        //    break; 
        //}
        //else if (message != WM_LBUTTONUP 
        //    && message != WM_LBUTTONDOWN)
        //{
        //    break; 
        //}

		{
			CWnd *browser_wnd; 

			browser_wnd = GetDlgItem( IDC_WEB_BROWSER_CONTROL ); 

			ASSERT( NULL != browser_wnd ); 

			browser_wnd->ScreenToClient( &pos ); 
		}

		_html_element = NULL; 
		//<a><img></img></a>使用POINT的方式会选中img
		//}while( FALSE ); 

		hr = html_doc->get_activeElement( &_html_element ); 
		if( FAILED( hr ))
		{
			break; 
		}

		{
			BOOLEAN valid_element; 
			hr = is_effective_html_element( _html_element, &valid_element ); 
			if( FAILED(hr)) 
			{

			}

			//if( FALSE == valid_element )
			//ret = ERROR_SUCCESS; 
			do
			{
				hr = get_html_element_from_point( html_doc, &pos, &_html_element); 
				if( FAILED(hr))
				{
					ret = hr; 
					break; 
				}

				if( NULL == _html_element )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

                if (old_element == _html_element.GetInterfacePtr())
                {
                    ret = ERROR_ALREADY_REGISTERED;
                    break; 
                }

                old_element = _html_element; 

				{
					RECT rect; 
				
	/*				{
						LRESULT ret; 
						IHTMLFrameBase2Ptr frame; 
						IHTMLWindow2Ptr window; 
						html_doc->get_parentWindow( &window ); 

						ret = html_window_2_html_frame( html_doc, window, &frame ); 
					}*/

					ret = get_html_element_position( html_doc, _html_element, &rect ); 
					if( ret != ERROR_SUCCESS )
					{
						//break; 
					}
					else
					{
						//y scrolled, and top is greater 1 than real value.
						ASSERT( PtInRect( &rect, pos ) == TRUE ); 
						dbg_print_w( MSG_IMPORTANT, L"html element rect(left:%d top:%d right:%d bottom:%d)\ncursor position(x:%d y:%d)\n", 
							rect.left, 
							rect.top, 
							rect.right, 
							rect.bottom, 
							pos.x, 
							pos.y ); 
					}
				}

				ret = get_html_element_xpath_from_point( html_doc, &pos, &point, _html_element, &xpath, &std_xpath, &html_element ); 
			}while( FALSE ); 
			
			if( ret != ERROR_SUCCESS )
			{
//#ifdef _DEBUG
//				wstring _xpath; 
//				ret = get_active_element_xpath_ex( _html_element, &_xpath, &std_xpath, &html_element ); 
//				//if( 0 != wcscmp( xpath.c_str(), _xpath.c_str() ) )
//				//{
//				//	dbg_print_w( MSG_IMPORTANT, L"xpath %s!=%s\n", _xpath.c_str(), xpath.c_str() ); 
//				//}
//#endif //_DEBUG
			}
		}

//#ifdef _DEBUG
//		{
//			HTML_ELEMENT_GROUP group; 
//			disp = m_WebBrowser.GetControlUnknown(); 
//
//			if( NULL == disp )
//			{
//				ASSERT( FALSE ); 
//				break; 
//			}
//
//			hr = disp->QueryInterface( IID_IWebBrowser2, ( PVOID*)&web_browser ); 
//			if( FAILED( hr ))
//			{
//				break; 
//			}
//
//			if( NULL == web_browser )
//			{
//				break; 
//			}
//
//			ret = get_html_element_from_xpath_ex( xpath.c_str(), &group, web_browser, FALSE ); 
//
//			release_html_element_group( &group ); 
//
//			//add_html_element_behevior( _html_element, element_behavior ); 
//		}
//#endif //_DEBUG

		//hr = get_selected_html_elements( html_doc, &html_element ); 
		//if( FAILED( hr ) )
		//{
		//	break; 
		//}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

        if (html_element.GetInterfacePtr() == NULL)
        {
            //ASSERT(FALSE);
            break;
        }

		//dump_html_element( html_element ); 

		//if( xpath_dlg.get_xpath1().GetLength() != 0 )
		//{
		//	xpath_dlg.set_xpath2( CString( xpath.c_str() ) ); 
		//}
		//else
		//{
		//	xpath_dlg.set_xpath1( CString( xpath.c_str() ) ); 
		//}

		//xpath_dlg.Create( MAKEINTRESOURCE( xpath_dlg.IDD ), this ); 
		//xpath_dlg.ShowWindow( SW_SHOW ); 

        do 
        {
            html_element->AddRef(); 
            elements.push_back(html_element.GetInterfacePtr());

            ret = select_html_elements(&elements);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }
        } while (FALSE);

        release_html_element_group(&elements); 
        elements.clear(); 
        //get_selected_html_element(); 

		html_process_dlg.set_target_html_element_info( html_element, xpath.c_str(), std_xpath.c_str() ); 
		
        html_process_dlg.ShowWindow( SW_SHOW ); 

		//sramble_list_dlg.on_html_element_active( ( PVOID )html_element, xpath.c_str() ); 

		//MessageBoxW( xpath.c_str(), NULL, 0 ); 
	}while( FALSE );

	//CDialog::OnLButtonUp(nFlags, point);
    return ret; 
}

void html_script_config_dlg::OnBnClickedButtonRunScript()
{
	// TODO: 在此加入控制項告知處理常式程式碼
}


void html_script_config_dlg::OnTimer(UINT_PTR nIDEvent)
{
	LRESULT ret; 

	CDialog::OnTimer(nIDEvent);

	ASSERT( CHECK_WEB_PAGE_LOADING_TIMER_ID == nIDEvent ); 

	do
	{
		ret = m_WebBrowser.check_web_page_loading_status(); 
	}while( FALSE ); 

	//KillTimer( nIDEvent ); 
}

#if 0 
LRESULT html_script_config_dlg::exec_html_element_action(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;
    IWebBrowser2Ptr web_browser;
    IDispatchPtr disp;
    IHTMLDocument2Ptr html_doc;
    HRESULT hr;
    HTML_ELEMENT_INFO *element_info = NULL;

    do
    {
        html_doc = m_WebBrowser.GetDocument();
        if (NULL == html_doc)
        {
            ret = ERROR_WEB_PAGE_IS_NOT_LOADED;
            dbg_print(MSG_FATAL_ERROR, "can not load web page, html document is null\n");
            ASSERT(FALSE);
            break;
        }

        disp = m_WebBrowser.GetControlUnknown();

        if (NULL == disp)
        {
            ret = get_html_element_not_found_error(ERROR_ERRORS_ENCOUNTERED);
            ASSERT(FALSE);
            break;
        }

        hr = disp->QueryInterface(IID_IWebBrowser2, (PVOID*)&web_browser);
        if (FAILED(hr))
        {
            ret = get_html_element_not_found_error(ERROR_ERRORS_ENCOUNTERED);
            break;
        }

        if (NULL == web_browser)
        {
            ret = get_html_element_not_found_error(ERROR_ERRORS_ENCOUNTERED);
            break;
        }

        do
        {
            element_info = new HTML_ELEMENT_INFO();
            if (NULL == element_info)
            {
                ret = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            element_info->action_config = *action;

            /*****************************************************************

            *****************************************************************/
            //ret = add_html_element_config_ex(config_xml_doc, 
            //	&config ); 

            //if( ret != ERROR_SUCCESS )
            //{
            //	break; 
            //}

        } while (FALSE);

        do
        {
            if (0 == _wcsicmp(element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK)
                && 0 == _wcsicmp(element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE))
            {
                g_web_browser_window->m_WebBrowser.switch_navigate(TRUE);
            }

            ret = ie_auto_execute_on_element_ex(element_info,
                web_browser);
            if (ret != ERROR_SUCCESS)
            {
                ret = get_html_element_not_found_error(ret);
                break;
            }

            if (0 != element_info->elements.size())
            {
                ret = get_html_element_not_found_error(ERROR_ERRORS_ENCOUNTERED);
            }

            if (0 == _wcsicmp(element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_CLICK)
                && 0 == _wcsicmp(element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE))
            {
                dbg_print_w(MSG_IMPORTANT, L"set location url to %s\n", g_web_browser_window->ui_handler->get_translate_url().c_str());
                g_web_browser_window->m_WebBrowser.set_loading_url(g_web_browser_window->ui_handler->get_translate_url().c_str());
            }

            action->outputs = element_info->action_config.outputs;

#if 0
            do
            {
                if (0 != wcsicmp(element_info->action_config.action.c_str(), HTML_ELEMENT_ACTION_OUTPUT))
                {
                    break;
                }

                if (0 != wcsicmp(element_info->action_config.param.c_str(), HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT))
                {
                    break;
                }
            } while (FALSE);
#endif //0
        } while (FALSE);
    } while (FALSE);

    if (element_info != NULL)
    {
        delete element_info;
    }

    return ret;
}
#endif //_DEBUG