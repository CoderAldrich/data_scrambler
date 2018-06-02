#ifndef __WEB_BROWSER_H__
#define __WEB_BROWSER_H__

//#define FIXED_WEB_PAGE_LOAD_TIME 1 

#ifdef _DEBUG
#define DEFAULT_DELAY_PROCESS_PAGE_TIME 8000
#define DEFAULT_WAIT_WEB_PAGE_LOAD_TIME 30000
#else
#define DEFAULT_DELAY_PROCESS_PAGE_TIME 8000
#define DEFAULT_WAIT_WEB_PAGE_LOAD_TIME 30000
#endif //_DEBUG

#define DATA_SCRAMBLER_MAIN_WINDOW_PROP_NAME L"DATA_SCRAMBLER" 
//typedef struct _SRAMBLE_LIST_WEB_PAGE_INFO 
//{
//	ULONG cc_url_len; 
//	ULONG cc_link_xpath_len; 
//	ULONG cc_next_xpath_len; 
//
//	WCHAR url[ 0 ]; 
//} SRAMBLE_LIST_WEB_PAGE_INFO, *PSRAMBLE_LIST_WEB_PAGE_INFO; 
//
//typedef struct _SRAMBLE_CONTANT_WEB_PAGE_INFO 
//{
//	ULONG cc_url_len; 
//	ULONG cc_xpaths_len; 
//	WCHAR url[ 0 ]; 
//} SRAMBLE_CONTANT_WEB_PAGE_INFO, *PSRAMBLE_CONTANT_WEB_PAGE_INFO; 

//typedef enum _SRAMBLE_INFO_TYPE
//{
//	FRAME_WEB_PAGE_SCRAMBLE, 
//	CONTENT_WEB_PAGE_SCRAMBLE, 
//} SRAMBLE_INFO_TYPE, *PSRAMBLE_INFO_TYPE; 

//typedef struct _LIST_SRAMBLE_INFO
//{
//	SRAMBLE_INFO_TYPE type; 
//	union
//	{
//		SRAMBLE_LIST_WEB_PAGE_INFO list_info; 
//		SRAMBLE_CONTANT_WEB_PAGE_INFO contant_info; 
//	};
//} LIST_SRAMBLE_INFO, *PLIST_SRAMBLE_INFO; 

//typedef struct _HTML_SCRAMBLED_LIST_DATA
//{
//	ULONG cc_next_link_len; 
//	ULONG cc_links_len; 
//	WCHAR next_link[ 1 ]; 
//} HTML_SCRAMBLED_LIST_DATA, *PHTML_SCRAMBLED_LIST_DATA; 
//
//typedef struct _HTML_SCRAMBLED_CONTANT_DATA
//{
//	ULONG cc_contant_len; 
//	WCHAR contant[ 1 ]; 
//} HTML_SCRAMBLED_CONTANT_DATA, *PHTML_SCRAMBLED_CONTANT_DATA; 
//
//typedef struct _HTML_SCRAMBLED_DATA
//{
//	SRAMBLE_INFO_TYPE type; 
//	union
//	{
//		HTML_SCRAMBLED_LIST_DATA list; 
//		HTML_SCRAMBLED_CONTANT_DATA contant; 
//	};
//} HTML_SCRAMBLED_DATA, *PHTML_SCRAMBLED_DATA; 

#define DATA_SCRAMBLE_PIPE_POINT_NAME L"\\\\.\\Pipe\\DataScramblePIPE0021%u"
#define COMMNAD_EXIT_WORK 0xffffffff 

#define WEB_RROWSER_PROCESS_SYNCH 1
#define WEB_RROWSER_LOAD_PAGE_NO_DELAY 2

LRESULT WINAPI _start_webbrowser( ULONG flags, ULONG *proc_id ); 

INLINE LRESULT WINAPI start_webbrowser()
{
	return _start_webbrowser( WEB_RROWSER_PROCESS_SYNCH, NULL ); 
}

ULONG CALLBACK web_page_bowser_manage_work( PVOID param ); 

LRESULT WINAPI embed_web_browser_window(HWND sub_window, 
										HWND parent_window, 
										RECT sub_window_rect ); 

LRESULT WINAPI execute_web_broswer_manage_command( PVOID data, ULONG data_size ); 

typedef LRESULT ( CALLBACK *ANALYZE_CLIENT_DATA_FUNC )( PVOID data, ULONG data_len, PVOID *ret_data, ULONG *ret_data_len ); 


HWND WINAPI find_web_browser_window( ULONG pid ); 

#define SCRABLER_INI_FILE_NAME L"scramble.ini" 

#define DEFAULT_HILIGHT_DELAY_TIME 8000

#ifdef FIXED_WEB_PAGE_LOAD_TIME
#define MAX_WEB_PAGE_LOAD_TIME 600000
#define MAX_PROCESS_DELAY_TIME 330000
#define MAX_HILIGHT_TIME 60000

#define MIN_WEB_PAGE_LOAD_TIME 8000
#define MIN_PROCESS_DELAY_TIME 30000
#define MIN_HILIGHT_TIME 5000
#else
#define MAX_WEB_PAGE_LOAD_TIME 60000
#define MAX_PROCESS_DELAY_TIME 33000
#define MAX_HILIGHT_TIME 60000

#define MIN_WEB_PAGE_LOAD_TIME 3000
#define MIN_PROCESS_DELAY_TIME 3000
#define MIN_HILIGHT_TIME 5000
#endif //FIXED_WEB_PAGE_LOAD_TIME

#define INVALID_TIME_VALUE 0xffffffff

LRESULT WINAPI get_html_action_global_config( ULONG *hilight_time, 
							   ULONG *web_page_load_time, 
							   ULONG *process_delay_time ); 

LRESULT WINAPI set_html_action_global_config( ULONG hilight_time, 
									  ULONG web_page_load_time, 
									  ULONG process_delay_time ); 

LRESULT WINAPI save_html_action_global_config(); 

LRESULT WINAPI _set_html_action_global_config( ULONG hilight_time, 
									   ULONG web_page_load_time, 
									   ULONG process_delay_time ); 

#endif //__WEB_BROWSER_H__