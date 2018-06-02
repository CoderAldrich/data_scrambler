// apture_input_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "html_maker_config.h"
#include "input_content_select_dlg.h"
#include "input_content.h"

#define IDC_MENU_ITEM_DELETE_TEXT_BLOCK 0x1011
// capture_input_dlg dialog

extern XPATH_PARAMS input_contents; 

IMPLEMENT_DYNAMIC(input_content_select_dlg, CDialog)

input_content_select_dlg::input_content_select_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(input_content_select_dlg::IDD, pParent)
{

}

input_content_select_dlg::~input_content_select_dlg()
{
	input_contents.clear(); 
}

void input_content_select_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TEXT_BLOCKS, text_block_list);
}

LRESULT input_content_select_dlg::init_html_config_ui()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG colume_index = 0; 

	CString text; 
	CString text_name; 
	INT32 list_item_index; 
	wstring conf_file_name; 
	XPATH_PARAM_ITERATOR it; 

	do
	{
		input_contents.clear(); 

		ret = get_input_context_file_path( conf_file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = load_input_content( conf_file_name.c_str(), input_contents ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_ret = text_block_list.InsertColumn( colume_index, _T("文本名"), LVCFMT_LEFT, 120);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}
		else
		{
			colume_index ++; 
		}

		_ret = text_block_list.InsertColumn( colume_index, _T("文本内容"), LVCFMT_LEFT, 120);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}
		else
		{
			colume_index ++; 
		}

		text_block_list.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES
			|LVS_SHOWSELALWAYS);

		for( it = input_contents.begin(); it != input_contents.end(); it ++ )
		{
			list_item_index = text_block_list.InsertItem( 0, ( *it ).first.c_str(), -1 ); 

			text_block_list.SetItemText(list_item_index,0,( *it ).first.c_str() ); 

			text_block_list.SetItemText(list_item_index,1,( *it ).second.c_str() ); 

			text_block_list.SetItemData( list_item_index, ( DWORD_PTR )( *it ).first.c_str() ); 
		}

	}while( FALSE ); 

	return ret; 
}

BOOL input_content_select_dlg::OnInitDialog()
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

	init_html_config_ui(); 

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(input_content_select_dlg, CDialog)
	ON_BN_CLICKED(IDOK, &input_content_select_dlg::OnBnClickedOk)
	ON_COMMAND( IDC_MENU_ITEM_DELETE_TEXT_BLOCK, &input_content_select_dlg::on_delete_text_block )
	ON_NOTIFY(NM_RCLICK, IDC_LIST_TEXT_BLOCKS, &input_content_select_dlg::OnNMRClickListTextBlocks)
END_MESSAGE_MAP()


// input_content_select_dlg message handlers

void input_content_select_dlg::on_delete_text_block()
{
	LRESULT ret; 
	POSITION pSelectedPos = NULL;
	UINT32 nSelectedCount; 
	INT32 nSelectedPos = -1; 
	LPCWSTR text_block; 


	do 
	{
		nSelectedCount = text_block_list.GetSelectedCount();
		if ( nSelectedCount < 1 )
		{
			break; 
		}

		pSelectedPos = text_block_list.GetFirstSelectedItemPosition();
		if ( pSelectedPos == NULL )
		{
			break;
		}

		nSelectedPos = text_block_list.GetNextSelectedItem( pSelectedPos );
		if ( nSelectedPos < 0 )
		{
			break; 
		}

		text_block = ( LPCWSTR )text_block_list.GetItemData( nSelectedPos ); 
		ASSERT( NULL != text_block ); 

		{
			XPATH_PARAM_ITERATOR text_block_it; 
		
			for( text_block_it = input_contents.begin(); text_block_it != input_contents.end(); text_block_it ++ )
			{
				if( 0 == wcscmp( text_block, ( *text_block_it ).first.c_str() ) )
				{
					{
						wstring conf_file_name;  

						ret = get_input_context_file_path( conf_file_name ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						ret = del_input_text_block_info( conf_file_name.c_str(), ( *text_block_it ).second.c_str(), ( *text_block_it ).first.c_str() ); 
						if( ret != ERROR_SUCCESS )
						{
							ASSERT( FALSE ); 
						}
					}

					input_contents.erase( text_block_it ); 
					break; 
				}
			}

			//ASSERT( text_block_it != input_contents.end() ); 
		}

		text_block_list.DeleteItem( nSelectedPos ); 

	}while( FALSE );

	return; 
}

void input_content_select_dlg::OnBnClickedOk()
{
	//LRESULT ret; 
	//CString text; 
	LPCWSTR _text_name; 
	//wstring conf_file_name; 
	//XPATH_PARAM_ITERATOR it; 

	LRESULT ret = ERROR_SUCCESS; 
	//INT32 item_count; 
	INT32 item_index = -1; 
	POSITION pos; 

	do 
	{
		pos = text_block_list.GetFirstSelectedItemPosition();
		if (pos == NULL)
		{
			TRACE(_T("No items were selected!\n"));
		}
		else
		{
			//for( ; ; )
			{
				do 
				{
					item_index = text_block_list.GetNextSelectedItem( pos ); 

					_text_name = ( LPCWSTR )text_block_list.GetItemData( item_index ); 
					if( NULL != _text_name )
					{
						XPATH_PARAM_ITERATOR xpath_it; 

						text_name = _text_name; 

						xpath_it = input_contents.find( text_name.c_str() ); 
					
						if( xpath_it == input_contents.end() )
						{
							text_block = L""; 
						}
						else
						{
							text_block = xpath_it->second.c_str(); 
						}
					}
					else
					{
						text_name = L""; 
					}

					break; 
				}while( FALSE ); 
			}
		}
	}while( FALSE ); 

	// TODO: Add your control notification handler code here
	OnOK();
}

void input_content_select_dlg::OnNMRClickListTextBlocks(NMHDR *pNMHDR, LRESULT *pResult)
{
	LRESULT ret; 
	INT32 _ret; 
	// TODO: Add your control notification handler code here
	CMenu menu; 
	NMLISTVIEW* pMListView; 
	CPoint pt; 

	do
	{
		*pResult = 0;

		_ret = menu.CreatePopupMenu();
		if( FALSE == _ret )
		{
			break; 
		}

		_ret = menu.AppendMenu( MF_STRING, IDC_MENU_ITEM_DELETE_TEXT_BLOCK, L"删除" );
		if( FALSE == _ret )
		{
			break; 
		}

		pMListView=(NMLISTVIEW*) pNMHDR;
		int nIndex = pMListView->iItem;
		if ( nIndex < 0 )
		{
			menu.EnableMenuItem( 0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED ); 
		}

		GetCursorPos( &pt );
		_ret = menu.TrackPopupMenu( TPM_LEFTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, this ); 
		if( FALSE == _ret )
		{
			break; 
		}

	}while( FALSE ); 
}
