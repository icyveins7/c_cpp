#include <iostream>
#include "ipp.h"

template<typename T>
class vector_t
{
public:
	vector_t<T>(); // templated ctor
	
	~vector_t()
	{
		printf("Default dtor\n");
	}
};

// default ctor
template <typename T>
vector_t<T>::vector_t()
{
	std::cout<<"Default ctor\n"<<std::endl;
}

// specialized ctor
template <>
vector_t<Ipp64fc>::vector_t()
{
	std::cout<<"Ipp64fc ctor\n"<<std::endl;
}

int main()
{
	std::cout<<"Test default ctor."<<std::endl;
	vector_t<int> v0;
	
	std::cout<<"Test Ipp64fc ctor."<<std::endl;
	vector_t<Ipp64fc> v_64fc;

	return 0;
}