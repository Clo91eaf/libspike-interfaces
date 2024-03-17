use crate::info;
use std::fs::File;
use std::io::Read;
use xmas_elf::{
	header,
	program::{ProgramHeader, Type},
	ElfFile,
};
use std::cell::RefCell;

mod libspike_interfaces;
use libspike_interfaces::*;

// read the addr from spike memory
// caller should make sure the address is valid

pub struct SpikeMem {
	pub mem: Vec<u8>,
	pub size: usize,
}

impl SpikeMem {
	fn new(size: usize) -> Self {
		info!("SpikeMem created: size: 0x{:x}", size);
		SpikeMem {
			mem: vec![0; size],
			size,
		}
	}

	#[no_mangle]
	pub extern "C" fn rs_addr_to_mem(&mut self, addr: u64) -> *mut u8 {
		let addr = addr as usize;
		&mut self.mem[addr] as *mut u8
	}

	fn ld(&mut self, addr: usize, len: usize, bytes: Vec<u8>) -> anyhow::Result<()> {
		info!("ld: addr: 0x{:x}, len: 0x{:x}", addr, len);
		assert!(addr + len <= self.size);

		let dst = &mut self.mem[addr..addr + len];
		for (i, byte) in bytes.iter().enumerate() {
			dst[i] = *byte;
		}

		Ok(())
	}

	pub fn load_elf(&mut self, fname: &str) -> anyhow::Result<u64> {
		let mut file = File::open(fname).unwrap();
		let mut buffer = Vec::new();
		file.read_to_end(&mut buffer).unwrap();

		let elf_file = ElfFile::new(&buffer).unwrap();

		let header = elf_file.header;
		assert_eq!(header.pt2.machine().as_machine(), header::Machine::RISC_V);
		assert_eq!(header.pt1.class(), header::Class::ThirtyTwo);

		for ph in elf_file.program_iter() {
			match ph {
				ProgramHeader::Ph32(ph) => {
					if ph.get_type() == Ok(Type::Load) {
						let offset = ph.offset as usize;
						let size = ph.file_size as usize;
						let addr = ph.virtual_addr as usize;

						let slice = &buffer[offset..offset + size];
						self.ld(addr, size, slice.to_vec()).unwrap();
					}
				}
				_ => (),
			}
		}

		Ok(header.pt2.entry_point())
	}
}

impl Drop for SpikeMem {
	fn drop(&mut self) {
		info!("SpikeMem dropped");
	}
}

pub struct SpikeHandle {
	spike: Spike,
	_mem: Box<SpikeMem>,
}

impl Drop for SpikeHandle {
	fn drop(&mut self) {
		info!("SpikeHandle dropped");
	}
}

impl SpikeHandle {
	pub fn new(size: usize, fname: &str) -> Self {
		// create a new spike memory instance
		let mut _mem = Box::new(SpikeMem::new(size));

		// load the elf file
		let entry_addr = _mem.load_elf(fname).unwrap();

		// initialize spike
		let arch = "vlen:1024,elen:32";
		let set = "rv32gcv";
		let lvl = "M";

		let mut callback = |x| _mem.rs_addr_to_mem(x);

		let spike = Spike::new(arch, set, lvl, &mut callback);

		// initialize processor
		let proc = spike.get_proc();
		let state = proc.get_state();
		proc.reset();
		state.set_pc(entry_addr);

		SpikeHandle { spike, _mem }
	}

	pub fn exec(&self) -> anyhow::Result<u64> {
		let spike = &self.spike;
		let proc = spike.get_proc();
		let state = proc.get_state();

		let pc = state.get_pc();
		let disasm = proc.disassemble(pc);

		info!("pc: 0x{:x}, disasm: {:?}", pc, disasm);

		let new_pc = proc.func(pc);

		state.handle_pc(new_pc).unwrap();

		let ret = state.exit();

		Ok(ret)
	}
}
