#include "chat.h"
#include "dbcontroller.h"

void generalChat(sqlite3 *db, const char *message) {
    string statement = "insert into history(message) values ('" + string(message) + "')";
    handleUpdate(db, statement.c_str());
}

void privateChat(sqlite3 *db, const char *message, const char *username1, const char *username2) {
    string statement = "insert into history(username1, username2, message)\
                       values ('" + string(username1) + "', '" + string(username2) + "', '" + string(message) + "')";
    handleUpdate(db, statement.c_str());
}

Table fetchHistory(sqlite3 *db, const char *username1, const char *username2) {
    char str[1000];
    if (username1 == NULL) {
        sprintf(str, "\
                       select message, id\
                         from history\
                        where (username1 is null) and (username2 is null)\
                     order by id");
    } else {
        sprintf(str, "\
                       select message, id\
                         from history\
                        where (username1 = '%s' and username2 = '%s') or (username1 = '%s' and username2 = '%s')\
                     order by id", username1, username2, username2, username1);
    }
    return handleSelect(db, str);
}