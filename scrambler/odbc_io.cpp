#include "stdafx.h"
#include "Resource.h"
#include <afxdb.h>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "web_browser_window.h"
#include "mshtml_addition.h"
#include "data_scrambler.h"
#define XPATH_CREATE
#include "html_xpath.h"
#include "odbc_io.h"

CDatabase m_database;
CRecordset m_recordset; 

HRESULT WINAPI ado_connect( LPCWSTR conn )
{
	HRESULT hr = S_OK; 
	INT32 ret; 

	do 
	{
		try
		{
			ret = m_database.Open( conn ); 
			if( FALSE == ret )
			{
				CString error_text;
				error_text.Format(L"ado error");
				AfxMessageBox(error_text); 
				break; 
			}

			m_recordset.m_pDatabase = &m_database;
		}
		catch(_com_error &e)
		{
			CString error_text;
			error_text.Format(L"ado error: %s",( LPSTR )e.Description());
			AfxMessageBox(error_text); 
			hr = e.Error(); 
		}
	}while( FALSE ); 
	return hr; 
}

HRESULT WINAPI ado_disconnect()
{
	HRESULT hr = S_OK; 
	m_recordset.Close(); 
	m_database.Close(); 
	return hr; 
}


HRESULT WINAPI ado_exec( LPCWSTR sql )
{
	HRESULT hr = S_OK; 

	try
	{
		do 
		{
			if( FALSE == m_database.IsOpen() )
			{
				break; 
			}
			m_recordset.m_pDatabase= &m_database; 

			hr = m_recordset.Open( CRecordset::forwardOnly, sql ); 
		} while ( FALSE );
	}
	catch(_com_error &e)
	{
		CString error_text;
		error_text.Format(L"ado error: %s",( LPSTR )e.Description());
		AfxMessageBox(error_text); 
		hr = e.Error(); 
	}

	return hr; 
}