#!/bin/bash
length=16
if [ $# -gt 1 ] 
then
    echo "enter_single_positive_integer"
    exit
elif [ $# -eq 1 ] && [ $1 -lt 0 ]
then
    echo "enter_positive_integer"
    exit
elif [ $# -eq 1 ]
then
    length=$1
fi
tr -dc 'A-Za-z0-9_' </dev/urandom | head -c $length; echo