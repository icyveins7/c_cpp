#include <iostream>

// Declare the general template but with no definition
template <typename T>
void unused_template_print(T i);

// Only allow specializations
template <>
void unused_template_print<int>(int i)
{
    std::cout << i + 1 << std::endl;
}

template <>
void unused_template_print<double>(double i)
{
    std::cout << i + 1.0 << std::endl;
}

// Use function overloading instead
void unused_print(int i)
{
    std::cout << i << std::endl;
}

void unused_print(double i)
{
    std::cout << i << std::endl;
}

int main()
{
    unused_print(1.0);
    unused_template_print(1.0);

    return 0;
}