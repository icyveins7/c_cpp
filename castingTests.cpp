#include <iostream>

struct fc {
    float re;
    float im;
};

struct fd : fc
{
    float ex;
};

int main()
{
    fc a = {1.1,2.2};

    float *b = reinterpret_cast<float*>(&a); // reinterpret_cast needed to 'assume' the memory ordering, static_cast doesn't work
    printf("%f %f\n", b[0], b[1]);

    fd c;
    c.re = 1.1; c.im = 2.2; c.ex = 3.3;

    fc *d = &c; // no static_cast required
    printf("upcast %f %f\n", d->re, d->im);

    fd *e = static_cast<fd*>(d); // static_cast required to downcast
    printf("downcast %f %f %f\n", e->re, e->im, e->ex);

    return 0;
}