#!/bin/bash

for ((i=0; i<=2000; i++))
do 
	sudo ./attack -o 68
	sudo ./attack -o 76
	
	echo "finish $i"
done
