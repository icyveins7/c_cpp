#include "ipp.h"
#include <iostream>
#include <chrono>

int main(){
	ippInit();
	
	int len = 100000000;
	Ipp32f *data1 = ippsMalloc_32f_L(len);
	Ipp32f *data2 = ippsMalloc_32f_L(len);
	Ipp32f *data3 = ippsMalloc_32f_L(len);
	std::cout<<"Allocated for length " << len << std::endl;
	
	// test add (we just run this to make sure the ipp library is warmed up / loaded / whatever
	auto t1 = std::chrono::high_resolution_clock::now();
	ippsAdd_32f(data1, data2, data3, len);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	// std::cout << "Took  " << time_span.count() << " seconds to complete adds" << std::endl;
	
	// test add
	auto ta1 = std::chrono::high_resolution_clock::now();
	ippsAdd_32f(data1, data2, data3, len);
	auto ta2 = std::chrono::high_resolution_clock::now();
	auto time_spana = std::chrono::duration_cast<std::chrono::duration<double>>(ta2 - ta1);
	std::cout << "Took  " << time_spana.count() << " seconds to complete adds" << std::endl;
	
	// test round
	auto tround1 = std::chrono::high_resolution_clock::now();
	ippsRound_32f (data1, data3, len);
	auto tround2 = std::chrono::high_resolution_clock::now();
	auto tround = std::chrono::duration_cast<std::chrono::duration<double>>(tround2 - tround1);
	std::cout << "Took  " << tround.count() << " seconds to complete rounds" << std::endl;
	
	// test mul
	auto tmul1 = std::chrono::high_resolution_clock::now();
	ippsMul_32f(data1, data2, data3, len);
	auto tmul2 = std::chrono::high_resolution_clock::now();
	auto tmul = std::chrono::duration_cast<std::chrono::duration<double>>(tmul2 - tmul1);
	std::cout << "Took  " << tmul.count() << " seconds to complete muls" << std::endl;
	
	
	// test addC
	auto tac1 = std::chrono::high_resolution_clock::now();
	ippsAddC_32f(data1, 1.23, data3, len);
	auto tac2 = std::chrono::high_resolution_clock::now();
	auto time_spanac = std::chrono::duration_cast<std::chrono::duration<double>>(tac2 - tac1);
	std::cout << "Took  " << time_spanac.count() << " seconds to complete addCs" << std::endl;
	
	// test mulC
	auto tmulc1 = std::chrono::high_resolution_clock::now();
	ippsMulC_32f(data1, 1.23, data3, len);
	auto tmulc2 = std::chrono::high_resolution_clock::now();
	auto tmulc = std::chrono::duration_cast<std::chrono::duration<double>>(tmulc2 - tmulc1);
	std::cout << "Took  " << tmulc.count() << " seconds to complete mulCs" << std::endl;
	
	// test modf
	auto tmodf1 = std::chrono::high_resolution_clock::now();
	ippsModf_32f (data1, data2, data3, len);
	auto tmodf2 = std::chrono::high_resolution_clock::now();
	auto tmodf = std::chrono::duration_cast<std::chrono::duration<double>>(tmodf2 - tmodf1);
	std::cout << "Took  " << tmodf.count() << " seconds to complete modfs" << std::endl;
	
	
	// cleanup
	ippsFree(data1);
	ippsFree(data2);
	ippsFree(data3);
	
	return 0;
}