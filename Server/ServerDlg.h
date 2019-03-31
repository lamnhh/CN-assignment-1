#pragma once

#include "afxwin.h"
#include <afxsock.h>
#include <vector>
#include <string>
#include "helper.h"
#include "utils/sqlite3.h"
using namespace std;

struct Client {
    SOCKET socket;
    char username[20];
};

struct AuthInfo {
    CString username;
    CString password;
    bool loggedIn;
};

class CServerDlg : public CDialogEx {
public:
	CServerDlg(CWnd* pParent = NULL);
	enum { IDD = IDD_SERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);
    
    SOCKET server;
    SOCKADDR_IN serverAddress;
    vector<Client> clientList;
    vector<AuthInfo> authList;

    void fetchAllUsername(char*);
    void sendToAll(Message msg);

    sqlite3 *db;

public:
    afx_msg void OnBnClickedOk();
    virtual void OnCancel();
    CListBox logs;
};
