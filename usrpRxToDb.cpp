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
	
	

	return 0;
}