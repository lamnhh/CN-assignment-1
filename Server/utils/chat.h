#pragma once
#include "sqlite3.h"
#include "dbcontroller.h"

void generalChat(sqlite3 *db, const char *message);
void privateChat(sqlite3 *db, const char *message, const char *username1, const char *username2);
Row fetchHistory(sqlite3 *db, const char *username1, const char *username2);