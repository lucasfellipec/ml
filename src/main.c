#include <stdio.h>
#include <stdlib.h>

#include "candles.h"
#include "strategy.h"
#include "times_and_trades.h"

#define TIMEFRAME 300
#define SMA_PERIOD 20
#define EMA_PERIOD 9

const char *files[] = {
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_02.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_03.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_06.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_07.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_08.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_09.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_10.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_13.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_14.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_15.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_16.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_17.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_20.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_21.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_22.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_23.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_24.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_27.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_28.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_29.txt",
    "times_and_trades_bases/2024.05/TT_NEG_WDO_2024_05_31.txt",
};

int main() {
    printf("Entry_Candle_Datetime\tEntry_Price\tTake_Profit\tStop_Loss\tOp\tExit_Candle_Datetime\tExit_Price\tExit_Times_and_Trades_Datetime\tOp_Result\n");
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

        Times_And_Trades *times_and_trades = read_times_and_trades(filename);
        Candle *candles = generate_candles(times_and_trades, TIMEFRAME, &last_ema, &last_closes, &last_close_count, SMA_PERIOD, EMA_PERIOD);

        // __print_candles(candles);
        // __generate_footprint(candles);
        generate_triggers(candles);
        Strategy_Result *strategy_result = strategy(candles);
        __print_strategy_result(strategy_result);

        free(times_and_trades);
        __free_candles(candles);
    }

    free(last_closes);
    return 0;
}
