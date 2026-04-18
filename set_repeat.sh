#!/usr/bin/zsh

count=80000
while [ $count -le 100000 ]; do
  ./tester.py set $count value &;
  ((count++))
done
