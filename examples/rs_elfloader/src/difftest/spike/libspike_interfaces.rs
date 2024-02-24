use libc::{c_char, c_void};

#[repr(C)]
pub struct Spike {
	spike: *const c_void,
}

#[repr(C)]
pub struct Processor {
	processor: *const c_void,
}

#[repr(C)]
pub struct State {
	state: *const c_void,
}

#[repr(C)]
pub struct Mmu {
	mmu: *const c_void,
}

type FfiCallback = extern "C" fn(u64) -> *mut u8;

#[link(name = "spike_interfaces")]
extern "C" {
  pub fn spike_register_callback(callback: FfiCallback);
	pub fn spike_new(arch: *const c_char, set: *const c_char, lvl: *const c_char) -> *const Spike;
	pub fn proc_disassemble(proc: *const Processor, pc: u64) -> *const c_char;
	pub fn proc_reset(proc: *const Processor);
	pub fn spike_get_proc(spike: *const Spike) -> *const Processor;
	pub fn proc_get_state(proc: *const Processor) -> *const State;
	pub fn proc_get_mmu(proc: *const Processor) -> *const Mmu;
	pub fn proc_func(proc: *const Processor, pc: u64) -> u64;
	pub fn state_get_pc(state: *const State) -> u64;
	pub fn handle_pc(state: *const State, pc: u64) -> u64;
	pub fn state_set_pc(state: *const State, pc: u64);
	pub fn state_set_serialized(state: *const State, serialized: bool);
	pub fn spike_destruct(spike: *const Spike);
	pub fn proc_destruct(proc: *const Processor);
	pub fn state_destruct(state: *const State);
	pub fn mmu_destruct(mmu: *const Mmu);
	pub fn spike_exit(state: *const State) -> u64;
}

