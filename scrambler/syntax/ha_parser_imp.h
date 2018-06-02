#ifndef __HAS_PARSER_IMP_H__
#define __HAS_PARSER_IMP_H__

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

#include "ha_script_common.h"

extern int yylineno;
extern char* yytext;
void yyerror(char *s,...);
//void yydestruct(const char *yymsg, int yytype, YYSTYPE *yyvaluep); 

LRESULT WINAPI print_grammer_tree(GRAMMER_ELEMENT_HEADER *header, 
								  INT level); 

STATEMENT_INFO *handle_grammer_element(INT32 nonteminal, INT32 num,...); 
TOKEN_INFO *handle_lex_element( INT32 token_type,
							   INT32 param_num, ...); 

LRESULT WINAPI get_script_output( HA_SCRIPT **script ); 
LRESULT WINAPI init_parse_text( LPCSTR text, ULONG cc_text_len, ULONG flags ); 

#endif //__HAS_PARSER_IMP_H__