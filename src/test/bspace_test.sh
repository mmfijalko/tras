#!/bin/sh

BITS=`echo "1024*1024*200*8" | bc`

for i in `seq 1 200`
do
    dd if=/dev/random bs=1M count=200 status=none | ./test -t bspace -S $BITS -s $BITS | grep pvalue | cut -d '=' -f2 | cut -d ' ' -f2
done

