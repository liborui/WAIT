#!/bin/bash
index=0
for filename in `find . -name "*.wat"`
do
    filelist[$index]=$filename
    index=`expr $index + 1`
    echo "found wat file $filename"
done
echo "-----------"
echo "instructions used in these files:"
cat ${filelist[*]} |grep "[A-Za-z][A-Za-z0-9]*\.[a-zA-Z][a-zA-Z0-9_]*" -o|sort|uniq
echo "-----------"
echo "files with i64 instructions:"
for filename in ${filelist[*]}
do 
    grep "i64" -l $filename
done