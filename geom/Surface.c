#include "Surface.h"
#include <stdlib.h>

Surface3f make_Surface3f(size_t size){
  Point3f* pts = (Point3f*)malloc(sizeof(Point3f)*size);
  Surface3f surf = {.pts=pts, .size=size};
  return surf;
}

void free_Surface3f(Surface3f *s){
  free(s->pts);
  s->pts = NULL;
  s->size = 0;
}
