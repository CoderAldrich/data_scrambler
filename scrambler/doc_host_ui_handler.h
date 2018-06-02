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

#ifndef _DOC_HOST_UI_HANDLER
#define _DOC_HOST_UI_HANDLER

#include <exdispid.h> 
#include <mshtmhst.h> 
//#include <wstring>
 
class log_manager
{
public:
    log_manager()
    {}
    ~log_manager()
    {}

    virtual LRESULT append_log(ULONG type, LPCWSTR text) = 0; 
};

LRESULT WINAPI set_logger(log_manager *_logger); 

class doc_host_ui_handler : public IDocHostUIHandler
{ 
private: 
	LONG ref_count; 
	wstring translate_url; 

public: 
	doc_host_ui_handler() : ref_count( 0 ) {} 

	wstring& get_translate_url() { return translate_url; } 

	// IUnknown Method 
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv); 
	ULONG	 __stdcall AddRef(); 
	ULONG	 __stdcall Release(); 

	// IDocHostUIHandler Method 
	STDMETHOD(ShowContextMenu)( 
		/* [in] */ DWORD dwID, 
		/* [in] */ POINT __RPC_FAR *ppt, 
		/* [in] */ IUnknown __RPC_FAR *pcmdtReserved, 
		/* [in] */ IDispatch __RPC_FAR *pdispReserved); 

	STDMETHOD(GetHostInfo)( 
		/* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo); 

	STDMETHOD(ShowUI)( 
		/* [in] */ DWORD dwID, 
		/* [in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject, 
		/* [in] */ IOleCommandTarget __RPC_FAR *pCommandTarget, 
		/* [in] */ IOleInPlaceFrame __RPC_FAR *pFrame, 
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pDoc){ return E_NOTIMPL; } 

	STDMETHOD(HideUI)( void){ return E_NOTIMPL; } 

	STDMETHOD(UpdateUI)( void){ return E_NOTIMPL; } 

	STDMETHOD(EnableModeless)( 
		/* [in] */ BOOL fEnable){ return E_NOTIMPL; } 

	STDMETHOD(OnDocWindowActivate)( 
		/* [in] */ BOOL fActivate){ return E_NOTIMPL; } 

	STDMETHOD(OnFrameWindowActivate)( 
		/* [in] */ BOOL fActivate){ return E_NOTIMPL; } 

	STDMETHOD(ResizeBorder)( 
		/* [in] */ LPCRECT prcBorder, 
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow, 
		/* [in] */ BOOL fRameWindow){ return E_NOTIMPL; } 

	STDMETHOD(TranslateAccelerator)( 
		/* [in] */ LPMSG lpMsg, 
		/* [in] */ const GUID __RPC_FAR *pguidCmdGroup, 
		/* [in] */ DWORD nCmdID){ return E_NOTIMPL; } 

	STDMETHOD(GetOptionKeyPath)( 
		/* [out] */ LPOLESTR __RPC_FAR *pchKey, 
		/* [in] */ DWORD dw){ return E_NOTIMPL; } 

	STDMETHOD(GetDropTarget)( 
		/* [in] */ IDropTarget __RPC_FAR *pDropTarget, 
		/* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget){ return E_NOTIMPL; } 

	STDMETHOD(GetExternal)( 
		/* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch){ return E_NOTIMPL; } 

	STDMETHOD(TranslateUrl)( 
		/* [in] */ DWORD dwTranslate, 
		/* [in] */ OLECHAR __RPC_FAR *pchURLIn, 
		/* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut); 

	STDMETHOD(FilterDataObject)( 
		/* [in] */ IDataObject __RPC_FAR *pDO, 
		/* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet){ return E_NOTIMPL; } 
}; 

LRESULT WINAPI set_host_ui_handler_on_doc( IHTMLDocument2 *doc, 
										  IDocHostUIHandler *handler ); 

#endif //_DOC_HOST_UI_HANDLER
