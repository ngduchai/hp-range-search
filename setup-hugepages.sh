#!/bin/bash

# Create hugepages for LARM

# IMPORTANT: This script must be run before involking
# LARM in order to achieve the maximum performance.
# After running this script, remember to reboot
# the computer to make changes take effect.

# NOTE: This script only work with grub2. You could
# specify the number of pages created when the computer
# is started up by using the following command
#	setup-hugepages.sh [num_of_pages]

if [ "$#" -eq "1" ]; then
	grubby --update-kernel=ALL --args=hugepagesz=1G
	grubby --update-kernel=ALL --args=default_hugepagesz=1G
	grubby --update-kernel=ALL --args=hugepages="$1"
else
	grubby --update-kernel=ALL --args=hugepagesz=1G
	grubby --update-kernel=ALL --args=default_hugepagesz=1G
	grubby --update-kernel=ALL --args=hugepages=10
fi



