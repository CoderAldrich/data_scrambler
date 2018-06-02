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

# include<stdio.h>
# include<stdlib.h>
# include<stdarg.h>
#include "ha_parser.tab.h"
# include"ha_parser_imp.h"
#include <string.h>
#include "common_func.h"

#pragma comment(lib, "libfl.a") 

#define VT_FLOAT 10001

HA_SCRIPT ha_script_context; 
LRESULT WINAPI free_grammer_element( GRAMMER_ELEMENT_HEADER *header, 
									ULONG token_type ); 

INLINE LPCSTR get_token_desc( INT32 token )
{
	LPCSTR desc = ""; 
	switch( token )
	{
	case program: 
		desc = "program"; 
		break; 
	case extdeflist: 
		desc = "extdeflist"; 
		break; 
	case extdef: 
		desc = "extdef"; 
		break; 
	case extdeclist: 
		desc = "extdeclist"; 
		break; 
	case specification: 
		desc = "specification"; 
		break; 
	case structspecification: 
		desc = "structspecification"; 
		break; 
	case opttag: 
		desc = "opttag"; 
		break; 
	case tag: 
		desc = "tag"; 
		break; 
	case vardec: 
		desc = "vardec"; 
		break; 
	case fundec: 
		desc = "fundec"; 
		break; 
	case varlist: 
		desc = "varlist"; 
		break; 
	case paramdec: 
		desc = "paramdec"; 
		break; 
	case compst: 
		desc = "compst"; 
		break; 
	case stmtlist: 
		desc = "stmtlist"; 
		break; 
	case stmt: 
		desc = "stmt"; 
		break; 
	case deflist: 
		desc = "deflist"; 
		break; 
	case def: 
		desc = "def"; 
		break; 
	case declist: 
		desc = "declist"; 
		break; 
	case decl: 
		desc = "decl"; 
		break; 
	case exp: 
		desc = "exp"; 
		break; 
	case args: 
		desc = "args"; 
		break; 
	case INTEGER_TYPE: 
		desc = "INTEGER_TYPE"; 
		break; 
	case FLOAT_TYPE: 
		desc = "FLOAT_TYPE"; 
		break; 
	case TYPE: 
		desc = "TYPE"; 
		break; 
	case STRUCT: 
		desc = "STRUCT"; 
		break; 
	case RETURN: 
		desc = "RETURN"; 
		break; 
	case IF: 
		desc = "IF"; 
		break; 
	case ELSE: 
		desc = "ELSE"; 
		break; 
	case WHILE: 
		desc = "WHILE"; 
		break; 
	case ID: 
		desc = "ID"; 
		break; 
	case SPACE: 
		desc = "SPACE"; 
		break; 
	case SEMI: 
		desc = "SEMI"; 
		break; 
	case COMMA: 
		desc = "COMMA"; 
		break; 
	case ASSIGNOP: 
		desc = "ASSIGNOP"; 
		break; 
	case PLUSASSNOP:
		desc = "PLUSASSNOP"; 
		break; 
	case MINUSASSNOP:
		desc = "MINUSASSNOP"; 
		break; 
	case PLUS: 
		desc = "PLUS"; 
		break; 
	case MINUS: 
		desc = "MINUS"; 
		break; 
	case STAR: 
		desc = "STAR"; 
		break; 
	case DIV: 
		desc = "DIV"; 
		break; 
	case AND: 
		desc = "AND"; 
		break; 
	case OR: 
		desc = "OR"; 
		break; 
	case BITAND:
		desc = "BITAND"; 
		break; 
	case BITOR:
		desc = "BITOR"; 
		break; 
	case GREATEROP:
		desc = "GREATEROP"; 
		break; 
	case LOWEROP:
		desc = "LOWEROP"; 
		break; 
	case GREATEQUALOP: 
		desc = "GREATEQUALOP"; 
		break; 
	case LOWEREUQALOP:
		desc = "LOWEREUQALOP"; 
		break; 
	case EUALOP:
		desc = "EUALOP"; 
		break; 
	case NEUQALOP:
		desc = "NEUQALOP"; 
		break; 
	case DOT: 
		desc = "DOT"; 
		break; 
	case NOT: 
		desc = "NOT"; 
		break; 
	case LPB: 
		desc = "LPB"; 
		break; 
	case RPB: 
		desc = "RPB"; 
		break; 
	case LB: 
		desc = "LB"; 
		break; 
	case RB: 
		desc = "RB"; 
		break; 
	case LC: 
		desc = "LC"; 
		break; 
	case RC: 
		desc = "RC"; 
		break; 
	case AERROR: 
		desc = "AERROR"; 
		break; 
	case EOL: 
		desc = "EOL"; 
		break; 
	default:
		break; 
	} 

	return desc; 
}
INLINE LRESULT _init_ha_script( HA_SCRIPT *script )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	ASSERT( NULL != script ); 
	do 
	{
		for( i = 0; i < ARRAYSIZE( script->operands ); i ++ )
		{
			script->operands[ i ] = NULL; 
		}

		for( i = 0; i < ARRAYSIZE( script->operators ); i ++ )
		{
			script->operators[ i ] = 0; 
		}

		for( i = 0; i < ARRAYSIZE( script->temp_operands ); i ++ )
		{
			script->temp_operands[ i ] = NULL; 
		}

		ASSERT( script->variables.size() == 0 ); 
		ASSERT( script->elements.size() == 0 ); 

		script->error_message.clear(); 
		script->error_code = 0; 

		//script->condition_op[ 0 ] = 0; 
		//script->condition_output = FALSE; 

		ret = init_mutex( &script->lock ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

INLINE LRESULT reset_operand( VARIABLE_INFO *operand ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( NULL != operand ); 

	operand->name.clear(); 
	operand->variable.vt = VARIABLE_NULL; 

	return ret; 
}

LRESULT WINAPI _free_grammer_element( GRAMMER_ELEMENT_HEADER *header, 
									 ULONG token_type ); 

INLINE LRESULT reset_ha_script( HA_SCRIPT *script )
{
	INT32 i; 
	vector< GRAMMER_ELEMENT_HEADER*>::iterator it; 

	ASSERT( NULL != script ); 

	lock_mutex( script->lock ); 

	for( it = script->elements.begin(); 
		it != script->elements.end(); 
		it ++ )
	{
		 _free_grammer_element( ( *it ), PARAMETER_TYPE_UNKNOWN ); 
	}

	script->elements.clear(); 
	unlock_mutex( script->lock ); 

	for( i = 0; i < ARRAYSIZE( script->operands ); i ++ )
	{
		if( script->operands[ i ] != NULL )
		{
			script->operands[ i ] = NULL; 
		}
	}

	for( i = 0; i < ARRAYSIZE( script->operators ); i ++ )
	{
		if( 0 != script->operators[ i ] )
		{
			script->operators[ i ] = 0; 
		}
	}

	for( i = 0; i < ARRAYSIZE( script->temp_operands ); i ++ )
	{
		if( script->temp_operands[ i ] != NULL )
		{
			reset_operand( script->temp_operands[ i ] ); 
		}
	}

	//script->condition_op[ 0 ] = 0; 
	//script->condition_output = FALSE; 

	return ERROR_SUCCESS; 
}

INLINE LRESULT _clean_ha_variables( HA_SCRIPT *script )
{
	VARIABLE_INFOS::iterator it; 

	ASSERT( NULL != script ); 

	for( it = script->variables.begin(); 
		it != script->variables.end(); 
		it ++ )
	{
		delete ( *it ).second; 
		( *it ).second = NULL; 
	}

	script->variables.clear(); 

	//unlock_mutex( script->lock ); 

	return ERROR_SUCCESS; 
}

INLINE LRESULT _uninit_ha_script( HA_SCRIPT *script )
{
	INT32 i; 
	ASSERT( NULL != script ); 

	reset_ha_script( script ); 

	_clean_ha_variables( script ); 

	for( i = 0; i < ARRAYSIZE( script->temp_operands ); i ++ )
	{
		if( script->temp_operands[ i ] != NULL )
		{
			//delete ( ( VARIABLE_INFO* )script->temp_operands[ i ] ); 
			script->temp_operands[ i ] = NULL; 
		}
	}

	CloseHandle( script->lock ); 

	return ERROR_SUCCESS; 
}

LRESULT WINAPI get_script_output( HA_SCRIPT **script ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != script ); 
		*script = &ha_script_context; 
	}while( FALSE );
	return ret; 
}

LRESULT WINAPI check_valid_token( TOKEN_INFO *token )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != token ); 
		if( token->header.flags != PARAMETER_TYPE_TOKEN )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_token( TOKEN_INFO *token, 
						   INT32 token_type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != token ); 

		ret = check_valid_token( token ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( token->header.token != token_type )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_statement_param( STATEMENT_INFO *statement ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != statement ); 
		//statement->type
	}while( FALSE ); 

	return ret; 
}

INLINE LRESULT WINAPI check_statement_vaild( STATEMENT_INFO *statement )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != statement ); 
		if( statement->header.flags != PARAMETER_TYPE_STATEMENT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_statement( STATEMENT_INFO *statement, INT32 token )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != statement ); 
		ret = check_statement_vaild( statement ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		if( statement->header.token != token )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = check_statement_param( statement ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

#define STATEMENT_TOKEN_BEGIN ( program )
#define STATEMENT_TOKEN_END ( args )

#define TOKEN_TOKEN_BEGIN ( INTEGER_TYPE )
#define TOKEN_TOKEN_END ( EOL )

LRESULT WINAPI _input_token_info( STATEMENT_INFO *statement, 
								 GRAMMER_ELEMENT_HEADER *header )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != statement ); 
		ASSERT( NULL != header ); 

		if( ( PVOID )statement == ( PVOID )header )
		{
			dbg_print( MSG_IMPORTANT, "recursive rule %s\n", 
				get_token_desc( statement->header.token ) ); 
			break; 
		}

		if( statement->head == NULL )
		{
			ASSERT( statement->tail == NULL ); 
			statement->head = header; 
			statement->tail = header; 
			ASSERT( header->prev == NULL ); 
			ASSERT( header->next == NULL ); 

			header->parent = &statement->header; 
		}
		else
		{
			ASSERT( statement->tail != NULL ); 
			statement->tail->next = header; 
			header->prev = statement->tail; 
			statement->tail = header; 
			ASSERT( header->next == NULL ); 

			header->parent = &statement->header; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI input_token_info( STATEMENT_INFO *statement, 
								INT32 token_type, 
								PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	STATEMENT_INFO *_statement; 
	TOKEN_INFO *token; 

	do 
	{
		ASSERT( NULL != statement ); 

		if( NULL == param )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( token_type >= STATEMENT_TOKEN_BEGIN 
			&& token_type <= STATEMENT_TOKEN_END )
		{
			_statement = ( STATEMENT_INFO* )param; 
			ret = check_statement( _statement, token_type ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else if( token_type >= TOKEN_TOKEN_BEGIN 
			&& token_type <= TOKEN_TOKEN_END )
		{
			token = ( TOKEN_INFO* )param; 
			ret = check_token( token, token_type ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		ret = _input_token_info( statement, ( GRAMMER_ELEMENT_HEADER* )param ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
	} while ( FALSE );

	return ret; 
}

LRESULT WINAPI check_valid_grammer_element_header( GRAMMER_ELEMENT_HEADER *header ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != header ); 

		ret = check_valid_token_type( header->flags ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

	}while( FALSE );

	return ret; 
}

//INLINE LRESULT WINAPI add_grammer_elements( GRAMMER_ELEMENTS *elements_src, 
//										  GRAMMER_ELEMENTS *elements_dest )
//{
//	LRESULT ret = ERROR_SUCCESS; 
//	GRAMMER_ELEMENT_ITERATOR it; 
//
//	do 
//	{
//		ASSERT( elements_src != NULL ); 
//		ASSERT( elements_dest != NULL ); 
//
//		if( elements_src->size() == 0 )
//		{
//			ret = ERROR_INVALID_PARAMETER; 
//			break; 
//		}
//
//		for( it = elements_src->elements.begin(); 
//			it != elements_src->elements.end(); 
//			it ++ )
//		{
//			elements_dest.push_back( *( it ) ); 
//		}
//	}while( FALSE );
//
//	return ret; 
//}

typedef vector<GRAMMER_ELEMENT_HEADER*> GRAMMER_ELEMENTS; 

LRESULT WINAPI check_variable_valid( VARIABLE_INFO *variable ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != variable ); 

		if( 0 == variable->name.length() )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

	}while( FALSE );

	return ret; 
}

VARIABLE_INFO* WINAPI get_next_temp_operand( HA_SCRIPT *ha_script )
{
	VARIABLE_INFO *variable = NULL; 
	INT32 i; 

	do 
	{
		for( i = 1; i < ARRAYSIZE( ha_script->operands ); i ++ )
		{
			if( NULL == ha_script->operands[ i ] )
			{
				if( ha_script->temp_operands[ i ] == NULL ) 
				{
					ha_script->temp_operands[ i ] = new VARIABLE_INFO(); 
					if( NULL == ha_script->temp_operands[ i ]  )
					{
						//ret = ERROR_NOT_ENOUGH_MEMORY; 
						break; 
					}

					ha_script->operands[ i ] = ha_script->temp_operands[ i ]; 
				}
				else
				{
					ASSERT( 0 == ha_script->temp_operands[ i ]->name.length() ); 

					ha_script->operands[ i ] = ha_script->temp_operands[ i ]; 
				}

				variable = ha_script->operands[ i ]; 
				break; 
			}
		}

		if( i == ARRAYSIZE( ha_script->operands ) )
		{
			break; 
		}

	}while( FALSE );

	return variable; 
}

INLINE LRESULT WINAPI is_assign_operator( INT32 operator_type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( operator_type == ASSIGNOP 
			|| operator_type == PLUSASSNOP 
			|| operator_type == MINUSASSNOP )
		{
			break; 
		}

		ret = ERROR_INVALID_PARAMETER; 
			break; 
	}while( FALSE ); 
	return ret; 
}

INT32 WINAPI get_next_unuse_right_operand( HA_SCRIPT *ha_script )
{
	INT32 i; 

	do 
	{
		for ( i = RIGHT_OPERAND_INDEX; 
			i < ARRAYSIZE( ha_script->operands ); 
			i ++ )
		{
			if( ha_script->operands[ i ] == NULL )
			{
				break; 
			}
		}

		if( i == ARRAYSIZE( ha_script->operands ) )
		{
			i = -1; 
			break; 
		}

		if(  i > RIGHT_OPERAND_INDEX )
		{
			ASSERT( ha_script->operators[ 1 ] != 0 ); 
		}

	}while( FALSE ); 

	return i; 
}

INLINE LRESULT WINAPI check_value_type_valid( INT32 vt )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( vt != VARIABLE_INT 
			&& vt != VARIABLE_R8 )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_operand_valid( VARIABLE_INFO *operand )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != operand ); 
		ret = check_value_type_valid( operand->variable.vt ); 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI convert_operand_to_type( VARIABLE_INFO *operand, INT32 vt ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != operand ); 

		ret = check_operand_valid( operand ); 
		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}

		switch( vt )
		{
		case VARIABLE_INT:
			switch( operand->variable.vt )
			{
			case VARIABLE_R8:
				operand->variable.intVal = ( INT32 )operand->variable.fltVal; 
				operand->variable.vt = VARIABLE_INT; 
				break; 
			}
			break; 
		case VARIABLE_R8:
			switch( operand->variable.vt )
			{
			case VARIABLE_INT:
				operand->variable.fltVal = ( FLOAT )operand->variable.intVal; 
				operand->variable.vt = VARIABLE_R8; 
				break; 
			}
			break; 
		default:
			ret = ERROR_INVALID_PARAMETER; 
			ASSERT( FALSE ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI config_operate_value_type( VARIABLE_INFO *left_operand, 
										VARIABLE_INFO *right_operand1, 
										VARIABLE_INFO *right_operand2 )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 base_value_type; 
	//INT32 precise_value_type; 

	do 
	{
		ASSERT( NULL != left_operand ); 
		ASSERT( NULL != right_operand1 ); 

		base_value_type = left_operand->variable.vt; 
		ASSERT( ERROR_SUCCESS == check_value_type_valid( base_value_type ) ); 

		ret = convert_operand_to_type( right_operand1, VARIABLE_INT ); 
		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		if( right_operand2 != NULL )
		{
			ret = convert_operand_to_type( right_operand2, VARIABLE_INT ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}
	}while( FALSE ); 
	
	return ret; 
}

INLINE LRESULT WINAPI _exec_variable_operate( VARIABLE_INFO *left_operand, 
									 VARIABLE_INFO *right_operand1, 
									 VARIABLE_INFO *right_operand2, 
									 INT32 operator1, 
									 INT32 operator2 )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != left_operand ); 
		ASSERT( NULL != right_operand1 ); 

		ret = config_operate_value_type( left_operand, right_operand1, right_operand2 ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		switch( operator2 )
		{
		case NOT:
			ASSERT( right_operand2 == NULL ); 
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = !right_operand1->variable.intVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case PLUS: 
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal += right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal += right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case MINUS:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal -= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal -= right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case STAR:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal *= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal *= right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case DIV:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal /= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal /= right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case AND:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal && right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case OR:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal || right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case BITAND:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal &= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case BITOR:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal |= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case GREATEROP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal > right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal > right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case LOWEROP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal < right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal < right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case GREATEQUALOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal >= right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal >= right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case LOWEREUQALOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal <= right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal <= right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case EUALOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal == right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal == right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case NEUQALOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = ( right_operand1->variable.intVal != right_operand2->variable.intVal ); 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = ( FLOAT )( right_operand1->variable.fltVal != right_operand2->variable.fltVal ); 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case ASSIGNOP: 
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal = right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal = right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case PLUSASSNOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal += right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal += right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case MINUSASSNOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				right_operand1->variable.intVal -= right_operand2->variable.intVal; 
				break; 
			case VARIABLE_R8:
				right_operand1->variable.fltVal -= right_operand2->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		default:
			if( operator2 != 0 )
			{
				ASSERT( FALSE ); 
				ret = ERROR_INVALID_PARAMETER; 
			}
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		switch( operator1 )
		{
		case ASSIGNOP: 
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				left_operand->variable.intVal = right_operand1->variable.intVal; 
				break; 
			case VARIABLE_R8:
				left_operand->variable.fltVal = right_operand1->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case PLUSASSNOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				left_operand->variable.intVal += right_operand1->variable.intVal; 
				break; 
			case VARIABLE_R8:
				left_operand->variable.fltVal += right_operand1->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		case MINUSASSNOP:
			switch( left_operand->variable.vt )
			{
			case VARIABLE_INT:
				left_operand->variable.intVal -= right_operand1->variable.intVal; 
				break; 
			case VARIABLE_R8:
				left_operand->variable.fltVal -= right_operand1->variable.fltVal; 
				break; 
			default:
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI exec_variable_operate( HA_SCRIPT *script )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ret = _exec_variable_operate( script->operands[ 0 ], 
			script->operands[ 1 ], 
			script->operands[ 2 ], 
			script->operators[ 0 ], 
			script->operators[ 1 ] ); 

		if( ret != ERROR_SUCCESS )
		{
			break; 
		}

		script->operands[ RIGHT_OPERAND_INDEX ] = NULL; 
		script->operands[ RIGHT_OPERAND_INDEX + 1 ] = NULL; 

		script->operators[ 1 ] = 0; 
	}while( FALSE );

	return ret; 
}

INLINE LRESULT WINAPI allocate_left_operation( VARIABLE_INFO **variable )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	ASSERT( NULL != variable ); 
	*variable = NULL; 

	do 
	{
		ASSERT( NULL == ha_script_context.operands[ LEFT_OPERAND_INDEX ] ); 
		if( NULL == ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] )
		{
			ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] = new VARIABLE_INFO(); 
			if( NULL == ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}
		}

		*variable = ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ]; 
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI exec_token_operation( TOKEN_INFO *token ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != token ); 

		switch( token->header.token )
		{
		case INTEGER_TYPE: 
			{
				VARIABLE_INFO *variable = NULL; 
				LPSTR text_end; 

				do 
				{
					variable = get_next_temp_operand( &ha_script_context ); 
					if( NULL == variable )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					variable->name = "*"; 
					variable->variable.vt = VARIABLE_INT; 
					variable->variable.intVal = ( INT32 )strtol( token->text.c_str(), 
						&text_end, 
						0 ); 

					ret = check_variable_valid( variable ); 
					if( ret != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						break; 
					}

					if( is_assign_operator( ha_script_context.operators[ 0 ] ) != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					ret = exec_variable_operate( &ha_script_context ); 
				} while ( FALSE ); 
			}
			break; 
		case FLOAT_TYPE: 
			{
				VARIABLE_INFO *variable = NULL; 
				LPSTR text_end; 

				do 
				{
					variable = get_next_temp_operand( &ha_script_context ); 
					if( NULL == variable )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					variable->name = "*"; 
					variable->variable.vt = VARIABLE_R8; 
					variable->variable.dblVal = strtod( token->text.c_str(), 
						&text_end ); 

					ret = check_variable_valid( variable ); 
					if( ret != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						break; 
					}
				} while ( FALSE ); 
			}
			break; 
		case TYPE: 
			{
				VARIABLE_INFO *variable = NULL; 
				
				do 
				{
					ASSERT( NULL == ha_script_context.operands[ LEFT_OPERAND_INDEX ] ); 
					if( NULL == ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] )
					{
						ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] = new VARIABLE_INFO(); 
						if( NULL == ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] )
						{
							ret = ERROR_NOT_ENOUGH_MEMORY; 
							break; 
						}
					}

					variable = ha_script_context.temp_operands[ LEFT_OPERAND_INDEX]; 

					ASSERT( variable->name.length() == 0 ); 

					if( 0 == strcmp( "int", token->text.c_str() ) )
					{
						variable->variable.vt = VARIABLE_INT; 
					}
					else if( 0 == strcmp( "float", token->text.c_str() ) )
					{
						variable->variable.vt = VARIABLE_R8; 
					}
					else
					{
						ASSERT( FALSE ); 
						ret = ERROR_INVALID_PARAMETER; 
						break; 
					}

					ha_script_context.operands[ LEFT_OPERAND_INDEX ] = variable; 
				} while ( FALSE ); 

				if( ERROR_SUCCESS )
				{
					if( NULL != variable )
					{
						delete variable; 
					}
				}
			}
			break; 
		case STRUCT: 
			break; 
		case RETURN: 
			break; 
		case IF: 
			{
				VARIABLE_INFOS::iterator it; 
				//INT32 right_operand_index; 

				ASSERT( NULL == ha_script_context.operands[ LEFT_OPERAND_INDEX ] ); 

				it = ha_script_context.variables.find( token->text ); 
				if( it == ha_script_context.variables.end() )
				{
					VARIABLE_INFO *variable = NULL; 
					ret = allocate_left_operation( &variable ); 

					if( ERROR_SUCCESS != ret )
					{
						break; 
					}

					variable->name = token->text; 
					variable->variable.vt = VARIABLE_INT; 
					variable->variable.intVal = 0; 

					ha_script_context.variables.insert(  pair< string, VARIABLE_INFO* >( token->text, 
						variable ) ); 

					ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] = NULL; 

					ha_script_context.operands[ LEFT_OPERAND_INDEX ] = variable; 
					ha_script_context.operators[ 0 ] = ASSIGNOP; 
					break; 
				}

				ha_script_context.operands[ LEFT_OPERAND_INDEX ] = ( *it ).second; 
				ha_script_context.operands[ LEFT_OPERAND_INDEX ]->variable.vt = VARIABLE_INT; 
				ha_script_context.operands[ LEFT_OPERAND_INDEX ]->variable.intVal = 0; 
				ha_script_context.operators[ 0 ] = ASSIGNOP; 
				break; 
			}
			break; 
		case ELSE: 
			break; 
		case WHILE: 
			break; 
		case ID: 
			{
				VARIABLE_INFO *variable = NULL; 

				do 
				{
#define ERROR_VARIABLE_NOT_FOUND 0xe0111012
					VARIABLE_INFOS::iterator it; 

					if( NULL == ha_script_context.operands[ LEFT_OPERAND_INDEX ] )
					{
						VARIABLE_INFOS::iterator it; 
						ASSERT( 0 < token->text.length() ); 

						it = ha_script_context.variables.find( token->text ); 
						if( it == ha_script_context.variables.end() )
						{
							ret = ERROR_VARIABLE_NOT_FOUND; 
							break; 
						}

						ha_script_context.operands[ LEFT_OPERAND_INDEX ] = ( *it ).second; 
						break; 
					}

					if( ha_script_context.operands[ LEFT_OPERAND_INDEX ]->name.length() == 0 )
					{
						it = ha_script_context.variables.find( token->text ); 
						if( it != ha_script_context.variables.end() )
						{
							CHAR message[ MAX_ERROR_MESSAGE_LEN ]; 

							StringCchPrintfA( message, 
								ARRAYSIZE( message ), 
								"variable(%s) already exist", 
								token->text.c_str() ); 

							ret = ERROR_INVALID_PARAMETER; 

							ha_script_context.error_message = message; 
							ha_script_context.error_code = ret; 

							//ha_script_context.operands[ LEFT_OPERAND_INDEX ] = NULL; 
							break; 
						}

						ha_script_context.operands[ LEFT_OPERAND_INDEX ]->name = token->text; 
						ha_script_context.variables.insert( pair< string, VARIABLE_INFO* >( token->text, 
							ha_script_context.operands[ LEFT_OPERAND_INDEX ] ) ); 
						ha_script_context.temp_operands[ LEFT_OPERAND_INDEX ] = NULL; 
						break; 
					}

					if( is_assign_operator( ha_script_context.operators[ 0 ] ) != ERROR_SUCCESS )
					{
						ASSERT( FALSE ); 
						ret = ERROR_ERRORS_ENCOUNTERED; 
						break; 
					}

					{
						INT32 right_operand_index; 
						right_operand_index = get_next_unuse_right_operand( &ha_script_context ); 
						if( right_operand_index == -1 )
						{
							ret = ERROR_ERRORS_ENCOUNTERED; 
							break; 
						}

						it = ha_script_context.variables.find( token->text ); 
						if( it == ha_script_context.variables.end() )
						{
							CHAR message[ MAX_ERROR_MESSAGE_LEN ]; 

							StringCchPrintfA( message, 
								ARRAYSIZE( message ), 
								"variable(%s) is not exist", 
								token->text.c_str() ); 

							ret = ERROR_INVALID_PARAMETER; 

							ha_script_context.error_message = message; 
							ha_script_context.error_code = ret; 

							break; 
						}

						ha_script_context.operands[ right_operand_index ] = ( *it ).second; 
						
						ret = exec_variable_operate( &ha_script_context ); 
					}
				} while ( FALSE ); 

				//if( NULL != ha_script_context.current_variable )
				//{
				//	ha_script_context.current_variable = NULL; 
				//}
			}
			break; 
		case SEMI: 
			{
				ASSERT( ha_script_context.operands[ RIGHT_OPERAND_INDEX ] == NULL ); 
			}
			break; 
		case COMMA: 
			break; 
		case ASSIGNOP: 
		case PLUSASSNOP: 
		case MINUSASSNOP: 
			{
				if( ha_script_context.operators[ 0 ] != 0 )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ha_script_context.operators[ 0 ] = token->header.token; 
			}
			break; 
		case PLUS: 
		case MINUS: 
		case STAR: 
		case DIV: 
		case AND: 
		case OR: 
		case GREATEROP:
		case LOWEROP:
		case GREATEQUALOP:
		case LOWEREUQALOP:
		case EUALOP:
		case NEUQALOP:
			{
				if( ha_script_context.operators[ 1 ] != 0 )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ha_script_context.operators[ 1 ] = token->header.token; 
				if( NULL == ha_script_context.operands[ RIGHT_OPERAND_INDEX ] )
				{
					VARIABLE_INFO *variable; 
					variable = get_next_temp_operand( &ha_script_context ); 
					if( NULL == variable )
					{
						break; 
					}

					variable->name = "*"; 
					variable->variable.vt = ha_script_context.operands[ LEFT_OPERAND_INDEX ]->variable.vt; 
					variable->variable.llVal = ha_script_context.operands[ LEFT_OPERAND_INDEX ]->variable.llVal; 
				}
			}
			break; 
		//case DOT: 
		//	{
		//		if( ha_script_context.operators[ 1 ] != 0 )
		//		{
		//			ret = ERROR_ERRORS_ENCOUNTERED; 
		//			break; 
		//		}

		//		ha_script_context.operators[ 1 ] = DOT; 
		//	}
		//	break; 
		case NOT: 
			{
				if( ha_script_context.operators[ 1 ] != 0 )
				{
					ret = ERROR_ERRORS_ENCOUNTERED; 
					break; 
				}

				ha_script_context.operators[ 1 ] = NOT; 
			}
			break; 
		case LPB: 
			break; 
		case RPB: 
			{
				
			}
			break; 
		case LB: 
			break; 
		case RB: 
			break; 
		case LC: 
			break; 
		case RC: 
			break; 
		case AERROR:
			break; 
		case EOL:
			ret = ERROR_INVALID_PARAMETER; 
			dbg_print( MSG_FATAL_ERROR, 
				"unknown word at line %d: mysterious character '%s'\n", 
				yylineno, 
				yytext ); 
			break; 
		default:
			ret = ERROR_INVALID_PARAMETER; 
			dbg_print( MSG_FATAL_ERROR, 
				"unknown word at line %d: mysterious character '%s'\n", 
				yylineno, 
				yytext ); 
			break; 
		}

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI exec_script_element( GRAMMER_ELEMENT_HEADER *element )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != element ); 
		switch( element->flags )
		{
		case PARAMETER_TYPE_STATEMENT: 
			break; 
		case PARAMETER_TYPE_TOKEN: 
			ret = exec_token_operation( ( TOKEN_INFO* )element ); 
			break; 
		default:
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI exec_script_parsed( STATEMENT_INFO *statement )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	//GRAMMER_ELEMENT_HEADER *element; 
	STATEMENT_INFO *_statement; 
	//TOKEN_INFO *_token; 
	GRAMMER_ELEMENTS elements; 
	GRAMMER_ELEMENT_HEADER *header; 
	BOOLEAN leaf_element; 

	do 
	{
		ASSERT( NULL != statement ); 
		print_grammer_tree( &statement->header, 0 ); 
		
		ret = check_statement( statement, program ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		ASSERT( NULL == statement->header.parent ); 
		
		if( statement->head == NULL )
		{
			ASSERT( NULL != statement->tail ); 

			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		elements.push_back( statement->head ); 

		for( ; ; )
		{
			if( elements.size() == 0 )
			{
				break; 
			}

			header = *elements.rbegin(); 
			elements.pop_back(); 

			dbg_print( MSG_IMPORTANT, "execute script element %s\n", 
				get_token_desc( header->token ) ); 

			leaf_element = TRUE; 
			if( NULL != header->next )
			{
				elements.push_back( header->next ); 
			}

			if( PARAMETER_TYPE_STATEMENT == header->flags )
			{
				_statement = ( STATEMENT_INFO* )header; 
				if( _statement->head != NULL )
				{
					elements.push_back( _statement->head ); 
					leaf_element = FALSE; 
				}
			}

			if( leaf_element == TRUE )
			{
				_ret = exec_script_element( header ); 
				if( ERROR_SUCCESS != _ret )
				{
					ASSERT( FALSE ); 
					ret = _ret; 
					break; 
				}
			}
		}
	}while( FALSE );

	return ret; 
}

VOID WINAPI init_grammer_element_header( GRAMMER_ELEMENT_HEADER *header, ULONG type )
{
	header->flags = type; 
	header->next = NULL; 
	header->prev = NULL; 
	header->parent = NULL; 
}

STATEMENT_INFO *allocate_statement_info( INT32 token )
{
	LRESULT ret = ERROR_SUCCESS; 
	STATEMENT_INFO *statement = NULL; 
	
	do 
	{
		statement = new STATEMENT_INFO(); 
		if( NULL == statement ) 
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			yyerror( "out of space" );
			break; 
		}

		init_grammer_element_header( &statement->header, 
			PARAMETER_TYPE_STATEMENT ); 

		statement->head = NULL; 
		statement->tail = NULL; 
		statement->header.token = token; 

		lock_mutex( ha_script_context.lock ); 
		ha_script_context.elements.push_back( &statement->header ); 
		unlock_mutex( ha_script_context.lock ); 
	}while( FALSE );

	if( ERROR_SUCCESS != ret )
	{
		delete statement; 
		statement = NULL; 
	}

	return statement; 
};

TOKEN_INFO *allocate_token_info( INT32 token )
{
	LRESULT ret = ERROR_SUCCESS; 
	TOKEN_INFO *_token = NULL; 

	do 
	{
		_token = new TOKEN_INFO(); 
		if( NULL == _token ) 
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			yyerror( "out of space" );
			break; 
		}

		init_grammer_element_header( &_token->header, PARAMETER_TYPE_TOKEN ); 

		_token->header.token = token; 

		lock_mutex( ha_script_context.lock ); 
		ha_script_context.elements.push_back( &_token->header ); 
		unlock_mutex( ha_script_context.lock ); 
	}while( FALSE );

	if( ERROR_SUCCESS != ret )
	{
		delete _token; 
		_token = NULL; 
	}

	return _token; 
} 

LRESULT WINAPI _free_grammer_element( GRAMMER_ELEMENT_HEADER *header, 
									ULONG token_type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( NULL != header ); 

		if( token_type == PARAMETER_TYPE_UNKNOWN )
		{
			token_type = header->flags; 
		}
		else
		{
			ASSERT( header->flags == token_type ); 
		}

		if( token_type == PARAMETER_TYPE_TOKEN )
		{
			TOKEN_INFO *token; 
			token = ( TOKEN_INFO* )header; 
			delete token; 
		}
		else if( token_type == PARAMETER_TYPE_STATEMENT )
		{
			STATEMENT_INFO *statement; 
			statement = ( STATEMENT_INFO* )header; 
			delete statement; 
		}
		else
		{
			ASSERT( FALSE ); 
			ret = ERROR_ERRORS_ENCOUNTERED; 
		}
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI free_grammer_element( GRAMMER_ELEMENT_HEADER *header, 
									ULONG token_type )
{
	LRESULT ret = ERROR_SUCCESS; 
	vector< GRAMMER_ELEMENT_HEADER*>::iterator it; 

	do 
	{
		ASSERT( NULL != header ); 
		
		lock_mutex( ha_script_context.lock ); 
		
		for( it = ha_script_context.elements.begin(); 
			it != ha_script_context.elements.end(); 
			it ++ )
		{
			if( ( *it ) == header )
			{
				ha_script_context.elements.erase( it ); 
				break; 
			}
		}

		unlock_mutex( ha_script_context.lock ); 
		ret = _free_grammer_element( header, token_type ); 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI add_grammer_element( ULONG none_terminal, 
									va_list va, 
									ULONG param_num, 
									STATEMENT_INFO **statement_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	STATEMENT_INFO *statement = NULL; 
	INT32 token; 
	PVOID param; 
	INT32 i; 
	
	do
	{
		ASSERT( NULL != statement_out ); 
		*statement_out = NULL; 

		statement = allocate_statement_info( none_terminal ); 
		if( NULL == statement ) 
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			yyerror( "out of space" );
			break; 
		}

		for( i = 0; ( ULONG )i < param_num; i ++ )
		{
			token = va_arg( va, INT32 ); 
			//if( token != ID )
			//{
			//	ret = ERROR_INVALID_PARAMETER; 
			//	break; 
			//}

			param = va_arg( va, PVOID ); 
			ret = input_token_info( statement, token, param ); 
			if( ret != ERROR_SUCCESS )
			{
				ASSERT( FALSE ); 
				break; 
			}
		}

		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
		
		*statement_out = statement; 
	}while( FALSE ); 

	if( ERROR_SUCCESS != ret )
	{
		if( NULL != statement )
		{
			free_grammer_element( &statement->header, PARAMETER_TYPE_STATEMENT ); 
			statement = NULL; 
		}
	}

	return ret; 
} 

STATEMENT_INFO *handle_grammer_element(INT32 none_terminal,
	INT32 param_num, ...) 
{
	LRESULT ret = ERROR_SUCCESS; 
	//INT32 i; 
	va_list va; 
	STATEMENT_INFO *statement = NULL; 

	//STATEMENT_INFO *_statement; 
	//TOKEN_INFO *_token; 

	//INT32 token; 

	do 
	{
		dbg_print( MSG_IMPORTANT, "handle grammer element %s %d\n", 
			get_token_desc( none_terminal ), 
			param_num ); 

		ret = ERROR_NOT_SUPPORTED; 
		switch( none_terminal )
		{
		case program:
			{
				//statement = allocate_statement_info( none_terminal ); 
				//if( NULL == statement )
				//{
				//	ret = ERROR_NOT_ENOUGH_MEMORY; 
				//	break; 
				//}

				//if( param_num != 0 
				//	&& param_num != 1 )
				//{
				//	ret = ERROR_INVALID_PARAMETER; 
				//	break; 
				//}

				//if( param_num == 0 )
				//{
				//	dbg_print( MSG_IMPORTANT, "script is null string\n" ); 
				//	break; 
				//}

				//va_start( va, param_num ); 
				//token = va_arg( va, INT32 ); 
				//if( token != extdeflist )
				//{
				//	ret = ERROR_INVALID_PARAMETER; 
				//	break; 
				//}

				//param = va_arg( va, PVOID ); 

				//ret = input_token_info( statement, extdeflist, param ); 
				//if( ret != ERROR_SUCCESS )
				//{
				//	break; 
				//}

				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 

				if( ret != ERROR_SUCCESS )
				{
					break; 
				}

				ret = exec_script_parsed( statement ); 
			}
			break; 
		case extdeflist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 			
		case extdef: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case extdeclist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case specification: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case structspecification: 
			break; 	
		case opttag: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case tag: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case vardec: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case fundec: 
			break; 	
		case varlist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case paramdec: 
			break; 	
		case compst: 
			break; 	
		case stmtlist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case stmt: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case deflist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 	
		case def: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 
		case declist: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 
		case decl: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 
		case exp: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
			break; 
		case args: 
			{
				va_start( va, param_num ); 
				ret = add_grammer_element( none_terminal, va, param_num, &statement ); 
				va_end( va ); 
			}
		default:
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( ret != ERROR_SUCCESS )
		{
			ASSERT( FALSE ); 
			break; 
		}
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( NULL != statement )
		{
			free_grammer_element( &statement->header, PARAMETER_TYPE_STATEMENT );  
			statement = NULL; 
		}
	}

	return statement;
}

LRESULT WINAPI check_lex_elemen_valid( INT32 otken )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
	//	switch( token_type )
	//	{
	//	case INTEGER_TYPE: 
	//		break; 
	//	case FLOAT_TYPE: 
	//		break; 
	//	case TYPE: 
	//		break; 
	//	case STRUCT: 
	//		break; 
	//	case RETURN: 
	//		break; 
	//	case IF: 
	//		break; 
	//	case ELSE: 
	//		break; 
	//	case WHILE: 
	//		break; 
	//	case ID: 
	//		break; 
	//	case SEMI: 
	//		break; 
	//	case COMMA: 
	//		break; 
	//	case ASSIGNOP: 
	//		break; 
	//	case RELOP: 
	//		break; 
	//	case PLUS: 
	//		break; 
	//	case MINUS: 
	//		break; 
	//	case STAR: 
	//		break; 
	//	case DIV: 
	//		break; 
	//	case AND: 
	//		break; 
	//	case OR: 
	//		break; 
	//	case DOT: 
	//		break; 
	//	case NOT: 
	//		break; 
	//	case LPB: 
	//		break; 
	//	case RPB: 
	//		break; 
	//	case LB: 
	//		break; 
	//	case RB: 
	//		break; 
	//	case LC: 
	//		break; 
	//	case RC: 
	//		break; 
	//	case AERROR:
	//		break; 
	//	case EOL:
	//		break; 
	//	default:
	//	break; 
	//}
		//if( otken < )
		//{
		//	ret = ERROR_INVALID_PARAMETER; 
		//	dbg_print( MSG_FATAL_ERROR, 
		//		"unknown word at line %d: mysterious character '%s'\n", 
		//		yylineno, 
		//		yytext ); 
		//}
	}while( FALSE );

	return ret; 
}

TOKEN_INFO *handle_lex_element( INT32 token_type, 
							   INT32 param_num, ...) 
{
	LRESULT ret = ERROR_SUCCESS; 
	//va_list valist; 
	//INT32 type; 
	TOKEN_INFO *token_info = NULL; 
	
	do 
	{
		dbg_print( MSG_IMPORTANT, "handle lex element %s %d\n", 
			get_token_desc( token_type ), 
			param_num ); 

		ret = check_lex_elemen_valid( token_type ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}

		token_info = allocate_token_info( token_type ); 
		if( NULL == token_info )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		token_info->text = yytext; 
	}while( FALSE );

	if( ret != ERROR_SUCCESS )
	{
		if( NULL != token_info )
		{
			free_grammer_element( &token_info->header, PARAMETER_TYPE_TOKEN ); 
			token_info = NULL; 
		}
	}

	return token_info;
}

LRESULT WINAPI print_grammer_tree(GRAMMER_ELEMENT_HEADER *header, 
								  INT level)
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 i; 

	do 
	{
		if( header == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		for( i = 0; i < level; i++  ) 
		{
			dbg_print( MSG_IMPORTANT, "  ");
		}

		//if( header->line != -1 ) 
		dbg_print( MSG_IMPORTANT, "%s ", get_token_desc( header->token ) ); 

		//if( ( 0 == strcmp( header->token, ID ) ) 
		//	|| ( 0 == strcmp( header->token, TYPE ) ) )
		//{
		//	dbg_print( MSG_IMPORTANT, ":%s ",header->token ); 
		//}
		//else if( 0 == strcmp( header->token, INTEGER_TYPE ) )
		//{
		//	dbg_print( MSG_IMPORTANT, ":%d", header->token ); 
		//}
		//else
		//{
		//	dbg_print( MSG_IMPORTANT, "(%d)",a->line);
		//}
		dbg_print( MSG_IMPORTANT, "\n" );

		if( header->flags == PARAMETER_TYPE_STATEMENT )
		{
			GRAMMER_ELEMENT_HEADER *_header; 
			STATEMENT_INFO *statement; 
			statement = ( STATEMENT_INFO* )header; 

			_header = statement->head; 
			for( ; ; )
			{
				if( _header == NULL )
				{
					break; 
				}

				print_grammer_tree( _header, level + 1 ); 
				
				_header = _header->next; 
			}
		}
	}while( FALSE );

	return ret; 
}

void yyerror(char*s,...) //变长参数错误处理函数
{
	va_list ap;
	va_start(ap,s);
	fprintf(stderr,"%d:error:",yylineno);//错误行号
	vfprintf(stderr,s,ap);
	fprintf(stderr,"\n");
}

#if (defined __STDC__ || defined __C99__FUNC__ \
	|| defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
const char *yymsg;
int yytype;
YYSTYPE *yyvaluep;
#endif
{
	//YYUSE (yyvaluep);

	//if (!yymsg)
	//	yymsg = "Deleting";
	//YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

	switch (yytype)
	{

	default:
		break;
	}
}

typedef struct _PARSE_TEXT_INFO
{
	LPSTR text; 
	ULONG cc_text_len; 
} PARSE_TEXT_INFO, *PPARSE_TEXT_INFO; 

PARSE_TEXT_INFO parse_text_info = { 0 }; 

#define DONT_ALLOCATE_INPUT_TEXT_BUFFER 0x00000001

LRESULT WINAPI uninit_parse_text()
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( parse_text_info.text != NULL )
		{
			//ASSERT( 0 < parse_text_info.cc_text_len ); 
			free( parse_text_info.text ); 
			parse_text_info.text = NULL; 
		}

		parse_text_info.cc_text_len = 0; 
	}while( FALSE ); 

	return ret; 
}

LRESULT WINAPI init_parse_text( LPCSTR text, ULONG cc_text_len, ULONG flags )
{
	LRESULT ret = ERROR_SUCCESS; 
	
	do 
	{
		ASSERT( NULL != text ); 
		ASSERT( parse_text_info.text == NULL ); 

		parse_text_info.text = NULL; 
		parse_text_info.cc_text_len = 0; 

		if( text == NULL )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}

		if( cc_text_len == 0 )
		{
			cc_text_len = strlen( text ); 

			if( cc_text_len == 0 )
			{
				ret = ERROR_INVALID_PARAMETER; 
				break; 
			}
		}

		if( 0 == ( flags & DONT_ALLOCATE_INPUT_TEXT_BUFFER ) )
		{
			parse_text_info.text = ( LPSTR )malloc( ( cc_text_len + 1 )* sizeof( CHAR ) ); 
			if( NULL == parse_text_info.text )
			{
				ret = ERROR_NOT_ENOUGH_MEMORY; 
				break; 
			}

			memcpy( parse_text_info.text, text, ( cc_text_len + 1 ) * sizeof( CHAR ) ); 
			parse_text_info.cc_text_len = cc_text_len; 
		}
		else
		{
			parse_text_info.text = ( LPSTR )text; 
			parse_text_info.cc_text_len = cc_text_len; 
		}
	}while( FALSE );
	
	return ret; 
}

int eval_getinput(char *buf, int maxlen )
{
	int ret; 

	do 
	{
		if( parse_text_info.text == NULL 
			|| parse_text_info.cc_text_len == 0 )
		{
			ret = 0; 
			break; 
		}

		ASSERT( maxlen > 0 ); 
		if( parse_text_info.cc_text_len >= ( ULONG )maxlen )
		{
			memcpy( buf, parse_text_info.text, maxlen * sizeof( CHAR ) ); 
			
			memmove( parse_text_info.text, 
				parse_text_info.text + maxlen, 
				parse_text_info.cc_text_len - maxlen ); 

			parse_text_info.cc_text_len -= maxlen; 
			ret = maxlen; 
			break; 
		}

		memcpy( buf, parse_text_info.text, parse_text_info.cc_text_len * sizeof( CHAR ) ); 
		*parse_text_info.text = L'\0'; 
		ret = parse_text_info.cc_text_len; 
		parse_text_info.cc_text_len = 0; 
	} while ( FALSE );

	return ret; 
}

LRESULT load_test_text( LPCWSTR file_name )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	LARGE_INTEGER file_size; 
	ULONG buf_size; 
	ULONG data_readed; 
	LPSTR text = NULL; 
	HANDLE file_handle = INVALID_HANDLE_VALUE; 

	do 
	{
		file_handle = CreateFileW( file_name, 
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL ); 

		if( INVALID_HANDLE_VALUE == file_handle )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		_ret = GetFileSizeEx( file_handle, &file_size ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

#define MAX_INPUT_FILE_SIZE ( 1024 * 1024 * 128 ) 
		if( file_size.QuadPart > MAX_INPUT_FILE_SIZE )
		{
			buf_size = MAX_INPUT_FILE_SIZE; 
		}
		else
		{
			buf_size = file_size.LowPart; 
		}

		text = ( LPSTR )malloc( buf_size * sizeof( CHAR ) );
		if( NULL == text )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY; 
			break; 
		}

		_ret = ReadFile( file_handle, text, buf_size, &data_readed, NULL ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		if( buf_size != data_readed )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ret = init_parse_text( text, buf_size, DONT_ALLOCATE_INPUT_TEXT_BUFFER ); 
	}while( FALSE );

	return ret; 
}

LRESULT init_yy( LPSTR buf, ULONG cc_buf_len )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
#if 0
		ASSERT( yy_init == 0 );
#ifdef YY_USER_INIT
		YY_USER_INIT;
#endif

		if ( ! yy_start )
			yy_start = 1;	/* first start state */

		if ( ! yyin )
			yyin = stdin;

		if ( ! yyout )
			yyout = stdout;

		if ( ! yy_current_buffer )
			yy_current_buffer =
			yy_create_buffer( yyin, YY_BUF_SIZE );

		yy_load_buffer_state(); 
#endif //0
	}while( FALSE );

	return ret; 
}

#define _HA_PARSER_STATIC_LINK_ 1
#ifndef _HA_PARSER_STATIC_LINK_
int main()
{
	load_test_text( L"E:\\html_maker\\data_scrambler\\syntax\\test_text.c" ); 
	dbg_print( MSG_IMPORTANT, ">");
	yyparse(); //启动文法分析，调用词法分析

	getch(); 
	return 0; 
}
#else
int yyparse ();
EXTERN_C LRESULT WINAPI parse_ha_script( LPCSTR script )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		ASSERT( ha_script_context.operands[ 0 ] == NULL ); 
		ASSERT( ha_script_context.operands[ 1 ] == NULL ); 
		ASSERT( ha_script_context.operands[ 2 ] == NULL ); 
		ASSERT( ha_script_context.operators[ 0 ] == 0 ); 

		//reset_ha_script( &ha_script_context ); 

		ret = init_parse_text( script, strlen( script ), 0 ); 
		if( ERROR_SUCCESS != ret )
		{
			break; 
		}
		
		dbg_print( MSG_IMPORTANT, ">");
		yyparse(); //启动文法分析，调用词法分析 

		uninit_parse_text(); 
		reset_ha_script( &ha_script_context ); 
	}while( FALSE ); 

	return ret; 
}

EXTERN_C LRESULT WINAPI uninit_ha_script()
{
	return _uninit_ha_script( &ha_script_context ); 
}

EXTERN_C LRESULT WINAPI init_ha_script()
{
	return _init_ha_script( &ha_script_context ); 
}

EXTERN_C LRESULT WINAPI clean_ha_variables()
{
	return _clean_ha_variables( &ha_script_context ); 
}

EXTERN_C LRESULT WINAPI get_script_variable( LPCSTR name, VARIABLE_INFO *variable ) 
{
	LRESULT ret = ERROR_SUCCESS; 
	string _name; 
	VARIABLE_INFOS::iterator it; 

	ASSERT( name != NULL ); 
	ASSERT( variable != NULL ); 

	do 
	{
		_name = name; 
		it = ha_script_context.variables.find( _name ); 
		if( it == ha_script_context.variables.end() )
		{
			variable->name.clear(); 
			variable->variable.vt = VARIABLE_NULL; 

			ret = ERROR_NOT_FOUND; 
			break; 
		}

		variable->name = ( *it ).second->name; 
		variable->variable = ( *it ).second->variable; 
	}while( FALSE );

	return ret; 
}
#endif //_HA_PARSER_STATIC_LINK_