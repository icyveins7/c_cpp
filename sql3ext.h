#include <sqlite3.h>
#include <iostream>

class sq3db{
	
public:
	sq3db(const char *in_filename, int in_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, const char *zVfs = nullptr);
	~sq3db();
	
private:
	sqlite3 *db;
	int flags;
	char filename[256];
	int err;
	
};