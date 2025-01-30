// g++ main.cpp a.cpp -o main
#include <iostream>

// Demonstrate that you can compile this even though it's a mismatch
extern "C" float func(float& x);

int main()
{
    float x = 2.5f;
    printf("%p\n", &x);
    float& xref = x;

    float v = func(xref);
    printf("%f\n", v);

    return 0;
}
