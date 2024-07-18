#!/bin/bash

for ((i=0; i<=2000; i++))
do 
	sudo ./attack -o 68
	sudo ./attack -o 66
	sudo ./attack -o 70
	
	echo "finish $i"
done
