/************************************************************************
	created:		2007-08-17
	updated:		2007-12-23
	filename:		SQLite.h
	author:			Daniel C. Gindi (danielgindi (at) gmail dot com)

	purpose:		Wrapper for SQLite3!

					There are 2 "public" classes in here,
					  under the namspace SQLite:

					Database	: Which is the main wrapper for an SQLite3
									database.

					Table		: Which is representing a table returned
									from a query on CSQLite

					I'm pretty sure that my classes fully support Unicode.

	Legal notes:	You are free you use these classes for whatever use 
					  you have in mind, even commercial,
					On one condition, don't ever claim you wrote it.
					And if you wanna give me credits, I would like that... :-)

	Contact notes:	I can be contacted at (danielgindi (at) gmail dot com)
					If you just wanna say thank you, or better, if you
					  think there's room for improvement for these
					  classes...
*************************************************************************/

#include <windows.h>
//#include "StdAfx.h"
#include <tchar.h>
#include <Strsafe.h>
#include "SQLite.h"

#import "MsXml.Tlb"
#include <regex>
#include <string>

using namespace std;
using namespace std::tr1;

using namespace MSXML;
#pragma  warning(disable:4995)
#pragma  warning(disable:4996)
namespace SQLite
{
	Database::Database(void)
	{
		m_sqlite3=0;
		m_iLastError=SQLITE_OK;
	}

	Database::~Database(void)
	{
		Close();
	}

	int Database::Open( LPCTSTR strFileName )
	{
		Close();

		int iErr=sqlite3_open(UTF8MBSTR(strFileName), &m_sqlite3);

		if (iErr!=SQLITE_OK) m_iLastError=iErr;

		return iErr;
	}

	void Database::Close()
	{
		if (m_sqlite3)
		{
			sqlite3_close(m_sqlite3);
			m_sqlite3=0;
		}
	}

	bool Database::IsOpen()
	{
		if (this  == 0) return false; // check for valid Database* pointer
		return m_sqlite3!=0; // check for open database
	}

	Table Database::QuerySQL( LPCTSTR strSQL )
	{
		if (!IsOpen()) {
			m_iLastError=SQLITE_ERROR;
			return Table();
		}

		// I do not need to imlpement the whole sqlite3_get_table
		//   with callbacks and all myself, because it won't spare me anything,
		// I'm converting the returned strings to TCHAR anyway, 
		//   so I'm not really copying the strings into the Table class

		char ** retStrings = 0;
		char * errmsg = 0;
		int iRows=0, iCols=0;

		int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
			&iRows, &iCols, &errmsg);

		if (iErr!=SQLITE_OK)
		{
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;

		}

#ifdef DEBUG
		if (errmsg) {

			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
			delete []strOut;

		
			
		}
#endif
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		if(errmsg)
		{
			sqlite3_free(errmsg);
		}

		Table retTable;

		if (iRows>0) retTable.m_iPos=0; // Put retTable's cursor on first row

		retTable.m_iCols=iCols;
		retTable.m_iRows=iRows;

		// allocate the memory for the header
		//retTable.m_strlstCols.reserve(iCols);

		int iPos=0;

		for (; iPos<iCols; iPos++)
		{
			// Add a string element
			//retTable.m_strlstCols.push_back(stdvstring());

			// Convert the UTF8 to TCHAR string
			if (retStrings[iPos] && strlen(retStrings[iPos]))
			{
				LPTSTR strOut=NULL;
				ConvertUTF8ToString( retStrings[iPos], strlen(retStrings[iPos]), strOut);
				retTable.m_strlstCols.Add(strOut);
				delete[] strOut;
			}
			else 
			{
				//retTable.m_strlstCols.back().push_back(_T('\0'));
				retTable.m_strlstCols.Add(_T(""));
			}
		}

		//retTable.m_lstRows.resize(iRows);
		for (int iRow=0; iRow<iRows; iRow++)
		{
			//retTable.m_lstRows[iRow].reserve(iCols);
			row r;
			
			for (int iCol=0; iCol<iCols; iCol++)
			{
				// Add a string element
				//retTable.m_lstRows[iRow].push_back(stdvstring());
					
				// Convert the UTF8 to TCHAR string
				if (retStrings[iPos] && strlen(retStrings[iPos]))
				{
					LPTSTR strOut;
					ConvertUTF8ToString( retStrings[iPos],strlen(retStrings[iPos]),strOut );
					r.Add(strOut);
					delete[] strOut;
				}
				else {
					r.Add(_T(""));
				}

				iPos++;
			}
			retTable.m_lstRows.Add(r);
		}

		// sqlite3_free_table() also checks for null pointer...
		sqlite3_free_table(retStrings);

		return retTable;
	}

	TablePtr Database::QuerySQL2( LPCTSTR strSQL )
	{
		if (!IsOpen()) {
			m_iLastError=SQLITE_ERROR;
			return 0;
		}

		// I do not need to imlpement the whole sqlite3_get_table
		//   with callbacks and all myself, because it won't spare me anything,
		// I'm converting the returned strings to TCHAR anyway, 
		//   so I'm not really copying the strings into the Table class

		char ** retStrings = 0;
		char * errmsg = 0;
		int iRows=0, iCols=0;

		int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
			&iRows, &iCols, &errmsg);

		if (iErr!=SQLITE_OK) {
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;

		}

#ifdef DEBUG
		if (errmsg) {
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
			delete []strOut;
		}
#endif	
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		if(errmsg)
		{
			sqlite3_free(errmsg);
		}

		Table * retTable = new Table();

		if (iRows>0) retTable->m_iPos=0; // Put retTable's cursor on first row

		retTable->m_iCols=iCols;
		retTable->m_iRows=iRows;

		// allocate the memory for the header
		//retTable->m_strlstCols.reserve(iCols);

		int iPos=0;

		for (; iPos<iCols; iPos++)
		{
			// Add a string element
			//retTable->m_strlstCols.push_back(stdvstring());

			// Convert the UTF8 to TCHAR string
			if (retStrings[iPos] && strlen(retStrings[iPos]))
			{
				LPTSTR strOut;
				ConvertUTF8ToString( retStrings[iPos], strlen(retStrings[iPos]),strOut );
				retTable->m_strlstCols.Add(strOut);
				delete[] strOut;
			}
			else
			{
				retTable->m_strlstCols.Add(_T(""));
			}
		}

		//retTable->m_lstRows.resize(iRows);
		row r;
		for (int iRow=0; iRow<iRows; iRow++)
		{
			//retTable->m_lstRows[iRow].reserve(iCols);
			for (int iCol=0; iCol<iCols; iCol++)
			{
				// Add a string element
				//retTable->m_lstRows[iRow].push_back(stdvstring());

				// Convert the UTF8 to TCHAR string
				if (retStrings[iPos] && strlen(retStrings[iPos]))
				{
					LPTSTR strOut;
					ConvertUTF8ToString( retStrings[iPos], strlen(retStrings[iPos]),strOut);
					r.Add(strOut);
					delete[] strOut;
				}
				else
				{
					r.Add(_T(""));
				}

				iPos++;
			}
			retTable->m_lstRows.Add(r);
		}

		// sqlite3_free_table() also checks for null pointer...
		sqlite3_free_table(retStrings);

		return TablePtr(retTable);
	}

	//strOut ->需要释放

	ULONG Database::QuerySQL3(LPCTSTR strSQL,pfnSQLite3QueryCallback fn)
	{
		if (!IsOpen()) {
			m_iLastError=SQLITE_ERROR;
			return 0;
		}

		// I do not need to imlpement the whole sqlite3_get_table
		//   with callbacks and all myself, because it won't spare me anything,
		// I'm converting the returned strings to TCHAR anyway, 
		//   so I'm not really copying the strings into the Table class

		char ** retStrings = 0;
		char * errmsg = 0;
		int iRows=0, iCols=0;

		ULONG ulCount = 0;
		int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
			&iRows, &iCols, &errmsg);

		if (iErr!=SQLITE_OK) {
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;

		}

#ifdef DEBUG
		if (errmsg) {
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
			delete []strOut;
		}
#endif	
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		if(errmsg)
		{
			sqlite3_free(errmsg);
		}

		if(fn && retStrings && iRows && iCols )
		{
			ulCount = fn((PVOID)this,(const char**)retStrings,iRows,iCols);
		}

		
		// sqlite3_free_table() also checks for null pointer...
		sqlite3_free_table(retStrings);

		return ulCount;
	}
	void Database::ConvertUTF8ToString( char * strInUTF8MB, size_t len, LPTSTR & strOut )
	{
		strOut=new TCHAR[len+1];
		memset(strOut,0,len+1);

#ifdef UNICODE
		//MultiByteToWideChar(CP_UTF8, 0, strInUTF8MB, (int)len, strOut, (int)len);
		CStringW wstrBuffer = CA2W(strInUTF8MB,CP_UTF8);
		//wcscpy(strOut,wstrBuffer);

		StringCbCopyW(strOut,(len+1)*sizeof(TCHAR),wstrBuffer);
#else
		CStringW wstrBuffer = CA2W(strInUTF8MB,CP_UTF8);
		StringCbCopyA(strOut,(len+1)*sizeof(TCHAR),CW2A(wstrBuffer));

#endif
		
	}
	void Database::ConvertUTF8ToString( char * strInUTF8MB, CString & strOut )
	{

#ifdef UNICODE
		CStringW wstrBuffer = CA2W(strInUTF8MB,CP_UTF8);
		strOut = wstrBuffer;

#else
		CStringW wstrBuffer = CA2W(strInUTF8MB,CP_UTF8);
		strOut = CW2T(wstrBuffer);
#endif

	
	}

	int Database::ExecuteSQL( LPCTSTR strSQL )
	{
		if (!IsOpen()) {
			m_iLastError=SQLITE_ERROR;
			return SQLITE_ERROR;
		}

		char * errmsg = 0;

		int iErr = sqlite3_exec( m_sqlite3, UTF8MBSTR(strSQL), 0, 0, &errmsg );

		if (iErr!=SQLITE_OK) 
		{
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;
		}

#ifdef DEBUG
		if (errmsg) {
		
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("ExecuteSQL error: %s\r\n"), strOut);
			delete []strOut;
		}
#endif	
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		sqlite3_free(errmsg);

		return iErr;
	}

	int Database::IsSQLComplete( LPCTSTR strSQL )
	{
		return sqlite3_complete( UTF8MBSTR(strSQL) );
	}

	int Database::GetLastChangesCount()
	{
		return sqlite3_changes(m_sqlite3);
	}

	int Database::ReadBlob( LPCTSTR strSQL, BYTE ** pData, int * iLenBlob )
	{
		if (!IsOpen() || !pData) {
			m_iLastError=SQLITE_ERROR;
			return SQLITE_ERROR;
		}

		sqlite3_stmt *pStmt;

		int iErr = SQLITE_OK;

		// In case  an error occurs, set *pData and *iLenBlob to 0 now.
		*pData = 0;
		*iLenBlob = 0;

		do {
			// Compile the SELECT statement into a compiled statement.
			iErr = sqlite3_prepare(m_sqlite3, UTF8MBSTR(strSQL), -1, &pStmt, 0);
			if( iErr!=SQLITE_OK ){
				m_iLastError=iErr;
				return iErr;
			}

			// Run the compiled statement.
			// We expect only 1 record, so we are only looking for SQLITE_ROW.
			iErr = sqlite3_step(pStmt);
			if( iErr  == SQLITE_ROW ){
				// The pointer returned by sqlite3_column_blob() points to memory
				//   that is owned by the statement handle (pStmt).
				// It is only good until the next call to an sqlite3_XXX()
				//   function (e.g. the sqlite3_finalize() below) that involves 
				//   the statement handle. 
				// So we need to make a copy of the blob into memory obtained from 
				//   malloc() to return to the caller.
				*iLenBlob = sqlite3_column_bytes(pStmt, 0);
				*pData = (unsigned char *)malloc(*iLenBlob);
				memcpy(*pData, sqlite3_column_blob(pStmt, 0), *iLenBlob);
			}

			// Finalize the statement ( this releases resources allocated by 
			//   sqlite3_prepare() ).
			iErr = sqlite3_finalize(pStmt);

			// If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
			//   the statement all over again.
			// ( SQLITE_SCHEMA means that something in the structure of the
			//   table has changed, so the statement was not valid anymore )
		} while( iErr  == SQLITE_SCHEMA );

		return iErr;
	}

	int Database::WriteBlob( LPCTSTR strSQL, BYTE * pData, int iLenBlob )
	{
		if (!IsOpen()) {
			m_iLastError=SQLITE_ERROR;
			return SQLITE_ERROR;
		}

		sqlite3_stmt *pStmt;

		int iErr=SQLITE_OK;

		do {
			// Compile the SQL statement into a compiled statement.
			iErr = sqlite3_prepare(m_sqlite3, UTF8MBSTR(strSQL), -1, &pStmt, 0);
			if( iErr!=SQLITE_OK ){
				m_iLastError=iErr;
				return iErr;
			}

			// Bind the pData to the compiled statement 
			//   (the ? characters in the sql statement)
			sqlite3_bind_blob(pStmt, 1, pData, iLenBlob, SQLITE_STATIC);

			// Call sqlite3_step() to run the compiled statement. 
			sqlite3_step(pStmt);
			//ASSERT( iErr!=SQLITE_ROW );

			// Finalize the statement ( this releases resources allocated by 
			//   sqlite3_prepare() ).
			iErr = sqlite3_finalize(pStmt);

			// If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
			//   the statement all over again.
			// ( SQLITE_SCHEMA means that something in the structure of the
			//   table has changed, so the statement was not valid anymore )
		} while( iErr  == SQLITE_SCHEMA );

		return iErr;
	}

	sqlite_int64 Database::GetLastInsertRowID()
	{
		if (m_sqlite3  == 0) return 0; // RowID's starts with 1...

		return sqlite3_last_insert_rowid(m_sqlite3);
	}

	bool Database::BeginTransaction()
	{
		if (!IsOpen())
		{
			m_iLastError=SQLITE_ERROR; 
			return false;
		}
		m_iLastError = ExecuteSQL(_T("BEGIN TRANSACTION"));
		return m_iLastError  == SQLITE_OK;
	}

	bool Database::CommitTransaction()
	{
		if (!IsOpen()) 
		{
			m_iLastError=SQLITE_ERROR; 
			return false;
		}
		m_iLastError = ExecuteSQL(_T("COMMIT TRANSACTION"));
		return m_iLastError  == SQLITE_OK;
	}

	bool Database::RollbackTransaction()
	{
		if (!IsOpen()) 
		{
			m_iLastError=SQLITE_ERROR;
			return false;
		}
		m_iLastError = ExecuteSQL(_T("ROLLBACK TRANSACTION"));
		return m_iLastError  == SQLITE_OK;
	}
	BOOL Database::IsTableExistd(const TCHAR* tszTableName )
	{
		if (  tszTableName==NULL)
		{

			AtlTrace ("invalid parameter ");
			
			return FALSE;
		}

		if ( tszTableName[0] ==0 )
		{
			AtlTrace ("  invalid parameter ");
			
			return FALSE;
		}


		TCHAR tszSQLStatement[256] = {0};
		StringCbPrintf (tszSQLStatement,sizeof (tszSQLStatement),_T("select count(*) from sqlite_master where type = \'table\' and name = \'%s\'"),tszTableName);

		Table table = QuerySQL (tszSQLStatement);
		if (table.GetRowCount ()>0)
		{
			table.GoFirst ();
			if (CString (table.GetValue (0)).CompareNoCase (_T("1")) == 0)
			{
				return TRUE;
			}

			return FALSE;
		}
		else 
			return FALSE;
	}
    BOOL Database::IsTableColumnExistd(const TCHAR* tszTableName,  const TCHAR* tszFieldName )
    {
        if (tszTableName==NULL ||tszTableName[0] == 0)
        {

            AtlTrace ("invalid parameter ");

            return FALSE;
        }

        if (tszFieldName == NULL ||tszFieldName[0] == 0)
        {
            AtlTrace ("  invalid parameter ");

            return FALSE;
        }


        TCHAR tszSQLStatement[256] = {0};
        StringCbPrintf (tszSQLStatement,sizeof (tszSQLStatement),_T("select sql from sqlite_master where tbl_name=\'%s\' and  type=\'table\'"),tszTableName);

        Table table = QuerySQL (tszSQLStatement);
        if (table.GetRowCount ()>0)
        {
            table.GoFirst ();
            CString strSQL = table.GetValue (_T("sql"));
            strSQL.MakeLower();
            
            TCHAR tszFieldNameNew[256] ={0};
            StringCbCopy(tszFieldNameNew,sizeof(tszFieldNameNew),tszFieldName);
            _tcslwr(tszFieldNameNew);
            if (strSQL.Find(tszFieldNameNew) != -1)
            {
                return TRUE;
            }

            return FALSE;
        }
        else 
            return FALSE;
    }

	BOOL Database::InsertTableRecord(const char * strTableName,Table table,pfnRecordCallback fnRecordCallBack)
	{
		CString sql;

        if(fnRecordCallBack)
        {
            fnRecordCallBack(strTableName,table);
        }
	

		BeginTransaction();
		for (int i = 0;i<table.m_lstRows.GetSize();i++)
		{

			sql = _T("insert into ");
			sql += CA2CT(strTableName);
			sql += _T(" (");
			for(int j=0;j<table.m_strlstCols.GetSize();j++)
			{
				if(j>0)
				{
					sql += _T(",");
				}
				sql += table.m_strlstCols[j];
			}

			sql += _T(") values (");

			row r = table.m_lstRows[i];
			for(int k=0;k<table.m_strlstCols.GetSize();k++)
			{
				if(k>0)
				{
					sql += _T(",");
				}
				sql += _T("\'");
				CString strTemp = r[k];
				strTemp.Replace(_T("\'"),_T("\'\'"));

                CString strUtf8Out;
                ConvertUTF8ToString(CT2A(strTemp.GetBuffer(NULL)),strUtf8Out);
				sql += strUtf8Out;
				sql += _T("\'");
			}

			sql += _T(")");

		
			if(SQLITE_OK!=ExecuteSQL(sql))
			{
				
#ifndef UNICODE
				AtlTrace (" %s error:  ",sql);
#else
				AtlTrace ("%s error",sql);
#endif

				continue;
			}


		}
		BOOL bCommit = CommitTransaction();
		if(!bCommit)
		{
			
#ifndef UNICODE
			AtlTrace (" %s error:  ",sql);
#else
			AtlTrace ("%s error",sql);
#endif


		}



		return bCommit;
	
	}
    BOOL Database::ImportTableAsBinary(LPSTR strTableName,LPSTR strFileName,pfnRecordCallback fnRecordCallBack)
    {

        BOOL bResult = FALSE;
        if (!IsOpen()) {
            ATLTRACE(_T("ExportTableAsCSV database is not opened\r\n"));
            m_iLastError=SQLITE_ERROR;
            return FALSE;
        }
        if(!IsTableExistd(CA2CT(strTableName)))
            return FALSE;
        DWORD dwAttribute=GetFileAttributesA(strFileName);

        if( dwAttribute== INVALID_FILE_ATTRIBUTES || (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)== FILE_ATTRIBUTE_DIRECTORY)
            return FALSE;

        HANDLE hFile = CreateFileA(strFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if( hFile != INVALID_HANDLE_VALUE )
        {
            char *buf=NULL;
            DWORD dwFileSizeHigh;
            BOOL  bMapView = FALSE;
            int dwFileSize = GetFileSize(hFile,&dwFileSizeHigh);
            HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,0,0,0);
            if(hFileMapOfView)
            {

                buf=(char *)MapViewOfFile(hFileMapOfView,FILE_MAP_READ,0,0,0);
                if(buf)
                {
                    try
                    {  
                       
                        int iRows = *(ULONG*)buf;
                        int iCols = *(ULONG*)(buf+4);
                        Table TempTable;
                        TempTable.m_iCols = iCols;
                        
                        char* szRecord = buf +8;
                        ULONG ulFieldNameLen = 0;
                        char* szTemp = NULL;
                        for (int i=0;i<iCols;i++)
                        {
                            ulFieldNameLen =*(ULONG*) szRecord;

                            char szFieldName[256]={0};
                            StringCbCopyNA(szFieldName,sizeof(szFieldName),szRecord+4,ulFieldNameLen);
                            TempTable.m_strlstCols.Add(szFieldName);
                            szRecord += ulFieldNameLen+4;
                          


                        }
                        
                        ULONG ulFieldValueLen = 0;        
                        for (int i=1;i<=iRows;i++)
                        {
                            row r;
                            for (int j=0;j<iCols;j++)
                            {
                                ulFieldValueLen =*(ULONG*) szRecord;

                                char szFieldValue[1024];
                                StringCbCopyNA(szFieldValue,sizeof(szFieldValue),szRecord+4,ulFieldValueLen);

                                szRecord += ulFieldValueLen+4;
                                r.Add(szFieldValue);
                                
                            }
                            TempTable.m_lstRows.Add(r);
                            if(TempTable.m_lstRows.GetSize()>=100)
                            {
                                TempTable.m_iRows = TempTable.m_lstRows.GetSize();
                                InsertTableRecord(strTableName,TempTable,fnRecordCallBack);

                                TempTable.m_lstRows.RemoveAll();
                                TempTable.m_iRows = 0;
                            }

                        }

                        TempTable.m_iRows = TempTable.m_lstRows.GetSize();
                        InsertTableRecord(strTableName,TempTable,fnRecordCallBack);

                        TempTable.m_lstRows.RemoveAll();
                        TempTable.m_iRows = 0;
              
                    }

                    catch (...)
                    {
                    }
                    UnmapViewOfFile(buf);


                }

                CloseHandle(hFileMapOfView);
            }

            CloseHandle(hFile);

        }

        else
        {


            AtlTrace("Error:32 另一个程序正在使用此文件，进程无法访问");

        }
        return bResult;
    }

    BOOL Database::ExportTableAsMemory(LPSTR strTableName,ULONG * pulRowCount,ULONG* pulColCount,CHAR*pszBuffer,ULONG* pulBufferLen,LPSTR strCond)
    {
        BOOL bResult = FALSE;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        if (!IsOpen()) {
            ATLTRACE(_T("ExportTableAsCSV database is not opened\r\n"));
            m_iLastError=SQLITE_ERROR;
            return FALSE;
        }
        if(!IsTableExistd(CA2CT(strTableName)))
            return FALSE;
        // I do not need to imlpement the whole sqlite3_get_table
        //   with callbacks and all myself, because it won't spare me anything,
        // I'm converting the returned strings to TCHAR anyway, 
        //   so I'm not really copying the strings into the Table class

        char ** retStrings = 0;
        char * errmsg = 0;
        int iRows=0, iCols=0;

        DWORD dwWritten = 0;

        TCHAR strSQL[256];
        StringCbCopy(strSQL,sizeof(strSQL),_T("Select * from "));
        StringCbCat(strSQL,sizeof(strSQL),CA2CT(strTableName));
        if(strCond && strlen(strCond))
        {
            StringCbCat(strSQL,sizeof(strSQL),_T(" where "));
            StringCbCat(strSQL,sizeof(strSQL),CA2CT(strCond));
        }

        int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
            &iRows, &iCols, &errmsg);

        if (iErr!=SQLITE_OK)
        {
            m_iLastError=iErr;
            LPTSTR strOut=NULL; 
            ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
            m_strLastError = strOut;
            delete []strOut;
            ATLTRACE(_T("ExportTableAsCSV execute sql error:%s\r\n"),strOut);

        }

#ifdef DEBUG
        if (errmsg) {

            m_iLastError=iErr;
            LPTSTR strOut=NULL; 
            ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
            ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
            delete []strOut;
        }
#endif
        // No need for string errors, 
        //   too costly is the comparision of strings.
        //
        // BTW: sqlite3_free() already checks for null pointer,
        //		  so I do not do double checking.
        //		And I can assume this for later versions, because
        //		  that is generally the point of sqlite3_free()...
        if(errmsg)
        {
            sqlite3_free(errmsg);

        }
        if(iRows)
        {
            

        
          

            if(pulRowCount)
            {
                *pulRowCount = iRows;
            }
            if(pulColCount)
            {
                *pulColCount = iCols;
            }
        
            int i= 0;
            int j= 0; 
            if(retStrings)
            {
                ULONG ulMemoryLen = 0;
              
                for (i=0;i<=iRows;i++)
                {
                    for (j=0;j<iCols;j++)
                    {
                        char* szUtf8String = retStrings[i*iCols+j];
                        if (szUtf8String == NULL)
                        {
                            ulMemoryLen += 4;
                            continue;
                        }
                       
                        LPTSTR strOut=NULL; 
                        ConvertUTF8ToString(szUtf8String,strlen(szUtf8String), strOut);
                        memcpy(&pszBuffer[ulMemoryLen],CT2A(strOut),(ULONG)_tcslen(strOut) ); 
                        ulMemoryLen += (ULONG)_tcslen(strOut) + 1;
                        delete []strOut;

                       
                       
                       
                    }
                }
                ulMemoryLen++;
                if(*pulBufferLen >=ulMemoryLen)
                {
                  
                   
                    *pulBufferLen = ulMemoryLen;
                    bResult = TRUE;

                }
              
                
            }
        }
        else
        {
            if(pulRowCount)
            {
                *pulRowCount = 0;
            }
            if(pulColCount)
            {
                *pulColCount = 0;
            }
            if(pulBufferLen)
            {
                *pulBufferLen = 0;
            }

         }

        if(retStrings)
        {
            sqlite3_free_table(retStrings);
        }

        return bResult;
    }

    BOOL Database::ExportTableAsBinary(LPSTR strTableName,LPSTR strFileName,LPSTR strCond)
    {
        BOOL bResult = FALSE;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        if (!IsOpen()) {
            ATLTRACE(_T("ExportTableAsCSV database is not opened\r\n"));
            m_iLastError=SQLITE_ERROR;
            return FALSE;
        }
        if(!IsTableExistd(CA2CT(strTableName)))
            return FALSE;
        // I do not need to imlpement the whole sqlite3_get_table
        //   with callbacks and all myself, because it won't spare me anything,
        // I'm converting the returned strings to TCHAR anyway, 
        //   so I'm not really copying the strings into the Table class

        char ** retStrings = 0;
        char * errmsg = 0;
        int iRows=0, iCols=0;

        DWORD dwWritten = 0;

        TCHAR strSQL[256];
        StringCbCopy(strSQL,sizeof(strSQL),_T("Select * from "));
        StringCbCat(strSQL,sizeof(strSQL),CA2CT(strTableName));
        if(strCond && strlen(strCond))
        {
            StringCbCat(strSQL,sizeof(strSQL),_T(" where "));
            StringCbCat(strSQL,sizeof(strSQL),CA2CT(strCond));
        }

        int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
            &iRows, &iCols, &errmsg);

        if (iErr!=SQLITE_OK)
        {
            m_iLastError=iErr;
            LPTSTR strOut=NULL; 
            ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
            m_strLastError = strOut;
            delete []strOut;
            ATLTRACE(_T("ExportTableAsCSV execute sql error:%s\r\n"),strOut);

        }

#ifdef DEBUG
        if (errmsg) {

            m_iLastError=iErr;
            LPTSTR strOut=NULL; 
            ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
            ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
            delete []strOut;
        }
#endif
        // No need for string errors, 
        //   too costly is the comparision of strings.
        //
        // BTW: sqlite3_free() already checks for null pointer,
        //		  so I do not do double checking.
        //		And I can assume this for later versions, because
        //		  that is generally the point of sqlite3_free()...
        if(errmsg)
        {
            sqlite3_free(errmsg);

        }
        if(iCols)
        {
            bResult = TRUE;

            DWORD dwAttributes = GetFileAttributesA(strFileName);
            if(dwAttributes != INVALID_FILE_ATTRIBUTES)
            {
                OutputDebugStringA(strFileName);
                OutputDebugStringA((" has been existed,will be overwrite\r\n"));
                DeleteFileA(strFileName);

            }
            hFile = CreateFileA (strFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {

                WriteFile(hFile,&iRows,sizeof(iRows),&dwWritten,NULL);
                WriteFile(hFile,&iCols,sizeof(iCols),&dwWritten,NULL);
                
                int i= 0;
                int j= 0; 
               if(retStrings)
               {
                for (i=0;i<=iRows;i++)
                {
                    for (j=0;j<iCols;j++)
                    {
                        char* szUtf8String = retStrings[i*iCols+j];
                        if (szUtf8String == NULL)
                        {
                            DWORD dwUtf8Len = 0;
                            WriteFile(hFile,&dwUtf8Len,sizeof(dwUtf8Len),&dwWritten,NULL);
                            continue;
                        }
                        int nUtf8Len = (int)strlen(szUtf8String);
                        WriteFile(hFile,&nUtf8Len,sizeof(nUtf8Len),&dwWritten,NULL);
                        WriteFile(hFile,szUtf8String,nUtf8Len,&dwWritten,NULL);

                    }
                }
               }

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(hFile);
                }

            }	
           



        }
  
        if(retStrings)
        {
            sqlite3_free_table(retStrings);
        }

        return bResult;
    }
	BOOL Database::ImportTableAsCSV(LPSTR strTableName,LPSTR strFileName,pfnRecordCallback fnRecordCallBack)
	{
		BOOL bResult = FALSE;
		if (!IsOpen()) {
			ATLTRACE(_T("ExportTableAsCSV database is not opened\r\n"));
			m_iLastError=SQLITE_ERROR;
			return FALSE;
		}
		if(!IsTableExistd(CA2CT(strTableName)))
			return FALSE;
		DWORD dwAttribute=GetFileAttributesA(strFileName);

		if( dwAttribute== INVALID_FILE_ATTRIBUTES || (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)== FILE_ATTRIBUTE_DIRECTORY)
			return FALSE;

		HANDLE hFile = CreateFileA(strFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if( hFile != INVALID_HANDLE_VALUE )
		{
			char *buf=NULL;
			DWORD dwFileSizeHigh;
			BOOL  bMapView = FALSE;
			int dwFileSize = GetFileSize(hFile,&dwFileSizeHigh);
			HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,0,0,0);
			if(hFileMapOfView)
			{

				buf=(char *)MapViewOfFile(hFileMapOfView,FILE_MAP_READ,0,0,0);
                if(buf)
                {
                   try
                   {     char* szLineBegin = buf;
			            CHAR * szEnd = strstr(szLineBegin,"\r\n");
			            while(szEnd)
			            {
				            MyString myData[256]={0};
				            if(split_string(szLineBegin,(int)(szEnd-szLineBegin),",",myData,sizeof(myData)/sizeof(MyString))==3)
				            {
					            //检查第一行，要求出现tablename,row_count,col_count
					            if(strnicmp(myData[0].szStart,"tablename",myData[0].nLen)!=0)
					            {
						            AtlTrace("Invalid CSV Format,Expect tablename occur\r\n");
						            break;
					            }
					            if(strnicmp(myData[1].szStart,"row_count",myData[1].nLen)!=0)
					            {
						            AtlTrace("Invalid CSV Format. Expect row_count occur\r\n");
						            break;
					            }
					            if(strnicmp(myData[2].szStart,"col_count",myData[2].nLen)!=0)
					            {
						            AtlTrace("Invalid CSV Format. Expect col_count occur\r\n");
						            break;
					            }
					            //检查第2行，要求出现tablename,row_count,col_count对应的内容
					            szLineBegin = szEnd+2;//szLineBegin指向第2行行首
					            szEnd = strstr(szLineBegin,"\r\n");
					            if(!szEnd)
					            {
						            break;
					            }

					            if(split_string(szLineBegin,(int)(szEnd-szLineBegin),",",myData,sizeof(myData)/sizeof(MyString))==3)
					            {
						            if(strnicmp(myData[0].szStart,strTableName,myData[0].nLen)!=0)
						            {
							            AtlTrace("Invalid CSV Format,Expect tablename error\r\n");
							            break;
						            }
						            char szRowCount [30] = {0};
						            char szColCount [30] = {0};

						            StringCbCopyNA(szRowCount,sizeof(szRowCount),myData[1].szStart,myData[1].nLen);
						            StringCbCopyNA(szColCount,sizeof(szColCount),myData[2].szStart,myData[2].nLen);
						            basic_regex<char> regex("[0-9]+",basic_regex<char>::icase);
						            if(!regex_match(szRowCount, regex))
						            {
							            AtlTrace("Invalid CSV Format,row_count value error\r\n");
							            break;
						            }
						            if(!regex_match(szColCount, regex))
						            {
							            AtlTrace("Invalid CSV Format,col_count value error\r\n");
							            break;
						            }
						            int nRecordCount = atoi(szRowCount);
						            int nFieldCount = atoi(szColCount);

						            szLineBegin = szEnd+2;//szLineBegin指向第2行行首
						            szEnd = strstr(szLineBegin,"\r\n");
						            if(!szEnd)
						            {
							            break;
						            }
						            Table TempTable;
						            TempTable.m_iCols = nFieldCount;
						            //处理字段名称，字段名不应出现","，字符这里忽略对双引号的处理
						            if(split_string(szLineBegin,(int)(szEnd-szLineBegin),",",myData,sizeof(myData)/sizeof(MyString))==nFieldCount)
						            {
							            char szFieldName[256];
							            for(int i=0;i<nFieldCount;i++)
							            {
								            StringCbCopyNA(szFieldName,sizeof(szFieldName),myData[i].szStart,myData[i].nLen);
								            TempTable.m_strlstCols.Add(szFieldName);
							            }

						            }
						            else
						            {
							            AtlTrace("Invalid CSV Format,field count  dismatch\r\n");
							            break;
						            }
						            bResult = TRUE;
						            //循环读取字段列表值，直到处理结束
						            while(szLineBegin < buf+dwFileSize || szEnd < buf+dwFileSize)
						            {
							            szLineBegin = szEnd+2;//szLineBegin指向第2行行首
							            szEnd = strstr(szLineBegin,"\r\n");
							            if(!szEnd)
							            {
								            break;
							            }
							            if(split_string(szLineBegin,(int)(szEnd-szLineBegin),",",myData,sizeof(myData)/sizeof(MyString))==nFieldCount)
							            {
								            char szFieldValue[512];
								            row r;
								            for(int i=0;i<nFieldCount;i++)
								            {
									            StringCbCopyNA(szFieldValue,sizeof(szFieldValue),myData[i].szStart,myData[i].nLen);
									            r.Add(szFieldValue);
								            }
								            TempTable.m_lstRows.Add(r);
								            if(TempTable.m_lstRows.GetSize()>=100)
								            {
									            TempTable.m_iRows = TempTable.m_lstRows.GetSize();
									            InsertTableRecord(strTableName,TempTable,fnRecordCallBack);

									            TempTable.m_lstRows.RemoveAll();
									            TempTable.m_iRows = 0;
								            }

							            }
							            else
							            {
								            AtlTrace("Invalid CSV Format,field count  dismatch\r\n");
								            break;
							            }


						            }

						            TempTable.m_iRows = TempTable.m_lstRows.GetSize();
						            InsertTableRecord(strTableName,TempTable,fnRecordCallBack);

						            TempTable.m_lstRows.RemoveAll();
						            TempTable.m_iRows = 0;

						            break;

					            }


					            else
					            {
						            AtlTrace("Invalid CSV Format\r\n");
					            }
        					
        						
				            }
				            else
				            {
					            AtlTrace("Invalid CSV Format\r\n");
				            }
				            break;
			            }

                   }
                
                   catch (...)
                   {
                   }
                   UnmapViewOfFile(buf);
               
				
                }
				
				CloseHandle(hFileMapOfView);
			}

			CloseHandle(hFile);

		}

		else
		{
			
		
			AtlTrace("Error:32 另一个程序正在使用此文件，进程无法访问");

		}
		return bResult;
	}
	BOOL Database::RealImportTableAsXML(LPSTR strTableName,LPSTR strFileName,pfnRecordCallback fnRecordCallBack)
	{
		BOOL bResult = FALSE;
		HRESULT                          hr;
		MSXML::IXMLDOMDocumentPtr        XMLDoc = NULL;
		MSXML::IXMLDOMElementPtr         XMLElement = NULL;
		MSXML::IXMLDOMNodePtr            Node = NULL;

		MSXML::IXMLDOMNamedNodeMapPtr    NodeMap = NULL;
		MSXML::IXMLDOMNodeListPtr        NodeList = NULL,CurNodeList = NULL;

		VARIANT_BOOL                     vBoolSuccess;

		int iRows ,iCols;
		hr = CoCreateInstance (__uuidof (MSXML::DOMDocument),NULL,CLSCTX_INPROC_SERVER,__uuidof (MSXML::IXMLDOMDocument),(void**)&XMLDoc);
		if (S_OK == hr)
		{
			vBoolSuccess = XMLDoc->load (strFileName);
			if (vBoolSuccess == VARIANT_TRUE)
			{
				XMLElement =  XMLDoc->documentElement ;
				if (XMLElement != NULL)
				{
					Node = XMLElement->selectSingleNode (L"//table");
					if (Node )
					{
						NodeMap = Node->attributes;
						if (NodeMap )
						{
							Node = NodeMap->getNamedItem (L"name");
							if (Node)
							{
								if(_strcmpi(Node->text.operator const char*(), strTableName)!=0)
								{
									AtlTrace("invalid parameter,tablename dismatched\r\n");
									return FALSE;
								}
							}
							Node = NodeMap->getNamedItem (L"row_count");
							if (Node)
							{
								iRows = atoi(Node->text.operator const char*());
							}
							Node = NodeMap->getNamedItem (L"col_count");
							if (Node)
							{
								iCols = atoi(Node->text.operator const char*());
							}
						}
						Table retTable;
						retTable.m_iCols=iCols;
						retTable.m_iRows=iRows;

						// allocate the memory for the header
						//retTable.m_strlstCols.reserve(iCols);

						MSXML::IXMLDOMNodeListPtr RowNodeList = XMLElement->selectNodes (L"//table/row");
						if(RowNodeList)
						{
							for (int i = 0; i < RowNodeList->Getlength();++i) 
							{ 
								Node = RowNodeList->Getitem(i);
								if(Node)
								{
									NodeMap = Node->attributes;
									int nAttributeCount= NodeMap->length;
									if(nAttributeCount == iCols)
									{
										row r;
										for (int j=0;j<nAttributeCount;j++)
										{
											Node = NodeMap->item[j];				
											_bstr_t bstrFieldName = Node->nodeName;
											if(retTable.m_strlstCols.GetSize()<iCols)
											{
												retTable.m_strlstCols.Add(bstrFieldName.operator const char*());
											}
											_bstr_t bstrFieldValue = Node->nodeValue;
											r.Add(bstrFieldValue.operator const char*());

										}
										retTable.m_lstRows.Add(r);
										if(retTable.m_lstRows.GetSize()>=100)
										{
											retTable.m_iRows = retTable.m_lstRows.GetSize();
											InsertTableRecord(strTableName,retTable,fnRecordCallBack);

											retTable.m_lstRows.RemoveAll();
											retTable.m_iRows = 0;
										}

									}
									
								}
							}
							retTable.m_iRows = retTable.m_lstRows.GetSize();
							InsertTableRecord(strTableName,retTable,fnRecordCallBack);

							retTable.m_lstRows.RemoveAll();
							retTable.m_iRows = 0;
						}
					
					}	

				}

			}
			else
			{
				MSXML::IXMLDOMParseErrorPtr spParseError;
				_bstr_t bstrReason;

				spParseError =XMLDoc->parseError;
				bstrReason = spParseError->reason;
				ATLTRACE ("parse xml document error (%s)",bstrReason.operator char*());

			}
		}


		return bResult;
	}

	BOOL Database::ImportTableAsXML(LPSTR strTableName,LPSTR strFileName,pfnRecordCallback fnRecordCallBack)
	{
		BOOL bResult = FALSE;
		if (!IsOpen()) {
			ATLTRACE(_T("ImportTableAsXML database is not opened\r\n"));
			m_iLastError=SQLITE_ERROR;
			return FALSE;
		}
		if(!IsTableExistd(CA2CT(strTableName)))
			return FALSE;

		DWORD dwAttribute=GetFileAttributesA(strFileName);
		if( dwAttribute== INVALID_FILE_ATTRIBUTES || (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)== FILE_ATTRIBUTE_DIRECTORY)
			return FALSE;

		CoInitialize(NULL);
		bResult = RealImportTableAsXML(strTableName,strFileName);
		CoUninitialize();
		return bResult;
	}
	BOOL Database::ExportTableAsCSV(LPSTR strTableName,LPSTR strFileName,LPSTR strCond)
	{
		BOOL bResult = FALSE;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		if (!IsOpen()) {
			ATLTRACE(_T("ExportTableAsCSV database is not opened\r\n"));
			m_iLastError=SQLITE_ERROR;
			return FALSE;
		}
		if(!IsTableExistd(CA2CT(strTableName)))
			return FALSE;
		// I do not need to imlpement the whole sqlite3_get_table
		//   with callbacks and all myself, because it won't spare me anything,
		// I'm converting the returned strings to TCHAR anyway, 
		//   so I'm not really copying the strings into the Table class

		char ** retStrings = 0;
		char * errmsg = 0;
		int iRows=0, iCols=0;

		char szWriteBuffer[1024]={0};
		DWORD dwWritten = 0;

		TCHAR strSQL[256];
		StringCbCopy(strSQL,sizeof(strSQL),_T("Select * from "));
		StringCbCat(strSQL,sizeof(strSQL),CA2CT(strTableName));
		if(strCond && strlen(strCond))
		{
			StringCbCat(strSQL,sizeof(strSQL),_T(" where "));
			StringCbCat(strSQL,sizeof(strSQL),CA2CT(strCond));
		}

		int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
			&iRows, &iCols, &errmsg);

		if (iErr!=SQLITE_OK)
		{
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;
			ATLTRACE(_T("ExportTableAsCSV execute sql error:%s\r\n"),strOut);

		}

#ifdef DEBUG
		if (errmsg) {

			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
			delete []strOut;
		}
#endif
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		if(errmsg)
		{
			sqlite3_free(errmsg);
		
		}
		if(iCols)
		{
			bResult = TRUE;

			DWORD dwAttributes = GetFileAttributesA(strFileName);
			if(dwAttributes != INVALID_FILE_ATTRIBUTES)
			{
				OutputDebugStringA(strFileName);
				OutputDebugStringA((" has been existed,will be overwrite\r\n"));
				DeleteFileA(strFileName);

			}
			hFile = CreateFileA (strFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				
				OutputDebugStringA(strFileName);
				OutputDebugStringA((" Create File Failed\r\n"));
				
			}	
			else
			{
				StringCbPrintfA(szWriteBuffer,sizeof(szWriteBuffer),"tablename,row_count,col_count\r\n%s,%d,%d\r\n",strTableName,iRows,iCols);
				WriteFile(hFile,szWriteBuffer,(DWORD)strlen(szWriteBuffer),&dwWritten,NULL);


			}
			


		}
		Table retTable;

		if (iRows>0) 
		{
			retTable.m_iPos=0; // Put retTable's cursor on first row
		}

		retTable.m_iCols=iCols;
		retTable.m_iRows=iRows;

		// allocate the memory for the header
		//retTable.m_strlstCols.reserve(iCols);

		int iPos=0;
		char szFieldName[256]={0};
	
	
		for (; iPos<iCols; iPos++)
		{
			// Add a string element
			//retTable.m_strlstCols.push_back(stdvstring());
			ZeroMemory(szWriteBuffer,sizeof(szWriteBuffer));
			ZeroMemory(szFieldName,sizeof(szFieldName));
			if(iPos != 0)//第2个字段
			{
				StringCbCopyA(szWriteBuffer,sizeof(szWriteBuffer),",");
			}
			// Convert the UTF8 to TCHAR string
			if (retStrings[iPos] && strlen(retStrings[iPos]))
			{
				LPTSTR strOut=NULL;
				ConvertUTF8ToString( retStrings[iPos], strlen(retStrings[iPos]), strOut);
				retTable.m_strlstCols.Add(strOut);

				StringCbCopyA(szFieldName,sizeof(szFieldName),CT2A(strOut));
				if(strchr(szFieldName,','))
				{
					StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),"\"");
					StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),szFieldName);
					StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),"\"");
				}
				else
				{
					
					StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),szFieldName);
				
				}
				WriteFile(hFile,szWriteBuffer,(DWORD)strlen(szWriteBuffer),&dwWritten,NULL);

				delete[] strOut;
			}
			else 
			{
				//retTable.m_strlstCols.back().push_back(_T('\0'));
				retTable.m_strlstCols.Add(_T(""));
			}
		}
		if(iCols)
		{
			StringCbCopyA(szWriteBuffer,sizeof(szWriteBuffer),"\r\n");
			WriteFile(hFile,szWriteBuffer,(DWORD)strlen(szWriteBuffer),&dwWritten,NULL);

		}
	
		//retTable.m_lstRows.resize(iRows);
		for (int iRow=0; iRow<iRows; iRow++)
		{
			//retTable.m_lstRows[iRow].reserve(iCols);
			row r;
			ZeroMemory(szWriteBuffer,sizeof(szWriteBuffer));
			for (int iCol=0; iCol<iCols; iCol++)
			{
				// Add a string element
				//retTable.m_lstRows[iRow].push_back(stdvstring());
				if(iCol != 0)//第2个字段
				{
					StringCbCopyA(szWriteBuffer,sizeof(szWriteBuffer),",");
				}
				
				// Convert the UTF8 to TCHAR string
				if (retStrings[iPos] && strlen(retStrings[iPos]))
				{
					LPTSTR strOut;
					ConvertUTF8ToString( retStrings[iPos],strlen(retStrings[iPos]),strOut );
					
					char* szFieldValue = (char*)malloc(_tcslen(strOut)+1);
					if(szFieldValue)
					{
						StringCbCopyA(szFieldValue,_tcslen(strOut)+1,CT2A(strOut));
						if(strchr(szFieldValue,','))
						{
							StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),"\"");
							StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),szFieldValue);
							StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),"\"");
						}
						else
						{

							StringCbCatA(szWriteBuffer,sizeof(szWriteBuffer),szFieldValue);

						}
						WriteFile(hFile,szWriteBuffer,(DWORD)strlen(szWriteBuffer),&dwWritten,NULL);

						free(szFieldValue);
					}
					
					r.Add(strOut);
					delete[] strOut;
				}
				else {
					r.Add(_T(""));
                    WriteFile(hFile,",",1,&dwWritten,NULL);
				}

				iPos++;
			}
			retTable.m_lstRows.Add(r);

			StringCbCopyA(szWriteBuffer,sizeof(szWriteBuffer),"\r\n");
			WriteFile(hFile,szWriteBuffer,(DWORD)strlen(szWriteBuffer),&dwWritten,NULL);

		}

		// sqlite3_free_table() also checks for null pointer...

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}
		if(retStrings)
		{
			sqlite3_free_table(retStrings);
		}
		
		return bResult;
	}
	BOOL Database::RealExportTableAsXML(LPSTR strTableName,LPSTR strFileName,LPSTR strCond)
	{
		BOOL bResult = FALSE;
		HANDLE hFile = INVALID_HANDLE_VALUE;
	
		// I do not need to imlpement the whole sqlite3_get_table
		//   with callbacks and all myself, because it won't spare me anything,
		// I'm converting the returned strings to TCHAR anyway, 
		//   so I'm not really copying the strings into the Table class

		char ** retStrings = 0;
		char * errmsg = 0;
		int iRows=0, iCols=0;

		TCHAR strSQL[256];
		StringCbCopy(strSQL,sizeof(strSQL),_T("Select * from "));
		StringCbCat(strSQL,sizeof(strSQL),CA2CT(strTableName));

		if(strCond && strlen(strCond))
		{
			StringCbCat(strSQL,sizeof(strSQL),_T(" where "));
			StringCbCat(strSQL,sizeof(strSQL),CA2CT(strCond));
		}
		int iErr=sqlite3_get_table(m_sqlite3, UTF8MBSTR(strSQL), &retStrings,
			&iRows, &iCols, &errmsg);

		if (iErr!=SQLITE_OK)
		{
			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			m_strLastError = strOut;
			delete []strOut;
			ATLTRACE(_T("ExportTableAsCSV execute sql error:%s\r\n"),strOut);

		}

#ifdef DEBUG
		if (errmsg) {

			m_iLastError=iErr;
			LPTSTR strOut=NULL; 
			ConvertUTF8ToString(errmsg,strlen(errmsg), strOut);
			ATLTRACE(_T("QuerySQL error: %s\r\n"), strOut);
			delete []strOut;
		}
#endif
		// No need for string errors, 
		//   too costly is the comparision of strings.
		//
		// BTW: sqlite3_free() already checks for null pointer,
		//		  so I do not do double checking.
		//		And I can assume this for later versions, because
		//		  that is generally the point of sqlite3_free()...
		if(errmsg)
		{
			sqlite3_free(errmsg);

		}
		if(iCols)
		{
			bResult = TRUE;

			DWORD dwAttributes = GetFileAttributesA(strFileName);
			if(dwAttributes != INVALID_FILE_ATTRIBUTES)
			{
				OutputDebugStringA(strFileName);
				OutputDebugStringA((" has been existed,will be overwrite\r\n"));

			}
			HRESULT                          hr;
			MSXML::IXMLDOMDocumentPtr        XMLDoc = NULL;
			MSXML::IXMLDOMElementPtr         XMLElement = NULL,NewXmlElement = NULL,NewXmlElement2 = NULL;
			MSXML::IXMLDOMNodePtr            Node = NULL,ChildNode=NULL;

			MSXML::IXMLDOMNamedNodeMapPtr    NodeMap = NULL;
			MSXML::IXMLDOMNodeListPtr        NodeList = NULL,CurNodeList = NULL;

			VARIANT_BOOL                     vBoolSuccess;

			char szBuffer[512];

			CHAR XmlTemplate[] = "<?xml version=\"1.0\"?>"
										  "<table name=\" \" row_count=\" \" col_count=\"\">"
										  "</table>";

			hr = CoCreateInstance (__uuidof (MSXML::DOMDocument),NULL,CLSCTX_INPROC_SERVER,__uuidof (MSXML::IXMLDOMDocument),(void**)&XMLDoc);
			if (S_OK == hr)
			{
				vBoolSuccess = XMLDoc->loadXML (XmlTemplate);
				if (vBoolSuccess == VARIANT_TRUE)
				{
					XMLElement =  XMLDoc->documentElement ;
					if (XMLElement != NULL)
					{
						Node = XMLElement->selectSingleNode (L"//table");
						if (Node )
						{
							NodeMap = Node->attributes;
							if (NodeMap )
							{
								Node = NodeMap->getNamedItem (L"name");
								if (Node)
								{
									Node->text = strTableName;
								}
								Node = NodeMap->getNamedItem (L"row_count");
								if (Node)
								{
									StringCbPrintfA(szBuffer,sizeof(szBuffer),"%u",(ULONG)iRows);
									Node->text = szBuffer;
								}
								Node = NodeMap->getNamedItem (L"col_count");
								if (Node)
								{
									StringCbPrintfA(szBuffer,sizeof(szBuffer),"%u",(ULONG)iCols);
									Node->text = szBuffer;
								}
							}


							Table retTable;

							if (iRows>0) 
							{
								retTable.m_iPos=0; // Put retTable's cursor on first row
							}

							retTable.m_iCols=iCols;
							retTable.m_iRows=iRows;

							// allocate the memory for the header
							//retTable.m_strlstCols.reserve(iCols);

							int iPos=0;
							
							
							for (; iPos<iCols; iPos++)
							{
								// Add a string element
								//retTable.m_strlstCols.push_back(stdvstring());
								
								
								// Convert the UTF8 to TCHAR string
								if (retStrings[iPos] && strlen(retStrings[iPos]))
								{
									LPTSTR strOut=NULL;
									ConvertUTF8ToString( retStrings[iPos], strlen(retStrings[iPos]), strOut);
									retTable.m_strlstCols.Add(strOut);
									delete[] strOut;
								}
								else 
								{
									//retTable.m_strlstCols.back().push_back(_T('\0'));
									retTable.m_strlstCols.Add(_T(""));
								}
							}
							MSXML::IXMLDOMNodePtr RootNode = XMLElement->selectSingleNode (L"//table");
							//retTable.m_lstRows.resize(iRows);
							for (int iRow=0; iRow<iRows; iRow++)
							{
								//retTable.m_lstRows[iRow].reserve(iCols);
								row r;
								NewXmlElement = XMLDoc->createElement (L"row");
								if(NewXmlElement)
								{
									NodeMap = NewXmlElement->attributes;
									

									for (int iCol=0; iCol<iCols; iCol++)
									{
										// Add a string element
										//retTable.m_lstRows[iRow].push_back(stdvstring());
									
										// Convert the UTF8 to TCHAR string
										if (retStrings[iPos] && strlen(retStrings[iPos]))
										{
											LPTSTR strOut;
											ConvertUTF8ToString( retStrings[iPos],strlen(retStrings[iPos]),strOut );
											
										

										
										
											r.Add(strOut);
											delete[] strOut;
										}
										else {
											r.Add(_T(""));
										}

										iPos++;
									}

									for(int iCol =0; iCol<iCols;iCol++)
									{
										MSXML::IXMLDOMAttributePtr  AttributeNode = XMLDoc->createAttribute(retTable.m_strlstCols[iCol].GetBuffer());
										AttributeNode->text = r[iCol].GetBuffer();
										NodeMap->setNamedItem(AttributeNode);
									}
								

									retTable.m_lstRows.Add(r);
									RootNode->appendChild(NewXmlElement);
								}
								
							

							}
						
							XMLDoc->save(strFileName);
							// sqlite3_free_table() also checks for null pointer...

							
						}	

						
					}

				
				}
				else
				{
					MSXML::IXMLDOMParseErrorPtr spParseError;
					_bstr_t bstrReason;

					spParseError =XMLDoc->parseError;
					bstrReason = spParseError->reason;
					ATLTRACE ("parse xml document error (%s)",bstrReason.operator char*());
					
				}
			}
		
		

		

		
		

		}
		
		if(retStrings)
		{
			sqlite3_free_table(retStrings);
		}

		return bResult;
	}


	BOOL Database::ExportTableAsXML(LPSTR strTableName,LPSTR strFileName,LPSTR strCond)
	{
		BOOL bResult = FALSE;
		if (!IsOpen()) {
			ATLTRACE(_T("ExportTableAsXML database is not opened\r\n"));
			m_iLastError=SQLITE_ERROR;
			return FALSE;
		}
		if(!IsTableExistd(CA2CT(strTableName)))
			return FALSE;

		CoInitialize(NULL);
		bResult = RealExportTableAsXML(strTableName,strFileName,strCond);
		CoUninitialize();
		return bResult;
	}
	LPCTSTR Table::GetColName( int iCol )
	{
		if (iCol>=0 && iCol<m_iCols)
		{
			return m_strlstCols[iCol];
		}
		return 0;
	}

	bool Table::GoFirst()
	{ 
		if (this  == 0) return false;
		if (m_lstRows.GetSize()) 
		{
			m_iPos=0; 
			return true;
		}
		return false;
	}

	bool Table::GoLast()
	{ 
		if (m_lstRows.GetSize()) 
		{
			m_iPos=(int)m_lstRows.GetSize()-1; 
			return true;
		}
		return false;
	}

	bool Table::GoNext()
	{ 
		if (m_iPos+1<(int)m_lstRows.GetSize()) 
		{
			m_iPos++; 
			return true;
		}
		return false;
	}

	bool Table::GoPrev()
	{
		if (m_iPos>0)
		{
			m_iPos--;
			return true;
		}
		return false;
	}

	bool Table::GoRow(unsigned int iRow)
	{
		if (this  == 0) return false;
		if (iRow<(ULONG)m_lstRows.GetSize())
		{
			m_iPos=iRow;
			return true;
		}
		return false;
	}

    int Table::GetColIndex(LPCTSTR lpColName)
    {
        if (!lpColName) return 0;
        if (m_iPos<0) return 0;
        for (int i=0; i<m_iCols; i++)
        {
            if (!_tcsicmp(m_strlstCols[i],lpColName))
            {
                return i;
            }
        }
        return -1;
    }
	LPCTSTR Table::GetValue(LPCTSTR lpColName)
	{
		if (!lpColName) return 0;
		if (m_iPos<0) return 0;
		for (int i=0; i<m_iCols; i++)
		{
			if (!_tcsicmp(m_strlstCols[i],lpColName))
			{
				return m_lstRows[m_iPos][i];
			}
		}
		return _T("");
	}

	LPCTSTR Table::GetValue(int iColIndex)
	{
		if (iColIndex<0 || iColIndex>=m_iCols) return 0;
		if (m_iPos<0) return 0;
		return m_lstRows[m_iPos][iColIndex];
	}
    LPCTSTR Table::SetValue(int iColIndex,LPCTSTR lpstNewValue)
    {
        if (iColIndex<0 || iColIndex>=m_iCols) return 0;
        if (m_iPos<0) return 0;
        m_lstRows[m_iPos][iColIndex] = lpstNewValue;
        return m_lstRows[m_iPos][iColIndex];
    }

	LPCTSTR Table::operator [] (LPCTSTR lpColName)
	{
		if (!lpColName) return 0;
		if (m_iPos<0) return 0;
		for (int i=0; i<m_iCols; i++)
		{
			if (!_tcsicmp(m_strlstCols[i],lpColName))
			{
				return m_lstRows[m_iPos][i];
			}
		}
		return _T("");
	}

	LPCTSTR Table::operator [] (int iColIndex)
	{
		if (iColIndex<0 || iColIndex>=m_iCols) return 0;
		if (m_iPos<0) return 0;
		return m_lstRows[m_iPos][iColIndex];
	}

	void Table::JoinTable(Table & tblJoin)
	{
		if (m_iCols  == 0) {
			*this=tblJoin;
			return;
		}
		if (m_iCols!=tblJoin.m_iCols) return;

		if (tblJoin.m_iRows>0)
		{
			m_iRows+=tblJoin.m_iRows;

			for (int i=0;i<tblJoin.m_lstRows.GetSize();i++)
			{
				m_lstRows.Add(tblJoin.m_lstRows[i]);
			}

		/*	for (std::vector<row>::iterator it=tblJoin.m_lstRows.begin();
				it!=tblJoin.m_lstRows.end(); it++)
			{
				m_lstRows.push_back(*it);
			}*/
		}
	}

	TablePtr::TablePtr( )
	{ 
		m_pTable=0; 
	}

	TablePtr::TablePtr( Table * pTable )
	{
		m_pTable = pTable; 
	}

	TablePtr::TablePtr( const TablePtr& cTablePtr )
	{
		m_pTable=cTablePtr.m_pTable;

		// Copy constructors and operators, 
		// for a good reason, must take const reference,
		// but here, for a good reason, we must modify the referenced object,
		// so it won't get freed by the original.
		// I'm using a simple technique to bypass the const declaration
		((TablePtr *)&cTablePtr)->m_pTable=0;
	}

	TablePtr::~TablePtr()
	{ 
		if (m_pTable) delete m_pTable; 
	}

	void TablePtr::operator =( const TablePtr& cTablePtr )
	{
		if (m_pTable) delete m_pTable;
		m_pTable=cTablePtr.m_pTable;

		// Copy constructors and operators, 
		// for a good reason, must take const reference,
		// but here, for a good reason, we must modify the referenced object,
		// so it won't get freed by the original.
		// I'm using a simple technique to bypass the const declaration
		((TablePtr *)&cTablePtr)->m_pTable=0;
	}

	Table * TablePtr::Detach()
	{
		Table * pTable=m_pTable;
		m_pTable=0;
		return pTable;
	}

	void TablePtr::Attach( Table * pTable )
	{
		if (m_pTable) delete m_pTable;
		m_pTable=pTable;
	}

	void TablePtr::Destroy()
	{
		if (m_pTable) delete m_pTable;
		m_pTable=0;
	}

	UTF8MBSTR::UTF8MBSTR()
	{
		m_strUTF8_MultiByte=new char[1];
		m_strUTF8_MultiByte[0]=0;
		m_iLen=0;
	}

	UTF8MBSTR::UTF8MBSTR( LPCTSTR lpStr )
	{
		if (lpStr)
			m_iLen=ConvertStringToUTF8(lpStr, m_strUTF8_MultiByte);
		else {
			m_strUTF8_MultiByte=new char[1];
			m_strUTF8_MultiByte[0]=0;
			m_iLen=0;
		}
	}

	UTF8MBSTR::UTF8MBSTR( UTF8MBSTR& lpStr )
	{
		m_iLen=lpStr.m_iLen;
		m_strUTF8_MultiByte=new char[m_iLen+1];
		strncpy_s(m_strUTF8_MultiByte, m_iLen+1, lpStr.m_strUTF8_MultiByte, m_iLen+1);
	}

	UTF8MBSTR::~UTF8MBSTR()
	{
		if (m_strUTF8_MultiByte)
		{
			delete [] m_strUTF8_MultiByte;
		}
	}

	void UTF8MBSTR::operator =( LPCTSTR lpStr )
	{
		if (m_strUTF8_MultiByte)
		{
			delete [] m_strUTF8_MultiByte;
		}
		if (lpStr)
			m_iLen=ConvertStringToUTF8(lpStr, m_strUTF8_MultiByte);
		else {
			m_strUTF8_MultiByte=new char[1];
			m_strUTF8_MultiByte[0]=0;
			m_iLen=0;
		}
	}

	void UTF8MBSTR::operator =( UTF8MBSTR& lpStr )
	{
		if (m_strUTF8_MultiByte)
		{
			delete [] m_strUTF8_MultiByte;
		}
		m_iLen=lpStr.m_iLen;
		m_strUTF8_MultiByte=new char[m_iLen+1];
		strncpy_s(m_strUTF8_MultiByte, m_iLen+1, lpStr.m_strUTF8_MultiByte, m_iLen+1);
	}

	UTF8MBSTR::operator char* ()
	{
		return m_strUTF8_MultiByte;
	}

	UTF8MBSTR::operator stdstring ()
	{
		TCHAR * strRet;
		ConvertUTF8ToString(m_strUTF8_MultiByte, m_iLen+1, strRet);

		stdstring cstrRet(strRet);

		delete [] strRet;

		return cstrRet;
	}

	size_t UTF8MBSTR::ConvertStringToUTF8( LPCTSTR strIn, char *& strOutUTF8MB )
	{
		size_t len=_tcslen(strIn);

#ifdef UNICODE
		int iRequiredSize=WideCharToMultiByte(CP_UTF8, 0, strIn, (int)len+1, 0, 0, 0, 0);

		strOutUTF8MB=new char[iRequiredSize];
		strOutUTF8MB[0]=0;

		WideCharToMultiByte(CP_UTF8, 0, strIn, (int)len+1, strOutUTF8MB, iRequiredSize, 0, 0);
#else
		WCHAR * wChar=new WCHAR[len+1];
		wChar[0]=0;
		MultiByteToWideChar(CP_ACP, 0, strIn, (int)len+1, wChar, (int)len+1);
		int iRequiredSize=WideCharToMultiByte(CP_UTF8, 0, wChar, (int)len+1, 0, 0, 0, 0);
		strOutUTF8MB=new char[iRequiredSize];
		strOutUTF8MB[0]=0;
		WideCharToMultiByte(CP_UTF8, 0, wChar, (int)len+1, strOutUTF8MB, iRequiredSize, 0, 0);
		delete [] wChar;
#endif

		return len;
	}

	void UTF8MBSTR::ConvertUTF8ToString( char * strInUTF8MB, size_t len, LPTSTR & strOut )
	{
		strOut=new TCHAR[len];
		strOut[0]=0;

#ifdef UNICODE
		MultiByteToWideChar(CP_UTF8, 0, strInUTF8MB, (int)len, strOut, (int)len);
#else
		WCHAR * wChar=new WCHAR[len];
		wChar[0]=0;
		MultiByteToWideChar(CP_UTF8, 0, strInUTF8MB, (int)len, wChar, (int)len);
		WideCharToMultiByte(CP_ACP, 0, wChar, (int)len, strOut, (int)len, 0, 0);
		delete [] wChar;
#endif
	}
}

//nBufferLen长度不要包含回车符和换行符

int split_string( char* szBuffer,int nBufferLen,const char* szSplitCharSet,PMyString pMyString,int nSize)
{

	int nCount = 0;
	int nElementCount = 0;


	BOOL bQuot= FALSE;

	while (szBuffer[nCount]!='\0' && nCount<nBufferLen)
	{
		bQuot= FALSE;
		while(szBuffer[nCount]=='\"')
		{	
			bQuot= TRUE;
			nCount++;
		}

		if(!bQuot)
		{

          
            int iCommaCount = 0;
			while(strchr(szSplitCharSet,szBuffer[nCount]))
            {
				nCount++;
                iCommaCount++;
                if (iCommaCount>1)
                {
                    nElementCount++;
                }
            }
           
			pMyString[nElementCount].szStart=&szBuffer[nCount];
			pMyString[nElementCount].nLen = 0;




			while(!strchr(szSplitCharSet,szBuffer[nCount]))
			{
				pMyString[nElementCount].nLen++;
				nCount++;
				if(nCount>=nBufferLen)
					break;
			}
		}
		else
		{
			pMyString[nElementCount].szStart=&szBuffer[nCount];
			pMyString[nElementCount].nLen = 0;

			do 
			{
				if(szBuffer[nCount]=='\"' && (szBuffer[nCount+1]=='\r'|| szBuffer[nCount+1]==','))
				{
					break;
				}
				pMyString[nElementCount].nLen++;
				nCount++;
			} while (nCount<nBufferLen);
			nCount++;

		}
		

	


		nElementCount++;
		if(nElementCount>=nSize)
			break;


	}

	return nElementCount;

}

