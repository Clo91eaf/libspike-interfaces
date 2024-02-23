use libc::{c_char, c_void};

#[repr(C)]
pub struct Spike {
	spike: *const c_void,
}

impl Drop for Spike {
	fn drop(&mut self) {
		unsafe {
			destruct(self.spike);
		}
	}
}

#[repr(C)]
pub struct Processor {
	processor: *const c_void,
}

impl Drop for Processor {
	fn drop(&mut self) {
		unsafe {
			destruct(self.processor);
		}
	}
}

#[repr(C)]
pub struct State {
	state: *const c_void,
}

impl Drop for State {
	fn drop(&mut self) {
		unsafe {
			destruct(self.state);
		}
	}
}

#[repr(C)]
pub struct Mmu {
	mmu: *const c_void,
}

impl Drop for Mmu {
	fn drop(&mut self) {
		unsafe {
			destruct(self.mmu);
		}
	}
}

type Addr2Mem = extern "C" fn(u64) -> *mut u8;

#[link(name = "spike-interfaces")]
extern "C" {
	pub fn spike_new(arch: *const c_char, set: *const c_char, lvl: *const c_char) -> Box<Spike>;
	pub fn spike_register_callback(callback: Addr2Mem);
	pub fn proc_disassemble(proc: Box<Processor>, mmu: Box<Mmu>, pc: u64) -> *const c_char;
	pub fn proc_reset(proc: Box<Processor>);
	pub fn spike_get_proc(spike: Box<Spike>) -> Box<Processor>;
	pub fn proc_get_state(proc: Box<Processor>) -> Box<State>;
	pub fn proc_get_mmu(proc: Box<Processor>) -> Box<Mmu>;
	pub fn mmu_func(mmu: Box<Mmu>, proc: Box<Processor>, pc: u64) -> u64;
	pub fn state_get_pc(state: Box<State>) -> u64;
	pub fn state_set_pc(state: Box<State>, pc: u64);
	pub fn state_set_serialized(state: Box<State>, serialized: bool);
	pub fn destruct(ptr: *const c_void);
	pub fn spike_exit(state: Box<State>) -> u64;
}
