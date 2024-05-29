#!/bin/sh

for i in $(seq 1 100)
do
	time -f "%e %E %S %U" -a -o /tmp/cusum_times.txt cat /tmp/rand2k.bin | ./test -t cusum 2>&1 > /dev/null
done
