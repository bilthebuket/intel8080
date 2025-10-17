#!/bin/bash
# Converts a binary file (machine code) to space-separated hex bytes,
# with 10 bytes per line.

# Usage: ./bin2hex.sh input.bin

if [ $# -ne 1 ]; then
    echo "Usage: $0 <binary_file>"
    exit 1
fi

input="$1"

# Check file existence
if [ ! -f "$input" ]; then
    echo "Error: File '$input' not found."
    exit 1
fi

# Use xxd to convert binary to hex bytes, then format output
xxd -p "$input" | tr -d '\n' | sed 's/../& /g' | \
awk '{ 
    count=0
    for (i=1; i<=NF; i++) {
        printf "%s", $i
        count++
        if (count == 10) { printf "\n"; count=0 } 
        else { printf " " }
    }
    if (count != 0) printf "\n"
}'
