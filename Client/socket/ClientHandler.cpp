#include <algorithm>
#include "ClientHandler.h"
#include "helper.h"
using namespace std;

void ClientHandler::fetchMessList() {
    const int MAX_LENGTH = 72;
    MessageList msgList = messageList[currentRoom];
    messBox->ResetContent();
    for (int i = 0; i < (int)msgList.size(); ++i) {
        string str(convertToChar(msgList[i]));
        for (int i = 0; i < (int)str.size(); i += MAX_LENGTH) {
            string s = str.substr(i, MAX_LENGTH);
            messBox->AddString(unicode(s.c_str()));
        }
    }
    messBox->SetCurSel((int)msgList.size() - 1);
}

void ClientHandler::fetchUserList() {
    int pos = -1;
    userBox->ResetContent();
    for (int i = 0; i < (int)userList.size(); ++i) {
        char str[1000];
        if (userList[i].first == currentRoom) {
            pos = i;
        }
        if (userList[i].second > 0) {
            sprintf(str, "%s (%d)", convertToChar(userList[i].first), userList[i].second);
        } else {
            sprintf(str, "%s", convertToChar(userList[i].first));
        }
        userBox->AddString(CString(str));
    }
    if (pos >= 0) {
        userBox->SetCurSel(pos);
    }
}






bool ClientHandler::Connect(HWND m_hWnd) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        return false;
    }

    client = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    int err = connect(client, (SOCKADDR*) &serverAddress, sizeof serverAddress);
    if (err == SOCKET_ERROR) {
        return false;
    }
    WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
    sendTo(client, Message("re-login", this->username));
    return true;
}

void ClientHandler::Disconnect() {
    closesocket(client);
}

void ClientHandler::Initialize(const char *username, CListBox *messBox, CListBox *userBox) {
    strcpy(this->username, username);
    this->currentRoom = CString("General");
    this->messBox = messBox;
    this->userBox = userBox;
}

void ClientHandler::InsertUser(CString username) {
    int countGeneral = userList.size() > 0 ? userList[0].second : 0;
    if (userList.size() >= 2) {
        userList.erase(userList.begin(), userList.begin() + 2);
    }
    userList.push_back(make_pair(username, 0));
    sort(userList.begin(), userList.end());

    userList.insert(userList.begin(), make_pair(CString("---------------"), -1));
    userList.insert(userList.begin(), make_pair(CString("General"), countGeneral));

    fetchUserList();
    sendTo(client, Message("request-one", convertToChar(username)));
}

void ClientHandler::RemoveUser(CString username) {
    int pos = -1;
    for (int i = 0; i < (int)userList.size(); ++i) {
        if (userList[i].first == username) {
            pos = i;
            break;
        }
    }
    if (pos != -1) {
        userList.erase(userList.begin() + pos);
    }
    messageList.erase(username);
    fetchUserList();
}

void ClientHandler::Send(CString message) {
    if (currentRoom == CString("General")) {
        sendTo(client, Message("message-all", convertToChar(message)));
    } else {
        static PrivateMessage pvt;
        strcpy(pvt.receiver, convertToChar(currentRoom));
        strcpy(pvt.message, convertToChar(message));        
        sendTo(client, Message("message-one", (char*) &pvt, sizeof pvt));
    }
}

void ClientHandler::ReceiveAll(CString message) {
    messageList[CString("General")].push_back(message);
    if (currentRoom != CString("General")) {
        userList[0].second += 1;
        fetchUserList();
    } else {
        fetchMessList();
    }
}

void ClientHandler::ReceiveOne(PrivateMessage pvt) {
    CString sender(pvt.receiver);
    messageList[sender].push_back(unicode(pvt.message));
    if (currentRoom != sender) {
        for (int i = 0; i < (int)userList.size(); ++i) {
            if (userList[i].first == sender) {
                userList[i].second += 1;
                break;
            }
        }
        fetchUserList();
    } else {
        fetchMessList();
    }
}

void ClientHandler::ChangeRoom(CString room) {
    // Reset unread count for 'room'
    for (int i = 0; i < (int)userList.size(); ++i) {
        if (userList[i].first == room) {
            userList[i].second = 0;
            break;
        }
    }    
    currentRoom = room;
    fetchMessList();
    fetchUserList();
    for (int i = 0; i < (int)userList.size(); ++i) {
        if (userList[i].first == room) {
            userBox->SetCurSel(i);
            break;
        }
    }
}