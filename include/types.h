#ifndef TYPES_H
#define TYPES_H

typedef char bool;
#define true 1
#define false 0
#include <stddef.h>

typedef unsigned char u8;
typedef char i8;
typedef unsigned short u16;
typedef short i16;
#ifdef AVRORA
typedef unsigned long u32;
typedef long i32;
typedef float f32;
typedef double f64; // need to be confirmed
#else
typedef unsigned int u32;
typedef int i32;
typedef float f32;
typedef double f64;
#endif
typedef unsigned long long u64;
typedef long long i64;
typedef u32 bytes;

struct WASM_MODULE;
struct WASM_FUNCTION;
struct IMPORT_INFO;
typedef struct IMPORT_INFO
{
    char *    moduleUtf8;
    char *    fieldUtf8;
//  unsigned char   type;
}import_info;
typedef import_info* import_info_ptr;
#define d_externalKind_function             0
#define d_externalKind_table                1
#define d_externalKind_memory               2
#define d_externalKind_global               3

typedef struct FUNC_TYPE
{
    struct FUNC_TYPE *     next;

    u32                     args_num;
    u8                      returnType;
    // u8                      need_memory_pass;
    u8                      args_type_list        [2];    // 用于32位对齐
}func_type;
typedef func_type* func_type_ptr;

typedef struct MEMORY_INFO
{
  u32 init_page_num;
  u32 max_page_num;
}memory_info;
typedef memory_info* memory_info_ptr;

typedef struct WASM_GLOBAL
{
    bytes                 initExpr;       // wasm code
    u32                     initExprSize;
    u8                      type;
    bool                    imported;
    struct IMPORT_INFO     import;
    bool                    isMutable;
    u16                   offset;
}wasm_global;
typedef wasm_global* wasm_global_ptr;

typedef void (*normal_function)();
typedef struct WASM_FUNCTION
{
    struct WASM_MODULE*      module;

    bytes                 wasm;
    bytes                 wasmEnd;

    char*                  name;

    struct FUNC_TYPE*      funcType;

    u16                    compiled;

    u16                     numArgSlots;

    u16                     numLocals;          // not including args
    u16                     numLocalBytes;

    struct IMPORT_INFO     import;

}wasm_function;
typedef wasm_function* wasm_function_ptr;

typedef struct DATA_SEGMENT
{
    u32              initExpr;       // wasm code
    u32              data;

    u16                     initExprSize;
    u16                     memoryRegion;
    u16                     size;
}data_segment;
typedef data_segment* data_segment_ptr;


typedef struct WASM_MODULE
{
  char* name;

  struct MEMORY_INFO memory;
  bool memory_imported;

  u16 global_num;
  struct WASM_GLOBAL*  global_list;

  u16 function_num;
  u16 import_num;

  struct WASM_FUNCTION** function_list;

  // u32 startFunction;
  // u16 entry_method;

  struct FUNC_TYPE** func_type_list;
  u16 func_type_num;

  struct DATA_SEGMENT* data_segment_list;
  u16 data_segment_num;
}wasm_module;
typedef wasm_module* wasm_module_ptr;

typedef struct WASM_CODE
{
  bytes ptr;
  u16 length;
}wasm_code;

typedef wasm_code* wasm_code_ptr;

typedef struct TRANSLATION_STATE
{
  u16 wasm_globals_size;
  bytes wasm_global_temp_space;
  bytes wasm_mem_space;
  u16 *codebuffer;
  u16 pc;
  u16 stack_top;
  wasm_function_ptr current_func;
  
}translation_state;

#define MAX_BLOCKS_NESTED 16
#define MAX_JUMP_INSTRUCTION_NUM 128
typedef struct BLOCK_CT
{
    u16 next_id;
    u16 block_label[MAX_BLOCKS_NESTED];
    u8 block_stack[MAX_BLOCKS_NESTED];
    u8 block_type[MAX_BLOCKS_NESTED];
    u16 block_pc[MAX_JUMP_INSTRUCTION_NUM];
    u16 top;
}wasm_block_ct;

static const char * const wasm_types_names []          = { "nil", "i32", "i64", "f32", "f64", "void", "void *" };
static const char * const wasm_types_names_compact []   = { "0", "i", "I", "f", "F", "v", "*" };

enum // WASM Variable Types
{
    WASM_Type_none   = 0,
    WASM_Type_i32    = 1,
    WASM_Type_i64    = 2,
    WASM_Type_f32    = 3,
    WASM_Type_f64    = 4,

    WASM_Type_void,
    WASM_Type_ptr,
    WASM_Type_trap
};

static inline
    u16 bswap16(u16 x) {
      return ((( x  >> 8 ) & 0xffu ) | (( x  & 0xffu ) << 8 ));
    }
    static inline
    u32 bswap32(u32 x) {
      return ((( x & 0xff000000u ) >> 24 ) |
              (( x & 0x00ff0000u ) >> 8  ) |
              (( x & 0x0000ff00u ) << 8  ) |
              (( x & 0x000000ffu ) << 24 ));
    }
    static inline
    u64 bswap64(u64 x) {
      return ((( x & 0xff00000000000000ull ) >> 56 ) |
              (( x & 0x00ff000000000000ull ) >> 40 ) |
              (( x & 0x0000ff0000000000ull ) >> 24 ) |
              (( x & 0x000000ff00000000ull ) >> 8  ) |
              (( x & 0x00000000ff000000ull ) << 8  ) |
              (( x & 0x0000000000ff0000ull ) << 24 ) |
              (( x & 0x000000000000ff00ull ) << 40 ) |
              (( x & 0x00000000000000ffull ) << 56 ));
    }

# if defined(BIG_ENDIAN)
#  define BSWAP_u8(X)  {}
#  define BSWAP_u16(X) { (X)=bswap16((X)); }
#  define BSWAP_u32(X) { (X)=bswap32((X)); }
#  define BSWAP_u64(X) { (X)=bswap64((X)); }
#  define BSWAP_i8(X)  {}
#  define BSWAP_i16(X) BSWAP_u16(X)
#  define BSWAP_i32(X) BSWAP_u32(X)
#  define BSWAP_i64(X) BSWAP_u64(X)
#  define BSWAP_f32(X) { union { f32 f; u32 i; } u; u.f = (X); BSWAP_u32(u.i); (X) = u.f; }
#  define BSWAP_f64(X) { union { f64 f; u64 i; } u; u.f = (X); BSWAP_u64(u.i); (X) = u.f; }
# else
#  define BSWAP_u8(X)  {}
#  define BSWAP_u16(X) {}
#  define BSWAP_u32(X) {}
#  define BSWAP_u64(X) {}
#  define BSWAP_i8(X)  {}
#  define BSWAP_i16(X) {}
#  define BSWAP_i32(X) {}
#  define BSWAP_i64(X) {}
#  define BSWAP_f32(X) {}
#  define BSWAP_f64(X) {}
# endif

#endif