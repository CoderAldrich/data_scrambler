// text_output_dlg.cpp : 實作檔
//

#include "stdafx.h"
#include "data_scrambler.h"
#include "text_output_dlg.h"


// text_output_dlg 對話方塊

IMPLEMENT_DYNAMIC(text_output_dlg, CDialog)

text_output_dlg::text_output_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(text_output_dlg::IDD, pParent)
{

}

text_output_dlg::~text_output_dlg()
{
}

void text_output_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(text_output_dlg, CDialog)
END_MESSAGE_MAP()


// text_output_dlg 訊息處理常式
