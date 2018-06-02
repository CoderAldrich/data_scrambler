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


#if !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
#define AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>
#include <Mshtml.h>

class html_document  
{
public:
	html_document();
	virtual ~html_document();

	bool SetDocument(IDispatch* pDisp);
	bool SetDocument(IHTMLDocument2* pdoc);
	LPDISPATCH GetHtmlDocument() const;
	const CString GetLastError() const;
	bool GetJScript(CComPtr<IDispatch>& spDisp);
	bool GetJScripts(CComPtr<IHTMLElementCollection>& spColl);

	CString ScanJScript(CString& strAText, CStringArray& args);

	bool CallJScript(const CString strFunc,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,const CString strArg4,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,const CString strArg4,const CString strArg5,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CStringArray& paramArray,CComVariant* pVarResult = NULL);

protected:
	void ShowError(LPCTSTR lpszText);

protected:

	CComPtr<IHTMLDocument2>	m_spDoc;
	CString	m_strError;

};

inline
void html_document::ShowError(LPCTSTR lpszText)
{
	m_strError = CString(_T("JSCall Error:\n")) + CString(lpszText);
}
inline 
const CString html_document::GetLastError() const
{
	return m_strError;
}
inline
LPDISPATCH html_document::GetHtmlDocument() const
{
	return m_spDoc;
}

CString GetNextToken(CString& strSrc, const CString strDelim,bool bTrim = false, bool bFindOneOf = true);

#endif // !defined(AFX_WEBPAGE_H__AEBD50B8_EE66_40AB_8B92_C4EECB9BCD22__INCLUDED_)
