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
#ifndef __HTML_ELEMENT_PAINT_BEHAVIOR_H__
#define __HTML_ELEMENT_PAINT_BEHAVIOR_H__

template<typename T>
class html_element_paint :
	public IElementBehavior,	
	public IHTMLPainter, 
	public IUnknown
{
private: 
	LONG ref_count; 

public:	
	html_element_paint():m_pHost( NULL ), 
		ref_count(0)
	{
	}

private:	
	T * m_pHost;	
	CComPtr<IElementBehaviorSite> m_spBehaviorSite;
	HTML_PAINTER_INFO m_htmlpaintinfo;

public:
	typedef html_element_paint<T> ThisClass;
	//BEGIN_COM_MAP(ThisClass)
	//	COM_INTERFACE_ENTRY(IElementBehavior)
	//	COM_INTERFACE_ENTRY(IHTMLPainter)
	//END_COM_MAP()

	void construct(T * p)
	{
		m_pHost = p;		
	}

// IElementBehavior
	STDMETHOD(Init)(IElementBehaviorSite * pBehaviorSite)
	{
		m_spBehaviorSite = pBehaviorSite;	
		CComQIPtr<IHTMLPaintSite> spPaintSite(pBehaviorSite);
		m_pHost->init(spPaintSite);
		return S_OK;
	}

	STDMETHOD(Notify)(LONG lEvent, VARIANT * pVar)
	{		
		if (pVar == NULL)
			return E_POINTER;				
		return S_OK;
	}

	STDMETHOD(Detach)()
	{
		m_pHost->Detach();
		m_spBehaviorSite = NULL;
		return S_OK;
	}
	
	STDMETHOD(Draw)(RECT rcBounds, RECT rcUpdate, LONG lDrawFlags, HDC hdc, LPVOID pvDrawObject)
	{		
		m_pHost->OnDraw(hdc,rcBounds,lDrawFlags,rcUpdate,m_spBehaviorSite);
		return S_OK;
	}

	STDMETHOD(OnResize)(SIZE size)
	{		
		return S_OK;
	}

	STDMETHOD(GetPainterInfo)(HTML_PAINTER_INFO * pInfo)
	{
		if (pInfo == NULL)
			return E_POINTER; 

		memset(&m_htmlpaintinfo,0,sizeof(m_htmlpaintinfo));
		m_htmlpaintinfo.lFlags =  HTMLPAINTER_OPAQUE | HTMLPAINTER_HITTEST ;
		m_htmlpaintinfo.lZOrder = HTMLPAINT_ZORDER_ABOVE_CONTENT;		
		*pInfo = m_htmlpaintinfo;
		return S_OK;
	}

	STDMETHOD(HitTestPoint)(POINT pt, BOOL * pbHit, LONG * plPartID)
	{
		return E_NOTIMPL; 
		
		//if (pbHit == NULL || plPartID)
		//	return E_POINTER; 

		////if(m_pHost->HitTest(pt) )
		////{
		////	*pbHit = TRUE;			
		////}
		////else
		////{
		//*pbHit = FALSE;			
		////}

		//return E_NOTIMPL; 
	} 

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
	{ 
		if(IsEqualIID(riid,IID_IUnknown)) 
		{ 
			*ppv = static_cast <IUnknown*> (this); 
			return S_OK; 
		} 
		else if(IsEqualIID(riid,IID_IElementBehavior)) 
		{ 
			*ppv = static_cast <IElementBehavior*> (this); 
			return S_OK; 
		} 
		else if(IsEqualIID(riid,IID_IHTMLPainter)) 
		{ 
			*ppv = static_cast <IElementBehavior*> (this); 
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

#endif //__HTML_ELEMENT_PAINT_BEHAVIOR_H__