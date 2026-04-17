#!/usr/bin/zsh

count=1
while [ $count -le 10 ]; do
  ./tester.py set $count value;
  ((count++))
done
