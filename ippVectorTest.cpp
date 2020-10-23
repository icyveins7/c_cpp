#include <iostream>
#include "ipp.h"
#include <vector>

namespace ippe
{
	const size_t INITIAL_CAP = 128;
	
	class vector
	{
		public:
			vector(size_t count = 0)
			{
				std::cout<<"Constructing base ipp vector."<<std::endl;
				numel = count;
				cap = INITIAL_CAP;
				if (numel > cap){
					cap = numel;
				}
			};
			
			size_t size()
			{
				return numel;
			}
			
			size_t capacity()
			{
				return cap;
			}
			
			~vector()
			{
				std::cout<<"Destructing base ipp vector."<<std::endl;
			}
		protected:
			size_t numel;
			size_t cap;
	};
	
	class vector_64fc : public vector
	{
		public:
			vector_64fc()
			{
				std::cout<<"Constructing 64fc ipp vector."<<std::endl;
				data = ippsMalloc_64fc_L(cap);
			}
			
			~vector_64fc()
			{
				std::cout<<"Destructing 64fc ipp vector."<<std::endl;
				ippsFree(data);
			}
		private:
			Ipp64fc *data;
	};
	
}

int main()
{
	std::cout << "This is a IPP vector test." << std::endl;
	
	// instantiate a standard vector and see characteristics?
	std::vector<int> oldvector;
	std::cout << "std vector capacity = " << oldvector.capacity() << " and size = " << oldvector.size() << std::endl;
	for (int i = 0; i < 1024; i++){
		oldvector.push_back(1);
		if (oldvector.capacity() != oldvector.size())
		{
			std::cout << "std vector capacity = " << oldvector.capacity() << " and size = " << oldvector.size() << std::endl;
			break;
		}
	}
	
	
	
	// create some ipp vectors?
	ippe::vector_64fc data;

	return 0;
}