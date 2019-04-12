#pragma once
#include <afxsock.h>
#include "afxwin.h"
#include "socket/helper.h"
#include "socket/ClientHandler.h"
#include <vector>
#include <map>
#include <utility>
using namespace std;

class CClientDlg: public CDialogEx {
public:
	CClientDlg(char username[20], CWnd* pParent = NULL);
	enum { IDD = IDD_CLIENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
    ClientHandler handler;

public:
    CListBox logs;
    CListBox userListBox;

    LRESULT handleEvents(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedOk();
    afx_msg void OnLbnSelchangeList2();
    afx_msg void OnBnClickedButton1();
    virtual void OnCancel();

	afx_msg void OnBnClickedButton2();
	CListBox fileList;
	afx_msg void OnLbnSelchangeList();
};
