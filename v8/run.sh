#!/bin/bash

for ((i=0; i<=5000; i++))
do 
	sudo ./attack -o 143
	echo "finish $i"
	sleep 3s
done
