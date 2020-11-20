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

void sq3db::createTable(std::string &tablename, std::vector<std::string> &columnnames, std::vector<std::string> &columntypes, bool ifNotExists)
{
	// construct statement
	std::string stmtstr;
	stmtstr = "create table ";
	if (ifNotExists){
		stmtstr = stmtstr + "if not exists ";
	}
	stmtstr = stmtstr + tablename + "(";
	for (int i = 0; i < columnnames.size(); i++){
		stmtstr = stmtstr + columnnames.at(i) + " " + columntypes.at(i);
		if (i < columnnames.size() - 1){
			stmtstr = stmtstr + ",";
		}
	}
	stmtstr = stmtstr + ");";
	std::cout << "Statement executed is : " << stmtstr << std::endl;
	
	try{exec(stmtstr);}
	catch (int err) {throw err;} // rethrow
}

std::vector<std::string> sq3db::getTableNames(std::string pattern)
{
	// create vector to return
	std::vector<std::string> r;
	
	// create statement string
	std::string stmtstr = "select name from sqlite_master where type='table' and name not like 'sqlite_%'";
	
	if (pattern != "")
	{
		stmtstr = stmtstr + " and name like '" + pattern + "'";
	}
	
	stmtstr = stmtstr + ";";
	std::cout << "Statement executed is : " << stmtstr << std::endl;
	
	// prepare statement
	sqlite3_stmt* stmt = 0;
	prepStatement(&stmt, stmtstr);
	
	// begin
	beginTransaction();
	
	// loop over results
	while (sqlite3_step(stmt) == SQLITE_ROW){
		// deep copy into strings
		r.push_back(std::string((const char*)sqlite3_column_text(stmt, 0)));
	}
	
	// end
	endTransaction();
	
	// free statement
	err = sqlite3_finalize(stmt);
	if (err != SQLITE_OK)
	{
		printf("Error %d finalizing statement : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
	
	return r;
}

/*
Note that this is really for convenience (e.g. retrieving to display in table).
For real storage of the data you would need to know the type already.
*/
std::vector<std::string> sq3db::getColumnNames(std::string &tablename)
{
	// create return vector
	std::vector<std::string> r;
	
	// create statement string
	std::string stmtstr = "select sql from sqlite_master where tbl_name = '" + tablename + "' AND type = 'table';";
	
	std::cout << "Statement executed is : " << stmtstr << std::endl;
	
	// prepare statement
	sqlite3_stmt *stmt = 0;
	prepStatement(&stmt, stmtstr);
	
	// begin
	beginTransaction();
	
	// loop over results (there's really only one)
	std::string original;
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// deep copy into strings
		original = std::string((const char*)sqlite3_column_text(stmt, 0));
	}
	
	// end
	endTransaction();
	
	// free statement
	err = sqlite3_finalize(stmt);
	if (err != SQLITE_OK)
	{
		printf("Error %d finalizing statement : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
	
	// parse the statement
	std::regex re("[()]+");
	std::sregex_token_iterator it{ original.begin(),  
                             original.end(), re, -1 }; 
    std::vector<std::string> tokenized{ it, {} }; 
  
    // Additional check to remove empty strings 
    tokenized.erase( 
        std::remove_if(tokenized.begin(),  
                            tokenized.end(), 
                       [](std::string const& s) { 
                           return s.size() == 0; 
                       }), 
        tokenized.end()); 
  
	// we use the last string and split on commas first
	std::regex commare(",");
	std::sregex_token_iterator fcolit{tokenized.back().begin(), tokenized.back().end(), commare, -1 };
	std::vector<std::string> columnfulltokens{fcolit, {} };
	
	// then now split on spaces, and take the first one of each
	std::regex spacere("\\s+");
	for (int i = 0; i < columnfulltokens.size(); i++){
		std::sregex_token_iterator spaceit{columnfulltokens.at(i).begin(), columnfulltokens.at(i).end(), spacere, -1};
		std::vector<std::string> columntokens{spaceit, {}};
		r.push_back(columntokens.front());
	}
	
	return r;
	
}

/*
Note that this still enumerates all the rows internally, so it's a good idea to keep the row count somewhere else.
*/
int sq3db::getRowCount(std::string &tablename)
{
	int r;
	
	// create statement string
	std::string stmtstr = "select count(*) from " + tablename;
	std::cout << "Statement executed is : " << stmtstr << std::endl;
	
	// prepare statement
	sqlite3_stmt *stmt = 0;
	prepStatement(&stmt, stmtstr);
	
	// begin
	beginTransaction();
	
	// loop over results (there's really only one)
	std::string original;
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		r = sqlite3_column_int(stmt, 0);
	}
	
	// end
	endTransaction();
	
	// free statement
	err = sqlite3_finalize(stmt);
	if (err != SQLITE_OK)
	{
		printf("Error %d finalizing statement : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
	
	return r;
}

void sq3db::exec(std::string &stmtstr)
{
	err = sqlite3_exec(db, stmtstr.c_str(), 0, 0, 0);
	if (err != SQLITE_OK)
	{
		printf("Error %d : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
}



void sq3db::prepStatement(sqlite3_stmt **stmt, std::string &stmtstr)
{
	err = sqlite3_prepare_v2(
		  db,            /* Database handle */
		  stmtstr.c_str(),       /* SQL statement, UTF-8 encoded */
		  stmtstr.size()+1,              /* Maximum length of zSql in bytes. */ // note we add 1 because of the null terminator
		  stmt,  /* OUT: Statement handle */
		  0     /* OUT: Pointer to unused portion of zSql */
		);
	if (err != SQLITE_OK)
	{
		printf("Error %d preparing statement : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
}

void sq3db::beginTransaction()
{
	err = sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
	if (err != SQLITE_OK)
	{
		printf("Error %d beginning transaction : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
}

void sq3db::endTransaction()
{
	err = sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
	if (err != SQLITE_OK)
	{
		printf("Error %d ending transaction : %s \n", err, sqlite3_errmsg(db));
		throw err;
	}
}
	

sq3db::~sq3db(){
	printf("Cleaning up database %s\n", filename);
	sqlite3_close(db);
}