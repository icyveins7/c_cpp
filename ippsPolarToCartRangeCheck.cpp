#include "ipp.h"
#include <iostream>

int main()
{
    Ipp64f a[4] = {1.0, 1.0, 1.0, 1.0};
    Ipp64f phase[4] = {IPP_PI, IPP_PI*3, IPP_PI*5, IPP_PI*7};
    
    Ipp64fc cart[4];
    
    ippsPolarToCart_64fc(a, phase, cart, 4);
    
    for (int i = 0; i < 4; i++){
        printf("%d: %f + %f i \n", i, cart[i].re, cart[i].im);
    }

    return 0;
}
