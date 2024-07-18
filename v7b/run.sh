#!/bin/bash

for ((i=0; i<=1000; i++))
do 
	sudo ./attack -o 86
	
	echo "finish $i"
	sleep 3s
done
