#!/bin/bash

# java -jar avrora.jar -single -monitors=trace,c-print -trace-only-when-enabled=true -colors=false -mcu=atmega128 ../bin/aot.elf | tee test.wasmoutput.txt 

java -jar avrora.jar -single -monitors=c-print -colors=false -mcu=atmega128 ../bin/aot.elf | tee test.wasmoutput.txt 