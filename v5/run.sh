#!/bin/bash

for ((i=1; i<=1000; i++))
do 
	sudo ./myattack -o 140
	echo "finish $i"
	sleep 5s
done
