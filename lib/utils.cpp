#include <minstd/types.hpp>
#include "utils.hpp"
#include <syslib.hpp>

void uprintf(const char *in, ...) {
  uint32_t *args = (uint32_t *)(&in + 1);
  call_write(3, in, args);
}