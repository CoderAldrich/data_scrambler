#include "stdafx.h"
#include "json_io.h"
#include "json/json.h"

#pragma  comment(lib, "lib_json.lib")

LRESULT WINAPI input_data_to_json( DATA_STORE *store, HTML_ELEMENT_ACTIONS *content ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	Json::Value root;
	Json::Value data;
	HTML_ELEMENT_ACTION *action; 
	HANDLE file_handle; 
	std::string text; 
	Json::FastWriter fw;

	do 
	{
		for( HTML_ELEMENT_ACTIONS_ITERATOR it = content->begin(); 
			it != content->end(); 
			it ++ )
		{
			action = ( *it ); 

			data[action->name.c_str()] = action->output.c_str(); 
			root.append(data); 
		}

		text = fw.write(root);

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
			LPCWSTR data_block; 

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

		//Json ::Reader freader;
		//Json::Value rootr;
		//freader.parse(msg, rootr);
		//for (int i = 0; i < size; i++)
		//{
		//	name = root[i]["name"].asString();
		//	age = root[i]["age"].asInt();
		//	std::cout << name << "  " << age << std::endl;
		//}
		//Json::Value::Members members(root[1].getMemberNames());

		//for (Json::Value::Members::iterator it  = members.begin(); it  != members.end(); ++it)
		//{
		//	const std::string &key  = *it;
		//	std::cout << key << std::endl;
		//}
	} while ( FALSE );

	return ret;
}