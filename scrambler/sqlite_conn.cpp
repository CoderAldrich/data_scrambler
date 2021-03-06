#include "stdafx.h"
#include "sqlite3.h"
#include "sqlite_conn.h"
#include "SQLite.h"
//#include <mysql.h>
#include "mysql_conn.h"
using namespace SQLite; 

//#include "sqlite3ext.h"
//#pragma comment(lib,"sqlite.lib") 

Database sqlite_db; 

//#if defined(_WIN32) || defined(WIN32)
//
///* string conversion routines only needed on Win32 */
//extern char *sqlite3_win32_unicode_to_utf8(LPCWSTR);
//extern char *sqlite3_win32_mbcs_to_utf8_v2(const char *, int);
//extern char *sqlite3_win32_utf8_to_mbcs_v2(const char *, int);
//extern LPWSTR sqlite3_win32_utf8_to_unicode(const char *zText);
//#endif
//
//#if defined(_WIN32) || defined(WIN32)
//void utf8_printf(FILE *out, const char *zFormat, ...){
//	va_list ap;
//	va_start(ap, zFormat);
//	if((out==stdout || out==stderr) ){
//		char *z1 = sqlite3_vmprintf(zFormat, ap);
//		char *z2 = sqlite3_win32_utf8_to_mbcs_v2(z1, 0);
//		sqlite3_free(z1);
//		fputs(z2, out);
//		sqlite3_free(z2);
//	}else{
//		vfprintf(out, zFormat, ap);
//	}
//	va_end(ap);
//}
//#elif !defined(utf8_printf)
//# define utf8_printf fprintf
//#endif

LRESULT WINAPI init_sqlite()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = sqlite3_initialize();
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = init_mysql_api(); 
		if( ret != ERROR_SUCCESS)
		{
			break; 
		}

	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI uninit_sqlite()
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		_ret = sqlite3_shutdown();
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI _connect_2_sqlite( LPCWSTR db_name )

{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	WCHAR db_file_name[ MAX_PATH ]; 
	//wstring _db_file_name; 
	ULONG cc_ret_len; 

	do 
	{
		ASSERT( db_name != NULL );

		if( sqlite_db.IsOpen() == true )
		{
			dbg_print( MSG_FATAL_ERROR, "%s %i error\n", __FUNCTION__, __LINE__ ); 
			ret = ERROR_NOT_READY; 
			break; 
		}

		get_app_path(db_file_name, ARRAYSIZE( db_file_name), &cc_ret_len); 
		StringCchCat(db_file_name, ARRAYSIZE( db_file_name ), L"\\db" ); 
		DWORD dwAttribute =  GetFileAttributes(db_file_name);
		if(dwAttribute !=INVALID_FILE_ATTRIBUTES && (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)  == FILE_ATTRIBUTE_DIRECTORY )
		{

		}
		else
		{
			CreateDirectory(db_file_name,NULL);
		}

		StringCchCat(db_file_name, ARRAYSIZE( db_file_name ), L"\\" ); 
		StringCchCat(db_file_name, ARRAYSIZE( db_file_name ), db_name); 

		//ret = utf8_to_unicode_ex( db_file_name, _db_file_name); 
		//if( ret != ERROR_SUCCESS )
		//{
		//	break; 
		//}

		_ret = sqlite_db.Open( db_file_name ); 

		if( SQLITE_OK != _ret )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			dbg_print(MSG_FATAL_ERROR, "Error: unable to open database \"%s\": %s\n",
				db_file_name, sqlite_db.GetErrorMessage() );
			break; 
		}

		//sqlite3_fileio_init(db, 0, 0);
		//sqlite3_shathree_init(db, 0, 0);
		//sqlite3_completion_init(db, 0, 0);

		dbg_print( MSG_INFO, "connect to sqlite successfully\n" ); 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI connect_2_sqlite( LPCWSTR db_name, 
							   wstring &error_string )
{
	LRESULT ret; 
	
	do 
	{
		ret = _connect_2_sqlite( db_name ); 
		if( ret != ERROR_SUCCESS )
		{
			string _error_string; 

			_error_string = sqlite_db.GetErrorMessage(); 

			utf8_to_unicode_ex( _error_string.c_str(), error_string ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI construct_sqlite_insert_sql( string &statement, 
										  LPCWSTR table_name, 
										  HTML_ELEMENT_ACTIONS *contents, 
										  vector<string> &blob )
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	//LPSTR ansi_text = NULL; 
	LPWSTR unicode_text = NULL; 
	string field_text;
    wstring output_data; 
	//ULONG string_len; 

	do 
	{
		ASSERT( contents != NULL ); 

//#define COMMON_WEB_PAGE_HTML_TEXT_LEN 65535
#define MAX_SQL_STATEMENT_SEGMENT 512 
		//ansi_text = ( LPSTR )malloc( sizeof( CHAR ) * COMMON_WEB_PAGE_HTML_TEXT_LEN ); 
		//if( NULL == ansi_text )
		//{
		//	ret = ERROR_NOT_ENOUGH_MEMORY; 
		//	break; 
		//}

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

		statement = "INSERT INTO `"; 
		statement += field_text.c_str(); 
		statement += "`( "; 

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

            cat_output_data((*it), output_data);

			ret = unicode_to_utf8_ex(output_data.c_str(), field_text );
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//if( 0 == wcscmp( ( *it )->name.c_str(), L"html" ) )
			{
				statement.append( "?" ); 
				blob.push_back( field_text ); 
			}
			//else
			//{
			//	if( field_text.length() * 2 + 1 > COMMON_WEB_PAGE_HTML_TEXT_LEN )
			//	{
			//		dbg_print( MSG_IMPORTANT, "field text too long %u>%u\n", field_text.length(), COMMON_WEB_PAGE_HTML_TEXT_LEN ); 

			//		free( ansi_text ); 
			//		ansi_text = ( CHAR* )malloc( field_text.length() * 2 + 1 ); 
			//		if( NULL == ansi_text )
			//		{
			//			ret = ERROR_NOT_ENOUGH_MEMORY; 
			//			break; 
			//		}
			//	}

			//	string_len = _mysql_real_escape_string( ansi_text, 
			//		field_text.c_str(), 
			//		field_text.length() ); 
			//	if( string_len < field_text.length() )
			//	{
			//		ASSERT( FALSE ); 
			//	}
			//}

			//statement.append( "'" ); 
			//statement.append( ansi_text ); 
			//statement.append( "'" ); 

			it ++; 
		}

		statement.append( ");" ); 
	} while ( FALSE );

	if( NULL != unicode_text )
	{
		free( unicode_text ); 
	}

	//if( NULL != ansi_text )
	//{
	//	free( ansi_text ); 
	//}

	return ret; 
}

LRESULT WINAPI sqlite_create_table_sql( string &statement_out, 
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

		if(sqlite_db.IsOpen() == false )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		text = ( LPWSTR )malloc( sizeof( WCHAR ) * MAX_SQL_STATEMENT_SEGMENT ); 
		if( NULL == text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_ret = sqlite_db.IsTableExistd(table_name); 
		if( TRUE == _ret )
		{
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

				_ret = sqlite_db.IsTableColumnExistd( table_name, ( *it )->name.c_str() ); 
				if( _ret == FALSE )
				{
					ULONG i; 
#define MAX_DUPLICATE_TABLE_BACKUP 200
					for( i = 0; i < MAX_DUPLICATE_TABLE_BACKUP; i ++ )
					{
						//alter table customers rename custs
						StringCchPrintfW( text, MAX_SQL_STATEMENT_SEGMENT, L"%s%u", table_name, i ); 

						statement = L"ALTER TABLE "; 
						statement += table_name; 
						statement += L" RENAME TO "; 
						statement += text; 
						statement += ';'; 

						ret = unicode_to_utf8_ex( statement.c_str(), _statement ); 
						if( ret != ERROR_SUCCESS )
						{
							_ret = SQLITE_ERROR; 
							break; 
						}

						_ret = sqlite_db.ExecuteSQL( _statement.c_str() );
						if( _ret == SQLITE_OK )
						{
							break; 
						}

						dbg_print( MSG_FATAL_ERROR, "change table name from %s to %s error:%s\n", 
							table_name, 
							text, 
							sqlite_db.GetErrorMessage() ); 
					}

					if( _ret != SQLITE_OK ) // i == MAX_DUPLICATE_TABLE_BACKUP )
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

		statement.append( L"id INTEGER PRIMARY KEY AUTOINCREMENT" ); 

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
			
			//if( 0 == wcscmp( ( *it )->name.c_str(), L"html" ) )
			//{
			//	statement.append( L"` BLOB" ); 
			//}
			//else
			statement.append( L"` TEXT" ); 

			it ++; 
		}

		statement.append( L");" ); // L") DEFAULT CHARSET=utf8;" ); 

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

LRESULT WINAPI sqlite_exec_statement( LPCSTR statement )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		ASSERT( NULL != statement ); 

		if(sqlite_db.IsOpen() == false )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		_ret = sqlite_db.ExecuteSQL(statement ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI sqlite_exec_statement_ex( LPCSTR statement, vector<string> &blobs )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 

	do 
	{
		ASSERT( NULL != statement ); 

		if(sqlite_db.IsOpen() == false )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		_ret = sqlite_db.ExecuteSQLEx(statement, blobs ); 
		if( _ret != SQLITE_OK )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI close_sqlite()
{
	sqlite_db.Close(); 

	return ERROR_SUCCESS; 
}

