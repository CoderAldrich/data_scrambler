#ifndef __ODBC_IO_H__
#define __ODBC_IO_H__

#import "C:\program files\common files\System\ado\msado15.dll" no_namespace \
	rename("EOF","EndOfFile") \
	rename("LockTypeEnum","newLockTypeEnum")\
	rename("DataTypeEnum","newDataTypeEnum")\
	rename("FieldAttributeEnum","newFieldAttributeEnum")\
	rename("EditModeEnum","newEditModeEnum")\
	rename("RecordStatusEnum","newRecordStatusEnum")\
	rename("ParameterDirectionEnum","newParameterDirectionEnum")

#define TEST_SQL_DB_CONNECTION_STRING (L"DRIVER=SQL Server Native Client 10.0;SERVER=.\\SQLEXPRESS;Trusted_Connection=Yes;APP=Microsoft\x00ae Visual Studio\x00ae 2010;WSID=EGGEN2;DATABASE=DB01;");


HRESULT WINAPI ado_connect( LPCWSTR conn ); 
HRESULT WINAPI ado_disconnect(); 
HRESULT WINAPI ado_exec( LPCWSTR sql ); 

INLINE HRESULT check_ado_connection( LPCWSTR conn )
{
	HRESULT hr; 
	do 
	{
		hr = ado_connect( conn ); 
		if( FAILED( hr ))
		{
			break; 
		}

		ado_disconnect(); 
	} while ( FALSE ); 

	return hr; 
}

#endif //__ODBC_IO_H__
