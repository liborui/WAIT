#include "benchmark.h"
#include "AvroraPrint.h"
#include "AvroraTrace.h"
#include <stdarg.h>
#include <stdio.h>

void avr_Print(char * str)
{
	int i;
	for (i=0; str[i]!=0; i++) {
		if (str[i] == '\n') {
			avroraPrintCharBuffer();
		} else {
			avroraWriteCharBuffer(str[i]);
		}
	}
}

void avr_VPrint(char * format, va_list arg)
{
	static char temp[128];
	vsnprintf(temp, 128, format, arg);
	avr_Print(temp);
}

void avr_Printf(char * format, ...)
{
	va_list arg;

	va_start(arg, format);
	avr_VPrint(format, arg);
	va_end(arg);
}


// extern void* malloc(uint16_t size);
void rtc_startBenchmarkMeasurement_Native(){
    // avr_Print("start");
    // avroraWriteCharBuffer('b');
    // avroraWriteCharBuffer('e');
    // avroraWriteCharBuffer('n');
    // avroraWriteCharBuffer('c');
    // avroraWriteCharBuffer('h');
    // avroraWriteCharBuffer('s');
    // avroraWriteCharBuffer('t');
    // avroraWriteCharBuffer('a');
    // avroraWriteCharBuffer('r');
    // avroraWriteCharBuffer('t');
    // avroraPrintCharBuffer();
    avroraTraceEnable();
}
void rtc_stopBenchmarkMeasurement(){
    avroraTraceDisable();
    // avroraWriteCharBuffer('b');
    // avroraWriteCharBuffer('e');
    // avroraWriteCharBuffer('n');
    // avroraWriteCharBuffer('c');
    // avroraWriteCharBuffer('h');
    // avroraWriteCharBuffer('s');
    // avroraWriteCharBuffer('t');
    // avroraWriteCharBuffer('o');
    // avroraWriteCharBuffer('p');
    // avroraPrintCharBuffer();
}

void printInt(int value){
    avr_Printf("%d\n",value);
}

void printStr(uint8_t* str){
    avr_Printf("%s\n",str);
}

int main(){
    javax_rtcbench_RTCBenchmark_void_test_native();
    asm volatile ("break");
}
