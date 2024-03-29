mod dut;
mod spike;

// use dut::Dut;
use spike::SpikeHandle;

pub struct Difftest {
	spike: SpikeHandle,
	// dut: Dut,
}

impl Difftest {
	pub fn new(size: usize, elf_file:&str) -> Self {
		Self {
			spike: SpikeHandle::new(size, elf_file),
			// dut: Dut::new(fst_file.to_string()),
		}
	}

	pub fn execute(&mut self) -> anyhow::Result<u64> {
		self.spike.exec()
	}
}
