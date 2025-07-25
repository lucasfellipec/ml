#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "candles.h"
#include "datetime.h"
#include "strategy.h"

#define TAKE_PROFIT 10
#define STOP_LOSS 10

Strategy_Result *strategy(Candle *candles) {
    Strategy_Result *strategy_result = {0};

    int strategy_result_count = 0;
    int initial_capacity = 1024;
    int capacity = initial_capacity;

    strategy_result = malloc(initial_capacity * sizeof(Strategy_Result));

    if (!strategy_result) {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    for (size_t i = 2; i < __get_candles_size(candles); i++) {
        if (strategy_result_count == capacity) {
            capacity *= 2;
            Strategy_Result *tmp = realloc(strategy_result, capacity * sizeof(Strategy_Result));
            if (!tmp) {
                fprintf(stderr, "Error allocating memory\n");
                free(strategy_result);
                return NULL;
            }
            strategy_result = tmp;
        }
        float delta_agg = get_delta_agg(candles, i - 1);
        float delta_agg_abs = fabs(get_delta_agg(candles, i - 1));
        float delta_std = get_delta_std(candles, i - 1);
        float delta_avg = get_delta_avg(candles, i - 1);
        int can_op = 0;

        float entry_price;
        float take_profit;
        float stop_loss;

        int pos = 0;

        if (get_trigger(candles, i - 1) == 'V') { // SELL
            entry_price = get_low(candles, i - 1) - 0.5;
            for (size_t j = 0; j < __get_candle_times_and_trades_size(candles, i); j++) {
                if (get_price_from_times_and_trades(candles, i, j) < entry_price) {
                    pos = get_candle_times_and_trades_row(candles, i, j);
                    // printf("Trigger:\n");
                    // __print_datetime(get_candle_datetime(candles, i - 1));
                    // printf("Entry Candle\n");
                    // __print_datetime(get_candle_datetime(candles, i));
                    // __print_datetime(get_candle_times_and_trades_datetime(candles, i, j));
                    // printf("Entry Price: %.1f\n", entry_price);
                    strategy_result[strategy_result_count].entry_times_and_trades_datetime = get_candle_times_and_trades_datetime(candles, i, j);
                    can_op = 1;
                    break;
                }
            }
            if (can_op) {
                take_profit = entry_price - TAKE_PROFIT;
                stop_loss = entry_price + STOP_LOSS;

                strategy_result[strategy_result_count].entry_candle_datetime = get_candle_datetime(candles, i);
                strategy_result[strategy_result_count].entry_price = entry_price;
                strategy_result[strategy_result_count].take_profit = take_profit;
                strategy_result[strategy_result_count].stop_loss = stop_loss;
                strategy_result[strategy_result_count].op = 'V';
                strcpy(strategy_result[strategy_result_count].op_result, "-");

                int hit = 0;

                for (size_t j = i; j < __get_candles_size(candles); j++) {
                    for (size_t k = (j == i ? pos : 0); k < __get_candle_times_and_trades_size(candles, j); k++) {
                        if (get_price_from_times_and_trades(candles, j, k) < take_profit) {
                            hit = 1;
                            // printf("Take Profit at:\n");
                            // __print_datetime(get_candle_datetime(candles, j));
                            // printf("Take Profit Price: %.1f\n", get_price_from_times_and_trades(candles, j, k));
                            strategy_result[strategy_result_count].exit_candle_datetime = get_candle_datetime(candles, j);
                            strategy_result[strategy_result_count].exit_times_and_trades_datetime = get_candle_times_and_trades_datetime(candles, j, k);
                            strcpy(strategy_result[strategy_result_count].op_result, "TP");
                            break;
                        } else if (get_price_from_times_and_trades(candles, j, k) > stop_loss) {
                            hit = 1;
                            // printf("Stop Loss at:\n");
                            // __print_datetime(get_candle_datetime(candles, j));
                            // printf("Stop Loss Price: %.1f\n", get_price_from_times_and_trades(candles, j, k));
                            strategy_result[strategy_result_count].exit_candle_datetime = get_candle_datetime(candles, j);
                            strategy_result[strategy_result_count].exit_times_and_trades_datetime = get_candle_times_and_trades_datetime(candles, j, k);
                            strcpy(strategy_result[strategy_result_count].op_result, "SL");
                            break;
                        }
                    }
                    if (hit) {
                        break;
                    }
                }
                strategy_result_count++;
            }
        }
    }

    strategy_result->size = strategy_result_count;

    return strategy_result;
}

void __print_strategy_result(Strategy_Result *strategy_result) {
    for (int i = 0; i < strategy_result->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", strategy_result[i].entry_candle_datetime.year, strategy_result[i].entry_candle_datetime.month, strategy_result[i].entry_candle_datetime.day, strategy_result[i].entry_candle_datetime.hour, strategy_result[i].entry_candle_datetime.minute, strategy_result[i].entry_candle_datetime.second);
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", strategy_result[i].entry_times_and_trades_datetime.year, strategy_result[i].entry_times_and_trades_datetime.month, strategy_result[i].entry_times_and_trades_datetime.day, strategy_result[i].entry_times_and_trades_datetime.hour, strategy_result[i].entry_times_and_trades_datetime.minute, strategy_result[i].entry_times_and_trades_datetime.second);
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", strategy_result[i].exit_candle_datetime.year, strategy_result[i].exit_candle_datetime.month, strategy_result[i].exit_candle_datetime.day, strategy_result[i].exit_candle_datetime.hour, strategy_result[i].exit_candle_datetime.minute, strategy_result[i].exit_candle_datetime.second);
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", strategy_result[i].exit_times_and_trades_datetime.year, strategy_result[i].exit_times_and_trades_datetime.month, strategy_result[i].exit_times_and_trades_datetime.day, strategy_result[i].exit_times_and_trades_datetime.hour, strategy_result[i].exit_times_and_trades_datetime.minute, strategy_result[i].exit_times_and_trades_datetime.second);
        printf("%.1f\t", strategy_result[i].entry_price);
        printf("%.1f\t", strategy_result[i].take_profit);
        printf("%.1f\t", strategy_result[i].stop_loss);
        printf("%c\t", strategy_result[i].op);
        printf("%s\n", strategy_result[i].op_result);
    }
}
