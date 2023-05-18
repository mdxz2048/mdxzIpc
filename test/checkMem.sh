#! /bin/sh
currentTime=$(date "+%Y-%m-%d %H:%M:%S")
#echo "[${startTime}] start app  begin!!!" >> /home/root/app.log
echo "[${currentTime}] START CHECK MEM!!!"
APP=daemon
PID=$(ps |grep ${APP}|grep -v grep |cut -d ' ' -f 1)
if [ -z "$PID" ]
then
	#PID=$(ps |grep ${APP}|grep -v grep |cut -d ' ' -f 2)
	PID=$(ps -ef |grep ipc_daemon|grep -v grep | awk '{print $2}')
fi
while true
do
	currentTime=$(date "+%Y-%m-%d %H:%M:%S")
	MEM=$(cat /proc/${PID}/status |grep VmRSS)
	echo "[${currentTime}] ${MEM}	"

sleep 10

done

