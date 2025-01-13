#pragma once

#include "utils.h"

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


#define PANIC(fmt, ...)                                                    \
  do {                                                                     \
    printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
    while (1) {}                                                           \
  } while (0)


/*
 *
 * Exception Handler
 *
 */

struct trap_frame {
  uint32_t ra;
  uint32_t gp;
  uint32_t tp;
  uint32_t t0;
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t4;
  uint32_t t5;
  uint32_t t6;
  uint32_t a0;
  uint32_t a1;
  uint32_t a2;
  uint32_t a3;
  uint32_t a4;
  uint32_t a5;
  uint32_t a6;
  uint32_t a7;
  uint32_t s0;
  uint32_t s1;
  uint32_t s2;
  uint32_t s3;
  uint32_t s4;
  uint32_t s5;
  uint32_t s6;
  uint32_t s7;
  uint32_t s8;
  uint32_t s9;
  uint32_t s10;
  uint32_t s11;
  uint32_t sp;
} __attribute__((packed));

#define READ_CSR(reg) \
  ({ \
    unsigned long __tmp; \
    __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; \
  })

#define WRITE_CSR(reg, val) \
  do { \
    unsigned long __tmp = (val); \
    __asm__ __volatile__("csrw " #reg ", %0" :: "r"(__tmp)); \
  } while(0)

/*
 *
 * Memory Management
 *
 */

#define PAGE_SIZE 4096


/*
 *
 * Process
 *
 */

#define PROCS_MAX 8

#define PROC_UNUSED 0
#define PROC_RUNNABLE 1

struct process {
  int pid;
  int state;
  vaddr_t sp;
  uint8_t stack[8192];
};
