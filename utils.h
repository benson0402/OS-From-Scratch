#pragma once

#include "sbi.h"

typedef int bool;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef uint32_t            size_t;
typedef uint32_t            paddr_t;
typedef uint32_t            vaddr_t;

#define true  1
#define false 0
#define NULL  ((void *)(0))

// Alignment Builtins
// ref: https://clang.llvm.org/docs/LanguageExtensions.html#alignment-builtins
#define align_up(val, align)    __builltin_align_up(val, align)
#define is_aligned(val, align)  __builltin_is_aligned(val, align)
#define offsetof(val, align)    __builltin_offsetof(val, align)

// Variadic Function Builtins
// ref: https://clang.llvm.org/docs/LanguageExtensions.html#variadic-function-builtins
#define va_list   __builtin_va_list
#define va_start  __builtin_va_start
#define va_end    __builtin_va_end
#define va_arg    __builtin_va_arg


void *memset(void *buf, uint8_t val, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);

const char *itoa(int val);
void printf(const char *fmt, ...);
