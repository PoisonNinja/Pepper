#pragma once

#include <stdint.h>
#include <sys/syscall_numbers.h>

extern uint32_t syscall(uint32_t num, uint32_t a, uint32_t b, uint32_t c,
                        uint32_t d, uint32_t e);
