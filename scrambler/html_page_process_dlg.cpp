// html_page_process_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "html_page_process_dlg.h"


// html_page_process_dlg 對話方塊

IMPLEMENT_DYNAMIC(html_page_process_dlg, CDialog)

html_page_process_dlg::html_page_process_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(html_page_process_dlg::IDD, pParent)
{
    set_logger(( log_manager*)this); 
}

html_page_process_dlg::~html_page_process_dlg()
{
    set_logger((log_manager*)NULL);
}

void html_page_process_dlg::DoDataExchange(CDataExchange* pDX)
{

	DDX_Control(pDX, IDC_MODE_TAB, mode_tab);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(html_page_process_dlg, CDialog)
END_MESSAGE_MAP()


// html_page_process_dlg 訊息處理常式

BOOL html_page_process_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此加入額外的初始化

	mode_tab.ModifyStyle(0, TCS_OWNERDRAWFIXED | TCS_HOTTRACK);
	mode_tab.InsertItem(0, L"HTML处理脚本编辑");
	mode_tab.InsertItem(1, L"SCRAPY脚本编辑");
    mode_tab.InsertItem(2, L"HTTP请求日志"); //(AJAX请求分析)

	html_command_dlg.Create(MAKEINTRESOURCE(html_command_dlg.IDD), &mode_tab);
	python_scrapy_dlg.Create(MAKEINTRESOURCE(python_scrapy_dlg.IDD), &mode_tab);
    log_dlg.Create(MAKEINTRESOURCE(log_dlg.IDD), &mode_tab);

	mode_tab.SetItemForm(&html_command_dlg, 0);
	mode_tab.SetItemForm(&python_scrapy_dlg, 1);
    mode_tab.SetItemForm(&log_dlg, 2);

	mode_tab.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX 屬性頁應傳回 FALSE
}

LRESULT html_page_process_dlg::on_html_document_complete(LPCWSTR url)
{
    LRESULT ret = ERROR_SUCCESS;


    do 
    {
        ret = html_command_dlg.on_html_document_complete(url); 
    } while ( FALSE );

    return ret; 
}

LRESULT html_page_process_dlg::set_target_html_element_info(PVOID html_element, LPCWSTR xpath, LPCWSTR std_xpath)
{
    LRESULT ret = ERROR_SUCCESS; 
    do 
    {
        switch (mode_tab.GetCurSel())
        {

        case 0:
            html_command_dlg.set_html_element(html_element);
            html_command_dlg.set_html_element_xpath(xpath);
            html_command_dlg.init_element_properties(std_xpath);
            break;
        case 1:
            python_scrapy_dlg.set_target_html_element_info(html_element, xpath, std_xpath);
            break;
        }

    } while ( FALSE ); 

    return ret; 
}

LRESULT html_page_process_dlg::append_log(ULONG type, LPCWSTR text )
{
    LRESULT ret = ERROR_SUCCESS;
    do
    {
        UNREFERENCED_PARAMETER(type); 

        //switch (mode_tab.GetCurSel())
        //{
        //case 2:
            log_dlg.append_log(text);
            //break;
        //}

    } while (FALSE);

    return ret;
}