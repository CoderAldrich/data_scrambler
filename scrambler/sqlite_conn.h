#ifndef __SQLITE_CONN_H__
#define __SQLITE_CONN_H__

LRESULT WINAPI init_sqlite(); 

LRESULT WINAPI uninit_sqlite(); 

LRESULT WINAPI connect_2_sqlite( LPCWSTR db_name, wstring &error_message ); 

LRESULT WINAPI _connect_2_sqlite( LPCWSTR db_name ); 

LRESULT WINAPI construct_sqlite_insert_sql( string &statement, 
										   LPCWSTR table_name, 
										   HTML_ELEMENT_ACTIONS *contents, 
										   vector<string> &blob ); 

LRESULT WINAPI sqlite_create_table_sql( string &statement_out, 
										  LPCWSTR table_name, 
										  HTML_ELEMENT_ACTIONS *contents); 

LRESULT WINAPI sqlite_exec_statement( LPCSTR statement ); 

LRESULT WINAPI sqlite_exec_statement_ex( LPCSTR statement, vector<string> &blob ); 

LRESULT WINAPI close_sqlite(); 

LRESULT WINAPI check_sqlite_db_conn(LPCSTR ip, 
								   ULONG port, 
								   LPCSTR user,
								   LPCSTR pwd, 
								   LPCSTR db_name, 
								   string &error_string ); 

#endif //__SQLITE_CONN_H__