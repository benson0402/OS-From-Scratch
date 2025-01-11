#include "utils.h"


void *memset(void *buf, uint8_t val, size_t n) {
  uint8_t *p = (uint8_t*)buf;
  while(n--)
    *p++ = val;
  return buf;
}

const char *itoa(int val) {
  static char buf[16];
  memset(buf, 0, 16);

  int i = 14;
  if(val == 0) {
    buf[i--] = '0';
    return &buf[i+1];
  }

  int sign = 0;
  if(val < 0) {
    sign = 1;
  }
  for(; val && i ; --i, val /= 10) {
    int digit = val % 10;
    if(digit < 0) {
      digit = -digit;
    }
    buf[i] = "0123456789"[digit];
  }
  if(sign)
    buf[i--] = '-';
  return &buf[i+1];
}


void putchar(char ch);

void printf(const char *fmt, ...) {
  va_list vargs;
  va_start(vargs, fmt);

  while(*fmt) {
    if(*fmt == '%') {
      fmt++;
      switch(*fmt) {
        case '\0': { // '%' is at the end of fmt
          putchar('%');
          goto end;
        }
        case '%': { // '%%' in fmt
          putchar('%');
          break;
        }
        case 'd': { // integer
          int val = va_arg(vargs, int);
          const char *str = itoa(val);
          while(*str) {
            putchar(*str);
            str++;
          }
          break;
        }
        case 's': { // string
          const char *str = va_arg(vargs, const char *);
          while(*str) {
            putchar(*str);
            str++;
          }
          break;
        }
        case 'x': { // hex
          int val = va_arg(vargs, int);
          for(int i = 7; i >= 0; --i) {
            int digit = (val >> (i * 4)) & 0xf;
            putchar("0123456789abcdef"[digit]);
          }
          break;
        }
      }
    }
    else {
      putchar(*fmt);
    }
    fmt++;
  }

end:
  va_end(vargs);
}
