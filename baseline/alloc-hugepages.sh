#!/bin/bash

# Dynamically allocate huge pages. Remember to run under root permission

if [ "$#" -eq  "1" ]; then
	echo $1 > /proc/sys/vm/nr_hugepages
else
	echo "Usage: alloc-hugepages [num_of_pages]"
fi

