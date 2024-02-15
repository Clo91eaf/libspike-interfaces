#include <elf.h>
#include <sys/mman.h>

#include "spike-interfaces.h"

using Spike = uint64_t;
using Entry_addr = uint32_t;

// Load ELF file
Entry_addr load_elf(Spike spike, const char* fname) {
  // Open file
  std::ifstream fs(fname, std::ios::binary);
  fs.exceptions(std::ios::failbit);

  Elf32_Ehdr ehdr;
  fs.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

  // Load program headers
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
    auto phdr_offset = ehdr.e_phoff + i * ehdr.e_phentsize;
    Elf32_Phdr phdr;
    fs.seekg((long)phdr_offset).read(reinterpret_cast<char*>(&phdr), sizeof(phdr));
    if (phdr.p_type == PT_LOAD) {
      uint8_t* buffer = new uint8_t[phdr.p_filesz];

      fs.seekg((long)phdr.p_offset).read(reinterpret_cast<char*>(buffer), phdr.p_filesz);
      auto res = spike_ld_elf(spike, phdr.p_vaddr, phdr.p_filesz, buffer);

      delete buffer;
      assert(res == 0);
    }
  }

  return  ehdr.e_entry;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <elf-file>" << std::endl;
    exit(1);
  }

  Spike spike = spike_new((uint64_t)1 << 32);
  std::cerr << "Creat spike: " << std::hex << spike << std::endl;

  Entry_addr addr = load_elf(spike, argv[1]);

  spike_init(spike, addr);

  for (int i = 0; i < 50; i++) {
    spike_execute(spike);
  }

  spike_delete(spike);

  return 0;
}