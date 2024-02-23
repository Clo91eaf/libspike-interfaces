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

type FfiCallback = extern "C" fn(u64) -> *mut u8;
static mut FFI_ADDR_TO_MEM: Option<FfiCallback> = None;

#[link(name = "spike-interfaces")]
extern "C" {
	pub fn spike_new(arch: *const c_char, set: *const c_char, lvl: *const c_char) -> *const Spike;
	pub fn proc_disassemble(proc: *const Processor, mmu: *const Mmu, pc: u64) -> *const c_char;
	pub fn proc_reset(proc: *const Processor);
	pub fn spike_get_proc(spike: *const Spike) -> *const Processor;
	pub fn proc_get_state(proc: *const Processor) -> *const State;
	pub fn proc_get_mmu(proc: *const Processor) -> *const Mmu;
	pub fn mmu_func(mmu: *const Mmu, proc: *const Processor, pc: u64) -> u64;
	pub fn state_get_pc(state: *const State) -> u64;
	pub fn state_set_pc(state: *const State, pc: u64);
	pub fn state_set_serialized(state: *const State, serialized: bool);
	pub fn destruct(ptr: *const c_void);
	pub fn spike_exit(state: *const State) -> u64;
}

#[no_mangle]
pub extern "C" fn spike_register_callback(callback: FfiCallback) {
	unsafe {
		FFI_ADDR_TO_MEM = Some(callback);
	}
}
