#pragma once
#pragma pack(1)

#include <afxsock.h>
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <cstdio>
using namespace std;

typedef vector<CString> MessageList;
struct PrivateMessage {
    char receiver[20];
    char message[980];
};

class ClientHandler {
private:
    char username[20];
    SOCKET client;
    SOCKADDR_IN serverAddress;

    map<CString, MessageList> messageList;
	map<CString, int> fileLength;

    map<string, FILE*> fileWriter;
    map<string, FILE*> fileReader;

    vector<pair<CString, int>> userList;
    CString currentRoom;
    CListBox *messBox;
    CListBox *userBox;

    void fetchMessList();
    void fetchUserList();

public:
    void Initialize(const char*, CListBox*, CListBox*);
    bool Connect(HWND);
    void Disconnect();

    void InsertUser(CString);
    void Send(CString);
    void ReceiveAll(CString, bool unread = false);
    void ReceiveOne(PrivateMessage, bool unread = false);
    void ChangeRoom(CString);

	void SendFile(const char*);
    void SendFilePart(const char*);

    void RequestFile(const char*);
    void ReceiveFilePart(const char*);
};

