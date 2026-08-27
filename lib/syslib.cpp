#include <minstd/types.hpp>
#include <common.hpp>
#include <syslib.hpp>

#define UNUSED 0

static inline int invoke_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) {
  int ret;

  asm volatile (
      "int $0x80"
      : "=a"(ret)
      : "a"(syscall_id),
        "b"(arg1),
        "c"(arg2),
        "d"(arg3),
        "S"(arg4),
        "D"(arg5)
      : "memory"
  );

  return ret;
}

void call_yield(void) {
  invoke_syscall(SYS_YIELD, UNUSED, UNUSED, UNUSED, UNUSED, UNUSED);
}

void call_exit(void) {
  invoke_syscall(SYS_EXIT, UNUSED, UNUSED, UNUSED, UNUSED, UNUSED);
}

void call_write(int fd, const char *fmt, uint32_t *args) {
  invoke_syscall(SYS_WRITE, fd, (uint32_t)fmt, (uint32_t)args, UNUSED, UNUSED); // could go out of bounds and explode without a terminator, but eh
}