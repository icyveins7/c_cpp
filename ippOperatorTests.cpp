#include "ipp.h"
#include <iostream>
#include <chrono>

int main(){
	ippInit();
	
	int len = 100000000;
	Ipp32f *data1 = ippsMalloc_32f_L(len);
	Ipp32f *data2 = ippsMalloc_32f_L(len);
	Ipp32f *data3 = ippsMalloc_32f_L(len);
	Ipp32fc *datafc1 = ippsMalloc_32fc_L(len);
	Ipp32fc *datafc2 = ippsMalloc_32fc_L(len);
	Ipp32fc *datafc3 = ippsMalloc_32fc_L(len);
	std::cout<<"Allocated for length " << len << std::endl;
	std::cout<<"All tests done in 32f or 32fc (not 64f/64fc), just as a gauge." << std::endl;
	
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
	
	// test sin
	auto tsin1 = std::chrono::high_resolution_clock::now();
	ippsSin_32f_A24 (data1, data2, len);
	auto tsin2 = std::chrono::high_resolution_clock::now();
	auto tsin = std::chrono::duration_cast<std::chrono::duration<double>>(tsin2 - tsin1);
	std::cout << "Took  " << tsin.count() << " seconds to complete sins, A24 (max for 32f)" << std::endl;
	
	// test cos
	auto tcos1 = std::chrono::high_resolution_clock::now();
	ippsCos_32f_A24 (data1, data2, len);
	auto tcos2 = std::chrono::high_resolution_clock::now();
	auto tcos = std::chrono::duration_cast<std::chrono::duration<double>>(tcos2 - tcos1);
	std::cout << "Took  " << tcos.count() << " seconds to complete cos, A24 (max for 32f)" << std::endl;
	
	// test sincos
	auto tsincos1 = std::chrono::high_resolution_clock::now();
	ippsSinCos_32f_A24 (data1, data2, data3, len);
	auto tsincos2 = std::chrono::high_resolution_clock::now();
	auto tsincos = std::chrono::duration_cast<std::chrono::duration<double>>(tsincos2 - tsincos1);
	std::cout << "Took  " << tsincos.count() << " seconds to complete sincos, A24 (max for 32f)" << std::endl;
	
	// test tone
	Ipp32f phase = 0;
	auto ttone1 = std::chrono::high_resolution_clock::now();
	ippsTone_32f(data1, len, 1.0f, 0.2f, &phase, ippAlgHintAccurate);
	auto ttone2 = std::chrono::high_resolution_clock::now();
	auto ttone = std::chrono::duration_cast<std::chrono::duration<double>>(ttone2 - ttone1);
	std::cout << "Took  " << ttone.count() << " seconds to complete tone" << std::endl;
	
	// test phase
	auto tphase1 = std::chrono::high_resolution_clock::now();
	ippsPhase_32fc(datafc1, data1, len);
	auto tphase2 = std::chrono::high_resolution_clock::now();
	auto tphase = std::chrono::duration_cast<std::chrono::duration<double>>(tphase2 - tphase1);
	std::cout << "Took  " << tphase.count() << " seconds to complete ippsPhase_32fc" << std::endl;
	
	// test mul complex
	tmul1 = std::chrono::high_resolution_clock::now();
	ippsMul_32fc(datafc1, datafc2, datafc3, len);
	tmul2 = std::chrono::high_resolution_clock::now();
	tmul = std::chrono::duration_cast<std::chrono::duration<double>>(tmul2 - tmul1);
	std::cout << "Took  " << tmul.count() << " seconds to complete ippsMul_32fc" << std::endl;
		
	
	// cleanup
	ippsFree(data1);
	ippsFree(data2);
	ippsFree(data3);
	ippsFree(datafc1);
	ippsFree(datafc2);
	ippsFree(datafc3);
	
	return 0;
}
