#include "drivers/serial.hpp"
#include "drivers/vga.hpp"
#include "drivers/ata.hpp"
#include "kernel.hpp"
#include "gdt.hpp"
#include "entry.hpp"
#include "scheduler.hpp"
#include "interrupts.hpp"
#include <common.hpp>
#include "test.hpp"
#include "pic.hpp"
#include <sleep.hpp>
#include <syslib.hpp>
#include "print.hpp"

extern "C" void kernel_main();

void halt() {
  while (1) asm volatile("hlt");
}

tcb_t *current_running = 0;
syscall_t syscalls[256];

StackAllocator stack_allocator;
IdAllocator pid_allocator = { 1, PROCESS_LIMIT }; // temporarily, we are using id for indexing tables 
IdAllocator tid_allocator = { 1, THREAD_LIMIT };
Process processes[PROCESS_LIMIT];
Thread thread_pool[THREAD_LIMIT];     // static pool: no heap/allocator exists yet

extern "C" void fault_print(const char* name, uint32_t error) {
  kprintf("%s=%d (not necessarily an error code)\n", name, error);
}

void init_syscalls() {
  syscalls[SYS_YIELD] = (syscall_t)scheduler_entry;
  syscalls[SYS_EXIT] = (syscall_t)r3_exit;
  syscalls[SYS_WRITE] = (syscall_t)fd_printf_internal;
}

extern "C" void syscall_handler(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3) { // could have used the trapframe instead but lazy
  syscalls[syscall_id](arg1, arg2, arg3);
}

/*
 * Creates a new thread and splices it into the circulat ready queue.
 * `user` picks whether it starts in ring 0 or ring 3 on first dispatch.
 * NOTE: only supports up to 8 threads right now, see stack layout comment
 * in scheduler.cpp. 
 */
template <bool User> // using a bool is rather opaque
tcb_t *thread_create(void (*entry_fn)()) {
  uint32_t tid = tid_allocator.allocate_id();
  if (tid == (uint32_t) - 1) return 0;

  tcb_t *t = &thread_pool[tid - 1];
  *t = {};
  t->tid = tid;
  t->state = READY;

  t->kernel_stack = stack_allocator.allocate();
  if (t->kernel_stack.sp == 0) return 0;  // out of stacks

  if constexpr (User) {
    t->user_stack = stack_allocator.allocate();
    if (t-> user_stack.sp == 0) {
      stack_allocator.free(t->kernel_stack);  // give back what er already took
      return 0;
    }
  }

 /* --- thread bootstrap ---
  *
  * A thread that's never run yet has no saved registers for scheduler_entry
  * to restore. So we pre-build its kernel stack to look like it just called
  * scheduler_entry and is about to `ret`, except the "return address"
  * points at a trampoline instead of a real saved code. Field order here
  * mirrors what scheduler_entry itself pushes/pops.
  */
  uint8_t *top = (uint8_t *)t->kernel_stack.sp;

  if constexpr (User) {
    t->user_stack.sp -= sizeof(uint32_t);
    *(uint32_t *)t->user_stack.sp = (uint32_t)call_exit;
  } else {
    top -= sizeof(uint32_t);
    *(uint32_t *)top = (uint32_t)r0_exit;
  }

  top -= sizeof(SwitchFrame);
  auto *frame = (SwitchFrame *)top;
  t->kernel_stack.sp = (uintptr_t)frame;
  *frame = {};
  frame->gs = 0x10;
  frame->fs = 0x10;
  frame->es = 0x10;
  frame->ds = 0x10;
  frame->ebx = (uintptr_t)entry_fn; // both trampolines read this

  if constexpr (User) {
    frame->return_eip = (uint32_t)(uintptr_t)uthread_trampoline;
    frame->esi = (uint32_t)t->user_stack.sp; // uthread_trampoline reads this 
  } else {
    frame->return_eip = (uint32_t)(uintptr_t)kthread_trampoline;
  }

  if (current_running) {
    t->next = current_running;
    t->prev = current_running->prev;
    current_running->prev->next = t;
    current_running->prev = t;
  } else { // this shouldn't happen anyway
    t->next = t;
    t->prev = t;
    current_running = t;
  }

  return t;
}

void kthread1() {
  while (1) {
    sleep(1000);
    serial_print("hello\n");
    vga_write("hello\n");
  }
}

void kernel_main() {
  int rc = 0;
  
  init_serial();
  init_vga();
  init_gdt();
  pic_remap();
  init_interrupts();
  init_syscalls();
  
  rc = ata_init();
  if (rc < 0) kprintf("DISK INIT ERROR\n");

  serial_print("Hello World.\n");
  vga_write("Hello World\n");
/*
  uint8_t sector[512];
  rc = ata_read_sector(0, sector);
  if (rc < 0) fd_write(1, "disk read error\n");
  kprintf("Test disk read: 0x%x%x = 0x55AA\n", sector[510], sector[511]);

  for (int i = 0; i < 512; i++) sector[i] = ((unsigned char*)"\xDE\xAD\xBE\xEF")[i % 4];

  int TEST_LBA = 31; // this is not a safe spot, change it later
  rc = ata_write_sector(TEST_LBA, sector);
  if (rc < 0) fd_write(1, "disk write error\n");
  rc = ata_read_sector(TEST_LBA, sector);
  if (rc < 0) fd_write(1, "disk read error\n");
  kprintf("Test disk write: 0x%x%x = 0xBEEF\n", sector[222], sector[223]);
*/
  /* A placeholder TCB representing kernel_main's own execution context, so
   * switching away from it doesn't collide with the first real thread.
   */
  static Thread boot_thread = {};
  boot_thread.next = &boot_thread;
  boot_thread.prev = &boot_thread;
  current_running = &boot_thread;

  thread_create<false>(kthread2);
  thread_create<true>(test_writes);
  /*
  thread_create<false>(test_mbox_1_basic_send_recv);
  thread_create<false>(test_mbox_2_blocking_recv);
  thread_create<false>(test_mbox_2_blocking_sender);
  thread_create<false>(test_mbox_3_multiple_messages);
  thread_create<false>(test_mbox_4_producer1);
  thread_create<false>(test_mbox_4_producer2);
  thread_create<false>(test_mbox_4_consumer);
  thread_create<false>(test_mbox_5_wraparound);
  thread_create<false>(test_mbox_6_producer);
  thread_create<false>(test_mbox_6_consumer);
  
  thread_create<false>(lock_test_1);
  thread_create<false>(lock_test_2);
  thread_create<false>(condition_test_waiter);
  thread_create<false>(condition_test_signaler);
  thread_create<false>(semaphore_test_1);
  thread_create<false>(semaphore_test_2);
  thread_create<false>(barrier_test_1);
  thread_create<false>(barrier_test_2);
  thread_create<false>(barrier_test_3);*/
/*
  thread_create<true>(test_writes_2);
  thread_create<false>(kthread1);
  thread_create<true>(test_writes_3);
*/
  //thread_create<true>(stress_a);
  //thread_create<true>(stress_b);
  
  //fd_write(1, "Stress test starting - let this run for at least 1-2 minutes\n");

  r0_exit();    // hand off to the first real thread

  /*
  thread_create<true>(test_exiter);

  yield(); // hands off to test_exiter — it runs to completion and exits;
           // this yield() only returns once that's fully happened, since
           // boot_thread and test_exiter are the only two threads right now
  yield(); // trivial self-switch (boot_thread alone in the list) — this is
           // what actually triggers the zombie-reaping check to run

  tcb_t *reused = thread_create<true>(test_after_exit);
  fd_write(1, reused ? "Reuse check: OK\n" : "Reuse check: FAILED\n");

  int failures = 0;
  for (int i = 0; i < 10; i++) {
    if (!thread_create<true>(noop_thread)) failures++;
  }
  fd_write(1, failures > 0 ? "Exhaustion check: OK (failed cleanly)\n" : "Exhaustion check: FAILED (should have run out of stacks)\n");
 */
  halt();
}