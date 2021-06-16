#! /bin/bash

# destination_ip: 目标主机IP
# packet_size: 单个包大小
# repeat_time: 单个线程重复发包次数
# thread_count: 启动线程数

./client 													\
	--destination_ip=10.252.152.130 \
	--packet_size=1024							\
	--repeat_time=100								\
	--thread_count=1
