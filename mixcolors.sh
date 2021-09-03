#!/usr/bin/env bash

for ((i=0; i < 30; ++i)); do
	./processpng.out \
		--file /run/media/jwmbe/SPARE/toprocess/forzip/done_statues/aphrodite_1.png \
		--hex-value $(./genranhex.py) \
		--out-file "foo${i}" \
		--mix-channels \
		--scale 4
done
