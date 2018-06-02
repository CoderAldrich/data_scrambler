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
#include "stdafx.h"
#include <winsock.h>
#include <mysql.h>
#include "mysql_conn.h"

//#pragma comment(lib,"mysqlclient.lib") 
#pragma comment(lib,"libmysql.lib") 
//#pragma comment(lib,"libmysqld.lib") 

static MYSQL mysql; 


LRESULT WINAPI _init_mysql( MYSQL *handle )

{
	LRESULT ret = ERROR_SUCCESS; 
	MYSQL *_ret; 

	do 
	{
		ASSERT( NULL != handle); 
		_ret = mysql_init( handle ); 

		if( _ret  == NULL ) 
		{
			dbg_print( MSG_FATAL_ERROR, "initialize mysql function error %s\n",  mysql_error( handle ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	}while( FALSE ); 
	
	return ret; 
}

static MYSQL mysql_api; 

LRESULT WINAPI init_mysql_api()
{
	return _init_mysql(&mysql_api); 
}

ULONG WINAPI _mysql_real_escape_string( LPSTR escaped_text, LPCSTR original_text, ULONG cc_original_text_len)
{
	ULONG string_len; 
	string_len = mysql_real_escape_string( &mysql_api, 
		escaped_text, 
		original_text, 
		cc_original_text_len ); 

	return string_len; 
}

LRESULT WINAPI _connect_2_mysql( LPCSTR ip, 
							   ULONG port, 
							   LPCSTR user,
							   LPCSTR pwd, 
							   LPCSTR db_name, 
							   MYSQL *handle )

{
	LRESULT ret = ERROR_SUCCESS; 
	MYSQL *_ret; 
	INT32 __ret; 

	do 
	{
		ASSERT( handle != NULL ); 

		_ret = mysql_init( handle ); 

		if( _ret  == NULL ) 
		{
			dbg_print( MSG_FATAL_ERROR, "initialize mysql function error %s\n",  mysql_error( handle ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		__ret = mysql_set_character_set( handle, "utf8");
		if( __ret != 0 )
		{
			dbg_print( MSG_FATAL_ERROR, "initialize mysql function error %s\n",  mysql_error( handle ) ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		//ret = mysql_real_query(handle, "SET NAMES UTF8;", 
		//	( ULONG )CONST_STR_LEN( "SET NAMES UTF8;" ) );

		//ret = mysql_options(handle, MYSQL_SET_CHARSET_NAME, "utf8");
		//L"CREATE DATABASE db_name DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;" 
		_ret = mysql_real_connect( handle, ip, user, pwd, db_name, port, NULL, 0 ); 

		if( NULL == _ret )
		{
			dbg_print( MSG_FATAL_ERROR, "connect to mysql error:%s\n", mysql_error( handle ) );  
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		dbg_print( MSG_INFO, "connect to mysql successfully\n" ); 
 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI connect_2_mysql( LPCSTR ip, 
							   ULONG port, 
							   LPCSTR user,
							   LPCSTR pwd, 
							   LPCSTR db_name, 
							   wstring &error_string )
{
	LRESULT ret; 
	
	do 
	{
		ret = _connect_2_mysql( ip, port, user, pwd, db_name, &mysql ); 
		if( ret != ERROR_SUCCESS )
		{
			string _error_string; 

			_error_string = mysql_error( &mysql ); 

			utf8_to_unicode_ex( _error_string.c_str(), error_string ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI construct_mysql_insert_sql( string &statement, 
										  LPCWSTR table_name, 
										  HTML_ELEMENT_ACTIONS *contents)
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	LPSTR ansi_text = NULL; 
	LPWSTR unicode_text = NULL; 
	string field_text; 
	ULONG string_len; 

	do 
	{
		ASSERT( contents != NULL ); 

#define COMMON_WEB_PAGE_HTML_TEXT_LEN 65535
#define MAX_SQL_STATEMENT_SEGMENT 512 
		ansi_text = ( LPSTR )malloc( sizeof( CHAR ) * COMMON_WEB_PAGE_HTML_TEXT_LEN ); 
		if( NULL == ansi_text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		unicode_text = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SQL_STATEMENT_SEGMENT ); 
		if( NULL == unicode_text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		ret = unicode_to_utf8_ex( table_name, field_text ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchPrintfA( ansi_text, 
			MAX_SQL_STATEMENT_SEGMENT, 
			"INSERT INTO `%s`(", 
			field_text.c_str() ); 

		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		statement = ansi_text; 

		it = contents->begin(); 
		for( ; ; )
		{
			//if( NULL == ( *it )->action )
			//{
			//	ret = ERROR_INVALID_PARAMETER; 
			//	break; 
			//}

			if( it == contents->end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 

			if( ( *it )->name.length() == 0 )
			{
				ASSERT( FALSE ); 
				ret = ERROR_ERRORS_ENCOUNTERED; 
				StringCchPrintfW( unicode_text, MAX_SQL_STATEMENT_SEGMENT, L"field%u", ( ULONG )( ( *it )->id ) ); 
				( *it )->name = unicode_text; 
				break; 
			}

			if( it != contents->begin() )
			{
				statement.append( "," ); 
			}

			ret = unicode_to_utf8_ex( ( *it )->name.c_str(), field_text ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			statement.append( "`" ); 
			statement.append( field_text.c_str() ); 
			statement.append( "`" ); 

			it ++; 
		}

		statement.append( ")" ); 
		statement.append( " VALUES(" ); 

		it = contents->begin(); 
		for( ; ; )
		{
			if( it == contents->end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 

			//if( *it->text.length() == 0 )
			//{
			//}

			if( it != contents->begin() )
			{
				statement.append( "," ); 
			}

            {
                wstring output_data;
                cat_output_data((*it), output_data);

                ret = unicode_to_utf8_ex(output_data.c_str(), field_text);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }
            }

			if( field_text.length() * 2 + 1 > COMMON_WEB_PAGE_HTML_TEXT_LEN )
			{
				dbg_print( MSG_IMPORTANT, "field text too long %u>%u\n", field_text.length(), COMMON_WEB_PAGE_HTML_TEXT_LEN ); 

				free( ansi_text ); 
				ansi_text = ( CHAR* )malloc( field_text.length() * 2 + 1 ); 
				if( NULL == ansi_text )
				{
					ret = ERROR_NOT_ENOUGH_MEMORY; 
					break; 
				}
			}

			string_len = mysql_real_escape_string( &mysql, 
				ansi_text, 
				field_text.c_str(), 
				field_text.length() ); 
			if( string_len < field_text.length() )
			{
				ASSERT( FALSE ); 
			}

			statement.append( "'" ); 
			statement.append( ansi_text ); 
			statement.append( "'" ); 

			it ++; 
		}

		statement.append( ");" ); 
	} while ( FALSE );

	if( NULL != unicode_text )
	{
		free( unicode_text ); 
	}

	if( NULL != ansi_text )
	{
		free( ansi_text ); 
	}

	return ret; 
}

LRESULT WINAPI construct_create_table_mysql( string &statement_out, 
										  LPCWSTR table_name, 
										  HTML_ELEMENT_ACTIONS *contents)
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	BOOLEAN table_same = FALSE; 
	HRESULT hr; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	LPWSTR text = NULL; 
	wstring statement; 
	string _statement; 

	do 
	{
		ASSERT( contents != NULL ); 
		ASSERT( 0 < contents->size() ); 

#define MAX_SQL_STATEMENT_SEGMENT 512 
		text = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SQL_STATEMENT_SEGMENT ); 
		if( NULL == text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		do 
		{
			my_ulonglong row_count; 
			MYSQL_RES *result = NULL; 

			statement = L"SELECT table_name FROM information_schema.TABLES WHERE table_name ='"; 
			statement += table_name; 
			statement += L"\';"; 

			ret = unicode_to_utf8_ex( statement.c_str(), _statement ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			//SHOW TABLES LIKE '%tb_bp_d_case%';
			//select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_SCHEMA='dbname' and TABLE_NAME='tablename' ;
			//"update mytable set username=\'%s\',visitelist=\'%s\', remark=\'%s\' where username=\'%s\'"
			//不能确定表的字段名称之间的相互关系，所以不使用UPDATE方式

			_ret = mysql_query( &mysql, _statement.c_str() );
			if( _ret != 0 )
			{
				dbg_print( MSG_FATAL_ERROR, "insert row to mysql error: %s", mysql_error( &mysql ) ); 

				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			result = mysql_store_result( &mysql ); 
			if( result == NULL )
			{
				dbg_print( MSG_FATAL_ERROR, "insert row to mysql error: %s", mysql_error( &mysql ) ); 
				/***************************
				mysql_errno() 
				CR_COMMANDS_OUT_OF_SYNC

				Commands were executed in an improper order.

				CR_OUT_OF_MEMORY

				Out of memory.

				CR_SERVER_GONE_ERROR

				The MySQL server has gone away.

				CR_SERVER_LOST

				The connection to the server was lost during the query.

				CR_UNKNOWN_ERROR
				********************************/
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			row_count = mysql_num_rows( result ); 
			mysql_free_result( result ); 

			if( row_count == 0 )
			{
				break; 
			}

			//SHOW TABLES LIKE '%表名%';
			it = contents->begin(); 
			for( ; ; )
			{
				if( it == contents->end() )
				{
					table_same = TRUE; 
					break; 
				}

				ASSERT( ( *it ) != NULL ); 

				if( ( *it )->name.length() == 0 )
				{
					StringCchPrintfW( text, MAX_SQL_STATEMENT_SEGMENT, L"colume%u", ( ULONG )( ULONG_PTR )( it - contents->begin() ) ); 
					( *it )->name = text; 
				}

				statement = L"DESCRIBE "; 
				statement += table_name; 
				statement += L" "; 
				statement += ( *it )->name; 
				statement += ';'; 

				unicode_to_utf8_ex( statement.c_str(), _statement ); 

				_ret = mysql_query( &mysql, _statement.c_str() );
				if( _ret != 0 )
				{
					dbg_print( MSG_FATAL_ERROR, "insert row to mysql error: %s", mysql_error( &mysql ) ); 

					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				result = mysql_store_result( &mysql ); 
				if( result == NULL )
				{
					dbg_print( MSG_FATAL_ERROR, "insert row to mysql error:%s", mysql_error( &mysql ) ); 
					/***************************
					mysql_errno() 
					CR_COMMANDS_OUT_OF_SYNC

					Commands were executed in an improper order.

					CR_OUT_OF_MEMORY

					Out of memory.

					CR_SERVER_GONE_ERROR

					The MySQL server has gone away.

					CR_SERVER_LOST

					The connection to the server was lost during the query.

					CR_UNKNOWN_ERROR
					********************************/

					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}
				else
				{
					row_count = mysql_num_rows( result ); 
					mysql_free_result( result ); 
				}

				if( row_count == 0 )
				{
					ULONG i; 
#define MAX_DUPLICATE_TABLE_BACKUP 200
					for( i = 0; i < MAX_DUPLICATE_TABLE_BACKUP; i ++ )
					{
						//alter table customers rename custs
						StringCchPrintfW( text, MAX_SQL_STATEMENT_SEGMENT, L"%s%u", table_name, i ); 

						statement = L"RENAME TABLE "; 
						statement += table_name; 
						statement += L" TO "; 
						statement += text; 
						statement += ';'; 

						ret = unicode_to_utf8_ex( statement.c_str(), _statement ); 
						if( ret != ERROR_SUCCESS )
						{
							break; 
						}

						_ret = mysql_query( &mysql, _statement.c_str() );
						if( _ret == 0 )
						{
							break; 
						}

						dbg_print( MSG_FATAL_ERROR, "change table name from %s to %s error:%s\n", 
							table_name, 
							text, 
							mysql_error( &mysql ) ); 
					}

					if( _ret != 0 ) // i == MAX_DUPLICATE_TABLE_BACKUP )
					{
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					break; 
				}

				it ++; 
			}
		}while( FALSE ); 

		if( table_same == TRUE )
		{
			ret = ERROR_ALREADY_EXISTS; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		//show columns from tablename like columnname 
		//$field = mysql_fetch_array($field);  

		//6 IF NOT EXISTS (SELECT * FROM information_schema.columns WHERE table_schema=CurrentDatabase AND table_name = 'rtc_order' AND column_name = 'IfUpSend') THEN  
		//	7     ALTER TABLE rtc_order
		//	8     ADD COLUMN `IfUpSend` BIT  NOT NULL  DEFAULT 0 COMMENT '是否上传 是否上传';

		//alte table A add TEL_number char(10)
		hr = StringCchPrintfW( text, 
			MAX_SQL_STATEMENT_SEGMENT, 
			L"CREATE TABLE %s(", //IF NOT EXISTS
			table_name ); 

		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		statement = text; 

		statement.append( L"id INT(4) NOT NULL PRIMARY KEY AUTO_INCREMENT" ); 

	//	create table MyClass(
	//> id int(4) not null primary key auto_increment,
	//> name char(20) not null,
	//> sex int(4) not null default '0',
	//> degree double(16,2)); 
		it = contents->begin(); 
		for( ; ; )
		{
			if( it == contents->end() )
			{
				break; 
			}

			ASSERT( ( *it ) != NULL ); 

			if( ( *it )->name.length() == 0 )
			{
				StringCchPrintfW( text, MAX_SQL_STATEMENT_SEGMENT, L"colume%u", ( ULONG )( ULONG_PTR )( it - contents->begin() ) ); 
				( *it )->name = text; 
			}

			statement.append( L",`" ); 
			statement.append( ( *it )->name.c_str() ); 
			//if( ( *it )->max_text_size < MAX_LONG_TEXT_SIZE )
			//{
			//	statement.append( L"` TEXT" ); //DEFAULT \'\'
			//}
			//else
			{
				statement.append( L"` LONGTEXT" ); //DEFAULT \'\'
			}

			it ++; 
		}

		statement.append( L") DEFAULT CHARSET=utf8;" ); 

		ret = unicode_to_utf8_ex( statement.c_str(), statement_out ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	} while ( FALSE );

	if( NULL != text )
	{
		free( text ); 
	}

	return ret; 
}

LRESULT WINAPI mysql_exec_statement( LPCSTR statement )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	string _statement; 

	do 
	{
		ASSERT( NULL != statement ); 

		_ret = mysql_query( &mysql, statement ); 
		if( 0 == _ret )
		{
			dbg_print( MSG_INFO, "insert row to mysql successfully\n", statement ); 
		}
		else
		{
			//statement, 
			dbg_print( MSG_FATAL_ERROR, "insert row to mysql error：%s", mysql_error( &mysql ) ); 
		}
	} while ( FALSE ); 

	return ret; 
}

//CString sqlstr="SELECT * FROM vctest";
//
//CString loginfo="";
//
//MYSQL_RES *result=NULL;
//
//if(0==mysql_query(&mysql,sqlstr))
//
//{
//
//	loginfo="mysql_query() select data succeed";
//
//	MessageBox(loginfo);
//
//	//一次性取得数据集
//
//	result=mysql_store_result(&mysql);
//
//
//
//	//取得并打印行数
//
//	int rowcount=mysql_num_rows(result);
//
//	loginfo.Format("row count=%d",rowcount);
//
//	MessageBox(loginfo);       
//
//
//
//	//取得并打印各字段的名称
//
//	unsigned int fieldcount=mysql_num_fields(result);
//
//	MYSQL_FIELD *field=NULL;
//
//	loginfo="";
//
//	for(unsignedint i=0;i<fieldcount;i++)
//
//	{
//
//		field=mysql_fetch_field_direct(result,i);
//
//		loginfo=loginfo+(field->name)+",";
//
//	}
//
//	MessageBox(loginfo);       
//
//
//
//	//打印各行
//
//	MYSQL_ROW row=NULL;
//
//	row=mysql_fetch_row(result);
//
//	loginfo="";
//
//	while(NULL!=row)
//
//	{
//
//		for(inti=0; i<fieldcount;i++)
//
//		{
//
//			loginfo=loginfo+(row[i])+",";
//
//		}
//
//		loginfo=loginfo+";";
//
//		row=mysql_fetch_row(result);
//
//	}
//
//	MessageBox(loginfo);
//
//}    
//
//else {MessageBox("mysql_query() select data failed");       }

LRESULT WINAPI _close_mysql( MYSQL *handle )
{
	ASSERT( NULL != handle ); 
	mysql_close( handle ); 

	return ERROR_SUCCESS; 
}

LRESULT WINAPI close_mysql()
{
	_close_mysql( &mysql ); 

	return ERROR_SUCCESS; 
}

LRESULT WINAPI check_mysql_db_conn(LPCSTR ip, 
								   ULONG port, 
								   LPCSTR user,
								   LPCSTR pwd, 
								   LPCSTR db_name, 
								   string &error_string ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	MYSQL handle; 

	do 
	{
		ASSERT( NULL != ip ); 
		ASSERT( 0 != port ); 
		ASSERT( NULL != user ); 
		ASSERT( NULL != pwd ); 
		ASSERT( NULL != db_name ); 

		ret = _connect_2_mysql( ip, port, user, pwd, db_name, &handle ); 
		if( ret != ERROR_SUCCESS )
		{
			error_string = mysql_error( &handle ); 
			break; 
		}

		error_string.clear(); 
	}while( FALSE );

	_close_mysql( &handle ); 

	return ret; 
}

LRESULT WINAPI parse_sql_conn_param( LPCWSTR variable_name, LPCWSTR variable_value, SQL_CONNECTION_PARAM *conn_param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != variable_name ); 
		ASSERT( NULL != variable_value ); 

		if( 0 == wcscmp( variable_name, L"SERVER" ) ) 
		{
			if( *variable_value == L'\0' )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			ret = unicode_to_utf8_ex( variable_value, conn_param->ip ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else if( 0 == wcscmp( variable_name, L"PORT" ) ) 
		{
			LPWSTR tmp_text; 

			if( *variable_value == L'\0' )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			conn_param->port = ( USHORT )wcstoul( variable_value, &tmp_text, 0 ); 
		}
		else if( 0 == wcscmp( variable_name, L"DATABASE" ) ) 
		{
			if( *variable_value == L'\0' )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			ret = unicode_to_utf8_ex( variable_value, conn_param->data_base_name ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else if( 0 == wcscmp( variable_name, L"USER" ) ) 
		{
			if( *variable_value == L'\0' )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			ret = unicode_to_utf8_ex( variable_value, conn_param->user_name ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else if( 0 == wcscmp( variable_name, L"PASSWORD" ) ) 
		{
			ret = unicode_to_utf8_ex( variable_value, conn_param->password ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else if( 0 == wcscmp( variable_name, L"OPTION" ) ) 
		{
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI get_sql_connection_info_from_string( SQL_CONNECTION_PARAM *param, 
												   LPCWSTR sql_conn_string )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	LPWSTR _sql_conn_string = NULL; 
	LPWSTR variable_assign_begin; 
	LPWSTR variable_assign_end; 
	LPWSTR variable_name_end; 
	LPWSTR variable_value_begin; 
	ULONG sql_conn_string_len; 
	SQL_CONNECTION_PARAM conn_param; 

	do 
	{
		sql_conn_string_len = wcslen( sql_conn_string ); 
		if( sql_conn_string_len == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_sql_conn_string = ( LPWSTR )malloc( ( sql_conn_string_len + 1 ) << 1 ); 
		if( NULL == _sql_conn_string )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		memcpy( _sql_conn_string, sql_conn_string, ( ( sql_conn_string_len + 1 ) << 1 ) ); 

		conn_param.port = 0; 
		variable_assign_begin = _sql_conn_string; 

		for( ; ; )
		{
			variable_assign_end = wcschr( variable_assign_begin, L';'); 
			if( variable_assign_end == NULL )
			{
				variable_name_end = wcschr( variable_assign_begin, L'='); 
				if( variable_name_end == NULL )
				{
					break; 
				}

				*variable_name_end = L'\0'; 

				variable_value_begin = variable_name_end + 1; 
				_ret = parse_sql_conn_param( variable_assign_begin, 
					variable_value_begin, 
					&conn_param ); 

				if( _ret != ERROR_SUCCESS )
				{
					ret = _ret; 
					break; 
				}

				break; 
			}
			else
			{
				*variable_assign_end = L'\0'; 

				variable_name_end = wcschr( variable_assign_begin, L'='); 
				if( variable_name_end == NULL )
				{
					break; 
				}

				*variable_name_end = L'\0'; 

				variable_value_begin = variable_name_end + 1; 
				_ret = parse_sql_conn_param( variable_assign_begin, 
					variable_value_begin, 
					&conn_param ); 

				if( _ret != ERROR_SUCCESS )
				{
					ret = _ret; 
					break; 
				}
			}

			variable_assign_begin = variable_assign_end + 1; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( conn_param.data_base_name.length() == 0 
			|| conn_param.ip.length() == 0 
			|| conn_param.user_name.length() == 0 
			|| conn_param.port == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		param->data_base_name = conn_param.data_base_name; 
		param->ip = conn_param.ip; 
		param->password = conn_param.password; 
		param->port = conn_param.port; 
		param->user_name = conn_param.user_name; 

	}while( FALSE );

	if( NULL != _sql_conn_string )
	{
		free( _sql_conn_string ); 
	}

	return ret; 
}