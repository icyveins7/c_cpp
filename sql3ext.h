#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>
#include <regex>

class sq3db{
	
public:
	sq3db(const char *in_filename, int in_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, const char *zVfs = nullptr);
	~sq3db();
	
	void createTable(std::string &tablename, std::vector<std::string> &columnnames, std::vector<std::string> &columntypes, bool ifNotExists=true);
	std::vector<std::string> getTableNames(std::string pattern="");
	std::vector<std::string> getColumnNames(std::string &tablename);
	int getRowCount(std::string &tablename);
	
	// for simple statements
	void exec(std::string &stmtstr);
	
	// as a reminder, any database which inherits from this should implement a virtual select function..
	void selectColumns() {std::cout << "Unimplemented generic select function." << std::endl;}
	
	void prepStatement(sqlite3_stmt **stmt, std::string &stmtstr);
	void beginTransaction();
	void endTransaction();

private:
	sqlite3 *db;
	int flags;
	char filename[256];
	int err;
	char errMsg[256];
	
};