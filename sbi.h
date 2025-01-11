#pragma once

#include "kernel.h"

struct sbiret {
  long error;
  long value;
};

void putchar(char ch);

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid);

