#ifndef CANDLES_H_
#define CANDLES_H_

#include "times_and_trades.h"

typedef enum {
    RED,
    GREEN,
    DOJI
} Candle_Color;

typedef struct Candles Candles;

Candles *generate_candles(Times_And_Trades *times_and_trades, int timeframe, float *prev_ema, float **prev_closes, int *prev_close_count, int sma_period, int ema_period);
int calculate_structure(Candles *candles, int pos, int lookback);
float calculate_region_distance(Candles *candles, int pos, int lookback, float pts);

Datetime get_candle_datetime(Candles *candles, int pos);
float get_open(Candles *candles, int pos);
float get_high(Candles *candles, int pos);
float get_low(Candles *candles, int pos);
float get_close(Candles *candles, int pos);
double get_real_volume(Candles *candles, int pos);
float get_daily_low_up_to_current_candle(Candles *candles, int pos);
float get_daily_high_up_to_current_candle(Candles *candles, int pos);
Candle_Color get_color(Candles *candles, int pos);
int get_structure(Candles *candles, int pos);
float get_price_from_times_and_trades(Candles *candles, int pos, int row);
Datetime get_candle_times_and_trades_datetime(Candles *candles, int pos, int row);

size_t __get_candle_times_and_trades_size(Candles *candles, int pos);
size_t __get_candles_size(Candles *candles);
void __free_candles(Candles *candles);
void __print_candles(Candles *candles);

#endif // CANDLES_H_
