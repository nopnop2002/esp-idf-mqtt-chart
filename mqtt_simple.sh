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
    deg=`echo "${deg}+10" | bc -l`
    if [ $deg -ge 360 ] ; then
        deg=0
    fi

    # Calculate sin and cos
    sin=`echo "s(${rad})"  | bc -l | awk '{printf "%.5f\n", $0}'`
	echo -n "sin="
	echo ${sin}
	mosquitto_pub -h ${BROKER} -p 1883 -t "/topic/test" -m "${sin}"
	sleep 2
done
