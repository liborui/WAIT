#!/bin/bash


benchlist=(hello-world)


for bench in ${benchlist[*]}
do 
    echo "start benchmark: $bench"
    cd ../src
    sed -i "s#\#include\ \"[a-zA-Z]*.wasm.h\"#\#include\ \"$bench.wasm.h\""# main.c
    cd ../example

    cd ../build
    make
    cd ../example

    java -jar avrora.jar -single -monitors=c-print,trace -trace-only-when-enabled=true -colors=false -mcu=atmega128 ../bin/aot.elf | tee $bench.wasmoutput.txt 
done
