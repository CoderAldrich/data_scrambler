/*
 * Name:ha_parser.l
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

%{
#include "stdio.h"
#include "stdlib.h"
#include "ha_parser_imp.h"
#include "ha_parser.tab.h"

extern int eval_getinput(char *buf, int maxlen);
#undef YY_INPUT
#define YY_INPUT(buf, retval, maxlen)        (retval = eval_getinput(buf, maxlen))

%}

%option yylineno
TYPE int|float
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
PLUS \+
MINUS -
INTEGER_TYPE [1-9]+[0-9]*|0 
FLOAT_TYPE   [0-9]+\.[0-9]*  
ID [a-z_A-Z][a-zA-Z_0-9]*
SPACE [ \t\r]*
EOL \n
SEMI ;
COMMA ,
ASSIGNOP =
PLUSASSNOP \+=
MINUSASSNOP -=
GREATEROP >
LOWEROP <
GREATEQUALOP >=
LOWEREUQALOP <=
EUALOP ==
NEUQALOP !=
STAR \*
DIV \/
AND &&
OR \|\|
BITAND &
BITOR \|
NOT !
LPB \(
RPB \)
LB \[
RB \]
LC \{
RC \}
AERROR .
%%
{TYPE} {yylval.token=handle_lex_element(TYPE,0,yylineno);return TYPE;}
{STRUCT} {yylval.token=handle_lex_element(STRUCT,0,yylineno);return STRUCT;}
{RETURN} {yylval.token=handle_lex_element(RETURN,0,yylineno); return RETURN;}
{IF} { yylval.token=handle_lex_element(IF,0,yylineno);return IF;}
{ELSE} {yylval.token=handle_lex_element(ELSE,0,yylineno); return ELSE;}
{WHILE} {yylval.token=handle_lex_element(WHILE,0,yylineno); return WHILE;}
{PLUS} {yylval.token=handle_lex_element(PLUS,0,yylineno); return PLUS;}
{MINUS} {yylval.token=handle_lex_element(MINUS,0,yylineno); return MINUS;}
{INTEGER_TYPE} {yylval.token=handle_lex_element(INTEGER_TYPE,0,yylineno); return INTEGER_TYPE;}
{ID} {yylval.token=handle_lex_element(ID,0,yylineno); return ID;}
{SPACE} {}
{EOL} {}
{SEMI} {yylval.token=handle_lex_element(SEMI,0,yylineno); return SEMI;}
{COMMA} {yylval.token=handle_lex_element(COMMA,0,yylineno); return COMMA;}
{ASSIGNOP} {yylval.token=handle_lex_element(ASSIGNOP,0,yylineno); return ASSIGNOP;}
{PLUSASSNOP} {yylval.token=handle_lex_element(PLUSASSNOP,0,yylineno); return PLUSASSNOP;}
{MINUSASSNOP} {yylval.token=handle_lex_element(MINUSASSNOP,0,yylineno); return MINUSASSNOP;}
{GREATEROP} {yylval.token=handle_lex_element(GREATEROP,0,yylineno); return GREATEROP;}
{LOWEROP} {yylval.token=handle_lex_element(LOWEROP,0,yylineno); return LOWEROP;}
{GREATEQUALOP} {yylval.token=handle_lex_element(GREATEQUALOP,0,yylineno); return GREATEQUALOP;}
{LOWEREUQALOP} {yylval.token=handle_lex_element(LOWEREUQALOP,0,yylineno); return LOWEREUQALOP;}
{EUALOP} {yylval.token=handle_lex_element(EUALOP,0,yylineno); return EUALOP;}
{NEUQALOP} {yylval.token=handle_lex_element(NEUQALOP,0,yylineno); return NEUQALOP;}
{STAR} {yylval.token=handle_lex_element(STAR,0,yylineno); return STAR;}
{DIV} {yylval.token=handle_lex_element(DIV,0,yylineno); return DIV;}
{AND} {yylval.token=handle_lex_element(AND,0,yylineno); return AND;}
{OR} {yylval.token=handle_lex_element(OR,0,yylineno); return OR;}
{BITAND} {yylval.token=handle_lex_element(BITAND,0,yylineno); return BITAND;}
{BITOR} {yylval.token=handle_lex_element(BITOR,0,yylineno); return BITOR;}
{NOT} {yylval.token=handle_lex_element(NOT,0,yylineno); return NOT;}
{LPB} {yylval.token=handle_lex_element(LPB,0,yylineno); return LPB;}
{RPB} {yylval.token=handle_lex_element(RPB,0,yylineno); return RPB;}
{LB} {yylval.token=handle_lex_element(LB,0,yylineno); return LB;}
{RB} {yylval.token=handle_lex_element(RB,0,yylineno); return RB;}
{LC} {yylval.token=handle_lex_element(LC,0,yylineno); return LC;}
{RC} {yylval.token=handle_lex_element(RC,0,yylineno); return RC;}
{AERROR} { }

%%
//int yywrap()
//{
//        return 1;
//}
//{DOT} {yylval.token=handle_lex_element(DOT,0,yylineno); return DOT;}
//DOT \.