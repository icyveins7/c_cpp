#include "sql3ext.h"

class usrpRXdb : public sq3db
{
public:
	usrpRXdb(const char *in_filename, int in_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, const char *zVfs = nullptr);
	~usrpRXdb();

};