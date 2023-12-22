#!/bin/bash
#set -x
BROKER="broker.emqx.io"
PI=$(echo "scale=10; 4*a(1)" | bc -l)
echo ${PI}
deg=0
while :
do
	# Convert fron degree to radian
	echo -n "deg="
	echo ${deg}
	rad=`echo "${deg}*${PI}/180"  | bc -l`
	echo -n "rad="
	echo ${rad}

	# Increment degree
	deg=`echo "${deg}+10" | bc -l`
	if [ $deg -ge 360 ] ; then
		deg=0
	fi

	# Calculate sin and cos
	sin=`echo "s(${rad})"  | bc -l | awk '{printf "%.5f\n", $0}'`
	cos=`echo "c(${rad})"  | bc -l | awk '{printf "%.5f\n", $0}'`

	# Do not calculate tan when cos is 0
	result=`echo "$cos == 0" | bc`
	echo -n "result="
	echo ${result}
	if [ ${result} -eq 1 ] ; then
		tan=0
	else
		tan=`echo "${sin}/${cos}"  | bc -l | awk '{printf "%.5f\n", $0}'`
	fi

	#echo ${sin}
	#echo ${cos}
	#echo ${tan}
	payload="{\"sin\": ${sin}, \"cos\": ${cos}, \"tan\":${tan}}"
	echo ${payload}
	mosquitto_pub -h ${BROKER} -p 1883 -t "/topic/test" -m "$payload"
	sleep 2
done
