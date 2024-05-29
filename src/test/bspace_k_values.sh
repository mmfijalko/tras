#!/bin/sh

for i in $(seq 1 10000)
do
	dd if=/dev/random bs=2k count=1 status=none | ./test -t bspace | grep -v pvalue | cut -d ' ' -f 5 >> /tmp/spacing.txt
done
