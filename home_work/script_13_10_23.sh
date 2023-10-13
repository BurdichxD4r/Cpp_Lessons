#!/bin/bash
for ((i=1;i<=$1;i+=1))
	do
	mkdir dir$i
	cd dir$i
	for ((n=1;n<=$2;n+=1))
		do
		touch file$n.txt
	done
	cd ..
done
