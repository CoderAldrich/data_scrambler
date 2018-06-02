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

#ifndef __WEB_PAGE_BROWSER_MANAGE_H__
#define __WEB_PAGE_BROWSER_MANAGE_H__

#include <vector>
#include <map>

using namespace std; 

typedef struct _IPC_INFO
{
    pipe_ipc_point point;
    BOOLEAN pipe_inited;
    BOOLEAN pipe_connected;
    BOOLEAN pipe_created;
    WCHAR pipe_name[MAX_PATH];
} IPC_INFO, *PIPC_INFO;

typedef struct _WEB_BROWSER_PROCESS
{
    _WEB_BROWSER_PROCESS() { memset(&ipc_info, 0, sizeof(ipc_info)); }

    IPC_INFO ipc_info;
    HTML_ACTION_CONTEXT context;
} WEB_BROWSER_PROCESS, *PWEB_BROWSER_PROCESS;
;
LRESULT WINAPI close_server_named_pipe(WEB_BROWSER_PROCESS *web_browser_info);

typedef vector< WEB_BROWSER_PROCESS*> WEB_BROWSER_PROCESSES;
typedef WEB_BROWSER_PROCESSES::iterator WEB_BROWSER_PROCESSES_ITERATOR;
typedef map< HTML_ELEMENT_ACTION*, WEB_BROWSER_PROCESS* > WEB_BROWSER_PROCESS_MAP;
typedef WEB_BROWSER_PROCESS_MAP::iterator WEB_BROWSER_PROCESS_MAP_ITERATOR;
typedef pair< HTML_ELEMENT_ACTION*, WEB_BROWSER_PROCESS* > WEB_BROWSER_PROCESS_MAP_PAIR;


LRESULT WINAPI start_web_browser(HTML_ELEMENT_ACTION *root,
    HTML_ELEMENT_ACTION *action,
    WEB_BROWSER_PROCESS *info);

LRESULT WINAPI _start_web_browser(HTML_ELEMENT_ACTION *root,
    HTML_ELEMENT_ACTION *action,
    WEB_BROWSER_PROCESS *browser_info);

LRESULT WINAPI get_web_browser_process_ex(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc,
    ULONG flags);
LRESULT WINAPI release_all_web_browser_processes();
LRESULT WINAPI get_web_browser_process(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc);
LRESULT WINAPI _get_web_browser_process(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc);
LRESULT WINAPI allocate_web_browser_process(HTML_ELEMENT_ACTION *container,
    WEB_BROWSER_PROCESS **web_browser_proc);
LRESULT WINAPI release_web_browser_process(WEB_BROWSER_PROCESS *web_browser_proc);
LRESULT WINAPI _release_web_browser_process(WEB_BROWSER_PROCESS *web_browser_proc);
LRESULT WINAPI get_action_web_browser(HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *action, WEB_BROWSER_PROCESS **info);; 
LRESULT WINAPI get_page_layout_web_browser(HTML_ELEMENT_ACTION *root, HTML_ELEMENT_ACTION *page_layout, WEB_BROWSER_PROCESS **info); 

#endif //__WEB_PAGE_BROWSER_MANAGE_H__