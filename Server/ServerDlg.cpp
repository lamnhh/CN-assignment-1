#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include "helper.h"
#include <cstdint>

#pragma pack(1)

struct UserList {
    int count;
    char list[30][20];
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CServerDlg dialog
CServerDlg::CServerDlg(CWnd *pParent): CDialogEx(CServerDlg::IDD, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, logs);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_SOCKET, handleEvents)
    ON_BN_CLICKED(IDOK, &CServerDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CServerDlg message handlers
BOOL CServerDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint() {
	if (IsIconic()) {
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
	} else {
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}


/*******************************************************************
********************************************************************
 * MY OWN CODE
********************************************************************
********************************************************************/

void CServerDlg::OnBnClickedOk() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        return;
    }
    UpdateData();
    server = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(server, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
    listen(server, 5);

    int err = WSAAsyncSelect(server, m_hWnd, WM_SOCKET, FD_READ | FD_ACCEPT | FD_CLOSE);
    if (err) {
        return;
    }

    logs.AddString(CString("Connection created. Listening."));
    clientList.clear();
}

void CServerDlg::fetchAllUsername(char *ans) {
    UserList list;
    list.count = clientList.size();
    for (int i = 0; i < list.count; ++i) {
        strcpy(list.list[i], clientList[i].username);
    }
    memcpy(ans, (char*) &list, sizeof(UserList));
}

void CServerDlg::sendToAll(Message msg) {
    for (int i = 0; i < (int)clientList.size(); ++i) {
        Client client = clientList[i];
        sendTo(client.socket, msg);
    }
}

LRESULT CServerDlg::handleEvents(WPARAM wParam, LPARAM lParam) {
    if (WSAGETSELECTERROR(lParam)) {
        closesocket(wParam);
        return 0;
    }
    switch (WSAGETSELECTEVENT(lParam)) {
        case FD_ACCEPT: {
            accept(wParam, NULL, NULL);
            break;
        }
        case FD_READ: {
            Message msg;
            receive(wParam, msg);
            if (strcmp(msg.action, "login") == 0 || strcmp(msg.action, "register") == 0) {
                Auth auth;
                memcpy(&auth, (char*) &msg.content, sizeof msg.content);
                char str[1000];
                sprintf(str, "User %s logged in.", auth.username);
                logs.AddString(unicode(str));

                Message res;
                strcpy(res.action, "login-response");
                strcpy(res.content, "OK");
                sendTo(wParam, res);
                return 0;
            }
            if (strcmp(msg.action, "re-login") == 0) {
                Client client;
                client.socket = wParam;
                strcpy(client.username, msg.content);
                
                Message announcement;
                strcpy(announcement.action, "new-user");
                strcpy(announcement.content, client.username);
                sendToAll(announcement);

                clientList.push_back(client);

                char str[1000];
                sprintf(str, "User %s logged in.", client.username);

                strcpy(announcement.action, "message-all");
                strcpy(announcement.content, str);
                sendToAll(announcement);

                fetchAllUsername(str);
                strcpy(announcement.action, "user-list");
                memcpy(announcement.content, str, sizeof str);
                sendTo(wParam, announcement);
                return 0;
            }
            char sender[20] = { 0 };
            for (int i = 0; i < (int)clientList.size(); ++i) {
                if (clientList[i].socket == wParam) {
                    strcpy(sender, clientList[i].username);
                    break;
                }
            }
            if (strcmp("message-all", msg.action) == 0) {
                char str[1000];
                sprintf(str, "%s: %s", sender, msg.content);
                logs.AddString(unicode(str));
                strcpy(msg.action, "message-all");
                strcpy(msg.content, str);
                sendToAll(msg);
            }
            if (strcmp("message-one", msg.action) == 0) {
                struct PrivateMessage {
                    char receiver[20];
                    char message[980];
                } pvt1;
                memcpy(&pvt1, msg.content, sizeof pvt1);

                SOCKET sender = wParam;
                SOCKET receiver;                
                for (int i = 0; i < (int)clientList.size(); ++i) {
                    if (strcmp(clientList[i].username, pvt1.receiver) == 0) {
                        receiver = clientList[i].socket;
                        break;
                    }
                }

                PrivateMessage pvt2;
                char str[980];
                for (int i = 0; i < (int)clientList.size(); ++i) {
                    if (clientList[i].socket == sender) {
                        strcpy(pvt2.receiver, clientList[i].username);
                        sprintf(str, "%s: %s", clientList[i].username, pvt1.message);
                        strcpy(pvt1.message, str);
                        break;
                    }
                }
                strcpy(pvt2.message, pvt1.message);

                Message msg;
                strcpy(msg.action, "message-one");
                memcpy(msg.content, &pvt1, sizeof pvt1);
                sendTo(sender, msg);
                
                strcpy(msg.action, "message-one");
                memcpy(&msg.content, &pvt2, sizeof pvt2);
                sendTo(receiver, msg);
            }
            break;
        }
    }
    return 0;
}