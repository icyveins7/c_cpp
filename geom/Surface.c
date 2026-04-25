#include "Surface.h"
#include "Point.h"
#include <stdlib.h>

Polygon make_Polygon(size_t size){
  Point2f* pts = (Point2f*)malloc(sizeof(Point2f)*size);
  if (pts == NULL){
    return (Polygon){.pts=NULL, .size=0};
  }
  Polygon poly = {.pts=pts, .size=size};
  return poly;
}

void free_Polygon(Polygon *s){
  free(s->pts);
  s->pts = NULL;
  s->size = 0;
}

Polygon polygonize_Rectangle(Rectangle r){
  Polygon p = make_Polygon(4);
  Point2f xdir = {.x = r.width, .y = 0};
  Point2f ydir = {.x = 0, .y = r.height};
  p.pts[0] = r.x0;
  p.pts[1] = add_Point2f(r.x0, xdir);
  p.pts[2] = add_Point2f(p.pts[1], ydir);
  p.pts[3] = sub_Point2f(p.pts[2], xdir);
  return p;
}

bool in_Rectangle(Point2f p, Rectangle r){
  return (p.x >= r.x0.x && p.x <= r.x0.x + r.width && p.y >= r.x0.y && p.y <= r.x0.y + r.height);
}

bool in_Ellipse(Point2f p, Ellipse e){
  Point2f dir = sub_Point2f(p, e.centre);
  float term1 = (dir.x * dir.x) / (e.rx * e.rx);
  float term2 = (dir.y * dir.y) / (e.ry * e.ry);
  if (term1 + term2 > 1.0f){
    return false;
  }
  else {
    return true;
  }
}

Surface_f make_Surface_f(ShapeType type, float z, void* shape){
  Surface_f s;
  s.type = type;
  s.z = z;
  switch(type){
    case POLYGON:
      s.polygon = *(Polygon*)shape;
      break;
    case RECTANGLE:
      s.rectangle = *(Rectangle*)shape;
      break;
    case ELLIPSE:
      s.ellipse = *(Ellipse*)shape;
      break;
    default:
      s.type = INVALID;
      break;
  }
  return s;
}

