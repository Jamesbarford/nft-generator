#!/usr/bin/bash

createImages() {
	local filename=$1
	if [ -z $filename ]; then
		echo '$1 filename must be provided'
	else
		./processpng.out \
			--file $1 \
			--out-file "quality" \
			--scale 2 \
			--block-size 3
	fi
}

createImages $@
