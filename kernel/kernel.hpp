#pragma once

#include <minstd/types.hpp>
#include "entry.hpp"

#define STACKS_START 0x20000
#define STACK_SIZE 0x2000
#define STACKS_END 0x60000

#define PROCESS_LIMIT 8
#define THREAD_LIMIT 12

struct Process;
struct Thread;
struct AddressSpace;

enum TaskState {
  READY,
  RUNNING,
  BLOCKED,
  EXITED
};

struct Stack {
  uintptr_t bottom;
  uintptr_t sp;
};

struct StackAllocator {
  static constexpr uint32_t NUM_SLOTS = (STACKS_END - STACKS_START) / STACK_SIZE;
  uint16_t used_mask = 0;   // bit i set = slot i currently in use

  Stack allocate() {
    for (uint32_t i = 0; i < NUM_SLOTS; i++) {
      if (!(used_mask & (1 << i))) {
        used_mask |= (1 << i);
        Stack stack;
        stack.bottom = STACKS_START + i * STACK_SIZE;
        stack.sp = stack.bottom + STACK_SIZE;
        return stack;
      }
    }
    return {0, 0};    // out of stacks
  }

  void free(Stack stack) {
    if (stack.bottom < STACKS_START || stack.bottom >= STACKS_END) return;  // never allocated by us
    uint32_t i = (stack.bottom - STACKS_START) / STACK_SIZE;
    used_mask &= ~(1 << i);
  }
};
typedef struct Thread {
  uint32_t tid;
  TaskState state;
  Stack kernel_stack;
  Stack user_stack;
  Thread *next;
  Thread *prev;
} tcb_t;

struct AddressSpace {
  uintptr_t begin;
  uintptr_t end;
};

typedef struct Process {
  uint32_t pid;
  TaskState state;
  AddressSpace* address_space;
  uint32_t tid; // temporarily used to index table
} pcb_t;

struct IdAllocator {
  uint32_t next;
  uint32_t limit;
  
  uint32_t allocate_id() { // Temporary approach. This is bound to run out of PIDs eventually
    next++;
    if (next < 1) return -1; // if it wrapped around
    if (next > limit) return -1;
    return next;
  }
};

typedef int (*syscall_t) (...);
extern syscall_t syscalls[256];
extern tcb_t *current_running;
extern tcb_t *zombie;   // thread that exited last cycle: its stacks are reaped on the next switch
extern IdAllocator tid_allocator;
extern StackAllocator stack_allocator;
extern Thread thread_pool[THREAD_LIMIT];

struct SwitchFrame {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp_unused, ebx, edx, ecx, eax;
  uint32_t return_eip;
};

void halt();
