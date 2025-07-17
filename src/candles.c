#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "times_and_trades.h"
#include "candles.h"
#include "datetime.h"

typedef struct {
    Datetime datetime;
    float price;
    int pos;
} Candle_Times_And_Trades;

struct Candles {
    Datetime datetime;
    float open;
    float high;
    float low;
    float close;
    double real_volume;
    Candle_Color color;
    int structure;
    size_t size;
    Candle_Times_And_Trades *candle_times_and_trades;
    size_t candle_times_and_trades_rows;
};

Datetime get_candle_datetime(Candles *candles, int pos) {
    return candles[pos].datetime;
}

float get_open(Candles *candles, int pos) {
    return candles[pos].open;
}

float get_high(Candles *candles, int pos) {
    return candles[pos].high;
}

float get_low(Candles *candles, int pos) {
    return candles[pos].low;
}

float get_close(Candles *candles, int pos) {
    return candles[pos].close;
}

Datetime get_candle_times_and_trades_datetime(Candles *candles, int pos, int row) {
    return candles[pos].candle_times_and_trades[row].datetime;
}

float get_price_from_times_and_trades(Candles *candles, int pos, int row) {
    return candles[pos].candle_times_and_trades[row].price;
}

int calculate_structure(Candles *candles, int pos, int lookback) {
    if (pos < lookback - 1) {
        return 0;
    }

    int hh_count = 0;
    int hl_count = 0;
    int lh_count = 0;
    int ll_count = 0;

    for (int p = pos - lookback + 2; p <= pos; p++) {
        if (candles[p].high > candles[p - 1].high) {
            hh_count++;
        }
        if (candles[p].low > candles[p - 1].low) {
            hl_count++;
        }
        if (candles[p].high < candles[p - 1].high) {
            lh_count++;
        }
        if (candles[p].low < candles[p - 1].low) {
            ll_count++;
        }
    }

    int comparisons = lookback - 1;

    if (hh_count > comparisons / 2 && hl_count > comparisons / 2) {
        return 1;
    }
    if (lh_count > comparisons / 2 && ll_count > comparisons / 2) {
        return -1;
    }

    return 0;
}

Candles *generate_candles(Times_And_Trades *times_and_trades, int timeframe) {
    time_t current_interval_start = 0;

    int initial_capacity = 1024;
    Candles *candles = malloc(initial_capacity * sizeof(Candles));

    if (!candles) {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    int capacity = initial_capacity;
    int count = 0;
    int j = 0;

    int price = 0;
    int open = 0;
    int high = 0;
    int low = 0;
    int close = 0;

    int rows = __get_times_and_trades_size(times_and_trades);

    for (int i = 0; i < rows; i++) {
        time_t row_time = timestamp_to_unix(get_times_and_trades_datetime(times_and_trades, i));
        time_t interval_start = (row_time / timeframe) * timeframe;

        if (interval_start != current_interval_start) {
            if (count > 0) {
                if (j == capacity) {
                    capacity *= 2;
                    Candles *tmp = realloc(candles, capacity * sizeof(Candles));
                    if (!tmp) {
                        fprintf(stderr, "Error allocating memory\n");
                        free(candles);
                        return NULL;
                    }
                    candles = tmp;
                }
                candles[j].datetime = time_to_timestamp(current_interval_start);
                candles[j].open = (float)open / 10.0;
                candles[j].high = (float)high / 10.0;
                candles[j].low = (float)low / 10.0;
                candles[j].close = (float)close / 10.0;
                candles[j].color = (open > close ? RED : open < close ? GREEN : DOJI);
                candles[j].candle_times_and_trades = malloc(count * sizeof(Candle_Times_And_Trades));

                for (int k = 0; k < count; k++) {
                    candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].pos = k;
                }

                candles[j].candle_times_and_trades_rows = count;
                j++;
            }
            current_interval_start = interval_start;
            price = (int)(get_times_and_trades_price(times_and_trades, i) * 10);
            open = price;
            high = (int)-1e9;
            low = (int)1e9;
            count = 0;
        }
        price = (int)(get_times_and_trades_price(times_and_trades, i) * 10);

        if (price > high) {
            high = price;
        }
        if (price < low) {
            low = price;
        }

        close = price;
        count++;
    }

    if (count > 0) {
        if (j == capacity) {
            capacity *= 2;
            Candles *tmp = realloc(candles, capacity * sizeof(Candles));
            if (!tmp) {
                fprintf(stderr, "Error allocating memory\n");
                free(candles);
                return NULL;
            }
            candles = tmp;
        }
        candles[j].datetime = time_to_timestamp(current_interval_start);
        candles[j].open = (float)open / 10.0;
        candles[j].high = (float)high / 10.0;
        candles[j].low = (float)low / 10.0;
        candles[j].close = (float)close / 10.0;
        candles[j].color = (open > close ? RED : open < close ? GREEN : DOJI);

        candles[j].candle_times_and_trades = malloc(count * sizeof(Candle_Times_And_Trades));

        for (int k = 0; k < count; k++) {
            candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, rows - count + k);
            candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, rows - count + k);
            candles[j].candle_times_and_trades[k].pos = k;
        }

        candles[j].candle_times_and_trades_rows = count;
        j++;
    }

    candles->size = j;

    const int LOOKBACK = 10;

    for (int k = 0; k < j; k++) {
        candles[k].structure = calculate_structure(candles, k, LOOKBACK);
    }

    return candles;
}

size_t __get_candles_size(Candles *candles) {
    return candles->size;
}

size_t __get_candle_times_and_trades_size(Candles *candles, int pos) {
    return candles[pos].candle_times_and_trades_rows;
}

void __free_candles(Candles *candles) {
    for (size_t i = 0; i < candles->size; i++) {
        free(candles[i].candle_times_and_trades);
    }
    free(candles);
}

void __print_candles(Candles *candles) {
    printf("Datetime\tOpen\tHigh\tLow\tClose\tStructure\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d\t%.1f\t%.1f\t%.1f\t%.1f\t%d\n",
            candles[i].datetime.year,
            candles[i].datetime.month,
            candles[i].datetime.day,
            candles[i].datetime.hour,
            candles[i].datetime.minute,
            candles[i].datetime.second,
            candles[i].open,
            candles[i].high,
            candles[i].low,
            candles[i].close,
            candles[i].structure
        );
    }
}
