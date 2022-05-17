#include "types.h"
#include "debug.h"
#include "compile.h"
#include "AvroraTrace.h"
#include "memory.h"
#include "safety_check.h"
#include <avr/pgmspace.h>
extern translation_state ts;
#if flash_data
// only ram
#define addr_is_in_flash(wasm_addr) (wasm_addr>=mem_areas[0].start&&wasm_addr<mem_areas[0].end)

#define addr_W2N(wasm_addr) (mem_areas[1].target+wasm_addr-mem_areas[1].start)
#define addr_W2N_P(wasm_addr) (mem_areas[0].target+wasm_addr-mem_areas[0].start)
#else
#define addr_W2N(wasm_addr) (ts.wasm_mem_space+ts.wasm_globals_size+wasm_addr)
#endif
i32 printInt(i32 res)
{
    // printf("hello, world!\r\n");
    printf("%ld\n",res);
    return res;
}
void printStr(u32 ptr)
{
#if flash_data
    if(addr_is_in_flash(ptr)){
        printf_P(addr_W2N_P(ptr));
        printf("\n");
    }else{
        printf("%s\n",addr_W2N(ptr));
    }
#else
    printf("%s\n",addr_W2N(ptr));
#endif

}

u32 shift(u32 input, u32 move){
    return input != move;
}

void rtc_startBenchmarkMeasurement_Native(){
    // printf("benchmark start.\r\n");
    avroraTraceEnable();
}
void rtc_stopBenchmarkMeasurement(){
    // printf("benchmark stop.\r\n");
    avroraTraceDisable();
}
u16  __attribute__((section (".wait"))) malloc_record;
u32 import_malloc(u32 size){
    u32 res = malloc_record;
    malloc_record +=size;
    log(sys,"malloc %ld %x",size,(u16)res);
    // printf("malloc from %d to %d\n",(u16)res,(u16)(malloc_record));
    return res;
}

u32 import_memset(u32 s,u32 ch,u32 n){
    for(int i=0;i<n;i++){
        *(char*)(addr_W2N(s)+i) = (u8)ch;
    }
    return s;
}

u32 import_memcpy(u32 d,u32 s,u32 n){
    #if flash_data
        if(addr_is_in_flash(s)){
            memcpy_PF(addr_W2N(d),addr_W2N_P(s),n);
        }else{
            memcpy(addr_W2N(d),addr_W2N(s),n);
        }
    #else
        memcpy(addr_W2N(d),addr_W2N(s),n);
    #endif
    return s;
}

u32 test(u32 input1,u32 input2,u32 input3){
    return input3==0?input2:input1;
}



#define IMPORTS_NUM 7
normal_function imports[IMPORTS_NUM]={
    printInt,
    rtc_startBenchmarkMeasurement_Native,
    rtc_stopBenchmarkMeasurement,
    import_malloc,
    import_memset,
    import_memcpy,
    printStr,
    };
char* imports_name[IMPORTS_NUM]={
    "printInt",
    "rtc_startBenchmarkMeasurement_Native",
    "rtc_stopBenchmarkMeasurement",
    "malloc",
    "memset",
    "memcpy",
    "printStr",
    };
u32 imports_num=IMPORTS_NUM;
