/*
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

#ifndef __HTML_EVENT_HANDLER_H__
#define __HTML_EVENT_HANDLER_H__

//#define TEST_BROWSER_WORK 1
#define HTML_ELEMENT_EVENT_HANDLER_ID 1001
class HtmlElementEventHandler : public IDispatch 
{ 
private:
	LONG m_ref_count; 

public: 
	HtmlElementEventHandler() : m_ref_count( 1 )
	{ 
	} 

	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( /* [out] */ UINT *pctinfo)
	{ 
		return S_OK; 
	} 

	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( /* [in] */ UINT iTInfo, /* [in] */ LCID lcid, /* [out] */ ITypeInfo **ppTInfo)
	{ 
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( /* [in] */ REFIID riid, /* [size_is][in] */ LPOLESTR *rgszNames, /* [in] */ UINT cNames, /* [in] */ LCID lcid, /* [size_is][out] */ DISPID *rgDispId)
	{ 
		*rgDispId = HTML_ELEMENT_EVENT_HANDLER_ID; 
		return S_OK; 
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( /* [in] */ DISPID dispIdMember, /* [in] */ REFIID riid, /* [in] */ LCID lcid, /* [in] */ WORD wFlags, /* [out][in] */ DISPPARAMS *pDispParams, /* [out] */ VARIANT *pVarResult, /* [out] */ EXCEPINFO *pExcepInfo, /* [out] */ UINT *puArgErr); 
	//{ 
	//	INT32 i; 
	//	wstring text; 

	//	dbg_print( MSG_IMPORTANT, "enter %s dispatch id is 0x%0.8x\n", __FUNCTION__, dispIdMember); 

	//	for( i = 0; ( UINT32 )i < pDispParams->cArgs; i ++ )
	//	{
	//		text = variant2string( &pDispParams->rgvarg[ i ] ); 
	//		dbg_print( MSG_IMPORTANT, "dispatch parameter(%u): %ws", text.c_str() ); 
	//	}

	//	if(dispIdMember == HTML_ELEMENT_EVENT_HANDLER_ID) 
	//	{ 
	//		
	//		//MessageBox(0,L"Hello World",L"Hello",0); 
	//		//place your code here 
	//	} 

	//	return S_OK; 
	//} 

	/*
	{A731980E-7D1E-4652-8432-AD2B3BC0EA44}
	IID_IDispatchEx
	{EBADE2E2-A8CC-4797-A430-2E863867EFD0}
	{00000008-0000-0000-C000-000000000046}
	{3050F49A-98B5-11CF-BB82-00AA00BDCE0B}
	*/
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID classid, void** intf) 
	{ 
		if( classid == IID_IDispatch ) 
		{ 
			*intf = ( IDispatch* )this; 
			AddRef(); 
		} 
		else 
		{
			return E_NOINTERFACE; 
		}

		return S_OK; 
	} 

	virtual ULONG STDMETHODCALLTYPE AddRef() 
	{
		ASSERT( m_ref_count > 0 ); 
		InterlockedIncrement( &m_ref_count ); 
		return m_ref_count; 
	} 

	virtual ULONG STDMETHODCALLTYPE Release() 
	{ 
		InterlockedDecrement( &m_ref_count ); 

		if( m_ref_count == 0 ) 
		{
			delete this; 
		}

		return m_ref_count; 
	} 
}; 

#endif //__HTML_EVENT_HANDLER_H__