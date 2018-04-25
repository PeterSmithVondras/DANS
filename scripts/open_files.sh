#!/bin/bash

SERVER='[0-9] ./server -s'
PROXY='[0-9] ./bazel-bin/common/dstage/dans_proxy'
CLIENT='[0-9] ./client'


while [ true ]
do
	SERVER_PID=$(ps -u | grep "${SERVER}" | sed 's_[a-z]* *\([0-9]*\).*_\1_')
	PROXY_PID=$(ps -u | grep "${PROXY}" | sed 's_[a-z]* *\([0-9]*\).*_\1_')
	CLIENT_PID=$(ps -u | grep "${CLIENT}" | sed 's_[a-z]* *\([0-9]*\).*_\1_')

	SERVER_FILES=""
	PROXY_FILES=""
	CLIENT_FILES=""

	if [ "" != "$SERVER_PID" ]; then
		SERVER_FILES="$(lsof -p ${SERVER_PID} | grep -c "")"
	fi
	if [ "" != "$PROXY_PID" ]; then
		PROXY_FILES="$(lsof -p ${PROXY_PID} | grep -c "")"
	fi
	if [ "" != "$CLIENT_PID" ]; then
		CLIENT_FILES="$(lsof -p ${CLIENT_PID} | grep -c "")"
	fi

	echo "OPEN FILES ------> Server:${SERVER_FILES} Proxy:${PROXY_FILES} Client:${CLIENT_FILES}"
	sleep 1

done
