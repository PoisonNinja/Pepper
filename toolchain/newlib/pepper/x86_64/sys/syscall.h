#pragma once

#include <stdint.h>
#include <sys/syscall_numbers.h>

extern uint64_t syscall(uint64_t num, uint64_t a, uint64_t b, uint64_t c,
                        uint64_t d, uint64_t e);
