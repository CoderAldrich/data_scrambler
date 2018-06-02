// scramble_list_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "scramble_list_dlg.h"
#include "external_link.h"
#include "html_element_config.h"

#include "html_script_config_dlg.h"
extern html_script_config_dlg *g_html_script_config_dlg; 

HTML_LIST_ELEMENT_SCRAMBLE_INFO list_sramble_config; 
// scramble_list_dlg dialog

IMPLEMENT_DYNAMIC(scramble_list_dlg, CDialog)

scramble_list_dlg::scramble_list_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(scramble_list_dlg::IDD, pParent), 
	scramble_config_step( STEP_LIST_LINKS )
{
	list_sramble_config.link_list_xpath.empty(); 
	list_sramble_config.next_link.empty(); 
	m_pListEdit = NULL;
	//scramble_info.page_content.element_contents.empty(); 
}

scramble_list_dlg::~scramble_list_dlg()
{
}

void scramble_list_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTENT_ELEMENTS, contants_elements);
}


BEGIN_MESSAGE_MAP(scramble_list_dlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CHECK_LIST_LINKS, &scramble_list_dlg::OnBnClickedButtonCheckListLinks)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_NEXT_LINK, &scramble_list_dlg::OnBnClickedButtonCheckNextLink)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_CONTENTS, &scramble_list_dlg::OnBnClickedButtonCheckContents)
	ON_BN_CLICKED(IDC_BUTTON_LIST_LINK_OK, &scramble_list_dlg::OnBnClickedButtonListLinkOk)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_LINK_OK, &scramble_list_dlg::OnBnClickedButtonNextLinkOk)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CONTENT_ELEMENTS, &scramble_list_dlg::OnNMDblclkListContentElements)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CONTENT_ELEMENTS, &scramble_list_dlg::OnNMClickListContentElements)

	ON_BN_CLICKED(IDC_BUTTON_CONTENTS_RETURN, &scramble_list_dlg::OnBnClickedButtonContentsReturn)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_LINK_RETURN, &scramble_list_dlg::OnBnClickedButtonNextLinkReturn)
	ON_BN_CLICKED(IDC_BUTTON_RECORD_LINKS, &scramble_list_dlg::OnBnClickedButtonRecord)

	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST_CONTENT_ELEMENTS, OnBeginlabeleditList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_CONTENT_ELEMENTS, OnEndlabeleditList)
END_MESSAGE_MAP()


// scramble_list_dlg message handlers

void scramble_list_dlg::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
	{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	CString str = pDispInfo->item.pszText;
	int item = pDispInfo->item.iItem;
	int subitem = pDispInfo->item.iSubItem;
	// Construct and create the custom multiline edit control.
	// We could just as well have used a combobox, checkbox, 
	// rich text control, etc.
	m_pListEdit = new CInPlaceEdit( item, subitem, str );
	// Start with a small rectangle.  We'll change it later.
	CRect  rect( 0,0,1,1 );
	DWORD dwStyle = ES_LEFT;
	dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL;
	m_pListEdit->Create( dwStyle, rect, &contants_elements, 103 );
	// Have the Grid position and size the custom edit control
	contants_elements.PositionControl( m_pListEdit, item, subitem );
	// Have the edit box size itself to its content.
	m_pListEdit->CalculateSize();
	// Return TRUE so that the list control will hnadle NOT edit label itself. 
	*pResult = 1;
	}

void scramble_list_dlg::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) 
	{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	int item = pDispInfo->item.iItem;
	int subitem = pDispInfo->item.iSubItem;
	// This is coming from the grid list control notification.
	if( m_pListEdit )
		{
		CString str;
		if( pDispInfo->item.pszText )
			contants_elements.SetItemText( item, subitem, pDispInfo->item.pszText );
		delete m_pListEdit;
		m_pListEdit = 0;
		}
	*pResult = 0;
	}


void scramble_list_dlg::OnBnClickedButtonCheckListLinks()
{
	// TODO: Add your control notification handler code here

	//GetDlgItemText( IDC_EDIT_LIST_LINK_XPATH, list_link_xpath ); 

	//filter_xpath_noisy( list_sramble_config.link_list_xpath ); 

	POSITION pos; 
	INT32 index; 
	LPCWSTR target_xpath = NULL; 

	switch( scramble_config_step )
	{
	case STEP_LIST_LINKS:
		target_xpath = list_sramble_config.link_list_xpath.c_str(); 
		break; 
	case STEP_NEXT_LINK:
		target_xpath = list_sramble_config.next_link_xpath.c_str(); 
		break; 
	case STEP_CONTENTS:
		pos = contants_elements.GetFirstSelectedItemPosition(); 

		do 
		{
			if( NULL == pos )
			{
				break; 
			}

			index = contants_elements.GetNextSelectedItem( pos ); 
			if( index < 0 
				|| index > contants_elements.GetItemCount() )
			{
				break; 
			}

			target_xpath = ( LPCWSTR )contants_elements.GetItemData( index ); 
			if( NULL == target_xpath )
			{
				break; 
			}

			ASSERT( NULL != target_xpath ); 

		} while ( FALSE );
		break; 
	default:
		ASSERT( FALSE ); 
		
		break; 
	}

	if( NULL != target_xpath )
	{
		g_html_script_config_dlg->highlight_html_element_show( target_xpath ); 
	}
}

void scramble_list_dlg::OnBnClickedButtonCheckNextLink()
{
	// TODO: Add your control notification handler code here

	//g_web_script_config_dlg->highlight_html_element_show( list_sramble_config.next_link_xpath.c_str() ); 
}

void scramble_list_dlg::OnBnClickedButtonCheckContents()
{
	// TODO: Add your control notification handler code here
	//HTML_ELEMENT_CONTENT *element_content = NULL; 
	//POSITION pos; 
	//INT32 index; 
	//LPCWSTR item_data; 

	//pos = contants_elements.GetFirstSelectedItemPosition(); 
	//
	//do 
	//{
	//	if( NULL == pos )
	//	{
	//		break; 
	//	}

	//	index = contants_elements.GetNextSelectedItem( pos ); 
	//	if( index < 0 
	//		|| index > contants_elements.GetItemCount() )
	//	{
	//		break; 
	//	}

	//	item_data = ( LPCWSTR )contants_elements.GetItemData( index ); 
	//	if( NULL == item_data )
	//	{
	//		break; 
	//	}
	//	element_content->xpath = item_data; //L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670

	//	ASSERT( NULL != item_data ); 
	//	g_web_script_config_dlg->highlight_html_element_show( item_data ); 
	//} while ( FALSE );
}

LRESULT WINAPI empty_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 
	LARGE_INTEGER current_pos; 
	LARGE_INTEGER old_pos; 

	do 
	{
		ret = check_file_exist( file_name ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		file_handle = CreateFileW( file_name, 
			FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_ALWAYS, 
			0, 
			NULL ); 

		if( INVALID_HANDLE_VALUE == file_handle ) 
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
		current_pos.QuadPart = 0; 

		_ret = SetFilePointerEx( file_handle, 
			current_pos, 
			&old_pos, 
			SEEK_SET); 

		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE ); 

	if( INVALID_HANDLE_VALUE != file_handle ) 
	{
		CloseHandle( file_handle ); 
	}

	return ret; 
}

void scramble_list_dlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnCancel();
}

void scramble_list_dlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	LRESULT ret; 
	HRESULT hr; 
	wstring url; 
	CString config_file_name; 
	WCHAR _config_file_name[ MAX_PATH ]; 
	ULONG cc_ret_len; 
	INT32 item_count; 
	INT32 i; 

	url = g_html_script_config_dlg->m_WebBrowser.get_loading_url(); 

	do 
	{
		if( url.length() == 0 )
		{
			config_file_name = g_html_script_config_dlg->m_WebBrowser.GetLocationURL(); 
			if( config_file_name.GetLength() == 0 )
			{
				SYSTEMTIME now; 
				GetSystemTime(&now); 
				config_file_name.Format( L"%u_%u_%u_%u_%u_%u.sct", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond ); 
			}

			url = config_file_name.GetBuffer(); 
		}

		if( url.length() == 0 )
		{
			ASSERT( FALSE ); 
			break; 
		}

		ret = get_file_name_from_url( url.c_str(), config_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		item_count = contants_elements.GetItemCount(); 

		HTML_ELEMENT_CONTENT *element_content = NULL; 
		LPCWSTR item_data; 

		list_sramble_config.page_content.element_contents.clear(); 
		for( i = 0; i < item_count; i ++ )
		{
			do 
			{
				element_content = new HTML_ELEMENT_CONTENT(); 
				if( element_content == NULL )
				{
					break; 
				}

				item_data = ( LPCWSTR )contants_elements.GetItemData( i ); 
				if( NULL == item_data )
				{
					break; 
				}

				element_content->name = contants_elements.GetItemText( i, 0 ).GetBuffer(); 
				element_content->xpath = item_data; //L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1670321352 POP POP-1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=summary;|DIV id=summary-price;|"; 
				//L"HTML class=root61;lang=zh-CN;|BODY class=cat-1-670 cat-2-716 cat-3-722 item-1000294 JD JD-1;version=140120;data-lazyload-install=1;|DIV id=p-box;|DIV class=w;|DIV id=product-intro;class=m-item-grid clearfix;|DIV class=m-item-inner;|DIV id=itemInfo;|DIV id=name;"; 

				list_sramble_config.page_content.element_contents.push_back( element_content ); 
				element_content = NULL; 
			}while( FALSE );

			if( NULL != element_content )
			{
				free( element_content ); 
			}
		}

		ret = get_app_path( _config_file_name, ARRAYSIZE( _config_file_name ), &cc_ret_len ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		hr = StringCchCatW( _config_file_name, ARRAYSIZE( _config_file_name ), CONFIG_FILE_DIRECTORY ); 
		if( S_OK != hr )
		{
			break; 
		}

		ret = create_directory_ex( _config_file_name, wcslen( _config_file_name ), 2 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( _config_file_name, 
			ARRAYSIZE( _config_file_name ), 
			config_file_name.GetBuffer() ); 
		if( FAILED(hr))
		{
			break; 
		}

		list_sramble_config.url = g_html_script_config_dlg->m_WebBrowser.get_loading_url(); 

		//make_config_file_exist( _config_file_name, wcslen( _config_file_name ) ); 

		empty_file( _config_file_name ); 

		create_config_file( _config_file_name ); 

		try
		{
			ret = save_scarmble_info( _config_file_name, 
				&list_sramble_config ); 
		}
		catch(...)
		{

		}

		//if( ret == ERROR_SUCCESS )
		//{
		//}

		{
#define MAX_MESSAGE 1024
			WCHAR message[ MAX_MESSAGE ]; 
			StringCchPrintf( message, ARRAYSIZE(message), L"配置文件%s已经生成，使用data_scrambler.exe来收集数据", _config_file_name ); 
			MessageBoxW( message, L"提示", 0 ); 
		}
	}while( FALSE );

	//CDialog::OnOK();
}

INT32 next_link_dlg_item_ids[] = 
{
	//IDC_STATIC_NEXT_LINK_DESC, 
	//IDC_BUTTON_CHECK_NEXT_LINK, 
	IDC_BUTTON_NEXT_LINK_RETURN, 
	//IDC_STATIC_NEXT_LIST, 
	IDC_BUTTON_NEXT_LINK_OK 
}; 

INT32 contents_dlg_item_ids[] = 
{
	//IDC_STATIC_CONTENTS, 
	//IDC_STATIC_CONTENTS_DESC, 
	//IDC_LIST_CONTENT_ELEMENTS, 
	//IDC_STATIC_CONTENT, 
	IDC_BUTTON_CONTENTS_RETURN, 
	//IDC_BUTTON_CHECK_CONTENTS 
}; 

INT32 list_links_dlg_item_ids[] = 
{
	//IDC_STATIC_LIST_LINK, 
	//IDC_STATIC_LINK_LIST_DESC, 
	//IDC_BUTTON_CHECK_LIST_LINKS, 
	IDC_BUTTON_LIST_LINK_OK 
}; 

LRESULT scramble_list_dlg::on_html_element_active( PVOID element, 
												  LPCWSTR xpath )
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTMLElementPtr html_element; 

	do 
	{
		ASSERT( NULL != element ); 
		ASSERT( NULL != xpath ); 

		//html_element = element; 

		if( *xpath == L'\0' )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		switch( scramble_config_step )
		{
		case STEP_LIST_LINKS:
			{
				HTML_ELEMENT_GROUP group; 
				//SetDlgItemText( IDC_EDIT_LIST_LINK_XPATH, xpath ); 
				static wstring previous_xpath; 
				wstring target_xpath; 
				target_xpath = xpath; 

				if( previous_xpath.length() == 0 )
				{
					list_sramble_config.link_list_xpath = target_xpath; 
				}
				else
				{
					if( 0 == _wcsicmp( previous_xpath.c_str(), target_xpath.c_str() ) )
					{
						//break; 
					}

					ret = xor_html_element_xpath( previous_xpath, 
						target_xpath ); 

					if( ret != ERROR_SUCCESS )
					{
						//break; 
					}

					//ret = filter_xpath_noisy( target_xpath ); 
					//if( ret != ERROR_SUCCESS )
					//{

					//}

					list_sramble_config.link_list_xpath = target_xpath; 
				}

				previous_xpath = xpath; 

				do 
				{
					HRESULT hr; 
					CString element_text; 
					IDispatchPtr disp; 
					IHTMLDocument2Ptr doc; 

					list_sramble_config.links.clear(); 

					g_html_script_config_dlg->highlight_html_element_by_xpath( target_xpath.c_str() ); 
					hr = g_html_script_config_dlg->get_html_element_from_xpath( target_xpath.c_str(), &group ); 
					if( FAILED(hr))
					{
					}

					if( group.size() > 0 )
					{
						wstring url; 
						INT32 i; 

						for( i = 0; i < ( INT32 )group.size(); i ++ )
						{
							do 
							{
								hr = get_html_element_url( group[ i ], &url ); 
								if( hr != S_OK )
								{
									break; 
								}

								list_sramble_config.links.push_back( url ); 

							} while ( FALSE ); 
						}
						//save_list_scrambled( L"D:\\kl_users.txt", list_sramble_config.links ); 
					}
					element_text.Format( L"共找到%u个链接", group.size() ); 

					SetDlgItemText( IDC_STATIC_ELEMENT_COUNT, element_text.GetBuffer() ); 
				}while( FALSE ); 

				release_html_element_group( &group ); 
			}
			break; 
		case STEP_NEXT_LINK:
			{
				HRESULT hr; 
				HTML_ELEMENT_GROUP group; 
				wstring target_xpath; 
				CString element_text; 

				//HTML|BODY|DIV id=GMain;|DIV class=container;|DIV class=maino;|DIV class=main;|DIV class=content;|DIV class=search_main;|DIV class=search_left;|DIV class=pager;|DIV class=inc_pg;|UL class=pgs;|LI class=pgNext;element_text=下一页;|
				list_sramble_config.next_link_xpath = xpath; 

				target_xpath = xpath; 

				ret = filter_xpath_noisy( target_xpath ); 
				if( ret != ERROR_SUCCESS )
				{

				}


				do 
				{
					//ULONG count; 
					CString element_text; 
					wstring text; 
					g_html_script_config_dlg->highlight_html_element_by_xpath( target_xpath.c_str() ); 
					hr = g_html_script_config_dlg->get_html_element_from_xpath( target_xpath.c_str(), &group ); 
					if( FAILED(hr))
					{
					}
					
					if( group.size() > 0 )
					{
						get_html_element_url( group[ 0 ], &text ); 
						if( text.length() > 30 )
						{
							text = text.substr(0, 30 ); 
							text += L"..."; 
						}

						element_text.Format( L"下一页链接:%s", text.c_str() ); 
						SetDlgItemText( IDC_STATIC_ELEMENT_URL, element_text.GetBuffer() ); 
					}
					else
					{
						SetDlgItemText( IDC_STATIC_ELEMENT_URL, L"" ); 
					}
				}while( FALSE ); 

				release_html_element_group( &group ); 
			}
			break; 
		case STEP_CONTENTS:
			{
				HRESULT hr; 
				ULONG item_count; 
				wstring element_title; 
				_bstr_t _text; 

				item_count = contants_elements.GetItemCount(); 

				html_element = ( IHTMLElement* )element; 

				//hr = get_html_element_title( html_element, &element_title ); 
				//if( FAILED(hr))
				//{
				//	//break; 
				//}

				element_title = L""; 
				contants_elements.InsertItem(item_count, element_title.c_str() ); 

				do 
				{
					LPWSTR __text; 
					hr = html_element->get_innerText( _text.GetAddress() ); 
					if( FAILED( hr ) )
					{
						break; 
					}
	
					__text = _text.operator wchar_t*(); 

					if( NULL == __text )
					{
						break; 
					}

					contants_elements.SetItemText( item_count, 1, __text ); 

					do
					{
						LPWSTR xpath_copy = NULL; 
						ULONG cc_xpath_len; 
						wstring _xpath; 

						_xpath = xpath; 

						ret = filter_xpath_noisy( _xpath ); 
						if( ret != ERROR_SUCCESS )
						{

						}

						cc_xpath_len = _xpath.size(); 

						xpath_copy = ( LPWSTR )malloc( sizeof( WCHAR ) * ( cc_xpath_len + 1 ) ); 
						if( NULL == xpath_copy )
						{
							break; 
						}

						memcpy( xpath_copy, _xpath.c_str(), sizeof( WCHAR ) * ( cc_xpath_len + 1 ) ); 

						contants_elements.SetItemData( item_count, ( DWORD_PTR )( PVOID )xpath_copy ); 
					}while( FALSE ); 
				} while ( FALSE );
			}
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT scramble_list_dlg::change_scramble_state( LIST_SCRAMBLE_CONFIG_STEP step ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 index; 
	CWnd *dlg_item; 

	ASSERT( step >= STEP_LIST_LINKS && step <= STEP_CONTENTS ); 

	scramble_config_step = step; 

	switch( step )
	{
	case STEP_LIST_LINKS: 
		{
			for( index = 0; index < ARRAYSIZE( next_link_dlg_item_ids ); index ++ )
			{
				do 
				{
					dlg_item = GetDlgItem( next_link_dlg_item_ids[ index ] ); 
					if( NULL == dlg_item )
					{
						break; 
					}

					dlg_item->EnableWindow( FALSE ); 
					//dlg_item->ShowWindow( SW_HIDE ); 
				}while( FALSE ); 
			}

			for( index = 0; index < ARRAYSIZE( list_links_dlg_item_ids ); index ++ )
			{
				do 
				{
					dlg_item = GetDlgItem( list_links_dlg_item_ids[ index ] ); 
					if( NULL == dlg_item )
					{
						break; 
					}

					dlg_item->EnableWindow( TRUE ); 
					//dlg_item->ShowWindow( SW_HIDE ); 
				}while( FALSE ); 
			}

			for( index = 0; index < ARRAYSIZE( contents_dlg_item_ids ); index ++ )
			{
				do 
				{
					dlg_item = GetDlgItem( contents_dlg_item_ids[ index ] ); 
					if( NULL == dlg_item )
					{
						break; 
					}

					dlg_item->EnableWindow( FALSE ); 
					//dlg_item->ShowWindow( SW_HIDE ); 
				}while( FALSE ); 
			}
		}
		break; 
	case STEP_NEXT_LINK: 
		for( index = 0; index < ARRAYSIZE( list_links_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( list_links_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( FALSE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}

		for( index = 0; index < ARRAYSIZE( contents_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( contents_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( FALSE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}

		for( index = 0; index < ARRAYSIZE( next_link_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( next_link_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( TRUE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}

		break; 
	case STEP_CONTENTS: 
		for( index = 0; index < ARRAYSIZE( next_link_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( next_link_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( FALSE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}

		for( index = 0; index < ARRAYSIZE( contents_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( contents_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( FALSE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}

		for( index = 0; index < ARRAYSIZE( contents_dlg_item_ids ); index ++ )
		{
			do 
			{
				dlg_item = GetDlgItem( contents_dlg_item_ids[ index ] ); 
				if( NULL == dlg_item )
				{
					break; 
				}

				dlg_item->EnableWindow( TRUE ); 
				//dlg_item->ShowWindow( SW_HIDE ); 
			}while( FALSE ); 
		}
		break; 
	default:
		ret = ERROR_INVALID_PARAMETER; 
		ASSERT( FALSE ); 
		break; 
	}

	return ret; 
}

BOOL scramble_list_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	//LVS_EDITLABELS
	contants_elements.PrepareControl(0);
	contants_elements.SetExtendedStyle( LVS_EX_GRIDLINES|LVS_EX_INFOTIP| LVS_EX_FULLROWSELECT | /*LVS_EX_CHECKBOXES|*/LVS_REPORT|LVS_EX_SUBITEMIMAGES );

	contants_elements.InsertColumn(0,_T("名称"),LVCFMT_LEFT,150); //添加列标题
	contants_elements.InsertColumn(1,_T("内容"),LVCFMT_LEFT,200); 

	change_scramble_state( STEP_LIST_LINKS ); 

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void scramble_list_dlg::OnBnClickedButtonListLinkOk()
{
	// TODO: Add your control notification handler code here

	change_scramble_state( STEP_NEXT_LINK ); 
}

void scramble_list_dlg::OnBnClickedButtonNextLinkOk()
{
	// TODO: Add your control notification handler code here
	change_scramble_state( STEP_CONTENTS ); 

	wstring url; 

	if( list_sramble_config.links.size() > 0 )
	{
		url = *list_sramble_config.links.begin(); 
		//browser_safe_navigate_start( &g_html_script_config_dlg->m_WebBrowser, url.c_str() ); 
        g_html_script_config_dlg->locate_to_url(url.c_str()); 
	}
}

void scramble_list_dlg::OnNMDblclkListContentElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	INT32 item_index; 

	*pResult = 0;


	do 
	{
		item_index = pNMItemActivate->iItem; 

		if( item_index < 0 )
		{
			break; 
		}

		if( item_index >= contants_elements.GetItemCount() )
		{
			ASSERT( FALSE ); 
			break; 
		}

		{
			PVOID item_data; 
			item_data = ( PVOID )contants_elements.GetItemData( item_index ); 
			if( NULL != item_data )
			{
				free( item_data ); 
			}
		}

		contants_elements.DeleteItem( item_index ); 

		SetDlgItemText( IDC_RICHEDIT21_CONTENT, L"" ); 

	}while( FALSE ); 

	return; 
}

void scramble_list_dlg::OnNMClickListContentElements(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	LONG item_index; 
	CString item_text; 

	// TODO: Add your control notification handler code here
	*pResult = 0;

	do 
	{
		item_index = pNMItemActivate->iItem; 

		if( item_index < 0 )
		{
			break; 
		}

		if( item_index >= contants_elements.GetItemCount() )
		{
			ASSERT( FALSE ); 
			break; 
		}

		item_text = contants_elements.GetItemText( item_index, 1 ); 

		SetDlgItemText( IDC_RICHEDIT21_CONTENT, item_text.GetBuffer() ); 
	}while( FALSE ); 
}

void scramble_list_dlg::OnBnClickedButtonContentsReturn()
{
	// TODO: Add your control notification handler code here
	change_scramble_state( STEP_NEXT_LINK ); 
}

void scramble_list_dlg::OnBnClickedButtonNextLinkReturn()
{
	// TODO: Add your control notification handler code here
	change_scramble_state( STEP_LIST_LINKS ); 
}

#define AJAX_SCRAMBLE_TIMER_ID 1001
void scramble_list_dlg::OnBnClickedButtonRecord()
{
	list_sramble_config.link_list_xpath = L"HTML|BODY|DIV id=GMain;|DIV|DIV|DIV|DIV|DIV|DIV|DIV|DIV id=ShowMain;|DIV|";
	SetTimer( AJAX_SCRAMBLE_TIMER_ID, 3000, NULL ); 
}

void scramble_list_dlg::OnTimer(UINT_PTR nIDEvent)
{
	save_scrambled_list(); 
}

HRESULT WINAPI get_html_element_text( IHTMLElement *html_element, 
									wstring *_text, 
									wstring *html )
{
	HRESULT hr = S_OK; 
	_bstr_t text; 
	LPCWSTR temp_text; 

	do 
	{
		ASSERT( NULL != html_element ); 

		html_element->get_innerText( text.GetAddress() ); 

		temp_text = text.operator wchar_t*(); 
		if( NULL == temp_text )
		{
			break; 
		}

		*_text = temp_text; 
		html_element->get_innerHTML( text.GetAddress() ); 

		temp_text = text.operator wchar_t*(); 
		if( NULL == temp_text )
		{
			break; 
		}

		*html = temp_text; 
	} while ( FALSE );

	return hr; 
}

LRESULT scramble_list_dlg::save_scrambled_list()
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_GROUP group; 

	do 
	{
		HRESULT hr; 
		CString element_text; 
		IDispatchPtr disp; 
		IHTMLDocument2Ptr doc; 
		wstring target_xpath; 

		//for( ; ; )
		{
			target_xpath = list_sramble_config.link_list_xpath; 

			list_sramble_config.links.clear(); 

			g_html_script_config_dlg->highlight_html_element_by_xpath( target_xpath.c_str() ); 
			hr = g_html_script_config_dlg->get_html_element_from_xpath( target_xpath.c_str(), &group ); 
			if( FAILED(hr))
			{
			}

			if( group.size() == 0 )
			{
				//KillTimer( AJAX_SCRAMBLE_TIMER_ID ); 
				break; 
			}

			do 
			{
				if( group.size() > 0 )
				{
					wstring text; 
					wstring html; 
					INT32 i; 

					for( i = 0; i < ( INT32 )group.size(); i ++ )
					{
						do 
						{
							hr = get_html_element_text( group[ i ], &text, &html ); 
							if( hr != S_OK )
							{
								break; 
							}

							list_sramble_config.links.push_back( text ); 
							list_sramble_config.links.push_back( html ); 
						} while ( FALSE ); 
					}

					save_list_scrambled( L"E:\\kl_users.txt", list_sramble_config.links ); 
				}
			} while ( FALSE ); 

			release_html_element_group( &group ); 

			do 
			{
				//if( list_sramble_config.next_link.size() == 0)
				//{
				//	break; 
				//}

				target_xpath = list_sramble_config.next_link_xpath; 

				g_html_script_config_dlg->highlight_html_element_by_xpath( target_xpath.c_str() ); 
				hr = g_html_script_config_dlg->get_html_element_from_xpath( target_xpath.c_str(), &group ); 
				if( FAILED(hr))
				{
				}

				if( group.size() > 0 )
				{
					wstring url; 
					INT32 i; 

					for( i = 0; i < ( INT32 )group.size(); i ++ )
					{
						do 
						{
							hr = click_next_link_button( group[ i ] ); 

						} while ( FALSE ); 
					}
				}
				else 
				{
					KillTimer( AJAX_SCRAMBLE_TIMER_ID ); 
					break; 
				}
			}while( FALSE ); 

			release_html_element_group( &group ); 
		}
	}while( FALSE ); 

	return ret; 
}
