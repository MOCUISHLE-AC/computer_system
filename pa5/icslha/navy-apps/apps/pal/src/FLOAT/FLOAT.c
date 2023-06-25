#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

const int FACTOR=1<<16;

typedef union
{
    float value;
    struct
    {
        unsigned m:23;//尾数
        unsigned e:8;//指数
        unsigned s:1;//符号位
    };
}ieee_float;

typedef union
{
    FLOAT value;
    struct
    {
        unsigned f:16;
        unsigned i:15;
        unsigned s:1;
    };
}Float;


FLOAT F_mul_F(FLOAT _a, FLOAT _b) {
  //assert(0);
//  Float a={_a},b={_b};
//  int sign=a.s^b.s;
//  a.value=a.s ? -a.value:a.value;
//  b.value=b.s ? -b.value:b.value;
//  int intResult=a.i*b.i*FACTOR;
//  int crossProduct1=a.f*b.i;
//  int crossProduct2=a.i*b.f;
//  FLOAT floatResult=(a.f*b.f/FACTOR)+(a.f*b.f%FACTOR>=FACTOR/2);
//  FLOAT result=intResult+crossProduct1+crossProduct2+floatResult;
//  return sign ? -result:result;

return ((uint64_t)_a*_b)>>16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    assert(b != 0);
    FLOAT x = Fabs(a);
    FLOAT y = Fabs(b);
    FLOAT ret = x / y;
    x = x % y;
    //处理低16位
    for (int i = 0; i < 16; i++) {
      x <<= 1;
      ret <<= 1;
      if (x >= y) {
        x -= y;
        ret++;
      }
    }
    if (((a ^ b) & 0x80000000) == 0x80000000) {
      ret = -ret;
    }
    return ret;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
    ieee_float value = {a};
    if(value.e==0)
    {
        return 0;
    }
    if(value.e==0xff)
    {
        //特殊值（NaN 或 Infinity）
        return (value.s ? 0x80000000 : 0x7fffffff);
    }
    int pow=value.e-127+16;
    if(pow<0)
    {
      return 0;
    }
    int base=value.m|(1<<23);
    pow-=23;
    return (value.s ? -1:1)*(pow<0 ? (base>>-pow):(base<<pow));
}

FLOAT Fabs(FLOAT a) {
    return a<0 ? -a:a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
