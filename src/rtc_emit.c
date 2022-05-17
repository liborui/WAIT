#include <stdlib.h>
#include "debug.h"
#include "wkreprog.h"
#include "rtc_emit.h"
#include "types.h"
#include "compile.h"


//9F 93 8F 93 7F 93 6F 93 6F 91 7F 91 8F 91 9F 91
void rtc_optimise(u16* buffer, u16** code_end){
    int optimized=0;
    int buffer_len = *code_end-buffer;

    if(buffer_len<8){
        return;
    }

    // log(emit,"before opti");
    // hexdump(buffer,buffer_len*2);
    for(int i=0;i<buffer_len-8;i++){
        if(buffer[i]==0x939f&&\
        buffer[i+1]==0x938f&&\
        buffer[i+2]==0x937f&&\
        buffer[i+3]==0x936f&&\
        buffer[i+4]==0x916f&&\
        buffer[i+5]==0x917f&&\
        buffer[i+6]==0x918f&&\
        buffer[i+7]==0x919f
        ){
            
            // log(emit,"push/pop optimize blocks:%d",blct.next_id);
            // log(emit,"memcpy from %p",buffer+i+8);
            // log(emit,"to %p",buffer+i);
            // log(emit,"size %d",buffer_len*2-(i+8)*2);
            memcpy(buffer+i,buffer+i+8,buffer_len*2-(i+8)*2);
            
            //修改PC记录
            // log(emit,"buffer:%p",buffer);
            // log(emit,"buffer end:%p",*code_end);
            
            int curr_pc = ts.pc-buffer_len*2+i*2;
            // log(emit,"pc:%d",ts.pc);
            // log(emit,"curr pc:%d",curr_pc);

            for(int j=0;j<blct.next_id;j++){
                if(blct.block_pc[j]>curr_pc){
                    // log(emit,"block %d",j);
                    // log(emit,"pc :%d",blct.block_pc[j]);
                    blct.block_pc[j]-=16;
                    // log(emit,"becomes :%d",blct.block_pc[j]);
                }
            }
            *code_end-=8;
            ts.pc -=16;
            buffer_len = *code_end-buffer;
            log(emit,"push/pop optimized");
        }
    }
    // log(emit,"after opti");
    // hexdump(buffer,buffer_len*2);

}

u16 *code_buffer,*codebuffer_position;

void emit_init(u16* buffer) {
    code_buffer= buffer;
    codebuffer_position = code_buffer;
}


void emit_raw_word(u16 word) {
    *(codebuffer_position++) = word;
    ts.pc+=2;

    if (codebuffer_position >= code_buffer+RTC_CODEBUFFER_SIZE) { // Buffer full, do a flush.
        emit_flush_to_flash();
    }
}

u16 emit_get_relative_position(){
    return codebuffer_position-code_buffer;
}

void emit(u16 opcode) {
#ifdef AVRORA
    // avroraRTCTraceSingleWordInstruction(opcode);
#endif
    emit_raw_word(opcode);
}

void emit2(u16 opcode1, u16 opcode2) {
#ifdef AVRORA
    // avroraRTCTraceDoubleWordInstruction(opcode1, opcode2);
#endif
    emit_raw_word(opcode1);
    emit_raw_word(opcode2);
}

void emit_without_optimisation(u16 word) {
    emit_flush_to_flash();
    *(codebuffer_position++) = word;
    emit_flush_to_flash();
}

void emit_flush_to_flash() {
    if (code_buffer != codebuffer_position) {
        // Try to optimise the code currently in the buffer. This may affect codebuffer_position if we're able to compact the code.
        rtc_optimise(code_buffer, &codebuffer_position);

        u8 *instructiondata = (u8 *)code_buffer;
        u16 count = codebuffer_position - code_buffer;

        // for (int i=0; i<count; i++) {
        //     log(temp, "[instruction] %04X",code_buffer[i]);
        // }


        // Write to flash buffer
        wkreprog_write(2*count, instructiondata);
        // Buffer is now empty
        codebuffer_position = code_buffer;
    }
}

