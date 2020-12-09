Dev=$(shell cat /proc/devices | grep cec | sed 's/cec//g')
ccs:
	echo $(Dev)