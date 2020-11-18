#include "usrpRXdb.h"

usrpRXdb::usrpRXdb(const char *in_filename, int in_flags, const char *zVfs)
	: sq3db(in_filename, in_flags, zVfs)
{
	std::cout << "usrpRXdb ctor." << std::endl;
}

usrpRXdb::~usrpRXdb()
{
	std::cout << "usrpRXdb dtor." << std::endl;
}