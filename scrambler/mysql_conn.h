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
#ifndef __MYSQL_CONN_H__
#define __MYSQL_CONN_H__

typedef struct _SQL_CONNECTION_PARAM
{
	string ip; 
	USHORT port; 
	string user_name; 
	string password; 
	string data_base_name; 
} SQL_CONNECTION_PARAM, *PSQL_CONNECTION_PARAM; 

typedef struct _SQL_CONNECTION_PARAMW
{
	wstring ip; 
	USHORT port; 
	wstring user_name; 
	wstring password; 
	wstring data_base_name; 
} SQL_CONNECTION_PARAMW, *PSQL_CONNECTION_PARAMW; 

LRESULT WINAPI get_sql_connection_info_from_string( SQL_CONNECTION_PARAM *param, 
												   LPCWSTR sql_conn_string ); 

LRESULT WINAPI check_mysql_db_conn(LPCSTR ip, 
								   ULONG port, 
								   LPCSTR user,
								   LPCSTR pwd, 
								   LPCSTR db_name, 
								   string &error_string ); 

LRESULT WINAPI construct_create_table_mysql( string &statement_out, 
											LPCWSTR table_name, 
											HTML_ELEMENT_ACTIONS *contents); 

LRESULT WINAPI mysql_exec_statement( LPCSTR statement ); 


LRESULT WINAPI init_mysql_api(); 

ULONG WINAPI _mysql_real_escape_string( LPSTR escaped_text, LPCSTR original_text, ULONG cc_original_text_len); 

LRESULT WINAPI connect_2_mysql( LPCSTR ip, 
							   ULONG port, 
							   LPCSTR user,
							   LPCSTR pwd, 
							   LPCSTR db_name, 
							   wstring &error_string ); 

LRESULT WINAPI close_mysql(); 

#endif //__MYSQL_CONN_H__