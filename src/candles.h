#ifndef CANDLES_H_
#define CANDLES_H_

#include "tape_reading.h"
#include "times_and_trades.h"

typedef enum {
    RED,
    GREEN,
    DOJI
} Candle_Color;

typedef struct Candle Candle;

Candle *generate_candles(Times_And_Trades *times_and_trades, int timeframe, float *prev_ema, float **prev_closes, int *prev_close_count, int sma_period, int ema_period);
int calculate_structure(Candle *candles, int pos, int lookback);

Datetime get_candle_datetime(Candle *candles, int pos);
float get_open(Candle *candles, int pos);
float get_high(Candle *candles, int pos);
float get_low(Candle *candles, int pos);
float get_close(Candle *candles, int pos);
double get_real_volume(Candle *candles, int pos);
float get_daily_low_up_to_current_candle(Candle *candles, int pos);
float get_daily_high_up_to_current_candle(Candle *candles, int pos);
Candle_Color get_color(Candle *candles, int pos);
int get_structure(Candle *candles, int pos);
float get_sma(Candle *candles, int pos);
float get_ema(Candle *candles, int pos);
Tape_Reading *get_price_volumes(Candle *candles, int pos);
float get_total_agg_volume_buy(Candle *candles, int pos);
float get_total_agg_volume_sell(Candle *candles, int pos);
float get_agg_delta(Candle *candles, int pos);
float get_avg_delta(Candle *candles, int pos);
float get_std_delta(Candle *candles, int pos);

float get_price_from_times_and_trades(Candle *candles, int pos, int row);
Datetime get_candle_times_and_trades_datetime(Candle *candles, int pos, int row);
int get_candle_times_and_trades_row(Candle *candles, int pos, int row);

size_t __get_candle_times_and_trades_size(Candle *candles, int pos);
size_t __get_candles_size(Candle *candles);
void __free_candles(Candle *candles);
void __print_candles(Candle *candles);

#endif // CANDLES_H_
