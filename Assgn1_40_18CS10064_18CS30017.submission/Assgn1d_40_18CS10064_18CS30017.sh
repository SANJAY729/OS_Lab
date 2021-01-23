#!/bin/bash
if [ -f $1 ] ; then
  case $1 in
    *.tar.bz2) tar -xvjf $1 ;;
    *.tar.gz) tar -xvzf $1 ;;
    *.tar.xz) tar -xvJf $1 ;;
    *.bz2) bunzip2 $1 ;;
    *.rar) unrar e $1 ;;
    *.gz) gunzip $1 ;;
    *.tar) tar -xvf $1 ;;
    *.tbz2) tar -xvjf $1 ;;
    *.tgz) tar -xf $1 ;;
    *.zip) unzip $1 ;;
    *.Z) uncompress $1 ;;
    *.7z) 7z e ./$1 ;;
    *) echo "Unknown file format: cannot extract" ;;
  esac
else
  echo "File_does_not_exist"
fi