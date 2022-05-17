# rm *.elf
# rm *.wasm.wat
# rm *.wasm.h
# rm *.wasm


# rm -r build
# mkdir build
# cd build
# cmake .. -DAVRORA=TRUE
# make
# cd ..

rm -r build
mkdir build
cd build
cmake ..
make
cmake ..