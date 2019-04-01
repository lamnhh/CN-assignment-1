#include "chat.h"
#include "dbcontroller.h"
#include "../socket/helper.h"

void generalChat(sqlite3 *db, const char *message) {
    string statement = "insert into history(message) values ('" + string(message) + "')";
    handleUpdate(db, statement.c_str());
}

void privateChat(sqlite3 *db, const char *message, const char *username1, const char *username2) {
    string statement = "insert into history(username1, username2, message)\
                       values ('" + string(username1) + "', '" + string(username2) + "', '" + string(message) + "')";
    handleUpdate(db, statement.c_str());
}

Table fetchHistory(sqlite3 *db, const char *user1, const char *user2, const char *user, bool unread) {
    char str[1000];
    if (user1 == NULL) {
        sprintf(str, "select latest from latest where (user1 is null and user2 is null) and user='%s'", user);
        Table table = handleSelect(db, str);
        int id = table.size() >= 2 ? stringToInt(table[1][0]) : 0;
        sprintf(str, "select message, id from history where username1 is null and username2 is null and id %s %d", unread ? ">" : "<=", id);
    } else {
        const char *f = (strcmp(user1, user) == 0) ? user2 : user1;
        sprintf(str, "select latest from latest where (user1 = '%s' or user2 = '%s') and user='%s'", f, f, user);
        Table table = handleSelect(db, str);
        int id = table.size() >= 2 ? stringToInt(table[1][0]) : 0;
        sprintf(str, "select message, id from history where ((username1 = '%s' and username2 = '%s') or (username1 = '%s' and username2 = '%s')) and id %s %d",
                user1, user2, user2, user1, unread ? ">" : "<=", id);
    }
    return handleSelect(db, str);
}