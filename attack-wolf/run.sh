#!/bin/bash
offset1=128
offset2=256
for ((i=0; i<=10000; i++))
do 
        sudo ./attack -o 84 -s $i
	sleep 3s
	sudo ./attack -o 84 -s $((i + offset1))
	sleep 3s
	sudo ./attack -o 84 -s $((i + offset2))
        sleep 3s
    
        echo "finish $i"
done

