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
// login_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "resource.h"
#include "login_dlg.h"
#include "user_reg.h"
#include "usage_intro_dialog.h"

// login_dlg 對話方塊

IMPLEMENT_DYNAMIC(login_dlg, CDialog)

login_dlg::login_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(login_dlg::IDD, pParent)
{

}

login_dlg::~login_dlg()
{
}

void login_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL login_dlg::OnInitDialog()  
{  
	CDialog::OnInitDialog();  
	CString text; 
	GetDlgItemText( IDC_STATIC_USER_REGISTER, text ); 

	text += L"<a style='color:#df3434; font-weight:bold;'></a>"; 

	register_link.SubclassDlgItem( IDC_STATIC_USER_REGISTER, this ); 
	register_link.SetText( text ); 

	GetDlgItemText( IDC_STATIC_USAGE_INTRO, text ); 

	text += L"<a style='color:#df3434; font-weight:bold;'></a>"; 

	show_usage_link.SubclassDlgItem( IDC_STATIC_USAGE_INTRO, this ); 
	show_usage_link.SetText( text ); 
	return TRUE;  
}   

BEGIN_MESSAGE_MAP(login_dlg, CDialog)
	ON_STN_CLICKED(IDC_STATIC_USER_REGISTER, &login_dlg::OnStnClickedStaticUserRegister)
	ON_STN_CLICKED(IDC_STATIC_USAGE_INTRO, &login_dlg::OnStnClickedStaticUsageIntro)
END_MESSAGE_MAP()

void login_dlg::OnOK()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		GetDlgItemText( IDC_EDIT_USER_NAME, user_name ); 
		GetDlgItemText( IDC_EDIT_PASSWORD, password ); 

		if( user_name.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			MessageBox( L"用户名不可以为空" ); 
			break; 
		}

		if( password.GetLength() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			MessageBox( L"密码不可以为空" ); 
			break; 
		}

		{
			wstring error_string; 
			USER_INFO user_info; 
			LOCAL_USER_INFO _user_info; 

			user_info.user_name = user_name.GetBuffer(); 
			user_info.password = password.GetBuffer(); 

			ret = get_user_info_from_server( &user_info, error_string ); 
			if( user_info.can_use == TRUE )
			{
				if( user_info.can_use == FALSE )
				{
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}
				
				_user_info.user_name = user_name.GetBuffer(); 
				_user_info.password = password.GetBuffer(); 

				_ret = set_local_user_info( &_user_info); 
				if( _ret != ERROR_SUCCESS )
				{
				}
				break; 
			}
			else
			{
				if( ret == ERROR_USER_DONOT_HAVE_ACCESS )
				{
					wstring message; 
					message = error_string; 
					message = L"请点击查看使用方法了解"; 

					MessageBox( error_string.c_str(), L"", 0 ); 
					//usage_intro_dialog dlg; 
					//dlg.DoModal(); 
				}
				else
				{
					if( error_string.length() > 0 )
					{
						MessageBox( error_string.c_str(), L"", 0 ); 
					}
				}
				break; 
			}
		}
	}while( FALSE );
	
	if( ret == ERROR_SUCCESS )
	{
		CDialog::OnOK(); 
	}
}

void login_dlg::OnCancel()
{
	CDialog::OnCancel(); 
}

void login_dlg::OnStnClickedStaticUserRegister()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	::ShellExecute(NULL, NULL, DONATE_URL, NULL, NULL, SW_SHOWNORMAL);
}

void login_dlg::OnStnClickedStaticUsageIntro()
{
	usage_intro_dialog dlg; 
	dlg.DoModal(); 
}
