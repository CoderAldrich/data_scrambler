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
#include "AfxOcc.h"
#include <MsHtmdid.h>
#include <afxctl.h>
#include "html_element_behavior.h"

#define HILIGHT_DEFLATE_PIXELS 2
#define HILIGHT_LINE_WIDTH 1
#define HILIGHT_LINE_COLOR RGB(0x9f, 0xe6, 0xac)

_COM_SMARTPTR_TYPEDEF(IHTMLEventObj2, __uuidof(IHTMLEventObj2));
_COM_SMARTPTR_TYPEDEF(IElementBehaviorFactory, __uuidof(IElementBehaviorFactory));

DEFINE_GUID(SID_SHTMLEditHost, 0x3050f6a0, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b);

//IMPLEMENT_DYNAMIC(html_element_behavior, CCmdTarget)

BEGIN_MESSAGE_MAP(html_element_behavior, CCmdTarget)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
//	Implementation of the html_element_behavior class, including the
//	IDocHostUIHandler implementation
BEGIN_INTERFACE_MAP(html_element_behavior, CCmdTarget)
	//INTERFACE_PART(CHTMLEditControlSite, IID_IDocHostUIHandler, DocHostUIHandler)
	//INTERFACE_PART(html_element_behavior, IID_IServiceProvider, ServiceProvider)
	//INTERFACE_PART(html_element_behavior, IID_IHTMLEditHost, HTMLEditHost)
	INTERFACE_PART(html_element_behavior, IID_IHTMLPainter, HTMLPainter)
	INTERFACE_PART(html_element_behavior, IID_IElementBehavior, HTMLElementBehavior)
	INTERFACE_PART(html_element_behavior, IID_IElementBehaviorFactory, HTMLElementBehaviorFactory)
	INTERFACE_PART(html_element_behavior, DIID_HTMLDocumentEvents2, Dispatch) 
END_INTERFACE_MAP()

BEGIN_DISPATCH_MAP(html_element_behavior, CCmdTarget)  
	DISP_FUNCTION_ID(html_element_behavior,"onclick", DISPID_HTMLELEMENTEVENTS2_ONCLICK, OnHtmlClick, VT_EMPTY, VTS_DISPATCH) 
END_DISPATCH_MAP() 

//html_element_behavior::html_element_behavior(COleControlContainer* pContainer) : COleControlSite(pContainer)
//{
// 	m_gridCookie = NULL;
//}

//inline CHtmlView* CHTMLEditControlSite::GetView() const
//{
//	return STATIC_DOWNCAST(CHtmlView, m_pCtrlCont->m_pWnd);
//}

void html_element_behavior::attatch_html_element(IHTMLElement *html_element)
{
	HRESULT hr;

	IHTMLElement2 *html_element2;
	VARIANT		  vFactory;

	if (html_element == (IHTMLElement *) NULL)
		return;

	hr = html_element->QueryInterface(IID_IHTMLElement2, (void **) &html_element2);

	if (html_element2 == (IHTMLElement2 *) NULL)
	{
		return;
	}

	if (m_gridCookie)
	{
		VARIANT_BOOL dummy;
		//BOOLEAN drawing; 

		//drawing = m_xHTMLPainter.drawing;

		//if( drawing != FALSE )
		//{
		//	m_xHTMLPainter.drawing = FALSE; 
		//}

		//if (m_xHTMLElementBehavior.m_spPaintSite != (IHTMLPaintSite *) NULL)
		//	m_xHTMLElementBehavior.m_spPaintSite->InvalidateRect(NULL); 

		//if( drawing != FALSE )
		//{
		//	m_xHTMLPainter.drawing = drawing; 
		//}

		hr = html_element2->removeBehavior(m_gridCookie, &dummy);

		//	MSHTML does an AddRef when you add a behaviour
		//	but it doesn't do a Release when you remove it.
		m_xHTMLElementBehaviorFactory.Release();
		m_gridCookie = NULL;
	}

	// Convert the grid factory pointer to the proper VARIANT data type 
	// for IHTMLElement2::AddBehavior
	V_VT(&vFactory) = VT_UNKNOWN;
	V_UNKNOWN(&vFactory) = &m_xHTMLElementBehaviorFactory;

	// Add Grid behavior
	hr = html_element2->addBehavior(NULL, &vFactory, &m_gridCookie);

	// Release resources
	hr = html_element2->Release();
	return;
}

void html_element_behavior::attatch_body_element(IHTMLDocument2 *pDoc)
{
	HRESULT hr;

	IHTMLElement  *pBody = NULL;
	IHTMLElement2 *pBody2;
    VARIANT		  vFactory;

	if (pDoc == (IHTMLDocument2 *) NULL)
		return;

	// Get IHTMLElement and IHTMLElement2 interfaces for the body
	hr = pDoc->get_body(&pBody);

	if (pBody == (IHTMLElement *) NULL)
		return;

	hr = pBody->QueryInterface(IID_IHTMLElement2, (void **) &pBody2);

	if (pBody2 == (IHTMLElement2 *) NULL)
	{
		pBody->Release();
		return;
	}

	if (m_gridCookie)
	{
		VARIANT_BOOL dummy;
		hr = pBody2->removeBehavior(m_gridCookie, &dummy);

		//	MSHTML does an AddRef when you add a behaviour
		//	but it doesn't do a Release when you remove it.
		m_xHTMLElementBehaviorFactory.Release();
		m_gridCookie = NULL;
	}

	// Convert the grid factory pointer to the proper VARIANT data type 
	// for IHTMLElement2::AddBehavior
	V_VT(&vFactory) = VT_UNKNOWN;
    V_UNKNOWN(&vFactory) = &m_xHTMLElementBehaviorFactory;

	// Add Grid behavior
	hr = pBody2->addBehavior(NULL, &vFactory, &m_gridCookie);

	// Release resources
	hr = pBody->Release();
	hr = pBody2->Release();
	return;
}

void html_element_behavior::set_drawing(BOOL bState)
{ 
	m_xHTMLPainter.drawing = bState;

	if (m_xHTMLElementBehavior.m_spPaintSite != (IHTMLPaintSite *) NULL)
		m_xHTMLElementBehavior.m_spPaintSite->InvalidateRect(NULL);
}

//	DocHostUIHandler methods.  These all delegate through the owner view
//	except for the standard IUnknown methods which delegate to the
//	CCmdTarget base class.
//STDMETHODIMP_(ULONG) CHTMLEditControlSite::XDocHostUIHandler::AddRef()
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler);
//	
//	return pThis->ExternalAddRef();
//}
//
//STDMETHODIMP_(ULONG) CHTMLEditControlSite::XDocHostUIHandler::Release()
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	return pThis->ExternalRelease();
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	return pThis->ExternalQueryInterface(&iid, ppvObj);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::GetExternal(LPDISPATCH *lppDispatch)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//	return pView->OnGetExternal(lppDispatch);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::ShowContextMenu(DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//	return pView->OnShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO *pInfo)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	CHtmlView* pView = pThis->GetView();
//	return pView->OnGetHostInfo(pInfo);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::ShowUI(DWORD dwID, LPOLEINPLACEACTIVEOBJECT pActiveObject, LPOLECOMMANDTARGET pCommandTarget, LPOLEINPLACEFRAME pFrame, LPOLEINPLACEUIWINDOW pDoc)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//	return pView->OnShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::HideUI(void)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnHideUI();
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::UpdateUI(void)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnUpdateUI();
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::EnableModeless(BOOL fEnable)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnEnableModeless(fEnable);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::OnDocWindowActivate(BOOL fActivate)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnDocWindowActivate(fActivate);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::OnFrameWindowActivate(BOOL fActivate)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnFrameWindowActivate(fActivate);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::ResizeBorder(LPCRECT prcBorder, LPOLEINPLACEUIWINDOW pUIWindow, BOOL fFrameWindow)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//
//	return pView->OnResizeBorder(prcBorder, pUIWindow, fFrameWindow);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::TranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//
//	return pView->OnTranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::GetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//
//	CHtmlView* pView = pThis->GetView();
//
//	return pView->OnGetOptionKeyPath(pchKey, dwReserved);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::GetDropTarget(LPDROPTARGET pDropTarget, LPDROPTARGET* ppDropTarget)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnGetDropTarget(pDropTarget, ppDropTarget);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::TranslateUrl(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnTranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
//}
//
//STDMETHODIMP CHTMLEditControlSite::XDocHostUIHandler::FilterDataObject(LPDATAOBJECT pDataObject, LPDATAOBJECT* ppDataObject)
//{
//	METHOD_PROLOGUE_EX_(CHTMLEditControlSite, DocHostUIHandler)
//	
//	CHtmlView* pView = pThis->GetView();
//	
//	return pView->OnFilterDataObject(pDataObject, ppDataObject);
//}

//	The service provider implementation. This is called by MSHTML to get
//	a pointer to an instance of IHTMLEditHost
STDMETHODIMP_(ULONG) html_element_behavior::XServiceProvider::AddRef()
{
	METHOD_PROLOGUE_EX_(html_element_behavior, ServiceProvider);

	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) html_element_behavior::XServiceProvider::Release()
{
	METHOD_PROLOGUE_EX_(html_element_behavior, ServiceProvider)

	return pThis->ExternalRelease();
}

STDMETHODIMP html_element_behavior::XServiceProvider::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE_EX_(html_element_behavior, ServiceProvider)

	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP html_element_behavior::XServiceProvider::QueryService(REFGUID guidService, REFIID riid, void **ppObj)
{
	METHOD_PROLOGUE_EX_(html_element_behavior, ServiceProvider)

	HRESULT hr = E_NOINTERFACE;

	*ppObj = NULL;

    if (guidService == SID_SHTMLEditHost && riid == IID_IHTMLEditHost)
    {
		*ppObj = (void **) &pThis->m_xHTMLEditHost;
		hr = S_OK;
	}

	return hr;
}

//	The edit host implementation, which is where the snap-to-grid is done
html_element_behavior::XHTMLEditHost::XHTMLEditHost()
{
	m_iSnap = 8;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLEditHost::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLEditHost::Release()
{
	return 1;
}

STDMETHODIMP html_element_behavior::XHTMLEditHost::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE(html_element_behavior, HTMLEditHost);

	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP html_element_behavior::XHTMLEditHost::SnapRect(IHTMLElement * /*pIElement*/, RECT * prcNew, ELEMENT_CORNER eHandle)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x10000000)
		//	If the control key is down return (no snap).
		return S_OK;

	LONG lWidth = prcNew->right - prcNew->left;
	LONG lHeight = prcNew->bottom - prcNew->top;

	switch (eHandle)
	{
	case ELEMENT_CORNER_NONE:
		prcNew->top = ((prcNew->top + (m_iSnap / 2)) / m_iSnap) * m_iSnap - 2;
		prcNew->left = ((prcNew->left + (m_iSnap / 2)) / m_iSnap) * m_iSnap - 2;
		prcNew->bottom = prcNew->top + lHeight;
		prcNew->right = prcNew->left + lWidth;
		break;

	case ELEMENT_CORNER_TOP:
		prcNew->top = ((prcNew->top + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_LEFT:
		prcNew->left = ((prcNew->left + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_BOTTOM:
		prcNew->bottom = ((prcNew->bottom + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_RIGHT:
		prcNew->right = ((prcNew->right + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_TOPLEFT:
		prcNew->top = ((prcNew->top + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		prcNew->left = ((prcNew->left + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_TOPRIGHT:
		prcNew->top = ((prcNew->top + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		prcNew->right = ((prcNew->right + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_BOTTOMLEFT:
		prcNew->bottom = ((prcNew->bottom + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		prcNew->left = ((prcNew->left + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;

	case ELEMENT_CORNER_BOTTOMRIGHT:
		prcNew->bottom = ((prcNew->bottom + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		prcNew->right = ((prcNew->right + (m_iSnap / 2)) / m_iSnap) * m_iSnap;
		break;
	}

	return S_OK;
}

//	The behavior implementation. This is queried by MSHTML to get an instance 
//	of an IHTMLPainter. Our painter draws the grid.
html_element_behavior::XHTMLElementBehavior::XHTMLElementBehavior()
{
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLElementBehavior::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLElementBehavior::Release()
{
	return 1;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehavior::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE(html_element_behavior, HTMLElementBehavior);

	if (IsEqualIID(iid, IID_IUnknown))
	{
		*ppvObj = (IUnknown *) pThis;
		return S_OK;
	}
	else if (IsEqualIID(iid, IID_IHTMLPainter))
	{
		*ppvObj = (IUnknown *) &pThis->m_xHTMLPainter;
		return S_OK;
	}

	*ppvObj = NULL;
	return E_NOTIMPL;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehavior::Init(IElementBehaviorSite *pBehaviorSite)
{
	HRESULT	hr = pBehaviorSite->QueryInterface(IID_IHTMLPaintSite, (void **) &m_spPaintSite);

	if (m_spPaintSite != (IHTMLPaintSite *) NULL)
		m_spPaintSite->InvalidateRect(NULL);

	return hr;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehavior::Notify(LONG /*lEvent*/, VARIANT *pVar)
{
	if (pVar == NULL)
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehavior::Detach()
{
	//m_spPaintSite->InvalidateRect(NULL);
	return S_OK;
}

//	The behavior factory
STDMETHODIMP_(ULONG) html_element_behavior::XHTMLElementBehaviorFactory::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLElementBehaviorFactory::Release()
{
	return 1;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehaviorFactory::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE(html_element_behavior, HTMLElementBehaviorFactory);

	*ppvObj = NULL;

	if (IsEqualIID(iid, IID_IElementBehaviorFactory))
	{
		*ppvObj = (IUnknown *) this;
		return S_OK;
	}
	else if (IsEqualIID(iid, IID_IElementBehavior))
	{
		*ppvObj = (IUnknown *) &pThis->m_xHTMLElementBehavior;
		return S_OK;
	}

	return E_NOTIMPL;
}

STDMETHODIMP html_element_behavior::XHTMLElementBehaviorFactory::FindBehavior(BSTR /*bstrBehavior*/, BSTR /*bstrBehaviorUrl*/, IElementBehaviorSite * /*pSite*/, IElementBehavior** ppBehavior)
{
	METHOD_PROLOGUE(html_element_behavior, HTMLElementBehaviorFactory)

	*ppBehavior = &pThis->m_xHTMLElementBehavior;
	return S_OK;
}

//	The painter part of the class
html_element_behavior::XHTMLPainter::XHTMLPainter()
{
	//m_iGrid = 32;
	drawing = TRUE;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLPainter::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) html_element_behavior::XHTMLPainter::Release()
{
	return 1;
}

STDMETHODIMP html_element_behavior::XHTMLPainter::QueryInterface(REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE(html_element_behavior, HTMLPainter);

	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP html_element_behavior::XHTMLPainter::Draw(RECT rcBounds, RECT /*rcUpdate*/, LONG /*lDrawFlags*/, HDC hdc, LPVOID /*pvDrawObject*/)
{
	INT32 i; 

	if (drawing != FALSE)
	{
		HPEN redPen	= (HPEN) CreatePen(PS_DOT, HILIGHT_LINE_WIDTH, HILIGHT_LINE_COLOR );
		HPEN oldPen = (HPEN) SelectObject(hdc, redPen);
		HBRUSH old_brush = ( HBRUSH )SelectObject( hdc, GetStockObject( HOLLOW_BRUSH ) ); 
#define GRID_WIDTH 8
		long lFirstLine = rcBounds.left + GRID_WIDTH;

		CRect rect_view = rcBounds;
		//rect_view.DeflateRect(1,1);
		//::Rectangle( hdc, rect_view.left + HILIGHT_LINE_WIDTH, 
		//	rect_view.top + HILIGHT_LINE_WIDTH, 
		//	rect_view.right - HILIGHT_LINE_WIDTH, 
		//	rect_view.bottom - HILIGHT_LINE_WIDTH ); 


#if 1
		for (i = lFirstLine; i <= rcBounds.right; i += GRID_WIDTH)
		{
			MoveToEx(hdc, i, rcBounds.top, NULL);
			LineTo(hdc, i, rcBounds.bottom);
		}

		lFirstLine = rcBounds.top + GRID_WIDTH;

		for (i = lFirstLine ; i <= rcBounds.bottom; i += GRID_WIDTH)
		{
			MoveToEx(hdc, rcBounds.left, i,  NULL);
			LineTo(hdc, rcBounds.right, i);
		}
#endif //0

		SelectObject( hdc, old_brush ); 
		SelectObject(hdc, oldPen);
		DeleteObject(redPen);
	}

	return S_OK;
}

STDMETHODIMP html_element_behavior::XHTMLPainter::OnResize(SIZE /*size*/)
{
	return S_OK;
}

STDMETHODIMP html_element_behavior::XHTMLPainter::GetPainterInfo(HTML_PAINTER_INFO *pInfo)
{
	if (pInfo == NULL)
		return E_POINTER;

	pInfo->lFlags = HTMLPAINTER_OPAQUE | HTMLPAINTER_NOSAVEDC; //HTMLPAINTER_TRANSPARENT;
	pInfo->lZOrder = HTMLPAINT_ZORDER_WINDOW_TOP; //HTMLPAINT_ZORDER_REPLACE_CONTENT; //HTMLPAINT_ZORDER_ABOVE_CONTENT; //HTMLPAINT_ZORDER_WINDOW_TOP; //HTMLPAINT_ZORDER_ABOVE_CONTENT; //HTMLPAINT_ZORDER_BELOW_CONTENT;
	
	memset(&pInfo->iidDrawObject, 0, sizeof(IID));

	pInfo->rcExpand.left = HILIGHT_DEFLATE_PIXELS;
	pInfo->rcExpand.right = HILIGHT_DEFLATE_PIXELS;
	pInfo->rcExpand.top = HILIGHT_DEFLATE_PIXELS;
	pInfo->rcExpand.bottom = HILIGHT_DEFLATE_PIXELS;

	return S_OK;
}

STDMETHODIMP html_element_behavior::XHTMLPainter::HitTestPoint(POINT /*pt*/, BOOL *pbHit, LONG *plPartID)
{
	if (pbHit == NULL)
		return E_POINTER;

	if (plPartID == NULL)
		return E_POINTER;

	return E_NOTIMPL;
}

HRESULT WINAPI add_html_element_behevior( IHTMLElement *html_element, 
										 html_element_behavior* handler )
{
	HRESULT hr = S_OK; 

	do 
	{
		ASSERT( NULL != html_element ); 

		handler->attatch_html_element( html_element ); 
		//long lBehaviorID;
		//VARIANT vBehavior;
		//IElementBehaviorFactoryPtr factory = NULL; 
		// Get the behavior factory interface for
		// the rendering behavior
		//hr = CoCreateInstance(CLSID_RenderingBehavior,
		//	NULL,
		//	CLSCTX_INPROC_SERVER,
		//	IID_IElementBehaviorFactory, (void**)&pRBFactory);

		// Convert pRBFactory to the proper VARIANT data type
		// for IHTMLElement2::AddBehavior

		//{
		//	IUnknown *unknown; 

		//	handler->QueryInterface( IID_IElementBehaviorFactory, ( PVOID*)&factory ); 
		//	if( NULL == factory )
		//	{
		//		hr = E_FAIL; 
		//		break; 
		//	}

		//	hr = factory->QueryInterface( IID_IUnknown, ( PVOID* )&unknown ); 
		//	if( FAILED(hr))
		//	{
		//		break; 
		//	}

		//	if( unknown == NULL )
		//	{
		//		hr = E_FAIL; 
		//		break; 
		//	}

		//	V_VT(&vBehavior) = VT_UNKNOWN;
		//	V_UNKNOWN(&vBehavior) = unknown;

		//	// AddRef because the pRBFactory pointer has been copied
		//	unknown->AddRef();
		//}

		//// Or Alternatively
		////vBehavior.vt = VT_UNKNOWN;
		////vBehavior.punkVal = pSnapperFactory;
		////vBehavior.punkVal->AddRef();

		//// Add behavior
		//{
		//	IHTMLElement2Ptr element; 
		//	hr = html_element->QueryInterface( IID_IHTMLElement2, ( PVOID* )&element); 
		//	if( FAILED(hr ))
		//	{
		//		break; 
		//	}

		//	if( NULL == element )
		//	{
		//		hr = E_FAIL; 
		//		break; 
		//	}

		//	hr = element->addBehavior(NULL,
		//		&vBehavior,
		//		&lBehaviorID);
		//}
	}while( FALSE );

	return hr; 
}

void html_element_behavior::OnHtmlClick(IHTMLEventObj *pEvtObj)
{
	//IHTMLElementPtr pElement; 
	//HRESULT	hr; 
	//IHTMLEventObj2Ptr pObjEvent2; 

	//do 
	//{
	//	hr = pEvtObj->put_returnValue(_variant_t(VARIANT_FALSE, VT_BOOL));
	//	if( FAILED( hr))
	//	{
	//	}

	//	hr	= pEvtObj->QueryInterface(IID_IHTMLEventObj2, ( PVOID*)&pObjEvent2);

	//	if( FAILED( hr))
	//	{
	//		break; 
	//	}

	//	if( NULL == pObjEvent2 )
	//	{
	//		break; 
	//	}

	//	hr = pEvtObj->get_srcElement(&pElement);
	//	if( FAILED( hr))
	//	{
	//		break; 
	//	}

	//	if( NULL == pElement )
	//	{
	//		break; 
	//	}

	//	//pObjEvent2->put_clientX(50);
	//	//pObjEvent2->put_clientY(50);

	//	while( pElement ) 
	//	{
	//		_bstr_t strTagname;
	//		pElement->get_tagName(&strTagname.GetBSTR());
	//		if(_bstr_t("a") == strTagname || _bstr_t("A") == strTagname)
	//		{

	//			_variant_t href; 
	//			
	//			pElement->getAttribute( L"href", 0, href.GetAddress() );
	//			break;
	//		}

	//		hr = pElement->get_parentElement( &pElement ); 
	//		if( FAILED(hr))
	//		{
	//			break; 
	//		}
	//	}
	//}while( FALSE ); 
}

HRESULT WINAPI set_event_handler_on_doc( IHTMLDocument2 *doc, html_element_behavior *handler, ULONG *cookie )
{
	HRESULT hr; 

	do 
	{
		ASSERT( NULL != doc ); 
		ASSERT( NULL != handler ); 

		hr	= AfxConnectionAdvise( doc, DIID_HTMLDocumentEvents2, handler->GetIDispatch(FALSE), FALSE, cookie ); 
	} while ( FALSE ); 

	return hr; 
} 

/*
void SDocEvtHandler::OnClick(MSHTML::IHTMLEventObjPtr pEvtObj)
{
MSHTML::IHTMLElementPtr pElement =
pEvtObj->GetsrcElement(); // 事件发生的对象元素
while(pElement) // 逐层向上检查
{
_bstr_t strTagname;
pElement->get_tagName(&strTagname.GetBSTR());
if(_bstr_t("a") == strTagname || _bstr_t("A") == strTagname)
{
// 已找到 "a" 标签，在这里写相应代码
// 例1：取得目标地址：
_variant_t vHref = pElement->getAttribute("href", 0);
// 例2：取消点击，禁止转到目标页面
pEvtObj->put_returnValue(_variant_t(VARIANT_FALSE, VT_BOOL));
break;
}
pElement = pElement->GetparentElement();
}
}*/