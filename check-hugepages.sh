#!/bin/bash

cat /proc/meminfo | fgrep Hugepagesize
for NODE in /sys/devices/system/node/node*; do
	cat "$NODE"/meminfo | fgrep HugePages_
done



