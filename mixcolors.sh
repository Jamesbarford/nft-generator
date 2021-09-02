#!/usr/bin/env bash

for ((i=0; i < 100; ++i)); do
	./processpng.out \
		--file /run/media/jwmbe/SPARE/toprocess/forzip/done_statues/alexander_great_head.png \
		--hex-value $(./genranhex.py) \
		--out-file "foo${i}" \
		--mix-channels \
		--scale 4
done
