#include <stdlib.h>

#include "candles.h"
#include "times_and_trades.h"
#include "strategy.h"

#define TIMEFRAME 300

const char *files[] = {
    "csv/WDO@D_202412020900_202412021829.csv",
    "csv/WDO@D_202412030900_202412031829.csv"
};

int main() {
    size_t files_size = sizeof(files) / sizeof(files[0]);
    for (size_t i = 0; i < files_size; i++) {
        const char *filename = files[i];

        Times_And_Trades* times_and_trades = read_times_and_trades(filename);
        Candles *candles = generate_candles(times_and_trades, TIMEFRAME);

        __print_candles(candles);

        strategy(candles);

        free(times_and_trades);
        __free_candles(candles);
    }
    return 0;
}
