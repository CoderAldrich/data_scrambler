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

#ifndef __BEHAVIOR_HANDLER_H__
#define __BEHAVIOR_HANDLER_H__

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include "behavior.h"
#include "behavior_factory.h"
#include <map>

typedef struct _BEHAVIOR_INFO
{
	IHTMLPaintSite *paint_site; 
	IHTMLElement *html_element; 
	ULONG behavior_id; 
	POINT pos; 
} BEHAVIOR_INFO, *PBEHAVIOR_INFO; 

#define HTML_ELEMENT_CLICK_BEHAVIOR "click" 
#define HTML_ELEMENT_HOVER_BEHAVIOR "hover" 

class html_behavior_handler
{
	enum { CLICK_STATE,OVER_STATE,OTHER_STATE};
	typedef std::map<IHTMLElement*,DWORD> AdviseEventMap;
	typedef std::map<std::string,BEHAVIOR_INFO*> BehaviorMap;
	BehaviorMap behaviors;
	CComPtr<IElementBehaviorFactory> m_spBehaviorFactory;	
	AdviseEventMap m_eventCookies;
	CPen m_pen;	
	UINT m_state; 
	BOOLEAN m_EnableNavigate; 
	CComPtr<IHTMLDocument2> m_html_doc;
	DWORD document_event_cookie;

public:	

	HRESULT WINAPI get_factory( IElementBehaviorFactory **factory )
	{
		ASSERT( NULL != factory ); 
		*factory = m_spBehaviorFactory; 
	
		return S_OK; 
	}

	html_behavior_handler():m_EnableNavigate(TRUE),m_state(OTHER_STATE)
	{ 
		document_event_cookie = 0xFEFEFEFE; 
		behavior_factory<html_behavior_handler> *factory = NULL;
		do 
		{
			factory = new behavior_factory<html_behavior_handler>(); 
			if( factory == NULL )
			{
				break; 
			}

			factory->construct(this);		
			m_spBehaviorFactory = NULL; 

			factory->QueryInterface(IID_IElementBehaviorFactory,(void**)&m_spBehaviorFactory);
			ASSERT( NULL != m_spBehaviorFactory);	
			
			m_pen.CreatePen(PS_SOLID,2,RGB(0,0,255)); 
			behaviors[HTML_ELEMENT_CLICK_BEHAVIOR] = new BEHAVIOR_INFO();
			behaviors[HTML_ELEMENT_HOVER_BEHAVIOR] = new BEHAVIOR_INFO();
		}while( FALSE );
	}		

	void init(IHTMLPaintSite * pSite)
	{
		BEHAVIOR_INFO * entry = NULL; 

		switch(m_state)
		{
		case CLICK_STATE:
			entry = behaviors[HTML_ELEMENT_CLICK_BEHAVIOR];			
			break;
		case OVER_STATE:
			entry = behaviors[HTML_ELEMENT_HOVER_BEHAVIOR];			
			break;
		} 

		if(entry)
		{
			entry->paint_site = pSite;
		}
	}

	void Detach()
	{	
		//TODO...DONOTHING...
	}

	void AddBehavior(IHTMLElement2 * html_element,LONG & ID)
	{
		ASSERT( ID == 0xEFFFFFFF); 

		VARIANT vBehavior;
		V_VT(&vBehavior) = VT_UNKNOWN;
		IUnknown * pUnk = NULL; 

		m_spBehaviorFactory.QueryInterface(&pUnk);
		V_UNKNOWN(&vBehavior) = pUnk;
		html_element->addBehavior(NULL,&vBehavior,&ID);
	}

	void RemoveBehavior(IHTMLElement2 * html_element,LONG & ID)
	{
		ASSERT( ID != 0xEFFFFFFF);
		VARIANT_BOOL bRet;
		html_element->removeBehavior(ID,&bRet);
		ID = 0xEFFFFFFF;
	}

	struct AdviseHelper;
	friend struct AdviseHelper;
	struct AdviseHelper
	{
		html_behavior_handler * pThis;
		AdviseHelper(html_behavior_handler * p):pThis(p)
		{}
		AdviseHelper():pThis(0){}

		bool operator()(IHTMLElement * pEle)
		{				
			DWORD dwCookie = 0xFEFEFEFE;
			AtlAdvise(pEle, (IUnknown*)this, DIID_HTMLElementEvents2, &dwCookie);
			pEle->AddRef();
			std::pair<AdviseEventMap::iterator,bool> p = pThis->m_eventCookies.insert(AdviseEventMap::value_type(pEle,dwCookie));			
			ASSERT( p.second );
			return true;
		}
	}; 

	void AdviseDocEvent( IHTMLDocument2 *html_doc)
	{
		m_html_doc = html_doc; 
		AtlAdvise(m_html_doc, (IUnknown*)this, DIID_HTMLDocumentEvents2, &document_event_cookie);

		AdviseHelper ah(this); 
		//Visitor(m_html_doc,ah); 	
		m_EnableNavigate = FALSE;
	}

	void UnAdviseDocEvent()
	{			
		DoDestroy();	
		m_EnableNavigate = TRUE;
	}

	void DoDestroy()
	{
		if(m_html_doc)
		{
			AtlUnadvise(m_html_doc,DIID_HTMLDocumentEvents2, document_event_cookie);
		}

		for(AdviseEventMap::iterator it =m_eventCookies.begin(); it != m_eventCookies.end();++it)
		{
			AtlUnadvise((*it).first, DIID_HTMLDocumentEvents2, (*it).second ); 
			CComQIPtr<IHTMLElement2> spEle2((*it).first);		
			(*it).first->Release();
		}
		m_eventCookies.clear();
	} 

	LRESULT find_paint_site( IHTMLElement *element, IHTMLPaintSite **paint_site )
	{
		LRESULT ret = ERROR_SUCCESS; 

		for( BehaviorMap::iterator it = behaviors.begin(); 
			it != behaviors.end(); 
			it ++ )
		{
			if( element == ( *it ).second->html_element )
			{
				*paint_site = ( *it ).second->paint_site; 
			}
		}

		return ret; 
	}

	void OnDraw(HDC dc, RECT const &rcBounds, LONG flags, RECT const &rcUpdate, IElementBehaviorSite * pSite ) 
	{			
		HTML_PAINT_DRAW_INFO pi = { 0 };
		CComPtr<IHTMLElement> html_element; 

		if(S_OK == pSite->GetElement(&html_element) && html_element )
		{			
			CComPtr<IHTMLPaintSite> pPaintSite; 

			find_paint_site(html_element, &pPaintSite); 
			if(pPaintSite)
			{
				pPaintSite->GetDrawInfo(HTMLPAINT_DRAWINFO_VIEWPORT,&pi);
				CRect rect_view = pi.rcViewport;
				rect_view.DeflateRect(1,1);
				::Rectangle( dc, rect_view.left, rect_view.top, rect_view.right, rect_view.bottom ); 
			}			
		}		
	}

	void DoDrawFoucs(CComPtr<IHTMLElement> pActiveElement,CComPtr<IHTMLElement> pEle,LONG ID)
	{		
		if(pEle && ID != 0xEFFFFFFF)
		{
			CComQIPtr<IHTMLElement2> spEle2(pEle);
			RemoveBehavior(spEle2,ID);			
		}

		pEle = NULL;
		CComBSTR tag;
		pActiveElement->get_tagName(&tag);
		//if(IsInlineTag(tag) )
		{
			CComPtr<IHTMLElement> parentElement;
			pActiveElement->get_parentElement(&parentElement);
			pEle = parentElement != NULL ? parentElement : pActiveElement;
		}
		//else
		//{
		//	pEle = pActiveElement;
		//}
		CComQIPtr<IHTMLElement2> spEle2(pEle);
		AddBehavior(spEle2,ID);	
	}

	void DoDrawFoucs(CComPtr<IHTMLElement> pActiveElement)
	{		
		if(m_state == CLICK_STATE)
		{
			BEHAVIOR_INFO * entry = behaviors[HTML_ELEMENT_CLICK_BEHAVIOR];
			DoDrawFoucs(pActiveElement,entry->html_element,entry->behavior_id);
			//m_timer.Stop();
			//m_timer.Start(700,TimerHelper(this));
		}
		else
		{			
			BEHAVIOR_INFO* entry = behaviors[HTML_ELEMENT_HOVER_BEHAVIOR];
			DoDrawFoucs(pActiveElement,entry->html_element,entry->behavior_id);		
		} 
	}

	CComPtr<IHTMLElement> HandleDrawEvent(IHTMLEventObj * evt,POINT & pt)
	{
		CComPtr<IHTMLElement> pActivedElement;
		if(!evt) return pActivedElement;		

		long x,y;
		evt->get_clientX(&x);
		evt->get_clientY(&y);	
		if(pt.x != x || pt.y != y)
		{
			pt.x = x;
			pt.y = y;
		}
		else
		{
			return pActivedElement;
		}		
		HRESULT hr = m_html_doc->elementFromPoint(x,y,&pActivedElement);		
		if(FAILED(hr))
		{
			pActivedElement = NULL;
			return pActivedElement;			
		}


		if( NULL == pActivedElement )
		{
			return pActivedElement;	
		}

		DoDrawFoucs(pActivedElement);		
		return pActivedElement;
	}

	void OnClick(IHTMLEventObj * evt)
	{			
		BEHAVIOR_INFO * entry = behaviors[HTML_ELEMENT_CLICK_BEHAVIOR];
		m_state = CLICK_STATE;
		CComPtr<IHTMLElement> pClickedElement = HandleDrawEvent(evt,entry->pos); 

		if(pClickedElement)
		{
			//::SendMessage(GetTopLevelWindow(),WM_BROWSERCLICK,(WPARAM)m_hWnd,(LPARAM)pClickedElement.p);
		}
	}

	void OnMouseOver(IHTMLEventObj * event_object)
	{		
		BEHAVIOR_INFO * entry = behaviors[HTML_ELEMENT_HOVER_BEHAVIOR];
		m_state = OVER_STATE;
		HandleDrawEvent(event_object,entry->pos);
	}
}; 

//HRESULT WINAPI add_html_element_behevior( IHTMLElement *html_element, 
//										 html_behavior_handler* handler ); 

#endif //__BEHAVIOR_HANDLER_H__