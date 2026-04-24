#include <stdio.h>
#include "Point.h"
#include "Surface.h"

int main(){
  Point3f p = {.x = 1, .y = 2};

  printf("%f, %f, %f\n", p.x , p.y, p.z);

  Surface3f s = make_Surface3f(3);
  printf("s size %zu\n", s.size);

  return 0;
}
