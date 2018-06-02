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

#ifndef __USER_REG_H__
#define __USER_REG_H__

#define MAX_PASSWORD 64
#define MAX_USER_NAME 128

typedef struct _USER_INFO
{
	wstring user_name; 
	wstring password; 
	wstring valid_time; 
	BOOLEAN can_use; 
}USER_INFO, *PUSER_INFO; 

#ifdef LOCAL_WEBSITE_TEST
#define GET_DM_USER_INFO_DIRECT_URL L"http://localhost/data_mining/user_info_response.php" 
#define DONATE_URL L"http://localhost/data_mining/register.php" 
#define GET_ACCESS_URL L"http://localhost/data_mining/get_access.php?u=%s" 
#else
#define POST_USER_INFO_URL L"http://www.simplestai.com/webkits/post_info.php" 
#define GET_DM_USER_INFO_DIRECT_URL L"http://www.simplestai.com/webkits/user_info_response.php" 
#define DONATE_URL L"http://www.simplestai.com/webkits/register.php" 
#define GET_ACCESS_URL L"http://www.simplestai.com/webkits/get_access.php?u=%s" 
#endif //LOCAL_WEBSITE_TEST

INLINE void init_user_info( USER_INFO *user_info )
{
	user_info->can_use = FALSE; 
	//memset( &user_info->valid_time, 0, sizeof( user_info->valid_time ) );  
	user_info->valid_time.clear(); 
	user_info->password.clear(); 
	user_info->user_name.clear(); 
}

#define ERROR_USER_DONOT_HAVE_ACCESS 0xea018011
LRESULT WINAPI get_user_info_from_server( USER_INFO *user_info, wstring &error_string ); 
LRESULT WINAPI check_current_user_access(HWND parent_wnd); 
LRESULT post_data_to_server( wstring &response, ULONG data_count, ... ); 

LRESULT WINAPI process_current_user_access( HWND parent_wnd ); 

#endif //__USER_REG_H__
