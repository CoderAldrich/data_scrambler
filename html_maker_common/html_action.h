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

#ifndef __HTML_ACTION_H__
#define __HTML_ACTION_H__

#include <string>
using namespace std; 


#define HTML_ACTION_BROWSER L"browser"
#define HTML_ELEMENT_ACTION_NONE L"none"
//#define HTML_ELEMENT_ACTION_LOCATE_PARAM_OUTPUT_CONTENT L"page_content"
#define HTML_ELEMENT_ACTION_CLICK L"click"
#define HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE L"load_page"
#define HTML_ELEMENT_ACTION_OUTPUT L"output"
#define HTML_ELEMENT_ACTION_INPUT L"input"
#define HTML_ELEMENT_ACTION_JUMP L"jump"
#define HTML_ELEMENT_ACTION_BACK L"back"
#define HTML_ELEMENT_ACTION_LOCATE L"locate"
#define HTML_ELEMENT_ACTION_START L"start"
#define HTML_ELEMENT_ACTION_HILIGHT L"hilight"
#define HTML_ELEMENT_ACTION_HOVER L"hover"
#define HTML_ELEMENT_ACTION_SCRIPT L"script"

#define HTML_ACTION_BROWSER_A "browser"
#define HTML_ELEMENT_ACTION_NONE_A "none"
#define HTML_ELEMENT_ACTION_CLICK_A "click"
#define HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE_A "load_page"
#define HTML_ELEMENT_ACTION_OUTPU_A "output"
#define HTML_ELEMENT_ACTION_INPUT_A "input"
#define HTML_ELEMENT_ACTION_JUMP_A "jump"
#define HTML_ELEMENT_ACTION_START_A "start"
#define HTML_ELEMENT_ACTION_HILIGHT_A "hilight"

#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT L"text"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_IMAGE L"images"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK L"links"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_ALL_DATA L"all_data"

#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT_A "text"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK_A "links"
#define HTML_ELEMENT_ACTION_OUTPUT_PARAM_ALL_DATA_A "all_data"

#define HTML_ELEMENT_ACTION_NONE_PARAM_TIP L"tip"
#define HTML_ELEMENT_ACTION_NONE_PARAM_LINK L"link"
#define HTML_ELEMENT_ACTION_NONE_PARAM_TEXT L"text"
#define HTML_ELEMENT_ACTION_NONE_PARAM_OBJECT L"object"
#define HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER L"container"

#define HTML_ELEMENT_ACTION_NONE_PARAM_TIP_A "tip"
#define HTML_ELEMENT_ACTION_NONE_PARAM_LINK_A "link"
#define HTML_ELEMENT_ACTION_NONE_PARAM_TEXT_A "text"

#define HTML_ELEMENT_ACTION_RESPONSE_CONTINUE L"continue"
#define HTML_ELEMENT_ACTION_RESPONSE_CONTINUE_A "continue"

#define HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE L"work_done"
#define HTML_ELEMENT_ACTION_RESPONSE_WORK_DONE_A "work_done"

#define URL_DELIM_CHAR L";"
#define URL_DELIM_CHAR_A ";"

#define _URL_DELIM_CHAR L';'
#define _URL_DELIM_CHAR_A ';'

#define RESPONSE_TEXT_DELIM L';'

typedef struct _HTML_ACTION_CONTEXT
{
	wstring locate_url; 
	wstring current_url; 
	wstring current_page; 
	//_HTML_ELEMENT_ACTION *root; 
} HTML_ACTION_CONTEXT, *PHTML_ACTION_CONTEXT; 

typedef struct _HTML_ELEMENT_ACTION
{
	ULONG id; 
	ULONG jump_to_id; 
    ULONG relate_ui_id; 
	wstring html_text; 
	wstring title; 
	BOOLEAN in_frame; 
	BOOLEAN output_data; 
	wstring properties; 
	wstring xpath; 
	wstring action; 
	wstring param; 
	vector<wstring> outputs; 
	wstring name; 
    boolean new_page; 
	struct _HTML_ELEMENT_ACTION *sub_item; 
	struct _HTML_ELEMENT_ACTION *parent_item; 
	struct _HTML_ELEMENT_ACTION *next_item; 
	//PVOID context; 
	//struct _HTML_ELEMENT_ACTION *prev_item; 
} HTML_ELEMENT_ACTION, *PHTML_ELEMENT_ACTION; 

typedef vector< HTML_ELEMENT_ACTION* > HTML_ELEMENT_ACTIONS; 
typedef HTML_ELEMENT_ACTIONS::iterator HTML_ELEMENT_ACTIONS_ITERATOR;

#define GET_ROOT_ELEMENT_INFO 0x00000001
#define OUTPUT_ACTION_SUB_ELEMENT_LEVEL_COUNT 2
#define INFINITE_SEARCH_LEVEL_COUNT 0xffffffff

INLINE LRESULT WINAPI cat_output_data(HTML_ELEMENT_ACTION *action, wstring &output_data)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        output_data.clear();

        for (vector<wstring>::iterator it = action->outputs.begin();
            it != action->outputs.end();
            it++)
        {
            output_data += (*it);
            //output_data += L" "; 
        }
    } while (FALSE);

    return ret;
}

INLINE LRESULT WINAPI check_have_sub_page( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, 
			action->action.c_str() )
			&& 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_LINK, 
			action->param.c_str() ) )
		{
			break; 
		}

        if (0 == wcscmp(HTML_ELEMENT_ACTION_CLICK,
            action->action.c_str())
            && 0 == wcscmp(HTML_ELEMENT_ACTION_CLICK_PARAM_LOAD_PAGE,
                action->param.c_str()))
        {
            break;
        }

		ret = ERROR_INVALID_PARAMETER; 
		break; 
	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT WINAPI check_action_output( HTML_ELEMENT_ACTION *action )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT, 
			action->action.c_str() )
			&& 0 == wcscmp( HTML_ELEMENT_ACTION_OUTPUT_PARAM_TEXT, 
			action->param.c_str() ) )
		{
			break; 
		}

		ret = ERROR_INVALID_PARAMETER; 
		break; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI check_page_layout_node(HTML_ELEMENT_ACTION *action);; 

LRESULT WINAPI get_parent_page_layout_node_ex( HTML_ELEMENT_ACTION *root, 
									HTML_ELEMENT_ACTION *action, 
									HTML_ELEMENT_ACTION **valid_parent, 
									ULONG level, 
									ULONG *level_out ); 

LRESULT WINAPI get_parent_page_layout_node( HTML_ELEMENT_ACTION *action, 
									 HTML_ELEMENT_ACTION **valid_parent ); 

LRESULT WINAPI get_parent_source_action(HTML_ELEMENT_ACTION *action,
    HTML_ELEMENT_ACTION **valid_parent); 


LRESULT WINAPI locate_to_sub_page_layout_node( HTML_ELEMENT_ACTION *action, 
										 HTML_ELEMENT_ACTION **action_out ); 

LRESULT WINAPI locate_to_page_action( HTML_ELEMENT_ACTION *action, 
										  HTML_ELEMENT_ACTION **action_out ); 

LRESULT WINAPI locate_to_sub_page_action(HTML_ELEMENT_ACTION *action, 
    HTML_ELEMENT_ACTION **sub_action); 

#endif //__HTML_ACTION_H__