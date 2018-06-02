/*
 * 
 * Copyright (c) 2003, Michael Carruth All rights reserved.
 * This file is a part of CrashRpt library.
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

#ifndef __HA_SCRIPT_PARSE_H__
#define __HA_SCRIPT_PARSE_H__

#include "ha_script_common.h"

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

LRESULT WINAPI get_script_variable( LPCSTR name, VARIABLE_INFO *variable ); 
LRESULT WINAPI parse_ha_script( LPCSTR script ); 
LRESULT WINAPI uninit_ha_script(); 
LRESULT WINAPI init_ha_script(); 
LRESULT WINAPI clean_ha_variables(); 

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__HA_SCRIPT_PARSE_H__