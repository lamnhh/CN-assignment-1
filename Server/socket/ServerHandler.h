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
    Client();
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
    sqlite3 *db;
    vector<Client> clientList;
    vector<AuthInfo> authList;

    void sendToAll(Message);
    string findUsername(SOCKET);
    SOCKET findSocket(string);

public:
    bool ConnectToDatabase();
    void DisconnectFromDatabase();
    void DisconnectEveryone();
    bool StartListening(HWND);
    string Logout(SOCKET);

    string HandleAuthentication(SOCKET, Message);
    void UpdateSocket(SOCKET, const char*);
    void UpdateLatest(SOCKET, const char*);

    void MessageToGeneral(SOCKET, Message);
    void MessageToOne(SOCKET, Message);
	void FetchHistoryAll(SOCKET);
    void FetchHistoryOne(SOCKET, const char*);
	void FetchFileAll(SOCKET);

	void SaveFile(SOCKET, const char*);
	void SendFile(SOCKET, const char*);
};

