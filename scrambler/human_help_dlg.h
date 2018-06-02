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

#pragma once
#include "hyper_link.h"

// human_help_dlg 对话框

class human_help_dlg : public CDialog
{
	DECLARE_DYNAMIC(human_help_dlg)

public:
	human_help_dlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~human_help_dlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_HELP };
#endif

protected:
    CHyperLink push_link;
    CHyperLink push_link2;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnStnClickedStaticBuyLink1();
    afx_msg void OnStnClickedStaticBuyLink2();
    virtual BOOL OnInitDialog();
};
