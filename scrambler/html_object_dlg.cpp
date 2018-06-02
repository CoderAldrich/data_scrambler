// html_object_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "html_object_dlg.h"


// html_object_dlg dialog

IMPLEMENT_DYNAMIC(html_object_dlg, CDialog)

html_object_dlg::html_object_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(html_object_dlg::IDD, pParent)
{

}

html_object_dlg::~html_object_dlg()
{
}

void html_object_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_OBJECTS, html_objects_list);
}


BEGIN_MESSAGE_MAP(html_object_dlg, CDialog)
END_MESSAGE_MAP()


// html_object_dlg message handlers

BOOL html_object_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

#include "html_common.h"
#include "html_xpath.h"

LRESULT WINAPI get_html_object_properties( IHTMLElement *element, wstring *text )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != element ); 
		ASSERT( NULL != text ); 


	} while ( FALSE );

	return ret; 
}

LRESULT html_object_dlg::init_object_properties()
{
	LRESULT ret = ERROR_SUCCESS; 
	IHTMLElement *_html_element; 
	wstring text; 

	do 
	{
		if( NULL == html_object )
		{
			break; 
		}

		_html_element = ( IHTMLElement* )html_object; 

		ret = get_html_object_properties( _html_element, &text ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}


	} while (FALSE ); 

	return ret; 
}
void html_object_dlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
}

void html_object_dlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::OnCancel();
}

LRESULT html_object_dlg::init_html_object_ui()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG colume_index = 0; 

	do
	{
		_ret = html_objects_list.InsertColumn( colume_index ++, _T("”Ú√˚"), LVCFMT_LEFT, 120);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}

		_ret = html_objects_list.InsertColumn( colume_index ++, _T("√Ë ˆ"), LVCFMT_LEFT, 100);
		if( _ret < 0 )
		{
			SAFE_SET_ERROR_CODE( ret ); 
		}

		html_objects_list.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP | LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES|LVS_SHOWSELALWAYS);
	}while( FALSE ); 
	return ret; 
}