#ifndef _AVRORATRACE_H_
#define _AVRORATRACE_H_

#ifdef __GNUC__
# define AVRORA_PRINT_INLINE __inline__
#else
/* Try the C99 keyword instead. */
# define AVRORA_PRINT_INLINE inline
#endif
#include"types.h"
volatile u8 __attribute__((section (".wait"))) avroraTraceEnabledVariable;

static AVRORA_PRINT_INLINE void avroraTraceEnable()
{
	avroraTraceEnabledVariable = 1;
}

static AVRORA_PRINT_INLINE void avroraTraceDisable()
{
	avroraTraceEnabledVariable = 0;
}

#endif
