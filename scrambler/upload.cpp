#include "stdafx.h"
#include "upload.h"
#include "http_request_send.h"
#include "resource.h"

#define BALANCESOFT_HTTP_SUGGESTION_PAGE _T( "http://simplestai.com/suggest.php" )
//#define BALANCESOFT_HTTP_SUGGESTION_PAGE _T( "http://192.168.152.128:8081/suggest.php" )

LRESULT WINAPI upload_data_to_center( LPCTSTR text, ULONG length )
{
	LRESULT ret = ERROR_SUCCESS; 
	BOOL _ret; 
	http_data_sender text_sender; 
	static ULONG tick_count = 0; 

#define MIN_SUGGESTION_SEND_DELAY_TIME 30000

	if( tick_count == 0 )
	{
		tick_count = GetTickCount(); 
	}
	else if( ( GetTickCount() - tick_count ) < MIN_SUGGESTION_SEND_DELAY_TIME )
	{
		ret = ERROR_TIME_DELAY_TOO_SMALL; 
		goto _return; 
	}
	else
	{
		tick_count = GetTickCount(); 
	}

	ret = text_sender.add_text_count( _T( "suggest" ), text ); 
	if( ret != ERROR_SUCCESS )
	{
		goto _return; 
	}

	_ret = text_sender.send_all_data( BALANCESOFT_HTTP_SUGGESTION_PAGE ); 

	if( _ret == FALSE )
	{
		ret = ERROR_ERRORS_ENCOUNTERED; 
	}

_return:
	return ret; 
}