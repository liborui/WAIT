#ifndef WKREPROG_IMPL_H
#define WKREPROG_IMPL_H

#include "types.h"
#include "stdint.h"
#include "utils.h"

extern u16 wkreprog_impl_get_page_size();
extern bool wkreprog_impl_open_app_archive(u16 start_write_position);
extern bool wkreprog_impl_open_raw(uint_farptr_t start_write_position, uint_farptr_t end_of_safe_region);
extern void wkreprog_impl_write(u16 size, u8* data, bool skip);
extern void wkreprog_impl_close();
extern void wkreprog_impl_reboot();
extern uint_farptr_t wkreprog_impl_get_raw_position();

// extern u16 pc;
// 烧写起始地址相关设置

extern const unsigned char rtc_start_of_compiled_code_marker;
#define RTC_START_OF_COMPILED_CODE_SPACE (GET_FAR_ADDRESS(rtc_start_of_compiled_code_marker))
#define RTC_END_OF_COMPILED_CODE_SPACE ((u32)122880)
extern u16 rtc_start_of_next_method;
#define RTC_SET_START_OF_NEXT_METHOD(addr) do { rtc_start_of_next_method = (u16)(addr/2); } while(0)
#define RTC_GET_START_OF_NEXT_METHOD()     ( ((u32)rtc_start_of_next_method)*2 )


#endif // WKREPROG_IMPL_H
