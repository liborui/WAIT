#include "debug.h"
#ifdef AVRORA
#include<AvroraPrint.h>
#include<avr/pgmspace.h>
#define GLOBAL_PRINT_BUFFER_SIZE 100

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

void avr_VPrint(u32 format, va_list arg)
{
	static char __attribute__((section (".wait"))) temp[GLOBAL_PRINT_BUFFER_SIZE];
	static char __attribute__((section (".wait"))) temp2[GLOBAL_PRINT_BUFFER_SIZE];
	int i=0;
	char c;
	do{
		c = pgm_read_byte_far(format+i);
		temp2[i++]=c;
	}while(c&&i<=90);
	vsnprintf(temp, GLOBAL_PRINT_BUFFER_SIZE, temp2, arg);
	avr_Print(temp);
}

void avr_Printf(u32 format, ...)
{
	va_list arg;

	va_start(arg, format);
	avr_VPrint(format, arg);
	va_end(arg);
}
// char global_print_buff[128];

#endif