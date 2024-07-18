#!/bin/bash

for ((i=0; i<=10000; i++))
do 
	sudo ./attack -o 88
	
	echo "finish $i"
done
