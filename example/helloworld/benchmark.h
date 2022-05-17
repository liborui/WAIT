#ifndef BENCHMARK_H
#define BENCHMARK_H
// #define NULL 0

#ifdef AVRORA
#include <stdint.h>
#include <stdbool.h>
#include<AvroraPrint.h>
void printInt(int value);
void printStr(uint8_t* str);
void avr_Printf(char * format, ...);
#else
#include <stddef.h>
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned long       uint32_t;
typedef long                int32_t;
typedef unsigned long long  uint64_t;
typedef long long           int64_t;
typedef uint8_t            bool;
#define true                1
#define false               0

extern void printInt(int value);
extern void PrintStr(uint8_t* str);
extern void* malloc(uint32_t size);
extern void *memset(void *s, int c, uint32_t n);
extern void *memcpy(void *d, void *s, uint32_t n);
#endif

extern void rtc_startBenchmarkMeasurement_Native();
extern void rtc_stopBenchmarkMeasurement();

#endif