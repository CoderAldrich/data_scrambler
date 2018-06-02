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
#ifndef __INPUT_CONTENT_H__
#define __INPUT_CONTENT_H__

#define INPUT_TEXT_CONTENT_ELEMENT_NAME L"INPUT_TEXT"
#define INPUT_TEXT_BLOCK_ATTRIBUTE_NAME L"text_block"
#define INPUT_TEXT_NAME_ATTRIBUTE_NAME L"text_name"

LRESULT WINAPI load_input_content( LPCWSTR file_name, XPATH_PARAMS &contents ); 
LRESULT WINAPI add_input_text_block_info( LPCWSTR file_name, LPCWSTR text, LPCWSTR text_name ); 
LRESULT WINAPI del_input_text_block_info( LPCWSTR file_name, LPCWSTR text, LPCWSTR text_name ); 

LRESULT WINAPI make_content_file_exist( LPCWSTR config_file_path, ULONG cc_path_len ); 
LRESULT WINAPI get_input_context_file_path( wstring &file_name ); 

#endif //__INPUT_CONTENT_H__