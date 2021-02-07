#include <iostream>
#include <Eigen/Dense>
#include "ipp.h"
#include <complex>
 
using namespace Eigen;
 
int main()
{
	MatrixXd m(2,2);
	m(0,0) = 3;
	m(1,0) = 2.5;
	m(0,1) = -1;
	m(1,1) = m(1,0) + m(0,1);
	std::cout << m << std::endl;
	
	// using Map with existing memory
	int rows = 2; int cols = 2;
	Ipp64fc *data = ippsMalloc_64fc_L(rows*cols);
	
	for (int i = 0; i < rows * cols; i++){
		data[i].re = i + 1.0;
		data[i].im = i + 2.0;
	}
	
	Map<Matrix<std::complex<double>, Dynamic, Dynamic>> mapped;
	
	std::cout<<"mapped is init at size " << mapped.rows() << ", " << mapped.cols() << std::endl;
	
	// cleanup
	ippsFree(data);
	
	return 0;
}