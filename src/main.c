#include <stdlib.h>

#include "candles.h"
#include "times_and_trades.h"

#define TIMEFRAME 300

int main() {
    const char *filename = "csv/WDO@D_202412020900_202412021829.csv";
    Times_And_Trades* times_and_trades = read_times_and_trades(filename);
    Candles *candles = generate_candles(times_and_trades, TIMEFRAME);

    __print_candles(candles);

    free(times_and_trades);
    __free_candles(candles);
    return 0;
}
