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
#define XPATH_CREATE
#include "html_xpath.h"
#include "scrapy_support.h"

LRESULT WINAPI action_to_scrapy(LPCWSTR xpath,
    wstring &script_text)
{
    LRESULT ret = ERROR_SUCCESS; 
    //wstring _text; 

    do 
    {
        if (NULL == xpath 
            || *xpath == L'\0' )
        {
            ret = ERROR_INVALID_PARAMETER; 
            break; 
        }

        script_text = L" = response.xpath(\'";
        
        script_text += xpath;
        script_text += L"\')"; 

    } while ( FALSE );

    return ret; 
}
