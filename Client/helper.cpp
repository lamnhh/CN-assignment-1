#include "stdafx.h"
#include "helper.h"

void sendTo(SOCKET socket, Message &msg) {
    send(socket, (char*) &msg, sizeof msg, 0);
}

void receive(SOCKET socket, Message &msg) {
    recv(socket, (char*) &msg, sizeof msg, 0);
}

char* convertToChar(const CString &s) {
	int nSize = s.GetLength();
	char *pAnsiString = new char[nSize+1];
	memset(pAnsiString,0,nSize+1);
	wcstombs(pAnsiString, s, nSize+1);
	return pAnsiString;
}