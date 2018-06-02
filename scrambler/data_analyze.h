/*
 * 
 * Copyright (c) 2003, Michael Carruth All rights reserved.
 * This file is a part of CrashRpt library.
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

#ifndef __DATA_ANALYZE_H__
#define __DATA_ANALYZE_H__

#include <map>
#include "mysql_conn.h" 

typedef enum _DATA_OUTPUT_TYPE
{
	DATA_OUTPUT_CSV_FILE, 
	DATA_OUTPUT_JSON_FILE, 
	DATA_OUTPUT_ADO, 
	MAX_DATA_OUTPUT_TYPE, 
} DATA_OUTPUT_TYPE, *PDATA_OUTPUT_TYPE; 

typedef struct _DATA_STORE
{
	BOOLEAN have_db_conn; 
	wstring data_file_name; 
	HANDLE data_file; 
	HANDLE default_data_file; 
	HANDLE link_list_file; 
	wstring table_name; 
} DATA_STORE, *PDATA_STORE; 

typedef struct _DATA_STORE_PARAM
{
#define MAX_DB_CONN_STRING 1024
	SQL_CONNECTION_PARAM sql_connection; 

	DATA_OUTPUT_TYPE type; 
	BOOLEAN to_data_center; 
	BOOLEAN output_link_list; 

	wstring output_path; 
	wstring sql_table_name; 
} DATA_STORE_PARAM, *PDATA_STORE_PARAM; 

typedef struct _CONTAINER_INFO
{
	DATA_STORE_PARAM store_param; 
	DATA_STORE store; 
	HTML_ELEMENT_ACTIONS content; 
	wstring name; 
	BOOLEAN have_ui; 
    BOOLEAN _using; 
} CONTAINER_INFO, *PCONTAINER_INFO; 

typedef vector<CONTAINER_INFO*> CONTAINER_INFOS; 
typedef CONTAINER_INFOS::iterator CONTAINER_INFO_ITERATOR; 
typedef map< ULONG, CONTAINER_INFO* > CONTAINER_INFO_MAP;
typedef CONTAINER_INFO_MAP::iterator CONTAINER_INFO_MAP_ITERATOR; 
typedef pair< ULONG, CONTAINER_INFO* > CONTAINER_INFO_MAP_PAIR; 

#define DEFAULT_SCRAMBLED_FILE_NAME L"\\csv\\scrambled_data.csv"
#define TO_DATA_CENTER_FILE_NAME L"\\csv\\data_center.csv"

INLINE LRESULT WINAPI init_sql_conn_param( SQL_CONNECTION_PARAM *sql_connection)
{
	sql_connection->ip = ""; 
	sql_connection->port = 0; 
	sql_connection->user_name = ""; 
	sql_connection->password = ""; 
	sql_connection->data_base_name = ""; 
	return ERROR_SUCCESS; 
}

INLINE LRESULT WINAPI init_data_data_store_param( DATA_STORE_PARAM *param )
{
	LRESULT ret = ERROR_SUCCESS; 

	param->output_path = L""; 
	param->sql_table_name = L""; 

	param->to_data_center = FALSE; 
	param->output_link_list = TRUE; 
	param->type = MAX_DATA_OUTPUT_TYPE; 

	init_sql_conn_param( &param->sql_connection ); 
	
	return ret; 
}

LRESULT WINAPI create_data_store( DATA_STORE_PARAM *param, DATA_STORE *store, wstring &error_string ); 

LRESULT WINAPI open_data_store( DATA_STORE_PARAM *param, DATA_STORE *store ); 

LRESULT WINAPI uninit_data_store( DATA_STORE *store ); 

HRESULT WINAPI get_csv_line_text( HTML_ELEMENT_ACTIONS *page_content, 
								 wstring *text ); 
HRESULT WINAPI get_json_line_text( HTML_ELEMENT_ACTIONS *page_content, 
								  wstring *text ); 

//HRESULT WINAPI get_web_page_content_sql( HTML_PAGE_CONTENT *page_content, 
//										wstring *text ); 

LRESULT WINAPI get_data_center_file_name( LPWSTR file_name, 
										 ULONG cc_buf_len, 
										 ULONG *cc_ret_len ); 

#define ERROR_CSV_FILE_HEADER_MISMATCH 0xea888081

LRESULT WINAPI init_csv_file( HTML_ELEMENT_ACTIONS *web_content, DATA_STORE *store ); 

LRESULT WINAPI config_data_format( HTML_ELEMENT_ACTIONS *content, 
						   DATA_STORE *store ); 

LRESULT WINAPI init_data_store( DATA_STORE_PARAM *param, 
						HTML_ELEMENT_ACTIONS *content, 
						DATA_STORE *store ); 

LRESULT WINAPI create_mysql_table( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content ); 

LRESULT WINAPI create_sqlite_table( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content ); 

#define OUTPUT_URLS_DELIM L"==>"

LRESULT WINAPI input_links( DATA_STORE *store, 
						   LPCWSTR xpath, 
						   LPCWSTR link_list ); 


LRESULT WINAPI output_data( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content); 

LRESULT WINAPI analyze_data_info(); 

//#define PREVIOUS_SCRIPT_FILE_NAME L"prev_script" 
//LRESULT WINAPI set_previous_script_file_name( DATA_STORE *store, LPCWSTR file_name ); 
LRESULT WINAPI __make_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len ); 

//LRESULT WINAPI _send_data_file( DATA_STORE_PARAM *param ); 

LRESULT WINAPI check_and_send_data( LPCWSTR file_name ); 

LRESULT WINAPI post_data_file( LPCWSTR file_name ); 

LRESULT WINAPI check_file_data_quantity( LPCWSTR file_name, ULONG *data_count); 

LRESULT WINAPI allocate_html_content_info( HTML_ELEMENT_ACTION *container, 
										   CONTAINER_INFO **container_info );  

LRESULT WINAPI get_html_content_info_ex( HTML_ELEMENT_ACTION *container, 
										   CONTAINER_INFO **conent_info, 
										   ULONG flags ); 

LRESULT WINAPI release_container_info( CONTAINER_INFO *container_info ); 

LRESULT WINAPI check_data_store( DATA_STORE *store ); 

LRESULT WINAPI init_default_data_store_settings( DATA_STORE_PARAM *param, 
										 LPCWSTR data_file_name ); 

LRESULT WINAPI get_html_content_info( HTML_ELEMENT_ACTION *container, 
									 CONTAINER_INFO **content ); 

LRESULT WINAPI release_container_content_info( HTML_ELEMENT_ACTIONS *page_content ); 

LRESULT WINAPI post_scrambled_data(); 

LRESULT WINAPI release_all_html_page_data_store(); 
LRESULT WINAPI release_all_html_conainer_infos(); 

LRESULT WINAPI format_sql_field_text( wstring &text ); 
LRESULT WINAPI format_csv_field_text( wstring &text ); 

#endif //__DATA_ANALYZE_H__