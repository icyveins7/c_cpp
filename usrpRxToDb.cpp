#include <iostream>
// #include "sql3ext.h"
#include "usrpRXdb.h"


// #include <uhd/types/tune_request.hpp>
// #include <uhd/usrp/multi_usrp.hpp>
// #include <uhd/utils/thread.hpp>


int main()
{
	// attempt to initialize a database
	sq3db mydb("mydb.db");
	
	// attempt to initialize read-only non-existent
	try{
		sq3db myotherdb("notinit.db", SQLITE_OPEN_READONLY);
	}
	catch(int err){
		std::cout<<"Error code : "<< err << std::endl;
	}
	
	// attempt to initialize inherited db
	usrpRXdb usrpdb("usrpdb.db");
	
	// add a table
	std::vector<std::string> columnnames;
	columnnames.push_back("c1");
	columnnames.push_back("c2");
	std::vector<std::string> columntypes;
	columntypes.push_back("int");
	columntypes.push_back("real");
	usrpdb.createTable(std::string("t"), columnnames, columntypes);
	usrpdb.createTable(std::string("t1"), columnnames, columntypes);
	
	// attempt to add again forcefully even though it exists
	try{
		usrpdb.createTable(std::string("t"), columnnames, columntypes, false);
	}
	catch(int err){
		std::cout << "Error caught : " << err << std::endl;
	}
	
	// get all tables
	std::vector<std::string> alltables = usrpdb.getTableNames();
	for (int i = 0; i < alltables.size(); i++){
		printf("Table %d : %s\n", i, alltables.at(i).c_str());
	}
	
	// get all tables with pattern
	std::vector<std::string> matchedtables = usrpdb.getTableNames(std::string("%1%"));
	for (int i = 0; i < matchedtables.size(); i++){
		printf("Table %d : %s\n", i, matchedtables.at(i).c_str());
	}
	
	// get the column names
	std::vector<std::string> allcols = usrpdb.getColumnNames(std::string("t"));
	for (int i = 0; i < allcols.size(); i++){
		printf("Column name %d : %s\n", i, allcols.at(i).c_str());
	}
	

	return 0;
}