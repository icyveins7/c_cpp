#include <iostream>
#include "ipp.h"

// return reference to data
Ipp32s& getRef(Ipp32s *data, int idx){
	return data[idx];
}

int main()
{
	Ipp32s *data = ippsMalloc_32s_L(32);
	// simple initialization
	for (int i = 0; i < 32; i++){ data[i] = i; }
	
	// get a reference?
	Ipp32s& r = getRef(data, 10);
	
	std::cout << "Reference = " << r << std::endl;
	
	// assign a variable to this value?
	Ipp32s n = r;
	
	std::cout << "New value = " << n << std::endl;
	
	// check address?
	std::cout << "Original address = " << &data[10] << std::endl;
	std::cout << "Address from reference = " << &r << std::endl;
	
	ippsFree(data);

	return 0;
}