#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include "ClientHandler.h"
#include "helper.h"
using namespace std;

const int PART_SIZE = 946;
struct FilePart {
	int size;
	char filename[50];
	char content[PART_SIZE];
};

string getFileName(string path) {
	string ans = "";
	for (int i = (int)path.size() - 1; i >= 0; --i) {
		if (path[i] == '/' || path[i] == '\\') {
			break;
		}
		ans = path[i] + ans;
	}
	return ans;
}

void ClientHandler::fetchMessList() {
    const int MAX_LENGTH = 72;
    int count = 0;
    MessageList msgList = messageList[currentRoom];
    messBox->ResetContent();
    for (int i = 0; i < (int)msgList.size(); ++i) {
        string str(convertToChar(msgList[i]));
        for (int i = 0; i < (int)str.size(); i += MAX_LENGTH) {
            string s = str.substr(i, MAX_LENGTH);
            messBox->AddString(unicode(s.c_str()));
            count += 1;
        }
    }
    messBox->SetCurSel(count - 1);
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
    bool exists = false;
    for (int i = 0; i < (int)userList.size(); ++i) {
        if (userList[i].first == username) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        int countGeneral = userList.size() > 0 ? userList[0].second : 0;
        if (userList.size() >= 2) {
            userList.erase(userList.begin(), userList.begin() + 2);
        }
        userList.push_back(make_pair(username, 0));
        sort(userList.begin(), userList.end());

        userList.insert(userList.begin(), make_pair(CString("---------------"), -1));
        userList.insert(userList.begin(), make_pair(CString("General"), countGeneral));
    }
    fetchUserList();
    sendTo(client, Message("request-one", convertToChar(username)));
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

void ClientHandler::ReceiveAll(CString message, bool unread) {
    messageList[CString("General")].push_back(message);
    if (currentRoom != CString("General")) {
        userList[0].second += unread ? 1 : 0;
        fetchUserList();
    } else {
        fetchMessList();
    }
}

void ClientHandler::ReceiveOne(PrivateMessage pvt, bool unread) {
    CString sender(pvt.receiver);
    messageList[sender].push_back(unicode(pvt.message));
    if (currentRoom != sender) {
        for (int i = 0; i < (int)userList.size(); ++i) {
            if (userList[i].first == sender) {
                userList[i].second += unread ? 1 : 0;
                break;
            }
        }
        fetchUserList();
    } else {
        fetchMessList();
        sendTo(client, Message("update-latest", convertToChar(currentRoom)));
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

    if (room != CString("General")) {
        sendTo(client, Message("update-latest", convertToChar(room)));
    }
}

void ClientHandler::SendFile(const char *path) {
	FILE *f = _wfopen(unicode(path), unicode("rb"));
	string filename(getFileName(string(path)));
    fileReader[filename] = f;
    sendTo(client, Message("upload-file", filename.c_str()));
}

void ClientHandler::SendFilePart(const char *rawname) {
    string filename(rawname);
    FILE *f = fileReader[filename];
    FilePart part;
    strcpy(part.filename, rawname);
    part.size = fread(part.content, 1, PART_SIZE, f);
    sendTo(client, Message("upload-file-part", (char*)&part, sizeof part));

    if (part.size < PART_SIZE) {
        fclose(f);
        fileReader.erase(filename);
    }
}

void ClientHandler::RequestFile(const char *rawname) {
    sendTo(client, Message("request-file", rawname));
}

void ClientHandler::ReceiveFilePart(const char *raw) {
	FilePart part;
	memcpy((char*)&part, raw, sizeof FilePart);
    
    string filename(part.filename);
    if (part.size == -1) {
        FILE *f = _wfopen(unicode(part.filename), unicode("wb"));
        fileWriter[filename] = f;
    } else {
        FILE *f = fileWriter[filename];
        fwrite(part.content, 1, part.size, f);
        if (part.size < PART_SIZE) {
            fclose(f);
            fileWriter.erase(filename);
            return;
        }
    }
    sendTo(client, Message("request-file-part", part.filename));
}