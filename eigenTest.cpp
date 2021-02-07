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
	
	Map<MatrixXcd> mapped((std::complex<double>*)data, rows, cols);
	
	std::cout<<"mapped is init at size " << mapped.rows() << ", " << mapped.cols() << std::endl;
	
	std::cout << mapped << std::endl;
	
	// remap it as row major?
	Map<Matrix<std::complex<double>, Dynamic, Dynamic, RowMajor>> rowmapped((std::complex<double>*)data, rows, cols);
	
	std::cout << "remapped as row major" << std::endl << rowmapped << std::endl;
	
	// make a new matrix to store matmul result
	MatrixXcd matmul(rows, cols);
	matmul = mapped * mapped;
	
	std::cout << "matmul product of mapped*mapped = " << std::endl << matmul << std::endl;
	
	matmul = rowmapped * rowmapped;
	
	std::cout << "rowmajor matmul product of mapped*mapped = " << std::endl << matmul << std::endl;
	
	// what if i make a wrong sized one?
	MatrixXcd matmul_wrong(rows,cols-1);
	
	std::cout << "init matmul prod size at " << matmul_wrong.rows() << ", " << matmul_wrong.cols() << std::endl;
	
	matmul_wrong = mapped * mapped;
	
	std::cout << "matmul product of mapped*mapped = " << std::endl << matmul_wrong << std::endl;
	std::cout << "but the size is now " << matmul_wrong.rows() << ", " << matmul_wrong.cols() << std::endl; // clearly this shows that a reallocation is performed for the product
	
	// now what if i make a wrong sized map?
	Ipp64fc *muldata = ippsMalloc_64fc_L(rows * (cols-1));
	
	Map<MatrixXcd> mapmul((std::complex<double>*)muldata, rows, cols-1);
	
	mapmul = mapped * mapped; // here is where it fails! which is good
	
	std::cout << "mapped matmul product of mapped*mapped = " << std::endl << mapmul << std::endl;	
	
	
	// cleanup
	ippsFree(data);
	ippsFree(muldata);
	
	return 0;
}