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
#include "pipe_line.h"
#include "web_page_browser_manage.h"
#include "webbrowser.h"

WEB_BROWSER_PROCESSES web_browser_processes;
WEB_BROWSER_PROCESS_MAP web_browser_processe_map;

LRESULT WINAPI close_server_named_pipe(WEB_BROWSER_PROCESS *web_browser_info)
{
    LRESULT ret = ERROR_SUCCESS;
    INT32 _ret;
    ULONG client_process_id = 0;

    //ASSERT( FALSE ); 
    _ret = GetNamedPipeClientProcessId(web_browser_info->ipc_info.point.pipe,
        &client_process_id);

    if (_ret == TRUE)
    {
        ASSERT(client_process_id != 0);
        ASSERT(TRUE == web_browser_info->ipc_info.pipe_connected);
        ret = disconnect_name_pipe_client(web_browser_info->ipc_info.point.pipe);
        if (ret != ERROR_SUCCESS)
        {
            //break; 
        }

        web_browser_info->ipc_info.pipe_connected = FALSE;
    }

    ASSERT(FALSE == web_browser_info->ipc_info.pipe_connected);

    if (web_browser_info->ipc_info.point.pipe != NULL)
    {
        ASSERT(TRUE == web_browser_info->ipc_info.pipe_created);
        close_pipe(web_browser_info->ipc_info.point.pipe);
        web_browser_info->ipc_info.point.pipe = NULL;
        web_browser_info->ipc_info.pipe_created = FALSE;
        web_browser_info->ipc_info.pipe_connected = FALSE;
    }

    if (client_process_id != 0)
    {
        HANDLE client_process;
        ULONG wait_ret;

        client_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, client_process_id);
        if (NULL != client_process)
        {
            do
            {
                wait_ret = WaitForSingleObject(client_process, 5000);
                if (wait_ret != WAIT_OBJECT_0)
                {
                    TerminateProcess(client_process, 0x000000ed);
                    break;
                }
            } while (FALSE);

            CloseHandle(client_process);
        }
    }
    return ret;
}

LRESULT WINAPI _release_web_browser_process(WEB_BROWSER_PROCESS *web_browser_proc)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        close_server_named_pipe(web_browser_proc);

        if (TRUE == web_browser_proc->ipc_info.pipe_inited)
        {
            uninit_pipe_point(&web_browser_proc->ipc_info.point);
        }

        web_browser_proc->ipc_info.pipe_inited = FALSE;
        web_browser_proc->context.current_url.clear();
        web_browser_proc->context.current_page.clear();

    } while (FALSE);

    return ret;
}

LRESULT WINAPI release_web_browser_process(WEB_BROWSER_PROCESS *web_browser_proc)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {

        ret = _release_web_browser_process(web_browser_proc);

        web_browser_proc->context.locate_url.clear();
    } while (FALSE);

    return ret;
}

LRESULT WINAPI allocate_web_browser_process(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc)
{
    LRESULT ret = ERROR_SUCCESS;
    WEB_BROWSER_PROCESS *_web_browser_proc;
    do
    {
        _web_browser_proc = new WEB_BROWSER_PROCESS();
        if (NULL == _web_browser_proc)
        {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        web_browser_processes.push_back(_web_browser_proc);
        web_browser_processe_map.insert(WEB_BROWSER_PROCESS_MAP_PAIR(container, _web_browser_proc));

        *web_browser_proc = _web_browser_proc;
    } while (FALSE);
    return ret;
}

LRESULT WINAPI get_web_browser_process(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc)
{
    LRESULT ret = ERROR_SUCCESS;
    WEB_BROWSER_PROCESS_MAP_ITERATOR it;

    do
    {
        ASSERT(NULL != container);
        ASSERT(NULL != web_browser_proc);
        ASSERT(0 == wcscmp(container->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER));

        it = web_browser_processe_map.find(container);
        if (it == web_browser_processe_map.end())
        {
            ret = ERROR_NOT_FOUND;
            break;
        }

        ASSERT(NULL != (*it).second);
        *web_browser_proc = (*it).second;
    } while (FALSE);

    return ret;
}

LRESULT WINAPI get_web_browser_process_ex(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc,
    ULONG flags)
{
    LRESULT ret = ERROR_SUCCESS;
    HTML_ELEMENT_ACTION *action;

    do
    {
        ASSERT(NULL != container);
        ASSERT(NULL != web_browser_proc);

        *web_browser_proc = NULL;

        if (GET_ROOT_ELEMENT_INFO == flags)
        {
            action = container;
        }
        else
        {
            action = container;
        }

        ret = get_web_browser_process(action, web_browser_proc);
        if (ret == ERROR_NOT_FOUND)
        {
            ret = allocate_web_browser_process(action, web_browser_proc);
            if (ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "create the web browser process error %x\n", ret);
            }
        }
    } while (FALSE);

    return ret;
}

LRESULT WINAPI _get_web_browser_process(HTML_ELEMENT_ACTION *action,
    WEB_BROWSER_PROCESS **web_browser_proc)
{
    LRESULT ret = ERROR_SUCCESS;

    do
    {
        ASSERT(NULL != action);
        ASSERT(NULL != web_browser_proc);

        *web_browser_proc = NULL;
        ret = get_web_browser_process(action, web_browser_proc);
        if (ret == ERROR_NOT_FOUND)
        {
            ret = allocate_web_browser_process(action, web_browser_proc);
            if (ret != ERROR_SUCCESS)
            {
                dbg_print(MSG_FATAL_ERROR, "create the web browser process error %x\n", ret);
            }
        }
    } while (FALSE);
    return ret;
}

LRESULT WINAPI disconnect_all_web_browser_processes()
{
    LRESULT ret = ERROR_SUCCESS;
    //LRESULT _ret;

    do
    {
        for (WEB_BROWSER_PROCESSES_ITERATOR it = web_browser_processes.begin();
            it != web_browser_processes.end();
            it++)
        {
            release_web_browser_process((*it));

            (*it)->ipc_info.pipe_inited = FALSE;
            (*it)->ipc_info.pipe_connected = FALSE;
        }
    } while (FALSE);

    return ret;
}

LRESULT WINAPI release_all_web_browser_processes()
{
    LRESULT ret = ERROR_SUCCESS;
    LRESULT _ret;

    do
    {
        web_browser_processe_map.clear();

        for (WEB_BROWSER_PROCESSES_ITERATOR it = web_browser_processes.begin();
            it != web_browser_processes.end();
            it++)
        {
            _ret = release_web_browser_process((*it));
            if (_ret != ERROR_SUCCESS)
            {
                ret = _ret;
            }

            delete (*it);
        }

        web_browser_processes.clear();
    } while (FALSE);

    return ret;
}

LRESULT WINAPI get_action_web_browser(HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *action, WEB_BROWSER_PROCESS **info)
{
    LRESULT ret = ERROR_SUCCESS;
    HTML_ELEMENT_ACTION *container;
    WEB_BROWSER_PROCESS *browser_info;

    do
    {
        ASSERT(NULL != root);
        ASSERT(NULL != action);
        ASSERT(NULL != info);

        *info = NULL;

        if (action == root)
        {
            ret = get_web_browser_process_ex(action, &browser_info, GET_ROOT_ELEMENT_INFO);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }
        }
        else
        {
            ASSERT(0 != wcscmp(action->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER)); 
            ret = get_parent_page_layout_node(action, &container);
            if (ret != ERROR_SUCCESS)
            {
                ASSERT(FALSE); 
                ret = get_web_browser_process_ex(root, &browser_info, GET_ROOT_ELEMENT_INFO);
                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                *info = browser_info;
                break;
            }

            ret = get_web_browser_process_ex(container, &browser_info, 0);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }
        }
        *info = browser_info;
    } while (FALSE);

    return ret;
}

LRESULT WINAPI get_page_layout_web_browser(HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *page_layout, WEB_BROWSER_PROCESS **info)
{
    LRESULT ret = ERROR_SUCCESS;
    WEB_BROWSER_PROCESS *browser_info;

    do
    {
        ASSERT(NULL != root);
        ASSERT(NULL != page_layout);
        ASSERT(NULL != info);

        *info = NULL;

        ASSERT(0 == wcscmp(page_layout->param.c_str(), HTML_ELEMENT_ACTION_NONE_PARAM_CONTAINER));

        if (page_layout == root)
        {
            ret = get_web_browser_process_ex(page_layout, &browser_info, GET_ROOT_ELEMENT_INFO);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }
        }
        else
        {
            ret = get_web_browser_process_ex(page_layout, &browser_info, 0);
            if (ret != ERROR_SUCCESS)
            {
                break;
            }
        }
        *info = browser_info;
    } while (FALSE);

    return ret;
}

LRESULT WINAPI start_web_browser(HTML_ELEMENT_ACTION *root,
    HTML_ELEMENT_ACTION *action,
    WEB_BROWSER_PROCESS *browser_info)
{
    LRESULT ret = ERROR_SUCCESS;
    HRESULT hr;
    ULONG web_browser_proc_id;
    //HTML_ELEMENT_ACTION *container;

    do
    {
        ASSERT(NULL != action);
        ASSERT(NULL != browser_info);
        ASSERT(0 != browser_info->context.locate_url.length());

        browser_info->context.current_url = L"";
        browser_info->context.current_page = L"";

        ret = _start_webbrowser(0, &web_browser_proc_id);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ASSERT(0 != web_browser_proc_id);
        hr = StringCchPrintfW(browser_info->ipc_info.pipe_name, ARRAYSIZE(browser_info->ipc_info.pipe_name), DATA_SCRAMBLE_PIPE_POINT_NAME, web_browser_proc_id);
        if (FAILED(hr))
        {

        }

        ret = init_pipe_point(&browser_info->ipc_info.point, browser_info->ipc_info.pipe_name);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        browser_info->ipc_info.pipe_inited = TRUE;

        ret = create_name_pipe(browser_info->ipc_info.point.pipe_name, &browser_info->ipc_info.point.pipe);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        browser_info->ipc_info.pipe_created = TRUE;
        browser_info->ipc_info.pipe_connected = FALSE;

        ret = accept_name_pipe_client_sync(browser_info->ipc_info.point.pipe);
        if (ret != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            break;
        }

        browser_info->ipc_info.pipe_connected = TRUE;
    } while (FALSE);

    return ret;
}

LRESULT WINAPI _start_web_browser(HTML_ELEMENT_ACTION *root,
    HTML_ELEMENT_ACTION *action,
    WEB_BROWSER_PROCESS *browser_info)
{
    LRESULT ret = ERROR_SUCCESS;
    HRESULT hr;
    ULONG web_browser_proc_id;
    //HTML_ELEMENT_ACTION *container;

    do
    {
        ASSERT(NULL != action);
        ASSERT(NULL != browser_info);
        ASSERT(0 != browser_info->context.locate_url.length());

        browser_info->context.current_url = L"";
        browser_info->context.current_page = L"";

        ret = _start_webbrowser(WEB_RROWSER_LOAD_PAGE_NO_DELAY, &web_browser_proc_id);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        ASSERT(0 != web_browser_proc_id);
        hr = StringCchPrintfW(browser_info->ipc_info.pipe_name, ARRAYSIZE(browser_info->ipc_info.pipe_name), DATA_SCRAMBLE_PIPE_POINT_NAME, web_browser_proc_id);
        if (FAILED(hr))
        {

        }

        ret = init_pipe_point(&browser_info->ipc_info.point, browser_info->ipc_info.pipe_name);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        browser_info->ipc_info.pipe_inited = TRUE;

        ret = create_name_pipe(browser_info->ipc_info.point.pipe_name, &browser_info->ipc_info.point.pipe);
        if (ret != ERROR_SUCCESS)
        {
            break;
        }

        browser_info->ipc_info.pipe_created = TRUE;
        browser_info->ipc_info.pipe_connected = FALSE;

        ret = accept_name_pipe_client_sync(browser_info->ipc_info.point.pipe);
        if (ret != ERROR_SUCCESS)
        {
            ASSERT(FALSE);
            break;
        }

        browser_info->ipc_info.pipe_connected = TRUE;
    } while (FALSE);

    return ret;
}
