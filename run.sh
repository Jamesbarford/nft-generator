#!/usr/bin/bash

createImages() {
	for ((i=0; i < 20; ++i)); do
		./processpng.out \
			--file ~/Downloads/oitmate.png \
			--out-file "quality:${i}" \
			--block-size $i \
			--scale 2
	done
}

createImages
