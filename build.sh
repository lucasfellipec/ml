#!/bin/sh

set -ex

gcc -lm -g src/main.c src/strategy.c src/datetime.c src/candles.c src/times_and_trades.c -o build/main && ./build/main
