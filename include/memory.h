#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
typedef struct MEM_AREA
{
    u16 start;
    u16 end;
    u32 target;
}mem_area;

#define WASM_MEM_AREA_NUM 2
extern mem_area mem_areas[WASM_MEM_AREA_NUM];

u32 embed_i32load(u16 offset1,u32 addr);
u64 embed_i64load(u16 offset1,u32 addr);
void embed_i32store(u32 value,u16 offset1,u16 addr);
void embed_i32store16(u32 value, u16 offset1, u16 addr);
void embed_i32store8(u32 value, u16 offset1, u16 addr);
#endif
