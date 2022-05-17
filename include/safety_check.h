#ifndef SAFETY_CHECK_H
#define SAFETY_CHECK_H

#define compile_time_check 1

#define check_memory_bound 1
#define check_icall_bound 0
#define check_icall_type 0
//只需要在栈净增加的操作后使用，如const,call,这里测试加在const上的开销
#define check_stack_overflow 0

#define flash_data 1

#define count_mem_acc 0
#define count_stack_check 0
#define count_global_local 0
#define count_all_insn 0
#define count_lowest_stack 0

#endif