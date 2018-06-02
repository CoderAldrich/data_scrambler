#ifndef __DATA_SCRAMBLE_H__
#define __DATA_SCRAMBLE_H__

typedef vector<wstring> LINK_LIST; 
/*********************************************
list page
HTML|BODY|DIV|DIV id=matchid;name=matchid;|DIV|DIV|UL id=house-lst;name=house-lst;|LI|DIV|H2|A|
content page
HTML|BODY|DIV|SECTION|DIV|DIV|SPAN|
HTML|BODY|DIV|DIV|DIV|H1
*********************************************/
typedef struct _EXTERMAL_LINK_INFO
{
	wstring xpath; 
} EXTERMAL_LINK_INFO, *PEXTERMAL_LINK_INFO; 

#define MAX_TEXT_SIZE 65535
#define MAX_LONG_TEXT_SIZE (65535<<1)
typedef struct _HTML_ELEMENT_CONTENT
{
	_HTML_ELEMENT_CONTENT() : max_text_size( MAX_TEXT_SIZE ){ } 

	HTML_ELEMENT_ACTION *action; 
	ULONG max_text_size; 
	wstring name; 
	wstring xpath; 
	wstring text; 
	vector<wstring> additions; 
} HTML_ELEMENT_CONTENT, *PHTML_ELEMENT_CONTENT; 

//typedef map< wstring, HTML_ELEMENT_CONTENT* > HTML_ELEMENT_CONTENTS; 
typedef vector<HTML_ELEMENT_CONTENT*> HTML_ELEMENT_CONTENTS; 
typedef HTML_ELEMENT_CONTENTS::iterator HTML_ELEMENT_CONTENTS_ITERATOR; 

typedef struct _HTML_PAGE_CONTENT
{
	HTML_ELEMENT_CONTENTS element_contents; 
} HTML_PAGE_CONTENT, *PHTML_PAGE_CONTENT; 

typedef vector<HTML_PAGE_CONTENT*> HTML_PAGE_CONTENTS; 
typedef HTML_PAGE_CONTENTS::iterator HTML_PAGE_CONTENTS_ITERATOR; 

typedef LINK_LIST::iterator LINK_LIST_ITERATOR; 

typedef struct _HTML_LIST_ELEMENT_SCRAMBLE_INFO
{
	wstring domain; 
	wstring url; 
	wstring next_link_xpath; 
	wstring link_list_xpath; 
	LINK_LIST links; 
	LINK_LIST next_link; 
	HTML_PAGE_CONTENT page_content; 
} HTML_LIST_ELEMENT_SCRAMBLE_INFO, *PHTML_LIST_ELEMENT_SCRAMBLE_INFO; 

//typedef struct _HTML_PAGE_LIST_LINKS_SCRAMBLE_INFO
//{
//	wstring url; 
//	wstring next_link_xpath; 
//	wstring link_list_xpath; 
//	LINK_LIST links; 
//	LINK_LIST next_link; 
//} HTML_PAGE_LIST_LINKS_SCRAMBLE_INFO, *PHTML_PAGE_LIST_LINKS_SCRAMBLE_INFO; 
//
//typedef struct _HTML_PAGE_CONTENT_SCRAMBLE_INFO
//{
//	wstring url; 
//	HTML_PAGE_CONTENT page_content; 
//} HTML_PAGE_CONTENT_SCRAMBLE_INFO, *PHTML_PAGE_CONTENT_SCRAMBLE_INFO; 

typedef vector< HTML_LIST_ELEMENT_SCRAMBLE_INFO* > HTML_LIST_ELEMENT_SCRAMBLE_INFOS; 
typedef vector< HTML_LIST_ELEMENT_SCRAMBLE_INFO* >::iterator HTML_LIST_ELEMENT_SCRAMBLE_INFOS_ITERATOR; 

#endif //__DATA_SCRAMBLE_H__