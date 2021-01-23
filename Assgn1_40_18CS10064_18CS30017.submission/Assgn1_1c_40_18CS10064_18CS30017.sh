#!/bin/bash
if [ $# -eq 2 ] && [ $1 = "1c_input.txt/1d_input.txt" ] && ( [ $2 -eq 1 ] || [ $2 -eq 2 ] || [ $2 -eq 3 ] || [ $2 -eq 4 ] );
then
    awk -v col=$2 '{print tolower($col)}' $1 | sort | uniq -c | sort -nr -k 1 | awk '{print $2 " " $1}' > 1c_output_$2_column.freq
fi