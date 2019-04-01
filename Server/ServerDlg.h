#pragma once

#include "afxwin.h"
#include <afxsock.h>
#include <vector>
#include <string>
#include "socket/ServerHandler.h"
using namespace std;

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

private:
    ServerHandler handler;

public:
    afx_msg void OnBnClickedOk();
    virtual void OnCancel();
    CListBox logs;
};
