/*
 * Copyright (c) 2009 Cork Institute of Technology, Ireland
 * All rights reserved."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written
 * agreement is hereby granted, provided that the above copyright notice, the
 * following two paragraphs and the author appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE CORK INSTITUTE OF TECHNOLOGY BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE
 * CORK INSTITUTE OF TECHNOLOGY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * THE CORK INSTITUTE OF TECHNOLOGY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE CORK INSTITUTE OF TECHNOLOGY HAS NO OBLIGATION 
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 */                   

/* AvroraPrint.h
 *
 * This is the C code to print variables to the Avrora emulator
 *
 * How to use:
 *   (1) Include this file "AvroraPrintf.h" in your WSN application 
 *   (2) Send print statements like this:
 *	  
 *	  printChar('a');
 *
 *	  printInt8(44);
 *	  printInt16(3333);
 *	  printInt32(55556666);
 *
 *	  printStr("hello world");
 *
 *	  printHex8(0xFF);
 *    printHex16(0xFFFF);
 *	  printHex32(0xFFFFAAAA);
 *
 *	 (3) Compile and run the code with Avrora including the c-print option.
 *
 * Known bugs/limitations:
 *
 * 	 - If you include many print statements the emulator will slow down 
 * 
 * Notes:	 
 * 	
 * 	 - You can log the print statements to a file including the avrora
 * 	 option printlogfile="logfile.log". The saved file will be in the format
 * 	 logfile.log+nodeid 
 *
 *
 * @author AWS / Rodolfo De Paz http://www.aws.cit.ie/rodolfo
 * @contact avrora@lists.ucla.edu
 */

#ifndef _AVRORAPRINT_H_
#define _AVRORAPRINT_H_
#include <stdarg.h>
#ifdef __GNUC__
# define AVRORA_PRINT_INLINE __inline__
#else
/* Try the C99 keyword instead. */
# define AVRORA_PRINT_INLINE inline
#endif
volatile uint8_t debugbuf1[5];


#define AVRORA_PRINT_STRINGS  					0x2
#define AVRORA_PRINT_2BYTE_HEXADECIMALS			0x1
#define AVRORA_PRINT_2BYTE_UNSIGNED_INTEGERS 	0x3
#define AVRORA_PRINT_2BYTE_SIGNED_INTEGERS 		0x8
#define AVRORA_PRINT_4BYTE_HEXADECIMALS 		0x4
#define AVRORA_PRINT_4BYTE_UNSIGNED_INTEGERS 	0x5
#define AVRORA_PRINT_4BYTE_SIGNED_INTEGERS 		0x9
#define AVRORA_PRINT_STRING_POINTERS 			0x6
#define AVRORA_PRINT_BINARY_HEX_DUMPS 			0x7
#define AVRORA_WRITE_CHAR_BUFFER 				0xA
#define AVRORA_PRINT_CHAR_BUFFER 				0xB
#define AVRORA_PRINT_R1                         0xC
#define AVRORA_PRINT_SP                         0xD
#define AVRORA_PRINT_REGS                       0xE
#define AVRORA_PRINT_FLASH_STRING_POINTER       0xF
#define AVRORA_PRINT_PANIC                      0x10
#define AVRORA_PRINT_FREEHEAPMEMORY             0x11
#define AVRORA_PRINT_PC                         0x12
#define AVRORA_PRINT_DJ_HEAP                    0x13

static AVRORA_PRINT_INLINE void avroraPrintSetVarType(uint8_t a)
{
	debugbuf1[0] = a;
}
static AVRORA_PRINT_INLINE void avroraWriteCharBuffer(char c)
{
	debugbuf1[1] = c;
	avroraPrintSetVarType(AVRORA_WRITE_CHAR_BUFFER);
}
static AVRORA_PRINT_INLINE void avroraPrintCharBuffer()
{
	avroraPrintSetVarType(AVRORA_PRINT_CHAR_BUFFER);
}
static AVRORA_PRINT_INLINE void avroraPrintChar(char c)
{
	debugbuf1[1] = c;
	debugbuf1[2] = 0;
	avroraPrintSetVarType(AVRORA_PRINT_STRINGS);
}
static AVRORA_PRINT_INLINE void avroraPrintInt8(char c)
{
	int16_t i = (int16_t)c;
	debugbuf1[1] = (uint8_t)((uint16_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) i >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_SIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintUInt8(uint8_t c)
{
	debugbuf1[1] = c;
	debugbuf1[2] = 0;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_UNSIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintInt16(int16_t i)
{
	debugbuf1[1] = (uint8_t)((uint16_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) i >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_SIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintUInt16(int16_t i)
{
	debugbuf1[1] = (uint8_t)((uint16_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) i >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_UNSIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintInt32(int32_t i)
{
	debugbuf1[1] = (uint8_t)((uint32_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint32_t) i >> 8) & 0x00ff;
	debugbuf1[3] = (uint8_t)((uint32_t) i >> 16) & 0x00ff;
	debugbuf1[4] = (uint8_t)((uint32_t) i >> 24) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_4BYTE_SIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintUInt32(int32_t i)
{
	debugbuf1[1] = (uint8_t)((uint32_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint32_t) i >> 8) & 0x00ff;
	debugbuf1[3] = (uint8_t)((uint32_t) i >> 16) & 0x00ff;
	debugbuf1[4] = (uint8_t)((uint32_t) i >> 24) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_4BYTE_UNSIGNED_INTEGERS);
}
static AVRORA_PRINT_INLINE void avroraPrintStr(const char * const s)
{
	debugbuf1[1] = (uint8_t)((uint16_t) s) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) s >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_STRING_POINTERS);
}
static AVRORA_PRINT_INLINE void avroraPrintHexBuf(const uint8_t *b, uint16_t len)
{
	debugbuf1[1] = (uint8_t)((uint16_t) b) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) b >> 8) & 0x00ff;
	debugbuf1[3] = (uint8_t)((uint16_t) len) & 0x00ff;
	debugbuf1[4] = (uint8_t)((uint16_t) len >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_BINARY_HEX_DUMPS);
}
static AVRORA_PRINT_INLINE void avroraPrintHex8(uint8_t c)
{
	debugbuf1[1] = c;
	debugbuf1[2] = 0;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_HEXADECIMALS);
}
static AVRORA_PRINT_INLINE void avroraPrintHex16(uint16_t i)
{
	debugbuf1[1] = i & 0x00ff;
	debugbuf1[2] = (i >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_HEXADECIMALS);
}
static AVRORA_PRINT_INLINE void avroraPrintHex32(uint32_t i)
{
	debugbuf1[1] = (uint8_t)((uint32_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint32_t) i >> 8) & 0x00ff;
	debugbuf1[3] = (uint8_t)((uint32_t) i >> 16) & 0x00ff;
	debugbuf1[4] = (uint8_t)((uint32_t) i >> 24) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_4BYTE_HEXADECIMALS);
}
static AVRORA_PRINT_INLINE void avroraPrintPtr(void * i)
{
	debugbuf1[1] = (uint8_t)((uint16_t) i) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint16_t) i >> 8) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_2BYTE_HEXADECIMALS);
}
static AVRORA_PRINT_INLINE void avroraPrintR1()
{
	avroraPrintSetVarType(AVRORA_PRINT_R1);
}
static AVRORA_PRINT_INLINE void avroraPrintSP()
{
	avroraPrintSetVarType(AVRORA_PRINT_SP);
}
static AVRORA_PRINT_INLINE void avroraPrintPC()
{
	avroraPrintSetVarType(AVRORA_PRINT_PC);
}
static AVRORA_PRINT_INLINE void avroraPrintRegs()
{
	avroraPrintSetVarType(AVRORA_PRINT_REGS);
}
static AVRORA_PRINT_INLINE void avroraPrintFlashStr(uint32_t s)
{
	debugbuf1[1] = (uint8_t)((uint32_t) s) & 0x00ff;
	debugbuf1[2] = (uint8_t)((uint32_t) s >> 8) & 0x00ff;
	debugbuf1[3] = (uint8_t)((uint32_t) s >> 16) & 0x00ff;
	debugbuf1[4] = (uint8_t)((uint32_t) s >> 24) & 0x00ff;
	avroraPrintSetVarType(AVRORA_PRINT_FLASH_STRING_POINTER);
}
static AVRORA_PRINT_INLINE void avroraPrintPanic(uint8_t code)
{
	debugbuf1[1] = code;
	avroraPrintSetVarType(AVRORA_PRINT_PANIC);
}
static AVRORA_PRINT_INLINE void avroraPrintFreeHeapMemory()
{
	avroraPrintSetVarType(AVRORA_PRINT_FREEHEAPMEMORY);
}
static AVRORA_PRINT_INLINE void avroraPrintDJHeap()
{
	avroraPrintSetVarType(AVRORA_PRINT_DJ_HEAP);
}
#endif
