#include "spike-interfaces.h"

Spike::Spike(const char* arch, const char* set, const char* lvl)
    : sim(),
      varch(arch),
      isa(set, lvl),
      cfg(/*default_initrd_bounds=*/std::make_pair((reg_t)0, (reg_t)0),
          /*default_bootargs=*/nullptr,
          /*default_isa=*/DEFAULT_ISA,
          /*default_priv=*/DEFAULT_PRIV,
          /*default_varch=*/varch.data(),
          /*default_misaligned=*/false,
          /*default_endianness*/ endianness_little,
          /*default_pmpregions=*/16,
          /*default_mem_layout=*/std::vector<mem_cfg_t>(),
          /*default_hartids=*/std::vector<size_t>(),
          /*default_real_time_clint=*/false,
          /*default_trigger_count=*/4),
      proc(
          /*isa*/ &isa,
          /*cfg*/ &cfg,
          /*sim*/ &sim,
          /*id*/ 0,
          /*halt on reset*/ true,
          /*log_file_t*/ nullptr,
          /*sout*/ std::cerr) {
  auto& csrmap = proc.get_state()->csrmap;
  constexpr uint32_t CSR_MSIMEND = 0x7cc;
  csrmap[CSR_MSIMEND] = std::make_shared<basic_csr_t>(&proc, CSR_MSIMEND, 0);
  proc.enable_log_commits();
}

spike_t* spike_new(const char* arch, const char* set, const char* lvl) {
  return new spike_t{new Spike(arch, set, lvl)};
}

const char* proc_disassemble(spike_processor_t* proc,
                             spike_insn_fetch_t* fetch) {
  auto d = proc->p->get_disassembler();
  return strdup(d->disassemble(fetch->f.insn).c_str());
}

spike_processor_t* spike_get_proc(spike_t* spike) {
  return new spike_processor_t{spike->s->get_proc()};
}

void proc_reset(spike_processor_t* proc) {
  proc->p->reset();
}

spike_state_t* proc_get_state(spike_processor_t* proc) {
  return new spike_state_t{proc->p->get_state()};
}

spike_mmu_t* proc_get_mmu(spike_processor_t* proc) {
  return new spike_mmu_t{proc->p->get_mmu()};
}

spike_insn_fetch_t* mmu_load_insn(spike_mmu_t* mmu, reg_t addr) {
  return new spike_insn_fetch_t{mmu->m->load_insn(addr)};
}

reg_t insn_fetch_func(spike_insn_fetch_t* fetch,
                      spike_processor_t* proc,
                      reg_t pc) {
  return fetch->f.func(proc->p, fetch->f.insn, pc);
}

reg_t state_get_pc(spike_state_t* state) {
  return state->s->pc;
}

void state_set_pc(spike_state_t* state, reg_t pc) {
  state->s->pc = pc;
}

void state_set_serialized(spike_state_t* state, bool serialized) {
  state->s->serialized = serialized;
}

void destruct(void* ptr) {
  if (ptr == nullptr) return;
  delete ptr;
}