// usage_intro_dialog.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "usage_intro_dialog.h"
#include "user_reg.h"


// usage_intro_dialog 對話方塊

IMPLEMENT_DYNAMIC(usage_intro_dialog, CDialog)

usage_intro_dialog::usage_intro_dialog(CWnd* pParent /*=NULL*/)
	: CDialog(usage_intro_dialog::IDD, pParent)
{

}

usage_intro_dialog::~usage_intro_dialog()
{
}

void usage_intro_dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

LRESULT WINAPI load_resource_data( ULONG res_id, ULONG *data_size, PVOID *data, HGLOBAL *handle ); 

BOOL usage_intro_dialog::OnInitDialog()
{
	LRESULT ret; 
	CDialog::OnInitDialog(); 
	ULONG data_size; 
	PVOID data; 
	HGLOBAL handle = NULL; 
	LPSTR _text = NULL; 
	wstring text; 
	CRichEditCtrl *edit; 

	do 
	{
		ret = load_resource_data( IDR_TXT_USAGE_INTRO, &data_size, &data, &handle ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		_text = ( LPSTR )malloc( data_size + 1 ); 
		memcpy( _text, data, data_size ); 
		_text[ data_size ] = '\0'; 

		ret = utf8_to_unicode_ex( _text + 3, text ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		font.CreateFont( 13,0,0,0,FW_DONTCARE,FALSE,FALSE,0, 1,
			OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, 
			DEFAULT_PITCH|FF_SWISS,
			_T("黑体") ); 

		edit = ( CRichEditCtrl* )GetDlgItem(IDC_RICHEDIT21_TEXT); 
		edit->SetFont( &font );

		edit->SetWindowText(text.c_str() ); 

		//int lineStart, lineEnd;
		//CHARFORMAT cFmt;
		//cFmt.cbSize = sizeof(CHARFORMAT);
		//cFmt.crTextColor = RGB(0,0,230);
		//cFmt.dwEffects = 0;
		//cFmt.dwMask = CFM_COLOR;

		//lineStart = edit->LineIndex(4);//取第一行的第一个字符的索引
		//lineEnd = edit->LineIndex(8)-1;//取第一行的最后一个字符的索引——用第二行的第一个索引减1来实现
		//edit->SetSel( lineStart,lineEnd );//选取第一行字符
		//edit->SetSelectionCharFormat( cFmt);//设置颜色
	}while( FALSE ); 

	if( NULL != handle )
	{
		UnlockResource( handle ); 
	}

	if( NULL != _text )
	{
		free( _text ); 
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(usage_intro_dialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET_ACCESS, &usage_intro_dialog::OnBnClickedButtonGetAccess)
END_MESSAGE_MAP()

void usage_intro_dialog::OnCancel()
{
	LRESULT ret = ERROR_SUCCESS; 
	CDialog::OnCancel(); 
}

void usage_intro_dialog::OnBnClickedButtonGetAccess()
{
	CString get_access_url; 

	get_access_url.Format( GET_ACCESS_URL, user_name.c_str() ); 
	::ShellExecute(NULL, NULL, get_access_url.GetBuffer(), NULL, NULL, SW_SHOWNORMAL);
}
