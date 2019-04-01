#pragma once
#pragma pack(1)

#include <afxsock.h>
#include <map>
#include <vector>
#include <utility>
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
    CString currentRoom;
    vector<pair<CString, int>> userList;

    CListBox *messBox;
    CListBox *userBox;

    void fetchMessList();
    void fetchUserList();

public:
    void Initialize(const char*, CListBox*, CListBox*);
    bool Connect(HWND);
    void Disconnect();

    void InsertUser(CString);
    void RemoveUser(CString);

    void Send(CString);
    void ReceiveAll(CString);
    void ReceiveOne(PrivateMessage);
    void ChangeRoom(CString);

};

