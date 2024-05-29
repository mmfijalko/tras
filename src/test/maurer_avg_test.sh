#!/bin/sh

while true; 
do
	dd if=/dev/random bs=48k count=1000 status=none | ./test -t maurer | cut -d ':' -f 2 | cut -d ' ' -f 4 | ministat;
done
