sh -c 'for i in `seq 1 1000`; do dd if=/dev/random bs=`echo "256004"` count=1 status=none | ./test -t c1tsbits -S `echo "256004"` -s `echo "256004 * 8" | bc`; done' | cut -d '=' -f2 | cut -d ' ' -f2 | ministat

