
// ClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"
#include "SignInDlg.h"
#include <cstdint>
#include <algorithm>
using namespace std;

#pragma pack(1)

struct UserList {
    int count;
    char list[30][20];
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CClientDlg dialog

CClientDlg::CClientDlg(char username[20], CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    strcpy(this->username, username);
    currentRoom = CString("General");
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, logs);
    DDX_Control(pDX, IDC_LIST2, userListBox);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_SOCKET, handleEvents)
    ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
    ON_LBN_SELCHANGE(IDC_LIST2, &CClientDlg::OnLbnSelchangeList2)
    ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    
	// TODO: Add extra initialization here
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        return TRUE;
    }

    UpdateData();
    client = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    int err = connect(client, (SOCKADDR*) &serverAddress, sizeof serverAddress);
    if (err == SOCKET_ERROR) {
        return TRUE;
    }
    WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);


    Message msg;
    strcpy(msg.action, "re-login");
    strcpy(msg.content, this->username);
    sendTo(client, msg);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*******************************************************************
********************************************************************
 * MY OWN CODE
********************************************************************
********************************************************************/

struct PrivateMessage {
    char receiver[20];
    char message[980];
};

void CClientDlg::OnBnClickedOk() {
    CString message;
    GetDlgItemText(IDC_EDIT1, message);
    SetDlgItemText(IDC_EDIT1, CString(""));
    
    Message msg;
    if (currentRoom == CString("General")) {
        strcpy(msg.action, "message-all");
        strcpy(msg.content, convertToChar(message));
    } else {
        static PrivateMessage pvt;
        strcpy(pvt.receiver, convertToChar(currentRoom));
        strcpy(pvt.message, convertToChar(message));
        
        strcpy(msg.action, "message-one");
        memcpy(msg.content, (char*) &pvt, sizeof pvt);
    }
    sendTo(client, msg);
}

void CClientDlg::fetchMessageList(MessageList list) {
    logs.ResetContent();
    for (int i = 0; i < (int)list.size(); ++i) {
        logs.AddString(list[i]);
    }
}

void CClientDlg::fetchUserList() {
    userListBox.ResetContent();
    for (int i = 0; i < (int)userList.size(); ++i) {
        char str[1000];
        if (userList[i].second > 0) {
            sprintf(str, "%s (%d)", convertToChar(userList[i].first), userList[i].second);
        } else {
            sprintf(str, "%s", convertToChar(userList[i].first));
        }
        userListBox.AddString(CString(str));
    }
}

LRESULT CClientDlg::handleEvents(WPARAM wParam, LPARAM lParam) {
    if (WSAGETSELECTERROR(lParam)) {
        closesocket(wParam);
        return 0;
    }
    switch (WSAGETSELECTEVENT(lParam)) {
        case FD_READ: {
            Message msg;
            receive(wParam, msg);

            if (strcmp(msg.action, "message-all") == 0) {
                messageList[CString("General")].push_back(unicode(msg.content));
                if (currentRoom != CString("General")) {
                    userList[0].second += 1;
                    fetchUserList();
                } else {
                    fetchMessageList(messageList[CString("General")]);
                }
            } else if (strcmp(msg.action, "message-one") == 0) {
                PrivateMessage pvt;
                memcpy(&pvt, msg.content, sizeof msg.content);
                messageList[CString(pvt.receiver)].push_back(unicode(pvt.message));
                if (currentRoom != pvt.receiver) {
                    CString sender(pvt.receiver);
                    for (int i = 0; i < (int)userList.size(); ++i) {
                        if (userList[i].first == sender) {
                            userList[i].second += 1;
                            break;
                        }
                    }
                    fetchUserList();
                } else {
                    fetchMessageList(messageList[CString(pvt.receiver)]);
                }
            } else if (strcmp(msg.action, "user-list") == 0) {
                UserList list;
                memcpy(&list, msg.content, sizeof msg.content);

                userList.clear();
                for (int i = 0; i < list.count; ++i) {
                    userList.push_back(make_pair(CString(list.list[i]), 0));
                }
                sort(userList.begin(), userList.end());
                userList.insert(userList.begin(), make_pair(CString("---------------"), -1));
                userList.insert(userList.begin(), make_pair(CString("General"), 0));

                userListBox.ResetContent();
                for (int i = 0; i < (int)userList.size(); ++i) {
                    userListBox.AddString(userList[i].first);
                }
            } else if (strcmp(msg.action, "new-user") == 0) {
                int countGeneral = userList[0].second;
                userList.erase(userList.begin(), userList.begin() + 2);
                userList.push_back(make_pair(CString(msg.content), 0));
                sort(userList.begin(), userList.end());
                
                userList.insert(userList.begin(), make_pair(CString("---------------"), -1));
                userList.insert(userList.begin(), make_pair(CString("General"), countGeneral));

                fetchUserList();
            } else if (strcmp(msg.action, "user-logout") == 0) {
                CString username(msg.content);
                int pos = -1;
                for (int i = 0; i < (int)userList.size(); ++i) {
                    if (userList[i].first == username) {
                        pos = i;
                        break;
                    }
                }
                userList.erase(userList.begin() + pos);
                fetchUserList();
            }
            break;
        }
    }
    return 0;
}

void CClientDlg::OnLbnSelchangeList2() {
    CString receiver;
    userListBox.GetText(userListBox.GetCurSel(), receiver);
    if (receiver == CString("---------------")) {
        return;
    }
    
    char *str = convertToChar(receiver);
    string tmp(str, str + strlen(str));
    string username = "";
    for (int i = 0; i < (int)tmp.size(); ++i) {
        if (tmp[i] == ' ') {
            break;
        }
        username += tmp[i];
    }

    CString cusername(username.c_str());
    for (int i = 0; i < (int)userList.size(); ++i) {
        if (userList[i].first == cusername) {
            userList[i].second = 0;
            break;
        }
    }
    fetchUserList();
    
    MessageList list = messageList[cusername];
    fetchMessageList(list);
    currentRoom = cusername;
}


void CClientDlg::OnCancel()
{
    // TODO: Add your specialized code here and/or call the base class
    closesocket(client);
    CDialogEx::OnCancel();
}


void CClientDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    closesocket(client);
    SignInDlg dlg;
    EndDialog(0);
    dlg.DoModal();
}
