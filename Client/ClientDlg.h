
// ClientDlg.h : header file
//

#pragma once
#pragma pack(1)
#include <afxsock.h>
#include "afxwin.h"

#define PORT 25000
#define WM_SOCKET (WM_USER + 2)

struct Message {
    char action[30];
    char content[70];
};

// CClientDlg dialog
class CClientDlg : public CDialogEx
{
// Construction
public:
	CClientDlg(CWnd* pParent = NULL);	// standard constructor

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

    SOCKET client;
    SOCKADDR_IN serverAddress;

    void sendTo(SOCKET socket, Message msg);
    void receive(SOCKET socket, Message &msg);

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedCancel();
    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);
    CListBox logs;
};
