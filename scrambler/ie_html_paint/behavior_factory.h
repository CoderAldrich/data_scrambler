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

#ifndef __BEGHAVIOR_HFACTORY_H__
#define __BEGHAVIOR_HFACTORY_H__

// {1C939A78-7E06-4f29-BDC9-A57EE873C423}
//DEFINE_GUID(CLSID_BEHAVIOR_FACTORY, 
//			0x1c939a78, 0x7e06, 0x4f29, 0xbd, 0xc9, 0xa5, 0x7e, 0xe8, 0x73, 0xc4, 0x23);

template<typename T>
class behavior_factory : 
	//public CComCoClass<behavior_factory, &CLSID_BEHAVIOR_FACTORY>,
	//public IObjectSafetyImpl<behavior_factory<T>, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
	public IElementBehaviorFactory

{	
private:
	LONG ref_count; 

	T *paint_host; 

public:
	behavior_factory():paint_host(NULL), 
		ref_count( 0 ) 
	{		
	}

	void construct(T *p) 
	{
		paint_host = p; 
	}
	
	//BEGIN_COM_MAP(ThisClass)
	//	COM_INTERFACE_ENTRY(IElementBehaviorFactory)		
	//END_COM_MAP()
	
	STDMETHOD(FindBehavior)(BSTR bstrBehavior,BSTR bstrBehaviorUrl,IElementBehaviorSite *pSite,IElementBehavior **behavior_out)
	{
		HRESULT hr;
		html_element_paint<T> *behavior = NULL; 
		
		do 
		{
			behavior = new html_element_paint<T>(); 

			if( NULL == behavior )
			{
				hr = E_OUTOFMEMORY; 
				break; 
			}

			behavior->construct(paint_host);
			hr = behavior->QueryInterface( IID_IElementBehavior, ( PVOID* )behavior_out ); 
		}while( FALSE );

		return hr;
	}

	//STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	//{ return _InternalQueryInterface(riid, ppv); }

	//STDMETHODIMP_(ULONG) AddRef()
	//{ return InternalAddRef(); }

	//STDMETHODIMP_(ULONG) Release() {
	//	ULONG l = InternalRelease();
	//	if( l == 0 ) delete this;
	//	return l;
	//}

	static STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid,
		void** ppv) 
	{
		*ppv = 0;
		if( pUnkOuter ) return CLASS_E_NOAGGREGATION;
		// Read on for why not to use new like this!
		CComObject<behavior_factory<T>>* pobj = new CComObject<behavior_factory<T>>;
		if( pobj ) 
		{
			pobj->AddRef();
			HRESULT hr = pobj->QueryInterface(riid, ppv);
			pobj->Release();
			return hr;
		}
		return E_OUTOFMEMORY;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
	{ 
		if(IsEqualIID(riid,IID_IUnknown)) 
		{ 
			*ppv = static_cast <IUnknown*> (this); 
			return S_OK; 
		} 
		else if(IsEqualIID(riid,IID_IElementBehaviorFactory)) 
		{ 
			*ppv = static_cast <IElementBehaviorFactory*> (this); 
			return S_OK; 
		} 
		else 
		{ 
			*ppv = NULL; 
			return E_NOINTERFACE; 
		} 
	} 

	STDMETHODIMP_(ULONG) AddRef() 
	{ 
		InterlockedIncrement( ( LONG* )&ref_count); 

		return ref_count; 
	} 

	STDMETHODIMP_(ULONG) Release() 
	{ 
		if(InterlockedDecrement( ( LONG* )&ref_count) <= 0 ) 
		{
			delete this; 
		}

		return ref_count; 
	} 
};

#endif //__BEGHAVIOR_HFACTORY_H__