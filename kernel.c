#include "kernel.h"
#include "utils.h"
#include "sbi.h"

extern uint8_t __bss[], __bss_end[], __stack_top[];


__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
  __asm__ __volatile__(
    "mv sp, %[stack_top]\n"
    "j kernel_main\n"
    :
    : [stack_top] "r" (__stack_top)
  );
}


void kernel_main(void) {
  // Clear the .bss section (Block Started by Symbol)
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  // Example printf usage
  printf("\n\nHello %s \n", "World!");
  printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);
  printf("MAX_INT = %d, MIN_INT = %d\n", 2147483647, -2147483648);

  PANIC("booted!\n");
  printf("no printed");

  for (;;) {
    __asm__ __volatile__("wfi"); // Wait For Interrupt
  }
}
