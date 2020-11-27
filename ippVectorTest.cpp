#include <iostream>
#include "ipp.h"
#include <vector>
#include <string>

#define NO_DEFAULT -1

namespace ippe
{
	const size_t INITIAL_CAP = 128;
	
	template <typename T>
	class vector
	{
		private:
			size_t numel;
			size_t cap;
			size_t copylen;
			bool reMalloc;
			T *m_data;
		
			void vector_base(size_t count)
			{
				std::cout << "Base ctor for all specialized types." << std::endl;
				numel = count;
				cap = INITIAL_CAP;
				
				if (numel > cap){
					cap = numel;
				}
				reMalloc = false;
			}
		public:
			vector<T>(size_t count = 0);
			
			// common methods
			T* data(){
				return m_data;
			}
			
			T& back(){
				return m_data[numel-1];
			}
			
			T& front(){
				return m_data[0];
			}
			
			T& at(size_t pos)
			{
				if (pos < numel){
					return m_data[pos];
				}
				else{
					throw std::out_of_range(std::string("ippe::vector::range_check: Size is ") + std::to_string(numel));
				}
			}
			
			void push_back(T value) // for now lets not deal with lvalue/rvalue refs
			{
				// check size
				if (numel == cap){
					resize(cap * 2);
				}
				
				m_data[numel] = value;
				numel++;
			}
			
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
			
			bool empty()
			{
				if (numel == 0){return true;}
				else{ return false;}
			}
			
			void base_resize(size_t count)
			{
				std::cout << "Base class resize." << std::endl;
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
	
			void resize(size_t count);
			
			~vector()
			{
				std::cout<<"Destructing base ipp vector."<<std::endl;
				
				ippsFree(m_data);
			}
	};
	
	// ========== specialized ctor
	
	// default ctor
	template <typename T>
	vector<T>::vector(size_t count)
	{
		std::cout<<"There is no default template for IPP vectors. Please specify a valid IPP type." << std::endl;
		throw NO_DEFAULT;
	}
	
	// Ipp64fc ctor
	template <>
	vector<Ipp64fc>::vector(size_t count)
	{
		vector_base(count);
		
		std::cout<<"Constructing Ipp64fc vector."<<std::endl;
		
		m_data = ippsMalloc_64fc_L(cap);
	}
	
	// ========== specialized resize
	
	// default resize
	template <typename T>
	void vector<T>::resize(size_t count)
	{
		std::cout << "No default resize. This should never happen, should have been caught in ctor." << std::endl;
		throw NO_DEFAULT;
	}
	
	// Ipp64fc resize
	template <>
	void vector<Ipp64fc>::resize(size_t count)
	{
		base_resize(count);
				
		std::cout << "Ipp64fc specific resize. " << std::endl;
		if (reMalloc){
			Ipp64fc *newm_data = ippsMalloc_64fc_L(cap);
			ippsCopy_64fc(m_data, newm_data, copylen);
			ippsFree(m_data);
			m_data = newm_data;
			
			reMalloc = false;
		}
	}
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
	
	// test wrong template?
	try{
		ippe::vector<int> wrongdata;
	}
	catch(int err)
	{
		std::cout<<"Caught error " << err << std::endl;
	}
	
	// create ipp template vectors
	ippe::vector<Ipp64fc> data;
	std::cout<<"ipp vector capacity = " << data.capacity() << " and size = " << data.size() << std::endl;

	// try a resize
	data.resize(256);
	data.resize(128);
	data.resize(8);
	std::cout<<"ipp vector capacity = " << data.capacity() << " and size = " << data.size() << std::endl;
	
	// pushback some data
	Ipp64fc val = {1.0, 2.0};
	data.push_back(val);
	std::cout<<"ipp vector size after push back is " << data.size() << std::endl;
	std::cout << "pushed back value is " << data.back().re << ", " << data.back().im << std::endl;
	std::cout << "or directly, = " << data.at(8).re << ", " << data.at(8).im << std::endl;
	
	// assign to last value
	data.at(data.size()-1) = {10.0, 20.0};
	// print allocate
	for (int i = 0; i < data.size(); i++){
		std::cout << "ipp vector val at " << i << " = " << data.at(i).re << ", " << data.at(i).im << std::endl;
	}
	// test exceptions
	try{
		data.at(data.size()) = {100.0, 200.0};
	}
	catch(std::out_of_range &exc){
		std::cout<<exc.what()<<std::endl;
	}

	return 0;
}