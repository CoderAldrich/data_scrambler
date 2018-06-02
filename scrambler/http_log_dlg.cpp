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

// scrapy_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "http_log_dlg.h"


// scrapy_dlg 對話方塊

IMPLEMENT_DYNAMIC(http_log_dlg, CDialog)

http_log_dlg::http_log_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(http_log_dlg::IDD, pParent)
{

}

http_log_dlg::~http_log_dlg()
{
}

/*
xpath(): 传入xpath表达式，返回该表达式所对应的所有节点的selector list列表 。
css(): 传入CSS表达式，返回该表达式所对应的所有节点的selector list列表.
extract(): 序列化该节点为unicode字符串并返回list。
re(): 根据传入的正则表达式对数据进行提取，返回unicode字符串list列表。
*/

void http_log_dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT2_LOG, edit_log);
}


BEGIN_MESSAGE_MAP(http_log_dlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_COPY, &http_log_dlg::OnBnClickedButtonCopy)
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &http_log_dlg::OnBnClickedButtonOpen)
    ON_WM_KEYUP()
    ON_WM_CHAR()
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &http_log_dlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// scrapy_dlg 訊息處理常式


BOOL http_log_dlg::OnInitDialog()
{
    LRESULT ret; 
    wstring file_name;
    CRichEditCtrl *edit;
    CFont font;

    CDialog::OnInitDialog();

    do
    {
        font.CreateFont(13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 0, 1,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            _T("黑体"));

        edit = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT2_LOG);
        edit->SetFont(&font);
    } while (FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

/******************************************************************
显示HTML结构化内容
显示元素XPATH
*******************************************************************/

LRESULT http_log_dlg::append_log(LPCWSTR text)
{
    LRESULT ret = ERROR_SUCCESS; 
    //LONG start_pos; 
    //LONG end_pos;

    do 
    {
        edit_log.SetSel(-1, -1); 
        edit_log.ReplaceSel(text, TRUE); 
        edit_log.SetSel(-1, -1);
        edit_log.ReplaceSel(L"\r\n", TRUE);
        //edit_script.SetSel(start_pos, start_pos+wcslen(std_xpath));
        //UpdateData(TRUE); 
        Invalidate(FALSE); 
    } while ( FALSE ); 
    return ret; 
}

void http_log_dlg::OnBnClickedButtonCopy()
{
    CString text; 

    edit_log.GetWindowTextW(text); 
}

LRESULT WINAPI get_log_dir(LPWSTR path, ULONG cc_max_len, ULONG *cc_ret_len)
{
    LRESULT ret = ERROR_SUCCESS; 

    do 
    {
        *path = L'\0'; 
        *cc_ret_len = 0; 
    } while ( FALSE );

    return ret; 
}

LRESULT http_log_dlg::open_log_file(LPCWSTR file_name)
{
    LRESULT ret = ERROR_SUCCESS;
    LRESULT _ret; 
    WCHAR app_path[MAX_PATH];
    ULONG cc_ret_len;
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    CHAR* pbBuff = NULL;
    DWORD pcb;
    DWORD dwSizeLow;
    DWORD dwRet;
    wstring text;

    do
    {
        file_handle = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == file_handle)
        {
            break;
        }

        dwSizeLow = GetFileSize(file_handle, 0);
        pbBuff = (CHAR*)malloc(dwSizeLow + 1);
        if (NULL == pbBuff)
        {
            break;
        }

        pcb = 0;

        dwRet = ::ReadFile(file_handle, pbBuff, dwSizeLow, (LPDWORD)&pcb, NULL);
        pbBuff[dwSizeLow] = '\0';

        utf8_to_unicode_ex(pbBuff, text);
        ATLASSERT(::IsWindow(edit_log.GetSafeHwnd()));

        ::SendMessage(edit_log.GetSafeHwnd(), WM_SETTEXT, 0, (LPARAM)text.c_str());

        script_file_name = file_name;

        _ret = set_main_config_ex(PREVIOUS_EDIT_SCRIPT_FILE_NAME, script_file_name.GetBuffer());
        if (_ret != ERROR_SUCCESS)
        {

        }

    } while (FALSE);

    if (INVALID_HANDLE_VALUE != file_handle)
    {
        ::CloseHandle(file_handle);
    }

    if (NULL != pbBuff)
    {
        free(pbBuff);
    }
    
    return ret; ;
}

void http_log_dlg::OnBnClickedButtonOpen()
{
    LRESULT ret; 
    WCHAR file_name[ MAX_PATH ]; 
    WCHAR app_path[MAX_PATH];
    ULONG cc_ret_len; 

    do
    {
        ret = get_log_dir(app_path, ARRAYSIZE(app_path), &cc_ret_len); 
        if (ret != ERROR_SUCCESS)
        {
            //break; 
        }

        *file_name = L'\0'; 
        ret = open_file_dlg(TRUE, L".log", file_name, NULL, app_path, L"Log(*.log)\0*.log\0\0", NULL);
        if (ret < 0)
        {
            //dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
            break;
        }

        ret = open_log_file(file_name); 
        if (ret < 0)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret); 
            break;
        }
    } while (FALSE); 
    
    return; 
}

//void http_log_dlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//    // TODO: 在此添加消息处理程序代码和/或调用默认值
//
//    CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
//}


LRESULT http_log_dlg::insert_text(LPCWSTR text)
{
    LRESULT ret = ERROR_SUCCESS; 
    LONG start_pos;
    LONG end_pos;

    do
    {
        if (NULL == text)
        {
            ASSERT(FALSE); 
            break; 
        }

        edit_log.GetSel(start_pos, end_pos);
        edit_log.ReplaceSel(text, TRUE);
        edit_log.SetSel(start_pos, start_pos + wcslen(text));
        //UpdateData(TRUE); 
        Invalidate(FALSE);
    } while (FALSE);
    return ret; 
}

LRESULT http_log_dlg::save_log_file()
{
    LRESULT ret = ERROR_SUCCESS; 
    INT32 _ret; 
    LONG text_size;
    ULONG cc_text_length; 
    LPWSTR text = NULL;
    string _text; 
    GETTEXTLENGTHEX gtl;
    ULONG writen; 
    HANDLE file_handle = INVALID_HANDLE_VALUE; 

    do
    {
        if (script_file_name.GetLength() == 0)
        {
            break;
        }

        ATLASSERT(::IsWindow(edit_log.GetSafeHwnd()));

        gtl.codepage = 1200;
        gtl.flags = GTL_NUMBYTES;

        text_size = ::SendMessage(edit_log.GetSafeHwnd(), EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
        if (0 == text_size)
        {
            break;
        }

        text = ( LPWSTR )malloc(text_size); 

        if (NULL == text_size)
        {
            break;
        }

        cc_text_length = edit_log.GetWindowTextW(text, (text_size >> 1));
        if (0 == cc_text_length)
        {
            ASSERT(FALSE); 
            break; 
        }

        text[(text_size >> 1) - 1 ] = L'\0'; 

        ret = unicode_to_utf8_ex(text, _text); 
        if (ret != ERROR_SUCCESS)
        {
            break; 
        }

        file_handle = CreateFile(script_file_name.GetBuffer(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == file_handle)
        {
            break;
        }

        _ret = ::WriteFile(file_handle, _text.c_str(), _text.size(), (LPDWORD)&writen, NULL); 
        if (FALSE == _ret)
        {
            break; 
        }
    } while (FALSE);

    if (INVALID_HANDLE_VALUE != file_handle)
    {
        CloseHandle(file_handle); 
    }

    if (NULL != text)
    {
        free(text); 
    }

    return ret; 
}
//void http_log_dlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//    do 
//    {
//        if ((GetKeyState(VK_LCONTROL) & 0x8000)
//            || (GetKeyState(VK_RCONTROL) & 0x8000)
//            && ((nChar == L's')
//                || (nChar == L'S')))
//        {
//            save_log_file(); 
//        }
//
//    } while (FALSE);
//
//    CDialog::OnChar(nChar, nRepCnt, nFlags);
//}


void http_log_dlg::OnBnClickedButtonSave()
{
    // TODO: 在此添加控件通知处理程序代码
    save_log_file();
}
