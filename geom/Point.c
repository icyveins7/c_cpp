#include "Point.h"

#include <math.h>

Point2f add_Point2f(Point2f a, Point2f b){
  return (Point2f){.x = a.x + b.x, .y = a.y + b.y};
}

Point2f sub_Point2f(Point2f a, Point2f b){
  // a - b
  return (Point2f){.x = a.x - b.x, .y = a.y - b.y};
}

void negate_Point2f(Point2f *p){
  p->x = -p->x;
  p->y = -p->y;
}

float magnSq_Point2f(Point2f p){
  return p.x * p.x + p.y * p.y;
}

float magn_Point2f(Point2f p){
  return sqrtf(p.x * p.x + p.y * p.y);
}

Point3f add_Point3f(const Point3f a, const Point3f b){
  return (Point3f){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

Point3f sub_Point3f(const Point3f a, const Point3f b){
  // a - b
  return (Point3f){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

void negate_Point3f(Point3f *p){
  p->x = -p->x;
  p->y = -p->y;
  p->z = -p->z;
}
