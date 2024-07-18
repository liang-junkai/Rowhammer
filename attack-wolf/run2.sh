#!/bin/bash

for ((i=0; i<=2000; i++))
do 
        sudo ./attack -o 73 -s 0
	sleep 3s
	sudo ./attack -o 73 -s 32
	sleep 3s
	sudo ./attack -o 73 -s 64
	sleep 3s
	sudo ./attack -o 73 -s 96
	sleep 3s
	sudo ./attack -o 73 -s 128
	sleep 3s
	sudo ./attack -o 73 -s 160
	sleep 3s
	sudo ./attack -o 73 -s 192
	sleep 3s
	sudo ./attack -o 73 -s 224
	sleep 3s
    
        echo "finish $i"
done

