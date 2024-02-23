use crate::info;
use lazy_static::lazy_static;
use libc::c_char;
use std::ffi::CString;
use std::fs::File;
use std::io::Read;
use std::sync::Mutex;
use xmas_elf::{
	header,
	program::{ProgramHeader, Type},
	ElfFile,
};

mod libspike_interfaces;
use libspike_interfaces::*;

// read the addr from spike memory
// caller should make sure the address is valid
#[no_mangle]
pub extern "C" fn rs_addr_to_mem(addr: u64) -> *mut u8 {
	let addr = addr as usize;
	let mut spike_mem = SPIKE_MEM.lock().unwrap();
	let spike_mut = spike_mem.as_mut().unwrap();
	&mut spike_mut.mem[addr] as *mut u8
}

pub struct SpikeMem {
	pub mem: Vec<u8>,
	pub size: usize,
}

lazy_static! {
	static ref SPIKE_MEM: Mutex<Option<Box<SpikeMem>>> = Mutex::new(None);
}

fn init_memory(size: usize) {
	let mut spike_mem = SPIKE_MEM.lock().unwrap();
	if spike_mem.is_none() {
		info!("Creating SpikeMem with size: 0x{:x}", size);
		*spike_mem = Some(Box::new(SpikeMem { mem: vec![0; size], size }));
	}
}

fn ld(addr: usize, len: usize, bytes: Vec<u8>) -> anyhow::Result<()> {
	info!("ld: addr: 0x{:x}, len: 0x{:x}", addr, len);
	let mut spike_mem = SPIKE_MEM.lock().unwrap();
	let spike_ref = spike_mem.as_mut().unwrap();

	assert!(addr + len <= spike_ref.size);

	let dst = &mut spike_ref.mem[addr..addr + len];
	for (i, byte) in bytes.iter().enumerate() {
		dst[i] = *byte;
	}

	Ok(())
}

fn load_elf(fname: &str) -> anyhow::Result<u64> {
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
					ld(addr, size, slice.to_vec()).unwrap();
				}
			}
			_ => (),
		}
	}

	Ok(header.pt2.entry_point())
}

pub struct SpikeHandle {
	spike: *const Spike,
}

impl SpikeHandle {
	pub fn new(size: usize, fname: &str) -> Self {
		// register the addr_to_mem callback
		unsafe { spike_register_callback(rs_addr_to_mem) }

		// create a new spike memory instance
		init_memory(size);

		// load the elf file
		let entry_addr = load_elf(fname).unwrap();

		// initialize spike
		let arch_cstring = CString::new("vlen:1024,elen:32").unwrap();
		let set_cstring = CString::new("rv32gcv").unwrap();
		let lvl_cstring = CString::new("M").unwrap();

		let arch = arch_cstring.as_ptr();
		let set = set_cstring.as_ptr();
		let lvl = lvl_cstring.as_ptr();
		let spike = unsafe { spike_new(arch, set, lvl) };

		// initialize processor
		let proc = unsafe { spike_get_proc(spike) };
		let state = unsafe { proc_get_state(proc) };
		unsafe { proc_reset(proc) };
		unsafe { state_set_pc(state, entry_addr) };

		SpikeHandle { spike }
	}

	pub fn exec(&self) -> anyhow::Result<i32> {
		let spike = self.spike;
		let proc = unsafe { spike_get_proc(spike) };
		let state = unsafe { proc_get_state(proc) };
		let mmu = unsafe { proc_get_mmu(proc) };

		let pc = unsafe { state_get_pc(state) };
		let disasm = unsafe { proc_disassemble(proc, mmu, pc) }; // TODO: free disasm

		info!("pc: 0x{:x}, disasm: {:?}", pc, unsafe {
			CString::from_raw(disasm as *mut c_char)
		});

		let new_pc = unsafe { mmu_func(mmu, proc, pc) };

		match new_pc {
			pc if pc % 2 == 0 => unsafe { state_set_pc(state, pc) },
			3 => unsafe { state_set_serialized(state, true) },
			5 => {}
			_ => panic!("Invalid new_pc: 0x{:x}", new_pc),
		}

		Ok(0)
	}
}
