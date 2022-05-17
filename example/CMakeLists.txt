cmake_minimum_required(VERSION 3.16)

PROJECT(benchmark)

INCLUDE_DIRECTORIES(.)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
########################### AVRORA NATIVE编译 ##########################
if(AVRORA)
add_definitions(-DAVRORA)

set (CMAKE_C_COMPILER "avr-gcc")
set (CMAKE_CXX_COMPILER "avr-g++")
#-Werror
add_compile_options(-mmcu=atmega128 -Os -finline-limit-0 -ggdb -Wall  -std=gnu99 -fdata-sections -ffunction-sections)
set (CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega128 -Wl,--gc-sections ")

else()

########################### WASM编译 ##########################
set (CMAKE_C_COMPILER "${PROJECT_SOURCE_DIR}/../third-party/wasi-sdk-12.0/bin/clang")
set (z_stack_size 16)
add_compile_options(-Os -DNDEBUG)
set (CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,--no-entry -Wl,--export-all -Wl,-s -Wl,--allow-undefined-file=${PROJECT_SOURCE_DIR}/stubs.txt -Wl,-z,stack-size=${z_stack_size}")

ADD_EXECUTABLE(helloworld.wasm helloworld/helloworld.c)

execute_process(COMMAND bash towat.sh WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

endif(AVRORA)





# add_custom_command(
#     TARGET ${PROJECT_NAME}.elf
#     POST_BUILD
#     COMMAND avr-nm -S -l ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf > ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.nm
#     COMMAND echo nmap done
#     BYPRODUCTS ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.nm
#     VERBATIM
# )
