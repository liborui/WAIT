#!/bin/bash

# java -jar avrora-beta-1.7.117.jar \
#  -colors=false -monitors=memory,c-timer,c-print,profile,stack \
#   -rtc-data-filename=${rtcdataFile}", "-rtc-gradle-build=${buildDir} \
#   -profile-data-filename=${profiledataFile} \
#    -single -mcu=atmega128 \
#    -dump-writes \
#    -locations=0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f \
#    ../bin/test

# cd ../app
# wat2wasm test.wat -o test.wasm --enable-annotations -v
# wasm2wat test.wasm -o testi.wat
# xxd -i test.wasm > test.wasm.h
# sed -i "s#char#char\ __attribute__\ ((section\ (\".rtc_code_marker\")))"# test.wasm.h
# cd ../libs


benchlist=(binsrch bsort fillarray funcall hsort fft lec outlier heatcalib heatdetect)
# benchlist=(wavelet)
# benchlist=(image)
for bench in ${benchlist[*]}
do 
    echo "start benchmark: $bench"
    java -jar avrora.jar -single -monitors=c-print,trace -trace-only-when-enabled=true -colors=false -mcu=atmega128 ../app/benchmark/$bench.elf | tee $bench.nativeoutput.txt 
done
