#ifndef PARSE_H
#define PARSE_H

// typedef struct WASM_MODULE
// {
// 	char *name;
// 	char *code_section;
// 	int code_section_len;

// 	bytes entry_method;
// } wasm_module;
// typedef wasm_module *wasm_module_ptr;

// typedef struct WASM_CODE
// {
// 	unsigned char *ptr;
// 	unsigned int length;
// } wasm_code;
// typedef wasm_code *wasm_code_ptr;

wasm_module_ptr wasm_load_module(wasm_code_ptr code);
#endif