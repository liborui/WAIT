#include "instructions.h"
#include "debug.h"
#include "asm.h"
#include "utils.h"
#include "compile.h"
#include "wkreprog_impl.h"
#include "safety_check.h"
#include <stdlib.h>
#include <avr/pgmspace.h>

// NOTE: Function pointers are a "PC address", so already divided by 2 since the PC counts in words, not bytes.
// avr-libgcc functions used by translation
// qi = quarter of integer (8 bits)
// hi = half integer (16 bits)
// si = single integer (32 bits)
// di = double integer (64 bits)
// https://github.com/gcc-mirror/gcc/blob/master/libgcc/config/avr/lib1funcs.S
// extern void __divmodhi4(void);
// extern void __mulsi3(void);
// extern void __mulhisi3(void);
// extern void __divmodsi4(void);



extern u16 __brkval;
extern u16 malloc_record;
void emit_single_instruction(wasm_module_ptr module, wasm_function_ptr func, bytes *start, bytes end)
{
    u8 op = pgm_read_byte_far(*start);
    (*start)++;
    // u32 temp;
    union
    {
        u64 num64;
        u32 num32[2];
        u16 num16[4];
        u8 num8[8];
    } operand;

    u16 base;
#if count_all_insn
    emit_2_CALL(embed_func_count);
#endif
    switch (op)
    {
    // 0x00 Unreachable
    case Unreachable:
    {
        emit_CLI();
        emit_RJMP(-2);
        // ts.pc += 4;
        break;
    }
    // 0x01 Nop
    case Nop:
    {
        // DEBUG
        emit_2_CALL(embed_func_print_stack);
        break;
    }
    // 0x02 Block
    case Block:
    {
        // 读取返回值类型
        ReadLEB_i7(&operand, start, end);
        NormalizeType(&operand.num8[1], operand.num8[0]);
        log(emit, "[BLOCK %s]", wasm_types_names[operand.num8[1]]);
        // 生成标签ID并入栈
        blct.block_label[blct.top++] = blct.next_id++;
        blct.block_stack[blct.top - 1] = ts.stack_top;
        blct.block_type[blct.top - 1] = operand.num8[1];
        blct.block_pc[blct.next_id - 1] = 0; //表示非loop
        break;
    }
    // 0x03 Loop
    case Loop:
    {
        // 读取返回值类型
        ReadLEB_i7(&operand, start, end);
        NormalizeType(&operand.num8[1], operand.num8[0]);
        log(emit, "[LOOP %s]", wasm_types_names[operand.num8[1]]);
        // 生成标签ID并入栈
        blct.block_label[blct.top++] = blct.next_id++;
        blct.block_stack[blct.top - 1] = ts.stack_top;
        blct.block_type[blct.top - 1] = operand.num8[1];
        blct.block_pc[blct.next_id - 1] = ts.pc; //loop的跳转目标为loop起始
        break;
    }
    // 0x04 If
    // 0x05 Else_
    // 0x0B End_
    case End_:
    {
        log(emit, "[END]:%p", RTC_START_OF_COMPILED_CODE_SPACE + ts.pc);
        if (blct.top) // 还在block内部
        {
            log(emit, "of block %d", blct.top);

            // 结束前恢复栈平衡
            int count = ts.stack_top - blct.block_stack[--blct.top];
            for (int i = 0; i < count; i++)
            {
                log(emit, "pop stack");
                if (i == 0)
                {
                    emit_x_POP_32bit(R22);
                }
                else
                {
                    emit_x_POP_32bit(R18);
                }
                ts.stack_top--;
                // ts.pc += 8;
            }
            if (!blct.block_pc[blct.block_label[blct.top]])
            {
                blct.block_pc[blct.block_label[blct.top]] = ts.pc; //没有填充PC（非loop）则填充END的PC
            }
            if (blct.block_type[blct.top] != WASM_Type_none)
            {
                log(emit, "push res %s", wasm_types_names[blct.block_type[blct.top]]);
                emit_x_PUSH_32bit(R22);
                // ts.pc += 8;
                ts.stack_top++;
                log(emit, "s: %d", ts.stack_top);
            }
        }
        else // 到达函数尾部的END
        {
            log(emit, "func end");
            // 将返回值弹出到对应寄存器
            if (func->funcType->returnType == WASM_Type_i32)
            {
                log(emit, "pop32 for result %s", func->funcType->returnType);
                emit_x_POP_32bit(R22);
                ts.stack_top--;
            }
            // 如果函数有局部变量，则恢复Y指针
            if (func->numLocals || func->funcType->args_num)
            {
                log(emit, "deinit %dB locals", func->numLocalBytes);
                emit_local_deinit(func->numLocalBytes);
            }
            if (is_entry_func(module, func))
            { //在入口函数的尾部恢复状态
                emit_x_call_restore();
            }
            log(emit, "ret");
            //DEBUG RET前返回地址打印
            // emit_x_POP_16bit(R22);
            // emit_x_PUSH_16bit(R22);
            // emit_EOR(R24,R24);
            // emit_EOR(R25,R25);
            // emit_2_CALL(4);//需要替换为当前wasm的print函数序号
            emit_RET();
        }
        break;
    }
    // 0x0C Br
    case Br:
    {
        ReadLEB_u32(&operand, start, end);
        log(emit, "[BR %d]", operand.num32[0]);
        log(emit, "%d", ts.pc);
        // 跳转前恢复栈平衡
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 1)
        {
            emit_x_POP_32bit(R22);
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 2)
        {
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
        }
        else if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] > 2)
        {
            emit_x_POP_32bit(R22);
            emit_IN(R26, 0x3d);
            emit_IN(R27, 0x3e);
            emit_ADIW(R26, (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] - 1) * 4);
            emit_OUT(0x3e, R27);
            emit_OUT(0x3d, R26);
        }
        emit_2_JMP((blct.block_label[blct.top - 1 - operand.num16[0]]) * 2); //填充对应Block的ID(emit宏会将地址除以2)
        break;
    }
    // 0x0D BrIf
    case BrIf: //TODO
    {
        ReadLEB_u32(&operand, start, end);
        log(emit, "[BR_IF %d]", operand.num32[0]);

        // 获取操作数
        emit_x_POP_32bit(R22);
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        // 与0比较
        emit_CP(R22, R1);
        emit_CPC(R23, R1);
        emit_CPC(R24, R1);
        emit_CPC(R25, R1);
        // 非0则跳转

        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 0)
        {
            base = 4;
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 1)
        {
            base = 4 + 8; //12
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 2)
        {
            base = 4 + 8 * 2; //20
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] > 2)
        {
            base = 4 + 8 + 10; //22
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] < 0)
        {
            panicf("stack not balanced!");
        }
        log(emit, "br %d", 4 + 8 * (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]]));
        emit_BREQ(base);

        // 跳转前恢复栈平衡
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 1)
        {
            emit_x_POP_32bit(R22);
        }
        if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] == 2)
        {
            emit_x_POP_32bit(R22);
            emit_x_POP_32bit(R18);
        }
        else if (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] > 2)
        {
            emit_x_POP_32bit(R22);
            emit_IN(R26, 0x3d);
            emit_IN(R27, 0x3e);
            emit_ADIW(R26, (ts.stack_top - blct.block_stack[blct.top - 1 - operand.num16[0]] - 1) * 4);
            emit_OUT(0x3e, R27);
            emit_OUT(0x3d, R26);
        }
        emit_2_JMP((blct.block_label[blct.top - 1 - operand.num16[0]]) * 2); //填充对应Block的ID(emit宏会将地址除以2)
        // ts.pc += 4;
        break;
    }
    // 0x0E BrTable
    // case BrTable: //TODO
    // {
    //     break;
    // }
    // 0x0E Return
    case Return:
    {
        log(emit, "[RET]");
        // 将返回值弹出到对应寄存器
        if (func->funcType->returnType == WASM_Type_i32)
        {
            log(emit, "pop32 for result %s", func->funcType->returnType);
            emit_x_POP_32bit(R22);
            ts.stack_top--;
        }
        // 如果函数有局部变量，则恢复Y指针
        if (func->numLocals || func->funcType->args_num)
        {
            log(emit, "deinit %dB locals", func->numLocalBytes);
            emit_local_deinit(func->numLocalBytes);
        }
        if (is_entry_func(module, func))
        { //在入口函数的尾部恢复状态
            emit_x_call_restore();
        }
        log(emit, "ret");
        emit_RET();
        break;
    }
    // 0x10 Call
    case Call:
    {
        // DEBUG
        // emit_2_CALL(embed_func_print_stack);
        // log(emit, "emiting CALL instruction...");
        ReadLEB_u32(&operand, start, end);
        log(emit, "[CALL %d]", operand.num32[0]);
        if (operand.num16[0] >= module->function_num)
        {
            panicf("called index out of scope");
        }
        wasm_function_ptr called_func = module->function_list[operand.num16[0]];
        if (operand.num16[0]<module->import_num)
        {
            func_type_ptr type = called_func->funcType;
            //先检查是否需要通过内存（栈）传参
            base = R26;
            bool need_memory_pass = false;
            for (int i = 0; i < type->args_num; i++)
            {
                if (type->args_type_list[i] == WASM_Type_i32)
                {
                    base -= 4;
                    if (base < R8)
                    {
                        need_memory_pass = true;
                    }
                }
            }

            //TODO 如果需要内存传参，则空出R2、3、4、5、6、7、26、27用于暂存参数(可以考虑全局暂存区方案，因为栈上有参数)
            if (need_memory_pass)
            {
            }

            //开始放置参数 TODO只做了32位参数，后续增加64位
            for (int i = 0; i < type->args_num; i++)
            {
                if (type->args_type_list[i] == WASM_Type_i32)
                {
                    logif(emit, printf("pop32 for param %d at", i); printf(" R%d.", base););
                    emit_x_POP_32bit(base);
                    base += 4;
                    // ts.pc += 8;
                    ts.stack_top--;
                }
            }
            //TODO 恢复R2345
            if (need_memory_pass)
            {
            }

            log(emit, "call func %s", called_func->name);
            emit_2_CALL(operand.num16[0]);
            // ts.pc += 4;

            //TODO 抛弃栈传递的参数
            if (need_memory_pass)
            {
            }

            //返回值压栈
            if (type->returnType != WASM_Type_none)
            {
                if (type->returnType == WASM_Type_i32)
                {
                    log(emit, "push32 for return type %s", wasm_types_names[WASM_Type_i32]);
                    emit_x_PUSH_32bit(R22);
                    ts.stack_top++;
                }
            }
        }
        else
        {
            func_type_ptr type = called_func->funcType;
            log(emit, "call func %s", called_func->name);
            emit_2_CALL(operand.num16[0]);
            // ts.pc += 4;
            //drop参数
            for (int i = 0; i < type->args_num; i++)
            {
                log(emit, "drop32");
                emit_x_POP_32bit(R18);
                // ts.pc += 8;
                ts.stack_top--;
            }
            // 结果压栈
            if (type->returnType == WASM_Type_i32)
            {
                log(emit, "push32 for return type %s", wasm_types_names[WASM_Type_i32]);
                emit_x_PUSH_32bit(R22);
                ts.stack_top++;
                // ts.pc += 8;
            }
            log(emit, "s: %d", ts.stack_top);
        }
        break;
    }
    // 0x11 CallIndirect
    // 0x1A Drop
    case Drop:
    {
        log(emit, "[DROP]");
        log(emit, "pop32");
        emit_x_POP_32bit(R22);
        // ts.pc += 8;
        ts.stack_top--;
        break;
    }
    // 0x1B Select
    case Select:
    {
        // 比较数
        emit_x_POP_32bit(R22);
        // pop第一个数
        emit_x_POP_32bit(R18);
        emit_OR(R22, R23);
        emit_OR(R22, R24);
        emit_OR(R22, R25);
        emit_BRNE(16);
        //=0 pop第二个数，并push第一个
        emit_x_POP_32bit(R22);
        emit_x_PUSH_32bit(R18);
        //!=0 pop一个，不push
        // ts.pc += 40;
        ts.stack_top -= 2;
        break;
    }
    // 0x20 LocalGet
    case LocalGet:
    {
        #if count_global_local
        emit_2_CALL(embed_func_count);
        #endif
        //读取局部变量索引
        ReadLEB_u32(&operand, start, end);
        log(emit, "[LOCAL.GET %d]", operand.num32[0]);

        if (operand.num16[0] >= ts.current_func->funcType->args_num)
        { //本地变量
            base = operand.num16[0] - ts.current_func->funcType->args_num;
            base *= 4;
            log(emit, "Ld R22. from local[%d]", base);
        }
        else
        { //参数
            base = ts.current_func->funcType->args_num - 1 - operand.num16[0];
            base = ts.current_func->numLocalBytes + 4 + base * 4;
            log(emit, "Ld R22. from param[%d]", base);
        }
        if(base>=60){
            emit_LDI(RZL,base&0xff);
            emit_LDI(RZH,base&0xff00);
            emit_ADD(RYL,RZL);
            emit_ADC(RYH,RZH);
            emit_LDD(R22, Y, 1);
            emit_LDD(R23, Y, 2);
            emit_LDD(R24, Y, 3);
            emit_LDD(R25, Y, 4);
            emit_SUB(RYL,RZL);
            emit_SBC(RYH,RZH);
        }else{
            emit_LDD(R22, Y, base + 1);
            emit_LDD(R23, Y, base + 2);
            emit_LDD(R24, Y, base + 3);
            emit_LDD(R25, Y, base + 4);
        }
        log(emit, "push32");
        emit_x_PUSH_32bit(R22);

        ts.stack_top++;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x21 LocalSet
    case LocalSet:
    {
        #if count_global_local
        emit_2_CALL(embed_func_count);
        #endif
        //读取局部变量索引
        ReadLEB_u32(&operand, start, end);
        log(emit, "[LOCAL.SET %d]", operand.num32[0]);
        log(emit, "pop32 to R22...");
        emit_x_POP_32bit(R22);

        if (operand.num16[0] >= ts.current_func->funcType->args_num)
        { //本地变量
            //这里有奇怪的bug，u32 无法参与加法计算，因此用u16的base变量来计算
            base = operand.num16[0] - ts.current_func->funcType->args_num;
            base *= 4;
            log(emit, "Store R22... to local[%d]", base);
        }
        else
        {
            base = ts.current_func->funcType->args_num - 1 - operand.num16[0];
            base = ts.current_func->numLocalBytes + 4 + base * 4;
            log(emit, "Store R22... to param[%d]", base);
        }
        if(base>=60){
            emit_LDI(RZL,base&0xff);
            emit_LDI(RZH,base&0xff00);
            emit_ADD(RYL,RZL);
            emit_ADC(RYH,RZH);
            emit_STD(R22, Y, 1);
            emit_STD(R23, Y, 2);
            emit_STD(R24, Y, 3);
            emit_STD(R25, Y, 4);
            emit_SUB(RYL,RZL);
            emit_SBC(RYH,RZH);
        }else{
            emit_STD(R22, Y, base + 1);
            emit_STD(R23, Y, base + 2);
            emit_STD(R24, Y, base + 3);
            emit_STD(R25, Y, base + 4);
        }
        // ts.pc += 16;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x22 LocalTee
    case LocalTee:
    {
        #if count_global_local
        emit_2_CALL(embed_func_count);
        #endif
        //读取局部变量索引
        ReadLEB_u32(&operand, start, end);
        log(emit, "[LOCAL.TEE %d]", operand.num32[0]);
        log(emit, "pop32 to R22...");
        emit_x_POP_32bit(R22);
        if (operand.num16[0] >= ts.current_func->funcType->args_num)
        { //本地变量
            //这里有奇怪的bug，u32 无法参与加法计算，因此用u16的base变量来计算
            base = operand.num16[0] - ts.current_func->funcType->args_num;
            base *= 4;
            log(emit, "Store R22... to local[%d]", base);
        }
        else
        {
            base = ts.current_func->funcType->args_num - 1 - operand.num16[0];
            base = ts.current_func->numLocalBytes + 4 + base * 4;
            log(emit, "Store R22... to param[%d]", base);
        }
        if(base>=60){
            emit_LDI(RZL,base&0xff);
            emit_LDI(RZH,base&0xff00);
            emit_ADD(RYL,RZL);
            emit_ADC(RYH,RZH);
            emit_STD(R22, Y, 1);
            emit_STD(R23, Y, 2);
            emit_STD(R24, Y, 3);
            emit_STD(R25, Y, 4);
            emit_SUB(RYL,RZL);
            emit_SBC(RYH,RZH);
        }else{
            emit_STD(R22, Y, base + 1);
            emit_STD(R23, Y, base + 2);
            emit_STD(R24, Y, base + 3);
            emit_STD(R25, Y, base + 4);
        }
        // 比Set多一个Push操作
        log(emit, "push32");
        emit_x_PUSH_32bit(R22);
        // ts.pc += 24;
        break;
    }
    // 0x23 GlobalGet
    case GlobalGet:
    {
        #if count_global_local
        emit_2_CALL(embed_func_count);
        #endif
        //读取全局变量索引
        ReadLEB_u32(&operand, start, end);
        log(emit, "[GLOBAL.GET %d]", operand.num32[0]);
        log(emit, "Load R22. from global[%d]", module->global_list[operand.num16[0]].offset);
        emit_MOVW(RZ, R2);
        // 由于STD的偏移量只支持[-64，63]范围内寻址，因此需要进行Z指针的运算
        if (module->global_list[operand.num16[0]].offset >= 60)
        {
            emit_LDI(R22,module->global_list[operand.num16[0]].offset&0x00ff);
            emit_LDI(R23,module->global_list[operand.num16[0]].offset&0xff00);
            emit_ADD(RZL,R22);
            emit_ADC(RZH,R23);
            emit_LDD(R22, Z, 0);
            emit_LDD(R23, Z, 1);
            emit_LDD(R24, Z, 2);
            emit_LDD(R25, Z, 3);
        }else{
            emit_LDD(R22, Z, module->global_list[operand.num16[0]].offset);
            emit_LDD(R23, Z, module->global_list[operand.num16[0]].offset + 1);
            emit_LDD(R24, Z, module->global_list[operand.num16[0]].offset + 2);
            emit_LDD(R25, Z, module->global_list[operand.num16[0]].offset + 3);
        }
        log(emit, "push32");
        emit_x_PUSH_32bit(R22);
        ts.stack_top++;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x24 GlobalSet
    case GlobalSet:
    {
        #if count_global_local
        emit_2_CALL(embed_func_count);
        #endif
        //TODO由于LDD的偏移量只支持[-64，63]范围内寻址，因此后续需要替换成Z指针的运算
        ReadLEB_u32(&operand, start, end);
        log(emit, "[GLOBAL.SET %d]", operand.num32[0]);
        log(emit, "pop32 to R22.");
        emit_x_POP_32bit(R22);
        // ts.pc += 8;
        log(emit, "Store R22. to global[%d]", module->global_list[operand.num16[0]].offset);
        emit_MOVW(RZ, R2);
        // 由于STD的偏移量只支持[-64，63]范围内寻址，因此需要进行Z指针的运算
        if (module->global_list[operand.num16[0]].offset >= 60)
        {
            emit_LDI(R22,module->global_list[operand.num16[0]].offset&0x00ff);
            emit_LDI(R23,module->global_list[operand.num16[0]].offset&0xff00);
            emit_ADD(RZL,R22);
            emit_ADC(RZH,R23);
            emit_STD(R22, Z, 0);
            emit_STD(R23, Z, 1);
            emit_STD(R24, Z, 2);
            emit_STD(R25, Z, 3);
        }
        else
        {
            emit_STD(R22, Z, module->global_list[operand.num16[0]].offset);
            emit_STD(R23, Z, module->global_list[operand.num16[0]].offset + 1);
            emit_STD(R24, Z, module->global_list[operand.num16[0]].offset + 2);
            emit_STD(R25, Z, module->global_list[operand.num16[0]].offset + 3);
        }
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }

    // 0x28 I32Load
    // 0x2C i32.load8_s
    // 0x2D i32.load8_u
    // 0x2E i32.load16_s
    // 0x2F i32.load16_u
    case I32Load:
    case I32Load8U:
    case I32Load16U:
    case I32Load8S:
    case I32Load16S:
    {
#if count_mem_acc
        emit_2_CALL(embed_func_count);
#endif
        // 读取对齐标签，丢弃
        ReadLEB_u32(&operand, start, end);
        // 读取offset1
        ReadLEB_u32(&operand, start, end);
        log(emit, "[I32.Load %d]", operand.num32[0]);

        // DEBUG
        // emit_2_CALL(embed_func_print_stack);
        // emit_LDI(R24,1);
        // emit_2_STS(0x227,R24);
#if check_memory_bound
        emit_LDI(R24, operand.num8[0]);
        emit_LDI(R25, operand.num8[1]);
        emit_x_POP_32bit(R20);
        emit_2_CALL(embed_func_i32load);
        if (op == I32Load8U || op == I32Load16U)
        {
            if (op == I32Load8U)
                emit_EOR(R23, R23);
            emit_EOR(R24, R24);
            emit_EOR(R25, R25);
        }
        else if (op == I32Load8S || op == I32Load16S)
        {
            if (op == I32Load8S)
            {
                emit_SUBI(R22, 0);
                emit_BRMI(8);
                emit_LDI(R23, 0x00);
                emit_LDI(R24, 0x00);
                emit_LDI(R25, 0x00);
                emit_RJMP(6);
                emit_LDI(R23, 0xFF);
                emit_LDI(R24, 0xFF);
                emit_LDI(R25, 0xFF);
            }
            else
            {
                emit_CP(R22, R1);
                emit_CPC(R23, R1);
                emit_BRMI(6);
                emit_LDI(R24, 0x00);
                emit_LDI(R25, 0x00);
                emit_RJMP(4);
                emit_LDI(R24, 0xFF);
                emit_LDI(R25, 0xFF);
            }
        }

#else
        operand.num16[0] += ts.wasm_globals_size;
        //读取offset2到R22.
        log(emit, "pop32 offset to R22.");
        emit_x_POP_32bit(R22);
        // Z = Z + offset2 由于AVR地址为16位，因此可以略去高16位偏移的计算
        emit_MOVW(RZ, R2);
        emit_ADD(RZL, R22);
        emit_ADC(RZH, R23);
        emit_ADIW(RZ, operand.num16[0]);

        emit_LDD(R22, Z, 0);
        if (op == I32Load)
        {
            emit_LDD(R23, Z, 1);
            emit_LDD(R24, Z, 2);
            emit_LDD(R25, Z, 3);
        }
        if (op == I32Load8U)
        {
            emit_EOR(R23, R23);
            emit_EOR(R24, R24);
            emit_EOR(R25, R25);
        }
        if (op == I32Load8S)
        {
            emit_SUBI(R22, 0);
            emit_BRMI(8);
            emit_LDI(R23, 0x00);
            emit_LDI(R24, 0x00);
            emit_LDI(R25, 0x00);
            emit_RJMP(6);
            emit_LDI(R23, 0xFF);
            emit_LDI(R24, 0xFF);
            emit_LDI(R25, 0xFF);
        }
        if (op == I32Load16U)
        {
            emit_LDD(R23, Z, 1);
            emit_EOR(R24, R24);
            emit_EOR(R25, R25);
        }
        if (op == I32Load16S)
        {
            emit_LDD(R23, Z, 1);
            emit_CP(R22, R1);
            emit_CPC(R23, R1);
            emit_BRMI(6);
            emit_LDI(R24, 0x00);
            emit_LDI(R25, 0x00);
            emit_RJMP(4);
            emit_LDI(R24, 0xFF);
            emit_LDI(R25, 0xFF);
        }
#endif
        // emit_2_STS(0x227,R1);
        emit_x_PUSH_32bit(R22);
        break;
    }
    // 0x29 i64.load
    case I64Load:
    {
#if count_mem_acc
        emit_2_CALL(embed_func_count);
#endif
        // 读取对齐标签，丢弃
        ReadLEB_u32(&operand, start, end);
        // 读取offset1
        ReadLEB_u32(&operand, start, end);
        log(emit, "[I32.Load %d]", operand.num32[0]);

        emit_LDI(R24, operand.num8[0]);
        emit_LDI(R25, operand.num8[1]);
        emit_x_POP_32bit(R20);
        emit_2_CALL(embed_func_i64load);
        emit_x_PUSH_32bit(R18);
        emit_x_PUSH_32bit(R22);
        ts.stack_top += 1;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x2A f32.load
    // 0x2B f64.load
    // 0x30 i64.load8_s
    // 0x31 i64.load8_u
    // 0x32 i64.load16_s
    // 0x33 i64.load16_u
    // 0x34 i64.load32_s
    // 0x35 i64.load32_u
    // 0x36 i32.store
    // 0x3A i32.store8
    // 0x3B i32.store16
    case I32Store:
    case I32Store8:
    case I32Store16:
    {
#if count_mem_acc
        emit_2_CALL(embed_func_count);
#endif
        // 读取对齐标签，丢弃
        ReadLEB_u32(&operand, start, end);
        // 读取offset1
        ReadLEB_u32(&operand, start, end);
        log(emit, "[I32.STORE %d]", operand.num32[0]);

#if check_memory_bound
        // offset 20-21
        emit_LDI(R20, operand.num8[0]);
        emit_LDI(R21, operand.num8[1]);
        // value 22-25
        emit_x_POP_32bit(R22);
        // addr 18-19
        emit_x_POP_16bit(R18);
        emit_x_POP_16bit(R30);//drop unused high 16bits
        switch (op)
        {
        case I32Store:
            emit_2_CALL(embed_func_i32store);
            break;
        case I32Store16:
            emit_2_CALL(embed_func_i32store16);
            break;
        case I32Store8:
            emit_2_CALL(embed_func_i32store8);
            break;
        default:
            break;
        }
        
#else
        operand.num16[0] += ts.wasm_globals_size;
        //读取要储存的值到R22...
        log(emit, "pop32 val to R22.");
        emit_x_POP_32bit(R22);
        //读取offset2到R18.
        log(emit, "pop32 offset to R18.");
        emit_x_POP_32bit(R18);
        // Z = Z + offset2 由于AVR地址为16位，因此可以略去高16位偏移的计算
        emit_MOVW(RZ, R2);
        emit_ADD(RZL, R18);
        emit_ADC(RZH, R19);

        emit_ADIW(RZ, operand.num16[0]);
        emit_STD(R22, Z, 0);
        if (op == I32Store16)
        {
            emit_STD(R23, Z, 1);
        }
        else if (op == I32Store)
        {
            emit_STD(R23, Z, 1);
            emit_STD(R24, Z, 2);
            emit_STD(R25, Z, 3);
        }
#endif

        ts.stack_top -= 2;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x37 i64.store
    case I64Store:
    {
#if count_mem_acc
        emit_2_CALL(embed_func_count);
#endif
        // 读取对齐标签，丢弃
        ReadLEB_u32(&operand, start, end);
        // 读取offset1
        ReadLEB_u32(&operand, start, end);
        log(emit, "[I64.STORE %d]", operand.num32[0]);
        // temp = global_size + offset1 越过全局变量区域
        operand.num16[0] += ts.wasm_globals_size;
        //读取要储存的值到R22...
        log(emit, "pop64 val to R18.");
        emit_x_POP_32bit(R18);
        emit_x_POP_32bit(R22);
        //读取offset2到R18.
        log(emit, "pop32 offset to R14.");
        emit_x_POP_32bit(R14);
        // Z = Z + offset2 由于AVR地址为16位，因此可以略去高16位偏移的计算
        emit_MOVW(RZ, R2);
        emit_ADD(RZL, R14);
        emit_ADC(RZH, R15);
        // ts.pc += 30;
        if (operand.num8[0] >= 30)
        {
            emit_LDI(R16, operand.num8[0]);
            emit_LDI(R17, operand.num8[1]);
            emit_ADD(RZL, R16);
            emit_ADC(RZH, R17);
            emit_STD(R18, Z, 0);
            emit_STD(R19, Z, 1);
            emit_STD(R20, Z, 2);
            emit_STD(R21, Z, 3);
            emit_STD(R22, Z, 4);
            emit_STD(R23, Z, 5);
            emit_STD(R24, Z, 6);
            emit_STD(R25, Z, 7);
            // ts.pc += 24;
        }
        else
        {
            emit_STD(R18, Z, operand.num8[0]);
            emit_STD(R19, Z, operand.num8[0] + 1);
            emit_STD(R20, Z, operand.num8[0] + 2);
            emit_STD(R21, Z, operand.num8[0] + 3);
            emit_STD(R22, Z, operand.num8[0] + 4);
            emit_STD(R23, Z, operand.num8[0] + 5);
            emit_STD(R24, Z, operand.num8[0] + 6);
            emit_STD(R25, Z, operand.num8[0] + 7);
            // ts.pc += 16;
        }
        ts.stack_top -= 3;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x38 f32.store
    // 0x39 f64.store
    // 0x3C i64.store8
    // 0x3D i64.store16
    // 0x3E i64.store32
    // 0x3F memory.size
    // 0x40 memory.grow
    // 0x41 i32.const
    case I32Const:
    {
#if count_stack_check
        emit_2_CALL(embed_func_count);
#endif
#if count_lowest_stack
        emit_2_CALL(embed_func_print_stack);
#endif
        ReadLEB_i32(&operand, start, end);
        log(emit, "[I32.CONST %ld]", operand.num32[0]);
        log(emit, "load %ld to R22.", operand.num32[0]);

        // emit_LDI(R26,1);
        // emit_2_STS(0x215,R26);
#if check_stack_overflow

        emit_IN(R24, 0x3d);
        emit_IN(R25, 0x3e);

        emit_2_STS(&malloc_record, R24);
        emit_2_STS(((u8 *)&malloc_record + 1), R25);

        emit_ADIW(R26, ts.wasm_mem_space + ts.wasm_globals_size + 12);
        emit_CP(R24, R26);
        emit_CPC(R25, R27);
        emit_BRGE(2);
        emit_BREAK();
#endif

        emit_LDI(R22, operand.num8[0]);
        emit_LDI(R23, operand.num8[1]);
        emit_LDI(R24, operand.num8[2]);
        emit_LDI(R25, operand.num8[3]);
        log(emit, "push32");

        // emit_2_STS(0x215,R1);

#if check_stack_overflow
        emit_2_STS(0x215, R1);
#endif
        emit_x_PUSH_32bit(R22);

        // ts.pc += 16;
        ts.stack_top++;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x42 i64.const
    case I64Const:
    {
#if count_stack_check
        emit_2_CALL(embed_func_count);
#endif
#if count_lowest_stack
        emit_2_CALL(embed_func_print_stack);
#endif
        ReadLEB_i64(&operand, start, end);
        log(emit, "[I64.CONST %ld]", operand.num32[0]);
        log(emit, "load %ld to R22.", operand.num32[0]);
        emit_LDI(R18, operand.num8[0]);
        emit_LDI(R19, operand.num8[1]);
        emit_LDI(R20, operand.num8[2]);
        emit_LDI(R21, operand.num8[3]);
        emit_LDI(R22, operand.num8[4]);
        emit_LDI(R23, operand.num8[5]);
        emit_LDI(R24, operand.num8[6]);
        emit_LDI(R25, operand.num8[7]);
        log(emit, "push32");
        emit_x_PUSH_32bit(R22);
        emit_x_PUSH_32bit(R18);
        // ts.pc += 32;
        ts.stack_top += 2; //暂时当做两个32位
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x43 f32.const
    // 0x44 f64.const
    // 0x45 i32.eqz
    case I32Eqz:
    {
        emit_x_POP_32bit(R18);
        emit_LDI(R22, 1);
        emit_LDI(R23, 0);
        emit_LDI(R24, 0);
        emit_LDI(R25, 0);
        emit_OR(R18, R19);
        emit_OR(R18, R20);
        emit_OR(R18, R21);
        emit_BREQ(2);
        emit_LDI(R22, 0);
        emit_x_PUSH_32bit(R22);
        // ts.pc += 34;
        break;
    }
    // 0x46 i32.eq
    case I32Eq:
    // 0x47 i32.ne
    case I32Ne:
    // 0x48 i32.lt_s
    case I32LtS:
    // 0x49 i32.lt_u
    case I32LtU:
    // 0x4A i32.gt_s
    case I32GtS:
    // 0x4B i32.gt_u
    case I32GtU:
    // 0x4C i32.le_s
    case I32LeS:
    // 0x4D i32.le_u
    case I32LeU:
    // 0x4E i32.ge_s
    case I32GeS:
    // 0x4F i32.ge_u
    case I32GeU:
    {
        log(emit, "[I32.(ComparisonInstruction)]");
        //Second operand
        emit_x_POP_32bit(R22);
        //First operand
        emit_x_POP_32bit(R18);
        // ts.pc += 16;
        // Compare per register
        if (op == I32LtS || op == I32LtU || op == I32GeS || op == I32GeU || op == I32Eq || op == I32Ne)
        {
            emit_CP(R18, R22);
            emit_CPC(R19, R23);
            emit_CPC(R20, R24);
            emit_CPC(R21, R25);
            // ts.pc += 8;
        }
        else if (op == I32LeS || op == I32LeU || op == I32GtS || op == I32GtU)
        {
            emit_CP(R22, R18);
            emit_CPC(R23, R19);
            emit_CPC(R24, R20);
            emit_CPC(R25, R21);
            // ts.pc += 8;
        }
        // if (op == I32GtS || op == I32LeS){
        //     emit_LDI(R22, 0);
        //     emit_LDI(R23, 0);
        //     emit_LDI(R24, 0);
        //     emit_LDI(R25, 0);
        // }
        // if (op == I32GtS || op == I32LtU || op == I32LeS || op == I32LeU){
        emit_LDI(R22, 1);
        emit_LDI(R23, 0);
        emit_LDI(R24, 0);
        emit_LDI(R25, 0);
        // ts.pc += 8;
        // }
        //BRSH = Branch if Same or Higher (Unsigned)
        //BRLO = Branch if Lower (Unsigned)
        //BRGE = Branch if Greater or Equal (Signed)
        //BRLT = Branch if Less Than (Signed)
        if (op == I32LtS || op == I32GtS)
        {
            emit_BRLT(2);
            // ts.pc += 2;
        }
        else if (op == I32LtU || op == I32GtU)
        {
            emit_BRCS(2);
            // ts.pc += 2;
        }
        else if (op == I32LeS || op == I32GeS)
        {
            emit_BRGE(2);
            // ts.pc += 2;
        }
        else if (op == I32LeU || op == I32GeU)
        {
            emit_BRCC(2);
            // ts.pc += 2;
        }
        else if (op == I32Eq)
        {
            emit_BREQ(2);
            // ts.pc += 2;
        }
        else if (op == I32Ne)
        {
            emit_BRNE(2);
            // ts.pc += 2;
        }

        emit_LDI(R22, 0);
        emit_x_PUSH_32bit(R22);
        // ts.pc += 10;
        ts.stack_top--;
        break;
    }
    // 0x50 i64.eqz
    // 0x51 i64.eq
    // 0x52 i64.ne
    // 0x53 i64.lt_s
    // 0x54 i64.lt_u
    // 0x55 i64.gt_s
    // 0x56 i64.gt_u
    // 0x57 i64.le_s
    // 0x58 i64.le_u
    // 0x59 i64.ge_s
    // 0x5A i64.ge_u
    // 0x5B f32.eq
    // 0x5C f32.ne
    // 0x5D f32.lt
    // 0x5E f32.gt
    // 0x5F f32.le
    // 0x60 f32.ge
    // 0x61 f64.eq
    // 0x62 f64.ne
    // 0x63 f64.lt
    // 0x64 f64.gt
    // 0x65 f64.le
    // 0x66 f64.ge
    // 0x67 i32.clz
    case I32Clz:
    {
        log(emit, "[I32.CLZ]");
        emit_x_POP_32bit(R22);
        emit_2_CALL(embed_func_clz32);
        emit_x_PUSH_32bit(R22);
        // ts.pc += 20;
        break;
    }
    // 0x68 i32.ctz
    // 0x69 i32.popcnt
    // 0x6A i32.add
    case I32Add:
    {
        log(emit, "[I32.ADD]");
        log(emit, "pop32 to R22.");
        emit_x_POP_32bit(R22);
        log(emit, "pop32 to R18.");
        emit_x_POP_32bit(R18);
        log(emit, "add32 R22. = R22. + R18.");
        emit_ADD(R22, R18);
        emit_ADC(R23, R19);
        emit_ADC(R24, R20);
        emit_ADC(R25, R21);
        log(emit, "push32");
        emit_x_PUSH_32bit(R22);
        // ts.pc += 32;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x6B i32.sub
    case I32Sub:
    {
        log(emit, "[I32.SUB]");
        // log(emit, "pop32 to R22.");
        emit_x_POP_32bit(R18);
        // log(emit, "pop32 to R18.");
        emit_x_POP_32bit(R22);
        // log(emit, "add32 R22. = R22. + R18.");
        emit_SUB(R22, R18);
        emit_SBC(R23, R19);
        emit_SBC(R24, R20);
        emit_SBC(R25, R21);
        // log(emit, "push32");
        emit_x_PUSH_32bit(R22);
        // ts.pc += 32;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x6C i32.mul
    case I32Mul:
    {
        log(emit, "[I32.MUL]");
        emit_x_POP_32bit(R18);
        emit_x_POP_32bit(R22);
        emit_2_CALL(embed_func_umul);
        emit_x_PUSH_32bit(R22);
        // ts.pc += 28;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x6D i32.div_s
    // 0x6E i32.div_u
    case I32DivS:
    case I32DivU:
    {
        log(emit, "[I32.DIV]");
        emit_x_POP_32bit(R18);
        emit_x_POP_32bit(R22);
        if (op == I32DivS)
        {
            emit_2_CALL(embed_func_idiv);
        }
        else
        {
            emit_2_CALL(embed_func_udiv);
        }
        emit_x_PUSH_32bit(R22);
        // ts.pc += 28;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x6F i32.rem_s
    // 0x70 i32.rem_u
    // 0x71 i32.and
    case I32And:
    // 0x72 i32.or
    case I32Or:
    // 0x73 i32.xor
    case I32Xor:
    {
        log(emit, "[I32.(logic)]");
        emit_x_POP_32bit(R22);
        emit_x_POP_32bit(R18);
        if (op == I32And)
        {
            emit_AND(R22, R18);
            emit_AND(R23, R19);
            emit_AND(R24, R20);
            emit_AND(R25, R21);
        }
        else if (op == I32Or)
        {
            emit_OR(R22, R18);
            emit_OR(R23, R19);
            emit_OR(R24, R20);
            emit_OR(R25, R21);
        }
        else if (op == I32Xor)
        {
            emit_EOR(R22, R18);
            emit_EOR(R23, R19);
            emit_EOR(R24, R20);
            emit_EOR(R25, R21);
        }

        emit_x_PUSH_32bit(R22);
        // ts.pc += 32;
        ts.stack_top--;
        log(emit, "s: %d", ts.stack_top);
        break;
    }
    // 0x74 i32.shl
    case I32Shl:
    // 0x75 i32.shr_s
    case I32ShrS:
    // 0x76 i32.shr_u
    case I32ShrU:
    {
        log(emit, "[I32.Shl | I32ShrS | I32ShrU]");
        //先pop要移动的位数
        emit_x_POP_32bit(R18);
        //现在的实现是，如果左移的位数>32的话就会全部变成0，wasmer的实现是循环左移，感觉不太对，需要找其他资料验证。
        // emit_LDI(R26, 0B00011111);
        // emit_AND(R18, R26);
        //再pop要移动的数
        emit_x_POP_32bit(R22);
        //思路1：将移动位数一直减，减到0为止
        emit_RJMP(8);
        // ts.pc += 18;
        if (op == I32Shl)
        {
            // emit_ADD(R22, R22);
            // emit_ADC(R23, R23);
            // emit_ADC(R24, R24);
            // emit_ADC(R25, R25);
            emit_LSL(R22);
            emit_ROL(R23);
            emit_ROL(R24);
            emit_ROL(R25);
            // ts.pc += 8;
        }
        else if (op == I32ShrU)
        {
            emit_LSR(R25);
            emit_ROR(R24);
            emit_ROR(R23);
            emit_ROR(R22);
            // ts.pc += 8;
        }
        else if (op = I32ShrS)
        { //TODO ShrS仍然有问题！！！-10移1位算的不对！！
            emit_ASR(R25);
            emit_ROR(R24);
            emit_ROR(R23);
            emit_ROR(R22);
            // ts.pc += 8;
        }
        emit_DEC(R18);
        emit_BRPL(-12);
        emit_x_PUSH_32bit(R22);
        // ts.pc += 12; //__attribute__ ((section (".rtc_code_marker")))
        ts.stack_top--;

        //思路2：取模的方式，可能在移位多的时候有优化空间，移位少的时候可能不如以上解法
        // emit_LDI(R18, 32);
        // emit_LDI(R19, 0);
        // emit_LDI(R20, 0);
        // emit_LDI(R21, 0);
        // emit_x_CALL((uint32_t)&__divmodsi4);
        break;
    }
        // 0x77 i32.rotl
        // 0x78 i32.rotr
        // 0x79 i64.clz
        // 0x7A i64.ctz
        // 0x7B i64.popcnt
        // 0x7C i64.add
        // 0x7D i64.sub
        // 0x7E i64.mul
        // 0x7F i64.div_s
        // 0x80 i64.div_u
        // 0x81 i64.rem_s
        // 0x82 i64.rem_u
        // 0x83 i64.and
        // 0x84 i64.or
        // 0x85 i64.xor
        // 0x86 i64.shl
        // 0x87 i64.shr_s
        // 0x88 i64.shr_u
        // 0x89 i64.rotl
        // 0x8A i64.rotr
        // 0x8B f32.abs
        // 0x8C f32.neg
        // 0x8D f32.ceil
        // 0x8E f32.floor
        // 0x8F f32.trunc
        // 0x90 f32.nearest
        // 0x91 f32.sqrt
        // 0x92 f32.add
        // 0x93 f32.sub
        // 0x94 f32.mul
        // 0x95 f32.div
        // 0x96 f32.min
        // 0x97 f32.max
        // 0x98 f32.copysign
        // 0x99 f64.abs
        // 0x9A f64.neg
        // 0x9B f64.ceil
        // 0x9C f64.floor
        // 0x9D f64.trunc
        // 0x9E f64.nearest
        // 0x9F f64.sqrt
        // 0xA0 f64.add
        // 0xA1 f64.sub
        // 0xA2 f64.mul
        // 0xA3 f64.div
        // 0xA4 f64.min
        // 0xA5 f64.max
        // 0xA6 f64.copysign
        // 0xA7 i32.wrap_i64
        // 0xA8 i32.trunc_f32_s
        // 0xA9 i32.trunc_f32_u
        // 0xAA i32.trunc_f64_s
        // 0xAB i32.trunc_f64_u
        // 0xAC i64.extend_i32_s
        // 0xAD i64.extend_i32_u
        // 0xAE i64.trunc_f32_s
        // 0xAF i64.trunc_f32_u
        // 0xB0 i64.trunc_f64_s
        // 0xB1 i64.trunc_f64_u
        // 0xB2 f32.convert_i32_s
        // 0xB3 f32.convert_i32_u
        // 0xB4 f32.convert_i64_s
        // 0xB5 f32.convert_i64_u
        // 0xB6 f32.demote_f64
        // 0xB7 f64.convert_i32_s
        // 0xB8 f64.convert_i32_u
        // 0xB9 f64.convert_i64_s
        // 0xBA f64.convert_i64_u
        // 0xBB f64.promote_f32
        // 0xBC i32.reinterpret_f32
        // 0xBD i64.reinterpret_f64
        // 0xBE f32.reinterpret_i32
        // 0xBF f64.reinterpret_i64
        // 0xC0 i32.extend8_s
        // 0xC1 i32.extend16_s
        // 0xC2 i64.extend8_s
        // 0xC3 i64.extend16_s
        // 0xC4 i64.extend32_s
        // 0xFC <i32|64>.trunc_sat_<f32|64>_<s|u>

    default:
    {
        panicf("unsupported instructions: %02X ", op);
        break;
    }
    }
    log(emit, "------------------------------");
}