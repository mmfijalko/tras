#!/bin/sh

BINFILE_NAME=longrun_binary_`date +"%s_%N"`.bin
OUTPUT_FILE=longrun_binary_`date +"%s_%N"`.txt

rm -rf /tmp/${BINFILE_NAME}
dd if=/dev/urandom bs=1k count=100000 status=none of=/tmp/${BINFILE_NAME}

`cat /tmp/${BINFILE_NAME} | ./test -t longruns > /tmp/${OUTPUT_FILE}`

ALL=`cat /tmp/${OUTPUT_FILE} | grep "p-value" | wc -l`
FAIL=`cat /tmp/${OUTPUT_FILE} | grep fail | wc -l`
SUCCESS=`cat /tmp/${OUTPUT_FILE} | grep success | wc -l`

echo "longruns test stats:"
echo "all : ${ALL}"
echo "success : ${SUCCESS}"
echo "fails : ${FAIL}"

rm /tmp/${BINFILE_NAME}
rm /tmp/${OUTPUT_FILE}
