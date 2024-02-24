use libc::{c_char, c_void};

#[repr(C)]
pub struct Spike {
	spike: *mut (),
}

#[repr(C)]
pub struct Processor {
	processor: *mut (),
}

#[repr(C)]
pub struct State {
	state: *mut (),
}

#[repr(C)]
pub struct Mmu {
	mmu: *mut (),
}

type FfiCallback = extern "C" fn(u64) -> *mut u8;

#[link(name = "spike_interfaces")]
extern "C" {
  pub fn spike_register_callback(callback: FfiCallback);
	pub fn spike_new(arch: *const c_char, set: *const c_char, lvl: *const c_char) -> *mut Spike;
	pub fn proc_disassemble(proc: *mut Processor, pc: u64) -> *mut c_char;
	pub fn proc_reset(proc: *mut Processor);
	pub fn spike_get_proc(spike: *mut Spike) -> *mut Processor;
	pub fn proc_get_state(proc: *mut Processor) -> *mut State;
	pub fn proc_get_mmu(proc: *mut Processor) -> *mut Mmu;
	pub fn proc_func(proc: *mut Processor, pc: u64) -> u64;
	pub fn state_get_pc(state: *mut State) -> u64;
	pub fn handle_pc(state: *mut State, pc: u64) -> u64;
	pub fn state_set_pc(state: *mut State, pc: u64);
	pub fn state_set_serialized(state: *mut State, serialized: bool);
	pub fn spike_destruct(spike: *mut Spike);
	pub fn proc_destruct(proc: *mut Processor);
	pub fn state_destruct(state: *mut State);
	pub fn mmu_destruct(mmu: *mut Mmu);
	pub fn spike_exit(state: *mut State) -> u64;
}
