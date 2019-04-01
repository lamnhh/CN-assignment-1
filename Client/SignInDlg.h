#pragma once
#include <afxsock.h>
#include "socket/helper.h"

// SignInDlg dialog

class SignInDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SignInDlg)

public:
	SignInDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SignInDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


    bool isConnected;
    SOCKET client;
    SOCKADDR_IN serverAddress;
    char username[20];

    Auth getAuthInfo();
    void connectToServer();

public:
    afx_msg void OnBnClickedOk();
    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedButton1();
    virtual void OnCancel();
};
