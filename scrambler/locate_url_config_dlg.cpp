// locate_url_config_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "locate_url_config_dlg.h"


// locate_url_config_dlg 對話方塊

IMPLEMENT_DYNAMIC(locate_url_config_dlg, CDialog)

locate_url_config_dlg::locate_url_config_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(locate_url_config_dlg::IDD, pParent)
{

}

locate_url_config_dlg::~locate_url_config_dlg()
{
}

void locate_url_config_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL locate_url_config_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText( IDC_EDIT_LOCATE_URL, locate_url.c_str() ); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(locate_url_config_dlg, CDialog)
END_MESSAGE_MAP()


// locate_url_config_dlg 訊息處理常式
