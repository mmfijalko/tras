#!/bin/sh

for i in `seq 1 200`
do
    dd if=/dev/random bs=1k count=20 status=none | ./test -t sbs -S 102400 -s 102400 | grep pvalue | cut -d '=' -f2 | cut -d ' ' -f2
done

