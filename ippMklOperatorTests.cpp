//  mkl_intel_lp64_dll.lib mkl_sequential_dll.lib mkl_core_dll.lib (dynamic)
//  mkl_intel_lp64.lib mkl_sequential.lib mkl_core.lib (static, probably use this)

#include "ipp.h"
#include "mkl.h"
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
	data1[0] = 1.5;
	data1[1] = -1.5;
	data1[2] = 2.3;
	data1[3] = -2.4;
	auto tmodf1 = std::chrono::high_resolution_clock::now();
	ippsModf_32f (data1, data2, data3, len);
	auto tmodf2 = std::chrono::high_resolution_clock::now();
	auto tmodf = std::chrono::duration_cast<std::chrono::duration<double>>(tmodf2 - tmodf1);
	std::cout << "Took  " << tmodf.count() << " seconds to complete modfs" << std::endl;
	for (int i = 0; i < 4; i++){
		printf("modf(%f) = %f, %f \n", data1[i], data2[i], data3[i]);
	}
	
	// // test mkl remainder (this is extremely long)
	// auto trem1 = std::chrono::high_resolution_clock::now();
	// vsRemainder(len, data1, data2, data3);
	// auto trem2 = std::chrono::high_resolution_clock::now();
	// auto trem = std::chrono::duration_cast<std::chrono::duration<double>>(trem2-trem1);
	// std::cout << "Took  " << trem.count() << " seconds to complete mkl:remainder" << std::endl;
	
	// test mkl add for comparison
	auto tmadd1 = std::chrono::high_resolution_clock::now();
	vsAdd(len, data1, data2, data3);
	auto tmadd2 = std::chrono::high_resolution_clock::now();
	auto tmadd = std::chrono::duration_cast<std::chrono::duration<double>>(tmadd2-tmadd1);
	std::cout << "Took  " << tmadd.count() << " seconds to complete mkl:add" << std::endl;
	
	// // test mkl fmod (also excessively long)
	// auto tfmod1 = std::chrono::high_resolution_clock::now();
	// vsFmod(len, data1, data2, data3);
	// auto tfmod2 = std::chrono::high_resolution_clock::now();
	// auto tfmod = std::chrono::duration_cast<std::chrono::duration<double>>(tfmod2-tfmod1);
	// std::cout << "Took  " << tfmod.count() << " seconds to complete mkl:fmod" << std::endl;
	
	// test phase
	auto tphase1 = std::chrono::high_resolution_clock::now();
	ippsPhase_32fc((Ipp32fc*)data1, data2, len/2);
	auto tphase2 = std::chrono::high_resolution_clock::now();
	auto tphase = std::chrono::duration_cast<std::chrono::duration<double>>(tphase2-tphase1);
	std::cout << "Took  " << tphase.count() << " seconds to complete phases" << std::endl;
	
	// test naive loops
	auto tnadd1 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < len; i++){
		data3[i] = data1[i] + data2[i];
	}
	auto tnadd2 = std::chrono::high_resolution_clock::now();
	auto tnadd = std::chrono::duration_cast<std::chrono::duration<double>>(tnadd2-tnadd1);
	std::cout << "Took  " << tnadd.count() << " seconds to complete naive loop adds" << std::endl;
	
	// test naive muls
	auto tnmul1 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < len; i++){
		data3[i] = data1[i] * data2[i];
	}
	auto tnmul2 = std::chrono::high_resolution_clock::now();
	auto tnmul = std::chrono::duration_cast<std::chrono::duration<double>>(tnmul2-tnmul1);
	std::cout << "Took  " << tnmul.count() << " seconds to complete naive loop muls" << std::endl;
	
	// test branching
	int cnt = 0;
	auto tnmulif1 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < len; i++){
		if (data1[i] < 1.5){
			data3[i] = data1[i] * data2[i];
			cnt++;
		}
	}
	auto tnmulif2 = std::chrono::high_resolution_clock::now();
	auto tnmulif = std::chrono::duration_cast<std::chrono::duration<double>>(tnmulif2-tnmulif1);
	std::cout << "Took  " << tnmulif.count() << " seconds to complete naive loop muls with if statements, successes = " << cnt << std::endl;
	
	// cleanup
	ippsFree(data1);
	ippsFree(data2);
	ippsFree(data3);
	
	return 0;
}