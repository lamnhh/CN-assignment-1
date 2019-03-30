#pragma once
#pragma pack(1)

#include <afxsock.h>

#define PORT 25000
#define WM_SOCKET (WM_USER + 1)

struct Message {
    char action[50];
    char content[1000];
};

struct Auth {
    char username[20];
    char password[20];
};

void sendTo(SOCKET socket, Message &msg);
void receive(SOCKET socket, Message &msg);
char* convertToChar(const CString &s);
CString unicode(char *str);