#ifndef DEBUG_H
#define DEBUG_H
#include"types.h"
#include<stdio.h>
#include<avr/io.h>
#define DEBUG 1
#define d_log_parse     0
#define d_log_compile   0
#define d_log_wkreprog  0
#define d_log_emit      0
#define d_log_sys       1
#define d_log_temp      1
#define d_log_panic     0


#ifdef AVRORA
// extern char global_print_buff[128];
// #define printf(FMT,...) do{snprintf(global_print_buff,128,FMT,##__VA_ARGS__);avr_Print(global_print_buff);}while(0)

    void avr_Printf(u32 format, ...);
    #define printf(fmt,...)    do{static char __attribute__((section (".progmem"))) literal[]=fmt;avr_Printf(literal,##__VA_ARGS__);}while(0)
    #define printf_P    avr_Printf
#endif

#if DEBUG

    // #define d_Log(CATEGORY, FMT, ...)                  printf (" %8s | " FMT, #CATEGORY, ##__VA_ARGS__)
    #define d_Log(CATEGORY, FMT, ...)                  printf (" "#CATEGORY" | " FMT, ##__VA_ARGS__)
    #if d_log_parse
        #define log_parse(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_parse(...) do{}while(0)
    #endif

    #if d_log_wkreprog
        #define log_wkreprog(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_wkreprog(...) do{}while(0)
    #endif

    #if d_log_compile
        #define log_compile(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_compile(...) do{}while(0)
    #endif

    #if d_log_emit
        #define log_emit(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_emit(...) do{}while(0)
    #endif

    #if d_log_sys
        #define log_sys(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_sys(...) do{}while(0)
    #endif

    #if d_log_temp
        #define log_temp(CATEGORY, FMT, ...)          d_Log(CATEGORY, FMT, ##__VA_ARGS__)
    #else
        #define log_temp(...) do{}while(0)
    #endif


    #define log(CATEGORY, FMT, ...)                    log_##CATEGORY (CATEGORY, FMT "\r\n", ##__VA_ARGS__)
    #define logif(CATEGORY, STATEMENT)                 do{log_##CATEGORY (CATEGORY, ""); if (d_log_##CATEGORY) { STATEMENT; printf ("\r\n"); }}while(0)



#else
    #define log(CATEGORY, FMT, ...)                    do{}while(0)
    #define logif(CATEGORY, STATEMENT)                 do{}while(0)
# endif

#if d_log_panic
#define panicf(FMT,...)  do{printf("panic! : "FMT " %s :%d\r\n",##__VA_ARGS__,__FILE__,__LINE__);asm volatile("break");}while(0)
#define panic()  do{printf("panic at ""%s :%d\r\n",__FILE__,__LINE__);asm volatile("break");}while(0)
#else
#define panicf(FMT,...)  do{while(1);}while(0)
#define panic()  do{while(1);}while(0)
#endif

#define STACK_POINTER() ((char *)AVR_STACK_POINTER_REG)

#endif //DEBUG_H
