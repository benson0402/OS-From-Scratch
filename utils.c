#include "utils.h"


void *memset(void *buf, uint8_t val, size_t n) {
  uint8_t *p = (uint8_t*)buf;
  while(n--)
    *p++ = val;
  return buf;
}

void *memcpy(void *dst, const void *src, size_t n) {
  uint8_t *d = (uint8_t *) dst; 
  const uint8_t *s = (const uint8_t *) src;
  while(n--)
    *d++ = *s++;
  return dst;
}

char *strcpy(char *dst, const char *src) {
  char *d = dst;
  while(*src)
    *d++ = *src++;
  *d = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *d = dst;
  while(*src && n--)
    *d++ = *src++;
  *d = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 && *s2) {
    if(*s1 != *s2)
      break;
    s1++;
    s2++;
  }
  return *(unsigned char *)s1 - *(unsigned char *)s2;
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
