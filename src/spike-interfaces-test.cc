#include <elf.h>
#include <sys/mman.h>

#include <spike-interfaces.h>

using Spike = uint64_t;

void load_segment(int fd, Elf32_Phdr *phdr, Spike spike) {
  void *mem = mmap((void*)phdr->p_vaddr, phdr->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  lseek(fd, phdr->p_offset, SEEK_SET);
  read(fd, mem, phdr->p_filesz);


  if (phdr->p_filesz < phdr->p_memsz) {
    memset(mem + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
  }

  std::cerr << "Loaded segment " << phdr->p_vaddr << " - " << phdr->p_vaddr + phdr->p_memsz << std::endl;

  spike_ld_elf(spike, phdr->p_vaddr, phdr->p_memsz, (uint8_t*)mem);
}

void load_elf(const char *filename) {
  std::cerr << "Loaded " << filename << " into spike" << std::endl;

  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  Elf32_Ehdr ehdr;
  read(fd, &ehdr, sizeof(ehdr));

  Spike spike = spike_new(1 << 32);

  for (int i = 0; i < ehdr.e_phnum; i++) {
    Elf32_Phdr phdr;
    lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
    read(fd, &phdr, sizeof(phdr));

    if (phdr.p_type == PT_LOAD) {
      load_segment(fd, &phdr, spike);
    }

    std::cerr << "Loaded segment " << i << std::endl;
  }

  spike_delete(spike);
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
    exit(1);
  }

  load_elf(argv[1]);

  return 0;
}