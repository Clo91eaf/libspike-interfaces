#include <elf.h>
#include <sys/mman.h>

#include "spike-interfaces.h"

char mem[1l << 32];
ffi_callback ffi_addr_to_mem;

// Load ELF file
uint32_t load_elf(const char* fname) {
  // Open file
  std::ifstream fs(fname, std::ios::binary);
  fs.exceptions(std::ios::failbit);

  Elf32_Ehdr ehdr;
  fs.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

  // Load program headers
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
    auto phdr_offset = ehdr.e_phoff + i * ehdr.e_phentsize;
    Elf32_Phdr phdr;
    fs.seekg((long)phdr_offset)
        .read(reinterpret_cast<char*>(&phdr), sizeof(phdr));
    if (phdr.p_type == PT_LOAD) {
      fs.seekg((long)phdr.p_offset)
          .read(reinterpret_cast<char*>(&mem[phdr.p_paddr]), phdr.p_filesz);
    }
  }

  return ehdr.e_entry;
}

char* addr_to_mem(uint64_t addr) {
  return &mem[addr];
}

void spike_register_callback(ffi_callback callback) {
  ffi_addr_to_mem = callback;

  return;
}

uint32_t execute(spike_t* spike) {
  spike_processor_t* proc = spike_get_proc(spike);
  spike_state_t* state = proc_get_state(proc);
  spike_mmu_t* mmu = proc_get_mmu(proc);

  reg_t pc = state_get_pc(state);
  const char* disasm = proc_disassemble(proc, mmu, pc);

  fprintf(stderr, "PC: %08x, disasm: %s\n", pc, disasm);

  reg_t new_pc = mmu_func(mmu, proc, pc);

  if (spike_exit(state) == 0) {
    return -1;
  }

  // Bypass CSR insns commitlog stuff.
  if ((new_pc & 1) == 0) {
    state_set_pc(state, new_pc);
  } else {
    switch (new_pc) {
      case PC_SERIALIZE_BEFORE:
        state_set_serialized(state, true);
        break;
      case PC_SERIALIZE_AFTER:
        break;
      default:
        fprintf(stderr, "Unknown PC: %08x\n", new_pc);
        return -1;
    }
  }

  delete[] disasm;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    exit(1);
  }

  // Register callback function for memory access
  spike_register_callback(addr_to_mem);

  // Prepare memory
  uint32_t addr = load_elf(argv[1]);

  // Initialize spike
  const char* varch = "vlen:1024,elen:32";
  const char* isa = "rv32gcv";
  const char* priv = "M";
  spike_t* spike = spike_new(varch, isa, priv);
  spike_processor_t* proc = spike_get_proc(spike);
  spike_state_t* state = proc_get_state(proc);
  proc_reset(proc);
  state_set_pc(state, addr);

  // execute
  while (true) {
    int ret = execute(spike);
    if (ret)
      break;
  }

  int exit_success = spike_exit(state);

  // destruct
  destruct(state);
  destruct(proc);
  destruct(spike);

  if (exit_success == 0) {
    fprintf(stderr, "Success\n");
  } else {
    fprintf(stderr, "Failed\n");
  }
  return exit_success;
}