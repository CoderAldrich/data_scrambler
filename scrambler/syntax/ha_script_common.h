#ifndef __HA_SCRIPT_COMMON_H__
#define __HA_SCRIPT_COMMON_H__

#include "common_func.h"
#include <string>
#include <map>
#pragma warning(push)
#pragma warning(disable:4995)
#include <vector>
#pragma warning(pop)
using namespace std; 

#define PARAMETER_TYPE_TOKEN 0x00000001
#define PARAMETER_TYPE_STATEMENT 0x00000002
#define PARAMETER_TYPE_UNKNOWN 0x00000000

INLINE LRESULT WINAPI check_valid_token_type( ULONG type )
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		if( type < PARAMETER_TYPE_TOKEN 
			|| type > PARAMETER_TYPE_STATEMENT )
		{
			ret = ERROR_INVALID_PARAMETER; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

typedef struct _GRAMMER_ELEMENT_HEADER
{
	ULONG flags; 
	INT32 token; 
	struct _GRAMMER_ELEMENT_HEADER *parent; 
	struct _GRAMMER_ELEMENT_HEADER *prev; 
	struct _GRAMMER_ELEMENT_HEADER *next; 
} GRAMMER_ELEMENT_HEADER, *PGRAMMER_ELEMENT_HEADER; 

typedef struct _TOKEN_INFO
{
	struct _GRAMMER_ELEMENT_HEADER header; 
	string text; 
} TOKEN_INFO, *PTOKEN_INFO; 

//typedef vector<PVOID> GRAMMER_ELEMENTS; 
//typedef GRAMMER_ELEMENTS::iterator GRAMMER_ELEMENT_ITERATOR; 

typedef struct _STATEMENT_INFO
{
	struct _GRAMMER_ELEMENT_HEADER header; 
	struct _GRAMMER_ELEMENT_HEADER *head; 
	struct _GRAMMER_ELEMENT_HEADER *tail; 
} STATEMENT_INFO, *PSTATEMENT_INFO; 

typedef enum _VARIABLE_TYPE_ENUM
{ 
	VARIABLE_EMPTY  = 0,
	VARIABLE_NULL,
	VARIABLE_I2, 
	VARIABLE_I4, 
	VARIABLE_R4, 
	VARIABLE_R8, 
	VARIABLE_DATE,
	VARIABLE_BOOL,
	VARIABLE_VARIANT,
	VARIABLE_DECIMAL,
	VARIABLE_I1,
	VARIABLE_UI1,
	VARIABLE_UI2,
	VARIABLE_UI4,
	VARIABLE_I8,
	VARIABLE_UI8,
	VARIABLE_INT,
	VARIABLE_UINT,
	VARIABLE_VOID,
	VARIABLE_PTR,
	VARIABLE_LPSTR,
	VARIABLE_LPWSTR,
	VARIABLE_INT_PTR,
	VARIABLE_UINT_PTR,
	VARIABLE_ARRAY,
	VARIABLE_TYPEMASK 
} VARIABLE_TYPE_ENUM, *PVARIABLE_TYPE_ENUM; 

typedef unsigned short VARIABLE_TYPE;

#define __VARIABLE 
typedef struct _VARIABLE
{
	union 
	{
		struct __VARIABLE
		{
			VARIABLE_TYPE vt;
			WORD wReserved1;
			WORD wReserved2;
			WORD wReserved3;
			union 
			{
				LONGLONG llVal;
				LONG lVal;
				BYTE bVal;
				SHORT iVal;
				FLOAT fltVal;
				DOUBLE dblVal;
				VARIANT_BOOL boolVal;
				_VARIANT_BOOL bool;
				SCODE scode;
				DATE date;
				BYTE *pbVal;
				SHORT *piVal;
				LONG *plVal;
				LONGLONG *pllVal;
				FLOAT *pfltVal;
				DOUBLE *pdblVal;
				VARIANT_BOOL *pboolVal;
				_VARIANT_BOOL *pbool;
				DATE *pdate;
				PVOID byref;
				CHAR cVal;
				USHORT uiVal;
				ULONG ulVal;
				ULONGLONG ullVal;
				INT intVal;
				UINT uintVal;
				DECIMAL *pdecVal;
				CHAR *pcVal;
				USHORT *puiVal;
				ULONG *pulVal;
				ULONGLONG *pullVal;
				INT *pintVal;
				UINT *puintVal;
			};
		};
		//DECIMAL decVal;
	};
} VARIABLE, *PVARIABLE; 

typedef struct _VARIABLE_INFO 
{
	string name; 
	VARIABLE variable; 
} VARIABLE_INFO, *PVARIABLE_INFO; 

typedef map<string, VARIABLE_INFO*> VARIABLE_INFOS; 
typedef pair<string, VARIABLE_INFO*> VARIABLE_INFO_PAIR; 

#define MAX_ERROR_MESSAGE_LEN 260
typedef struct _HA_SCRIPT
{
	HANDLE lock; 
	vector< GRAMMER_ELEMENT_HEADER*> elements; 
#define MAX_OPERANDS 3
#define LEFT_OPERAND_INDEX 0
#define RIGHT_OPERAND_INDEX 1
#define MAX_OPERATORS 2
#define MAX_CONDITION_OPERATORS 1

	VARIABLE_INFOS variables; 
	VARIABLE_INFO *operands[ MAX_OPERANDS ]; 
	VARIABLE_INFO *temp_operands[ MAX_OPERANDS ]; 
	INT32 operators[ MAX_OPERATORS ]; 
	//INT32 condition_op[ MAX_CONDITION_OPERATORS ]; 
	//INT32 condition_output; 

	string error_message; 
	LRESULT error_code; 
} HA_SCRIPT, *PHA_SCRIPT; 

#endif //__HA_SCRIPT_COMMON_H__