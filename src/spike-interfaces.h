#ifndef __SPIKE_INTERFCES_H__
#define __SPIKE_INTERFCES_H__

#include "cfg.h"
#include "decode_macros.h"
#include "disasm.h"
#include "mmu.h"
#include "processor.h"
#include "simif.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char* (*ffi_callback)(uint64_t);
ffi_callback ffi_addr_to_mem;

class sim_t : public simif_t {
 public:
  sim_t() {}
  ~sim_t() {}
  char* addr_to_mem(reg_t addr) override { return ffi_addr_to_mem(addr); }
  bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) override {}
  bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) override {}
  virtual void proc_reset(unsigned id) override {}
  virtual const char* get_symbol(uint64_t addr) override {}
  [[nodiscard]] const cfg_t& get_cfg() const override {}
  [[nodiscard]] const std::map<size_t, processor_t*>& get_harts()
      const override {}
};

class Spike {
 public:
  Spike(const char* arch, const char* set, const char* lvl);
  processor_t* get_proc() { return &proc; }

 private:
  std::string varch;
  cfg_t cfg;
  sim_t sim;
  isa_parser_t isa;
  processor_t proc;
};

struct spike_t {
  Spike* s;
};
struct spike_processor_t {
  processor_t* p;
};
struct spike_state_t {
  state_t* s;
};
struct spike_mmu_t {
  mmu_t* m;
};

void spike_register_callback(ffi_callback callback);
spike_t* spike_new(const char* arch, const char* set, const char* lvl);
const char* proc_disassemble(spike_processor_t* proc,
                             spike_mmu_t* mmu,
                             reg_t pc);
void proc_reset(spike_processor_t* proc);
spike_processor_t* spike_get_proc(spike_t* spike);
spike_state_t* proc_get_state(spike_processor_t* proc);
spike_mmu_t* proc_get_mmu(spike_processor_t* proc);
reg_t mmu_func(spike_mmu_t* mmu, spike_processor_t* proc, reg_t pc);
reg_t state_get_pc(spike_state_t* state);
void state_set_pc(spike_state_t* state, reg_t pc);
void state_set_serialized(spike_state_t* state, bool serialized);
void destruct(void* ptr);
reg_t spike_exit(spike_state_t* state);

#ifdef __cplusplus
}
#endif

#endif  // __SPIKE_INTERFCES_H__
