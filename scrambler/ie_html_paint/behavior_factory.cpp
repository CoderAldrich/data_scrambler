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
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include "behavior_handler.h"
#include "mshtml_addition.h"

_COM_SMARTPTR_TYPEDEF(IElementBehaviorFactory, __uuidof(IElementBehaviorFactory));

#if 0
HRESULT WINAPI add_html_element_behevior( IHTMLElement *html_element, 
										 html_behavior_handler* handler )
{
	HRESULT hr = S_OK; 

	do 
	{
		ASSERT( NULL != html_element ); 

		HRESULT hr;
		long lBehaviorID;
		VARIANT vBehavior;
		IElementBehaviorFactoryPtr factory = NULL; 
		// Get the behavior factory interface for
		// the rendering behavior
		//hr = CoCreateInstance(CLSID_RenderingBehavior,
		//	NULL,
		//	CLSCTX_INPROC_SERVER,
		//	IID_IElementBehaviorFactory, (void**)&pRBFactory);

		// Convert pRBFactory to the proper VARIANT data type
		// for IHTMLElement2::AddBehavior

		{
			IUnknown *unknown; 

			handler->get_factory( &factory ); 
			if( NULL == factory )
			{
				hr = E_FAIL; 
				break; 
			}

			hr = factory->QueryInterface( IID_IUnknown, ( PVOID* )&unknown ); 
			if( FAILED(hr))
			{
				break; 
			}

			if( unknown == NULL )
			{
				hr = E_FAIL; 
				break; 
			}

			V_VT(&vBehavior) = VT_UNKNOWN;
			V_UNKNOWN(&vBehavior) = unknown;

			// AddRef because the pRBFactory pointer has been copied
			unknown->AddRef();
		}

		// Or Alternatively
		//vBehavior.vt = VT_UNKNOWN;
		//vBehavior.punkVal = pSnapperFactory;
		//vBehavior.punkVal->AddRef();

		// Add behavior
		{
			IHTMLElement2Ptr element; 
			hr = html_element->QueryInterface( IID_IHTMLElement2, ( PVOID* )&element); 
			if( FAILED(hr ))
			{
				break; 
			}

			if( NULL == element )
			{
				hr = E_FAIL; 
				break; 
			}

			hr = element->addBehavior(NULL,
				&vBehavior,
				&lBehaviorID);
		}
	}while( FALSE );

	return hr; 
}
#endif //0