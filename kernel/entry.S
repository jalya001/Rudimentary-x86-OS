.section .rodata
si_msg:
  .asciz "Syscall"
s2_msg:
  .asciz "FINISH SYSCALL"
tim_msg:
  .asciz "Timer Interrupt"

.section .text

.extern fault_print

.globl enter_critical
.globl leave_critical
.globl scheduler_entry
.globl direct_scheduler_entry
.globl syscall_entry
.globl timer_handler
.globl kthread_trampoline
.globl uthread_trampoline

.extern syscall_handler
.extern scheduler
.extern r0_exit

enter_critical:
  cli
  ret

leave_critical:
  sti
  ret

timer_handler:
  pusha

  # push $0
  # push $tim_msg
  # call fault_print
  # add $8, %esp

  mov $0x20, %al
  out %al, $0x20

  call scheduler_entry

  popa
  iret

scheduler_entry:
  pusha
  push %ds
  push %es
  push %fs
  push %gs

  push %esp # should change these not to need passing through the stack at some point
  call scheduler
  add $4, %esp
  mov %eax, %esp

  pop %gs
  pop %fs
  pop %es
  pop %ds
  popa
  ret


syscall_entry:
  pushal

  # args
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx
  pushl %eax

  pushl %eax
  pushl $si_msg
  call fault_print
  addl $8, %esp

  call syscall_handler
  addl $24, %esp  # 6 × 4 bytes

  popal
  iret

direct_scheduler_entry:
  # r0 iret frame
  pushfl
  push %cs
  pushl 8(%esp) # eip was already pushed when calling this

  cli
  call scheduler_entry

  iret

# Reached via `ret` from scheduler_entry the first time a kernel thread runs.
# ebx = entry point (placed there by init_kernel_thread_stack).
kthread_trampoline:
  # r0 iret frame
  push $0x202        # EFLAGS (IF set)
  push $0x08         # KERNEL_CS
  push %ebx          # entry point (eip)
  
  iret

# Reached via `ret` from scheduler_entry the first time a user thread runs.
# ebx = entry point, esi = user stack (placed there by init_user_thread_stack).
uthread_trampoline:
  # r3 iret frame
  push $0x23         # USER_SS
  push %esi          # user esp
  push $0x202        # EFLAGS (IF set)
  push $0x1B         # USER_CS
  push %ebx          # entry point (eip)

  mov $0x23, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  
  iret