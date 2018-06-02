// xpath_compare_dlg.cpp : Œ×÷™n
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "xpath_compare_dlg.h"
#include "html_xpath.h"

// xpath_compare_dlg Œ¦Ô’·½‰K

IMPLEMENT_DYNAMIC(xpath_compare_dlg, CDialog)

xpath_compare_dlg::xpath_compare_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(xpath_compare_dlg::IDD, pParent)
{

}

xpath_compare_dlg::~xpath_compare_dlg()
{
}

void xpath_compare_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(xpath_compare_dlg, CDialog)
	ON_MESSAGE( WM_XPATH_MISMATCH_INFO, on_check_xpath_mismatch )
END_MESSAGE_MAP()

#define TEXT_COMPARE_COLOR_DELETE RGB(240, 80, 80)
#define TEXT_COMPARE_COLOR_INSERT RGB(80, 80, 240)
#define TEXT_COMPARE_COLOR_EQUAL RGB(0, 0, 0)

LRESULT WINAPI set_edit_text_color(CRichEditCtrl* the_richedit, INT32 min, INT32 max, ULONG color); 

#include "diff_match_patch.h"

BOOL xpath_compare_dlg::OnInitDialog()
{		
	CRichEditCtrl *richedit; 
	INT32 _ret; 
	CDialog::OnInitDialog();
	CHARFORMAT char_format; 

	do 
	{
		richedit = ( CRichEditCtrl* )GetDlgItem( IDC_RICHEDIT21_XPATH_COMPARE ); 

		memset( &char_format, 0, sizeof( char_format ) ); 

		richedit->GetDefaultCharFormat( char_format ); 

		char_format.cbSize = sizeof( CHARFORMAT ); 
		char_format.dwMask = CFM_COLOR;
		char_format.dwEffects &= ~CFE_AUTOCOLOR;
		char_format.crTextColor = 0;  

		_ret = richedit->SetDefaultCharFormat( char_format ); 
	}while( FALSE ); 

	return TRUE; 
}

LRESULT xpath_compare_dlg::on_check_xpath_mismatch( WPARAM wparam, 
													   LPARAM lparam) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( NULL == wparam 
			&& NULL == lparam )
		{
			SetDlgItemText( IDC_RICHEDIT21_XPATH_COMPARE, L"" ); 
			break; 
		}

		ShowWindow( SW_SHOW ); 
		compare_xpath_mismatch( ( LPCWSTR )( PVOID )wparam, ( LPCWSTR )( PVOID )lparam ); 
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI append_one_line( CRichEditCtrl *edit, LPCWSTR ch )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 text_len; 
	INT32 _text_len; 
	INT32 i; 

	text_len = edit->GetTextLength(); 

	for( i = 0; i < 85; i ++ )
	{
		edit->SetSel( -1, -1 ); 
		edit->ReplaceSel( ch );

		//if( edit->GetLineCount() > line_count )
		//{
		//	break; 
		//}
	}

	_text_len = edit->GetTextLength(); 
	set_edit_text_color( edit, text_len, _text_len, 0 ); 

	return ret; 
}
LRESULT xpath_compare_dlg::find_string_difference( LPCWSTR xpath_src, LPCWSTR xpath_dest ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	INT32 start_index;
	INT32 end_index;

	diff_match_patch<wstring> dmp;
	diff_match_patch<wstring>::Diffs diffs; 
	diff_match_patch<wstring>::Diffs::iterator it; 
	CRichEditCtrl *edit; 
	CString _input_text; 
	ULONG text_color; 

	do 
	{
		CHARFORMAT char_format; 

		diffs = dmp.diff_main( xpath_src, xpath_dest, false ); 

		edit = ( CRichEditCtrl* )GetDlgItem( IDC_RICHEDIT21_XPATH_COMPARE ); 
		ASSERT( NULL != edit ); 

		//edit->GetWindowText( _input_text ); 

		//for( it = diffs.begin(); it != diffs.end(); it ++ )
		//{
		//	switch( ( *it ).operation )
		//	{
		//	case diff_match_patch<wstring>::OP_DELETE:
		//		input_text_src += ( *it ).text.c_str(); 
		//		break; 
		//	case diff_match_patch<wstring>::OP_INSERT:
		//		input_text_dest += ( *it ).text.c_str(); 
		//		break; 
		//	case diff_match_patch<wstring>::OP_EQUAL:
		//		input_text_src += ( *it ).text.c_str(); 
		//		input_text_dest += ( *it ).text.c_str(); 
		//		break; 
		//	default:
		//		ASSERT( FALSE ); 
		//		ret = ERROR_ERRORS_ENCOUNTERED; 
		//		break; 
		//	}

		//	if( ret != ERROR_SUCCESS )
		//	{
		//		break; 
		//	}
		//}

		//if( ret != ERROR_SUCCESS )
		//{
		//	dbg_print( MSG_FATAL_ERROR, "%s:%u\n", __FUNCTION__, __LINE__ ); 
		//	break; 
		//}

		for( it = diffs.begin(); it != diffs.end(); it ++ )
		{
			_ret = ERROR_SUCCESS; 
			start_index = edit->GetTextLength(); 
			switch( ( *it ).operation )
			{
			case diff_match_patch<wstring>::OP_DELETE:
				text_color = TEXT_COMPARE_COLOR_DELETE; 
				edit->SetSel( -1, -1 ); 
				edit->ReplaceSel( ( *it ).text.c_str() ); 
				break; 
			case diff_match_patch<wstring>::OP_INSERT:
				_ret = ERROR_INVALID_PARAMETER; 
				break; 
			case diff_match_patch<wstring>::OP_EQUAL:
				text_color = TEXT_COMPARE_COLOR_EQUAL; 
				edit->SetSel( -1, -1 ); 
				edit->ReplaceSel( ( *it ).text.c_str() ); 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "%s:%u\n", __FUNCTION__, __LINE__ ); 
				break; 
			}

			do 
			{
				if( ERROR_SUCCESS != _ret )
				{
					break; 
				}

				end_index = edit->GetTextLength(); 
				_ret = set_edit_text_color( edit, start_index, end_index, text_color ); 
				if( _ret != ERROR_SUCCESS )
				{

				}
			}while( FALSE ); 
		}

		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "%s:%u\n", __FUNCTION__, __LINE__ ); 
			break; 
		}

		append_one_line( edit, L" " ); 

		//_input_text = L"\r\n"; 
		//edit->SetSel( -1, -1 ); 
		//edit->ReplaceSel( L"\r\n" );

		for( it = diffs.begin(); it != diffs.end(); it ++ )
		{
			start_index = edit->GetTextLength(); 
			_ret = ERROR_SUCCESS; 
			switch( ( *it ).operation )
			{
			case diff_match_patch<wstring>::OP_DELETE:
				_ret = ERROR_INVALID_PARAMETER; 
				break; 
			case diff_match_patch<wstring>::OP_INSERT:
				text_color = TEXT_COMPARE_COLOR_INSERT; 
				edit->SetSel( -1, -1 ); 
				edit->ReplaceSel( ( *it ).text.c_str() ); 
				ASSERT( edit->GetTextLength() - start_index == ( *it ).text.length() ); 
				break; 
			case diff_match_patch<wstring>::OP_EQUAL:
				text_color = TEXT_COMPARE_COLOR_EQUAL; 
				edit->SetSel( -1, -1 ); 
				edit->ReplaceSel( ( *it ).text.c_str() ); 
				ASSERT( edit->GetTextLength() - start_index == ( *it ).text.length() ); 
				break; 
			default:
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "%s:%u\n", __FUNCTION__, __LINE__ ); 
				break; 
			}

			do 
			{
				if( ERROR_SUCCESS != _ret )
				{
					break; 
				}

				dbg_print_w( MSG_IMPORTANT, L"text %s color:%x\n", ( *it ).text.c_str(), text_color ); 

				end_index = edit->GetTextLength(); 
				_ret = set_edit_text_color( edit, start_index, end_index, text_color ); 
				if( _ret != ERROR_SUCCESS )
				{

				}

			}while( FALSE ); 
		}

		append_one_line( edit, L" " ); 
		append_one_line( edit, L"-" ); 

		//_input_text = L"\r\n"; 
		//edit->SetSel( -1, -1 ); 
		//edit->ReplaceSel( L"\r\n" );

	} while ( FALSE ); 
	return ret; 
}

LRESULT xpath_compare_dlg::compare_xpath_mismatch( LPCWSTR xpath_src, LPCWSTR xpath_dest )
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 i; 
	//CRichEditCtrl *edit; 
	//INT32 text_len; 
	//INT32 _text_len; 

	do 
	{
		ASSERT( xpath_src != NULL ); 
		ASSERT( xpath_dest != NULL ); 

		//for( i = 0; i < 2; i ++ )
		//{
		ret = find_string_difference( xpath_src, xpath_dest ); 
		//}

	//	edit = ( CRichEditCtrl*)GetDlgItem( IDC_RICHEDIT21_XPATH_COMPARE ); 
	//	if( NULL == edit )
	//	{
	//		break; 
	//	}

	//	CString text; 
	//	text = xpath_src; 
	//INT32 i; 
	//	for( i = 0; i < text.GetLength() - 2; i ++ )
	//	{
	//		text_len = edit->GetTextLength(); 
	//		edit->SetSel( -1, -1 ); 
	//		edit->ReplaceSel( text.Mid( i, 1 ) ); 
	//		edit->SetSel( -1, -1 ); 
	//		edit->ReplaceSel( L"\r\n" ); 
	//		_text_len = edit->GetTextLength(); 

	//		if( i % 2 )
	//		{
	//			set_edit_text_color( edit, text_len, _text_len, RGB( 255,0,0) ); 
	//		}
	//		else
	//		{
	//			set_edit_text_color( edit, text_len, _text_len, RGB( 200,200,0) ); 
	//		}
	//	}

	//	text = xpath_dest; 

	//	for( i = 0; i < text.GetLength() - 2; i ++ )
	//	{
	//		text_len = edit->GetTextLength(); 
	//		edit->SetSel( -1, -1 ); 
	//		edit->ReplaceSel( text.Mid( i, 1 ) ); 
	//		edit->SetSel( -1, -1 ); 
	//		edit->ReplaceSel( L"\r\n" ); 
	//		_text_len = edit->GetTextLength(); 

	//		if( i % 2 )
	//		{
	//			set_edit_text_color( edit, text_len, _text_len, RGB( 255,0,0) ); 
	//		}
	//		else
	//		{
	//			set_edit_text_color( edit, text_len, _text_len, RGB( 200,200,0) ); 
	//		}
	//	}

	}while( FALSE ); 

	return ret; 
}