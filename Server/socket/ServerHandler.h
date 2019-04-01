#pragma once
#include <afxsock.h>
#include <vector>
#include <string>
#include "helper.h"
#include "../utils/sqlite3.h"
using namespace std;

struct Client {
    SOCKET socket;
    char username[20];
};

struct AuthInfo {
    CString username;
    CString password;
    bool loggedIn;
};

class ServerHandler {
private:
    SOCKET server;
    SOCKADDR_IN serverAddress;
    vector<Client> clientList;
    vector<AuthInfo> authList;

    void fetchAllUsername(char*);
    void sendToAll(Message);

    sqlite3 *db;
    string findUsername(SOCKET);

public:
    bool ConnectToDatabase();
    void DisconnectFromDatabase();
    bool StartListening(HWND);
    string HandleAuthentication(SOCKET, Message);
    void UpdateSocket(SOCKET, const char*);

    void MessageToGeneral(SOCKET, Message);
    void MessageToOne(SOCKET, Message);
    string Logout(SOCKET);
};

