#pragma once
#include <afxsock.h>
#include "afxwin.h"
#include "helper.h"

// CClientDlg dialog
class CClientDlg : public CDialogEx
{
// Construction
public:
	CClientDlg(char username[20], CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    char username[20];
    SOCKET client;
    SOCKADDR_IN serverAddress;

public:
    afx_msg void OnBnClickedOk();
    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);
    CListBox logs;
    CListBox userListBox;
};
