#include <iostream>
#include "ipp.h"
#include <chrono>
#include "ipp_ext.h"

int main(){
	const int len = 1000000;
	
	// load input x
	ippe::vector<Ipp64f> xx(len);
	FILE *fp = fopen("lerp_xx.bin","rb");
	fread(xx.data(), sizeof(Ipp64f), len, fp);
	fclose(fp);
	
	// load input y
	ippe::vector<Ipp64f> yy(len);
	fp = fopen("lerp_yy.bin","rb");
	fread(yy.data(), sizeof(Ipp64f), len, fp);
	fclose(fp);
	
	printf("xx[0] = %f\nxx[-1] = %f\n", xx.front(), xx.back());
	printf("yy[0] = %f\nyy[-1] = %f\n", yy.front(), yy.back());
	
	// load input xxq
	const int anslen = 1000000; // in this case, the same
	ippe::vector<Ipp64f> xxq(anslen);
	fp = fopen("lerp_xxq.bin", "rb");
	fread(xxq.data(), sizeof(Ipp64f), anslen, fp);
	fclose(fp);
	
	printf("xxq[0] = %f\nxxq[-1] = %f\n", xxq.front(), xxq.back());
	
	// create workspace arrays
	ippe::vector<Ipp64f> divAns(anslen);
	ippe::vector<Ipp64f> intPart(anslen);
	ippe::vector<Ipp64f> remPart(anslen);
	ippe::vector<Ipp32s> indexes(anslen);
	ippe::vector<Ipp64f> yyq(anslen); // output vector
	
	// start timer
	auto t1 = std::chrono::high_resolution_clock::now();
	
	// divide first
	ippsDivC_64f(xxq.data(), 1.0, divAns.data(), anslen);
	// modf the whole array
	ippsModf_64f(divAns.data(), intPart.data(), remPart.data(), anslen);
	// convert to integers for indexing
	ippsConvert_64f32s_Sfs(intPart.data(), indexes.data(), anslen, ippRndNear, 0);
	// reuse the intPart which is not needed any more as the gradients vector
	Ipp64f *gradients = intPart.data();
	ippsZero_64f(gradients, anslen); // zero it out
	Ipp32s idx;
	for (int qi = 0; qi < anslen; qi++){
		idx = indexes.at(qi);
		if (idx >=0 && idx < len-1){ // don't index outside, we need to access the next point as well
			gradients[qi] = (yy.at(idx+1) - yy.at(idx)) / 1.0;
		}
		yyq.at(qi) = yy.at(idx); // write the output value as well
	}
	// multiply gradients into the decimal part and add to output
	ippsAddProduct_64f(remPart.data(), gradients, yyq.data(), anslen);
	
	// end timer
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Took " << std::chrono::duration<double>(t2-t1).count() << " seconds" << std::endl;
	
	// checking output
	for (int i = 0; i < 5; i++){
		printf("yyq[%d] = %f\n", i, yyq.at(i));
	}
	for (int i = -5; i < 0; i++){
		printf("yyq[%d] = %f\n", i, yyq.at(yyq.size()+i));
	}
	
	
	return 0;
}