#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "candles.h"

typedef struct Strategy Strategy;

typedef struct {
    Datetime entry_candle_datetime;
    Datetime entry_times_and_trades_datetime;
    Datetime exit_candle_datetime;
    Datetime exit_times_and_trades_datetime;
    float entry_price;
    float take_profit;
    float stop_loss;
    char op;
    char op_result[3];
    size_t size;
} Strategy_Result;

Strategy_Result *strategy(Candle *candles);
void __print_strategy_result(Strategy_Result *strategy_result);

#endif // STRATEGY_H_
