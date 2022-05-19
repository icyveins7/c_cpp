#include <iostream>
#include <complex>

#define NROWS 2
#define NCOLS 2

int main()
{
    std::complex<double> b[NROWS][NCOLS] = {
        {{10,0}, {12,4}},
        {{12,-4}, {26,0}}
    };


    for (int i = 0; i < NROWS; i++)
    {
        for (int j = 0; j < NCOLS; j++)
        {
            std::cout << b[i][j];
        }
    }

    std::cout << std::endl << b[0][1] * b[1][0] << std::endl << std::endl;

    // begin cholesky
    std::complex<double> R[NROWS][NCOLS]; // final output 
    // pre-zero output R
    std::complex<double> Rii; // the first value for each row
    for (int row = 0; row < NROWS; row++)
    {
        // compute Rii
        Rii = b[row][row];
        for (int i = 0; i < row; i++)
        {
            Rii -= R[i][row] * R[i][row]; // Yii - R_ki^2
        }
        Rii = sqrt(Rii.real()); // it is defined to be real, so we can do this
        
        // Write the values in the row
        for (int col = row; col < NCOLS; col++)
        {
            if (row == col){ R[row][col] = Rii; }
            else
            {
                R[row][col] = b[row][col];
                for (int i = 0; i < ) // TODO: complete
                R[row][col] = b[row][col] / Rii; // Yij / Rii
            }

        }

    }


    return 0;
}