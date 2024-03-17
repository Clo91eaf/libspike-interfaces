#ifndef __SPIKE_INTERFCES_C_H__
#define __SPIKE_INTERFCES_C_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char* (*callback_t)(void* data, uint64_t args);

callback_t global_callback;
void* global_data;

typedef struct spike_t spike_t;
typedef struct spike_processor_t spike_processor_t;
typedef struct spike_state_t spike_state_t;

void spike_set_callback(callback_t callback, void* data);
spike_t* spike_new(const char* arch, const char* set, const char* lvl);
spike_processor_t* spike_get_proc(spike_t* spike);
void spike_destruct(spike_t* spike);
const char* proc_disassemble(spike_processor_t* proc, uint64_t pc);
void proc_reset(spike_processor_t* proc);
spike_state_t* proc_get_state(spike_processor_t* proc);
uint64_t proc_func(spike_processor_t* proc, uint64_t pc);
void proc_destruct(spike_processor_t* proc);
uint64_t state_get_pc(spike_state_t* state);
uint64_t state_handle_pc(spike_state_t* state, uint64_t new_pc);
void state_set_pc(spike_state_t* state, uint64_t pc);
void state_destruct(spike_state_t* state);
uint64_t state_exit(spike_state_t* state);

#ifdef __cplusplus
}
#endif

#endif  // __SPIKE_INTERFCES_C_H__
