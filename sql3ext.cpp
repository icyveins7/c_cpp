#include "sql3ext.h"

sq3db::sq3db(const char *in_filename, int in_flags, const char *zVfs){
	
	std::cout << "Base sq3db ctor." << std::endl;
	
	db = nullptr;
	snprintf(filename, 256, "%s", in_filename);
	flags = in_flags;
	
	// actually open the database
	err = sqlite3_open_v2(filename, &db, flags, zVfs);
	
	if (err != SQLITE_OK)
	{
		printf("Error starting database: %s \n", sqlite3_errmsg(db));
		sqlite3_close(db);
		throw err;
	}
}

sq3db::~sq3db(){
	printf("Cleaning up database %s\n", filename);
	sqlite3_close(db);
}