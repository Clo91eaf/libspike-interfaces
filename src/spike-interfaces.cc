#include "spike-interfaces.h"

class mmio_dev {
 public:
  virtual bool do_read(uint64_t start_addr, uint64_t size, uint8_t* buffer) = 0;
  virtual bool do_write(uint64_t start_addr,
                        uint64_t size,
                        const uint8_t* buffer) = 0;
};

#define SR_TX_FIFO_EMPTY (1 << 2)      /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA (1 << 0) /* data in receive FIFO */

#define ULITE_CONTROL_RST_TX 0x01
#define ULITE_CONTROL_RST_RX 0x02

struct uartlite_regs {
  unsigned int rx_fifo;
  unsigned int tx_fifo;
  unsigned int status;
  unsigned int control;
};

class uartlite : public mmio_dev {
 public:
  uartlite() { wait_ack = false; }
  bool do_read(uint64_t start_addr, uint64_t size, unsigned char* buffer) {
    if (size != 4)
      return false;
    uint32_t& res_buffer = *reinterpret_cast<uint32_t*>(buffer);
    switch (start_addr) {
      case offsetof(uartlite_regs, rx_fifo): {
        std::unique_lock<std::mutex> lock(rx_lock);
        if (!rx.empty()) {
          res_buffer = rx.front();
          rx.pop();
        } else
          res_buffer = 0;
        break;
      }
      case offsetof(uartlite_regs, status): {
        std::unique_lock<std::mutex> rxlock(rx_lock);
        std::unique_lock<std::mutex> txlock(tx_lock);
        res_buffer = (tx.empty() ? SR_TX_FIFO_EMPTY : 0) |
                     (rx.empty() ? 0 : SR_RX_FIFO_VALID_DATA);
        break;
      }
      case offsetof(uartlite_regs, control):
      case offsetof(uartlite_regs, tx_fifo): {
        res_buffer = 0;
        break;
      }
      default:
        return false;
    }
    return true;
  }
  bool do_write(uint64_t start_addr,
                uint64_t size,
                const unsigned char* buffer) {
    if (size != 4)
      return false;
    switch (start_addr) {
      case offsetof(uartlite_regs, tx_fifo): {
        std::unique_lock<std::mutex> lock_tx(tx_lock);
        tx.push(static_cast<char>(*buffer));
        break;
      }
      case offsetof(uartlite_regs, control): {
        if (*buffer & ULITE_CONTROL_RST_TX) {
          std::unique_lock<std::mutex> lock_tx(tx_lock);
          while (!tx.empty())
            tx.pop();
        }
        if (*buffer & ULITE_CONTROL_RST_RX) {
          std::unique_lock<std::mutex> lock_rx(rx_lock);
          while (!rx.empty())
            rx.pop();
        }
        break;
      }
      case offsetof(uartlite_regs, rx_fifo):
      case offsetof(uartlite_regs, status): {
        break;
      }
      default:
        return false;
    }
    return true;
  }
  void putc(char c) {
    std::unique_lock<std::mutex> lock(rx_lock);
    rx.push(c);
  }
  char getc() {
    std::unique_lock<std::mutex> lock(tx_lock);
    if (!tx.empty()) {
      char res = tx.front();
      tx.pop();
      if (tx.empty())
        wait_ack = true;
      return res;
    } else
      return -1;
  }
  bool exist_tx() {
    std::unique_lock<std::mutex> lock(tx_lock);
    return !tx.empty();
  }
  bool irq() {
    std::unique_lock<std::mutex> lock(rx_lock);
    return !rx.empty() || wait_ack;
  }

 private:
  uartlite_regs regs;
  std::queue<char> rx;
  std::queue<char> tx;
  std::mutex rx_lock;
  std::mutex tx_lock;
  bool wait_ack;
};

class sim_t : public simif_t {
 public:
  sim_t(uint64_t size) {
    mem = new char[size];
    mem_size = size;
  }
  ~sim_t() { delete[] mem; }
  char* addr_to_mem(reg_t addr) override {
    if (uart_addr <= addr && addr < uart_addr + sizeof(uartlite_regs)) {
      return NULL;
    }
    return &mem[addr];
  }

  bool mmio_load(reg_t addr, size_t len, uint8_t* bytes) override {
    if (uart_addr <= addr && addr < uart_addr + sizeof(uartlite_regs)) {
      return uart.do_read(addr - uart_addr, len, bytes);
    }
    assert(0);
  }

  bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes) override {
    if (uart_addr <= addr && addr < uart_addr + sizeof(uartlite_regs)) {
      bool res = uart.do_write(addr - uart_addr, len, bytes);
      while (uart.exist_tx()) {
        std::cerr << uart.getc();
        std::cerr.flush();
      }
      return res;
    }
    assert(0);
  }

  bool load_elf(reg_t addr, size_t len, const uint8_t* bytes) {
    memcpy(&mem[addr], bytes, len);
    return true;
  }

  virtual void proc_reset(unsigned id) override {}
  virtual const char* get_symbol(uint64_t addr) override { return NULL; }
  [[nodiscard]] const cfg_t& get_cfg() const override { assert(0); }

  [[nodiscard]] const std::map<size_t, processor_t*>& get_harts()
      const override {
    assert(0);
  }

 private:
  char* mem;
  uint64_t mem_size;
  uartlite uart;
  reg_t uart_addr = 0x60000000;
};

class Spike {
 public:
  Spike(uint64_t mem_size);

  processor_t* get_proc() { return &proc; }
  sim_t* get_sim() { return &sim; }

 private:
  std::string varch;
  cfg_t cfg;
  sim_t sim;
  isa_parser_t isa;
  processor_t proc;
};

Spike::Spike(uint64_t mem_size)
    : sim(mem_size),
      isa("rv32gcv", "M"),
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
          /*sout*/ std::cerr) {}

uint64_t spike_new(uint64_t mem_size) {
  Spike* spike = new Spike(mem_size);

  return (uint64_t)spike;
}

void spike_delete(uint64_t spike) {
  Spike* s = (Spike*)spike;
  std::cerr << "Deleting spike: " << s << std::endl;
  delete s;
}

int32_t spike_execute(uint64_t spike) {
  Spike* s = (Spike*)spike;
  processor_t* proc = s->get_proc();

  auto state = proc->get_state();
  auto fetch = proc->get_mmu()->load_insn(state->pc);

  std::cerr << "pc:" << fmt::format("{:08x}", state->pc) << " ";
  std::cerr << "bits:" << fmt::format("{:08x}", fetch.insn.bits()) << " ";
  std::cerr << "disasm:" << proc->get_disassembler()->disassemble(fetch.insn) << std::endl;

  reg_t pc = fetch.func(proc, fetch.insn, state->pc);

  if ((pc & 1) == 0) {
    state->pc = pc;
  } else {
    return -1;
  }

  return 0;
}

int32_t spike_get_reg(uint64_t spike, uint64_t index, uint64_t* content) {
  Spike* s = (Spike*)spike;
  processor_t* proc = s->get_proc();
  state_t* state = proc->get_state();
  *content = state->XPR[index];
  return 0;
}

int32_t spike_set_reg(uint64_t spike, uint64_t index, uint64_t content) {
  Spike* s = (Spike*)spike;
  processor_t* proc = s->get_proc();
  if (index >= NXPR) {
    return -1;
  }
  state_t* state = proc->get_state();
  state->XPR.write(index, content);
  return 0;
}

int spike_ld(uint64_t spike, uint64_t addr, uint64_t len, uint8_t* bytes) {
  Spike* s = (Spike*)spike;
  processor_t* proc = s->get_proc();
  sim_t* sim = s->get_sim();
  bool success = sim->mmio_load(addr, len, bytes);
  if (success) {
    return 0;
  } else {
    return -2;
  }
}

int spike_sd(uint64_t spike, uint64_t addr, uint64_t len, uint8_t* bytes) {
  Spike* s = (Spike*)spike;
  sim_t* sim = s->get_sim();
  bool success = sim->mmio_store(addr, len, bytes);
  if (success) {
    return 0;
  } else {
    return -3;
  }
}

int spike_ld_elf(uint64_t spike, uint64_t addr, uint64_t len, uint8_t* bytes) {
  Spike* s = (Spike*)spike;
  sim_t* sim = s->get_sim();
  bool success = sim->load_elf(addr, len, bytes);
  if (success) {
    return 0;
  } else {
    return -4;
  }
}

int spike_init(uint64_t spike, uint64_t entry_addr) {
  Spike* s = (Spike*)spike;
  processor_t* proc = s->get_proc();

  proc->reset();

  // Set the virtual supervisor mode and virtual user mode
  auto status = proc->get_state()->sstatus->read() | SSTATUS_VS | SSTATUS_FS;
  proc->get_state()->sstatus->write(status);
  proc->get_state()->pc = entry_addr;

  return 0;
}
