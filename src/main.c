#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "candles.h"
#include "times_and_trades.h"

#define TIMEFRAME 300
#define SIMPLE_MOVING_AVERAGE_PERIOD 20
#define EXPONENTIAL_MOVING_AVERAGE_PERIOD 9

const char *files[] = {
    "times_and_trades_bases/2025.07 inc/TT_NEG_WDO_2025_07_22 inc.txt",
    "times_and_trades_bases/2025.07 inc/TT_NEG_WDO_2025_07_23 inc.txt"
};

int main() {
    size_t files_size = sizeof(files) / sizeof(files[0]);

    float last_ema = 0;
    float *last_closes = malloc(SIMPLE_MOVING_AVERAGE_PERIOD * sizeof(float));

    if (!last_closes) {
        fprintf(stderr, "Error allocating memory\n");
        return 1;
    }

    int last_close_count = 0;

    for (size_t i = 0; i < files_size; i++) {
        const char *filename = files[i];

        Times_And_Trades *times_and_trades = read_times_and_trades(filename);
        Candle *candles = generate_candles(times_and_trades, TIMEFRAME, &last_ema, &last_closes, &last_close_count, SIMPLE_MOVING_AVERAGE_PERIOD, EXPONENTIAL_MOVING_AVERAGE_PERIOD);

        char buffer[512] = {0};
        strcat(buffer, filename);
        char *out = ".candles";
        strcat(buffer, out);

        __print_candles(candles, buffer);

        free(times_and_trades);
        __free_candles(candles);
    }

    free(last_closes);
    return 0;
}
