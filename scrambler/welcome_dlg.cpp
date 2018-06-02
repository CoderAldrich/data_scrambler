// welcome_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "welcome_dlg.h"


// welcome_dlg 對話方塊

IMPLEMENT_DYNAMIC(welcome_dlg, CDialog)

welcome_dlg::welcome_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(welcome_dlg::IDD, pParent)
{

}

welcome_dlg::~welcome_dlg()
{
}

void welcome_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(welcome_dlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &welcome_dlg::OnBnClickedButtonHelp)
END_MESSAGE_MAP()

LRESULT WINAPI load_resource_data( ULONG res_id, ULONG *data_size, PVOID *data, HGLOBAL *handle )
{
	LRESULT ret = ERROR_SUCCESS; 
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, 
		MAKEINTRESOURCE(res_id),
		L"TXT");
	HGLOBAL hGlob = NULL; 

	do 
	{
		if( NULL == hRes)
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		hGlob = LoadResource(hInst, hRes);
		if( NULL == hGlob)
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		*data_size = SizeofResource(hInst, hRes);
		*data = ( PVOID )LockResource( hGlob );
		if( *data_size == 0 || NULL == *data )
		{
			ret = ERROR_ERRORS_ENCOUNTERED;
			break; 
		}

		*handle = hGlob; 
		hGlob = NULL; 
	}while( FALSE );

	if( NULL != hGlob )
	{
		UnlockResource( hGlob ); 
	}

	return ret;
}

BOOL welcome_dlg::OnInitDialog()
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
		ret = load_resource_data( IDR_TXT_WELCOME, &data_size, &data, &handle ); 
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

		int lineStart, lineEnd;
		CHARFORMAT cFmt;
		cFmt.cbSize = sizeof(CHARFORMAT);
		cFmt.crTextColor = RGB(0,0,230);
		cFmt.dwEffects = 0;
		cFmt.dwMask = CFM_COLOR;

		lineStart = edit->LineIndex(4);//取第一行的第一个字符的索引
		lineEnd = edit->LineIndex(8)-1;//取第一行的最后一个字符的索引——用第二行的第一个索引减1来实现
		edit->SetSel( lineStart,lineEnd );//选取第一行字符
		edit->SetSelectionCharFormat( cFmt);//设置颜色
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
void welcome_dlg::OnBnClickedButtonHelp()
{
	// TODO: 在此加入控制項告知處理常式程式碼
#define DONATE_URL L"http://www.simplestai.com/webkits/index.php" 
	::ShellExecute(NULL, NULL, DONATE_URL, NULL, NULL, SW_SHOWNORMAL);

}
