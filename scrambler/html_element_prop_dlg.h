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
#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "data_scrambler.h"
#include "xpath_edit_dlg.h"
#include "text_output_dlg.h"
#include "data_list_dlg.h"
#include "InPlaceEdit.h"
#include "GridListCtrl.h"
#include "pipe_line.h"
#include "DragEventListener.h"
#include "DragDropTreeCtrl.h"
#include "xpath_compare_dlg.h"
#include "OleRichEditCtrl.h"
#import "msxml.tlb"

#define WM_LOCATE_TO_HTML_ACTION ( WM_USER + 1011 )
#define WM_HTML_WORK_STOPPED ( WM_USER + 1012 )
#define WM_HTML_ELEMENTS_HILIGHT ( WM_USER + 1013 ) 
#define WM_DATA_OUTPUT ( WM_USER + 1014 ) 
#define WM_UPDATE_HTML_ACTION ( WM_USER + 1015 ) 
#define WM_UPDATE_EXIT_WORK_TIME_DELAY ( WM_USER + 1016 )
#define WM_ERROR_HANDLE ( WM_USER + 1017 )

#define OUTPUT_TEXTS 0x00000001
#define OUTPUT_LINKS 0x00000002
#define OUTPUT_HTML_ELEMENTS 0x00000003

//#define LOCATE_CURRENT_URL 0x00000001
#define LOCATE_CURRENT_ACTION 0x00000000

#include <list> 
typedef std::list< HTML_ELEMENT_ACTION* > HTML_ELEMENT_ACTION_LIST; 
typedef HTML_ELEMENT_ACTION_LIST::iterator HTML_ELEMENT_ACTION_LIST_ITERATOR; 

#define INVALID_JUMP_TO_ID 0xffffffff
typedef enum _HTML_INSTRUCTION_CONFIGURE_STATE
{
	HTML_INSTRUCTION_SET_JUMP_TO = 1, 
	HTML_INSTRUCTION_CUT = 2, 
	HTML_INSTRUCTION_COPY = 4, 
	HTML_INSTRUCTION_CONTENT_PROCESS = 8, 
	MAX_HTML_INSTRUCTION_STATE, 
} HTML_INSTRUCTION_CONFIGURE_STATE, *PHTML_INSTRUCTION_CONFIGURE_STATE; 

typedef vector< HTML_ELEMENT_ACTION* > ELEMENT_CONFIG_LIST; 
typedef ELEMENT_CONFIG_LIST::iterator ELEMENT_CONFIG_LIST_ITERATOR; 

typedef LRESULT ( CALLBACK *html_element_action_send_func )( PVOID data, ULONG data_size ); 
typedef LRESULT ( CALLBACK *html_element_action_recv_func )( PVOID *data, ULONG *data_size ); 
typedef LRESULT ( CALLBACK *html_element_action_free_func )( PVOID *data, ULONG *data_size ); 

typedef struct _element_action_handler
{
	html_element_action_send_func send_data; 
	html_element_action_recv_func recv_data; 
	html_element_action_free_func free_data; 
} element_action_handler, *pelement_action_handler; 

typedef struct _SCRIPT_WORKER
{
	CRITICAL_SECTION lock; 
	HTML_ELEMENT_ACTION *action; 
	HANDLE work_notifier; 
	HANDLE state_notifier; 
	HANDLE worker; 
	BOOLEAN stop_work; 
	BOOLEAN running; 
	BOOLEAN exit; 
} SCRIPT_WORKER, *PSCRIPT_WORKER; 

typedef struct _HTML_ACTION_OUTPUT_PARAM
{
	HTML_ACTION_CONTEXT *context; 
	HTML_ELEMENT_ACTION *container; 
    HTML_ELEMENT_ACTION *action;
} HTML_ACTION_OUTPUT_PARAM, *PHTML_ACTION_OUTPUT_PARAM; 

typedef struct _HTML_ACTION_EXECUATE_PARAM
{
	wstring locate_url; 
	HTML_ELEMENT_ACTION *action; 
} HTML_ACTION_EXECUATE_PARAM, *PHTML_ACTION_EXECUATE_PARAM; 

typedef struct _HTML_SCRIPT_INSTANCE
{
	wstring begin_url; 
	wstring location_url; 
	LONG loop_count; 
	HTML_ELEMENT_ACTION *actions; 
	SCRIPT_WORKER worker; 
} HTML_SCRIPT_INSTANCE, *PHTML_SCRIPT_INSTANCE; 

typedef vector<HTML_SCRIPT_INSTANCE*> HTML_SCRIPT_INSTANCES; 

typedef struct _HTML_ELEMENT_PROPERTIES
{
	wstring field_name; 
} HTML_ELEMENT_PROPERTIES, *PHTML_ELEMENT_PROPERTIES; 

#define HTML_ELEMENT_SCRIPT_FLAG_POSTED 0x00000001

LRESULT WINAPI get_html_element_action_id( HTML_ELEMENT_ACTION *root, ULONG *id ); 
LRESULT WINAPI get_script_file_flag( LPCWSTR file_name, ULONG *flags ); 
LRESULT WINAPI set_script_file_flag( LPCWSTR file_name, ULONG flags ); 
LRESULT WINAPI uninit_config_list(); 
LRESULT WINAPI init_config_list(); 
LRESULT WINAPI html_script_worker_construct( SCRIPT_WORKER *worker ); 
LRESULT WINAPI html_script_instance_construct( HTML_SCRIPT_INSTANCE *instance ); 
LRESULT WINAPI disconnect_all_web_browser_processes(); 

typedef struct _HTML_SCRIPT_INFO
{
	ULONG get_data_time; 
	ULONG get_link_time; 
	wstring file_name; 
	BOOLEAN share; 
	BOOLEAN shared; 
	BOOLEAN valid; 
} HTML_SCRIPT_INFO, *PHTML_SCRIPT_INFO; 

LRESULT WINAPI share_html_script( HTML_SCRIPT_INFO *info ); 
LRESULT WINAPI init_html_script_info( HTML_SCRIPT_INFO *info, 
									 LPCWSTR file_name );  

LRESULT WINAPI increment_script_run_time( HTML_SCRIPT_INFO *info, 
										 BOOLEAN data, 
										 BOOLEAN links ); 

LRESULT WINAPI set_html_script_info( HTML_SCRIPT_INFO *info, 
									BOOLEAN share ); 

LRESULT WINAPI execute_html_commmand( HTML_ELEMENT_ACTION *action, LPCWSTR locate_url ); 

LRESULT WINAPI exit_html_script_worker( SCRIPT_WORKER *worker ); 
LRESULT WINAPI stop_html_script_work( HTML_SCRIPT_INSTANCE *instance); 
LRESULT WINAPI start_html_script_work( HTML_SCRIPT_INSTANCE *instance ); 
LRESULT WINAPI free_element_config_tree( HTML_ELEMENT_ACTION *action ); 
LRESULT WINAPI create_html_action_script_worker( SCRIPT_WORKER *worker ); 
LRESULT WINAPI _get_container_content_info( HTML_ELEMENT_ACTION *action, 
										  HTML_ACTION_CONTEXT *context, 
										  HTML_ELEMENT_ACTIONS *page_content ); 
LRESULT WINAPI get_container_content_info( HTML_ELEMENT_ACTION *action, 
										  HTML_ACTION_CONTEXT *context, 
										  HTML_ELEMENT_ACTIONS *page_content ); 

LRESULT WINAPI config_html_element_text(HTML_ELEMENT_ACTION *dest, HTML_ELEMENT_ACTION *src);

LRESULT WINAPI copy_html_element_action( HTML_ELEMENT_ACTION *dest, HTML_ELEMENT_ACTION *src ); 

LRESULT WINAPI exec_html_action_script( HTML_ELEMENT_ACTION *root, element_action_handler *handler); 

LRESULT WINAPI get_html_text_field_name( IHTMLElement *html_element, 
										ULONG action_id, 
										wstring &field_name ); 

LRESULT WINAPI release_html_script_instances( vector<HTML_SCRIPT_INSTANCE*> *instances ); 

#include "data_analyze.h"
#include "D:\dev_programs\vs2008\VC\atlmfc\include\afxcmn.h"
class html_element_prop_dlg : public CDialog, CDragEventListener
{
	DECLARE_DYNAMIC(html_element_prop_dlg)

public:
	html_element_prop_dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~html_element_prop_dlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_OBJECT_PROPERTY };

	VOID set_html_element_xpath(LPCWSTR xpath ); 

#define INNER_LINK 0x00000001

    LRESULT get_html_elements_info(HTML_ELEMENT_ACTION *action, HTML_ELEMENT_INFOS &infos, ULONG flags ); 
    LRESULT get_html_elements(LPCWSTR xpath, HTML_ELEMENT_GROUP &elements, BOOLEAN analyze_xpath); 
    LRESULT config_action_type_from_xpath(LPCWSTR xpath); 

	VOID receive_html_element_xpath()
	{
		wstring xpath; 
		xpath_dlg.get_xpath_result( xpath ); 

		SetDlgItemText( IDC_EDIT_XPATH, xpath.c_str() ); 

        config_action_type_from_xpath(xpath.c_str()); 
	}

	VOID set_html_element( PVOID html_element ); 

	LRESULT get_active_element_properties( HTML_ELEMENT_PROPERTIES *properties ); 
	LRESULT init_element_properties( LPCWSTR std_xpath ); 
    LRESULT edit_html_action(HTREEITEM selected_item); 

protected:

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	PVOID element; 
	CComboBox element_actions;
	wstring xpath; 
    //HTML_ELEMENT_PROPERTIES properties;
	//browser_test_dlg *main_browser_dlg; 

public:
	afx_msg void OnBnClickedButtonXpathValidate();
	afx_msg void OnCbnSelendokComboAction(); 

    LRESULT on_html_document_complete(LPCWSTR url); 

	LRESULT attach_element_config( HTREEITEM item, 
		HTML_ELEMENT_ACTION *config ); 

	LRESULT set_element_config( HTREEITEM item, 
		HTML_ELEMENT_ACTION *config ); 
	LRESULT detach_element_config( HTREEITEM item, 
		HTML_ELEMENT_ACTION *config ); 

	LRESULT free_tree_item_config( HTREEITEM tree_item, PVOID data, PVOID context ); 


	typedef LRESULT ( __thiscall html_element_prop_dlg::*traverse_tree_item_func )( HTREEITEM item, 
		PVOID data, 
		PVOID context ); 

	LRESULT traverse_tree_item_in_order( HTREEITEM item, 
		traverse_tree_item_func func, 
		PVOID context ); 

	LRESULT start_html_script_work_ex( HTML_ELEMENT_ACTION *action ); 
	LRESULT copy_actions( HTML_ELEMENT_ACTION *action, HTML_ELEMENT_ACTION **actions_out ); 

	LRESULT add_item_action_to_list( HTREEITEM item, 
		PVOID data, 
		PVOID context ); 

	LRESULT expand_html_action_script( HTREEITEM item, 
		HTML_ELEMENT_ACTION *root, 
		HTML_ELEMENT_ACTION_LIST &actions ); 

	LRESULT get_tree_item_config( HTREEITEM item, 
		HTML_ELEMENT_ACTION **action ); 
	
	LRESULT get_tree_item_configs( HTREEITEM item, 
		HTML_ELEMENT_ACTION_LIST &actions ); 

	LRESULT get_max_html_action_id( ULONG *id ); 
	LRESULT _allocate_element_config( HTML_ELEMENT_ACTION **config ); 
    LRESULT update_base_html_element_action_id(HTML_ELEMENT_ACTION *action); 
	LRESULT allocate_html_element_action( HTML_ELEMENT_ACTION **config ); 
	LRESULT allocate_element_config_ex( HTML_ELEMENT_ACTION **config ); 
	LRESULT free_element_config( HTML_ELEMENT_ACTION *config ); 

    LRESULT load_instruction_page(LPCWSTR prefix,
        LPCWSTR name,
        HTML_PAGE_INFO *page_info,
        HTREEITEM parent,
        HTREEITEM *item_out); 

	LRESULT load_html_element( 
		MSXML::IXMLDOMDocument *xml_doc, 
		MSXML::IXMLDOMNode *xml_node, 
		HTREEITEM parent, 
		HTREEITEM *item_out ); 

	LRESULT detach_free_element_config( HTREEITEM item, HTML_ELEMENT_ACTION *config )
	{
		LRESULT ret; 

		do 
		{
			ret = detach_element_config( item, config ); 
			if( ret != ERROR_SUCCESS )
			{
				dbg_print( MSG_FATAL_ERROR, "detach the element configure error 0x%0.8x\n", ret ); 
				break; 
			}

			ret = free_element_config( config ); 
		}while( FALSE ); 

		return ret; 
	}

	LRESULT allocate_attach_element_config( HTREEITEM item, 
		HTML_ELEMENT_ACTION **config )
	{
		LRESULT ret; 

		do 
		{
			ret = allocate_html_element_action( config ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}

			ret = attach_element_config( item, *config ); 
			if( ret != ERROR_SUCCESS )
			{
				break; 
			}
		}while( FALSE );

		return ret; 
	}

	LRESULT prepare_data_output_config( HTML_ELEMENT_ACTION *root, LPCWSTR url ); 

	void set_tip( LPCWSTR tip_text ); 

	LRESULT load_html_elements_properties( HTML_SCRIPT_INSTANCES *instances ); 
	LRESULT _html_element_action_test( HTML_ELEMENT_ACTION *config ); 
    LRESULT on_error(WPARAM wparam, LPARAM lparam); 
	LRESULT on_xpath_config( WPARAM wparam, LPARAM lparam ); 
	LRESULT on_locate_to_action( WPARAM wparam, LPARAM lparam ); 
	LRESULT find_html_action_item( HTML_ELEMENT_ACTION *action, HTREEITEM *item_out ); 
#define UPDATE_ROOT_HTML_ACTION 0x00000001

	LRESULT user_output_data( DATA_STORE *store, 
		HTML_ELEMENT_ACTIONS *content); 
	LRESULT on_data_output( WPARAM wparam, LPARAM lparam ); 
	LRESULT on_update_html_action( WPARAM wparam, LPARAM lparam ); 
	LRESULT on_work_stoppped( WPARAM wparam, LPARAM lparam ); 
	LRESULT on_hilight( WPARAM wparam, LPARAM lparam ); 

    LRESULT output_data_to_list(HTML_ELEMENT_ACTION *action); 

	LRESULT save_html_script( LPCWSTR file_name, vector<HTML_SCRIPT_INSTANCE*> *instances ); 

	LRESULT update_html_script( LPCWSTR file_name, 
		LPCWSTR location_url ); 

	HTREEITEM insert_tree_item( ULONG action, 
		LPCWSTR item_text, 
		HTREEITEM parent_item, 
		HTREEITEM after_item ); 

	HTREEITEM _insert_tree_item( LPCWSTR action, 
		LPCWSTR item_text, 
		HTREEITEM parent_item, 
		HTREEITEM after_item ); 

	LRESULT get_html_script_instances(vector<HTML_SCRIPT_INSTANCE*> *instances); 
	LRESULT add_jump_to_instruction( HTREEITEM select_item, HTREEITEM item ); 
	LRESULT empty_tree_elements(); 
	LRESULT get_xpath_links( LPCWSTR xpath, LINK_LIST &links, BOOLEAN analyze_xpath ); 
	LRESULT delete_tree_item(HTREEITEM item ); 
	LRESULT delete_tree_item_siblings(HTREEITEM item); 
	LRESULT _delete_tree_item(HTREEITEM item); 
	LRESULT get_xpath_content( LPCWSTR xpath, wstring &content, BOOLEAN analyze_xpath ); 
	LRESULT get_container_element( HTREEITEM item, HTREEITEM *container_item ); 
	LRESULT get_element_config_tree( HTML_ELEMENT_ACTION **root ); 
	HTREEITEM get_tree_hit_item(); 
	LRESULT clear_selection(); 
	LRESULT show_html_action_param_control( HTML_ELEMENT_ACTION_TYPE type ); 
	LRESULT get_html_script_locate_url_by_user( wstring &url ); 
	void on_exit(); 
	LRESULT end_data_scramble(); 
	LRESULT move_tree_item( HTREEITEM item_source, HTREEITEM item_dest ); 
	LRESULT copy_tree_item( HTREEITEM item_source, HTREEITEM item_dest ); 
	
	LRESULT OnDataScramble( WPARAM wparam, 
		LPARAM lparam) ; 

	LRESULT data_output_config( LPCWSTR name, CONTAINER_INFOS *container_infos ); 
	LRESULT html_element_action_test(); 
	LRESULT get_html_action_output( HTREEITEM selected_item, HTML_ELEMENT_ACTION *config ); 
	LRESULT get_script_instances_from_ui(); 
	LRESULT get_html_script_instance( INT32 i, HTML_SCRIPT_INSTANCE *instance ); 
    LRESULT resort_html_actions_id(); 

#define SHOW_UI 0x00000001
    LRESULT save_html_script_ex(ULONG flags);

	void OnDragMove(POINT point);
	void OnDragRelease(POINT point,HTREEITEM hDropTarget);
	void OnDrag(); 
    
    LRESULT _get_html_element_action_info_from_ui(HTML_ELEMENT_ACTION *action,
        wstring &element_text,
        ULONG &action_type); 

	afx_msg void OnBnClickedButtonAdd();
protected:
	virtual void OnCancel();

public:
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonFileBrowser();
	afx_msg void OnBnClickedButtonExecute();
	//afx_msg void OnHdnItemdblclickListElements(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnNMDblclkListElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonXpathEdit();

	afx_msg LRESULT on_update_exit_work_time_delay( WPARAM wparam, 
		LPARAM lparam); 
	
	//afx_msg void OnNMClickListElements(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnNMRClickListElements(NMHDR *pNMHDR, LRESULT *pResult);

	xpath_compare_dlg *get_compare_dlg() { return &compare_dlg; }

protected:
	ULONG configure_state; //HTML_INSTRUCTION_CONFIGURE_STATE
	HTREEITEM jump_from_item; 
	HTREEITEM cut_copy_item; 
	xpath_edit_dlg xpath_dlg; 
	xpath_compare_dlg compare_dlg; 
	text_output_dlg output_dlg; 
    //scrapy_edit_dlg python_scrapy_dlg; 
	CComboBox contant_type; 
	CRITICAL_SECTION config_tree_lock; 
	BOOLEAN	script_running; 
	data_list_dlg data_dlg; 
	CInPlaceEdit *m_pListEdit; 
	HANDLE exit_script_work_thread_handle; 
	HANDLE exist_work_event; 
	//ELEMENT_CONFIG *root; 
	CImageList tree_image_list;
	CImageList* tree_drag_image_list;
	CDragDropTreeCtrl tree_elements;
	CEdit input_content;
	CButton open_new_page;
	CComboBox output_method;
	CEdit output_content_path;
	CEdit field_name;
	CSliderCtrl web_page_load_time;
	CSliderCtrl process_delay_time;
	//CListCtrl locate_urls; 
	CGridListCtrl locate_urls; 
	CGridListCtrl data_set; 
    BOOLEAN instruction_chnaged;

public:
	afx_msg void OnNMClickTreeElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTreeElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTreeElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRDblclkTreeElements(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonReplace();
	afx_msg LRESULT get_html_element_action_info_from_ui( HTML_ELEMENT_ACTION *config ); 
	afx_msg void OnBnClickedButtonOutputMethodConfirm(); 

	afx_msg void OnBeginUrlslabeleditList(NMHDR* pNMHDR, LRESULT* pResult);  
	afx_msg void OnEndUrllabeleditList(NMHDR* pNMHDR, LRESULT* pResult); 

	//afx_msg void OnBeginDataSetlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);  
	//afx_msg void OnEndDataSetUrllabeleditList(NMHDR* pNMHDR, LRESULT* pResult); 

	afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult);  
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, ULONG control_id, LRESULT* pResult); 

	afx_msg void OnBnClickedButtonOutputConfig();
	afx_msg void OnTRBNThumbPosChangingSliderWebPageLoadTime(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTRBNThumbPosChangingSliderProcessDelayTime(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonSaveHtmlScript();
	afx_msg void OnBnClickedButtonDataProcess();
	afx_msg void OnBnClickedButtonDataLearning();
	afx_msg void OnBnClickedButtonExitSystem();
	afx_msg void OnBnClickedButtonCancelExitSystem();
	afx_msg void OnNMDblclkListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListHtmlLocateUrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPlugins();
    COleRichEditCtrl scrapy_script_edit;
    COleRichEditCtrl property_edit;
    afx_msg void OnBnClickedButtonHelp();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedButtonXpathClean();
    afx_msg void OnBnClickedButtonWebPageLayout();
};
