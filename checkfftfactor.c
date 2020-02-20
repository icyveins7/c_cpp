#include <stdio.h>
#include <stdlib.h>

int checkFFTfactorization(int check, int *outfactors, int *numFactors){
	int rem = check;
	int count = 0;
	int factors[5] = {2,3,5,7,11}; // define the factors to check for

	for(int i=0;i<5;i++){
		
		while(rem%factors[i]==0){
			rem = rem/factors[i];
			outfactors[count] = factors[i];;
			count++;
		}
	}
	*numFactors = count;
	
	return rem;

}

int getNearestDecentFactorization(int start, int *factors, int *numFactors){
	int rem = 0;
	int i = start;
	while(rem!=1){
		rem = checkFFTfactorization(i, factors, numFactors);
		i++;
	}

	return i;
}

int main(){

	int numFactors;
	int factors[64];
	int rem;
	for(int i=25; i<31;i++){
		rem = checkFFTfactorization(i, factors, &numFactors);
		if (rem != 1){
			printf("%i not factorized, remainder %i\n",i,rem);
		}
		else{
			printf("%i factorised fully\n",i);
		}
		for(int j=0;j<numFactors;j++){
			printf("%i\n",factors[j]);
		}
	}

	int padlen=getNearestDecentFactorization(202000, factors, &numFactors);
	printf("First decent factorization is at %i\n",padlen);
	for(int j=0;j<numFactors;j++){
		printf("%i\n",factors[j]);
	}

	return 0;
}


