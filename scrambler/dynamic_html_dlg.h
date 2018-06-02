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

/********************************************************************
/*  2005-2006 (C)Webmote & luo31, All Rights Reserved.
/*	created:	2005-5-12   10:51
/*	filename: 	d:\luo31_prj\DHtmlViewSpec.h
/*********************************************************************/
#pragma once

#include <afxdhtml.h>
#include <afxhtml.h>
class CDHtmlSpecEventSink 
{
public:
	virtual const DHtmlEventMapEntry* GetDHtmlEventMap();
	virtual BOOL DHtmlEventHook(HRESULT *phr, DISPID dispIdMember, DISPPARAMS *pDispParams,
		VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
	virtual HRESULT GetDHtmlDocument(IHTMLDocument2 **pphtmlDoc);

	//create
	CDHtmlSpecEventSink();
	void InitEventSink(IHTMLDocument2* pDoc);
protected:
	IHTMLDocument2* m_pDocHtml;
	CHtmlView* m_pView;
};

// CSpecDHtmlView Html view

class dynamic_html_dlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(dynamic_html_dlg)
	//DECLARE_DHTML_EVENT_MAP()
	//DECLARE_EVENTSINK_MAP()
	//DECLARE_DHTML_EVENT_MAP()
protected:
	dynamic_html_dlg();
	dynamic_html_dlg(UINT nIDTemplate, UINT nHtmlResID = 0, CWnd *pParentWnd = NULL);
	dynamic_html_dlg(LPCTSTR lpszTemplateName, LPCTSTR szHtmlResID, CWnd *pParentWnd = NULL);
	virtual ~dynamic_html_dlg();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void GotoUrl(LPCTSTR pszUrl);
	//Implementation
public:
	// DDX functions
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, CString& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, short& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, int& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, long& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, DWORD& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, float& value);
	void DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, double& value);
	void DDX_DHtml_CheckBox(CDataExchange* pDX, LPCTSTR szId, int& value);
	void DDX_DHtml_Radio(CDataExchange* pDX, LPCTSTR szId, long& value);
	void DDX_DHtml_SelectValue(CDataExchange* pDX, LPCTSTR szId, CString& value);
	void DDX_DHtml_SelectString(CDataExchange* pDX, LPCTSTR szId, CString& value);
	void DDX_DHtml_SelectIndex(CDataExchange* pDX, LPCTSTR szId, long& value);
	void DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, DISPID dispid, VARIANT& var);
	void DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, LPCTSTR szPropName, VARIANT& var);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	// DDX Functions
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, CString& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, short& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, int& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, long& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, DWORD& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, float& value, BOOL bSave);
	void DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, double& value, BOOL bSave);

	/////////////////////////////////////////////////////////////////////////////
	// Data exchange for special controls
	void DDX_DHtml_CheckBox(LPCTSTR szId, int& value, BOOL bSave);
	void DDX_DHtml_Radio(LPCTSTR szId, long& value, BOOL bSave);
	void DDX_DHtml_SelectValue(LPCTSTR szId, CString& value, BOOL bSave);
	void DDX_DHtml_SelectString(LPCTSTR szId, CString& value, BOOL bSave);
	void DDX_DHtml_SelectIndex(LPCTSTR szId, long& value, BOOL bSave);
	void DDX_DHtml_AxControl(LPCTSTR szId, DISPID dispid, VARIANT& var, BOOL bSave);
	void DDX_DHtml_AxControl(LPCTSTR szId, LPCTSTR szPropName, VARIANT& var, BOOL bSave);
	void OnDDXError(LPCTSTR szId, UINT nIDPrompt, BOOL bSave);

	void __cdecl DDX_DHtml_ElementTextWithFormat(LPCTSTR szId,
		DISPID dispid, LPCTSTR lpszFormat, UINT nIDPrompt, BOOL bSave, ...);
	void DDX_DHtml_ElementTextFloatFormat(LPCTSTR szId,
		DISPID dispid, void* pData, double value, int nSizeGcvt, BOOL bSave);
	BOOL _AfxSimpleScanf(LPCTSTR lpszText,
		LPCTSTR lpszFormat, va_list pData);
	BOOL _AfxSimpleFloatParse(LPCTSTR lpszText, double& d);
	long Select_FindString(IHTMLSelectElement *pSelect, BSTR bstr, BOOL fExact);
	void SetFocusToElement(LPCTSTR szId);
	
	//// host handler
	//DWORD m_dwHostFlags;
	//CComPtr<IDispatch> m_spExternalDisp;

public:
	void Initialize(void);

protected:
	// event sink
	DWORD m_dwDHtmlEventSinkCookie;
	CSimpleArray<CDHtmlControlSink *> m_ControlSinks;
	CSimpleArray<CDHtmlElementEventSink *> m_SinkedElements;
	//////////////////////////////////////////////////////////////////////////
	//deal event helper functions
	HRESULT get_element_from_id(LPCTSTR szElementId, IDispatch **ppdisp, BOOL *pbCollection = NULL);
	HRESULT get_element_from_id(LPCTSTR szElementId, IHTMLElement **pphtmlElement);
	template <class Q>
	HRESULT GetElementInterface(LPCTSTR szElementId, Q** ppvObj)
	{
		return GetElementInterface(szElementId, __uuidof(Q), (void **) ppvObj);
	}
	HRESULT GetElementInterface(LPCTSTR szElementId, REFIID riid, void **ppvObj);

	void OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR szUrl); 
	virtual void OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	HRESULT ConnectDHtmlEvents(IUnknown *punkDoc);
	void DisconnectDHtmlEvents();
	HRESULT ConnectDHtmlElementEvents(DWORD_PTR dwThunkOffset = 0);
	BOOL FindSinkForObject(LPCTSTR szName);
	BOOL IsSinkedElement(IDispatch *pdispElem);
	void DisconnectDHtmlElementEvents();
	afx_msg void OnDestroy();

public:
	//access HTML element
	BSTR GetElementText(LPCTSTR szElementId);
	void SetElementText(LPCTSTR szElementId, BSTR bstrText);
	void SetElementText(IUnknown *punkElem, BSTR bstrText);
	BSTR GetElementHtml(LPCTSTR szElementId);
	void SetElementHtml(LPCTSTR szElementId, BSTR bstrText);
	void SetElementHtml(IUnknown *punkElem, BSTR bstrText);
	VARIANT GetElementProperty(LPCTSTR szElementId, DISPID dispid);
	void SetElementProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar);
	HRESULT GetControlDispatch(LPCTSTR szId, IDispatch **ppdisp);
	VARIANT GetControlProperty(IDispatch *pdispControl, DISPID dispid);
	VARIANT GetControlProperty(LPCTSTR szId, DISPID dispid);
	VARIANT GetControlProperty(LPCTSTR szId, LPCTSTR szPropName);
	void SetControlProperty(IDispatch *pdispControl, DISPID dispid, VARIANT *pVar);
	void SetControlProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar);
	void SetControlProperty(LPCTSTR szElementId, LPCTSTR szPropName, VARIANT *pVar);
	HRESULT GetEvent(IHTMLEventObj **ppEventObj);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

