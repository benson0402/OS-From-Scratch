#pragma once

#include "kernel.h"
#include "sbi.h"

// Variadic Function Builtins - Clang
// ref: https://clang.llvm.org/docs/LanguageExtensions.html#variadic-function-builtins
#define va_list   __builtin_va_list
#define va_start  __builtin_va_start
#define va_end    __builtin_va_end
#define va_arg    __builtin_va_arg


void *memset(void *buf, uint8_t val, size_t n);
const char *itoa(int val);
void printf(const char *fmt, ...);
