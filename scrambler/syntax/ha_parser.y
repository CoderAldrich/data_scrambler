/*
 * Name:ha_parser.y
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
#include<unistd.h>
#include<stdio.h>   
#include "ha_parser_imp.h"
extern int yylex();

#include <string>
#include <map>
#include <vector>
using namespace std; 

%}

%union{
	struct _TOKEN_INFO *token; 
	struct _STATEMENT_INFO *statement; 
}
/*declare tokens*/
%token program extdeflist extdef extdeclist specification structspecification 
opttag  tag vardec  fundec varlist paramdec compst stmtlist stmt deflist def declist decl exp args
%token <token> INTEGER_TYPE FLOAT_TYPE
%token <token> TYPE STRUCT RETURN IF ELSE WHILE ID SPACE SEMI COMMA ASSIGNOP PLUSASSNOP MINUSASSNOP PLUS
MINUS STAR DIV AND OR NOT BITAND BITOR GREATEROP LOWEROP GREATEQUALOP LOWEREUQALOP EUALOP NEUQALOP LPB 
RPB LB RB LC RC AERROR

%token <token> EOL
%type  <statement> Program Exp 

/*ExtDefList ExtDef ExtDecList Specification StructSpecification 
OptTag VarDec FunDec VarList ParamDec Compst StmtList Stmt DefList Def DecList Dec Args*/

/*|ID ASSIGNOP Exp SEMI {$$=handle_grammer_element(def,2,dec,$1,SEMI,$2);}*/
/*priority*/
%right ASSIGNOP PLUSASSNOP MINUSASSNOP
%left OR
%left AND
%left EUALOP NEUQALOP GREATEROP LOWEROP GREATEQUALOP LOWEREUQALOP 
%left BITAND BITOR 
%left PLUS MINUS
%left STAR DIV
%right NOT 
%left LPB RPB LB RB DOT
%%

Program:TYPE Exp SEMI {$$=handle_grammer_element( program,3,TYPE,$1,exp,$2,SEMI,$3);}
    |Exp SEMI {$$=handle_grammer_element( program,2,exp,$1,SEMI,$2);}
    |IF LPB Exp RPB {$$=handle_grammer_element( program,4,IF,$1,LPB,$2,exp,$3,RPB,$4);}
    ;
/*
ExtDefList:ExtDef {$$=handle_grammer_element(extdeflist,1,extdef,$1);}
	|ExtDef ExtDefList {$$=handle_grammer_element(extdeflist,2,extdef,$1,extdeflist,$2);}
	|{$$=handle_grammer_element(extdeflist,0,-1,-1);}
	;
ExtDef:Specification ExtDecList SEMI {$$=handle_grammer_element(extdef,3,specification,$1,extdeclist,$2,SEMI,$3);}    
	|Specification SEMI	{$$=handle_grammer_element(extdef,2,specification,$1,SEMI,$2);}
	|Specification FunDec Compst {$$=handle_grammer_element(extdef,3,specification,$1,fundec,$2,compst,$3);}
	;
ExtDecList:Dec {$$=handle_grammer_element(extdeclist,1,decl,$1);}
	|Dec COMMA ExtDecList {$$=handle_grammer_element(extdeclist,3,decl,$1,COMMA,$2,extdeclist,$3);}
	;
Specification
Specification:TYPE {$$=handle_grammer_element(specification,1,TYPE,$1);}
	|StructSpecification {$$=handle_grammer_element(specification,1,structspecification,$1);}
	;
StructSpecification:STRUCT OptTag LC DefList RC {$$=handle_grammer_element(structspecification,5,STRUCT,$1,opttag,$2,LC,$3,deflist,$4,RC,$5);}
	|STRUCT ID {$$=handle_grammer_element(structspecification,2,STRUCT,$1,ID,$2);}
	;
OptTag:ID {$$=handle_grammer_element(opttag,1,ID,$1);}
	|{$$=handle_grammer_element(opttag,0,-1,-1);}
	;
Declarators(variable,array)
VarDec:ID {$$=handle_grammer_element(vardec,1,ID,$1);}
	|VarDec LB INTEGER_TYPE RB {$$=handle_grammer_element(vardec,4,vardec,$1,LB,$2,INTEGER_TYPE,$3,RB,$4);}
	;
FunDec:ID LPB VarList RPB {$$=handle_grammer_element(fundec,4,ID,$1,LPB,$2,varlist,$3,RPB,$4);}
	|ID LPB RPB {$$=handle_grammer_element(fundec,3,ID,$1,LPB,$2,RPB,$3);}
	;
VarList:ParamDec COMMA VarList {$$=handle_grammer_element(varlist,3,paramdec,$1,COMMA,$2,varlist,$3);}
	|ParamDec {$$=handle_grammer_element(varlist,1,paramdec,$1);}
	;
ParamDec:Specification VarDec {$$=handle_grammer_element(paramdec,2,specification,$1,vardec,$2);}
    ;
Statement
Compst:LC DefList StmtList RC {$$=handle_grammer_element(compst,4,LC,$1,deflist,$2,stmtlist,$3,RC,$4);}
	;
StmtList:Stmt StmtList{$$=handle_grammer_element(stmtlist,2,stmt,$1,stmtlist,$2);}
	|{$$=handle_grammer_element(stmtlist,0,-1,-1);}
	;
Stmt:Exp SEMI {$$=handle_grammer_element(stmt,2,exp,$1,SEMI,$2);}
	|Compst {$$=handle_grammer_element(stmt,1,compst,$1);}
	|RETURN Exp SEMI {$$=handle_grammer_element(stmt,3,RETURN,$1,exp,$2,SEMI,$3);}
	|IF LPB Exp RPB Stmt {$$=handle_grammer_element(stmt,5,IF,$1,LPB,$2,exp,$3,RPB,$4,stmt,$5);}
	|IF LPB Exp RPB Stmt ELSE Stmt {$$=handle_grammer_element(stmt,7,IF,$1,LPB,$2,exp,$3,RPB,$4,stmt,$5,ELSE,$6,stmt,$7);}
	|WHILE LPB Exp RPB Stmt {$$=handle_grammer_element(stmt,5,WHILE,$1,LPB,$2,exp,$3,RPB,$4,stmt,$5);}
	;
Local Definitions
DefList:Def DefList{$$=handle_grammer_element(deflist,2,def,$1,deflist,$2);}
	|{$$=handle_grammer_element(deflist,0,-1,-1);}
	;
Def:Specification DecList SEMI {$$=handle_grammer_element(def,3,specification,$1,declist,$2,SEMI,$3);}
	;
DecList:Dec {$$=handle_grammer_element(declist,1,decl,$1);}
	|Dec COMMA DecList {$$=handle_grammer_element(declist,3,decl,$1,COMMA,$2,declist,$3);}
	;
Dec:VarDec {$$=handle_grammer_element(decl,1,vardec,$1);}
	|VarDec ASSIGNOP Exp {$$=handle_grammer_element(decl,3,vardec,$1,ASSIGNOP,$2,exp,$3);}
	;
Args:Exp COMMA Args {$$=handle_grammer_element(args,3,exp,$1,COMMA,$2,args,$3);}
        |Exp {$$=handle_grammer_element(args,1,exp,$1);}
        ;

|ID LPB RPB {$$=handle_grammer_element(exp,3,ID,$1,LPB,$2,RPB,$3);}
|ID LPB Args RPB {$$=handle_grammer_element(exp,4,ID,$1,LPB,$2,args,$3,RPB,$4);}
*/

/*Expressions*/
Exp:ID ASSIGNOP Exp{$$=handle_grammer_element(exp,3,ID,$1,ASSIGNOP,$2,exp,$3);}
		|ID PLUSASSNOP Exp{$$=handle_grammer_element(exp,3,ID,$1,PLUSASSNOP,$2,exp,$3);}
		|ID MINUSASSNOP Exp{$$=handle_grammer_element(exp,3,ID,$1,MINUSASSNOP,$2,exp,$3);}
        |Exp AND Exp{$$=handle_grammer_element(exp,3,exp,$1,AND,$2,exp,$3);}
        |Exp OR Exp{$$=handle_grammer_element(exp,3,exp,$1,OR,$2,exp,$3);}
        |Exp GREATEROP Exp{$$=handle_grammer_element(exp,3,exp,$1,GREATEROP,$2,exp,$3);}
        |Exp LOWEROP Exp{$$=handle_grammer_element(exp,3,exp,$1,LOWEROP,$2,exp,$3);}
        |Exp GREATEQUALOP Exp{$$=handle_grammer_element(exp,3,exp,$1,GREATEQUALOP,$2,exp,$3);}
        |Exp LOWEREUQALOP Exp{$$=handle_grammer_element(exp,3,exp,$1,LOWEREUQALOP,$2,exp,$3);}
        |Exp EUALOP Exp{$$=handle_grammer_element(exp,3,exp,$1,EUALOP,$2,exp,$3);}
        |Exp NEUQALOP Exp{$$=handle_grammer_element(exp,3,exp,$1,NEUQALOP,$2,exp,$3);}
        |Exp BITAND Exp{$$=handle_grammer_element(exp,3,exp,$1,BITAND,$2,exp,$3);}
        |Exp BITOR Exp{$$=handle_grammer_element(exp,3,exp,$1,BITOR,$2,exp,$3);}
        |Exp PLUS Exp{$$=handle_grammer_element(exp,3,exp,$1,PLUS,$2,exp,$3);}
        |Exp MINUS Exp{$$=handle_grammer_element(exp,3,exp,$1,MINUS,$2,exp,$3);}
        |Exp STAR Exp{$$=handle_grammer_element(exp,3,exp,$1,STAR,$2,exp,$3);}
        |Exp DIV Exp{$$=handle_grammer_element(exp,3,exp,$1,DIV,$2,exp,$3);}
        |MINUS Exp {$$=handle_grammer_element(exp,2,MINUS,$1,exp,$2);}
        |NOT Exp {$$=handle_grammer_element(exp,2,NOT,$1,exp,$2);}
        |Exp LB Exp RB {$$=handle_grammer_element(exp,4,exp,$1,LB,$2,exp,$3,RB,$4);}
        |INTEGER_TYPE {$$=handle_grammer_element(exp,1,INTEGER_TYPE,$1);}
        |FLOAT_TYPE{$$=handle_grammer_element(exp,1,FLOAT_TYPE,$1);}
        |ID {$$=handle_grammer_element(exp,1,ID,$1);}
        ;
%%