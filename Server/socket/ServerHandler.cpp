#include "ServerHandler.h"
#include <cstdio>
#include "../utils/usersys.h"
#include "../utils/chat.h"

Client::Client() : socket(0) {};

const int PART_SIZE = 946;
struct FilePart {
	int size;
	char filename[50];
	char content[PART_SIZE];
};
struct PrivateMessage {
    char receiver[20];
    char message[980];
};

void ServerHandler::sendToAll(Message msg) {
    for (int i = 0; i < (int)clientList.size(); ++i) {
        Client client = clientList[i];
        if (client.socket) {
            sendTo(client.socket, msg);
        }
    }
}

string ServerHandler::findUsername(SOCKET sender) {
    char result[20] = { 0 };
    for (int i = 0; i < (int)clientList.size(); ++i) {
        if (clientList[i].socket == sender) {
            strcpy(result, clientList[i].username);
            break;
        }
    }
    return string(result);
}

SOCKET ServerHandler::findSocket(string username) {
    for (int i = 0; i < (int)clientList.size(); ++i) {
        if (string(clientList[i].username) == username) {
            return clientList[i].socket;
        }
    }
    return 0;
}




bool ServerHandler::ConnectToDatabase() {
    int err = sqlite3_open("server.db", &db);
    if (err) {
        return false;
    }
    string statement = "select username from user";
    Table result = handleSelect(db, statement.c_str());
    clientList.clear();
    for (int i = 1; i < (int)result.size(); ++i) {
        Client client;
        strcpy(client.username, result[i][0].c_str());
        clientList.push_back(client);
    }
    return true;
}

void ServerHandler::DisconnectFromDatabase() {    
    sqlite3_close(db);
}

void ServerHandler::DisconnectEveryone() {
    string statement = "update user set loggedIn = 0";
    handleUpdate(db, statement.c_str());
    sendToAll(Message("force-logout", ""));
}

bool ServerHandler::StartListening(HWND m_hWnd) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        return false;
    }
    server = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(server, (SOCKADDR *)&serverAddress, sizeof(serverAddress));
    listen(server, 5);

    int err = WSAAsyncSelect(server, m_hWnd, WM_SOCKET, FD_READ | FD_ACCEPT | FD_CLOSE);
    if (err) {
        return false;
    }
    return true;
}

string ServerHandler::HandleAuthentication(SOCKET sender, Message msg) {
    Auth auth;
    memcpy(&auth, (char*) &msg.content, sizeof auth);
    try {
        if (strcmp(msg.action, "login") == 0) {
            signin(db, auth);
        } else {
            signup(db, auth);
        }
    } catch (const char *err) {
        sendTo(sender, Message("login-response", err));
        throw 1;
    }

    char str[1000];
    sprintf(str, "%s %s.", auth.username, strcmp(msg.action, "login") == 0 ? "logged in" : "registered");

    if (strcmp(msg.action, "register") == 0) {       
        Client client;
        strcpy(client.username, auth.username);
        clientList.push_back(client);
        sendToAll(Message("new-user", auth.username));
    }
    sendTo(sender, Message("login-response", "OK"));
    return string(str);
}

void ServerHandler::UpdateSocket(SOCKET sender, const char *username) {
    for (int i = 0; i < (int)clientList.size(); ++i) {
        if (strcmp(clientList[i].username, username) == 0) {
            clientList[i].socket = sender;
            break;
        }
    }
    
    char str[1000];
    sprintf(str, "%s joined the chat.", username);
    sendToAll(Message("message-all", str));
    
    for (int i = 0; i < (int)clientList.size(); ++i) {
        sendTo(sender, Message("new-user", clientList[i].username));
    }
    FetchHistoryAll(sender);
	FetchFileAll(sender);
}

void ServerHandler::UpdateLatest(SOCKET sender, const char *username) {
    string user = findUsername(sender);
    string user1 = user;
    string user2 = string(username);
    if (user1 > user2) {
        swap(user1, user2);
    }
    char str[1000];
    sprintf(str, "select max(id) from history where (username1 = '%s' and username2 = '%s') or (username1 = '%s' and username2 = '%s')", user1.c_str(), user2.c_str(), user2.c_str(), user1.c_str());
    Table table = handleSelect(db, str);
    int id = table.size() == 2 ? stringToInt(table[1][0]) : 0;
    sprintf(str, "select latest from latest where user1 = '%s' and user2 = '%s' and user = '%s'", user1.c_str(), user2.c_str(), user.c_str());
    table = handleSelect(db, str);
    if (table.size() == 1) {
        sprintf(str, "insert into latest values ('%s', '%s', '%s', %d)", user1.c_str(), user2.c_str(), user.c_str(), id);
    } else {
        sprintf(str, "update latest set latest = %d where user1 = '%s' and user2 = '%s' and user = '%s'", id, user1.c_str(), user2.c_str(), user.c_str());
    }
    handleUpdate(db, str);
}

void ServerHandler::MessageToGeneral(SOCKET sender, Message msg) {
    char str[1000];
    sprintf(str, "%s: %s", findUsername(sender).c_str(), msg.content);
    generalChat(db, str);
    sendToAll(Message("message-all-new", str));
}

void ServerHandler::MessageToOne(SOCKET sender, Message msg) {
    PrivateMessage pvt1, pvt2;
    memcpy(&pvt1, msg.content, sizeof pvt1);

    // Find receiver's socket
    SOCKET receiver;
    for (int i = 0; i < (int)clientList.size(); ++i) {
        if (strcmp(clientList[i].username, pvt1.receiver) == 0) {
            receiver = clientList[i].socket;
            break;
        }
    }

    // Convert msg to "sender: msg"
    // and prepare message to send to both sender and receiver
    char str[980];
    string senderUsername = findUsername(sender);
    sprintf(str, "%s: %s", senderUsername.c_str(), pvt1.message);
    strcpy(pvt2.receiver, senderUsername.c_str());
    strcpy(pvt1.message, str);
    strcpy(pvt2.message, str);

    privateChat(db, pvt1.message, pvt1.receiver, pvt2.receiver);
    sendTo(sender, Message("message-one-new", (char*) &pvt1, sizeof pvt1));
    if (sender != receiver) {
        sendTo(receiver, Message("message-one-new", (char*) &pvt2, sizeof pvt2));
    }
}

string ServerHandler::Logout(SOCKET sender) {
    char username[20];
    for (int i = 0; i < (int)clientList.size(); ++i) {
        if (clientList[i].socket == sender) {
            logout(db, string(clientList[i].username));
            strcpy(username, clientList[i].username);
            break;
        }
    }

    char str[1000];
    sprintf(str, "%s left the chat.", username);

    sendToAll(Message("message-all", str));
    sendToAll(Message("user-logout", username));

    return string(str);
}

void ServerHandler::FetchHistoryAll(SOCKET receiver) {
    Table messList = fetchHistory(db, NULL, NULL, findUsername(receiver).c_str());
    for (int i = 1; i < (int)messList.size(); ++i) {
        sendTo(receiver, Message("message-all", messList[i][0].c_str()));
        Sleep(25);
    }
    messList = fetchHistory(db, NULL, NULL, findUsername(receiver).c_str(), true);
    for (int i = 1; i < (int)messList.size(); ++i) {
        sendTo(receiver, Message("message-all-new", messList[i][0].c_str()));
        Sleep(25);
    }
}

void ServerHandler::FetchFileAll(SOCKET receiver){
	Table fileList = fetchFile(db);
	for (int i = 1; i < (int)fileList.size(); ++i){
		sendTo(receiver, Message("new-file", fileList[i][0].c_str()));
	}
}

void ServerHandler::FetchHistoryOne(SOCKET sender, const char *receiverUsername) {
    string user1 = findUsername(sender);
    string user2 = string(receiverUsername);
    SOCKET receiver = findSocket(user2);

    Table messList = fetchHistory(db, user1.c_str(), user2.c_str(), user1.c_str());
    PrivateMessage pvt;
    strcpy(pvt.receiver, user2.c_str());
    for (int i = 1; i < (int)messList.size(); ++i) {
        strcpy(pvt.message, messList[i][0].c_str());
        sendTo(sender, Message("message-one", (char*) &pvt, sizeof pvt));
        Sleep(25);
    }
    messList = fetchHistory(db, user1.c_str(), user2.c_str(), user1.c_str(), true);
    for (int i = 1; i < (int)messList.size(); ++i) {
        strcpy(pvt.message, messList[i][0].c_str());
        sendTo(sender, Message("message-one-new", (char*) &pvt, sizeof pvt));
        Sleep(25);
    }
}

void ServerHandler::CreateForWriting(SOCKET socket, const char *rawname) {
    string filename(rawname);
    FILE *f = _wfopen(unicode(rawname), unicode("wb"));
    fileWriter[filename] = f;
    sendTo(socket, Message("upload-file-response", rawname));
}

void ServerHandler::SaveFile(SOCKET socket, const char *raw) {
	FilePart part;
	memcpy((char*)&part, raw, sizeof FilePart);
    
    FILE *f = fileWriter[string(part.filename)];
    fwrite(part.content, 1, part.size, f);

    if (part.size < PART_SIZE) {
        fclose(f);
        fileWriter.erase(string(part.filename));
        sendToAll(Message("new-file", part.filename));
        
        char statement[100];
        sprintf(statement, "insert into file values ('%s')", part.filename);
        handleUpdate(db, statement);
    } else {
        sendTo(socket, Message("upload-file-response", part.filename));
    }
}

void ServerHandler::CreateForReading(SOCKET socket, const char *rawname) {
    string filename(rawname);
    FILE *f = _wfopen(unicode(rawname), unicode("rb"));
    fileReader[make_pair(filename, socket)] = f;

    FilePart part;
    strcpy(part.filename, rawname);
    part.size = -1;
    sendTo(socket, Message("request-file-response", (char*)&part, sizeof part));
}

void ServerHandler::SendFile(SOCKET socket, const char *rawname) {
    string filename(rawname);
    FILE *f = fileReader[make_pair(filename, socket)];

    FilePart part;
    strcpy(part.filename, rawname);
    part.size = fread(part.content, 1, PART_SIZE, f);

    if (part.size < PART_SIZE) {
        fclose(f);
        fileReader.erase(make_pair(filename, socket));
    }
    sendTo(socket, Message("request-file-response", (char*)&part, sizeof part));
}

void ServerHandler::Initialize(CListBox *logs) {
    this->logs = logs;
}