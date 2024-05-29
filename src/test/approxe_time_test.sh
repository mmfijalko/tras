#!/bin/sh

for i in $(seq 1 1000)
do
	dd if=/dev/random bs=1k count=2 status=none | ./test -t approxe | grep pvalue
done
