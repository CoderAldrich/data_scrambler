
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     program = 258,
     extdeflist = 259,
     extdef = 260,
     extdeclist = 261,
     specification = 262,
     structspecification = 263,
     opttag = 264,
     tag = 265,
     vardec = 266,
     fundec = 267,
     varlist = 268,
     paramdec = 269,
     compst = 270,
     stmtlist = 271,
     stmt = 272,
     deflist = 273,
     def = 274,
     declist = 275,
     decl = 276,
     exp = 277,
     args = 278,
     INTEGER_TYPE = 279,
     FLOAT_TYPE = 280,
     TYPE = 281,
     STRUCT = 282,
     RETURN = 283,
     IF = 284,
     ELSE = 285,
     WHILE = 286,
     ID = 287,
     SPACE = 288,
     SEMI = 289,
     COMMA = 290,
     ASSIGNOP = 291,
     PLUSASSNOP = 292,
     MINUSASSNOP = 293,
     PLUS = 294,
     MINUS = 295,
     STAR = 296,
     DIV = 297,
     AND = 298,
     OR = 299,
     NOT = 300,
     BITAND = 301,
     BITOR = 302,
     GREATEROP = 303,
     LOWEROP = 304,
     GREATEQUALOP = 305,
     LOWEREUQALOP = 306,
     EUALOP = 307,
     NEUQALOP = 308,
     LPB = 309,
     RPB = 310,
     LB = 311,
     RB = 312,
     LC = 313,
     RC = 314,
     AERROR = 315,
     EOL = 316,
     DOT = 317
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 38 "h:\\publish\\data_scrambler\\scrambler\\\\syntax\\ha_parser.y"

	struct _TOKEN_INFO *token; 
	struct _STATEMENT_INFO *statement; 



/* Line 1676 of yacc.c  */
#line 121 "h:\\publish\\data_scrambler\\scrambler\\\\syntax\\ha_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


