#pragma once

#include "Point.h"
#include "stddef.h"

typedef struct Surface3f {
  Point3f *pts;
  size_t size;
} Surface3f;

Surface3f make_Surface3f(size_t size);
void free_Surface3f(Surface3f *s);


