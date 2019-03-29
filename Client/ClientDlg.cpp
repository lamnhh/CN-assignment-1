
// ClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CClientDlg dialog

CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, logs);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_SOCKET, handleEvents)
    ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDCANCEL, &CClientDlg::OnBnClickedCancel)
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


char* convertToChar(const CString &s) {
	int nSize = s.GetLength();
	char *pAnsiString = new char[nSize+1];
	memset(pAnsiString,0,nSize+1);
	wcstombs(pAnsiString, s, nSize+1);
	return pAnsiString;
}

void CClientDlg::OnBnClickedOk() {
    CString message;
    GetDlgItemText(IDC_EDIT1, message);
    
    Message msg;
    strcpy(msg.action, "message");
    strcpy(msg.content, convertToChar(message));

    send(client, (char*) &msg, sizeof msg, 0);
}


void CClientDlg::OnBnClickedButton1() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        return;
    }

    UpdateData();
    client = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    int err = connect(client, (SOCKADDR*) &serverAddress, sizeof serverAddress);
    if (err == SOCKET_ERROR) {
        MessageBox(LPCTSTR(L"Cannot connect"));
        return;
    }
    MessageBox(LPCTSTR(L"Connected"));
    WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
}


void CClientDlg::OnBnClickedCancel() {
    closesocket(client);
    CDialogEx::OnCancel();
}

void CClientDlg::sendTo(SOCKET socket, Message msg) {
    send(socket, (char*) &msg, sizeof msg, 0);
}

void CClientDlg::receive(SOCKET socket, Message &msg) {
    recv(socket, (char*) &msg, sizeof msg, 0);
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

            logs.AddString(CString(msg.content));
            break;
        }
    }
    return 0;
}