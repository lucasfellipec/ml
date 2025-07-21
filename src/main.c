#include <stdio.h>
#include <stdlib.h>

#include "candles.h"
// #include "strategy.h"
#include "times_and_trades.h"

#define TIMEFRAME 300
#define SMA_PERIOD 20
#define EMA_PERIOD 9

const char *files[] = {
    "csv/TT_NEG_WDO_24_06_21-fixed.txt",
};

int main() {
    size_t files_size = sizeof(files) / sizeof(files[0]);

    float last_ema = 0;
    float *last_closes = malloc(SMA_PERIOD * sizeof(float));

    if (!last_closes) {
        fprintf(stderr, "Error allocating memory\n");
        return 1;
    }

    int last_close_count = 0;

    for (size_t i = 0; i < files_size; i++) {
        const char *filename = files[i];

        Times_And_Trades* times_and_trades = read_times_and_trades(filename);
        Candle *candles = generate_candles(times_and_trades, TIMEFRAME, &last_ema, &last_closes, &last_close_count, SMA_PERIOD, EMA_PERIOD);

        // __print_candles(candles);
        __generate_footprint_csv(candles);

        // strategy(candles);

        free(times_and_trades);
        __free_candles(candles);
    }

    free(last_closes);
    return 0;
}
