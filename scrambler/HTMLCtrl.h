 // Microsoft Systems Journal -- January 2000
 // If this code works, it was written by Paul DiLascia.
 // If not, I don't know who wrote it.
 // Compiles with Visual C++ 6.0, runs on Windows 98 and probably Windows NT too.
 //
#ifndef _HTML_CTRL_
#define _HTML_CTRL_

class CHTMLCtrl : public CHtmlView 
{
public:
	
	CHTMLCtrl();
	~CHTMLCtrl();

	// Generated message map functions
	//{{AFX_MSG(CHTMLCtrl)
	// overrides to bypass MFC doc/view frame dependencies
	afx_msg void OnDestroy();
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd,UINT nHitTest,UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CHTMLCtrl)

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHTMLCtrl)
	public:
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	virtual void PostNcDestroy() {};
	virtual void OnAppCmd(LPCTSTR lpszWhere);
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	//}}AFX_VIRTUAL


	//----------------------------------------------
	// INIT. MICROSOFT BUG CORRECTION
	//----------------------------------------------
	CString GetFullName() const;
	CString GetType() const;
	CString GetLocationName() const;
	CString GetLocationURL() const;

	void Navigate(LPCTSTR lpszURL, DWORD dwFlags = 0 ,  
								LPCTSTR lpszTargetFrameName = NULL ,  
								LPCTSTR lpszHeaders = NULL, LPVOID lpvPostData = NULL,  
								DWORD dwPostDataLen = 0);

	//Add the following functions. This action prevents the leak because the 
	//CHtmlView versions of LoadFromResource() call CHtmlView::Navigate().
	//These need to be defined because CHtmlView::Navigate() is not virtual.
	BOOL LoadFromResource(LPCTSTR lpszResource);
	BOOL LoadFromResource(UINT nRes);

	//----------------------------------------------
	// END MICROSOFT BUG CORRECTION
	//----------------------------------------------

	
	// Methods...
	BOOL CreateFromStatic(UINT nID, CWnd* pParent);

	bool SetNewHTMLContent(CString csHTML, bool bReformat = false);

	bool ParseHTMLString(CString &csCadenaATractar);

private:

	// Functions:
	bool SetHTMLInDocument();


	// Members.
	CString m_csHTMLContent;

	bool m_bHTMLDocumentLoadedOK;
};

#endif
