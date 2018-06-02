// scrapy_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "scrapy_dlg.h"


// scrapy_dlg 對話方塊

IMPLEMENT_DYNAMIC(scrapy_dlg, CDialog)

scrapy_dlg::scrapy_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(scrapy_dlg::IDD, pParent)
{

}

scrapy_dlg::~scrapy_dlg()
{
}

/*
xpath(): 传入xpath表达式，返回该表达式所对应的所有节点的selector list列表 。
css(): 传入CSS表达式，返回该表达式所对应的所有节点的selector list列表.
extract(): 序列化该节点为unicode字符串并返回list。
re(): 根据传入的正则表达式对数据进行提取，返回unicode字符串list列表。
*/

void scrapy_dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT2_SCRIPT, edit_script);
}


BEGIN_MESSAGE_MAP(scrapy_dlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_COPY, &scrapy_dlg::OnBnClickedButtonCopy)
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &scrapy_dlg::OnBnClickedButtonOpen)
    ON_WM_KEYUP()
    ON_WM_CHAR()
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &scrapy_dlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// scrapy_dlg 訊息處理常式


BOOL scrapy_dlg::OnInitDialog()
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

        edit = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT2_SCRIPT);
        edit->SetFont(&font);

        ret = get_main_config(PREVIOUS_EDIT_SCRIPT_FILE_NAME, file_name);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ret = open_script_file(file_name.c_str()); 
    } while (FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

/******************************************************************
显示HTML结构化内容
显示元素XPATH
*******************************************************************/

LRESULT scrapy_dlg::set_target_html_element_info(PVOID html_element, LPCWSTR xpath, LPCWSTR std_xpath)
{
    LRESULT ret = ERROR_SUCCESS; 
    LONG start_pos; 
    LONG end_pos;

    do 
    {
        edit_script.GetSel(start_pos, end_pos); 
        edit_script.ReplaceSel(std_xpath, TRUE); 
        edit_script.SetSel(start_pos, start_pos+wcslen(std_xpath));
        //UpdateData(TRUE); 
        Invalidate(FALSE); 
    } while ( FALSE ); 
    return ret; 
}

void scrapy_dlg::OnBnClickedButtonCopy()
{
    CString text; 

    edit_script.GetWindowTextW(text); 
}

LRESULT WINAPI get_py_project_dir(LPWSTR path, ULONG cc_max_len, ULONG *cc_ret_len)
{
    LRESULT ret = ERROR_SUCCESS; 

    do 
    {
        *path = L'\0'; 
        *cc_ret_len = 0; 
    } while ( FALSE );

    return ret; 
}

LRESULT scrapy_dlg::open_script_file(LPCWSTR file_name)
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
        ATLASSERT(::IsWindow(edit_script.GetSafeHwnd()));

        ::SendMessage(edit_script.GetSafeHwnd(), WM_SETTEXT, 0, (LPARAM)text.c_str());

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

void scrapy_dlg::OnBnClickedButtonOpen()
{
    LRESULT ret; 
    WCHAR file_name[ MAX_PATH ]; 
    WCHAR app_path[MAX_PATH];
    ULONG cc_ret_len; 

    do
    {
        ret = get_py_project_dir(app_path, ARRAYSIZE(app_path), &cc_ret_len); 
        if (ret != ERROR_SUCCESS)
        {
            //break; 
        }

        *file_name = L'\0'; 
        ret = open_file_dlg(TRUE, L".py", file_name, NULL, app_path, L"Python Script(*.py)\0*.py\0\0", NULL);
        if (ret < 0)
        {
            //dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
            break;
        }

        ret = open_script_file(file_name); 
        if (ret < 0)
        {
            dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret); 
            break;
        }
    } while (FALSE); 
    
    return; 
}


void scrapy_dlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

LRESULT scrapy_dlg::save_script_file()
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

        ATLASSERT(::IsWindow(edit_script.GetSafeHwnd()));

        gtl.codepage = 1200;
        gtl.flags = GTL_NUMBYTES;

        text_size = ::SendMessage(edit_script.GetSafeHwnd(), EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
        if (0 == text_size)
        {
            break;
        }

        text = ( LPWSTR )malloc(text_size); 

        if (NULL == text_size)
        {
            break;
        }

        file_handle = CreateFile(script_file_name.GetBuffer(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == file_handle)
        {
            break;
        }

        cc_text_length = edit_script.GetWindowTextW(text, (text_size >> 1));
        if (0 == cc_text_length)
        {
            ASSERT(FALSE);
            break;
        }

        text[(text_size >> 1) - 1] = L'\0';

        ret = unicode_to_utf8_ex(text, _text);
        if (ret != ERROR_SUCCESS)
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
void scrapy_dlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    LRESULT ret; 

    do 
    {
        if ((GetKeyState(VK_LCONTROL) & 0x8000)
            || (GetKeyState(VK_RCONTROL) & 0x8000)
            && ((nChar == L's')
                || (nChar == L'S')))
        {
            if (script_file_name.GetLength() == 0)
            {
                WCHAR file_name[MAX_PATH];
                WCHAR app_path[MAX_PATH];
                ULONG cc_ret_len;

                //if (edit_script.GetTextLength() == 0)
                //{
                //    break;
                //}

                ret = get_py_project_dir(app_path, ARRAYSIZE(app_path), &cc_ret_len);
                if (ret != ERROR_SUCCESS)
                {
                    //break; 
                }

                *file_name = L'\0';
                ret = open_file_dlg(TRUE, L".py", file_name, NULL, app_path, L"Python Script(*.py)\0*.py\0\0", NULL);
                if (ret < 0)
                {
                    //dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
                    break;
                }

                script_file_name = file_name;
            }

            save_script_file(); 
        }

    } while (FALSE);

    CDialog::OnChar(nChar, nRepCnt, nFlags);
}


void scrapy_dlg::OnBnClickedButtonSave()
{
    LRESULT ret;

    do
    {
        // TODO: 在此添加控件通知处理程序代码
        if (script_file_name.GetLength() == 0)
        {
            WCHAR file_name[MAX_PATH];
            WCHAR app_path[MAX_PATH];
            ULONG cc_ret_len;

            //if (edit_script.GetTextLength() == 0)
            //{
            //    break;
            //}

            ret = get_py_project_dir(app_path, ARRAYSIZE(app_path), &cc_ret_len);
            if (ret != ERROR_SUCCESS)
            {
                //break; 
            }

            *file_name = L'\0';
            ret = open_file_dlg(TRUE, L".py", file_name, NULL, app_path, L"Python Script(*.py)\0*.py\0\0", NULL);
            if (ret < 0)
            {
                //dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
                break;
            }

            script_file_name = file_name;
        }

        save_script_file();
    } while (FALSE);
}


BOOL scrapy_dlg::PreTranslateMessage(MSG* pMsg)
{
    dbg_print(MSG_IMPORTANT, "message %u, wparameter %x, lparameter %x, window %p\n",
        pMsg->message,
        pMsg->wParam,
        pMsg->lParam, 
        pMsg->hwnd ); 

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && pMsg->hwnd  == edit_script.GetSafeHwnd())
    {
        CString text; 
        CString _text; 
        LONG nStart, nEnd;

        text = edit_script.GetSelText();
        edit_script.GetSel(nStart, nEnd);
        
        _text = L"\t"; 
        _text += text; 

        edit_script.ReplaceSel(_text.GetBuffer(), TRUE);
        edit_script.SetSel(nStart + 1, nStart + 1);
        return TRUE;
    }
    else if (pMsg->message == WM_KEYUP
        ) 
        //&& pMsg->hwnd == edit_script.GetSafeHwnd())
    {
        SHORT state; 
        state = GetKeyState(VK_LCONTROL); 
        if ((pMsg->wParam == 's' || pMsg->wParam == 'S')
            && ((state & 0x80)
            || (GetKeyState(VK_RCONTROL) & 0x80)))
        {
            LRESULT ret;

            do
            {
                // TODO: 在此添加控件通知处理程序代码
                if (script_file_name.GetLength() == 0)
                {
                    WCHAR file_name[MAX_PATH];
                    WCHAR app_path[MAX_PATH];
                    ULONG cc_ret_len;

                    //if (edit_script.GetTextLength() == 0)
                    //{
                    //    break; 
                    //}

                    ret = get_py_project_dir(app_path, ARRAYSIZE(app_path), &cc_ret_len);
                    if (ret != ERROR_SUCCESS)
                    {
                        //break; 
                    }

                    *file_name = L'\0';
                    ret = open_file_dlg(TRUE, L".py", file_name, NULL, app_path, L"Python Script(*.py)\0*.py\0\0", NULL);
                    if (ret < 0)
                    {
                        //dbg_print(MSG_ERROR, "%s %u error %u\n", __FUNCTION__, __LINE__, ret);
                        break;
                    }

                    script_file_name = file_name;
                }

                save_script_file();
            } while (FALSE);
        }
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}
