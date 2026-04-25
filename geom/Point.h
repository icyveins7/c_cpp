#pragma once

typedef struct Point2f {
  float x;
  float y;
} Point2f;

Point2f add_Point2f(Point2f a, Point2f b);
Point2f sub_Point2f(Point2f a, Point2f b);
void negate_Point2f(Point2f *p);
float magnSq_Point2f(Point2f p);
float magn_Point2f(Point2f p);

typedef struct Point3f {
  float x;
  float y;
  float z;
} Point3f;

Point3f add_Point3f(Point3f a, Point3f b);
Point3f sub_Point3f(Point3f a, Point3f b);
void negate_Point3f(Point3f *p);
