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


// error_process_dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "error_process_dlg.h"


// error_process_dlg 对话框

IMPLEMENT_DYNAMIC(error_process_dlg, CDialog)

error_process_dlg::error_process_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_ERROR_PROCESS, pParent)
{

}

error_process_dlg::~error_process_dlg()
{
}

void error_process_dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT_TEXT, edit_error_process);
}


BEGIN_MESSAGE_MAP(error_process_dlg, CDialog)
    ON_BN_CLICKED(IDYES, &error_process_dlg::OnBnClickedYes)
    ON_BN_CLICKED(IDNO, &error_process_dlg::OnBnClickedNo)
END_MESSAGE_MAP()


BOOL error_process_dlg::OnInitDialog()
{
    CFont font;

    CDialog::OnInitDialog();

    do
    {
        font.CreateFont(15, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 0, 1,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            _T("黑体"));

        edit_error_process.SetFont(&font);

        edit_error_process.SetWindowTextW( error_message.c_str() );

    } while (FALSE);

    return FALSE;
}
// error_process_dlg 消息处理程序


LRESULT error_process_dlg::set_error_message(LPCWSTR text)
{
    LRESULT ret = ERROR_SUCCESS; 
    
    do 
    {
        error_message = text; 
    } while ( FALSE );
    
    return ret; 
}


void error_process_dlg::OnBnClickedYes()
{
    // TODO: 在此添加控件通知处理程序代码
    EndDialog(IDYES); 
}



void error_process_dlg::OnBnClickedNo()
{
    // TODO: 在此添加控件通知处理程序代码
    EndDialog(IDNO);
}
