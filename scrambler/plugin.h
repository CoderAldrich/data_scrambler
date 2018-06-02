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

#ifndef __DATA_PROCESS_PLUGIN_H__
#define __DATA_PROCESS_PLUGIN_H__

#include <vector>
using namespace std; 

#include "plugin_api.h"

typedef struct _DATA_PROCESS_PLUGIN
{
	wstring path; 
	HMODULE module; 
	collected_data_process_func data_handler; 
} DATA_PROCESS_PLUGIN, *PDATA_PROCESS_PLUGIN; 

typedef vector<DATA_PROCESS_PLUGIN*> DATA_PROCESS_PLUGINS; 

extern DATA_PROCESS_PLUGINS modules_info; 

typedef struct _DATA_PROCESS_PLUGIN_INFO
{
	wstring path; 
} DATA_PROCESS_PLUGIN_INFO, *PDATA_PROCESS_PLUGIN_INFO; 

#define DATA_SCRAMBLE_PLUGINS_CONFIG_FILE L"plugins.xml"
#define DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_PATH L"//PLUGINS"
#define DATA_SCRAMBLE_PLUGIN_ELEMENT_NAME L"PLUGIN"
#define DATA_SCRAMBLE_PLUGINS_ROOT_ELEMENT_NAME L"PLUGINS"
#define DATA_SCRAMBLE_PLUGIN_ELEMENT_PATH L"//PLUGINS/PLUGIN"
#define DATA_SCRAMBLE_PLUGIN_ATTRIBUTE_PATH L"path"

LRESULT WINAPI init_plugin( DATA_PROCESS_PLUGIN *info ); 
LRESULT WINAPI get_plugins( DATA_PROCESS_PLUGINS ** modules ); 
LRESULT WINAPI uninit_plugin( DATA_PROCESS_PLUGIN *info ); 
LRESULT WINAPI unload_all_plugins(); 
LRESULT WINAPI load_plugin( DATA_PROCESS_PLUGIN *info); 
//LRESULT WINAPI get_module_info( MSXML::IXMLDOMNode *xml_node, DATA_PROCESS_PLUGIN_INFO *info ); 
LRESULT WINAPI load_all_plugins(); 
LRESULT WINAPI call_plugin_data_handler( DATA_INFO datas[], ULONG count ); 
LRESULT WINAPI add_plugin_info(DATA_PROCESS_PLUGIN_INFO* info); 
LRESULT WINAPI del_plugin_info(DATA_PROCESS_PLUGIN_INFO* info); 

#endif //__DATA_PROCESS_PLUGIN_H__