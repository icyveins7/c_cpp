#include <stdio.h>
#include <stdint.h>

struct BaseOffset{
  int m;
  float c;
};

void initBaseOffset(struct BaseOffset* ba, int _m, int x){
  ba->m = _m;
  ba->c = x - _m;
}

double baseOffsetAsDouble(struct BaseOffset* ba){
    return (double)ba->m + (double)ba->c;
  }

double diff(int a, int b){
  struct BaseOffset ba;
  initBaseOffset(&ba, a, b);
  return baseOffsetAsDouble(&ba);
}

double lerp(int a, int b, float x){
  struct BaseOffset ba;
  initBaseOffset(&ba, a, b);
  ba.c *= x;
  return baseOffsetAsDouble(&ba);
}

int main(){
  printf("test floats emulating doubles\n");

  int32_t x = 678912345;
  float xf = x;
  double xd = x;
  printf("x = %d\nxf = %12.8f\nxd = %12.8f\n", x, xf, xd);


  int32_t y = 678912345 + 1;
  float yf = y;
  double yd = y;
  printf("y = %d\nyf = %12.8f\nyd = %12.8f\n", y, yf, yd);

  
  int32_t z = y - x;
  float zf = yf - xf;
  double zd = yd - xd;
  printf("z = %d\nzf = %12.8f\nzd = %12.8f\n", z, zf, zd);

  int32_t m = x;
  int32_t xc = x - m;
  int32_t yc = y - m;
  float xcf = xc, ycf = yc;
  float zcf = ycf - xcf;
  printf("m = %d\nxc = %d\nyc = %d\n", m, xc, yc);
  printf("xcf = %12.8f\nycf = %12.8f\nzcf = %12.8f\n", xcf, ycf, zcf);

  double zdd = diff(x, y);
  printf("zdd = %12.8f\n", zdd);

  float k = 0.123f;
  double zl = xd + (yd-xd)*k;
  double zll = lerp(x, y, k);
  printf("zl = %12.8f\nzll = %12.8f\n", zl, zll);

  return 0;
}
