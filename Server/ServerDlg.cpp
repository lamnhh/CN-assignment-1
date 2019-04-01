#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include "socket/helper.h"
#include "utils/usersys.h"
#include "utils/chat.h"
#include <cstdint>

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
    if (!handler.ConnectToDatabase()) {    
        MessageBox(L"Cannot connect to database");
        EndDialog(0);
        return TRUE;
    }
    logs.AddString(L"Connected to database");

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

void CServerDlg::OnBnClickedOk() {
    if (handler.StartListening(m_hWnd)) {
        logs.AddString(CString("Connection created. Listening."));
    } else {
        logs.AddString(CString("Failed to create connection."));
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
                try {
                    string response = handler.HandleAuthentication(wParam, msg);
                    logs.AddString(unicode(response.c_str()));
                } catch (int) {}
            }
            if (strcmp(msg.action, "re-login") == 0) {
                handler.UpdateSocket(wParam, msg.content);
            }
            if (strcmp("message-all", msg.action) == 0) {
                handler.MessageToGeneral(wParam, msg);
            }
            if (strcmp("message-one", msg.action) == 0) {
                handler.MessageToOne(wParam, msg);
            }
            if (strcmp("request-one", msg.action) == 0) {
                handler.FetchHistoryOne(wParam, msg.content);
            }
            break;
        }
        case FD_CLOSE: {
            try {
                string response = handler.Logout(wParam);
                logs.AddString(unicode(response.c_str()));
            } catch (int) {}
            break;
        }
    }
    return 0;
}

void CServerDlg::OnCancel() {
    handler.DisconnectFromDatabase();
    CDialogEx::OnCancel();
}