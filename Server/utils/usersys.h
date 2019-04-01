#pragma once
#include "sqlite3.h"
#include "../socket/helper.h"
#include <string>
using std::string;

void signin(sqlite3 *db, Auth auth);
void signup(sqlite3 *db, Auth auth);
void logout(sqlite3 *db, string username);