#include <iostream>
#include "ipp.h"
#include <vector>
#include <string>

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
				reMalloc = false;
			};
			
			size_t size()
			{
				return numel;
			}
			
			size_t capacity()
			{
				return cap;
			}
			
			void clear()
			{
				numel = 0;
			}
			
			virtual void resize(size_t count)
			{
				std::cout << "Parent class resize." << std::endl;
				if (count > cap)
				{
					copylen = numel; // old, shorter length
					numel = count; // the new length
					cap = count; // capacity = new length
					reMalloc = true;
				}
				else{
					numel = count;
				}
			}
			
			~vector()
			{
				std::cout<<"Destructing base ipp vector."<<std::endl;
			}
		protected:
			size_t numel;
			size_t cap;
			size_t copylen;
			bool reMalloc;
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
			
			Ipp64fc& at(size_t pos)
			{
				if (pos < numel){
					return data[pos];
				}
				else{
					throw std::out_of_range(std::string("Size is ") + std::to_string(numel));
				}
			}
			
			void push_back(Ipp64fc value) // for now lets not deal with lvalue/rvalue refs
			{
				// check size
				if (numel == cap){
					resize(cap * 2);
				}
				
				data[numel] = value;
				numel++;
			}
			
			void resize(size_t count) override
			{
				vector::resize(count);
				std::cout << "Child class resize" << std::endl;
				if (reMalloc){
					Ipp64fc *newdata = ippsMalloc_64fc_L(cap);
					ippsCopy_64fc(data, newdata, copylen);
					ippsFree(data);
					data = newdata;
					
					reMalloc = false;
				}
				
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
	// resize to the capacity and check
	oldvector.resize(oldvector.capacity());
	std::cout << "std vector capacity = " << oldvector.capacity() << " and size = " << oldvector.size() << std::endl;
	std::cout << "std vector last element = " << oldvector.back() << std::endl;
	oldvector.push_back(10);
	std::cout << "std vector capacity = " << oldvector.capacity() << " and size = " << oldvector.size() << std::endl;
	std::cout << "std vector last element = " << oldvector.back() << std::endl;
	oldvector.resize(2);
	std::cout << "std vector capacity = " << oldvector.capacity() << " and size = " << oldvector.size() << std::endl;
	
	
	// create some ipp vectors?
	ippe::vector_64fc data;
	std::cout<<"ipp vector capacity = " << data.capacity() << " and size = " << data.size() << std::endl;
	
	// try a resize
	data.resize(256);
	data.resize(128);
	std::cout<<"ipp vector capacity = " << data.capacity() << " and size = " << data.size() << std::endl;
	
	// pushback some data
	Ipp64fc val = {1.0, 2.0};
	std::cout << "pushed back value is " << val.re << ", " << val.im << std::endl;

	return 0;
}