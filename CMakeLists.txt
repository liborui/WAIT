cmake_minimum_required(VERSION 3.16)

PROJECT(aot)

if(DEFINED NATIVE)
else()
add_definitions(-DAVRORA)
set (CMAKE_C_COMPILER "avr-gcc")
set (CMAKE_CXX_COMPILER "avr-g++")
#-Werror
add_compile_options(-mmcu=atmega128 -O2 -finline-limit-0 -ggdb -Wall  -std=gnu99 -fdata-sections -ffunction-sections)
set (CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega128 \
-Wl,--section-start=.reprogram_flash_page=0x1F800 \
-Wl,-T${PROJECT_SOURCE_DIR}/include/ld.lnk \
-Wl,--gc-sections \
")
# -Wl,--defsym=__heap_start=0x802000,--defsym=__heap_end=0x803fff")

endif()

#head file path
INCLUDE_DIRECTORIES(
include
example
example/hello-world
)
#source directory
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

SET(test
${DIR_SRCS}
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
if(DEFINED NATIVE)
ADD_EXECUTABLE(test ${test})
else()


ADD_EXECUTABLE(${PROJECT_NAME}.elf ${test})

add_custom_command(
    TARGET ${PROJECT_NAME}.elf
    POST_BUILD
    COMMAND avr-objdump -D ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf > ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.S
    COMMAND echo dissassemble done
    BYPRODUCTS ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.S
    VERBATIM
)

add_custom_command(
    TARGET ${PROJECT_NAME}.elf
    POST_BUILD
    COMMAND avr-nm -S -l ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.elf > ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.nm
    COMMAND echo nmap done
    BYPRODUCTS ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.nm
    VERBATIM
)

endif() 