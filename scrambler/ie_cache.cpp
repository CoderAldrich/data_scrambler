#include "stdafx.h"
#include "ie_cache.h"
#include <WinInet.h>

LRESULT WINAPI get_ie_cache_data( LPCWSTR url, on_data_readed callback_func, PVOID context )
{
	LRESULT ret = ERROR_SUCCESS;
	INT32 _ret; 
	LRESULT __ret; 
	LPINTERNET_CACHE_ENTRY_INFOW info = NULL; 
	ULONG info_size; 
	HANDLE stream_handle = NULL; 
#define IE_CACHE_READ_SIZE_ONE_TIME ( 1024 * 8 )
	PVOID _data = NULL; 
	ULONG _data_size; 
	//LARGE_INTEGER file_size; 
	ULONG offset; 

	do 
	{
		ASSERT( NULL != url ); 
		ASSERT( NULL != callback_func ); 

		info_size = 0; 
		stream_handle = RetrieveUrlCacheEntryStreamW( url, NULL, &info_size, FALSE, 0 ); 
		//_ret = GetUrlCacheEntryInfoEx( url, NULL, &info_size, NULL, NULL, NULL, 0 ); 

		if( NULL == stream_handle )
		{
			ret = GetLastError(); 
			if( ERROR_INSUFFICIENT_BUFFER != ret )
			{
				SAFE_SET_ERROR_CODE( ret ); 
				break; 
			}

			if( info_size == 0 )
			{
				ret = ERROR_ERRORS_ENCOUNTERED; 
				break; 
			}

			info = ( LPINTERNET_CACHE_ENTRY_INFOW )malloc( info_size ); 
			if( NULL == info )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}
		else
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = ERROR_SUCCESS; 
		stream_handle = RetrieveUrlCacheEntryStreamW( url, info, &info_size, FALSE, 0 ); 
		if( NULL == stream_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		//dbg_print_w( MSG_IMPORTANT, L"url name of ie cache is %s\n", info->lpszSourceUrlName ); 
		//SetUrlCacheEntryInfo()

		if( 0 != info->dwSizeHigh )
		{
			ret = ERROR_NOT_SUPPORTED; 
			break; 
		}

		//file_size.HighPart = info->dwSizeHigh; 
		//file_size.LowPart = info->dwSizeLow; 

		offset = 0; 
		_data = malloc( IE_CACHE_READ_SIZE_ONE_TIME ); 
		if( NULL == _data )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		for( ; ; )
		{
			//if( offset > 0xffffffff )
			//{
			//	break; 
			//}

			_data_size = IE_CACHE_READ_SIZE_ONE_TIME; 
			_ret = ReadUrlCacheEntryStream( stream_handle, 
				( ULONG )offset, 
				_data, 
				&_data_size, 
				NULL ); 

			if( FALSE == _ret )
			{
				ret = GetLastError(); 
				break; 
			}

			if( _data_size == 0 )
			{
				break; 
			}

			__ret = callback_func( _data, _data_size, context ); 
			if( __ret != ERROR_SUCCESS )
			{
				//break; 
			}

			offset += _data_size; 
		}
	} while ( FALSE );

	if( NULL != stream_handle )
	{
		UnlockUrlCacheEntryStream( stream_handle, 0 ); 
	}

	if( NULL != info )
	{
		free( info ); 
	}

	if( NULL != _data )
	{
		free( _data ); 
	}

	return ret; 
}

//LRESULT WINAPI get_ie_cache_file_name( LPCWSTR url, LPWSTR file_name, ULONG cc_buf_len, ULONG *cc_ret_len )
//{
//	LRESULT ret = ERROR_SUCCESS;
//	INT32 _ret; 
//	LPINTERNET_CACHE_ENTRY_INFOW info = NULL; 
//	ULONG info_size; 
//
//	do 
//	{
//		ASSERT( NULL != url ); 
//		ASSERT( NULL != file_name ); 
//		ASSERT( 0 < cc_buf_len ); 
//
//		_ret = GetUrlCacheEntryInfoEx( url, NULL, &info_size, NULL, NULL, NULL, 0 ); 
//		if( FALSE ==_ret )
//		{
//			ret = GetLastError(); 
//			if( ERROR_INSUFFICIENT_BUFFER != ret )
//			{
//				SAFE_SET_ERROR_CODE( ret ); 
//				break; 
//			}
//
//			if( info_size == 0 )
//			{
//				ret = ERROR_ERRORS_ENCOUNTERED; 
//				break; 
//			}
//
//			info = ( LPINTERNET_CACHE_ENTRY_INFOW )malloc( info_size ); 
//			if( NULL == info )
//			{
//				ret = ERROR_NOT_ENOUGH_MEMORY; 
//				break; 
//			}
//		}
//		else
//		{
//			ret = ERROR_ERRORS_ENCOUNTERED; 
//			break; 
//		}
//
//		_ret = GetUrlCacheEntryInfoEx( url, info, info_size, NULL, NULL, NULL, 0 ); 
//		if( FALSE == _ret )
//		{
//			SAFE_SET_ERROR_CODE( ret ); 
//			break; 
//		}
//
//		dbg_print_w( MSG_IMPORTANT, L"url name of ie cache is %s\n", info->lpszSourceUrlName ); 
//
//		//SetUrlCacheEntryInfo()
//	} while ( FALSE );
//
//	return ret; 
//}

#if 0
{
	LRESULT ret = ERROR_SUCCESS; 
	INTERNET_CACHE_ENTRY_INFOW info; 
	DWORD dwEntrySize;
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;
	DWORD MAX_CACHE_ENTRY_INFO_SIZE = 4096;
	HANDLE hCacheDir;
	INT32 nCount=0;

	// "cookie:", "visited:", 
	dwEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
	lpCacheEntry = ( LPINTERNET_CACHE_ENTRY_INFO )malloc( dwEntrySize ); 
	if( NULL == lpCacheEntry )
	{
		goto _return; 
	}

	lpCacheEntry->dwStructSize = dwEntrySize; 
	lpCacheEntry->lpszSourceUrlName = ( LPWSTR )url; 
again:
	hCacheDir = FindFirstUrlCacheEntry( NULL,
		lpCacheEntry,
		&dwEntrySize); 

	if( NULL == hCacheDir ) 
	{
		delete[]lpCacheEntry; 

		switch(GetLastError())
		{
		case ERROR_NO_MORE_ITEMS: 
			TCHAR tempout[80];
			_stprintf_s(tempout, 
				80,   
				TEXT("The number of cache entries = %d \n"),
				nCount);
			FindCloseUrlCache(hCacheDir); 
			goto _return; 
			break;
		case ERROR_INSUFFICIENT_BUFFER:
			lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)malloc( dwEntrySize );
			lpCacheEntry->dwStructSize = dwEntrySize;
			goto again;
			break;
		default:
			dbg_print( MSG_FATAL_ERROR, 
				"FindNextUrlCacheEntry Init");
			FindCloseUrlCache(hCacheDir);
			SAFE_SET_ERROR_CODE( ret ); 
			goto _return; 
			break; 
		}
	}

	nCount++;
	delete (lpCacheEntry);

	do 
	{
		dwEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
		lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)malloc( dwEntrySize );
		lpCacheEntry->dwStructSize = dwEntrySize;

retry:
		if (!FindNextUrlCacheEntry(hCacheDir,
			lpCacheEntry, 
			&dwEntrySize))
		{
			delete[]lpCacheEntry;
			switch(GetLastError())
			{
			case ERROR_NO_MORE_ITEMS: 
				TCHAR tempout[80];
				_stprintf_s(tempout,
					80,
					TEXT("The number of cache entries = %d \n"),nCount);

				FindCloseUrlCache(hCacheDir);
				goto _return; 
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				lpCacheEntry = ( LPINTERNET_CACHE_ENTRY_INFO )malloc( dwEntrySize );
				lpCacheEntry->dwStructSize = dwEntrySize;
				goto retry;
				break;
			default:
				dbg_print( MSG_IMPORTANT, "FindNextUrlCacheEntry Init");
				FindCloseUrlCache(hCacheDir); 
				SAFE_SET_ERROR_CODE( ret ); 
				goto _return; 
				break; 
			}

			goto _return; 
		}

		nCount ++;

		free( lpCacheEntry );        
		lpCacheEntry = NULL; 
	}while( FALSE );     

_return: 
	if( NULL != lpCacheEntry)
	{
		free( lpCacheEntry ); 
	}
	return ret; 
}
#endif //0
//void   ClearInternetCache()   
//{   
//	DWORD   dwNeeded   =   0;   
//	FindFirstUrlCacheEntry(NULL,   NULL,   &dwNeeded);   
//
//	if   (GetLastError()   ==   ERROR_INSUFFICIENT_BUFFER)   
//	{   
//		unsigned   char   *buffer   =   new   unsigned   char[dwNeeded];   
//		try   
//		{   
//			LPINTERNET_CACHE_ENTRY_INFO   lpicei   =   
//				reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(buffer);   
//			HANDLE   HFind   =     FindFirstUrlCacheEntry(NULL,   lpicei,   &dwNeeded);   
//			DeleteUrlCacheEntry(lpicei->lpszSourceUrlName);   
//
//			bool   no_more_files   =   false;   
//			while   (!no_more_files)   
//			{   
//				if   (FindNextUrlCacheEntry(HFind,   lpicei,   &dwNeeded))   
//				{   
//					DeleteUrlCacheEntry(lpicei->lpszSourceUrlName);   
//				}   
//				else   switch   (GetLastError())   
//				{   
//	case   ERROR_INSUFFICIENT_BUFFER:   
//		{   
//			delete   []   buffer;   
//			buffer   =   new   unsigned   char[dwNeeded];   
//			lpicei   =   reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(buffer);   
//			break;   
//		}   
//	default:   
//		{   
//			no_more_files   =   true;   
//			break;   
//		}   
//				}   
//			}
//			FindCloseUrlCache(HFind);
//		}   
//		catch   (...)   
//		{   
//			delete   []   buffer;   
//		}   
//		delete   []   buffer;                   
//	}   
//}
////RetrieveUrlCacheEntryFile
//
//int WINAPI GetCacheEntryInfo(HWND hX,LPTSTR lpszUrl)
//{
//	DWORD dwEntrySize=0;
//	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry;
//
//	SetCursor(LoadCursor(NULL,IDC_WAIT));
//	if (!GetUrlCacheEntryInfo(lpszUrl,NULL,&dwEntrySize))
//	{
//		if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
//		{
//			ErrorOut(hX,GetLastError(),TEXT("GetUrlCacheEntryInfo"));
//			SetCursor(LoadCursor(NULL,IDC_ARROW));
//			return FALSE;
//		}
//		else
//			lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) 
//			new char[dwEntrySize];
//	}
//	else
//		return FALSE; // should not be successful w/ NULL buffer
//	// and 0 size
//
//	if (!GetUrlCacheEntryInfo(lpszUrl,lpCacheEntry,&dwEntrySize))
//	{
//		ErrorOut(hX,GetLastError(),TEXT("GetUrlCacheEntryInfo"));
//		SetCursor(LoadCursor(NULL,IDC_ARROW));
//		return FALSE;
//	}
//	else
//	{
//		if ((lpCacheEntry->dwHeaderInfoSize)!=0)
//		{
//			LPSTR(lpCacheEntry->lpHeaderInfo)
//				[lpCacheEntry->dwHeaderInfoSize]=TEXT('\0');
//			SetDlgItemText(hX,IDC_Headers,
//				lpCacheEntry->lpHeaderInfo);
//		}
//		else
//		{
//			SetDlgItemText(hX,IDC_Headers,TEXT("None"));
//		}
//
//		SetCursor(LoadCursor(NULL,IDC_ARROW));
//		return TRUE;
//	}
//
//}
//
//int WINAPI CommitEntry(HWND hX)
//{
//	LPTSTR lpszUrl, lpszExt, lpszFileName;
//	LPTSTR lpszData,lpszSize;
//	DWORD dwSize;
//	DWORD dwEntryType=0;
//	FILE *lpfCacheEntry;
//	LPFILETIME lpdtmExpire, lpdtmLastModified;
//	LPSYSTEMTIME lpdtmSysTime;
//	errno_t err;
//
//	if( SendDlgItemMessage(hX,IDC_RBNormal,BM_GETCHECK,0,0) )
//	{
//		dwEntryType = dwEntryType + NORMAL_CACHE_ENTRY;
//	}
//	else if( SendDlgItemMessage(hX,IDC_RBSticky, BM_GETCHECK,0,0) )
//	{
//		dwEntryType = dwEntryType + STICKY_CACHE_ENTRY;
//	}
//	else if(SendDlgItemMessage( hX,IDC_RBSparse, BM_GETCHECK,0,0) )
//	{
//		dwEntryType = dwEntryType + SPARSE_CACHE_ENTRY;
//	}
//
//
//	if( SendDlgItemMessage(hX,IDC_RBCookie, BM_GETCHECK,0,0))
//	{
//		dwEntryType = dwEntryType + COOKIE_CACHE_ENTRY;
//	}
//	else if( SendDlgItemMessage(hX,IDC_RBUrl, BM_GETCHECK,0,0) )
//	{
//		dwEntryType = dwEntryType + URLHISTORY_CACHE_ENTRY;
//	}
//
//
//	if( SendDlgItemMessage(hX,IDC_RBNone, BM_GETCHECK,0,0) )
//	{
//		dwEntryType=0;
//	}
//
//	lpdtmSysTime = new SYSTEMTIME;
//	lpdtmExpire = new FILETIME;
//	lpdtmLastModified = new FILETIME;
//
//	GetLocalTime(lpdtmSysTime);
//	SystemTimeToFileTime(lpdtmSysTime,lpdtmExpire);
//	SystemTimeToFileTime(lpdtmSysTime,lpdtmLastModified);
//	delete(lpdtmSysTime);
//
//	lpszUrl = new TCHAR[MAX_PATH];
//	lpszFileName = new TCHAR[MAX_PATH];
//	lpszExt = new TCHAR[5];
//	lpszSize = new TCHAR[10];
//
//	GetDlgItemText(hX,IDC_SourceURL,lpszUrl,MAX_PATH);
//	GetDlgItemText(hX,IDC_LocalFile,lpszFileName,MAX_PATH);
//	GetDlgItemText(hX,IDC_FileExt,lpszExt,5);
//
//	GetDlgItemText(hX,IDC_SizeLow,lpszSize,10);
//	dwSize = (DWORD)_ttol(lpszSize);
//	delete(lpszSize);
//
//	if (dwSize==0)
//	{
//		if((MessageBox(hX,
//			TEXT("Incorrect File Size.\nUsing 8000 characters, Okay?\n"),
//			TEXT("Commit Entry"),MB_YESNO))
//			==IDYES)
//		{
//			dwSize = 8000;
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//
//	lpszData = new TCHAR[dwSize];
//	GetDlgItemText(hX,IDC_CacheDump,lpszData,dwSize);
//
//	err = _tfopen_s(&lpfCacheEntry,lpszFileName,_T("w"));
//	if (err)
//		return FALSE;
//	fprintf(lpfCacheEntry,"%s",lpszData);
//	fclose(lpfCacheEntry);
//	delete(lpszData);
//
//	if ( !CommitUrlCacheEntry( lpszUrl, 
//		lpszFileName, 
//		*lpdtmExpire,
//		*lpdtmLastModified, 
//		dwEntryType,
//		NULL,
//		0,
//		lpszExt,
//		0) )
//	{
//		ErrorOut(hX,GetLastError(),TEXT("Commit Cache Entry"));
//		delete(lpszUrl);
//		delete(lpszFileName);
//		delete(lpszExt);
//		delete(lpdtmExpire);
//		delete(lpdtmLastModified);
//		return FALSE;
//	}
//	else
//	{
//		delete(lpszUrl);
//		delete(lpszFileName);
//		delete(lpszExt);
//		delete(lpdtmExpire);
//		delete(lpdtmLastModified);
//		return TRUE;
//	}
//}
//
//lpDstCEInfo->lpszSourceUrlName = 
//(LPINTERNET_CACHE_ENTRY_INFO) ((LPBYTE) lpSrcCEInfo + 
//							   ((DWORD)(lpOldCEInfo->lpszSourceUrlName) - (DWORD)lpOldCEInfo));
//

#if 0
INT32 WINAPI EnumerateCacheOld()
{
	DWORD dwEntrySize;
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry;
	DWORD MAX_CACHE_ENTRY_INFO_SIZE = 4096;
	HANDLE hCacheDir;
	int nCount=0;

	dwEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
	lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
	lpCacheEntry->dwStructSize = dwEntrySize;

again:

	hCacheDir = FindFirstUrlCacheEntry(NULL,
		lpCacheEntry,
		&dwEntrySize);
	if (!hCacheDir)                                             
	{
		delete[]lpCacheEntry;
		switch(GetLastError())
		{
		case ERROR_NO_MORE_ITEMS: 
			TCHAR tempout[80];
			_stprintf_s(tempout, 
				80,   
				TEXT("The number of cache entries = %d \n"),
				nCount);
			FindCloseUrlCache(hCacheDir);
			return TRUE;
			break;
		case ERROR_INSUFFICIENT_BUFFER:
			lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) 
				new char[dwEntrySize];
			lpCacheEntry->dwStructSize = dwEntrySize;
			goto again;
			break;
		default:
			dbg_print( MSG_FATAL_ERROR, 
				"FindNextUrlCacheEntry Init");
			FindCloseUrlCache(hCacheDir);
			return FALSE;
		}
	}

	nCount++;
	delete (lpCacheEntry);

	do 
	{
		dwEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
		lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
		lpCacheEntry->dwStructSize = dwEntrySize;

retry:
		if (!FindNextUrlCacheEntry(hCacheDir,
			lpCacheEntry, 
			&dwEntrySize))
		{
			delete[]lpCacheEntry;
			switch(GetLastError())
			{
			case ERROR_NO_MORE_ITEMS: 
				TCHAR tempout[80];
				_stprintf_s(tempout,
					80,
					TEXT("The number of cache entries = %d \n"),nCount);

				FindCloseUrlCache(hCacheDir);
				return TRUE;
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				lpCacheEntry = 
					(LPINTERNET_CACHE_ENTRY_INFO) 
					new char[dwEntrySize];
				lpCacheEntry->dwStructSize = dwEntrySize;
				goto retry;
				break;
			default:
				dbg_print( MSG_IMPORTANT, "FindNextUrlCacheEntry Init");
				FindCloseUrlCache(hCacheDir);
				return FALSE;
			}
		}

		nCount++;
		delete[] lpCacheEntry;        
	}  while (TRUE);

	return TRUE;        
}
#endif //0
