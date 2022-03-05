#!/usr/bin/env bash

# Create a gif by layering some pictures

file=$1

if [ -z $file ]; then
	echo '$1 file must be provided'
else
	convert -fuzz 10% -layers Optimize -delay 20 -loop 0 "${file}*.png" out.gif
fi
