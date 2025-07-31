#!/bin/sh

set -ex

gcc -lm -g src/main.c src/strategy.c src/datetime.c src/candles.c src/times_and_trades.c -o build/main

for file in times_and_trades_bases/**/*.txt; do
    echo ${file}
    if [ -s "$file" ]; then
        ./build/main "${file}"
    fi
done
