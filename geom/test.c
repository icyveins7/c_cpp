#include <stdio.h>
#include "Point.h"
#include "Surface.h"

int main(){
  Point3f p = {.x = 1, .y = 2};

  printf("%f, %f, %f\n", p.x , p.y, p.z);

  Polygon poly = make_Polygon(3);
  printf("poly size %zu\n", poly.size);

  Rectangle rect = {.x0 = {.x = 0, .y = 0}, .width = 1.0f, .height = 2.0f};
  Polygon r = polygonize_Rectangle(rect);
  for(size_t i = 0; i < r.size; i++){
    printf("%f, %f\n", r.pts[i].x, r.pts[i].y);
  }
  printf("r size %zu\n\n", r.size);

  Point2f outsideRect = {.x = 3, .y = 3};
  bool isInRect = in_Rectangle(outsideRect, rect);
  printf("%f, %f %s rect\n",
      outsideRect.x,
      outsideRect.y,
      isInRect ? "in" : "not in");

  Point2f insideRect = {.x = 0.5f, .y = 1.0f};
  isInRect = in_Rectangle(insideRect, rect);
  printf("%f, %f %s rect\n",
      insideRect.x,
      insideRect.y,
      isInRect ? "in" : "not in");

  Ellipse e = {.centre = {.x = 0, .y = 0}, .rx = 1.0f, .ry = 1.0f};
  printf("ellipse %f, %f, rx %f, ry %f\n", e.centre.x, e.centre.y, e.rx, e.ry);
  Point2f insideEllipse = {.x = 0.5f, .y = 0.5f};
  isInRect = in_Ellipse(insideEllipse, e);
  printf("%f, %f %s ellipse\n",
      insideEllipse.x,
      insideEllipse.y,
      isInRect ? "in" : "not in");

  Point2f outsideEllipse = {.x = 2.5f, .y = 1.0f};
  isInRect = in_Ellipse(outsideEllipse, e);
  printf("%f, %f %s ellipse\n",
      outsideEllipse.x,
      outsideEllipse.y,
      isInRect ? "in" : "not in");

  return 0;
}
