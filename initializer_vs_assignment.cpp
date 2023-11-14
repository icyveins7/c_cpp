/*
Usually initializer lists are recommended over assignment of member vars.
But this is not necessary for inbuilt types. Here we check this.

Easiest way to do this is to use https://godbolt.org/, so I'm gonna do that.

As you can see, the assembly is identical! Even better, the separate assignment outside the constructor is also equivalent!

Assigned::Assigned(unsigned int) [base object constructor]:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     DWORD PTR [rbp-12], esi
        mov     rax, QWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rbp-12]
        mov     DWORD PTR [rax], edx
        nop
        pop     rbp
        ret
AssignedAfter::set_x(unsigned int):
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     DWORD PTR [rbp-12], esi
        mov     rax, QWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rbp-12]
        mov     DWORD PTR [rax], edx
        nop
        pop     rbp
        ret
Initialized::Initialized(unsigned int) [base object constructor]:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     DWORD PTR [rbp-12], esi
        mov     rax, QWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rbp-12]
        mov     DWORD PTR [rax], edx
        nop
        pop     rbp
        ret
*/

#include <iostream>

class Assigned
{
public:
    Assigned(unsigned int x){
        m_x = x;
    }


    unsigned int m_x;
};

class AssignedAfter
{
public:
    void set_x(unsigned int x){ m_x = x; }

private:
    
    unsigned int m_x;
};

class Initialized
{
public:
    Initialized(unsigned int x) : m_x(x) 
    {

    }


    unsigned int m_x;
};




int main()
{
    Assigned A(0xFFFFFFFF);
    Initialized I(0xFFFFFFFF);
    AssignedAfter B;
    B.set_x(0xFFFFFFFF);

    std::cout << A.m_x << std::endl;
    std::cout << I.m_x << std::endl;

    return 0;
}