#!/usr/bin/zsh

count=80000
while [ $count -le 100000 ]; do
  ./tester.py hset user:1 age 24 &;
  ((count++))
done
