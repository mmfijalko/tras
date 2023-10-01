#!/bin.sh

# Time format:
# %S sys
# %U user

set -e

for x in $(seq 1 1 200)
do
	/usr/bin/time --format="%E" -a -o /tmp/frequency_times_3.txt cat ${1} | ./test3
done
cat /tmp/frequency_times_3.txt | cut -d ':' -f 2 > /tmp/frequency_times_3.out
