#pragma once

#include "entry.hpp"

extern "C" void kprintf(const char* fmt, ...);

#define critical_kprintf(fmt, ...) \
  do { \
    enter_critical(); \
    kprintf(fmt __VA_OPT__(,) __VA_ARGS__); \
    leave_critical(); \
  } while (0)

void fd_printf_internal(int fd, const char *fmt, uint32_t *args);