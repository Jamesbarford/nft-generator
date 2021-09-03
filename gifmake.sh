#!/usr/bin/env bash

convert -fuzz 10% -layers Optimize -delay 8 -loop 0 ./david/*.png out.gif
