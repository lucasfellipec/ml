#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "candles.h"
#include "datetime.h"
#include "strategy.h"

#define TAKE_PROFIT 10
#define STOP_LOSS 10

typedef struct {
    Datetime entry_datetime;
    float entry_price;
    float take_profit;
    float stop_loss;
    char direction;
} Strategy_Result;

Strategy_Result strategy_result[1024];
int strategy_result_count = 0;

void strategy(Candle *candles) {
    for (size_t i = 2; i < __get_candles_size(candles); i++) {
        float agg_delta = get_agg_delta(candles, i - 1);
        float fabs_agg_delta = fabs(get_agg_delta(candles, i - 1));
        float std_delta = get_std_delta(candles, i - 1);
        float avg_delta = get_avg_delta(candles, i - 1);
        int can_op = 0;

        float entry_price;
        float take_profit;
        float stop_loss;

        int pos = 0;

        if (fabs_agg_delta > 1.1 * std_delta && fabs_agg_delta > 1.1 * avg_delta && agg_delta > 0) { // SELL
            entry_price = get_low(candles, i - 1) - 0.5;
            for (size_t j = 0; j < __get_candle_times_and_trades_size(candles, i); j++) {
                if (get_price_from_times_and_trades(candles, i, j) < entry_price) {
                    __print_datetime(get_candle_datetime(candles, i - 2));
                    __print_datetime(get_candle_datetime(candles, i - 1));
                    __print_datetime(get_candle_times_and_trades_datetime(candles, i, j));
                    pos = get_candle_times_and_trades_row(candles, i, j);
                    __print_datetime(get_candle_datetime(candles, i));
                    printf("Entry Price: %.1f\n", entry_price);
                    can_op = 1;
                    break;
                }
            }
            if (can_op) {
                take_profit = entry_price - TAKE_PROFIT;
                stop_loss = entry_price + STOP_LOSS;

                strategy_result[strategy_result_count].entry_datetime = get_candle_datetime(candles, i);
                strategy_result[strategy_result_count].entry_price = entry_price;
                strategy_result[strategy_result_count].take_profit = take_profit;
                strategy_result[strategy_result_count].stop_loss = stop_loss;
            }
        }
        if (fabs_agg_delta > 1.1 * std_delta && fabs_agg_delta > 1.1 * avg_delta && agg_delta < 0) { // BUY
            entry_price = get_high(candles, i - 1) + 0.5;
            for (size_t j = 0; j < __get_candle_times_and_trades_size(candles, i); j++) {
                if (get_price_from_times_and_trades(candles, i, j) > entry_price) {
                    printf("Gatilho\n");
                    __print_datetime(get_candle_datetime(candles, i - 1));
                    __print_datetime(get_candle_times_and_trades_datetime(candles, i, j));
                    pos = get_candle_times_and_trades_row(candles, i, j);
                    __print_datetime(get_candle_datetime(candles, i));
                    printf("Entry Price: %.1f\n", entry_price);
                    can_op = 1;
                    break;
                }
            }
            if (can_op) {
                take_profit = entry_price - TAKE_PROFIT;
                stop_loss = entry_price + STOP_LOSS;

                strategy_result[strategy_result_count].entry_datetime = get_candle_datetime(candles, i);
                strategy_result[strategy_result_count].entry_price = entry_price;
                strategy_result[strategy_result_count].take_profit = take_profit;
                strategy_result[strategy_result_count].stop_loss = stop_loss;
            }
        }
    }
}
