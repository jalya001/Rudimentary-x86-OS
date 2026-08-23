#pragma once

#include "kernel.hpp"

void r0_yield(void);
extern "C" void r0_exit(void);
extern "C" uint32_t scheduler(uint32_t old_esp);

void r3_exit(void);

void block(tcb_t **q);
void unblock(tcb_t **q);