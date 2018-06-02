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
#include "html_common.h"
#include "html_action.h"

LRESULT WINAPI get_parent_page_layout_node_ex( HTML_ELEMENT_ACTION *root,
									HTML_ELEMENT_ACTION *action, 
									HTML_ELEMENT_ACTION **valid_parent, 
									ULONG level, 
									ULONG *level_out ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *parent; 
	INT32 i; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != valid_parent ); 
		ASSERT( NULL != root ); 

		*valid_parent = NULL; 
        ASSERT(0 != wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_NONE));

		if( NULL != level_out )
		{
			*level_out = INFINITE_SEARCH_LEVEL_COUNT; 
		}

		if( level == 0 )
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		//if( level == INFINITE_SEARCH_LEVEL_COUNT )
		//{
		//	*valid_parent = root; 
		//	break; 
		//}

		for( i = 0; ( ULONG )i < level; i ++ )
		{
			parent = action->parent_item; 
			if( NULL == parent )
			{
				if( NULL != level_out )
				{
					*level_out = i; 
				}

				*valid_parent = root; 
				break; 
			}

			if( ERROR_SUCCESS == check_page_layout_node( parent ) )
			{
				if( NULL != level_out )
				{
					*level_out = i; 
				}

				*valid_parent = parent; 
				break; 
			}

			ASSERT( parent->next_item == NULL 
				|| parent->next_item->action.length() == 0 
				|| 0 == wcscmp( parent->next_item->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 

			action = parent; 
		}

		if( i == level )
		{
			*valid_parent = root; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_page_layout_node(HTML_ELEMENT_ACTION *action)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        if (0 != wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_NONE)
            || 0 != wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER))
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

    } while (FALSE);

    return ret;
}

LRESULT WINAPI get_parent_source_action(HTML_ELEMENT_ACTION *action, 
    HTML_ELEMENT_ACTION **valid_parent)
{
    LRESULT ret = ERROR_SUCCESS;
    HTML_ELEMENT_ACTION *parent;

    do
    {
        ASSERT(NULL != action);
        ASSERT(NULL != valid_parent);

        *valid_parent = NULL;
        parent = action->parent_item;
        if (NULL == parent)
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

        if (ERROR_SUCCESS != check_page_layout_node(parent))
        {
            ret = ERROR_INVALID_PARAMETER; 
            break;
        }

        if( NULL == parent->parent_item )
        {
        	ret = ERROR_INVALID_PARAMETER; 
        	break; 
        }

        if( ERROR_SUCCESS == check_have_sub_page( parent->parent_item ) )
        {
        	*valid_parent = parent->parent_item; 
        	break; 
        }

        ret = ERROR_INVALID_PARAMETER;
        break;
    } while ( FALSE ); 
    return ret; 
}

LRESULT WINAPI get_parent_page_layout_node( HTML_ELEMENT_ACTION *action, 
									 HTML_ELEMENT_ACTION **valid_parent ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *parent; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != valid_parent ); 

		*valid_parent = NULL; 

        ASSERT(0 != wcscmp(action->action.c_str(), HTML_ELEMENT_ACTION_NONE)); 

        parent = action->parent_item; 
		if( NULL == parent )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ERROR_SUCCESS == check_page_layout_node( parent ) )
		{
			*valid_parent = parent; 
			break; 
		}

		//if( NULL == parent->parent_item )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		//if( ERROR_SUCCESS == check_have_sub_page( parent->parent_item ) )
		//{
		//	*valid_parent = parent->parent_item; 
		//	break; 
		//}

		ret = ERROR_INVALID_PARAMETER; 
		break; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI locate_to_sub_page_layout_node( HTML_ELEMENT_ACTION *action, HTML_ELEMENT_ACTION **action_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_action_next; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != action_out ); 

		*action_out = NULL; 
		
		if( action->sub_item == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_action_next = action->sub_item; 

		ASSERT( 0 == wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 

		if( _action_next->next_item == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_action_next = _action_next->next_item; 
		ASSERT( 0 == wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
        ASSERT(0 == wcscmp(_action_next->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER));

        if (_action_next->sub_item == NULL)
        {
            ret = ERROR_INVALID_PARAMETER;
            break;
        }

		*action_out = _action_next; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI locate_to_page_action( HTML_ELEMENT_ACTION *action, 
										  HTML_ELEMENT_ACTION **action_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	HTML_ELEMENT_ACTION *_action_next; 

	do 
	{
		ASSERT( NULL != action ); 
		ASSERT( NULL != action_out ); 

		*action_out = NULL; 

		if( action->sub_item == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		_action_next = action->sub_item; 

		ASSERT( 0 != wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 

		//if( _action_next->next_item == NULL )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	break; 
		//}

		//_action_next = _action_next->next_item; 
		//ASSERT( 0 != wcscmp( _action_next->action.c_str(), HTML_ELEMENT_ACTION_NONE ) ); 
		*action_out = _action_next; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI locate_to_sub_page_action(HTML_ELEMENT_ACTION *action, HTML_ELEMENT_ACTION **sub_action)
{
    LRESULT ret = ERROR_SUCCESS; 
    HTML_ELEMENT_ACTION *layout_node; 

    do 
    {
        ASSERT(NULL != sub_action); 
        *sub_action = NULL; 

        ret = locate_to_sub_page_layout_node(action, &layout_node);
        if (ret != ERROR_SUCCESS)
        {
            ASSERT(layout_node == NULL);
            break;
        }

        ret = locate_to_page_action(layout_node, sub_action);
        if (ret != ERROR_SUCCESS)
        {
            break; 
        }
    } while (FALSE);

    return ret; 
}