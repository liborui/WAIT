#ifndef COMPILE_H
#define COMPILE_H
#include"types.h"

void wasm_compile_module(wasm_module_ptr module);
void wasm_call_entry_method(wasm_module_ptr module);
extern translation_state ts;
extern wasm_block_ct blct;

#endif