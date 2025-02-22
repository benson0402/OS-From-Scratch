#include "kernel.h"
#include "utils.h"
#include "sbi.h"

extern uint8_t __bss[], __bss_end[], __stack_top[];

extern uint8_t __heap_start[], __heap_end[];


/*
 *
 * Memeory Allocation
 *
 */

paddr_t alloc_pages(uint32_t n) {
  static paddr_t next_paddr = (paddr_t) __heap_start;
  paddr_t paddr = next_paddr;
  next_paddr += n * PAGE_SIZE;

  if(next_paddr > (paddr_t) __heap_end) {
    PANIC("Out of heap");
  }

  memset((void *) paddr, 0, n * PAGE_SIZE);
  return paddr;
}


/*
 *
 * Page Table
 *
 */

void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
  if(!is_aligned(vaddr, PAGE_SIZE))    
    PANIC("unaligned vaadr %x", vaddr);
  if(!is_aligned(paddr, PAGE_SIZE))
    PANIC("unaligned paddr %x", paddr);

  uint32_t vpn1 = (vaddr >> 22) & 0x3ff; // 10 bits
  uint32_t vpn0 = (vaddr >> 12) & 0x3ff; // 10 bits
  
  if((table1[vpn1] & PAGE_V) == 0) {
    // need to allocate table0 page 
    
    uint32_t pt_paddr = alloc_pages(1); // page table address
    
    // (pg_addr / PAGE_SIZE) is Physical Page Number (PPN) for second level page(table0)
    table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_V; // Sv32 format
  }
  
  uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);
  table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V; // map to physical addr
}


/*
 *
 * Process
 *
 */

struct process procs[PROCS_MAX]; 
extern char __kernel_base[];

__attribute__((naked))
void context_switch(uint32_t *prev_sp, uint32_t *next_sp) {
  __asm__ __volatile__(
    // Save callee-saved registers 
    "addi sp, sp, 4 * -13\n"
    "sw ra, 4 * 0(sp)\n"
    "sw s0, 4 * 1(sp)\n"
    "sw s1, 4 * 2(sp)\n"
    "sw s2, 4 * 3(sp)\n"
    "sw s3, 4 * 4(sp)\n"
    "sw s4, 4 * 5(sp)\n"
    "sw s5, 4 * 6(sp)\n"
    "sw s6, 4 * 7(sp)\n"
    "sw s7, 4 * 8(sp)\n"
    "sw s8, 4 * 9(sp)\n"
    "sw s9, 4 * 10(sp)\n"
    "sw s10, 4 * 11(sp)\n"
    "sw s11, 4 * 12(sp)\n"

    // switch stack pointer
    "sw sp, (a0)\n"
    "lw sp, (a1)\n"
    
    // Restore callee-saved registers
    "lw ra, 4 * 0(sp)\n"
    "lw s0, 4 * 1(sp)\n"
    "lw s1, 4 * 2(sp)\n"
    "lw s2, 4 * 3(sp)\n"
    "lw s3, 4 * 4(sp)\n"
    "lw s4, 4 * 5(sp)\n"
    "lw s5, 4 * 6(sp)\n"
    "lw s6, 4 * 7(sp)\n"
    "lw s7, 4 * 8(sp)\n"
    "lw s8, 4 * 9(sp)\n"
    "lw s9, 4 * 10(sp)\n"
    "lw s10, 4 * 11(sp)\n"
    "lw s11, 4 * 12(sp)\n"
    "addi sp, sp, 4 * 13\n"
    "ret\n"
  );
}

struct process *create_process(uint32_t pc) {
  // Find an unused process controll block.
  
  struct process *proc = NULL;
  int pid = 0;
  for(int i = 0; i < PROCS_MAX; ++i) {
    if(procs[i].state == PROC_UNUSED) {
      pid = i;
      proc = &procs[i];
      break;
    }
  }

  if(proc == NULL)
    PANIC("no free process slots");

  uint32_t *sp = (uint32_t *) &proc->stack[sizeof(proc->stack)];
  *--sp = 0; // s11
  *--sp = 0; // s10
  *--sp = 0; // s9
  *--sp = 0; // s8
  *--sp = 0; // s7
  *--sp = 0; // s6
  *--sp = 0; // s5
  *--sp = 0; // s4
  *--sp = 0; // s3
  *--sp = 0; // s2
  *--sp = 0; // s1
  *--sp = 0; // s0
  *--sp = pc; // ra
  proc->sp = (uint32_t) sp;

  // Map kernel pages
  uint32_t *page_table = (uint32_t *) alloc_pages(1);
  for (paddr_t paddr = (paddr_t) __kernel_base;
      paddr < (paddr_t) __heap_end; paddr += PAGE_SIZE)
    map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);
  

  proc->state = PROC_RUNNABLE;
  proc->pid = pid + 1;
  proc->sp = (uint32_t) sp;
  proc->page_table = page_table;
  return proc;
}

struct process *current_proc;
struct process *idle_proc;

void yield(void) {

  if(current_proc == NULL)
    return;

  // Search for a runnable process
  struct process *next = idle_proc;
  for(int i = 0; i < PROCS_MAX; i++) {
    int next_id = (current_proc->pid + i) % PROCS_MAX;
    struct process *proc = &procs[next_id];
    if(proc->state == PROC_RUNNABLE && proc->pid > 0) {
      next = proc;
      break;
    }
  }
  
  if(next == current_proc)
    return;

  __asm__ __volatile__(
    "sfence.vma\n" // Clear the TLB (Supervisor Fence Virtual Memory Address)
    "csrw satp, %[satp]\n"
    "sfence.vma\n"
    "csrw sscratch, %[sscratch]\n"
    :
    : [satp] "r" (SATP_SV32 | ((uint32_t) next->page_table / PAGE_SIZE)),
      [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
  );

  struct process *pre = current_proc;
  current_proc = next;
  context_switch(&pre->sp, &next->sp);
}





/*
 *
 * Kernel Trap (Exception Handler)
 *
 */

void trap_handler(struct trap_frame *frame) {
  uint32_t scause = READ_CSR(scause);
  uint32_t stval = READ_CSR(stval);
  uint32_t sepc = READ_CSR(sepc);
  
  PANIC("unexpected trap scause=%x, stval=%x, sepc=%x, a0=%d\n",
        scause, stval, sepc, frame->a0);
}

__attribute__((naked))
__attribute__((aligned(4)))
void trap_entry(void) {
  __asm__ __volatile__(
    // Retrieve the kernel stack of the running process from sscratch
    "csrrw sp, sscratch, sp\n"

    "addi sp, sp, -4 * 31\n"
    "sw ra, 4 * 0(sp)\n"
    "sw gp, 4 * 1(sp)\n"
    "sw tp, 4 * 2(sp)\n"
    "sw t0, 4 * 3(sp)\n"
    "sw t1, 4 * 4(sp)\n"
    "sw t2, 4 * 5(sp)\n"
    "sw t3, 4 * 6(sp)\n"
    "sw t4, 4 * 7(sp)\n"
    "sw t5, 4 * 8(sp)\n"
    "sw t6, 4 * 9(sp)\n"
    "sw a0, 4 * 10(sp)\n"
    "sw a1, 4 * 11(sp)\n"
    "sw a2, 4 * 12(sp)\n"
    "sw a3, 4 * 13(sp)\n"
    "sw a4, 4 * 14(sp)\n"
    "sw a5, 4 * 15(sp)\n"
    "sw a6, 4 * 16(sp)\n"
    "sw a7, 4 * 17(sp)\n"
    "sw s0, 4 * 18(sp)\n"
    "sw s1, 4 * 19(sp)\n"
    "sw s2, 4 * 20(sp)\n"
    "sw s3, 4 * 21(sp)\n"
    "sw s4, 4 * 22(sp)\n"
    "sw s5, 4 * 23(sp)\n"
    "sw s6, 4 * 24(sp)\n"
    "sw s7, 4 * 25(sp)\n"
    "sw s8, 4 * 26(sp)\n"
    "sw s9, 4 * 27(sp)\n"
    "sw s10, 4 * 28(sp)\n"
    "sw s11, 4 * 29(sp)\n"

    // Retrieve and save the sp at the time of exception.
    "csrr a0, sscratch\n" 
    "sw a0, 4 * 30(sp)\n" // store original sp address
    
    // Reset the kernel stack.
    "addi a0, sp, 4 * 31\n"
    "csrw sscratch, a0\n"

    "mv a0, sp\n"
    "call trap_handler\n"

    "lw ra, 4 * 0(sp)\n"
    "lw gp, 4 * 1(sp)\n"
    "lw tp, 4 * 2(sp)\n"
    "lw t0, 4 * 3(sp)\n"
    "lw t1, 4 * 4(sp)\n"
    "lw t2, 4 * 5(sp)\n"
    "lw t3, 4 * 6(sp)\n"
    "lw t4, 4 * 7(sp)\n"
    "lw t5, 4 * 8(sp)\n"
    "lw t6, 4 * 9(sp)\n"
    "lw a0, 4 * 10(sp)\n"
    "lw a1, 4 * 11(sp)\n"
    "lw a2, 4 * 12(sp)\n"
    "lw a3, 4 * 13(sp)\n"
    "lw a4, 4 * 14(sp)\n"
    "lw a5, 4 * 15(sp)\n"
    "lw a6, 4 * 16(sp)\n"
    "lw a7, 4 * 17(sp)\n"
    "lw s0, 4 * 18(sp)\n"
    "lw s1, 4 * 19(sp)\n"
    "lw s2, 4 * 20(sp)\n"
    "lw s3, 4 * 21(sp)\n"
    "lw s4, 4 * 22(sp)\n"
    "lw s5, 4 * 23(sp)\n"
    "lw s6, 4 * 24(sp)\n"
    "lw s7, 4 * 25(sp)\n"
    "lw s8, 4 * 26(sp)\n"
    "lw s9, 4 * 27(sp)\n"
    "lw s10, 4 * 28(sp)\n"
    "lw s11, 4 * 29(sp)\n"
    "lw sp, 4 * 30(sp)\n" // move sp to original address
  );
}


/*
 *
 * Kernel Main
 *
 */

void delay(void) {
  for(int i = 0; i < 1000000000; ++i) {
    __asm__ __volatile__("nop");
  }
}

struct process *proc_a, *proc_b;

void proc_a_main(void) {
  int t = 5;
  printf("start proc_a\n");
  while(t--) {
    printf("proc_a\n");
    yield();
    delay();
  }
}

void proc_b_main(void) {
  int t = 5;
  printf("start proc_b\n");
  while(t--) {
    printf("proc_b\n");
    yield();
    delay();
  }
}

void kernel_main(void) {
  // Clear the .bss section (Block Started by Symbol)
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  // Set Exception Handler (Supervisor Trap Vector)
  WRITE_CSR(stvec, (uint32_t) trap_entry);

  // Process Initalization
  idle_proc = create_process((uint32_t) NULL); 
  idle_proc->pid = -1;
  current_proc = idle_proc;
  

  // Example printf usage
  printf("\n\nHello %s \n", "World!");
  printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);
  printf("MAX_INT = %d, MIN_INT = %d\n", 2147483647, -2147483648);

  // Test Allocate Pages
  paddr_t paddr0 = alloc_pages(2);
  paddr_t paddr1 = alloc_pages(1);

  printf("alloc_pages test: paddr0=%x\n", paddr0);
  printf("alloc_pages test: paddr1=%x\n", paddr1);

  // Test Process Switching
  proc_a = create_process((uint32_t) proc_a_main);
  proc_b = create_process((uint32_t) proc_b_main);
  

  yield();
  PANIC("switched back to idle process");

  // Test Exception Handler  
  __asm__ __volatile__("unimp"); // illegal instruction


  for (;;) {
    __asm__ __volatile__("wfi"); // Wait For Interrupt
  }
}

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
