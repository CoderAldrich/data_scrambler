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
#include "local_config.h"

LRESULT WINAPI get_data_scrambler_config(wstring &old_conf_file)
{
    LRESULT ret = ERROR_SUCCESS; 

    ret = get_main_config(PREVIOUS_SCRIPT_FILE_PATH_SETTING_NAME, old_conf_file);
    return ret; 
}

LRESULT WINAPI get_main_config( LPCWSTR value_name, wstring &old_conf_file )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	WCHAR old_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
        ASSERT(NULL != value_name); 
        ASSERT(L'\0'!= *value_name); 

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = GetPrivateProfileStringW( MAIN_CONFIG_CAPTION, 
            value_name,
			L"", 
			old_config_file, 
			ARRAYSIZE( old_config_file ), 
			main_config_file ); 

		if( _ret == 0 
			|| _ret >= ARRAYSIZE( old_config_file ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		old_conf_file = old_config_file; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_data_scrambler_config(LPCWSTR old_conf_file)
{
    LRESULT ret = ERROR_SUCCESS;
    ret = set_main_config(PREVIOUS_SCRIPT_FILE_PATH_SETTING_NAME, old_conf_file); 
    return ret; 
}

LRESULT WINAPI set_main_config( LPCWSTR value_name, LPCWSTR old_conf_file)
{
    LRESULT ret = ERROR_SUCCESS; 
    HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		if( NULL == old_conf_file )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = WritePrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			value_name, 
			old_conf_file, 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_data_scrambler_config_ex(PCWSTR old_conf_file)
{
    LRESULT ret = ERROR_SUCCESS;
    ret = set_main_config_ex(PREVIOUS_SCRIPT_FILE_PATH_SETTING_NAME, old_conf_file); 
    return ret; 
}

LRESULT WINAPI set_main_config_ex( LPCWSTR value_name, LPCWSTR old_conf_file )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	WCHAR temp_file_name[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		if( NULL == old_conf_file )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( ( wcslen( old_conf_file ) > wcslen(main_config_file) ) 
			&& 0 == _wcsnicmp( old_conf_file, main_config_file, wcslen(main_config_file) ) )
		{
			hr = StringCchCopyW( temp_file_name, ARRAYSIZE( temp_file_name ), L".\\" ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			hr = StringCchCatW( temp_file_name, ARRAYSIZE( temp_file_name ), old_conf_file + wcslen(main_config_file) ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}
		else
		{
			hr = StringCchCopyW( temp_file_name, ARRAYSIZE( temp_file_name ), old_conf_file ); 
			if( FAILED(hr))
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

	
		_ret = WritePrivateProfileStringW( MAIN_CONFIG_CAPTION, 
            value_name,
			temp_file_name, 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_data_scrambler_conn_string( wstring &conn_string )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	WCHAR old_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = GetPrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			SQL_CONNECTION_STRING_SETTING_NAME, 
			L"", 
			old_config_file, 
			ARRAYSIZE( old_config_file ), 
			main_config_file ); 

		if( _ret == 0 
			|| _ret >= ARRAYSIZE( old_config_file ) )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		conn_string = old_config_file; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_data_scrambler_conn_string( LPCWSTR conn_string )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		if( NULL == conn_string )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = WritePrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			SQL_CONNECTION_STRING_SETTING_NAME, 
			conn_string, 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_working_mode( ULONG *mode )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	//ULONG _ret; 
	//WCHAR text[ MAX_PATH ]; 
	WCHAR main_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		ASSERT( NULL != mode ); 

		*mode = 0; 

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		*mode = ( ULONG )GetPrivateProfileInt( MAIN_CONFIG_CAPTION, 
			DATA_SCRAMBLER_WORK_MODE, 
			0, 
			main_config_file ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI get_local_user_info( LOCAL_USER_INFO *user_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR text[ MAX_PATH ]; 
	WCHAR main_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		ASSERT( NULL != user_info ); 

		user_info->password.clear(); 
		user_info->user_name.clear(); 

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = GetPrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			DATA_SCRAMBLER_USER_NAME, 
			L"", 
			text, 
			ARRAYSIZE( text ), 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		user_info->user_name = text; 

		_ret = GetPrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			DATA_SCRAMBLER_PASSWORD, 
			L"", 
			text, 
			ARRAYSIZE( text ), 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		user_info->password = text; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI set_local_user_info( LOCAL_USER_INFO *user_info )
{
	LRESULT ret = ERROR_SUCCESS; 
	HRESULT hr; 
	ULONG _ret; 
	WCHAR main_config_file[ MAX_PATH ]; 
	ULONG cc_ret_len; 

	do 
	{
		ASSERT( NULL != user_info ); 

		if( user_info->user_name.length() == 0 
			|| user_info->password.length() == 0 )
		{
			break; 
		}

		ret = get_app_path( main_config_file, ARRAYSIZE( main_config_file ), &cc_ret_len ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		hr = StringCchCatW( main_config_file, ARRAYSIZE( main_config_file ), SCRABLER_INI_FILE_NAME ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		_ret = WritePrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			DATA_SCRAMBLER_USER_NAME, 
			user_info->user_name.c_str(), 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = WritePrivateProfileStringW( MAIN_CONFIG_CAPTION, 
			DATA_SCRAMBLER_PASSWORD, 
			user_info->password.c_str(), 
			main_config_file ); 

		if( _ret == FALSE )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE ); 

	return ret; 
}
