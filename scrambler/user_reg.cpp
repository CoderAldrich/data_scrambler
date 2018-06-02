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
#include "user_reg.h"
#include "login_dlg.h"
#include "data_sender.h"
#include "md5.h"
//#include "VMProtectSDK.h"

LRESULT WINAPI set_default_user_info( LOCAL_USER_INFO *info )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != info ); 

		info->password = L""; 
		info->user_name = L""; 

	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI check_current_user_access(HWND parent_wnd)
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	login_dlg dlg( CWnd::FromHandle( parent_wnd ) ); 
	LOCAL_USER_INFO user_info; 
	USER_INFO _user_info; 
	wstring error_string; 

	//VMProtectBeginUltra( "check_current_user_access" ); 

	do 
	{
		do 
		{
			ret = get_local_user_info( &user_info ); 
			if( ERROR_SUCCESS != ret )
			{
				break; 
			}

			ret = check_user_info_content_is_valid( &user_info ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE );

#if 1
		if( ret != ERROR_SUCCESS )
		{
			set_default_user_info( &user_info ); 
		}

		_user_info.user_name = user_info.user_name; 
		_user_info.password = user_info.password; 

		ret = get_user_info_from_server( &_user_info, error_string ); 
		if( _user_info.can_use == TRUE )
		{
		}
#else
		if( ret != ERROR_SUCCESS )
		{
			//set_default_user_info( &user_info ); 
//#ifdef _REGISTER_USER
			if( IDOK != dlg.DoModal() )
			{
				ret = ERROR_INVALID_PASSWORDNAME; 
				break; 
			}

			dlg.get_user_info( &user_info ); 
			//init_user_info( &_user_info ); 
			//break; 
//#endif //_REGISTER_USER
		}

		for( ; ; )
		{
			_user_info.user_name = user_info.user_name; 
			_user_info.password = user_info.password; 

			ret = get_user_info_from_server( &_user_info, error_string ); 
			if( _user_info.can_use == TRUE )
			{
				user_info.user_name = _user_info.user_name; 
				user_info.password = _user_info.password; 

				_ret = set_local_user_info( &user_info); 
				if( _ret != ERROR_SUCCESS )
				{
				}
				break; 
			}
			else
			{
				if( ret == ERROR_USER_DONOT_HAVE_ACCESS )
				{
					wstring message; 
					message = error_string; 
					message = L"请点击查看使用方法了解"; 

					MessageBox( NULL, error_string.c_str(), L"", 0 ); 
					//usage_intro_dialog dlg; 
					//dlg.DoModal(); 
				}
				else
				{
					if( error_string.length() > 0 )
					{
						MessageBox( NULL, error_string.c_str(), L"", 0 ); 
					}
				}

				if(IDOK != dlg.DoModal() )
				{
					ret = ERROR_INVALID_PASSWORDNAME; 
					break; 
				}

				dlg.get_user_info( &user_info ); 
			}
		}
#endif //0
	}while( FALSE );
	
	//VMProtectEnd(); 

	return ret; 
}

LRESULT WINAPI process_current_user_access( HWND parent_wnd )
{
	LRESULT ret = ERROR_SUCCESS; 
	USER_INFO _user_info; 
	wstring error_string; 

	ret = check_current_user_access( parent_wnd );  
	return ret; 
}

LRESULT post_data_to_server( wstring &response, ULONG data_count, ... ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	DATA_ITEMS input_data; 
	DATA_ITEMS output_data; 
	DATA_ITEM *data_item; 
	va_list va; 
	ULONG i; 

	do
	{
		if( 0 == data_count )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		va_start( va, data_count ); 
		for( i = 0; i < data_count; i ++ )
		{
			data_item = new DATA_ITEM(); 
			if( data_item == NULL )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			data_item->name = ( LPWSTR )va_arg( va, LPWSTR ); 
			data_item->data_text= ( LPWSTR )va_arg( va, LPWSTR ); 

			input_data.push_back( data_item ); 
		}
		va_end( va ); 

		ret = send_data_over_http( POST_USER_INFO_URL, &input_data, &output_data ); 

		if( output_data.size() == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		response = output_data[ 0 ]->data_text.c_str(); 

	}while( FALSE ); 

	release_data_items( &input_data ); 
	release_data_items( &output_data ); 
	return ret; 
}

#define MD5_HASH_LENGTH 16

LRESULT WINAPI buffer_to_text_w( UCHAR *buffer, ULONG size, wstring &text )
{
	LRESULT ret = ERROR_SUCCESS; 
	WCHAR buf[3]; 
	text.clear(); 

	for( INT32 i=0; ( ULONG )i<size; i++ )  
	{
		StringCchPrintfW(buf, ARRAYSIZE( buf ), L"%02x", buffer[ i ] );  
		text += buf; 
	}
	
	return ret; 
}  

LRESULT WINAPI get_user_info_from_server( USER_INFO *user_info, 
										 wstring &error_string )
{
	LRESULT ret = ERROR_SUCCESS; 
	//LRESULT _ret; 
	DATA_ITEMS input_data; 
	DATA_ITEMS output_data; 
	DATA_ITEM *data_item; 

	do
	{
		if( NULL == user_info )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		user_info->can_use = FALSE; 

		data_item = new DATA_ITEM(); 
		if( data_item == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		data_item->name = L"username"; 
		data_item->data_text= user_info->user_name.c_str(); 

		input_data.push_back( data_item ); 

		data_item = new DATA_ITEM(); 
		if( data_item == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		data_item->name = L"password_md5"; 
		
		{
			context_md5_t ctx; 
			string _password; 
			wstring __password; 

			UCHAR password_md5[ MD5_HASH_LENGTH + 1 ]; 

			ret = unicode_to_utf8_ex( user_info->password.c_str(), _password ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			MD5Init( &ctx ); 
			MD5Update( &ctx, ( CONST UCHAR* )_password.c_str(), _password.length() ); 
			MD5Final( password_md5, &ctx ); 

			password_md5[ MD5_HASH_LENGTH ] = '\0'; 

			buffer_to_text_w(password_md5, MD5_HASH_LENGTH, __password ); 

			data_item->data_text= __password; 
		}
		
		input_data.push_back( data_item ); 

		ret = send_data_over_http( GET_DM_USER_INFO_DIRECT_URL, &input_data, &output_data ); 

		if( output_data.size() == 0 )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

#define VALUE_VALID_INDEX 0
#define VALUE_MESSAGE_INDEX 1
#define VALUE_ERROR_CODE_INDEX 2
#define VALUE_VALID_TIME_INDEX 3

		if( output_data.size() > VALUE_MESSAGE_INDEX )
		{
			CString message; 
			message = output_data[ VALUE_MESSAGE_INDEX ]->data_text.c_str(); 

			error_string = message.GetBuffer(); 
		}

		if( output_data.size() > VALUE_VALID_TIME_INDEX )
		{

			//string text; 
			//_ret = unicode_to_utf8_ex( [ VALUE_VALID_TIME_INDEX ]->data_text.c_str(), text );  
			//if( _ret != ERROR_SUCCESS )
			//{
			//	break; 
			//}

			//if( 6 != sscanf_s( text.c_str(), 
			//	"%4u-%2u-%2u %2u:%2u:%2u", 
			//	&user_info->valid_time.Year, 
			//	&user_info->valid_time.Month, 
			//	&user_info->valid_time.Day,
			//	&user_info->valid_time.Hour, 
			//	&user_info->valid_time.Minute, 
			//	&user_info->valid_time.Second ) )
			//{
			//	break; 
			//}

			user_info->valid_time = output_data[ VALUE_VALID_TIME_INDEX ]->data_text.c_str(); 
		}

		if( output_data.size() > VALUE_ERROR_CODE_INDEX )
		{
			LPWSTR temp_text; 
			ULONG error_code; 
			
			error_code = wcstoul( output_data[ VALUE_ERROR_CODE_INDEX ]->data_text.c_str(), 
				&temp_text, 0 ); 

#define ERROR_CODE_USER_HAVE_NOT_ACCESS ( 1001 )
			if( ERROR_CODE_USER_HAVE_NOT_ACCESS == error_code )
			{
				ret = ERROR_USER_DONOT_HAVE_ACCESS; 
				break; 
			}
			else if( 0 != error_code )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}
		}

		if( output_data.size() > VALUE_VALID_INDEX )
		{
			LPWSTR temp_text; 
			ULONG is_valid; 

			is_valid = wcstoul( output_data[ VALUE_VALID_INDEX ]->data_text.c_str(), 
				&temp_text, 0 ); 

			if( 1 != is_valid )
			{
				ASSERT( FALSE ); 
				ret = ERROR_USER_DONOT_HAVE_ACCESS; 
				break; 
			}
			else
			{
				ASSERT( is_valid == 1 ); 
			}
		}
		else
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			ASSERT( FALSE ); 
			break; 
		}

		user_info->can_use = TRUE; 
	}while( FALSE ); 

	release_data_items( &input_data ); 
	release_data_items( &output_data ); 

	return ret; 
}

LRESULT WINAPI user_authority_warning()
{
	;LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI using_time_limit()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_licence()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	}while( FALSE );

	return ret; 	
}


#if 0
void IconMT()
{
	BOOL bIconFlag = 0;
	CTime t = CTime::GetCurrentTime();
	CString a;

	t = CTime::GetCurrentTime();// add 试用版本 有效期 30天

	int i = 0;
	char direct[256];
	int dirsize=256;
	char day[5];
	unsigned char value[256];
	HKEY keyRet;

	day [0] = NULL;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T("SOFTWARE//Microsoft//Windows//CurrentVersion")
		,0,KEY_READ|KEY_WRITE,&keyRet)==ERROR_SUCCESS)
	{
		DWORD type=REG_SZ;
		DWORD size = 256;
		value[0] = 0;
		if (RegQueryValueEx(keyRet,_T("MProductId"),NULL,&type,value,&size) != ERROR_SUCCESS)
		{
			a = "000-0098007-05662";
			a = a + (LPCTSTR)t.Format(_T("%d"));
			strcpy(direct, (LPCSTR)CStringA((LPCWSTR)a));
			DWORD nowsize=strlen(direct);
			RegSetValueEx(keyRet,_T("MProductId"),NULL,REG_SZ,(unsigned char*)direct,nowsize);
		}
		sscanf((char*)value, "%d-0098007-05662%s", &i, day);

		if (i < 31)
		{
			a = day;
			if (a.Compare((LPCTSTR)t.Format(_T("%d"))) != 0)
				i++;
		}
		else
		{
			if(i < 100)
				i+=100;
		}
		strcpy(day, (char *)(LPCTSTR)t.Format(_T("%d")));
		sprintf((char*)value, "%03d-0098007-05662%s", i, day);
		DWORD nowsize=strlen((char*)value);
		RegSetValueEx(keyRet,_T("MProductId"),NULL,REG_SZ,(unsigned char*)value,nowsize);//MProductId
		RegCloseKey(keyRet);
	}
	if (i < 31)
	{
		a.Format(_T("监控机Agnet试用版  您还有%d天"), 31-i);
	else
		if (i != 100)
		{
			a.Format(_T("监控机Agnet试用版已经到期了."));
		}
		else
			a.Format(_T("监控机Agnet"));
} 
#endif //0