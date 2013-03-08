#!/bin/sh 
file=` find . -type f -printf "%T@ %Tx %TX %p\n"  | sort -n -r | head -1 \
    | awk -F '/' '{print $NF}' `
output=`echo $file | sed 's/.c//'`
echo $output
echo "gcc -o $output $file"
gcc -o $output $file
