#! /bin/bash

./sender01 												\
	--interface=eno2							\
	--repeatTime=10000						\
	--threadCount=1								\
	--packetSize=1024							\
	--destIP="10.126.82.28"					\
	--destMAC="50:65:f3:65:c5:9e"
