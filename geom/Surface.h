#pragma once

#include "Point.h"
#include <stddef.h>
#include <stdbool.h>

#define SUCCESS 0
#define ERROR_FAILED_ALLOC 1
#define ERROR_INVALID_SURFACE_SIZE 2


typedef struct Polygon {
  size_t size;
  Point2f *pts;
} Polygon;

Polygon make_Polygon(size_t size);
void free_Polygon(Polygon *s);

typedef struct Rectangle {
  Point2f x0; // bottom-left
  float width;
  float height;
} Rectangle;

Polygon polygonize_Rectangle(Rectangle r);
bool in_Rectangle(Point2f p, Rectangle r);

typedef struct Ellipse {
  Point2f centre;
  float rx;
  float ry;
} Ellipse;

bool in_Ellipse(Point2f p, Ellipse e);

// TODO: Maybe worth to define Square/Circle as special cases, since
// they use less memory and the calculations are simpler too?

typedef enum {
  INVALID,
  POLYGON,
  RECTANGLE,
  ELLIPSE
} ShapeType;

typedef struct Surface_f {
  ShapeType type;
  float z;
  union {
    Polygon polygon;
    Rectangle rectangle;
    Ellipse ellipse;
  };
} Surface_f;

Surface_f make_Surface_f(ShapeType type, float z, void *shape);
