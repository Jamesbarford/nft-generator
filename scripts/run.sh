#!/usr/bin/bash

createImages() {
	local filename=$1
	if [ -z $filename ]; then
		echo '$1 filename must be provided'
		return
	fi

	./src/nftgen \
		--file $1 \
		--out-file "quality" \
		--scale 2 \
		--block-size 3
}

createImages $@
