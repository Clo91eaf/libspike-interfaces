#include <elf.h>
#include <sys/mman.h>

#include "spike_interfaces_c.h"

char mem[1l << 32];

// Load ELF file
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Load ELF file
uint32_t load_elf(const char* fname) {
  // Open file
  FILE* fs = fopen(fname, "rb");
  if (fs == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  Elf32_Ehdr ehdr;
  size_t read_size = fread(&ehdr, sizeof(ehdr), 1, fs);
  if (read_size != 1) {
    perror("Error reading ELF header");
    exit(EXIT_FAILURE);
  }

  // Load program headers
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
    long phdr_offset = ehdr.e_phoff + i * ehdr.e_phentsize;
    Elf32_Phdr phdr;
    fseek(fs, phdr_offset, SEEK_SET);
    read_size = fread(&phdr, sizeof(phdr), 1, fs);
    if (read_size != 1) {
      perror("Error reading program header");
      exit(EXIT_FAILURE);
    }
    if (phdr.p_type == PT_LOAD) {
      fseek(fs, phdr.p_offset, SEEK_SET);
      read_size = fread(&mem[phdr.p_paddr], phdr.p_filesz, 1, fs);
    }
  }

  fclose(fs);
  return ehdr.e_entry;
}

char* addr_to_mem(uint64_t addr) {
  return &mem[addr];
}

uint32_t execute(spike_t* spike) {
  spike_processor_t* proc = spike_get_proc(spike);
  spike_state_t* state = proc_get_state(proc);
  spike_mmu_t* mmu = proc_get_mmu(proc);

  uint64_t pc = state_get_pc(state);
  const char* disasm = proc_disassemble(proc, mmu, pc);

  fprintf(stderr, "PC: %08x, disasm: %s\n", pc, disasm);

  uint64_t new_pc = mmu_func(mmu, proc, pc);

  if (spike_exit(state) == 0) {
    return -1;
  }

  // Bypass CSR insns commitlog stuff.
  int ret = handle_pc(state, new_pc);
  if (ret != 0) {
    perror("Invalid PC");
    exit(EXIT_FAILURE);
  }

  free((void*)disasm);
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

  if (exit_success != 0) {
    perror("Execute failed");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Execute success\n");

  return EXIT_SUCCESS;
}