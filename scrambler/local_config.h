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
#ifndef __LOCAL_CONFIG_H__
#define __LOCAL_CONFIG_H__

#include <string>
using namespace std; 

#define MAIN_CONFIG_CAPTION L"MAIN"
#define PREVIOUS_SCRIPT_FILE_PATH_SETTING_NAME L"PREV_SCRIPT"
#define PREVIOUS_EDIT_SCRIPT_FILE_NAME L"PREV_EDIT_SCRIPT"
#define SQL_CONNECTION_STRING_SETTING_NAME L"CONNECTION_STRING"
#define DATA_SCRAMBLER_USER_NAME L"USER_NAME"
#define DATA_SCRAMBLER_PASSWORD L"PASSWORD"
#define DATA_SCRAMBLER_WORK_MODE L"WORK_MODE"


#define SCRABLER_INI_FILE_NAME L"scramble.ini" 

typedef struct _LOCAL_USER_INFO 
{
	wstring user_name; 
	wstring password; 
} LOCAL_USER_INFO, *PLOCAL_USER_INFO; 

INLINE LRESULT WINAPI check_user_info_content_is_valid( LOCAL_USER_INFO *user_info )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != user_info ); 

		if( user_info->user_name.length() == 0 
			|| user_info->password.length() == 0 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	} while (FALSE ); 

	return ret; 
}

LRESULT WINAPI get_data_scrambler_config( wstring &old_conf_file ); 
LRESULT WINAPI set_data_scrambler_config( LPCWSTR old_conf_file ); 
LRESULT WINAPI set_data_scrambler_config_ex( LPCWSTR old_conf_file ); 

LRESULT WINAPI get_data_scrambler_conn_string( wstring &conn_string ); 
LRESULT WINAPI set_data_scrambler_conn_string( LPCWSTR conn_string ); 

LRESULT WINAPI get_local_user_info( LOCAL_USER_INFO *user_info ); 
LRESULT WINAPI set_local_user_info( LOCAL_USER_INFO *user_info ); 

LRESULT WINAPI get_working_mode( ULONG *mode ); 

LRESULT WINAPI set_main_config_ex(LPCWSTR value_name, LPCWSTR value); 
LRESULT WINAPI set_main_config(LPCWSTR value_name, LPCWSTR value);
LRESULT WINAPI get_main_config(LPCWSTR value_name, wstring &value);

#endif //__LOCAL_CONFIG_H__