/////////////////////////////////////////////////////////////////////
// ATNlmPg.h : header file
//
// Copyright 1992-1996 Symantec, Peter Norton Product Group
/////////////////////////////////////////////////////////////////////
// $Header:   S:/ALERTUI/VCS/ATNLMPG.H_v   1.1   25 Apr 1997 11:04:50   JBRENNA  $
//
// Description:
//
/////////////////////////////////////////////////////////////////////
//
// $Log:   S:/ALERTUI/VCS/ATNLMPG.H_v  $
// 
//    Rev 1.1   25 Apr 1997 11:04:50   JBRENNA
// Ported from Tesla.
// 
//    Rev 1.0   15 Apr 1997 20:08:56   sedward
// Initial revision.
//
//
/////////////////////////////////////////////////////////////////////


#ifndef ALERT_NLM_PROPERTY_PAGE_HEADER
#define ALERT_NLM_PROPERTY_PAGE_HEADER


#include "ATMachPg.h"


#define MASTER_NLM_ITEM_INDEX       0   // position in the list control



/////////////////////////////////////////////////////////////////////////////
//
// CAlertNlmPropertyPage
//
/////////////////////////////////////////////////////////////////////////////

class CAlertNlmPropertyPage : public CAlertMachinePropertyPage
{
    DECLARE_DYNCREATE(CAlertNlmPropertyPage)

// Construction
public:
    CAlertNlmPropertyPage();
    CAlertNlmPropertyPage(UINT  dlgID);
    ~CAlertNlmPropertyPage();

// Dialog Data
    //{{AFX_DATA(CAlertNlmPropertyPage)
	enum { IDD = IDD_NLM_PROP_PAGE };
	CButton	m_allNlmCheckbox;
	CButton	m_removeButton;
	CButton	m_modifyButton;
	CButton	m_addButton;
	CListCtrl	m_listCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CAlertNlmPropertyPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
    //{{AFX_MSG(CAlertNlmPropertyPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnIncludeAllNlmsCheckbox();
	afx_msg void OnClickNlmTargetList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkNlmTargetList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Functions
protected:
    BOOL                    InitializeData(void);

public:
    virtual BOOL            CopyAlertTargetInfo(CAlertTarget*  source
                                                        , CAlertTarget*  dest);
    virtual BOOL            GetDisplayString(CAlertTarget*  pAlertTarget
                                            , CString&  displayStr, int  index);
    virtual CAlertTarget*   GetNewAlertTarget(void);
    virtual void            HandleRclickListCtrl(void);
    int                     InitListbox(void);
    BOOL                    MasterNLMItemInit(void);
    virtual BOOL            TargetIsActive(CAlertTarget*  pAlertTarget);
    BOOL                    UpdateMasterNLMItem(BOOL  checkedState);
    int                     UpdateNLMItemIcons(BOOL  checkedState);

// Data members
public:
};

#endif  // ALERT_NLM_PROPERTY_PAGE_HEADER