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
#include "ie_hook.h"

MYHOOKDATA myhookdata[NUMHOOKS]; 
static BOOL afHooks[NUMHOOKS];   

LRESULT WINAPI init_hook_config()
{
	LRESULT ret = ERROR_SUCCESS; 
    
    memset(&myhookdata, 0, sizeof(myhookdata)); 

    myhookdata[HOOK_INDEX_MOUSE].nType = WH_MOUSE; 
	myhookdata[HOOK_INDEX_MOUSE].hkprc = MouseProc; 

	memset( &afHooks, 0, sizeof( afHooks ) ); 
	return ret; 
}

LRESULT WINAPI set_hook( INT32 index )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( index < 0 
			|| index >= NUMHOOKS )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if (!afHooks[index])   
		{   
			myhookdata[index].hhook = SetWindowsHookEx(   
				myhookdata[index].nType, 
				myhookdata[index].hkprc, 
				( HINSTANCE )NULL, 
				GetCurrentThreadId() ); 

			afHooks[index] = TRUE; 
		} 
		else
		{
			ret = ERROR_NOT_INSTALLED; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI reset_hook( INT32 index )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( index < 0 
			|| index >= NUMHOOKS )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if (TRUE == afHooks[index])   
		{   
			UnhookWindowsHookEx(myhookdata[index].hhook);   
			afHooks[index] = FALSE;   
		} 
		else
		{
			ret = ERROR_NOT_INSTALLED; 
			break; 
		}
	}while( FALSE );
	return ret; 
}