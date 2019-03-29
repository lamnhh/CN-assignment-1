#include "stdafx.h"
#include "helper.h"

void sendTo(SOCKET socket, Message &msg) {
    send(socket, (char*) &msg, sizeof msg, 0);
}

void receive(SOCKET socket, Message &msg) {
    recv(socket, (char*) &msg, sizeof msg, 0);
}

char* convertToChar(const CString &s) {
	CStringA utf8 = CW2A(s, CP_UTF8);
    return _strdup(utf8);
}

CString unicode(char *str) {
    return (LPCTSTR)CA2W(str, CP_UTF8);
}