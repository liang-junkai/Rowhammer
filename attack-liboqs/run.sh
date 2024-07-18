#!/bin/bash
offset1=1280
offset2=2560
for ((i=0; i<=10000; i++))
do 
        sudo ./attack -o 91 -s $i
	sleep 3s
	sudo ./attack -o 91 -s $((i + offset1))
	sleep 3s
	sudo ./attack -o 91 -s $((i + offset2))
        sleep 3s
    
        echo "finish $i"
done

