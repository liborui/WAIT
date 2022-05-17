#include "types.h"
#include "debug.h"
#include "config.h"
#include "utils.h"
// #include "heap.h"
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

void ReadLebUnsigned(u64 *o_value, u32 i_maxNumBits, bytes *io_bytes, bytes i_end)
{
    u64 value = 0;

    u32 shift = 0;
    bytes ptr = *io_bytes;

    while (ptr < i_end)
    {
        u64 byte = pgm_read_byte_far(ptr);
        ptr++;

        value |= ((byte & 0x7f) << shift);
        shift += 7;

        if ((byte & 0x80) == 0)
        {
            break;
        }

        if (shift >= i_maxNumBits)
        {
            panicf("lebOverflow!");
            break;
        }
    }

    *o_value = value;
    *io_bytes = ptr;
}
void ReadLebSigned(i64 *o_value, u32 i_maxNumBits, bytes *io_bytes, bytes i_end)
{

    i64 value = 0;

    u32 shift = 0;
    bytes ptr = *io_bytes;

    while (ptr < i_end)
    {
        u64 byte = pgm_read_byte_far(ptr);
        ptr++;

        value |= ((byte & 0x7f) << shift);
        shift += 7;

        if ((byte & 0x80) == 0)
        {
            if ((byte & 0x40) && (shift < 64)) // do sign extension
            {
                u64 extend = 0;
                value |= (~extend << shift);
            }
            break;
        }

        if (shift >= i_maxNumBits)
        {
            panicf("leb overflow");
        }
    }
    *o_value = value;
    *io_bytes = ptr;
}
void ReadLEB_u7(u8 *o_value, bytes *io_bytes, bytes i_end)
{
    u64 value;
    ReadLebUnsigned(&value, 7, io_bytes, i_end);
    *o_value = (u8)value;
}
void ReadLEB_u32(u32 *o_value, bytes *io_bytes, bytes i_end)
{
    u64 value;
    ReadLebUnsigned(&value, 32, io_bytes, i_end);
    *o_value = (u32)value;
}
void Read_u8(u8 *o_value, bytes *io_bytes, bytes i_end)
{
    bytes ptr = *io_bytes;

    if (ptr < i_end)
    {
        *o_value = pgm_read_byte_far(ptr);
        ptr += sizeof(u8);
        *io_bytes = ptr;
    }
    else
        panicf("Wasm Underrun");
}

void Read_u32(u32 *o_value, bytes *io_bytes, bytes i_end)
{
    bytes ptr = *io_bytes;
    ptr += sizeof(u32);
    
    if (ptr <= i_end)
    {
        *o_value = pgm_read_dword_far(*io_bytes);
        // memcpy(o_value, *io_bytes, sizeof(u32));
        BSWAP_u32(*o_value);
        *io_bytes = ptr;
    }
    else
    {
        panic();
    }
}

void ReadLEB_i7(i8 *o_value, bytes *io_bytes, bytes i_end)
{
    i64 value;
    ReadLebSigned(&value, 7, io_bytes, i_end);
    *o_value = (i8)value;
}
void ReadLEB_i32(i32 *o_value, bytes *io_bytes, bytes i_end)
{
    i64 value;
    ReadLebSigned(&value, 32, io_bytes, i_end);
    *o_value = (i32)value;
}

void ReadLEB_i64(i64 *o_value, bytes *io_bytes, bytes i_end)
{
    i64 value;
    ReadLebSigned(&value, 64, io_bytes, i_end);
    *o_value = value;
}

void pgm_memcpy(u8* dst_dm,u8* src_pgm,u16 len){
    for(int i=0;i<len;i++){
        *dst_dm = pgm_read_byte_far(src_pgm);
        dst_dm++;
        src_pgm++;
    }
}

void Read_utf8(bytes *o_utf8, bytes *io_bytes, bytes i_end)
{
    *o_utf8 = NULL;

    u32 utf8Length;
    ReadLEB_u32(&utf8Length, io_bytes, i_end);

    if (utf8Length <= d_m3MaxSaneUtf8Length)
    {
        bytes ptr = *io_bytes;
        bytes end = ptr + utf8Length;

        if (end <= i_end)
        {
            char *utf8;
            utf8 = sys_malloc(utf8Length + 1);
            memcpy_PF(utf8,ptr,utf8Length);

            // pgm_memcpy(utf8, ptr, utf8Length);
            utf8[utf8Length] = 0;
            *o_utf8 = utf8;

            *io_bytes = end;
        }
        else
            panicf("wasm underrun");
    }
    else
        panicf("missing utf8");
}

void hexdump(char*  buf, u32 buf_len) {
	int i, j, mod = buf_len % 16;
	int n = 16 - mod;
	for (i = 0; i < buf_len; i++)
	{
		if (i % 16 == 0)
		{
            if(i!=0){
                printf("\n");
            }
			printf("0x0800%04x:\t",buf+i);
		}
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\t");
			for (j = i - 15; j <= i; j++)
			{
				if (j == i - 7)
					printf(" ");
				if (buf[j] >= 32 && buf[j] < 127)
					printf("%c", buf[j]);
				else
					printf(".");
			}
		}
	}
	for (i = 0; i < n; i++)
		printf("   ");
	printf("\t");
	for (i = buf_len - mod; i < buf_len; i++)
	{
		if (i == buf_len - mod + 8)
			printf(" ");
		if (buf[i] >= 32 && buf[i] < 127)
			printf("%c", buf[i]);
		else
			printf(".");
	}
	printf("\n");
}

void hexdump_pgm(u32 buf, u32 buf_len) {
	int i, j, mod = buf_len % 16;
	int n = 16 - mod;
    u8 c;
	for (i = 0; i < buf_len; i++)
	{
		if (i % 16 == 0)
		{
            if(i!=0){
                printf("\n");
            }
			printf("0x%08x:\t",buf+i);
		}
        // c =  pgm_read_byte_far(&buf[i]);
        c =  pgm_read_byte_far(buf+i);
		printf("%02X ", c);
		if ((i + 1) % 16 == 0)
		{
			printf("\t");
			for (j = i - 15; j <= i; j++)
			{
                c =  pgm_read_byte_far(buf+j);
				if (j == i - 7)
					printf(" ");
				if (c >= 32 && c < 127)
					printf("%c", c);
				else
					printf(".");
			}
		}
	}
	for (i = 0; i < n; i++)
		printf("   ");
	printf("\t");
	for (i = buf_len - mod; i < buf_len; i++)
	{
        c =  pgm_read_byte_far(buf+i);
		if (i == buf_len - mod + 8)
			printf(" ");
		if (c >= 32 && c < 127)
			printf("%c", c);
		else
			printf(".");
	}
	printf("\n");
}

u32 mystrcmp(const char* S1,const char* S2){
    while(*S1){
        if(*(S1++)!=*(S2++)){
            return 1;
        }
    }
    if(*S2){
        return 1;
    }

    return 0;
}

bool is_entry_func(wasm_module_ptr module, wasm_function_ptr func){
    return func==module->function_list[module->function_num-1];
    // return func==module->function_list[module->import_num];
}
int NormalizeType(u8 *o_type, i8 i_convolutedWasmType)
{
    int result = 0;

    u8 type = -i_convolutedWasmType;

    if (type == 0x40)
        type = WASM_Type_none;
    else if (type < WASM_Type_i32 || type > WASM_Type_f64)
        result = 1;

    *o_type = type;

    return result;
}

void* sys_malloc(u16 size){
    void* ret = malloc(size);
    // logif(sys,printf("malloc %d ",size);printf("%d",ret););
    log(sys,"malloc %d %x",size,ret);
    return ret;
}
void* sys_calloc(u16 numBlocks,u16 size){
    void* ret = calloc(numBlocks,size);
    // logif(sys,printf("calloc %d ",size*numBlocks);printf("%d",ret););
    log(sys,"calloc %d %x",size,ret);
    return ret;
}
void* sys_realloc(void* ptr,u16 size){
    void* ret = realloc(ptr,size);
    // logif(sys,printf("realloc %d",size);printf("%d",ret););
    log(sys,"realloc %d %x %x",size,ptr,ret);
    return ret;
}
void sys_free(void* ptr){
    free(ptr);
    log(sys,"free %x",ptr);
}
