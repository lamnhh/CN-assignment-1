
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CClientDlg dialog

CClientDlg::CClientDlg(char username[20], CWnd *pParent): CDialogEx(CClientDlg::IDD, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    handler.Initialize(username, &logs, &userListBox);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, logs);
	DDX_Control(pDX, IDC_LIST2, userListBox);
	DDX_Control(pDX, FILE_LIST, fileList);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_SOCKET, handleEvents)
    ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
    ON_LBN_SELCHANGE(IDC_LIST2, &CClientDlg::OnLbnSelchangeList2)
    ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnBnClickedButton2)
	ON_LBN_SELCHANGE(FILE_LIST, &CClientDlg::OnLbnSelchangeList)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    
	// TODO: Add extra initialization here
    handler.Connect(m_hWnd);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientDlg::OnPaint() {
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
HCURSOR CClientDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

void CClientDlg::OnBnClickedOk() {
    CString message;
    GetDlgItemText(IDC_EDIT1, message);
    SetDlgItemText(IDC_EDIT1, CString(""));  
    handler.Send(message);
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
                handler.ReceiveAll(unicode(msg.content));
            }
            if (strcmp(msg.action, "message-one") == 0) {
                PrivateMessage pvt;
                memcpy(&pvt, msg.content, sizeof msg.content);
                handler.ReceiveOne(pvt);
            }
            if (strcmp(msg.action, "message-all-new") == 0) {
                handler.ReceiveAll(unicode(msg.content), true);
            }
            if (strcmp(msg.action, "message-one-new") == 0) {
                PrivateMessage pvt;
                memcpy(&pvt, msg.content, sizeof msg.content);
                handler.ReceiveOne(pvt, true);
            }
			if (strcmp(msg.action, "new-file") == 0) {
				fileList.AddString(unicode(msg.content));
			}
			if (strcmp(msg.action, "file") == 0) {
				handler.SaveFile(msg.content);
			}
			if (strcmp(msg.action, "file-length") == 0) {
				struct FileLength {
					char filename[50];
					int length;
				} fl;
				memcpy(&fl, msg.content, sizeof FileLength);
				handler.ReceiveFileLength(fl.filename, fl.length);
			}
            if (strcmp(msg.action, "new-user") == 0) {
                handler.InsertUser(CString(msg.content));
            }
            if (strcmp(msg.action, "user-logout") == 0) {
                handler.RemoveUser(CString(msg.content));
            }
            if (strcmp(msg.action, "force-logout") == 0) {
                MessageBox(L"Connection lost.");
                closesocket(wParam);
                SignInDlg dlg;
                EndDialog(0);
                dlg.DoModal();
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
    handler.ChangeRoom(CString(username.c_str()));
}

void CClientDlg::OnCancel() {
    handler.Disconnect();
    CDialogEx::OnCancel();
}

void CClientDlg::OnBnClickedButton1() {
    handler.Disconnect();
    SignInDlg dlg;
    EndDialog(0);
    dlg.DoModal();
}


void CClientDlg::OnBnClickedButton2() {
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, nullptr);
	if (dlg.DoModal() == IDOK) {
		handler.SendFile(convertToChar(dlg.GetPathName()));
	}
}


void CClientDlg::OnLbnSelchangeList() {
	CString file;
	fileList.GetText(fileList.GetCurSel(), file);
	handler.RequestFile(convertToChar(file));
}
