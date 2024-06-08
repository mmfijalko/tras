#!/bin/sh

for i in $(seq 1 100)
do
	time -f "%e %E %S %U" -a -o /tmp/frequency_times.txt cat /tmp/rand100m.bin | ./test -t frequency 2>&1 > /dev/null
done
