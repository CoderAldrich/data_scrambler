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

#include "stdafx.h"
#pragma hdrstop 
#include "doc_host_ui_handler.h " 

HRESULT __stdcall doc_host_ui_handler::QueryInterface(REFIID riid, void **ppv) 
{ 
	if(IsEqualIID(riid,IID_IUnknown)) 
	{ 
		*ppv = static_cast <IUnknown*> (this); 
		return S_OK; 
	} 
	else if(IsEqualIID(riid,IID_IDocHostUIHandler)) 
	{ 
		*ppv = static_cast <IDocHostUIHandler*> (this); 
		return S_OK; 
	} 
	else 
	{ 
		*ppv = NULL; 
		return E_NOINTERFACE; 
	} 
} 

ULONG WINAPI doc_host_ui_handler::AddRef() 
{ 
	InterlockedIncrement( ( LONG* )&ref_count); 

	return ref_count; 
} 

ULONG WINAPI doc_host_ui_handler::Release() 
{ 
	if(InterlockedDecrement( ( LONG* )&ref_count) <= 0 ) 
	{
		delete this; 
	}

	return ref_count; 
} 


STDMETHODIMP doc_host_ui_handler::ShowContextMenu(DWORD dwID,POINT *ppt, 
IUnknown *pcmdtReserved,IDispatch *pdispReserved) 
{ 
	return S_OK; 
} 

STDMETHODIMP doc_host_ui_handler::GetHostInfo(DOCHOSTUIINFO *pInfo) 
{ 
	pInfo-> dwFlags = pInfo->dwFlags | DOCHOSTUIFLAG_NO3DBORDER; 

	return S_OK; 
} 

log_manager *logger = NULL;

STDMETHODIMP doc_host_ui_handler::TranslateUrl( 
						/* [in] */ DWORD dwTranslate, 
						/* [in] */ OLECHAR __RPC_FAR *pchURLIn, 
						/* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{ 
	HRESULT hr = S_OK; 
	
	if( NULL != pchURLIn )
	{
		translate_url = pchURLIn; 

        if (logger != NULL)
        {
            logger->append_log(0, pchURLIn); 
        }
	}

	return hr; 
}


LRESULT WINAPI set_logger(log_manager *_logger)
{
    logger = _logger;
    return ERROR_SUCCESS;
}

//STDMETHODIMP doc_host_ui_handler::TranslateAccelerator(LPMSG lpMsg,const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
//{
//	return S_OK; 
//}

LRESULT WINAPI set_host_ui_handler_on_doc( IHTMLDocument2 *doc, 
										  IDocHostUIHandler *handler )
{
	LRESULT ret = ERROR_SUCCESS; 
	ICustomDocPtr cumstom_doc; 
	HRESULT hr; 

	do 
	{
		ASSERT( NULL != doc ); 
		ASSERT( NULL != handler); 
		
		hr = doc->QueryInterface( IID_ICustomDoc,( PVOID *)&cumstom_doc ); 
		if( FAILED(hr))
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		if( NULL == cumstom_doc )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		hr = cumstom_doc->SetUIHandler( handler ); 
		if( FAILED( hr ) ) 
		{ 
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		} 

	} while ( FALSE ); 

	return ret; 
} 