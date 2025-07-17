#ifndef CANDLES_H_
#define CANDLES_H_

#include "times_and_trades.h"

typedef enum {
    RED,
    GREEN,
    DOJI
} Candle_Color;

typedef struct Candles Candles;

Candles *generate_candles(Times_And_Trades *times_and_trades, int timeframe);
int calculate_structure(Candles *candles, int pos, int lookback);

size_t __get_candles_times_and_trades_size(Candles *candles, int pos);
size_t __get_candles_size(Candles *candles);
void __free_candles(Candles *candles);
void __print_candles(Candles *candles);

#endif // CANDLES_H_
