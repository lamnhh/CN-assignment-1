#include "dbcontroller.h"

void handleUpdate(sqlite3 *db, const char *statement) {
    char *errMsg = 0;
    int test = sqlite3_exec(db, statement, NULL, NULL, &errMsg);
    if (test == SQLITE_OK) {
        sqlite3_free(errMsg);
    } else {
        string err(errMsg);
        sqlite3_free(errMsg);
        throw err.c_str();
    }
}

Table handleSelect(sqlite3 *db, const char* query) {
	Table ans;
	sqlite3_stmt* stm;
	if (sqlite3_prepare(db, query, -1, &stm, 0) != SQLITE_OK) {
		return ans;
	}

	int colCount = sqlite3_column_count(stm);
    Row header;
	for (int i = 0; i < colCount; ++i) {
		header.push_back((char*) sqlite3_column_name(stm, i));
	}
	ans.push_back(header);
	while (1) {
		int res = sqlite3_step(stm);
		if (res == SQLITE_ROW) {
			Row row;
			for (int i = 0; i < colCount; ++i) {
				void* ptr = (void*) sqlite3_column_text(stm, i);
				if (ptr == NULL) {
					row.push_back("");
				} else {
					row.push_back((char*) ptr);
				}
			}
			ans.push_back(row);
		}
		if (res == SQLITE_DONE) {
			sqlite3_finalize(stm);
			break;
		}
	}
	return ans;

}