#!/bin/bash

for filename in `find . -name "*.wasm"`
do
    rm $filename
done

for filename in `find . -name "*.wat"`
do
    rm $filename
done

for filename in `find . -name "*.wasm.h"`
do
    rm $filename
done

for filename in `find . -name "*.wasmoutput.txt"`
do
    rm $filename
done
