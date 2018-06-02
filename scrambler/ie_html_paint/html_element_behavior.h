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

#ifndef __HTML_ELEMENT_BEHAVIOR_H__
#define __HTML_ELEMENT_BEHAVIOR_H__

#include <afxocc.h>

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

class html_element_behavior : public CCmdTarget
{
public:
	
	html_element_behavior()
	{
		EnableAutomation(); 
		m_gridCookie = NULL;
	}

	//html_element_behavior(COleControlContainer* pParentWnd);
	void attatch_body_element(IHTMLDocument2 *pDoc); 
	void attatch_html_element(IHTMLElement *html_element); 

	BOOL		is_drawing()				{ return m_xHTMLPainter.drawing; }
	void		set_drawing(BOOL bState);

protected:
	LONG			m_gridCookie;

// Implementation
	DECLARE_INTERFACE_MAP()

	//	This is the implementation of the IServiceProvider interface
	//	MSHMTL gets this interface from us so we have to reference count it.
	BEGIN_INTERFACE_PART(ServiceProvider, IServiceProvider)
		STDMETHOD(QueryService)(REFGUID, REFIID, void **);
	END_INTERFACE_PART(ServiceProvider)

	//	This is the implementation of the IHTMLEditHost interface
	//	MSHMTL gets this interface from us so we have to reference count it.
	BEGIN_INTERFACE_PART(HTMLEditHost, IHTMLEditHost)
		STDMETHOD(SnapRect)(IHTMLElement *pIElement, RECT *prcNew, ELEMENT_CORNER eHandle);

					XHTMLEditHost();

		int			m_iSnap;
	END_INTERFACE_PART(HTMLEditHost)

	//	This is the implementation of the IHTMLPainter interface
	//	We pass this interface to MSHMTL so we don't reference count it.
	//	(It ain't going away unless the entire class goes away)
	BEGIN_INTERFACE_PART(HTMLPainter, IHTMLPainter)
		STDMETHOD(Draw)(RECT rcBounds, RECT rcUpdate, LONG lDrawFlags, HDC hdc, LPVOID pvDrawObject);
		STDMETHOD(OnResize)(SIZE size);
		STDMETHOD(GetPainterInfo)(HTML_PAINTER_INFO * pInfo);
		STDMETHOD(HitTestPoint)(POINT pt, BOOL* pbHit, LONG * plPartID);

						XHTMLPainter();

		BOOLEAN			drawing;
		//int				m_iGrid;
	END_INTERFACE_PART(HTMLPainter)

	//	This is the implementation of the HTMLElementBehavior interface
	//	We pass this interface to MSHMTL so we don't reference count it.
	//	(It ain't going away unless the entire class goes away)
	BEGIN_INTERFACE_PART(HTMLElementBehavior, IElementBehavior)
		STDMETHOD(Init)(IElementBehaviorSite *pBehaviorSite);
		STDMETHOD(Notify)(LONG lEvent, VARIANT * pVar);
		STDMETHOD(Detach)();

					XHTMLElementBehavior();

		IHTMLPaintSite		 *m_spPaintSite;
	END_INTERFACE_PART(HTMLElementBehavior)

	//	This is the implementation of the HTMLElementBehaviorFactory interface
	//	We pass this interface to MSHMTL so we don't reference count it.
	//	(It ain't going away unless the entire class goes away)
	BEGIN_INTERFACE_PART(HTMLElementBehaviorFactory, IElementBehaviorFactory)
		STDMETHOD(FindBehavior)(BSTR bstrBehavior, BSTR bstrBehaviorUrl, IElementBehaviorSite* pSite, IElementBehavior** ppBehavior);
	END_INTERFACE_PART(HTMLElementBehaviorFactory)

		DECLARE_DISPATCH_MAP() 
		 DECLARE_MESSAGE_MAP() 
		void OnHtmlClick(IHTMLEventObj *pEvtObj); 
};

HRESULT WINAPI add_html_element_behevior( IHTMLElement *html_element, 
										 html_element_behavior* handler );  

HRESULT WINAPI set_event_handler_on_doc( IHTMLDocument2 *doc, html_element_behavior *handler, ULONG *cookie ); 

#endif //__HTML_ELEMENT_BEHAVIOR_H__