#!/bin/bash
mkdir 1.b.files.out
for file in 1.b.files/*.txt; do
    sort -nr $file > 1.b.files.out/${file:10}
    cat 1.b.files.out/${file:10} >> temp2.txt
done
sort -nr temp2.txt > 1.b.out.txt
rm temp2.txt