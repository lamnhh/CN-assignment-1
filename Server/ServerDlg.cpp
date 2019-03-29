
// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

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

int Client::count = 0;

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
        MessageBox(LPCTSTR(L"Can't call"));
    }
    clientList.clear();
}

void CServerDlg::sendTo(SOCKET socket, Message msg) {
    send(socket, (char*) &msg, sizeof msg, 0);
}

void CServerDlg::receive(SOCKET socket, Message &msg) {
    recv(socket, (char*) &msg, sizeof msg, 0);
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
            Client client;
            client.socket = accept(wParam, NULL, NULL);
            client.id = ++Client::count;
            clientList.push_back(client);
            
            char str[1000];
            sprintf(str, "User %d just logged in", client.id);
            logs.AddString(CString(str));

            Message msg;
            strcpy(msg.content, str);
            sendToAll(msg);
            break;
        }
        case FD_READ: {
            Message msg;
            receive(wParam, msg);
            int senderID = 0;
            for (int i = 0; i < (int)clientList.size(); ++i) {
                if (clientList[i].socket == wParam) {
                    senderID = clientList[i].id;
                    break;
                }
            }
            if (strcmp("message", msg.action) == 0) {
                char str[1000];
                sprintf(str, "%d: %s", senderID, msg.content);
                logs.AddString(CString(str));
                strcpy(msg.content, str);
                sendToAll(msg);
            }
            break;
        }
    }
    return 0;
}