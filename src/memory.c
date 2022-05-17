#include "compile.h"
#include "memory.h"
#include "debug.h"
#include "safety_check.h"
#include <avr/pgmspace.h>
#define show_addr  0
#define show_value 0

mem_area __attribute__((section(".wait"))) mem_areas[WASM_MEM_AREA_NUM] = {{0, 0, 0, 0}, {0, 4096, 0, 0}};
// __attribute__((section (".wait")))
u32 embed_i32load(u16 offset1, u32 addr)
{
    u16 target = addr + offset1;
    u32 res;
    #if show_addr
    printf("[read]\t%d\n", target);
    #endif
#if flash_data
    if (target >= mem_areas[0].start && target < mem_areas[0].end)
    {
        // printf("actual r %p\n",mem_areas[0].target + target - mem_areas[0].start);
        res = pgm_read_dword_far(mem_areas[0].target + target - mem_areas[0].start);
    }
    else
    {
        if (target >= mem_areas[1].start && target < mem_areas[1].end)
        {
            // printf("r %p\r\n",(u8*)mem_areas[i].target+target-mem_areas[i].start);
            u16 addr = mem_areas[1].target + target - mem_areas[1].start;
            // printf("actual r %p\n",addr);
            // printf("mem[1] s:%p e:%p t:%p\n",mem_areas[1].start,mem_areas[1].end,mem_areas[1].target);
            res = *(u32 *)(addr);
        }
        else
        {
            printf("read out of bound! at %d\n", target);
            asm volatile("break");
        }
    }
#else
    if (target < 0 || target >= 4096)
    {
        printf("read out of bound! at %d\n", target);
        asm volatile("break");
    }
    res = *(u32 *)((u8 *)ts.wasm_mem_space + ts.wasm_globals_size + target);
#endif

#if show_value
    printf("[value]\t%ld\n",res);
#endif
    return res;
}
u64 embed_i64load(u16 offset1, u32 addr)
{
    u16 target = addr + offset1;
    union
    {
        u32 n32[2];
        u64 n64;
    } res;

    u8 overflow = 1;
    #if show_addr
    printf("[read]\t%d\n",target);
    #endif

#if flash_data
    if (target >= mem_areas[0].start && target < mem_areas[0].end)
    {
        res.n32[0] = pgm_read_dword_far(mem_areas[0].target + target - mem_areas[0].start);
        res.n32[1] = pgm_read_dword_far(mem_areas[0].target + target - mem_areas[0].start + 4);
    }
    else
    {
        if (target >= mem_areas[1].start && target < mem_areas[1].end)
        {
            // printf("r %p\r\n",(u8*)mem_areas[i].target+target-mem_areas[i].start);
            res.n32[0] = *(u32 *)((u8 *)mem_areas[1].target + target - mem_areas[1].start);
            res.n32[1] = *(u32 *)((u8 *)mem_areas[1].target + target - mem_areas[1].start + 4);
        }
        else
        {
            printf("read out of bound! at %d\n", target);
            asm volatile("break");
        }
    }
#else
    if (target < 0 || target >= 4096)
    {
        printf("read out of bound! at %d\n", target);
        asm volatile("break");
    }
    res.n32[0] = *(u32 *)((u8 *)ts.wasm_mem_space + ts.wasm_globals_size + target);
    res.n32[1] = *(u32 *)((u8 *)ts.wasm_mem_space + ts.wasm_globals_size + target + 4);
#endif
#if show_value
    printf("[value]\t%ld:%ld\n",res.n32[0],res.n32[1]);
#endif
    return res.n64;
}

void embed_i32store(u32 value, u16 offset1, u16 addr)
{
    u16 target = addr + offset1;
    #if show_addr
    printf("[write]\t%d\n",target);
    #endif
    #if show_value
    printf("[value]\t%ld\n",value);
    #endif
    if (target >= mem_areas[0].start && target < mem_areas[0].end)
    {
        printf("write to read-only section!\n");
        asm volatile("break");
    }
    else
    {
        if (target >= mem_areas[1].start && target < mem_areas[1].end)
        {
            *(u32 *)((u8 *)mem_areas[1].target + target - mem_areas[1].start) = value;
        }
        else
        {
            printf("write out of bound! at %d\n", target);
            asm volatile("break");
        }
    }
}
void embed_i32store16(u32 value, u16 offset1, u16 addr)
{
    u16 target = addr + offset1;
    #if show_addr
    printf("[write]\t%d\n",target);
    #endif
    #if show_value
    printf("[value]\t%ld\n",value);
    #endif
    if (target >= mem_areas[0].start && target < mem_areas[0].end)
    {
        printf("write to read-only section!\n");
        asm volatile("break");
    }
    else
    {
        if (target >= mem_areas[1].start && target < mem_areas[1].end)
        {
            *(u16 *)((u8 *)mem_areas[1].target + target - mem_areas[1].start) = (u16)value;
        }
        else
        {
            printf("write out of bound! at %d\n", target);
            asm volatile("break");
        }
    }
}
void embed_i32store8(u32 value, u16 offset1, u16 addr)
{
    u16 target = addr + offset1;
    #if show_addr
    printf("[write]\t%d\n",target);
    #endif
    #if show_value
    printf("[value]\t%ld\n",value);
    #endif
    if (target >= mem_areas[0].start && target < mem_areas[0].end)
    {
        printf("write to read-only section!\n");
        asm volatile("break");
    }
    else
    {
        if (target >= mem_areas[1].start && target < mem_areas[1].end)
        {
            *(u8 *)((u8 *)mem_areas[1].target + target - mem_areas[1].start) = (u8)value;
        }
        else
        {
            printf("write out of bound! at %d\n", target);
            asm volatile("break");
        }
    }
}

void empty_function()
{
}
u16 func_type_map[5] = {0, 1, 2, 3, 4};
u16 pseudo_table[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
u16 table_length = 2;
u32 table_address;
void icall(u32 index)
{
    normal_function func = pgm_read_word_far(table_address + 2 * index);

#if check_icall_type
    if (func_type_map[pseudo_table[index]] != 0)
    {
        asm volatile("break");
    }
#endif
#if check_icall_bound
    if (index >= table_length)
    {
        asm volatile("break");
    }
#endif
    func();
}