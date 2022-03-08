#!/usr/bin/env bash

# Run program with some preset flags

file=${1}

if [ -z $file ]; then
	echo "usage: ${0} <file>"
fi

createHex() {
	chars=('a' 'b' 'c' 'd' 'e' 'f' '0' '1' '2' '3' '4' '5' '6' '7' '8' '9')
	hexstr="#"

	for ((i = 0; i < 6; ++i)); do
		hexstr+=${chars[$(shuf -i 0-15 -n 1)]}
	done

	echo $hexstr
}

createImages() {
	for ((i=0; i < 30; ++i)); do
		./src/nftgen \
			--file $file \
			--hex-value $(createHex) \
			--out-file "foo${i}" \
			--mix-channels \
			--scale 4
	done
}

createImages
