#!/bin/bash
index=0
for filename in `find . -name "*.wasm"`
do
    filelist[$index]=$filename
    index=`expr $index + 1`
    echo "found c file $filename"
done

for filename in ${filelist[*]}
do 
    echo "in $filename"
    if test $filename = "./benchmark.c"
    then 
        echo "continue"
        continue
    fi
    # $cc $filename -o "$filename".wasm -O3 -DNDEBUG -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,-s -Wl,--allow-undefined-file=stubs.txt -Wl,-z,stack-size=$z_stack_size
    wasm2wat "$filename" -o "$filename".wat
    xxd -i "$filename" > "$filename".h
    sed -i "s#char#char\ __attribute__\ ((section\ (\".rtc_code_marker\")))"# "$filename".h
    sed -i "s#__[a-zA-Z]*_wasm#test_wasm"# "$filename".h
done