#!/usr/bin/env bash

for ((i=0; i < 30; ++i)); do
	./processpng.out \
		--file ./layered_man/complete/man_pouring_water_computer_genisis.png \
		--hex-value $(./genranhex.py) \
		--out-file "foo${i}" \
		--mix-channels \
		--scale 4
done
