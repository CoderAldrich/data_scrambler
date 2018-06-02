// OleRichEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "OleRichEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COleRichEditCtrl

COleRichEditCtrl::COleRichEditCtrl()
{
	m_bCallbackSet = FALSE;
}

COleRichEditCtrl::~COleRichEditCtrl()
{
	// IExRichEditOleCallback class is a reference-counted class  
	// which deletes itself and for which delete should not be called

	// delete m_pIRichEditOleCallback;
}


BEGIN_MESSAGE_MAP(COleRichEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(COleRichEditCtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int COleRichEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
 	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
 		return -1;
 	
	// m_pIRichEditOleCallback should have been created in PreSubclassWindow

 	ASSERT( m_pIRichEditOleCallback != NULL );	

	// set the IExRichEditOleCallback pointer if it wasn't set 
	// successfully in PreSubclassWindow

	if ( !m_bCallbackSet )
	{
		SetOLECallback( m_pIRichEditOleCallback );
	}
 	
 	return 0;
}

void COleRichEditCtrl::PreSubclassWindow() 
{
	// base class first
	CRichEditCtrl::PreSubclassWindow();	

	m_pIRichEditOleCallback = NULL;
	m_pIRichEditOleCallback = new IExRichEditOleCallback(this);
	ASSERT( m_pIRichEditOleCallback != NULL );

	m_bCallbackSet = SetOLECallback( m_pIRichEditOleCallback );
}


long COleRichEditCtrl::StreamInFromResource(int iRes, LPCTSTR sType)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	HRSRC hRsrc = ::FindResource(hInst,
		MAKEINTRESOURCE(iRes), sType);
	
	DWORD len = SizeofResource(hInst, hRsrc); 
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc); 
	ASSERT(lpRsrc); 
 
	CMemFile mfile;
	mfile.Attach(lpRsrc, len); 

	EDITSTREAM es;
	es.pfnCallback = readFunction;
	es.dwError = 0;
	es.dwCookie = (DWORD) &mfile;

	return StreamIn( SF_RTF, es );
}


/* static */
DWORD CALLBACK COleRichEditCtrl::readFunction(DWORD dwCookie,
		 LPBYTE lpBuf,			// the buffer to fill
		 LONG nCount,			// number of bytes to read
		 LONG* nRead)			// number of bytes actually read
{
	CFile* fp = (CFile *)dwCookie;
	*nRead = fp->Read(lpBuf,nCount);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

//EDITSTREAM editstream;
//editstream.pfnCallback = EditStreamCallback;	//tell it the callback function
//editstream.dwCookie = 0; 
//editstream.dwError = 0; 

//SendMessage(scrapy_script_edit, EM_STREAMIN, SF_TEXT, (LPARAM)&editstream); //tell it to start stream in

COleRichEditCtrl::IExRichEditOleCallback::IExRichEditOleCallback( CWnd *wnd)
    : _this(wnd)
{
	pStorage = NULL;
	m_iNumStorages = 0;
	m_dwRef = 0;

	// set up OLE storage

	HRESULT hResult = ::StgCreateDocfile(NULL,
		STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE /*| STGM_DELETEONRELEASE */|STGM_CREATE ,
		0, &pStorage );

	if ( pStorage == NULL ||
		hResult != S_OK )
	{
		AfxThrowOleException( hResult );
	}
}

COleRichEditCtrl::IExRichEditOleCallback::~IExRichEditOleCallback()
{
}

HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::GetNewStorage(LPSTORAGE* lplpstg)
{
	m_iNumStorages++;
	WCHAR tName[50];
	swprintf(tName, L"REOLEStorage%d", m_iNumStorages);

	HRESULT hResult = pStorage->CreateStorage(tName, 
		STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
		0, 0, lplpstg );

	if (hResult != S_OK )
	{
		::AfxThrowOleException( hResult );
	}

	return hResult;
}

HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::QueryInterface(REFIID iid, void ** ppvObject)
{

	HRESULT hr = S_OK;
	*ppvObject = NULL;
	
	if ( iid == IID_IUnknown ||
		iid == IID_IRichEditOleCallback )
	{
		*ppvObject = this;
		AddRef();
		hr = NOERROR;
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	return hr;
}



ULONG STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::AddRef()
{
	return ++m_dwRef;
}



ULONG STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::Release()
{
	if ( --m_dwRef == 0 )
	{
		delete this;
		return 0;
	}

	return m_dwRef;
}


HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
	LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
	DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
	return S_OK;
}



HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)
{
	return S_OK;
}


HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

#include "resource.h"
HRESULT STDMETHODCALLTYPE 
COleRichEditCtrl::IExRichEditOleCallback::GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
    HMENU FAR *lphmenu)
{
    INT32 sub_menu_item; 
    CMenu menu;
    //METHOD_PROLOGUE_EX(COleRichEditCtrl, IExRichEditOleCallback);
    HMENU hMenu = NULL;

    menu.LoadMenu(IDR_MENU_EDITOR);//菜单资源ID
    POINT pt = { 0 };
    GetCursorPos(&pt);//得到鼠标点击位置
    sub_menu_item = menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, _this ); //m_newListCtrl是CListCtrl对象

    switch (sub_menu_item)
    {
    case ID_MENU_ITEM_COPY:
    {
        CRichEditCtrl *edit;
        LONG start_pos;
        LONG end_pos;
        CString text;
        PVOID _data;
        HGLOBAL data;

        edit = (CRichEditCtrl*)_this;

        //edit->GetSel(start_pos, end_pos);
        text = edit->GetSelText();
        //edit->SetSel(start_pos, start_pos + wcslen(std_xpath));

        if (text.GetLength() == 0)
        {
            break;
        }

        if (::OpenClipboard(_this->GetSafeHwnd()) == FALSE)
        {
            break;
        }

        ::EmptyClipboard();

        data = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, ((text.GetLength() + 1) << 1));
        _data = (LPSTR)GlobalLock(data);
        StringCchCopyW((LPWSTR)_data, ((text.GetLength() + 1) << 1), text.GetBuffer());
        GlobalUnlock(data);
        SetClipboardData(CF_TEXT, data);
        CloseClipboard();
    }
    break;
    case ID_MENU_ITEM_CLEAR:
    {
        CRichEditCtrl *edit;

        edit = (CRichEditCtrl*)_this;
        ::SendMessage(edit->GetSafeHwnd(), WM_SETTEXT, 0, (LPARAM)(LPCWSTR)L"");
    }
    break;
    case ID_MENU_ITEM_PASTE:
    {
        CRichEditCtrl *edit;
        HANDLE cliphandle = NULL; 
        PVOID _data; 
        SIZE_T data_size; 

        edit = (CRichEditCtrl*)_this;
        if (::OpenClipboard(edit->GetSafeHwnd() ) == false)
        {
            break; 
        }

        cliphandle = ::GetClipboardData(CF_TEXT);
        if (cliphandle == 0)//no text
        {
            CloseClipboard();
            break; 
        }

        _data = (LPWSTR)GlobalLock(cliphandle);
        data_size = GlobalSize(cliphandle);
        
        if (data_size == sizeof( WCHAR ) ) 
        {
            break; 
        }

        ( ( LPWSTR )_data )[(data_size >> 1) - 1] = L'\0'; 
        edit->ReplaceSel(( LPWSTR )_data, TRUE); 

        GlobalUnlock(cliphandle);
        CloseClipboard();
    }
        break; 
    default:
        break;
    }

    menu.DestroyMenu(); 

    //hMenu = menu.Detach();
    //if (hMenu == NULL)
    //{
    //    return E_NOTIMPL; 
    //    //break; 
    //}

    *lphmenu = hMenu;
    return S_OK;
}






