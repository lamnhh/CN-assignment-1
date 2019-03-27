
// ServerDlg.h : header file
//

#pragma once
#pragma pack(1)

#include "afxwin.h"
#include <afxsock.h>
#include <vector>
#include <string>
using namespace std;

#define PORT 25000
#define WM_SOCKET (WM_USER + 1)

struct Message {
    char action[30];
    char content[70];
};

struct Client {
    SOCKET socket;
    int id;
    static int count;
};

// CServerDlg dialog
class CServerDlg : public CDialogEx
{
// Construction
public:
	CServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SERVER_DIALOG };

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
    
    SOCKET server;
    SOCKADDR_IN serverAddress;
    vector<Client> clientList;

    void sendTo(SOCKET socket, Message msg);
    void receive(SOCKET socket, Message &msg);
    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);

    void sendToAll(Message msg);

public:
    afx_msg void OnBnClickedOk();
    CListBox logs;
};
