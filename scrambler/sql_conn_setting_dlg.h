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
#include "data_analyze.h"

// sql_conn_setting_dlg 對話方塊

#define DEFAULT_MYSQL_SERVER_IP L"127.0.0.1" 
#define DEFAULT_MYSQL_SERVER_PORT L"3306"
#define DEFAULT_MYSQL_SERVER_USER_NAME L"root" 
#define DEFAULT_ECONOMIC_DATABASE_NAME L"data" 
#define DEFAULT_MYSQL_SERVER_USER_PASSWORD L""

class sql_conn_setting_dlg : public CDialog
{
	DECLARE_DYNAMIC(sql_conn_setting_dlg)

public:
	sql_conn_setting_dlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~sql_conn_setting_dlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_SQL_CONN };

protected:
	virtual BOOL OnInitDialog(); 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	virtual void OnOK(); 
	virtual void OnCancel(); 

	DECLARE_MESSAGE_MAP()

public:
	LRESULT get_sql_conn_param( SQL_CONNECTION_PARAMW **param )
	{
		ASSERT( NULL != param ); 
		*param = &conn_param; 
		return ERROR_SUCCESS; 
	}

	LRESULT set_sql_conn_param( SQL_CONNECTION_PARAM *param )
	{
		ASSERT( NULL != param ); 
	
		utf8_to_unicode_ex(param->data_base_name.c_str(), conn_param.data_base_name ); 
		utf8_to_unicode_ex(param->ip.c_str(), conn_param.ip ); 
		utf8_to_unicode_ex(param->password.c_str(), conn_param.password ); 
		conn_param.port = param->port; 
		utf8_to_unicode_ex(param->user_name.c_str(), conn_param.user_name ); 

		return ERROR_SUCCESS; 
	}

protected:
	SQL_CONNECTION_PARAMW conn_param; 
};
