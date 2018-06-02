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
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include <strsafe.h>
#import "msxml.tlb"
//#import "mshtml.tlb"
#include "mshtml_addition.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "markup.h"
#include "odbc_io.h"
#include "data_analyze.h"
#include "mysql_conn.h"
#include "sqlite_conn.h"
#include <mysql.h>
#include "data_sender.h"


LRESULT WINAPI format_csv_field_text( wstring &text )
{
	LRESULT ret = ERROR_SUCCESS; 
	size_t pos; 

	pos = 0; 
	while (TRUE)
	{
		pos = text.find(L',', pos);
		if (pos != std::wstring::npos )
		{
			text.replace(pos, 1, L" ", 1);
		}
		else
		{
			break;
		}
	}

	pos = 0; 
	while(TRUE)
	{
		pos = text.find(L'\"', pos);
		if (pos != std::wstring::npos )
		{
			text.replace(pos, 1, L"\"\"", 2);
			pos += 2; 
		}
		else
		{
			break;
		}
	}

	do
	{
		pos = 0; 
		pos = text.find(L'\n', pos);
		if (pos != std::wstring::npos )
		{
			text.insert( 0, 1, L'\"' ); 
			text += '\"'; 
			break; 
		}

		pos = 0; 
		pos = text.find(L'\r', pos);
		if (pos != std::wstring::npos )
		{
			text.insert( 0, 1,  L'\"' ); 
			text += '\"'; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI format_sql_field_text( wstring &text )
{
	LRESULT ret = ERROR_SUCCESS; 

	while (TRUE)
	{
		size_t pos = text.find(L"\r\n");
		if (pos != std::wstring::npos )
		{
			text.replace(pos, 2, L" ", 0);
		}
		else
		{
			break;
		}
	}

	while (TRUE)
	{
		size_t pos = text.find(L",");
		if (pos != std::wstring::npos )
		{
			text.replace(pos, 1, L" ", 0);
		}
		else
		{
			break;
		}
	}

	return ret; 
}

#define SUPPORT_MYSQL_DATA_STORE 1
//LRESULT WINAPI set_previous_script_file_name( DATA_STORE *store, LPCWSTR script_file_name )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HRESULT hr; 
//	MSXML::IXMLDOMDocumentPtr xml_doc; 
//	MSXML::IXMLDOMElementPtr xml_node; 
//
//	_bstr_t temp_text; 
//	//LPCWSTR _temp_text; 
//
//	MSXML::IXMLDOMNodeListPtr node_list;
//	MSXML::IXMLDOMNodePtr node;
//	BOOLEAN node_exist; 
//
//	MSXML::IXMLDOMElementPtr root_element;
//
//	_bstr_t dest_text;
//	LONG node_count;
//
//	//VARIANT_BOOL __ret; 
//
//	do
//	{
//		ASSERT( NULL != script_file_name ); 
//
//		//hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
//		//	NULL, 
//		//	CLSCTX_INPROC_SERVER, 
//		//	__uuidof( MSXML::IXMLDOMDocument ), 
//		//	( void** )&xml_doc ); 
//
//		//if( hr != S_OK)
//		//{
//		//	wsprintfW (Msg, L"构造KEY XML:初始化XML文档对象失败, %08x", hr);
//		//	ret = hr; 
//		//	break; 
//		//}
//
//		//__ret = xml_doc->loadXML( ); 
//
//		//if( __ret != VARIANT_TRUE )
//		//{
//		//	MSXML::IXMLDOMParseErrorPtr spParseError;
//		//	_bstr_t bstrReason;
//
//		//	spParseError = xml_doc->parseError;
//
//		//	bstrReason = spParseError->reason;
//
//		//	_temp_text = bstrReason.operator wchar_t*(); 
//
//		//	if( NULL != _temp_text )
//		//	{
//		//		dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
//		//	}
//
//		//	break; 		
//		//}
//
//		if( store->config_file == NULL )
//		{
//			ret = ERROR_NOT_READY; 
//			break; 
//		}
//
//		xml_doc = ( IXMLDOMDocument* )store->config_file; 
//
//
//		hr = xml_doc->get_documentElement( &root_element ); 
//		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
//		if (hr != S_OK 
//			|| NULL == root_element )
//		{
//			ret = ERROR_INVALID_PARAMETER;
//			break;
//		}
//
//		do
//		{
//			node = NULL; 
//			node_exist = FALSE; 
//			node_list = root_element->selectNodes( PREVIOUS_SCRIPT_FILE_NAME ); 
//			if( NULL == node_list )
//			{
//				ret = ERROR_INVALID_PARAMETER; 
//				break; 
//			}
//
//			hr = node_list->get_length(&node_count);
//			if (S_OK != hr)
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			if( node_count < 0 )
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			if( node_count == 0 )
//			{
//				break; 
//			}
//
//			node_exist = TRUE; 
//
//			hr = node_list->get_item(node_count - 1, &node);
//			if (S_OK != hr)
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED;
//				break;
//			}
//
//			hr = node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), ( PVOID* )&xml_node ); 
//			if( FAILED(hr))
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			temp_text = script_file_name; 
//			hr = xml_node->put_text(temp_text);
//
//		}while( FALSE ); 
//
//		if( node_exist == FALSE )
//		{
//			ASSERT( node == NULL ); 
//
//			temp_text = PREVIOUS_SCRIPT_FILE_NAME; 
//			xml_node = xml_doc->createElement( temp_text ); 
//			if( NULL == xml_node )
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			hr = root_element->appendChild( xml_node ); 
//			if( FAILED( hr ) )
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			temp_text = script_file_name; 
//			hr = xml_node->put_text(temp_text);
//		}
//
//		{
//
//			_variant_t save_file_name; 
//
//			if( store->config_file_name.length() == 0 )
//			{
//				ASSERT( FALSE ); 
//				break; 
//			}
//
//			save_file_name = store->config_file_name.c_str(); 
//
//			hr = xml_doc->save( save_file_name ); 
//
//			if( hr != S_OK )
//			{
//				ret = hr; 
//			}
//		}
//
//	} while (FALSE);
//
//	if( NULL == xml_node )
//	{
//		log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML根节点失败");
//		ret = ERROR_ERRORS_ENCOUNTERED; 
//	} 
//
//	// 释放xml文档
//	//if (xml_doc != NULL)
//	//{
//	//	xml_doc->Release ();
//	//}
//
//	//CoUninitialize ();
//
//	return ret; 
//}

HRESULT WINAPI get_csv_line_text( HTML_ELEMENT_ACTIONS *page_content, 
										 wstring *text )
{
	HRESULT hr = S_OK; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	wstring _text; 

	*text = L""; 

	it = page_content->begin(); 
	for( ; ; )
	{
		if( it == page_content->end() )
		{
			break; 
		}

		ASSERT( ( *it ) != NULL ); 

        cat_output_data((*it), _text);
		format_csv_field_text( _text ); 
		text->append( _text.c_str() ); 

		if( ( it + 1 ) != page_content->begin() )
		{
			text->append( L"," ); 
		}

		it ++; 
	}

	return hr; 
}

HRESULT WINAPI get_web_page_content_sql( HTML_ELEMENT_ACTIONS *page_content, 
										wstring *text )
{
	HRESULT hr = S_OK; 
	HTML_ELEMENT_ACTIONS::iterator it; 
	wstring _text; 

	*text = L""; 

	it = page_content->begin(); 
	for( ; ; )
	{
		if( it == page_content->end() )
		{
			break; 
		}

		ASSERT( ( *it ) != NULL ); 

        cat_output_data((*it), _text);
		format_sql_field_text( _text ); 
		text->append( L"," ); 
		text->append( _text.c_str() ); 

		it ++; 
	}

	return hr; 
}

//CString strUsername,strList,strRemark,strSQL,str_PreName;
//strSQL.Format("update mytable set username=\'%s\',visitelist=\'%s\',                              remark=\'%s\' where username=\'%s\'",strUsername,strList,strRemark,str_PreName);
//if(mysql_real_query(&mysql,(char*)(LPCTSTR)strSQL,(UINT)strSQL.GetLength())!=0)
//{ 
//	AfxMessageBox("修改失败"); 
//}
//
//CString strSQL;
//strSQL.Format("delete from mytable where username=\'%s\'",str_PreName);//必须要有\'\'
//if(mysql_real_query(&mysql,(char*)(LPCTSTR)strSQL,(UINT)strSQL.GetLength())!=0)
//{ 
//	AfxMessageBox("删除失败"); 
//}

//处理列名不同的问题

LRESULT WINAPI format_db_table_name( wstring &text) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		while (TRUE)
		{
			size_t pos = text.find(L".");
			if (pos != std::wstring::npos )
			{
				text.replace(pos, 1, L"_", 1);
				//text.insert(pos, strDest);
			}
			else
			{
				break;
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI create_mysql_table( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content )
{
	LRESULT ret = ERROR_SUCCESS; 
	string statement; 

	do 
	{
		ASSERT( NULL != store ); 
		ASSERT( content ); 

		format_db_table_name(store->table_name); 

		ret = construct_create_table_mysql( statement, store->table_name.c_str(), content); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = mysql_exec_statement( statement.c_str() ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI create_sqlite_table( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content )
{
	LRESULT ret = ERROR_SUCCESS; 
	string statement; 

	do 
	{
		ASSERT( NULL != store ); 
		ASSERT( content ); 

		format_db_table_name(store->table_name); 

		ret = sqlite_create_table_sql( statement, store->table_name.c_str(), content); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		ret = sqlite_exec_statement( statement.c_str() ); 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_csv_file_compatible( LPCWSTR columes, HANDLE file_handle )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != columes ); 
		ASSERT( INVALID_HANDLE_VALUE != file_handle 
			&& NULL != file_handle ); 

		
		
	}while( FALSE );

	return ret; 
}


//HRESULT WINAPI get_json_line_text( HTML_PAGE_CONTENT *page_content, 
//						   wstring *text )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	HTML_ELEMENT_CONTENTS_ITERATOR it; 
//	Json::Value line_text; 
//	Json::Value new_item; 
//	string field_name; 
//	string field_text; 
//
//	*text = L""; 
//
//	it = page_content->element_contents.begin(); 
//	for( ; ; )
//	{
//		if( it == page_content->element_contents.end() )
//		{
//			break; 
//		}
//
//		ASSERT( ( *it ) != NULL ); 
//
//		ret = unicode_to_utf8_ex( ( *it )->text.c_str(), field_text ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		ret = unicode_to_utf8_ex( ( *it )->name.c_str(), field_name ); 
//		if( ret != ERROR_SUCCESS )
//		{
//			break; 
//		}
//
//		new_item[field_name.c_str()] = field_text.c_str();  
//		line_text.append(new_item); 
//		it ++; 
//	}
//
//	std::string out = line_text.toStyledString();  
//	Json::FastWriter writer;  
//	std::string strWrite = writer.write(line_text); 
//
//	ret = utf8_to_unicode_ex( strWrite.c_str(), *text ); 
//	if( ret != ERROR_SUCCESS )
//	{
//
//	}
//
//	return ret; 
//}

LRESULT WINAPI init_csv_file( HTML_ELEMENT_ACTIONS *web_content, DATA_STORE *store )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG data_size; 
	ULONG wrote; 
	LARGE_INTEGER current_pos; 
	LARGE_INTEGER old_pos; 
	wstring text; 
	LPCWSTR data_block; 
	BOOLEAN create_new = FALSE; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 

	do 
	{
		text = L""; 

		if( store->default_data_file == INVALID_HANDLE_VALUE 
			&& store->data_file == INVALID_HANDLE_VALUE )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		for( HTML_ELEMENT_ACTIONS::iterator it = web_content->begin(); 
			it != web_content->end(); 
			it ++ )
		{
			text += ( *it )->name.c_str(); 
			if( ( it + 1 ) != web_content->end())
			{
				text += L","; 
			}
		}

		if( store->data_file == INVALID_HANDLE_VALUE )
		{
			file_handle = store->default_data_file; 
		}
		else
		{
			file_handle = store->data_file; 
		}

		do 
		{
			data_size = ( ULONG )text.length() * sizeof( WCHAR ); 
			data_block = text.c_str(); 
			current_pos.QuadPart = 0; 

			_ret = SetFilePointerEx( file_handle, 
				current_pos, 
				&old_pos, 
				SEEK_END ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			{
				WCHAR *line = NULL;

				do 
				{ 
					ULONG readed; 

#define MAX_LINE_LEN 2048
					line = ( WCHAR* )malloc( sizeof( WCHAR ) * MAX_LINE_LEN ); 
					if( NULL == line )
					{
						break; 
					}

					_ret = ReadFile( file_handle, 
						line, 
						sizeof( WCHAR ) *MAX_LINE_LEN, 
						&readed, 
						NULL ); 

					if( FALSE == _ret )
					{
						break; 
					}

					if( readed == 0 )
					{
						break; 
					}

					_ret = memmem( ( CHAR* )line, readed, ( CHAR* )LINE_TERM, ( LINE_TERM_LEN << 1 ) ); 
					if( _ret < 0 )
					{
						break; 
					}

					ASSERT( ( INT32 )_ret < ( INT32 )readed ); 

					line[ ( _ret >> 1 ) ] = L'\0'; 

					if( _ret != data_size )
					{
						ret = ERROR_CSV_FILE_HEADER_MISMATCH; 
						break; 
					}

					if( 0 != memcmp( data_block, 
						line, 
						_ret ) )
					{
						ret = ERROR_CSV_FILE_HEADER_MISMATCH; 
						break; 
					}
				}while( FALSE );

				if( NULL != line )
				{
					free( line ); 
				}

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}
			}

			_ret = SetFilePointerEx( file_handle, 
				current_pos, 
				&old_pos, 
				SEEK_END ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				data_block, 
				data_size, 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				L"\r\n", 
				sizeof( L"\r\n" ) - sizeof( WCHAR ), 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		} while ( FALSE );
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI construct_mysql_insert_sql( string &statement, 
										  LPCWSTR table_name, 
										  HTML_ELEMENT_ACTIONS *contents); 

LRESULT WINAPI input_links( DATA_STORE *store, 
						   LPCWSTR xpath, 
						   LPCWSTR link_list ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	//HRESULT hr; 
	HANDLE file_handle; 
	INT32 _ret; 

	do 
	{
		ASSERT( NULL != store ); 
		ASSERT( NULL != link_list ); 

		if( INVALID_HANDLE_VALUE == store->link_list_file )
		{
			ret = ERROR_NOT_READY; 
			break; 
		}

		file_handle = store->link_list_file; 

		{
			ULONG data_size; 
			ULONG wrote; 
			LARGE_INTEGER current_pos; 
			LARGE_INTEGER old_pos; 
			ULONG url_len; 
			ULONG xpath_len; 
			//LPCWSTR url; 
			//LPWSTR url_end; 
			LPCWSTR data_block; 

			//for( ; ; )
			//{
			//	url_end = wcschr( url, ',' ); 
			//	if( url_end != NULL )
			//	{
			//		*url_end = L'\0'; 
			//	}

			current_pos.QuadPart = 0; 
			_ret = SetFilePointerEx( file_handle, 
				current_pos, 
				&old_pos, 
				SEEK_END ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			xpath_len = ( ULONG )wcslen( xpath ); 
			data_size = xpath_len * sizeof( WCHAR ); 
			data_block = xpath; 

			_ret = WriteFile( file_handle, 
				data_block, 
				data_size, 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				OUTPUT_URLS_DELIM, 
				sizeof( OUTPUT_URLS_DELIM ) - sizeof( WCHAR ), 
				&wrote, 
				NULL ); 

			url_len = ( ULONG )wcslen( link_list );  
			data_size = url_len* sizeof( WCHAR ); 
			data_block = link_list; 

			_ret = WriteFile( file_handle, 
				data_block, 
				data_size, 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				L"\r\n", 
				sizeof( L"\r\n" ) - sizeof( WCHAR ), 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		}

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI output_data( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	HANDLE file_handle; 
	INT32 _ret; 

	do 
	{
		ASSERT( NULL != store ); 

#ifdef SUPPORT_MYSQL_DATA_STORE
		do 
		{	
			if( store->have_db_conn != TRUE )
			{
				break; 
			}

			string mysql_sql; 

			if( store->table_name.length() == 0 )
			{
				ASSERT( FALSE ); 
				dbg_print(MSG_FATAL_ERROR, "table name is null\n" ); 
				break; 
			}

			ret = construct_mysql_insert_sql( mysql_sql, 
				store->table_name.c_str(), 
				content ); 
		
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "construct the insert sql statement error\n" ); 
				break; 
			}

			ret = mysql_exec_statement(mysql_sql.c_str()); 
			if( ERROR_SUCCESS != ret )
			{
				ASSERT( FALSE ); 
				dbg_print( MSG_FATAL_ERROR, "construct the insert sql statement error\n" ); 
				break; 
			}
		}while( FALSE ); 
#endif //SUPPORT_MYSQL_DATA_STORE

		do 
		{
			vector<string> blobs; 
			string sqlite_sql; 

			if( store->table_name.length() == 0 )
			{
				ASSERT( FALSE ); 
				dbg_print(MSG_FATAL_ERROR, "table name is null\n" ); 
				break; 
			}

			ret = construct_sqlite_insert_sql( sqlite_sql, 
				store->table_name.c_str(), 
				content, 
				blobs ); 

			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "construct the insert sql statement error\n" ); 
				break; 
			}
			ret = sqlite_exec_statement_ex(sqlite_sql.c_str(), blobs); 
			if( ERROR_SUCCESS != ret )
			{
				ASSERT( FALSE ); 
				dbg_print( MSG_FATAL_ERROR, "construct the insert sql statement error\n" ); 
				break; 
			}
		} while ( FALSE ); 

		if( INVALID_HANDLE_VALUE == store->data_file )
		{
			file_handle = store->default_data_file; 
		}
		else
		{
			file_handle = store->data_file; 
		}

		if( INVALID_HANDLE_VALUE == file_handle )
		{
			break; 
		}

		{
			ULONG data_size; 
			ULONG wrote; 
			LARGE_INTEGER current_pos; 
			LARGE_INTEGER old_pos; 
			wstring text; 
			LPCWSTR data_block; 

			hr = get_csv_line_text( content, &text ); 
			if( FAILED(hr))
			{
				break; 
			}

			data_size = ( ULONG )text.length() * sizeof( WCHAR ); 
			data_block = text.c_str(); 
			current_pos.QuadPart = 0; 

			_ret = SetFilePointerEx( file_handle, 
				current_pos, 
				&old_pos, 
				SEEK_END ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				data_block, 
				data_size, 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			_ret = WriteFile( file_handle, 
				L"\r\n", 
				sizeof( L"\r\n" ) - sizeof( WCHAR ), 
				&wrote, 
				NULL ); 

			if( FALSE == _ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}
		}

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI get_data_center_file_name( LPWSTR file_name, ULONG cc_buf_len, ULONG *cc_ret_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 

	do 
	{
		ret = get_app_path( file_name, cc_buf_len, cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( file_name, cc_buf_len, TO_DATA_CENTER_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI create_data_store( DATA_STORE_PARAM *param, DATA_STORE *store, wstring &error_string )
{
	HRESULT hr; 
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	BOOLEAN create_new = FALSE; 
	BYTE unicode_fmt_sign[] = { 0xff, 0xfe }; 
	wstring _error_string; 

	do 
	{
		do 
		{
			ASSERT( NULL != param ); 
			ASSERT( NULL != store ); 

			error_string = L""; 
			//store->table_name = param->sql_table_name; 

#ifdef SUPPORT_MYSQL_DATA_STORE
			do 
			{
				wstring statement; 

				store->have_db_conn = FALSE; 
				store->data_file_name = param->output_path; 

				store->table_name = param->sql_table_name; 
				if( store->table_name.length() == 0 )
				{
					error_string += L"没有数据库表名"; 
					error_string += L";"; 
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				//"DSN=data-source-name[;SERVER=value] [;PWD=value] [;UID=value] [;<Attribute>=<value>]"

				ret = connect_2_mysql( param->sql_connection.ip.c_str(), 
					param->sql_connection.port, 
					param->sql_connection.user_name.c_str(), 
					param->sql_connection.password.c_str(), 
					param->sql_connection.data_base_name.c_str(), 
					_error_string ); 

				if( ret != ERROR_SUCCESS )
				{
					error_string += _error_string.c_str(); 
					//dbg_print
					break; 
				} 

				store->have_db_conn = TRUE; 
			} while ( FALSE ); 

#endif //SUPPORT_MYSQL_DATA_STORE

			do 
			{
				wstring statement; 

				store->have_db_conn = FALSE; 
				store->data_file_name = param->output_path; 

				store->table_name = param->sql_table_name; 
				if( store->table_name.length() == 0 )
				{
					error_string += L"没有数据库表名"; 
					error_string += L";"; 
					ret = ERROR_INVALID_PARAMETER; 
					break; 
				}

				//"DSN=data-source-name[;SERVER=value] [;PWD=value] [;UID=value] [;<Attribute>=<value>]"

				ret = connect_2_sqlite( param->sql_table_name.c_str(), 
					_error_string ); 

				if( ret != ERROR_SUCCESS )
				{
					error_string += _error_string.c_str(); 
					//dbg_print
					break; 
				} 
			} while (FALSE ); ;
			//if( param->type == DATA_OUTPUT_ADO )
			//{
			//	store->have_db_conn = FALSE; 
			//	if( *param->output_path == L'\0' )
			//	{
			//		break; 
			//	}

			//	hr = ado_connect( param->output_path ); 
			//	if( FAILED( hr ) )
			//	{
			//		break; 
			//	}

			//	store->have_db_conn = TRUE; 
			//	break; 
			//}

			_ret = check_file_exist( param->output_path.c_str()); 

			if( _ret != ERROR_SUCCESS )
			{
				create_new = TRUE; 
			}

			store->data_file = CreateFileW( param->output_path.c_str(), 
				FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_ALWAYS, 
				0, 
				NULL ); 

			if( INVALID_HANDLE_VALUE == store->data_file ) 
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			if( create_new == TRUE )
			{
				ULONG data_size; 
				ULONG wrote; 
				LARGE_INTEGER current_pos; 
				LARGE_INTEGER old_pos; 

				data_size = sizeof( unicode_fmt_sign ); 

				current_pos.QuadPart = 0; 

				_ret = SetFilePointerEx( store->data_file, 
					current_pos, 
					&old_pos, 
					SEEK_SET); 

				if( FALSE == _ret )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				_ret = WriteFile( store->data_file, 
					unicode_fmt_sign, 
					data_size, 
					&wrote, 
					NULL ); 

				if( FALSE == _ret )
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}
			}
		}while( FALSE ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( param->output_link_list == TRUE )
		{
			WCHAR file_name[ MAX_PATH ]; 
			//ULONG cc_ret_len; 

			do 
			{
				hr = StringCchCopyW( file_name, ARRAYSIZE( file_name ), param->output_path.c_str() ); 
				if( FAILED(hr))
				{
					break; 
				}

				hr = StringCchCatW( file_name, ARRAYSIZE( file_name ), L".links" ); 
				if( FAILED(hr))
				{
					break; 
				}

				_ret = check_file_exist( file_name ); 

				if( _ret != ERROR_SUCCESS )
				{
					create_new = TRUE; 
				}

				store->link_list_file = CreateFileW( file_name, 
					FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
					FILE_SHARE_READ | FILE_SHARE_WRITE, 
					NULL, 
					OPEN_ALWAYS, 
					0, 
					NULL ); 

				if( INVALID_HANDLE_VALUE == store->link_list_file ) 
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				if( create_new == TRUE )
				{
					ULONG data_size; 
					ULONG wrote; 
					LARGE_INTEGER current_pos; 
					LARGE_INTEGER old_pos; 

					data_size = sizeof( unicode_fmt_sign ); 

					current_pos.QuadPart = 0; 

					_ret = SetFilePointerEx( store->link_list_file, 
						current_pos, 
						&old_pos, 
						SEEK_SET); 

					if( FALSE == _ret )
					{
						SAFE_SET_ERROR_CODE( ret ); 
						break; 
					}

					_ret = WriteFile( store->link_list_file, 
						unicode_fmt_sign, 
						data_size, 
						&wrote, 
						NULL ); 

					if( FALSE == _ret )
					{
						SAFE_SET_ERROR_CODE( ret ); 
						break; 
					}
				}
			} while ( FALSE );
		}

		if( store->data_file == INVALID_HANDLE_VALUE
			&& param->to_data_center == TRUE )
		{
			WCHAR file_name[ MAX_PATH ]; 
			ULONG cc_ret_len; 

			do 
			{
				ret = get_data_center_file_name( file_name, ARRAYSIZE( file_name ), &cc_ret_len ); 
				if( ret != ERROR_SUCCESS )
				{
					error_string += L"程序所在路径>260,不支持"; 
					error_string += L";"; 
					break; 
				}

				_ret = check_file_exist( file_name ); 

				if( _ret != ERROR_SUCCESS )
				{
					create_new = TRUE; 
				}

				store->default_data_file = CreateFileW( file_name, 
					FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
					FILE_SHARE_READ | FILE_SHARE_WRITE, 
					NULL, 
					OPEN_ALWAYS, 
					0, 
					NULL ); 

				if( INVALID_HANDLE_VALUE == store->default_data_file ) 
				{
					SAFE_SET_ERROR_CODE( ret ); 
					break; 
				}

				if( create_new == TRUE )
				{
					ULONG data_size; 
					ULONG wrote; 
					LARGE_INTEGER current_pos; 
					LARGE_INTEGER old_pos; 

					data_size = sizeof( unicode_fmt_sign ); 

					current_pos.QuadPart = 0; 

					_ret = SetFilePointerEx( store->default_data_file, 
						current_pos, 
						&old_pos, 
						SEEK_SET); 

					if( FALSE == _ret )
					{
						SAFE_SET_ERROR_CODE( ret ); 
						break; 
					}

					_ret = WriteFile( store->default_data_file, 
						unicode_fmt_sign, 
						data_size, 
						&wrote, 
						NULL ); 

					if( FALSE == _ret )
					{
						SAFE_SET_ERROR_CODE( ret ); 
						break; 
					}
				}
			} while ( FALSE );
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI config_data_format( HTML_ELEMENT_ACTIONS *content, 
						   DATA_STORE *store )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		do 
		{
			if( store->table_name.length() == 0 )
			{
				break; 
			}

			store->table_name = store->table_name; 
			ret = create_mysql_table( store, content ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE );

		do 
		{
			if( store->table_name.length() == 0 )
			{
				break; 
			}

			store->table_name = store->table_name; 
			ret = create_sqlite_table( store, content ); 

			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE );

		do 
		{
			if( INVALID_HANDLE_VALUE == store->data_file 
				&& INVALID_HANDLE_VALUE == store->default_data_file )
			{
				break; 
			}

			ret = init_csv_file( content, store ); 
			if( ret != ERROR_SUCCESS )
			{
				if( ERROR_CSV_FILE_HEADER_MISMATCH == ret )
				{
					MessageBox( NULL, L"CSV已经保存内容的字段定义与配置的字段定义不同", NULL, 0 ); 
				}
				break; 
			}
		}while( FALSE );
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI init_data_store( DATA_STORE_PARAM *param, 
						HTML_ELEMENT_ACTIONS *content, 
						DATA_STORE *store )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		wstring message; 
		wstring error_string; 

		_ret = create_data_store( param, store, error_string ); 
		if( _ret != ERROR_SUCCESS )
		{
			message = L"创建输出对象失败:"; 
			message += error_string.c_str(); 

			dbg_print_w( MSG_ERROR, L"%s", message.c_str() );  
		}

		if( ( FALSE == store->have_db_conn 
			|| store->table_name.empty() == 0 )
			&& store->data_file == NULL )
		{
			MessageBox( NULL, message.c_str(), NULL, 0 ); 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}

#if 0
{
	_ret = __make_config_file_exist( param->conf_file_name, wcslen( param->conf_file_name ) ); 
	if( ERROR_SUCCESS != _ret )
	{
		error_string += L"生成配置文件失败"; 
		error_string += L";"; 
		break; 
	}

	{
		VARIANT_BOOL __ret; 
		HRESULT hr; 
		MSXML::IXMLDOMDocumentPtr xml_doc; 
		MSXML::IXMLDOMElementPtr xml_node = NULL; 

		_bstr_t temp_text; 
		LPCWSTR _temp_text; 
		_variant_t file_name; 

		do
		{

			hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
				NULL, 
				CLSCTX_INPROC_SERVER, 
				__uuidof( MSXML::IXMLDOMDocument ), 
				( void** )&xml_doc ); 

			if( hr != S_OK)
			{
				dbg_print( MSG_FATAL_ERROR, "create xml document error %08x\n", hr);
				ret = hr; 
				break; 
			}

			temp_text = param->conf_file_name; 
			file_name = temp_text.GetBSTR(); 

			__ret = xml_doc->load( file_name ); 

			if( __ret != VARIANT_TRUE )
			{
				MSXML::IXMLDOMParseErrorPtr spParseError;
				_bstr_t bstrReason;

				spParseError = xml_doc->parseError;

				bstrReason = spParseError->reason;

				_temp_text = bstrReason.operator wchar_t*(); 

				if( NULL != _temp_text )
				{
					dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
				}

				break; 
			}

			store->config_file = ( PVOID )xml_doc.Detach(); 
			store->config_file_name = param->conf_file_name; 
		}while( FALSE ); 
	}
}while( FALSE ); 

LRESULT WINAPI get_previous_script_file_name( LPCWSTR config_file_name, 
										   wstring &script_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMElementPtr xml_node; 

	_bstr_t temp_text; 
	LPCWSTR _temp_text; 

	MSXML::IXMLDOMNodeListPtr node_list;
	MSXML::IXMLDOMNodePtr node;
	//BOOLEAN node_exist; 

	MSXML::IXMLDOMElementPtr root_element;

	_bstr_t dest_text; 
	_variant_t file_name; 
	LONG node_count;

	VARIANT_BOOL __ret; 

	do
	{
		ASSERT( NULL != config_file_name ); 

		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 

		if( hr != S_OK)
		{
			ret = hr; 
			break; 
		}

		temp_text = config_file_name; 
		file_name = temp_text.GetBSTR(); 

		__ret = xml_doc->load( file_name ); 

		if( __ret != VARIANT_TRUE )
		{
			MSXML::IXMLDOMParseErrorPtr spParseError;
			_bstr_t bstrReason;

			spParseError = xml_doc->parseError;

			bstrReason = spParseError->reason;

			_temp_text = bstrReason.operator wchar_t*(); 

			if( NULL != _temp_text )
			{
				dbg_print( DBG_MSG_AND_ERROR_OUT, "load xml error %ws\n", _temp_text ); 
			}

			break; 		
		}

		hr = xml_doc->get_documentElement( &root_element ); 
		//hr = xml_dest_doc->get_xml( dest_text.GetAddress());
		if (hr != S_OK 
			|| NULL == root_element )
		{
			ret = ERROR_INVALID_PARAMETER;
			break;
		}

		do
		{
			node = NULL; 
			node_list = root_element->selectNodes( PREVIOUS_SCRIPT_FILE_NAME ); 
			if( NULL == node_list )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}

			hr = node_list->get_length(&node_count);
			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( node_count < 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			if( node_count == 0 )
			{
				break; 
			}

			hr = node_list->get_item(node_count - 1, &node);
			if (S_OK != hr)
			{
				ret = ERROR_ERRORS_ENCOUNTERED;
				break;
			}

			hr = node->QueryInterface( __uuidof( MSXML::IXMLDOMElement ), ( PVOID* )&xml_node ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = xml_node->get_text(temp_text.GetAddress()); 
			if( FAILED( hr))
			{
				break; 
			}

			_temp_text = temp_text.operator wchar_t*(); 
			if( NULL == _temp_text )
			{
				hr = E_FAIL; 
				break; 
			}
			
			script_file_name = _temp_text; 
		}while( FALSE ); 

	} while (FALSE);

	if( NULL == xml_node )
	{
		log_trace_ex( MSG_IMPORTANT, "get next link node error\n");
		ret = ERROR_ERRORS_ENCOUNTERED; 
	} 

	//if (xml_doc != NULL)
	//{
	//	xml_doc->Release();
	//}

	//CoUninitialize();

	return ret; 
}
#endif //0

LRESULT WINAPI uninit_data_store( DATA_STORE *store )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != store ); 

		if( INVALID_HANDLE_VALUE != store->data_file )
		{
			CloseHandle( store->data_file ); 
			store->data_file  = INVALID_HANDLE_VALUE; 
		}

		if( INVALID_HANDLE_VALUE != store->default_data_file )
		{
			CloseHandle( store->default_data_file ); 
			store->default_data_file = INVALID_HANDLE_VALUE; 
		}

		if( INVALID_HANDLE_VALUE != store->link_list_file )
		{
			CloseHandle( store->link_list_file ); 
			store->link_list_file = INVALID_HANDLE_VALUE; 
		}

		//if( NULL != store->config_file )
		//{
		//	IXMLDOMDocument *doc; 

		//	doc = ( IXMLDOMDocument* )store->config_file; 
		//	doc->Release(); 

		//	store->config_file = NULL; 
		//}

		if( TRUE == store->have_db_conn )
		{
			//ado_disconnect(); 
#ifdef SUPPORT_MYSQL_DATA_STORE
			close_mysql(); 
#endif //SUPPORT_MYSQL_DATA_STORE
			close_sqlite(); 
			store->have_db_conn = FALSE; 
		}
	} while ( FALSE ); 

	return ret; 
}

LRESULT WINAPI _create_config_file( LPCWSTR config_file_name, LPCWSTR root_element_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	MSXML::IXMLDOMDocumentPtr xml_doc; 
	MSXML::IXMLDOMProcessingInstructionPtr xml_instruction; 
	MSXML::IXMLDOMElementPtr xml_element; 
	_bstr_t text; 

	do
	{
		hr = CoCreateInstance(__uuidof( MSXML::DOMDocument ), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof( MSXML::IXMLDOMDocument ), 
			( void** )&xml_doc ); 
		if( hr != S_OK)
		{
			log_trace_ex( MSG_IMPORTANT, "create xml document error %08x", hr);
			ret = hr; 
			break; 
		}

		// 创建Xml声明
		hr = xml_doc->raw_createProcessingInstruction( L"xml", L"version=\"1.0\" encoding=\"utf-8\"", &xml_instruction );

		if( S_OK != hr )
		{
			log_trace_ex( MSG_IMPORTANT, "构造KEY XML:创建XML声明失败" );
			ret = hr; 
			break; 
		}

		hr = xml_doc->raw_appendChild( xml_instruction, NULL );
		if( S_OK != hr )
		{
			ret = hr; 
			break; 
		}

		text = root_element_name; 
		xml_element = xml_doc->createElement( text ); 

		if( NULL == xml_element )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = xml_doc->appendChild( xml_element ); 
		if( FAILED( hr ) )
		{
			ret = hr; 
			//break; 
		}

		{

			_variant_t save_file_name; 

			text = config_file_name; 
			save_file_name = text; 			
			hr = xml_doc->save( save_file_name ); 

			if( hr != S_OK )
			{
				ret = hr; 
			}
		}

	}while( FALSE ); 

	//// 释放xml声明
	//if (xml_instruction != NULL)
	//{
	//	xml_instruction.Release();
	//}

	//// 释放xml文档
	//if (xml_doc != NULL)
	//{
	//	xml_doc.Release ();
	//}

	//CoUninitialize ();

	return ret; 
}

LRESULT WINAPI __create_config_file( LPCWSTR config_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	ret = _create_config_file( config_file_name, HTML_SCRIPT_ROOT_ELEMENT_NAME ); 
	return ret; 
}

LRESULT WINAPI __make_config_file_exist( LPCWSTR config_file_path, ULONG cc_path_len )
{
	LRESULT ret = ERROR_SUCCESS; 
	//BOOL _ret; 
	//HANDLE file_handle = INVALID_HANDLE_VALUE; 
	//WCHAR xml_template[] = L"<html_sites></html_sites>"; 
	//#define MAX_TIME_TEXT 128

	//L"<?xml version=\"1.0\"?>"; 
	//L"<site url=\"%s\" >" \
	//L"<page url=\"%s\">" \
	//L"</page>" \
	//L"</site>";

	do 
	{
		ASSERT( config_file_path != NULL ); 

		ret = check_file_exist( config_file_path ); 
		if( ret != ERROR_SUCCESS )
		{
			//ret = ERROR_SUCCESS; 


			//file_handle = CreateFileW( config_file_path, 
			//	GENERIC_READ | GENERIC_WRITE, 
			//	FILE_SHARE_READ, 
			//	NULL, 
			//	OPEN_ALWAYS, 
			//	0, 
			//	NULL ); 

			//if( file_handle == INVALID_HANDLE_VALUE )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			//_ret = WriteFile( file_handle, 
			//	xml_template, 
			//	sizeof( xml_template ) - sizeof( WCHAR ), 
			//	&wrote, 
			//	NULL ); 

			//if( FALSE == _ret )
			//{
			//	ret = GetLastError(); 
			//	break; 
			//}

			ret = __create_config_file( config_file_path ); 

		}
		else
		{
#ifdef _DEBUG
			//ret = create_config_file( config_file_path ); 
#else	
			ASSERT( FALSE ); 
#endif //_DEBUG

		}

	}while( FALSE ); 

	//if( INVALID_HANDLE_VALUE != file_handle )
	//{
	//	CloseHandle( file_handle ); 
	//}

	return ret; 
}

/****************************************************************************
		//_ret = GetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, config_file_path ); 
		//if( _ret == 0 )
		{
			WCHAR file_name[ MAX_PATH ]; 
			WCHAR app_path[ MAX_PATH ]; 
			ULONG cc_ret_len; 
			SYSTEMTIME _time; 

#define CONFIG_FILE_DIRECTORY L"html_page_config\\"

			ret = get_app_path( app_path, ARRAYSIZE( app_path ), &cc_ret_len ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			//hr = html_doc->get_URL( text.GetAddress() ); 
			//if( S_OK != hr )
			//{
			//	break; 
			//}

			//_text = text.operator const wchar_t*(); 
			//if( _text == NULL )
			//{
			//	break; 
			//}

			hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), CONFIG_FILE_DIRECTORY ); 
			if( S_OK != hr )
			{
				break; 
			}

			//if( url.GetLength() == 0 )
			//{
			//	app_path[ cc_ret_len - 1 ] = L'\0'; 
			//}
			//else
			//{
			//	if( domain_name.length() == 0 )
			//	{
			//		app_path[ cc_ret_len - 1 ] = L'\0'; 
			//	}

			//	//hr = StringCchCatW( app_path, ARRAYSIZE( app_path ), domain_name.c_str() ); 
			//	//if( hr != S_OK )
			//	//{
			//	//	break; 
			//	//}
			//}

			//*file_name = _T( '\0' ); 

			//_ret = SetCurrentDirectoryW( app_path ); 
			//if( FALSE == _ret )
			//{

			//}

			//ret = open_file_dlg( FALSE, L"xml", file_name, NULL, app_path, L"*.xml\0*.xml\0\0", NULL ); 
			//if( ret < 0 )
			//{
			//	break; 
			//}

			//if( *file_name != L'\0' )
			//{
			//	SetDlgItemTextW( IDC_EDIT_CONFIG_FILE_PATH, file_name ); 
			//}

			ret = create_directory_ex( app_path, wcslen( app_path ), 2 ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			GetLocalTime( &_time ); 

#ifdef _FILE_NAME_BY_TIME
			hr = StringCchPrintfW( file_name, 
				ARRAYSIZE( file_name ), 
				L"%s%s_%04u%02u%02u%02u%02u%02u", 
				app_path, 
				domain_name.c_str(), 
				_time.wYear, 
				_time.wMonth, 
				_time.wDay, 
				_time.wHour, 
				_time.wMinute, 
				_time.wSecond ); 
#else
			hr = StringCchPrintfW( file_name, 
				ARRAYSIZE( file_name ), 
				L"%s%s", 
				app_path, 
				config_file_name.GetBuffer() ); 
#endif //_FILE_NAME_BY_TIME

			if( hr != S_OK )
			{

			}

			config_file_name = file_name; 
		}

		ret = make_config_file_exist( config_file_name.GetBuffer(), 
			config_file_name.GetLength() ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
****************************************************************************/

LRESULT WINAPI _send_data_file( DATA_STORE_PARAM *param )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != param ); 

		if( TRUE == param->to_data_center)
		{
			param->to_data_center = FALSE; 

			if( DATA_OUTPUT_CSV_FILE == param->type )
			{
				if( param->output_path.length() == 0 )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				check_and_send_data( param->output_path.c_str() ); 
			}
			else
			{
				WCHAR file_name[MAX_PATH ]; 
				ULONG cc_ret_len; 

				do 
				{
					ret = get_data_center_file_name( file_name, ARRAYSIZE(file_name), &cc_ret_len ); 
					if( ERROR_SUCCESS != ret )
					{
						break; 
					}

					ret = check_and_send_data( file_name ); 
				} while (FALSE );
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI post_data_file( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
#define CMD_LINE_LEN 320
#define _TMP_FILE_LEN 64

	HRESULT hr; 
	TCHAR mod_path[ MAX_PATH ]; 
	TCHAR cmd_line[ CMD_LINE_LEN ]; 
	DWORD _ret; 
	ULONG cc_ret_len; 
	STARTUPINFO clone_start_info; 
	PROCESS_INFORMATION clone_process_info = { 0 };
	//DWORD wait_ret; 
	static BOOLEAN file_posted  = FALSE; 
	do 
	{
		if( file_posted == TRUE )
		{
			break; 
		}

		file_posted = TRUE; 

		ASSERT( NULL != file_name ); 
		log_trace( ( MSG_INFO, "enter %s \n", __FUNCTION__ ) ); 

		ret = check_file_exist( file_name ); 

		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "the file to post is not exist %ws \n", file_name ); 
			break; 
		}

		_ret = get_app_path( mod_path, ARRAYSIZE( mod_path ), &cc_ret_len ); 
		if( _ret != ERROR_SUCCESS )
		{
			ret = GetLastError(); 
			goto _return; 
		}

		hr = StringCchCatW( mod_path, ARRAYSIZE( mod_path ), L"data_center.exe" ); 

		_ret = check_file_exist( mod_path ); 
		if( _ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "data post executable file(%ws) is not exist\n", mod_path ); 
			break; 
		}

		ZeroMemory( &clone_start_info, sizeof( clone_start_info ));
		clone_start_info.cb = sizeof( clone_start_info ); 
		_ret = StringCchPrintfW( cmd_line, CMD_LINE_LEN, L"%s %s", mod_path, file_name ); 

		if( ( INT32 )_ret < 0 )
		{
			ret = ERROR_BUFFER_OVERFLOW; 
			goto _return; 
		}

		_ret = ( BOOL )CreateProcess( NULL, 
			cmd_line, 
			NULL, 
			NULL, 
			TRUE, 
			0, 
			NULL, 
			NULL, 
			&clone_start_info, 
			&clone_process_info ); 

		if( _ret == FALSE )
		{
			ret = GetLastError(); 
			log_trace( ( DBG_MSG_AND_ERROR_OUT, "start the process from the clone file failed \n" ) ); 
			goto _return; 
		}

		//wait_ret = WaitForSingleObject( clone_process_info.hProcess, INFINITE ); 

		//if( wait_ret != WAIT_TIMEOUT 
		//	&& wait_ret != WAIT_OBJECT_0 
		//	&& wait_ret != WAIT_ABANDONED )
		//{
		//	log_trace( ( DBG_MSG_AND_ERROR_OUT, "!!!wait clone process failed\n" ) ); 
		//	ASSERT( FALSE ); 
		//}

		log_trace( ( MSG_INFO, "leave %s 0x%0.8x\n", __FUNCTION__, ret ) ); 
	} while ( FALSE );
	//if( process != NULL )
	//{
	//	CloseHandle( process );
	//}
_return:

	if( clone_process_info.hProcess != NULL )
	{
		CloseHandle( clone_process_info.hProcess );
		ASSERT( clone_process_info.hThread != NULL ); 
		CloseHandle( clone_process_info.hThread );
	}

	return ret; 
}

LRESULT WINAPI check_file_data_quantity( LPCWSTR file_name, ULONG *data_count)
{
	LRESULT ret = ERROR_SUCCESS;

	do 
	{
		ASSERT( NULL != file_name ); 
		ASSERT( NULL != data_count ); 

	} while( FALSE );

	return ret; 
}

LRESULT WINAPI check_and_send_data( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != file_name ); 

		ret = check_file_exist( file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "the data file %ws is not exist\n", file_name ); 
		}

		ULONG data_count; 
		ret= check_file_data_quantity( file_name, &data_count); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

#define CODEREBA_WEBKITS_ULOAD_DATA_URL L"http://www.simplestai.com/webkits/data_upload.php" 

		//fix runtime close
		//ret = send_data_file( CODEREBA_WEBKITS_ULOAD_DATA_URL, ( LPWSTR )file_name ); 
		if( ret != ERROR_SUCCESS )
		{
			dbg_print( MSG_FATAL_ERROR, "send the data file error 0x%0.8x\n", ret ); 
			break; 
		}
	} while (FALSE ); 

	return ret; 
}

/***********************************************************
将网页按其结构进行分类，不同的结构，内容需要使用不同的记录
方式
***********************************************************/

CONTAINER_INFOS html_content; 
CONTAINER_INFO_MAP html_content_infos; 

LRESULT WINAPI post_scrambled_data()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		for( CONTAINER_INFOS::iterator it = html_content.begin(); 
			it != html_content.end(); 
			it ++ )
		{
			if( ( *it )->store_param.to_data_center != TRUE )
			{
				break; 
			}

			( *it )->store_param.to_data_center = FALSE; 

			if( ( *it )->store_param.type == DATA_OUTPUT_CSV_FILE )
			{
				LRESULT ret; 
				ret = post_data_file( ( *it )->store_param.output_path.c_str() ); 
			}
			else
			{
				LRESULT ret; 
				WCHAR file_name[ MAX_PATH ]; 
				ULONG cc_ret_len; 
				HRESULT hr; 

				do 
				{
					ret = get_app_path( file_name, ARRAYSIZE( file_name ), &cc_ret_len ); 
					if( ret != ERROR_SUCCESS )
					{
						break; 
					}

					hr = StringCchCatW( file_name, ARRAYSIZE( file_name ), TO_DATA_CENTER_FILE_NAME ); 
					if( FAILED(hr))
					{
						break; 
					}

					//ret = check_file_exist( file_name ); 

					//if( ret != ERROR_SUCCESS )
					//{
					//	break; 
					//}

					ret = post_data_file( file_name ); 
				} while ( FALSE );
			}
		}
	} while ( FALSE ); 
	return ret; 
}

LRESULT WINAPI get_html_content_info( HTML_ELEMENT_ACTION *container, 
										CONTAINER_INFO **content )
{
	LRESULT ret = ERROR_SUCCESS; 
	CONTAINER_INFO_MAP_ITERATOR it; 

	do 
	{
		ASSERT( NULL != container ); 
		ASSERT( NULL != content ); 

        *content = NULL; 
		it = html_content_infos.find( container->id ); 
		if( it == html_content_infos.end() )
		{
			ret = ERROR_NOT_FOUND; 
			break; 
		}

		ASSERT( NULL != ( *it ).second ); 
		*content  = ( *it ).second; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI check_data_store( DATA_STORE *store ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != store ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI init_default_data_store_settings( DATA_STORE_PARAM *param, 
										 LPCWSTR data_file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG cc_ret_len; 
	WCHAR file_name[ MAX_PATH ]; 

	do 
	{
		ASSERT( NULL != data_file_name ); 

		param->to_data_center = FALSE; 
		param->type = DATA_OUTPUT_CSV_FILE; 

		ret = get_app_path( file_name, ARRAYSIZE( file_name ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( file_name, ARRAYSIZE( file_name ), L"\\csv\\" ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = create_directory_ex( file_name, wcslen( file_name ), 2 ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( file_name, ARRAYSIZE( file_name ), data_file_name ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		param->output_path = file_name; 

		param->sql_connection.ip = ""; 
		param->sql_connection.port = 0; 
		param->sql_connection.user_name = ""; 
		param->sql_connection.password = ""; 
		param->sql_connection.data_base_name = ""; 

	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI release_container_info( CONTAINER_INFO *container_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	CONTAINER_INFO_MAP::iterator it; 

	do 
	{
		ASSERT( NULL != container_info ); 
		
		release_container_content_info( &container_info->content ); 
		
		for( it = html_content_infos.begin(); 
			it != html_content_infos.end(); 
			it ++ )
		{
			if( ( *it ).second == container_info )
			{
				html_content_infos.erase( it ); 
				break; 
			}
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_html_content_info_ex( HTML_ELEMENT_ACTION *container, 
									 CONTAINER_INFO **conent_info,  
									 ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *action; 

	do 
	{
		ASSERT( NULL != container ); 
		ASSERT( NULL != conent_info ); 

		*conent_info = NULL; 

		//if( GET_ROOT_ELEMENT_INFO == flags )
		//{
		//	action = container; 
		//}
		//else if( ERROR_SUCCESS == check_have_sub_page( container ) ) 
		//{
		//	ret = locate_to_sub_page_instruction( container, &action ); 
		//	if( ERROR_SUCCESS != ret )
		//	{
		//		break; 
		//	}
		//}
		//else
		{
			action = container; 
		}

		ret = get_html_content_info( action, conent_info ); 
		if( ret == ERROR_NOT_FOUND )
		{
			ret = allocate_html_content_info( action, conent_info ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "create the web browser process error %x\n", ret ); 
			}
		}

	}while( FALSE ); 

	return ret; 
}

VOID data_store_construct( DATA_STORE *store )
{
	store->have_db_conn = FALSE; 
	store->data_file = INVALID_HANDLE_VALUE; 
	store->default_data_file = INVALID_HANDLE_VALUE; 
	store->link_list_file = INVALID_HANDLE_VALUE; 
	store->data_file_name = L""; 
	store->table_name = L""; 
}

LRESULT WINAPI allocate_html_content_info( HTML_ELEMENT_ACTION *container, 
											CONTAINER_INFO **container_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	CONTAINER_INFO *_container_info; 
	do 
	{
		ASSERT( NULL != container ); 

		_container_info = NULL; 
		for( CONTAINER_INFOS::iterator it = html_content.begin(); 
			it != html_content.end(); 
			it ++ )
		{
			if( ( *it )->_using == FALSE )
			{
				_container_info = ( *it ); 
			}
		}

		if( NULL == _container_info )
		{
			_container_info = new CONTAINER_INFO(); 
			if( NULL == _container_info )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

            {
                _container_info->name = container->name;
            }

			html_content.push_back( _container_info ); 
		}

		_container_info->have_ui = FALSE; 

		data_store_construct( &_container_info->store ); 

		html_content_infos.insert( CONTAINER_INFO_MAP_PAIR( container->id, _container_info ) ); 

		*container_info = _container_info; 
	}while( FALSE ); 
	return ret; 
}

LRESULT WINAPI release_all_html_conainer_infos()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		post_scrambled_data(); 

		html_content_infos.clear(); 

		for( CONTAINER_INFOS::iterator it = html_content.begin(); 
			it != html_content.end(); 
			it ++ )
		{
			_ret = uninit_data_store( &( *it )->store ); 
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
			}

			_ret = release_container_content_info( &( *it )->content ); 
			_ret = init_data_data_store_param( &( *it )->store_param ); 
			( *it )->name = L""; 
			( *it )->have_ui = FALSE; 
            (*it)->_using = FALSE;
			//delete ( *it ); 
		}

		//html_content.clear(); 
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI release_all_html_page_data_store()
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		for( CONTAINER_INFOS::iterator it = html_content.begin(); 
			it != html_content.end(); 
			it ++ )
		{
			//post_data_file( &( *it )->store_param ); 
			_ret = uninit_data_store( &( *it )->store ); 
			if( _ret != ERROR_SUCCESS )
			{
				ret = _ret; 
			}
		}
	} while ( FALSE );

	return ret; 
}
