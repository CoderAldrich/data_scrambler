// DHtmlViewSpec.cpp : 
//

#include "stdafx.h"
#include "DHtmlViewSpec.h"
#include ".\dhtmlviewspec.h"


// CDHtmlViewSpec

IMPLEMENT_DYNCREATE(CDHtmlViewSpec, CHtmlView)


CDHtmlViewSpec::CDHtmlViewSpec():
	CHtmlView()
{
}

CDHtmlViewSpec::~CDHtmlViewSpec()
{
}

void CDHtmlViewSpec::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDHtmlViewSpec, CHtmlView)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDHtmlViewSpec 

#ifdef _DEBUG
void CDHtmlViewSpec::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CDHtmlViewSpec::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CDHtmlViewSpec 

void CDHtmlViewSpec::Initialize(void)
{
	
	//SetHostFlags(DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_SCROLL_NO);
	m_dwDHtmlEventSinkCookie = 0;
}

BOOL CDHtmlViewSpec::PreTranslateMessage(MSG* pMsg)
{
	//ban function keys
	if(
		(
		( pMsg->message == WM_KEYDOWN )
		||
		( pMsg->message == WM_KEYUP )
		||
		( pMsg->message == WM_SYSKEYDOWN )
		||
		( pMsg->message == WM_SYSKEYUP )
		)
		&&
		(
		( pMsg -> wParam == VK_F1 )
		||
		( pMsg -> wParam == VK_HELP )
		||
		( pMsg -> wParam == VK_F2 )
		||
		( pMsg -> wParam == VK_F3 )
		||
		( pMsg -> wParam == VK_F4 )
		||
		( pMsg -> wParam == VK_F5 )
		||
		( pMsg -> wParam == VK_F6 )
		||
		( pMsg -> wParam == VK_F7 )
		||
		( pMsg -> wParam == VK_F8 )
		||
		( pMsg -> wParam == VK_F9 )
		||
		( pMsg -> wParam == VK_F10 )
		||
		( pMsg -> wParam == VK_F11 )
		||
		( pMsg -> wParam == VK_F12 )
		||
		( pMsg -> wParam == VK_ESCAPE )
		)
		)
	{
		OutputDebugString("Function key ignored\r\n");
		return true;
	}

	//ban openning new page
	if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('N' == pMsg->wParam) )
	{
		//Ctrl-N
		OutputDebugString("Ctrl-N ignored\r\n");
		return true;
	}
	//ban printing
	if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('P' == pMsg->wParam) )
	{
		//Ctrl-P
		OutputDebugString("Ctrl-P ignored\r\n");
		return true;
	}
	//ban adding to favorites
	if( ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) && ('D' == pMsg->wParam) )
	{
		//Ctrl-D
		OutputDebugString("Ctrl-D ignored\r\n");
		return true;
	}
	//ban navigating backward
	if( ( GetAsyncKeyState(VK_MENU) & 0x8000 ) && (VK_LEFT == pMsg->wParam) )
	{
		//Atl-Left
		OutputDebugString("Atl-Left ignored\r\n");
		return true;
	}
	//ban navigating forward
	if( ( GetAsyncKeyState(VK_MENU) & 0x8000 ) && (VK_RIGHT == pMsg->wParam) )
	{
		//Atl-Right
		OutputDebugString("Atl-Right ignored\r\n");
		return true;
	}


	//mouse 
	if( pMsg->message == WM_RBUTTONDOWN || 
		pMsg->message == WM_RBUTTONDBLCLK)
	{
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}
void CDHtmlViewSpec::GotoUrl(LPCTSTR pszUrl)
{
	CString strPath,str;
	DWORD dwSize=MAX_PATH;
	::GetModuleFileName(NULL,strPath.GetBuffer(MAX_PATH),dwSize); //AfxGetResourceHandle()
	strPath.ReleaseBuffer(dwSize);
	str=strPath.Left(strPath.ReverseFind('\\')+1);
	ASSERT(pszUrl!=NULL);
	if(pszUrl!=NULL)
	{
		CString strUrl;
		strUrl=str+pszUrl;
		Navigate(_T("file:///")+strUrl);
	}
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, CString& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, short& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, int& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, long& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, DWORD& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, float& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(CDataExchange* pDX, LPCTSTR szId, DISPID dispid, double& value)
{
	DDX_DHtml_ElementText(szId, dispid, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_CheckBox(CDataExchange* pDX, LPCTSTR szId, int& value)
{
	DDX_DHtml_CheckBox(szId, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_Radio(CDataExchange* pDX, LPCTSTR szId, long& value)
{
	DDX_DHtml_Radio(szId, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_SelectValue(CDataExchange* pDX, LPCTSTR szId, CString& value)
{
	DDX_DHtml_SelectValue(szId, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_SelectString(CDataExchange* pDX, LPCTSTR szId, CString& value)
{
	DDX_DHtml_SelectString(szId, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_SelectIndex(CDataExchange* pDX, LPCTSTR szId, long& value)
{
	DDX_DHtml_SelectIndex(szId, value, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, DISPID dispid, VARIANT& var)
{
	DDX_DHtml_AxControl(szId, dispid, var, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_AxControl(CDataExchange *pDX, LPCTSTR szId, LPCTSTR szPropName, VARIANT& var)
{
	DDX_DHtml_AxControl(szId, szPropName, var, pDX->m_bSaveAndValidate);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, CString& value, BOOL bSave)
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	CComVariant var;
	if (bSave)
	{
		var = GetElementProperty(szId, dispid);
		var.ChangeType(VT_BSTR);
		value = var.bstrVal;
	}
	else
	{
		var = (LPCTSTR)value;
		SetElementProperty(szId, dispid, &var);
	}
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, short& value, BOOL bSave)
{
	if (bSave)
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%sd"), AFX_IDP_PARSE_INT, bSave, &value);
	else
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%hd"), AFX_IDP_PARSE_INT, bSave, value);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, int& value, BOOL bSave)
{
	if (bSave)
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%d"), AFX_IDP_PARSE_INT, bSave, &value);
	else
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%d"), AFX_IDP_PARSE_INT, bSave, value);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, long& value, BOOL bSave)
{
	if (bSave)
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%ld"), AFX_IDP_PARSE_INT, bSave, &value);
	else
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%ld"), AFX_IDP_PARSE_INT, bSave, value);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, DWORD& value, BOOL bSave)
{
	if (bSave)
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%lu"), AFX_IDP_PARSE_UINT, bSave, &value);
	else
		DDX_DHtml_ElementTextWithFormat(szId, dispid, _T("%lu"), AFX_IDP_PARSE_UINT, bSave, value);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, float& value, BOOL bSave)
{
	DDX_DHtml_ElementTextFloatFormat(szId, dispid, &value, value, FLT_DIG, bSave);
}

void CDHtmlViewSpec::DDX_DHtml_ElementText(LPCTSTR szId, DISPID dispid, double& value, BOOL bSave)
{
	DDX_DHtml_ElementTextFloatFormat(szId, dispid, &value, value, DBL_DIG, bSave);
}

/////////////////////////////////////////////////////////////////////////////
// Data exchange for special controls
void CDHtmlViewSpec::DDX_DHtml_CheckBox(LPCTSTR szId, int& value, BOOL bSave)
{
	COleVariant var;

	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	CComPtr<IHTMLOptionButtonElement> spOptionButton;
	HRESULT hr = S_OK;

	hr = GetElementInterface(szId, __uuidof(IHTMLOptionButtonElement), (void **) &spOptionButton);
	if (spOptionButton == NULL)
		goto Error;


	if (bSave)
	{
		VARIANT_BOOL bIndeterminate;
		VARIANT_BOOL bChecked;
		hr = spOptionButton->get_checked(&bChecked);
		if (FAILED(hr))
			goto Error;
		hr = spOptionButton->get_indeterminate(&bIndeterminate);
		if (FAILED(hr))
			goto Error;
		value = ((bChecked != ATL_VARIANT_FALSE) ? BST_CHECKED : BST_UNCHECKED) | 
			((bIndeterminate != ATL_VARIANT_FALSE) ? BST_INDETERMINATE : 0);
	}
	else
	{
		hr = spOptionButton->put_checked((value & BST_CHECKED) ? ATL_VARIANT_TRUE : ATL_VARIANT_FALSE);
		if (FAILED(hr))
			goto Error;
		hr = spOptionButton->put_indeterminate((value & BST_INDETERMINATE) ? ATL_VARIANT_TRUE : ATL_VARIANT_FALSE);
		if (FAILED(hr))
			goto Error;
	}
Error:
	if (FAILED(hr))
	{
		TRACE(traceHtml, 0, "Failed DDX_DHtml_CheckBox\n");
	}
}

void CDHtmlViewSpec::DDX_DHtml_Radio(LPCTSTR szId, long& value, BOOL bSave)
{
	COleVariant var;

	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	CComPtr<IHTMLOptionButtonElement> spOptionButton;
	CComPtr<IDispatch> spdispCollection;
	CComPtr<IHTMLElementCollection> spElementColl;
	BOOL bIsCollection = FALSE;
	CComPtr<IDispatch> spdispElem;

	HRESULT hr = S_OK;
	COleVariant varIndex;
	COleVariant varEmpty;
	varIndex.vt = VT_I4;
	varIndex.lVal = 0;

	// get the radio buttons in the group
	hr = GetElement(szId, &spdispCollection, &bIsCollection);
	if (spdispCollection && bIsCollection)
	{
		hr = spdispCollection->QueryInterface(__uuidof(IHTMLElementCollection), (void **) &spElementColl);
		if (spElementColl == NULL)
		{
			TRACE(traceHtml, 0, "Error: Collection didn't support IHTMLElementCollection!\n");
			ASSERT(FALSE);
			goto Error;
		}

		if (bSave)
			value = -1;     // value if none found

		long lCount = 0;
		spElementColl->get_length(&lCount);

		for (long lCntr = 0; lCntr < lCount; lCntr++)
		{				
			spdispElem = NULL;
			spOptionButton = NULL;
			varIndex.lVal = lCntr;
			hr = spElementColl->item(varIndex, varEmpty, &spdispElem);
			if (spdispElem == NULL)
				break;
			hr = spdispElem->QueryInterface(__uuidof(IHTMLOptionButtonElement), (void **) &spOptionButton);

			if (bSave)
			{
				VARIANT_BOOL bChecked;
				hr = spOptionButton->get_checked(&bChecked);
				if (FAILED(hr))
					goto Error;
				if (bChecked != ATL_VARIANT_FALSE)
				{
					value = varIndex.lVal;
					break;
				}
			}
			else
			{
				if (varIndex.lVal == value)
				{
					spOptionButton->put_checked(ATL_VARIANT_TRUE);
					break;
				}
			}
		};
	}
Error:
	if (FAILED(hr))
	{
		TRACE(traceHtml, 0, "Warning: Failed DDX_DHtml_Radio\n");
	}
}

void CDHtmlViewSpec::DDX_DHtml_SelectValue(LPCTSTR szId, CString& value, BOOL bSave)
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	CComBSTR bstrText;
	CComPtr<IHTMLSelectElement> spSelect;
	HRESULT hr = GetElementInterface(szId, __uuidof(IHTMLSelectElement), (void **) &spSelect);
	if (FAILED(hr))
		goto Error;

	if (bSave)
	{
		spSelect->get_value(&bstrText);
		if (bstrText)
			value = bstrText;
	}
	else
	{
		bstrText.Attach(value.AllocSysString());
		spSelect->put_value(bstrText);
	}

Error:
	return;
}

void CDHtmlViewSpec::DDX_DHtml_SelectString(LPCTSTR szId, CString& value, BOOL bSave)
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	COleVariant varEmpty, varIndex;

	CComPtr<IHTMLSelectElement> spSelect;
	CComPtr<IDispatch> spdispOption;
	CComPtr<IHTMLOptionElement> spOption;
	CComBSTR bstrText;
	HRESULT hr = S_OK;
	long lIndex=-1;

	hr = GetElementInterface(szId, __uuidof(IHTMLSelectElement), (void **) &spSelect);
	if (spSelect == NULL)
		return;

	if (bSave)
	{
		// get the selected item
		value.Empty();
		spSelect->get_selectedIndex(&lIndex);
		if (lIndex >= 0)
		{
			varIndex = lIndex;

			spSelect->item(varIndex, varEmpty, &spdispOption);
			if (spdispOption)
			{
				spdispOption->QueryInterface(__uuidof(IHTMLOptionElement), (void **) &spOption);
				if (spOption)
				{
					spOption->get_text(&bstrText);
					if (bstrText)
						value = bstrText;
				}
			}
		}
	}
	else
	{
		bstrText.Attach(value.AllocSysString());
		lIndex = Select_FindString(spSelect, bstrText, FALSE);
		spSelect->put_selectedIndex(lIndex);
	}
}

void CDHtmlViewSpec::DDX_DHtml_SelectIndex(LPCTSTR szId, long& value, BOOL bSave)
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return;

	CComPtr<IHTMLSelectElement> spSelect;
	HRESULT hr;

	hr = GetElementInterface(szId, __uuidof(IHTMLSelectElement), (void **) &spSelect);
	if (spSelect == NULL)
		return;

	if (bSave)
		spSelect->get_selectedIndex(&value);
	else
		spSelect->put_selectedIndex(value);
}

void CDHtmlViewSpec::DDX_DHtml_AxControl(LPCTSTR szId, DISPID dispid, VARIANT& var, BOOL bSave)
{
	if (bSave)
		var = GetControlProperty(szId, dispid);
	else
		SetControlProperty(szId, dispid, &var);
}

void CDHtmlViewSpec::DDX_DHtml_AxControl(LPCTSTR szId, LPCTSTR szPropName, VARIANT& var, BOOL bSave)
{
	if (bSave)
		var = GetControlProperty(szId, szPropName);
	else
		SetControlProperty(szId, szPropName, &var);
}

void CDHtmlViewSpec::OnDDXError(LPCTSTR szId, UINT nIDPrompt, BOOL /*bSave*/)
{
	AfxMessageBox(nIDPrompt);
	// default implementation just sets the
	// focus back to the offending element
	SetFocusToElement(szId);
	CComPtr<IHTMLEventObj> sphtmlEvent;
	GetEvent(&sphtmlEvent);
	if (sphtmlEvent)
		sphtmlEvent->put_cancelBubble(ATL_VARIANT_TRUE);
	AfxThrowUserException();
}

void __cdecl CDHtmlViewSpec::DDX_DHtml_ElementTextWithFormat(LPCTSTR szId,
														   DISPID dispid, LPCTSTR lpszFormat, UINT nIDPrompt, BOOL bSave, ...)
														   // only supports windows output formats - no floating point
{
	va_list pData;
	va_start(pData, bSave);

	CString value;

	if (bSave)
	{
		DDX_DHtml_ElementText(szId, dispid, value, bSave);
		// the following works for %d, %u, %ld, %lu
		if (!_AfxSimpleScanf(value, lpszFormat, pData))
		{
			// set the focus to the offending element
			SetFocusToElement(szId);

			OnDDXError(szId, nIDPrompt, bSave);
			va_end(pData);
			return;
		}
	}
	else
	{
		CString strTemp ;
		strTemp.FormatV(lpszFormat,pData);
		// does not support floating point numbers - see dlgfloat.cpp
		DDX_DHtml_ElementText(szId, dispid, strTemp, bSave);
	}
	va_end(pData);
}

void CDHtmlViewSpec::DDX_DHtml_ElementTextFloatFormat(LPCTSTR szId,
													DISPID dispid, void* pData, double value, int nSizeGcvt, BOOL bSave)
{
	ATLASSERT(pData != NULL);

	CString strValue;
	if (bSave)
	{
		DDX_DHtml_ElementText(szId, dispid, strValue, bSave);
		double d;
		if (!_AfxSimpleFloatParse(strValue, d))
		{
			OnDDXError(szId, AFX_IDP_PARSE_REAL, bSave);
		}
		if (nSizeGcvt == FLT_DIG)
			*((float*)pData) = (float)d;
		else
			*((double*)pData) = d;
	}
	else
	{
		CString szBuffer;
		szBuffer.Format(_T("%.*g"), nSizeGcvt, value);
		DDX_DHtml_ElementText(szId, dispid, szBuffer, bSave);
	}
}


BOOL CDHtmlViewSpec::_AfxSimpleScanf(LPCTSTR lpszText,
								   LPCTSTR lpszFormat, va_list pData)
{
	ATLASSERT(lpszText != NULL);
	ATLASSERT(lpszFormat != NULL);

	ATLASSERT(*lpszFormat == '%');
	lpszFormat++;        // skip '%'

	BOOL bLong = FALSE;
	BOOL bShort = FALSE;
	if (*lpszFormat == 'l')
	{
		bLong = TRUE;
		lpszFormat++;
	}
	else if (*lpszFormat == 's')
	{
		bShort = TRUE;
		lpszFormat++;
	}

	ATLASSERT(*lpszFormat == 'd' || *lpszFormat == 'u');
	ATLASSERT(lpszFormat[1] == '\0');

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;
	TCHAR chFirst = lpszText[0];
	long l, l2;
	if (*lpszFormat == 'd')
	{
		// signed
		l = _tcstol(lpszText, (LPTSTR*)&lpszText, 10);
		l2 = (int)l;
	}
	else
	{
		// unsigned
		if (*lpszText == '-')
			return FALSE;
		l = (long)_tcstoul(lpszText, (LPTSTR*)&lpszText, 10);
		l2 = (unsigned int)l;
	}
	if (l == 0 && chFirst != '0')
		return FALSE;   // could not convert

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;
	if (*lpszText != '\0')
		return FALSE;   // not terminated properly

	if (bShort)
	{
		if ((short)l != l)
			return FALSE;   // too big for short
		*va_arg(pData, short*) = (short)l;
	}
	else
	{
		ATLASSERT(sizeof(long) == sizeof(int));
		ATLASSERT(l == l2);
		*va_arg(pData, long*) = l;
	}

	// all ok
	return TRUE;
}

BOOL CDHtmlViewSpec::_AfxSimpleFloatParse(LPCTSTR lpszText, double& d)
{
	ATLASSERT(lpszText != NULL);
	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	TCHAR chFirst = lpszText[0];
	d = _tcstod(lpszText, (LPTSTR*)&lpszText);
	if (d == 0.0 && chFirst != '0')
		return FALSE;   // could not convert
	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	if (*lpszText != '\0')
		return FALSE;   // not terminated properly

	return TRUE;
}

long CDHtmlViewSpec::Select_FindString(IHTMLSelectElement *pSelect, BSTR bstr, BOOL /*fExact*/)
{
	long lIndexFound = -1;
	COleVariant varIndex, varEmpty;
	CComPtr<IDispatch> spdispOption;
	CComPtr<IHTMLOptionElement> spOption;
	long lCount = 0;

	HRESULT hr = pSelect->get_length(&lCount);
	if (FAILED(hr))
		goto Error;
	varIndex.vt = VT_I4;
	// loop through the items searching for the string
	for (varIndex.lVal=0; varIndex.lVal<lCount; varIndex.lVal++)
	{
		pSelect->item(varIndex, varEmpty, &spdispOption);
		if (spdispOption)
		{
			spdispOption->QueryInterface(__uuidof(IHTMLOptionElement), (void **) &spOption);
			if (spOption)
			{
				CComBSTR bstrText;
				spOption->get_text(&bstrText);
				if (bstrText && !wcscmp(bstrText, bstr))
				{
					// we found it
					lIndexFound = varIndex.lVal;
					break;
				}
				spOption = NULL;
			}
			spdispOption = NULL;
		}
	}

Error:
	return lIndexFound;
}

void CDHtmlViewSpec::SetFocusToElement(LPCTSTR szId)
{
	// check if the element is a control element
	CComPtr<IHTMLControlElement> sphtmlCtrlElem;
	HRESULT hr = GetElementInterface(szId, &sphtmlCtrlElem);
	if (sphtmlCtrlElem)
	{
		sphtmlCtrlElem->focus();
		return;
	}

	// check if the element is an anchor element
	CComPtr<IHTMLAnchorElement> sphtmlAnchorElem;
	hr = GetElementInterface(szId, &sphtmlAnchorElem);
	if (sphtmlAnchorElem)
	{
		sphtmlAnchorElem->focus();
		return;
	}

	// otherwise all we can do is scroll the element into view
	CComPtr<IHTMLElement> sphtmlElem;
	hr = GetElementInterface(szId, &sphtmlElem);
	if (sphtmlElem)
	{
		CComVariant var;
		var.vt = VT_BOOL;
		var.boolVal = ATL_VARIANT_TRUE;
		sphtmlElem->scrollIntoView(var);
	}
}

HRESULT CDHtmlViewSpec::GetControlDispatch(LPCTSTR szId, IDispatch **ppdisp)
{
	HRESULT hr = S_OK;
	CComPtr<IDispatch> spdispElem;

	hr = GetElement(szId, &spdispElem);

	if (spdispElem)
	{
		CComPtr<IHTMLObjectElement> sphtmlObj;

		hr = spdispElem.QueryInterface(&sphtmlObj);
		if (sphtmlObj)
		{
			spdispElem.Release();
			hr = sphtmlObj->get_object(ppdisp);
		}
	}
	return hr;
}

VARIANT CDHtmlViewSpec::GetControlProperty(IDispatch *pdispControl, DISPID dispid)
{
	VARIANT varRet;
	varRet.vt = VT_EMPTY;
	if (pdispControl)
	{
		DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
		pdispControl->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, 
			DISPATCH_PROPERTYGET, &dispparamsNoArgs, &varRet, NULL, NULL);
	}
	return varRet;
}

VARIANT CDHtmlViewSpec::GetControlProperty(LPCTSTR szId, DISPID dispid)
{
	CComPtr<IDispatch> spdispElem;

	GetControlDispatch(szId, &spdispElem);
	return GetControlProperty(spdispElem, dispid);
}

VARIANT CDHtmlViewSpec::GetControlProperty(LPCTSTR szId, LPCTSTR szPropName)
{
	CComVariant varEmpty;
	CComPtr<IDispatch> spdispElem;

	GetControlDispatch(szId, &spdispElem);
	if (!spdispElem)
		return varEmpty;

	DISPID dispid;
	USES_CONVERSION;
	LPOLESTR pPropName = (LPOLESTR)T2COLE(szPropName);
	HRESULT hr = spdispElem->GetIDsOfNames(IID_NULL, &pPropName, 1, LOCALE_USER_DEFAULT, &dispid);
	if (SUCCEEDED(hr))
		return GetControlProperty(spdispElem, dispid);
	return varEmpty;
}

void CDHtmlViewSpec::SetControlProperty(IDispatch *pdispControl, DISPID dispid, VARIANT *pVar)
{
	if (pdispControl != NULL)
	{
		DISPPARAMS dispparams = {NULL, NULL, 1, 1};
		dispparams.rgvarg = pVar;
		DISPID dispidPut = DISPID_PROPERTYPUT;
		dispparams.rgdispidNamedArgs = &dispidPut;

		pdispControl->Invoke(dispid, IID_NULL,
			LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
			&dispparams, NULL, NULL, NULL);
	}
}

void CDHtmlViewSpec::SetControlProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar)
{
	CComPtr<IDispatch> spdispElem;
	GetControlDispatch(szElementId, &spdispElem);

	SetControlProperty(spdispElem, dispid, pVar);
}

void CDHtmlViewSpec::SetControlProperty(LPCTSTR szElementId, LPCTSTR szPropName, VARIANT *pVar)
{
	CComPtr<IDispatch> spdispElem;
	GetControlDispatch(szElementId, &spdispElem);
	if (!spdispElem)
		return;
	DISPID dispid;
	USES_CONVERSION;
	LPOLESTR pPropName = (LPOLESTR)T2COLE(szPropName);
	HRESULT hr = spdispElem->GetIDsOfNames(IID_NULL, &pPropName, 1, LOCALE_USER_DEFAULT, &dispid);
	if (SUCCEEDED(hr))
		SetControlProperty(spdispElem, dispid, pVar);
}

HRESULT CDHtmlViewSpec::GetEvent(IHTMLEventObj **ppEventObj)
{

	CComPtr<IHTMLWindow2> sphtmlWnd;
	CComPtr<IHTMLDocument2> sphtmlDoc;

	*ppEventObj = NULL;

	HRESULT hr = GetDHtmlDocument(&sphtmlDoc);
	if (sphtmlDoc == NULL)
		return hr;

	hr = sphtmlDoc->get_parentWindow(&sphtmlWnd);
	if (FAILED(hr))
		goto Error;
	hr = sphtmlWnd->get_event(ppEventObj);

Error:
	return hr;
}
//////////////////////////////////////////////////////////////////////////
//event sink functions
HRESULT CDHtmlViewSpec::ConnectDHtmlEvents(IUnknown *punkDoc)
{
	return ConnectToConnectionPoint(punkDoc, __uuidof(HTMLDocumentEvents), &m_dwDHtmlEventSinkCookie);
}

void CDHtmlViewSpec::DisconnectDHtmlEvents()
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	GetDHtmlDocument(&sphtmlDoc);

	if (sphtmlDoc == NULL)
		return;
	DisconnectFromConnectionPoint(sphtmlDoc, __uuidof(HTMLDocumentEvents), m_dwDHtmlEventSinkCookie);
	DisconnectDHtmlElementEvents();
}

HRESULT CDHtmlViewSpec::ConnectDHtmlElementEvents(DWORD_PTR dwThunkOffset /*= 0*/)
{
	HRESULT hr = S_OK;
	const DHtmlEventMapEntry* pEventMap = GetDHtmlEventMap();
	if (!pEventMap)
		return hr;

	for (int i=0; pEventMap[i].nType != DHTMLEVENTMAPENTRY_END; i++)
	{
		if (pEventMap[i].nType==DHTMLEVENTMAPENTRY_ELEMENT)
		{
			// an element name must be specified when using element events
			ATLASSERT(pEventMap[i].szName);

			// connect to the element's event sink
			CComPtr<IDispatch> spdispElement;
			GetElement(pEventMap[i].szName, &spdispElement);
			if (spdispElement)
			{
				if (!IsSinkedElement(spdispElement))
				{
					CDHtmlElementEventSink *pSink = NULL;
					ATLTRY(pSink = new CDHtmlElementEventSink(this, spdispElement));
					if (pSink == NULL)
						return E_OUTOFMEMORY;
					hr = AtlAdvise(spdispElement, pSink, __uuidof(IDispatch), &pSink->m_dwCookie);
					if (SUCCEEDED(hr))
						m_SinkedElements.Add(pSink);
					else
						delete pSink;
#ifdef _DEBUG
					if (FAILED(hr))
						TRACE(traceHtml, 0, "Warning: Failed to connect to ConnectionPoint!\n");
#endif
				}
			}
		}
		else if (pEventMap[i].nType==DHTMLEVENTMAPENTRY_CONTROL)
		{
			// check if we already have a sink connected to this control
			if (!FindSinkForObject(pEventMap[i].szName))
			{
				// create a new sink and
				// connect it to the element's event sink
				CComPtr<IDispatch> spdispElement;
				GetElement(pEventMap[i].szName, &spdispElement);
				if (spdispElement)
				{
					CComPtr<IHTMLObjectElement> sphtmlObj;
					spdispElement->QueryInterface(__uuidof(IHTMLObjectElement), (void **) &sphtmlObj);
					if (sphtmlObj)
					{
						CComPtr<IDispatch> spdispControl;
						sphtmlObj->get_object(&spdispControl);
						if (spdispControl)
						{
							// create a new control sink to connect to the control's events
							CDHtmlControlSink *pSink = NULL; 
							ATLTRY(pSink = new CDHtmlControlSink(spdispControl, this, pEventMap[i].szName, dwThunkOffset));
							if (pSink == NULL)
								return E_OUTOFMEMORY;
							m_ControlSinks.Add(pSink);
						}
					}
				}
			}
		}
	}
	return hr;
}
void CDHtmlViewSpec::DisconnectDHtmlElementEvents()
{
	const DHtmlEventMapEntry* pEventMap = GetDHtmlEventMap();

	if (!pEventMap)
		return;

	int i;

	// disconnect from element events
	for (i=0; i<m_SinkedElements.GetSize(); i++)
	{
		CDHtmlElementEventSink *pSink = m_SinkedElements[i];
		AtlUnadvise(pSink->m_spunkElem, __uuidof(IDispatch), pSink->m_dwCookie);
		delete pSink;
	}
	m_SinkedElements.RemoveAll();

	// disconnect from control events
	for (i=0; i<m_ControlSinks.GetSize(); i++)
	{
		DisconnectFromConnectionPoint(m_ControlSinks[i]->m_spunkObj, 
			m_ControlSinks[i]->m_iid, m_ControlSinks[i]->m_dwCookie);
		delete m_ControlSinks[i];
	}
	m_ControlSinks.RemoveAll();
	return;
}
HRESULT CDHtmlViewSpec::GetElement(LPCTSTR szElementId, IDispatch **ppdisp, 
								 BOOL *pbCollection /*= NULL*/)
{
	CComPtr<IHTMLElementCollection> sphtmlAll;
	CComPtr<IHTMLElementCollection> sphtmlColl;
	CComPtr<IDispatch> spdispElem;
	CComVariant varName;
	CComVariant varIndex;
	HRESULT hr = S_OK;
	CComPtr<IHTMLDocument2> sphtmlDoc;

	*ppdisp = NULL;

	if (pbCollection)
		*pbCollection = FALSE;

	sphtmlDoc=(IHTMLDocument2*)GetHtmlDocument();
	if (sphtmlDoc == NULL)
		return E_INVALIDARG;

	if(szElementId == NULL)
		return E_INVALIDARG;

	varName.vt = VT_BSTR;
	varName.bstrVal = T2BSTR_EX(szElementId);

#ifndef _UNICODE	
	if (!varName.bstrVal)
	{
		hr = E_OUTOFMEMORY;
		goto Error;
	}
#endif

	hr = sphtmlDoc->get_all(&sphtmlAll);
	if (sphtmlAll == NULL)
		goto Error;
	hr = sphtmlAll->item(varName, varIndex, &spdispElem);
	if (spdispElem == NULL)
	{
		hr = E_NOINTERFACE;
		goto Error;
	}

	spdispElem->QueryInterface(__uuidof(IHTMLElementCollection), (void **) &sphtmlColl);
	if (sphtmlColl)
	{
		if (pbCollection)
			*pbCollection = TRUE;
#ifdef _DEBUG
		else
		{
			TRACE(traceHtml, 0, "Warning: duplicate IDs or NAMEs.\n");
			ATLASSERT(FALSE);
		}
#endif

	}
Error:
	if (SUCCEEDED(hr))
	{
		*ppdisp = spdispElem;
		if (spdispElem)
			(*ppdisp)->AddRef();
	}
	return hr;
}

HRESULT CDHtmlViewSpec::GetElement(LPCTSTR szElementId, IHTMLElement **pphtmlElement)
{
	return GetElementInterface(szElementId, __uuidof(IHTMLElement), (void **) pphtmlElement);
}


BSTR CDHtmlViewSpec::GetElementText(LPCTSTR szElementId)
{
	BSTR bstrText = NULL;
	CComPtr<IHTMLElement> sphtmlElem;
	GetElement(szElementId, &sphtmlElem);
	if (sphtmlElem)
		sphtmlElem->get_innerText(&bstrText);
	return bstrText;
}

void CDHtmlViewSpec::SetElementText(LPCTSTR szElementId, BSTR bstrText)
{
	CComPtr<IHTMLElement> sphtmlElem;
	GetElement(szElementId, &sphtmlElem);
	if (sphtmlElem)
		sphtmlElem->put_innerText(bstrText);
}

void CDHtmlViewSpec::SetElementText(IUnknown *punkElem, BSTR bstrText)
{
	CComPtr<IHTMLElement> sphtmlElem;
	punkElem->QueryInterface(__uuidof(IHTMLElement), (void **) &sphtmlElem);
	if (sphtmlElem != NULL)
		sphtmlElem->put_innerText(bstrText);
}

BSTR CDHtmlViewSpec::GetElementHtml(LPCTSTR szElementId)
{
	BSTR bstrText = NULL;
	CComPtr<IHTMLElement> sphtmlElem;
	GetElement(szElementId, &sphtmlElem);
	if (sphtmlElem)
		sphtmlElem->get_innerHTML(&bstrText);
	return bstrText;
}

void CDHtmlViewSpec::SetElementHtml(LPCTSTR szElementId, BSTR bstrText)
{
	CComPtr<IHTMLElement> sphtmlElem;
	GetElement(szElementId, &sphtmlElem);
	if (sphtmlElem)
		sphtmlElem->put_innerHTML(bstrText);
}

void CDHtmlViewSpec::SetElementHtml(IUnknown *punkElem, BSTR bstrText)
{
	CComPtr<IHTMLElement> sphtmlElem;
	punkElem->QueryInterface(__uuidof(IHTMLElement), (void **) &sphtmlElem);
	if (sphtmlElem != NULL)
		sphtmlElem->put_innerHTML(bstrText);
}

VARIANT CDHtmlViewSpec::GetElementProperty(LPCTSTR szElementId, DISPID dispid)
{
	VARIANT varRet;
	CComPtr<IDispatch> spdispElem;
	varRet.vt = VT_EMPTY;
	GetElement(szElementId, &spdispElem);
	if (spdispElem)
	{
		DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
		spdispElem->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, 
			DISPATCH_PROPERTYGET, &dispparamsNoArgs, &varRet, NULL, NULL);
	}
	return varRet;
}

void CDHtmlViewSpec::SetElementProperty(LPCTSTR szElementId, DISPID dispid, VARIANT *pVar)
{
	CComPtr<IDispatch> spdispElem;

	GetElement(szElementId, &spdispElem);
	if (spdispElem)
	{
		DISPPARAMS dispparams = {NULL, NULL, 1, 1};
		dispparams.rgvarg = pVar;
		DISPID dispidPut = DISPID_PROPERTYPUT;
		dispparams.rgdispidNamedArgs = &dispidPut;

		spdispElem->Invoke(dispid, IID_NULL,
			LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
			&dispparams, NULL, NULL, NULL);
	}
}

HRESULT CDHtmlViewSpec::GetElementInterface(LPCTSTR szElementId, REFIID riid, void **ppvObj)
{
	HRESULT hr = E_NOINTERFACE;
	*ppvObj = NULL;
	CComPtr<IDispatch> spdispElem;

	hr = GetElement(szElementId, &spdispElem);

	if (spdispElem)
		hr = spdispElem->QueryInterface(riid, ppvObj);
	return hr;
}
BOOL CDHtmlViewSpec::FindSinkForObject(LPCTSTR szName)
{
	int nLength = m_ControlSinks.GetSize();
	for (int i=0; i<nLength; i++)
	{
		if (!_tcscmp(szName, m_ControlSinks[i]->m_szControlId))
			return TRUE;
	}
	return FALSE;
}
BOOL CDHtmlViewSpec::IsSinkedElement(IDispatch *pdispElem)
{
	CComPtr<IUnknown> spunk;
	pdispElem->QueryInterface(__uuidof(IUnknown), (void **) &spunk);
	if (!spunk)
		return FALSE;
	for (int i=0; i<m_SinkedElements.GetSize(); i++)
	{
		if (spunk == m_SinkedElements[i]->m_spunkElem)
			return TRUE;
	}
	return FALSE;
}

void CDHtmlViewSpec::OnNavigateComplete2(LPCTSTR strURL)
{
	CComPtr<IHTMLDocument2> sphtmlDoc;
	IDispatch *pdispDoc = NULL;	
	m_pBrowserApp->get_Document(&pdispDoc);
	if (!pdispDoc)
		return;
	//ASSERT(m_spHtmlDoc==NULL);

	pdispDoc->QueryInterface(IID_IHTMLDocument2, (void **) &sphtmlDoc);
	InitEventSink(sphtmlDoc);
	ConnectDHtmlEvents(pdispDoc);
	pdispDoc->Release();
	CHtmlView::OnNavigateComplete2(strURL);
}
void CDHtmlViewSpec::OnDocumentComplete(LPCTSTR lpszURL)
{
	ConnectDHtmlElementEvents((((DWORD_PTR)static_cast< CDHtmlSinkHandler* >(this)) - (DWORD_PTR)this));
	UpdateData(FALSE);
}
void CDHtmlViewSpec::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel)
{	
	DisconnectDHtmlEvents();
	InitEventSink(NULL);
	CHtmlView::OnBeforeNavigate2(lpszURL, nFlags, lpszTargetFrameName, baPostedData, lpszHeaders, pbCancel);
}

void CDHtmlViewSpec::OnDestroy()
{
	CHtmlView::OnDestroy();
	DisconnectDHtmlEvents();
	InitEventSink(NULL);
	// now tell the browser control we're shutting down
	if (m_pBrowserApp)
	{
		CComPtr<IOleObject> spObject;
		m_pBrowserApp->QueryInterface(IID_IOleObject, (void **) &spObject);
		if (spObject != NULL)
		{
			spObject->Close(OLECLOSE_NOSAVE);
			spObject.Release();
		}
		m_pBrowserApp = NULL;
		//m_pBrowserApp = NULL;
		//m_wndBrowser.DestroyWindow();
		//DestroyWindow();
	}


}




//////////////////////////////////////////////////////////////////////////
//

BOOL CDHtmlSpecEventSink::DHtmlEventHook(HRESULT *phr, DISPID dispIdMember, DISPPARAMS *pDispParams,
									 VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	phr; // unused
	dispIdMember; // unused
	pDispParams; // unused
	pVarResult; // unused
	pExcepInfo; // unused
	puArgErr; // unused
	//TRACE("dispid=%d",dispIdMember);
	// stub base implementation
	return FALSE;
}

const DHtmlEventMapEntry* CDHtmlSpecEventSink::GetDHtmlEventMap()
{
	return NULL;
}
HRESULT CDHtmlSpecEventSink::GetDHtmlDocument(IHTMLDocument2 **pphtmlDoc)
{
	if(pphtmlDoc == NULL)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}

	*pphtmlDoc = NULL;

	if (m_pDocHtml)
	{
		*pphtmlDoc = m_pDocHtml;
		(*pphtmlDoc)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}
CDHtmlSpecEventSink::CDHtmlSpecEventSink()
{
	m_pDocHtml=NULL;
}
void CDHtmlSpecEventSink::InitEventSink(IHTMLDocument2* pDoc)
{
	//ASSERT(pDoc!=NULL);
	m_pDocHtml=pDoc;
	//m_pDocHtml->AddRef();
}
/*
void CDHtmlSpecEventSink::EndEventSink()
{
	ASSERT(pDoc!=NULL);	
	m_pDocHtml->Release();
}
*/


