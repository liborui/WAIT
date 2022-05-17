
#include "asm.h"
#include "rtc_emit.h"
#include "config.h"
#include "debug.h"
// #include "execution.h"
#include <avr/pgmspace.h>
// push pop order
// Ints: Push 1, Push 0     Pop 0, Pop 1 (Ints are stored LE in the registers, so this push order stores them BE in memory since int stack grows down. This is what DJ expects)
// Refs: Push 0, Push 1     Pop 1, Pop 0

void emit_x_CALL(uint16_t target) {
    // Flush the code buffer before emitting a CALL to prevent PUSH/POP pairs being optimised across a CALL instruction.
    emit_PUSH(RXH);
    emit_PUSH(RXL);
    emit_flush_to_flash();
    emit_2_CALL(target);
    emit_POP(RXL);
    emit_POP(RXH);
}

void emit_x_PUSH_32bit(uint8_t base) {
    emit_PUSH(base+3);
    emit_PUSH(base+2);
    emit_PUSH(base+1);
    emit_PUSH(base+0);
}
void emit_x_PUSH_16bit(uint8_t base) {
    emit_PUSH(base+1);
    emit_PUSH(base+0);
}
void emit_x_PUSH_REF(uint8_t base) {
    emit_x_PUSHREF8(base+0);
    emit_x_PUSHREF8(base+1);
}

void emit_x_POP_32bit(uint8_t base) {
    emit_POP(base+0);
    emit_POP(base+1);
    emit_POP(base+2);
    emit_POP(base+3);
}
void emit_x_POP_16bit(uint8_t base) {
    emit_POP(base+0);
    emit_POP(base+1);
}
void emit_x_POP_REF(uint8_t base) {
    emit_x_POPREF8(base+1);
    emit_x_POPREF8(base+0);
}

u32 udiv(u32 input1,u32 input2){
    return input1/input2;
}

i32 idiv(i32 input1,i32 input2){
    return input1/input2;
}

u32 umul(u32 input1,u32 input2){
    return input2*input1;
}

u32 LeadingZeros_32(u32 x)
{
    //do the smearing
    x |= x >> 1; 
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    //count the ones
    x -= x >> 1 & 0x55555555;
    x = (x >> 2 & 0x33333333) + (x & 0x33333333);
    x = (x >> 4) + x & 0x0f0f0f0f;
    x += x >> 8;
    x += x >> 16;
    return 32 - (x & 0x0000003f); //subtract # of 1s from 32
}
u16 __attribute__((section (".wait"))) lowest_stack=0x10ff;
void print_stack(){
    u8* sp = STACK_POINTER();
    // printf("----stack----\n");
    // for(int i=20;i>=11;i--){
    //     printf("%04p: %02x\n",(sp+i),*(sp+i));
    // }
    // printf("-------------\n");
    if(sp+2<=lowest_stack){
        lowest_stack = sp+2;
    }
}

u32 __attribute__((section (".wait"))) global_counter=0;
void count_func(){
    global_counter++;
}

// void emit_x_avroraBeep(uint8_t beep) {
//     emit_PUSH(R24);
//     emit_LDI(R24, beep);
//     emit_2_STS((uint16_t)&rtcMonitorVariable+1, R24);
//     emit_LDI(R24, AVRORA_RTC_BEEP);
//     emit_2_STS((uint16_t)&rtcMonitorVariable, R24);
//     emit_POP(R24);
// }
// void emit_x_avroraPrintPC() {
//     emit_PUSH(R24);
//     emit_LDI(R24, 0x12);
//     emit_2_STS((uint16_t)&(debugbuf1), R24);
//     emit_POP(R24);
// }
// void emit_x_avroraPrintRegs() {
//     emit_PUSH(R30);
//     emit_LDI(R30, 0xE);
//     emit_2_STS((uint16_t)&(debugbuf1), R30);
//     emit_POP(R30);
// }

// This needs to be a #define to calculate the instruction at compile time.
// There is also a asm_opcodeWithSingleRegOperand, which is a function. This
// needs to be a function to save programme size, because expanding this macro
// with variable parameters will be much larger than the function calls.
#define asm_const_opcodeWithSingleRegOperand(opcode, reg) (((opcode) + ((reg) << 4)))

// NOTE THAT THIS CODE ONLY WORKS ON AVR DEVICES WITH <=128KB flash.
// On larger devices, such as the WuDevice, the return address after
// a call is 24 bit, so we need to pop/push 3 bytes at the beginning
// and end of the fragments below.
// (POP R18; POP19 -> POP R18; POP R19; POP R20)
void emit_x_preinvoke_code() {

    asm volatile("       pop  r18" "\n\r"                   
                 "       pop  r19" "\n\r"
                                                                          // set intStack to SP 
                 "       push r1" "\n\r"                                  // NOTE: THE DVM STACK IS A 16 BIT POINTER, SP IS 8 BIT.
                                                                          // BOTH POINT TO THE NEXT free SLOT, BUT SINCE THEY GROW down THIS MEANS THE DVM POINTER SHOULD POINT TO TWO BYTES BELOW THE LAST VALUE,
                                                                          // WHILE CURRENTLY THE NATIVE SP POINTS TO THE BYTE DIRECTLY BELOW IT. RESERVE AN EXTRA BYTE TO FIX THIS.
                 "       in   r0, %[_SP_L_port]" "\n\r"                   // Load SPL into R0
                 "       sts  intStack, r0" "\n\r"                        // Store R0 into intStackL
                 "       in   r0, %[_SP_H_port]" "\n\r"                   // Load SPH into R0
                 "       sts  intStack+1, r0" "\n\r"                      // Store R0 into intStackH
                                                                          // Reserve 8 bytes of space on the stack, in case the returned int is large than passed ints
                                                                          // TODO: make this more efficient by looking up the method, and seeing if the return type is int,
                                                                          //       and if so, if the size of the return type is larger than the integers passed. Then only
                                                                          //       reserve the space that's needed.
                                                                          //       This is for the worst case, where no ints are passed, so there's no space reserved, and
                                                                          //       a 32 bit long is returned.
                 "       rcall next1" "\n\r"                              // RCALL to offset 0 does nothing, except reserving 2 bytes on the stack. cheaper than two useless pushes.
                 "next1: rcall next2" "\n\r"
                                                                          // Pre possible GC: need to store X in refStack: for INVOKEs to pass the references, for other cases just to make sure the GC will update the pointer if it runs.
                 "next2: sts  refStack, r26" "\n\r"
                 "       sts  refStack+1, r27" "\n\r"
                 "       push r19" "\n\r"
                 "       push r18" "\n\r"
             :: [_SP_L_port] "M" (SP_L_port), [_SP_H_port] "M" (SP_H_port));
}

void emit_x_preinvoke() {
    emit_2_CALL((uint16_t)&emit_x_preinvoke_code);
}

void emit_x_postinvoke_code() {

    asm volatile("       pop  r18" "\n\r"                   
                 "       pop  r19" "\n\r"

#ifndef EXECUTION_FRAME_ON_STACK
                                                                          // Y is call-saved and won't move if the frame is on the stack
                                                                          // Post possible GC: need to reset Y to the start of the stack frame's local references (the frame may have moved, so the old value may not be correct)
                 "       lds  r28, localReferenceVariables" "\n\r"        // Load localReferenceVariables into Y
                 "       lds  r29, localReferenceVariables+1" "\n\r"      // Load localReferenceVariables into Y
#endif
                                                                          // Post possible GC: need to restore X to refStack which may have changed either because of GC or because of passed/returned references
                 "       lds  r26, refStack" "\n\r"                       // Load refStack into X
                 "       lds  r27, refStack+1" "\n\r"                     // Load refStack into X
                                                                          // get SP from intStack
                 "       lds  r0, intStack" "\n\r"                        // Load intStackL into R0
                 "       out  %[_SP_L_port], r0" "\n\r"                   // Store R0 into SPL
                 "       lds  r0, intStack+1" "\n\r"                      // Load intStackL into R0
                 "       out  %[_SP_H_port], r0" "\n\r"                   // Store R0 into SPH
                 "       pop  r0" "\n\r"                                  // JUST POP AND DISCARD TO CLEAR THE BYTE WE RESERVED IN THE asm_const_PUSH(ZERO_REG) LINE IN PREINVOKE
                 "       push r19" "\n\r"
                 "       push r18" "\n\r"
             :: [_SP_L_port] "M" (SP_L_port), [_SP_H_port] "M" (SP_H_port));
}

void emit_x_postinvoke() {
    emit_2_CALL((uint16_t)&emit_x_postinvoke_code);
}

void emit_x_call_save(){
    for(int i=2;i<18;i++){
        emit_PUSH(R0+i);  
    }
}
void emit_x_call_restore(){
    for(int i=17;i>=2;i--){
        emit_POP(R0+i);  
    }
}
// 6 bit offset q has to be inserted in the opcode like this:
// 00q0 qq00 0000 0qqq
#define makeLDDSTDoffset(offset) ( \
               ((offset) & 0x07) \
            + (((offset) & 0x18) << 7) \
            + (((offset) & 0x20) << 8))
// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_LDD(uint8_t reg, uint8_t yz, uint16_t offset) {
    emit (OPCODE_LDD
             + ((reg) << 4)
             + ((yz) << 3)
             + makeLDDSTDoffset(offset));
}
// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
void emit_STD(uint8_t reg, uint8_t yz, uint16_t offset) {
    emit ((OPCODE_STD
             + ((reg) << 4)
             + ((yz) << 3)
             + makeLDDSTDoffset(offset)));
}
void emit_LDI_SBCI_SUBI_CPI(uint16_t opcode, uint8_t reg, uint8_t constant) {
    uint16_t encoded_constant = (constant & 0x0F) + ((constant & 0xF0) << 4); // 0000 KKKK 0000 KKKK
    emit (opcode
             + (((reg) - 16) << 4)
             + encoded_constant);

}
// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
void emit_ADIW(uint8_t reg, uint8_t constant) {
	emit ((OPCODE_ADIW
			 + ((((reg)-24)/2)<<4)
			 + ((constant) & 0x0F)
             + (((constant) & 0x30) << 2)));
}
// SBIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
void emit_SBIW(uint8_t reg, uint8_t constant) {
    emit ((OPCODE_SBIW
             + ((((reg)-24)/2)<<4)
             + ((constant) & 0x0F)
             + (((constant) & 0x30) << 2)));
}
//                                      0000 00kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
void emit_BRANCH(uint16_t opcode, uint8_t offset) {
    emit (opcode + makeBranchOffset(((offset)/2)));
}
//                                      0000 000d dddd 0000
uint16_t asm_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	return (((opcode) + ((reg) << 4)));
}
void emit_opcodeWithSingleRegOperand(uint16_t opcode, uint8_t reg) {
	emit (asm_opcodeWithSingleRegOperand(opcode, reg));
}
//                                      0000 00rd dddd rrrr, with d=dest register, r=source register
uint16_t asm_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
    return (((opcode) + ((destreg) << 4) + makeSourceRegister(srcreg)));
}
void emit_opcodeWithSrcAndDestRegOperand(uint16_t opcode, uint8_t destreg, uint8_t srcreg) {
    emit (asm_opcodeWithSrcAndDestRegOperand(opcode, destreg, srcreg));
}

// 初始化局部变量
void emit_local_init(u16 numLocalBytes){
    // 先保存28 29寄存器
    emit_PUSH(R28);
    emit_PUSH(R29);
    // 载入栈指针
    emit_IN(R28,0x3d);
    emit_IN(R29,0x3e);

    // 如果局部变量区过大，需要使用SUB指令进行计算
    if(numLocalBytes>=60){
        emit_LDI(RZL,(numLocalBytes&0x00ff));
        emit_LDI(RZH,(numLocalBytes&0xff00));
        emit_SUB(RYL,RZL);
        emit_SBC(RYH,RZH);
    }else{
        // 否则只需要SBIW直接计算
        emit_SBIW(R28,numLocalBytes);
    }
    
    // R29需要在关中断的情况下进行修改(目前不涉及多线程和中断)
    // emit_IN(R0,0x3f);
    // emit_CLI();
    emit_OUT(0x3e,R29);
    // emit_OUT(0x3f,R0);
    emit_OUT(0x3d,R28);
}
// 释放局部变量区（反向操作）
void emit_local_deinit(u16 numLocalBytes){
 
    if(numLocalBytes>=60){
        emit_LDI(RZL,(numLocalBytes&0x00ff));
        emit_LDI(RZH,(numLocalBytes&0xff00));
        emit_ADD(RYL,RZL);
        emit_ADC(RYH,RZH);
    }else{
        emit_ADIW(R28,numLocalBytes);
    }
    

    // R29需要在关中断的情况下进行修改(目前不涉及多线程和中断)
    // emit_IN(R0,0x3f);
    // emit_CLI();
    emit_OUT(0x3e,R29);
    // emit_OUT(0x3f,R0);
    emit_OUT(0x3d,R28);
    emit_POP(R29);
    emit_POP(R28);
}
void emit_MOVW(uint8_t destreg, uint8_t srcreg) {
    emit_opcodeWithSrcAndDestRegOperand(OPCODE_MOVW, (destreg/2), (srcreg/2));
}
// void emit_set_SP(){
//     emit_OUT()
// }
// 前置流程
//                              push	r28
//     2d34:	df 93       	push	r29
//     2d36:	cd b7       	in	r28, 0x3d	; 61
//     2d38:	de b7       	in	r29, 0x3e	; 62
//     2d3a:	2c 97       	sbiw	r28, 0x0c	; 12
//     2d3c:	0f b6       	in	r0, 0x3f	; 63
//     2d3e:	f8 94       	cli
//     2d40:	de bf       	out	0x3e, r29	; 62
//     2d42:	0f be       	out	0x3f, r0	; 63
//     2d44:	cd bf       	out	0x3d, r28	; 61
// 后置流程
    // 2eb4:	2c 96       	adiw	r28, 0x0c	; 12
    // 2eb6:	0f b6       	in	r0, 0x3f	; 63
    // 2eb8:	f8 94       	cli
    // 2eba:	de bf       	out	0x3e, r29	; 62
    // 2ebc:	0f be       	out	0x3f, r0	; 63
    // 2ebe:	cd bf       	out	0x3d, r28	; 61
    // 2ec0:	df 91       	pop	r29
    // 2ec2:	cf 91       	pop	r28
// void emit_save_Y(){
//     emit_PUSH(R28);
//     emit_PUSH(R29);
// }
// void emit_restore_Y(){
//     emit_POP(R29);
//     emit_POP(R28);
// }
// void emit_init_Y(){
//     emit_IN(R28,0x3d);
//     emit_IN(R29,0x3e);
// }