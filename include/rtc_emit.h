#ifndef RTC_EMIT_H
#define RTC_EMIT_H

#include "types.h"

#define RTC_CODEBUFFER_SIZE 96

void emit_init(u16* buffer);

void emit(u16 opcode);
void emit2(u16 opcode1, u16 opcode2);
void emit_raw_word(u16 word);
void emit_without_optimisation(u16 word);

void emit_flush_to_flash();

#endif // RTC_EMIT_H