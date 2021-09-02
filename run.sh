#!/usr/bin/bash

createImages() {
	local filename=$1
	if [ -z $filename ];
		echo '$1 filename must be provided'
	else 
		./processpng.out \
			--file $1 \
			--out-file "quality" \
			--from 1 \
			--to 2 \
			--scale 2
	fi
}

createImages
