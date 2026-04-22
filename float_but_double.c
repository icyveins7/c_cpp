#include <stdio.h>
#include <stdint.h>

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

  

  return 0;
}
