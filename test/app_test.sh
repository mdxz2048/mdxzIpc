#!/bin/bash
currentTime=$(date "+%Y-%m-%d %H:%M:%S")
#echo "[${startTime}] start app  begin!!!" >> /home/root/app.log
echo "[${currentTime}] START  TEST !!"
APP=./debug/bin/libmdxzipctest


while true
do
	currentTime=$(date "+%Y-%m-%d %H:%M:%S")
	TESTA=$((RANDOM % 40000 + 1))
	TESTB=1
	echo "[$(date "+%Y-%m-%d %H:%M:%S")] ${TESTA} to ${TESTB}	"
	${APP} ${TESTA} ${TESTB} &
	sleep 10
	killall -9 libmdxzipctest

done
