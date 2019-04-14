#include "usersys.h"
#include "dbcontroller.h"
#include <cassert>
using namespace std;

void signin(sqlite3 *db, Auth auth) {
    string query = "select * \
                   from user \
                   where username='" + string(auth.username) + "'";
    Table result = handleSelect(db, query.c_str());
    if (result.size() != 2) {
        throw "No such user";
    }
    if (result[1][1] != string(auth.password)) {
        throw "Wrong password";
    }
    if (result[1][2] != "0") {
        throw "User already logged in";
    }
    string statement = "update user\
                       set loggedIn = 1\
                       where username = '" + string(auth.username) + "'";
    handleUpdate(db, statement.c_str());
}

void signup(sqlite3 *db, Auth auth) {
    if (strlen(auth.password) < 6) {
        throw "Password must contain at least 6 characters";
    }
    string query = "select * \
                   from user \
                   where username='" + string(auth.username) + "'";
    Table result = handleSelect(db, query.c_str());
    if (result.size() != 1) {
        throw "Username already used";
    }
    string statement = "insert into user \
                       values ('" + string(auth.username) + "', '" + string(auth.password) + "', 1)";
    handleUpdate(db, statement.c_str());
}

void logout(sqlite3 *db, string username) {
    string statement = "update user\
                       set loggedIn = 0\
                       where username = '" + username + "'";
    handleUpdate(db, statement.c_str());
}