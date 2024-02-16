#ifndef __SPIKE_INTERFCES_H__
#define __SPIKE_INTERFCES_H__

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <optional>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "disasm.h"
#include "mmu.h"
#include "processor.h"
#include "simif.h"
#include "cfg.h"
#include "decode_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t spike_new(uint64_t mem_size);
void spike_delete(uint64_t spike);
int32_t spike_execute(uint64_t spike);
int32_t spike_get_reg(uint64_t spike, uint64_t index, uint64_t *content);
int32_t spike_set_reg(uint64_t spike, uint64_t index, uint64_t content);
int spike_ld(uint64_t spike, uint64_t addr, uint64_t len, uint8_t *bytes);
int spike_sd(uint64_t spike, uint64_t addr, uint64_t len, uint8_t *bytes);
int spike_ld_elf(uint64_t spike, uint64_t addr, uint64_t len, uint8_t *bytes);
int spike_init(uint64_t spike, uint64_t entry_addr);

#ifdef __cplusplus
}
#endif

#endif // __SPIKE_INTERFCES_H__
