#!/bin/bash
offset1=3107
offset2=1011
for ((i=0; i<=10000; i++))
do 
	sudo ./attack -o 95 -s $((i * 7))
	sleep 2s
	sudo ./attack -o 96 -s $((i * 7 + offset1))
	sleep 2s
	sudo ./attack -o 97 -s $((i * 7 + offset2))
        sleep 2s
    
        echo "finish $i"
done

