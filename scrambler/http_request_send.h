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

// File: httpsend.h
// Description: Sends error report over HTTP connection.
// Authors: zexspectrum
// Date: 2009

#pragma once

#include <WinInet.h>

struct CHttpRequestFile
{  
    CString m_sSrcFileName;  // Name of the file attachment.
    CString m_sContentType;  // Content type.
};

typedef struct _http_content
{
	CString text_name; 
	CString text_cont; 
} http_content, *phttp_content; 

#define MAX_HTTP_CONT_NUM 10
// HTTP request information
class http_request
{
public:
	ULONG text_count; 
    CString m_sUrl;      // Script URL  
    http_content m_aTextFields[ MAX_HTTP_CONT_NUM ];    // Array of text fields to include into POST data
};

// Sends HTTP request
// See also: RFC 1867 - Form-based File Upload in HTML (http://www.ietf.org/rfc/rfc1867.txt)
class http_data_sender
{
public:

    http_data_sender();

	LRESULT add_text_count( LPCTSTR name, LPCTSTR text )
	{
		LRESULT ret = ERROR_SUCCESS; 

		if( m_Request.text_count >= MAX_HTTP_CONT_NUM )
		{
			ret = ERROR_NOT_ENOUGH_QUOTA; 
			goto _return; 
		}

		m_Request.m_aTextFields[ m_Request.text_count ].text_name = name; 
		m_Request.m_aTextFields[ m_Request.text_count ].text_cont = text; 

		m_Request.text_count ++; 
_return:
		return ret; 
	}

	BOOL send_all_data( LPCTSTR url ); 

    // Sends HTTP request assynchroniously
    BOOL SendAssync(http_request& Request );

private:

    // Worker thread procedure
    static DWORD WINAPI WorkerThread(VOID* pParam);  

    BOOL InternalSend();

    // Used to calculate summary size of the request
    BOOL CalcRequestSize(LONGLONG& lSize);
	BOOL FormatTextPartHeader(CString sName, CString& sText);
	BOOL FormatTextPartFooter(CString sName, CString& sText);  
	//BOOL FormatAttachmentPartHeader(CString sName, CString& sText);
	//BOOL FormatAttachmentPartFooter(CString sName, CString& sText);
	BOOL FormatTrailingBoundary(CString& sBoundary);
	BOOL CalcTextPartSize(CString sName, CString sData, LONGLONG& lSize); 
	//BOOL CalcAttachmentPartSize(CString sFileName, LONGLONG& lSize);
	BOOL WriteTextPart(HINTERNET hRequest, CString sName, CString sData);
	//BOOL WriteAttachmentPart(HINTERNET hRequest, CString sName);
	BOOL WriteTrailingBoundary(HINTERNET hRequest);
	void UploadProgress(DWORD dwBytesWritten);

    // This helper function is used to split URL into several parts
    void ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol,
        LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, LPTSTR szURI, UINT cbURI);

    http_request m_Request;       // HTTP request being sent

    CString m_sFilePartHeaderFmt;
    CString m_sFilePartFooterFmt;
    CString m_sTextPartHeaderFmt;
    CString m_sTextPartFooterFmt;
    CString m_sBoundary;
    DWORD m_dwPostSize;
    DWORD m_dwUploaded;
};


