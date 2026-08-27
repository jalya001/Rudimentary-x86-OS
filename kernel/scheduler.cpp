#include "kernel.hpp"
#include "gdt.hpp"
#include "scheduler.hpp"
#include "entry.hpp"
#include "print.hpp"

void r0_yield() {
  direct_scheduler_entry();
}

uint32_t scheduler(uint32_t old_esp) {
  static tcb_t *zombie = 0;
  tcb_t *to_remove;
  
  current_running->kernel_stack.sp = old_esp;
  
  /* Reap whichever thread exited during the *previous* switch. By now
   * we're running on a different stack than it was, so its memory is
   * safe to hand back to the allocator. (Freeing it any earlier, e.g.
   * right when it exits, would mean freeing the very stack we're still
   * executing on). 
   */
  if (zombie) {
    stack_allocator.free(zombie->kernel_stack);
    if (zombie->user_stack.bottom) stack_allocator.free(zombie->user_stack);
    zombie = 0;
  }

  switch (current_running->state) {
    case READY:
      current_running = current_running->next;
      break;
    case BLOCKED: // removes from ready queue because it's in a blocked queue now
    case EXITED:
      if (current_running->next == current_running) halt();
      to_remove = current_running;
      current_running = current_running->next;
      to_remove->prev->next = to_remove->next;
      to_remove->next->prev = to_remove->prev;
      to_remove->next = nullptr;
      to_remove->prev = nullptr;
      break;
    case RUNNING:
    default:
      break;
  }
  
  current_running->state = READY;
  tss.esp0 = current_running->kernel_stack.bottom + STACK_SIZE;
  return current_running->kernel_stack.sp;
}

void r0_exit() {
  current_running->state = EXITED;
  direct_scheduler_entry();
}

void r3_exit() { // For when interrupts already get disabled
  current_running->state = EXITED;
  scheduler_entry();
}

void block(tcb_t **q) {
  current_running->state = BLOCKED;

  if (*q == nullptr) {
    *q = current_running;
  } else {
    tcb_t *t = *q;
    while (t->next != nullptr) t = t->next;
    t->next = current_running;
  }

  scheduler_entry();
}

void unblock(tcb_t **q) {
  if (*q == nullptr) kprintf("ERROR: unblocking a nullpointer\n");

  tcb_t *t = *q;
  *q = (*q)->next;

  t->state = READY;
  t->next = current_running->next;
  t->prev = current_running;
  current_running->next->prev = t;
  current_running->next = t;
}
