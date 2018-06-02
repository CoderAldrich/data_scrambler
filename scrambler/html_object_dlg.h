#pragma once
#include "afxcmn.h"


// html_object_dlg dialog

class html_object_dlg : public CDialog
{
	DECLARE_DYNAMIC(html_object_dlg)

public:
	html_object_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~html_object_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_OBJECT_PROPERTY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

public:
	LRESULT set_html_object( PVOID object ) 
	{
		html_object = object; 
		return ERROR_SUCCESS; 
	}

	LRESULT init_object_properties(); 
	LRESULT init_html_object_ui(); 

protected: 
	PVOID html_object; 
	virtual void OnOK();
	virtual void OnCancel();
public:
	CListCtrl html_objects_list;
};
