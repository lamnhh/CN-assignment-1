#pragma once

#include "sqlite3.h"
#include <vector>
#include <string>
using namespace std;

typedef vector<string> Row;
typedef vector<Row> Table;

void handleUpdate(sqlite3 *db, const char *statement);
Table handleSelect(sqlite3 *db, const char *query);