// SignInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "SignInDlg.h"
#include "afxdialogex.h"
#include "ClientDlg.h"


// SignInDlg dialog

IMPLEMENT_DYNAMIC(SignInDlg, CDialogEx)

SignInDlg::SignInDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(SignInDlg::IDD, pParent) {
    isConnected = false;    
}

SignInDlg::~SignInDlg() {}

void SignInDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SignInDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &SignInDlg::OnBnClickedOk)
    ON_MESSAGE(WM_SOCKET, handleEvents)
    ON_BN_CLICKED(IDC_BUTTON1, &SignInDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// SignInDlg message handlers

void SignInDlg::connectToServer() {
    if (isConnected == false) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            return;
        }

        UpdateData();
        client = socket(AF_INET, SOCK_STREAM, 0);
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(PORT);
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

        int err = connect(client, (SOCKADDR*)&serverAddress, sizeof serverAddress);
        if (err == SOCKET_ERROR) {
            return;
        }
        isConnected = true;
        WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
    }
}

Auth SignInDlg::getAuthInfo() {
    CString username;
    CString password;
    GetDlgItemText(IDC_EDIT1, username);
    GetDlgItemText(IDC_EDIT2, password);

    Auth auth;
    strcpy(auth.username, convertToChar(username));
    strcpy(auth.password, convertToChar(password));    
    strcpy(this->username, convertToChar(username));
    return auth;
}

void SignInDlg::OnBnClickedOk() {
    connectToServer();
    Auth auth = getAuthInfo();
    Message msg;
    strcpy(msg.action, "login");
    memcpy(msg.content, (char*) &auth, sizeof auth);
    sendTo(client, msg);
}

void SignInDlg::OnBnClickedButton1() {
    connectToServer();
    Auth auth = getAuthInfo();
    Message msg;
    strcpy(msg.action, "register");
    memcpy(msg.content, (char*) &auth, sizeof auth);
    sendTo(client, msg);
}

LRESULT SignInDlg::handleEvents(WPARAM wParam, LPARAM lParam) {
    if (WSAGETSELECTERROR(lParam)) {
        closesocket(wParam);
        return 0;
    }
    switch (WSAGETSELECTEVENT(lParam)) {
        case FD_READ: {
            Message msg;
            receive(wParam, msg);
            if (strcmp(msg.action, "login-response") == 0) {
                if (strcmp(msg.content, "OK") == 0) {
                    CClientDlg dlg(this->username);
                    EndDialog(0);
                    dlg.DoModal();
                } else {
                    MessageBox(CString(msg.content));
                }
            }
            break;
        }
    }
    return 0;
}

void SignInDlg::OnCancel() {
    if (isConnected) {
        closesocket(client);
    }
    CDialogEx::OnCancel();
}
