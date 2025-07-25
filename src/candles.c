#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "candles.h"
#include "datetime.h"
#include "tape_reading.h"
#include "times_and_trades.h"

typedef struct {
    Datetime datetime;
    float price;
    float volume;
    int pos;
} Candle_Times_And_Trades;

struct Candle {
    Datetime datetime;
    float open;
    float high;
    float low;
    float close;
    double real_volume;
    float daily_low_up_to_current_candle;
    float daily_high_up_to_current_candle;
    float sma;
    float ema;
    Candle_Color color;
    int structure;
    size_t size;
    Candle_Times_And_Trades *candle_times_and_trades;
    size_t candle_times_and_trades_rows;
    Tape_Reading *price_volumes;
    size_t price_volumes_rows;
    float total_buy_agg_volume;
    float total_sell_agg_volume;
    float delta_agg;
    float delta_avg;
    float delta_avg_per_agg;
    float delta_std;
    float buy_volume_avg;
    float sell_volume_avg;
    float displacement;
    float impact;
    float impact_avg;
    float expected_movement;
    float real_movement;
    float buy_agg_per_displacement;
    float sell_agg_per_displacement;
    float delta_agg_per_displacement;
    float buy_agg_per_displacement_avg;
    float sell_agg_per_displacement_avg;
    float delta_agg_per_displacement_avg;
    char trigger;
};

Datetime get_candle_datetime(Candle *candles, int pos) {
    return candles[pos].datetime;
}

float get_open(Candle *candles, int pos) {
    return candles[pos].open;
}

float get_high(Candle *candles, int pos) {
    return candles[pos].high;
}

float get_low(Candle *candles, int pos) {
    return candles[pos].low;
}

float get_close(Candle *candles, int pos) {
    return candles[pos].close;
}

double get_real_volume(Candle *candles, int pos) {
    return candles[pos].real_volume;
}

float get_daily_low_up_to_current_candle(Candle *candles, int pos) {
    return candles[pos].daily_low_up_to_current_candle;
}

float get_daily_high_up_to_current_candle(Candle *candles, int pos) {
    return candles[pos].daily_high_up_to_current_candle;
}

Candle_Color get_color(Candle *candles, int pos) {
    return candles[pos].color;
}

int get_structure(Candle *candles, int pos) {
    return candles[pos].structure;
}

float get_sma(Candle *candles, int pos) {
    return candles[pos].sma;
}

float get_ema(Candle *candles, int pos) {
    return candles[pos].ema;
}

Tape_Reading *get_price_volumes(Candle *candles, int pos) {
    return candles[pos].price_volumes;
}

float get_total_buy_agg_volume(Candle *candles, int pos) {
    return candles[pos].total_buy_agg_volume;
}

float get_total_sell_agg_volume(Candle *candles, int pos) {
    return candles[pos].total_sell_agg_volume;
}

float get_delta_agg(Candle *candles, int pos) {
    return candles[pos].delta_agg;
}

float get_delta_avg(Candle *candles, int pos) {
    return candles[pos].delta_avg;
}

float get_delta_std(Candle *candles, int pos) {
    return candles[pos].delta_std;
}

float get_buy_volume_avg(Candle *candles, int pos) {
    return candles[pos].buy_volume_avg;
}

float get_sell_volume_avg(Candle *candles, int pos) {
    return candles[pos].sell_volume_avg;
}

Datetime get_candle_times_and_trades_datetime(Candle *candles, int pos, int row) {
    return candles[pos].candle_times_and_trades[row].datetime;
}

float get_price_from_times_and_trades(Candle *candles, int pos, int row) {
    return candles[pos].candle_times_and_trades[row].price;
}

int get_candle_times_and_trades_row(Candle *candles, int pos, int row) {
    return candles[pos].candle_times_and_trades[row].pos;
}

char get_trigger(Candle *candles, int pos) {
    return candles[pos].trigger;
}

int calculate_structure(Candle *candles, int pos, int lookback) {
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

Candle *generate_candles(Times_And_Trades *times_and_trades, int timeframe, float *prev_ema, float **prev_closes, int *prev_close_count, int sma_period, int ema_period) {
    time_t current_interval_start = 0;

    int initial_capacity = 1024;
    Candle *candles = malloc(initial_capacity * sizeof(Candle));

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
    int daily_low_up_to_current_candle = (int)1e9;
    int daily_high_up_to_current_candle = (int)-1e9;

    int rows = __get_times_and_trades_size(times_and_trades);

    for (int i = 0; i < rows; i++) {
        time_t row_time = timestamp_to_unix(get_times_and_trades_datetime(times_and_trades, i));
        time_t interval_start = (row_time / timeframe) * timeframe;

        if (interval_start != current_interval_start) {
            if (count > 0) {
                if (j == capacity) {
                    capacity *= 2;
                    Candle *tmp = realloc(candles, capacity * sizeof(Candle));
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
                candles[j].daily_high_up_to_current_candle = (float)daily_high_up_to_current_candle / 10.0;
                candles[j].daily_low_up_to_current_candle = (float)daily_low_up_to_current_candle / 10.0;
                candles[j].delta_avg = 0;
                candles[j].delta_avg_per_agg = 0;
                candles[j].delta_std = 0;
                candles[j].displacement = candles[j].close - candles[j].open;
                candles[j].impact_avg = 0;
                candles[j].real_movement = candles[j].close - candles[j].open;
                candles[j].buy_agg_per_displacement = 0;
                candles[j].sell_agg_per_displacement = 0;
                candles[j].delta_agg_per_displacement = 0;

                Tape_Reading *tr = malloc(count * sizeof(Tape_Reading));
                size_t tr_count = 0;

                float total_buy_agg_volume = 0;
                float total_sell_agg_volume = 0;

                for (int k = 0; k < count; k++) {
                    candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].pos = k;

                    float price = get_times_and_trades_price(times_and_trades, i - count + k);
                    float volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    float buy_volume = 0;
                    float sell_volume = 0;
                    char agg = get_times_and_trades_agg(times_and_trades, i - count + k);

                    if (agg == 'C') {
                        total_buy_agg_volume += volume;
                        buy_volume = volume;
                    }
                    if (agg == 'V') {
                        total_sell_agg_volume += volume;
                        sell_volume = volume;
                    }

                    int found = 0;

                    for (int m = 0; m < tr_count; m++) {
                        if (fabs(tr[m].price - price) < 0.0001 && agg == 'C') {
                            tr[m].buy_volume += buy_volume;
                            found = 1;
                            break;
                        }
                        if (fabs(tr[m].price - price) < 0.0001 && agg == 'V') {
                            tr[m].sell_volume += sell_volume;
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        tr[tr_count].price = price;
                        tr[tr_count].buy_volume = buy_volume;
                        tr[tr_count].sell_volume = sell_volume;
                        tr[tr_count].agg = agg;
                        tr_count++;
                    }
                }

                candles[j].candle_times_and_trades_rows = count;
                candles[j].total_buy_agg_volume = total_buy_agg_volume;
                candles[j].total_sell_agg_volume = total_sell_agg_volume;
                candles[j].delta_agg = total_buy_agg_volume - total_sell_agg_volume;
                candles[j].price_volumes = realloc(tr, tr_count * sizeof(Tape_Reading));
                candles[j].price_volumes_rows = tr_count;
                candles[j].impact = candles[j].displacement / candles[j].delta_agg;
                if (candles[j].displacement != 0) {
                    candles[j].buy_agg_per_displacement = total_buy_agg_volume / fabs(candles[j].displacement);
                    candles[j].sell_agg_per_displacement = total_sell_agg_volume / fabs(candles[j].displacement);
                    candles[j].delta_agg_per_displacement = fabs(candles[j].delta_agg) / fabs(candles[j].displacement);
                }

                if (high > daily_high_up_to_current_candle) {
                    daily_high_up_to_current_candle = high;
                }
                if (low < daily_low_up_to_current_candle) {
                    daily_low_up_to_current_candle = low;
                }


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
            Candle *tmp = realloc(candles, capacity * sizeof(Candle));
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
        candles[j].daily_low_up_to_current_candle = 0;
        candles[j].daily_high_up_to_current_candle = 0;
        candles[j].delta_avg = 0;
        candles[j].delta_avg_per_agg = 0;
        candles[j].delta_std = 0;
        candles[j].displacement = (candles[j].close - candles[j].open);
        candles[j].impact = 0;

        candles[j].candle_times_and_trades = malloc(count * sizeof(Candle_Times_And_Trades));

        for (int k = 0; k < count; k++) {
            candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, rows - count + k);
            candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, rows - count + k);
            candles[j].candle_times_and_trades[k].volume = get_times_and_trades_volume(times_and_trades, rows - count + k);
            candles[j].candle_times_and_trades[k].pos = k;
        }

        candles[j].candle_times_and_trades_rows = count;
        candles[j].price_volumes = NULL;
        candles[j].price_volumes_rows = 0;

        j++;
    }

    candles->size = j;

    const int LOOKBACK = 20;

    for (int k = 1; k < j; k++) {
        for (int x = 0; x < candles[k].price_volumes_rows; x++) {
            float sum_buy_volume = 0;
            float sum_sell_volume = 0;
            candles[k].buy_volume_avg = 0;
            candles[k].sell_volume_avg = 0;
            float price = candles[k].price_volumes[x].price;
            int count = 0;
            for (int l = k - 1; l >= 0; l--) {
                for (int y = 0; y < candles[l].price_volumes_rows; y++) {
                    sum_buy_volume += candles[l].price_volumes[y].buy_volume;
                    sum_sell_volume += candles[l].price_volumes[y].sell_volume;
                    // if (candles[l].price_volumes[y].agg == 'L' && candles[l].price_volumes[y].buy_volume == 0 && candles[l].price_volumes[y].sell_volume == 0) {
                    //     continue;
                    // }
                    count++;
                }
            }
            if (count > 0) {
                candles[k].buy_volume_avg = sum_buy_volume / count;
                candles[k].sell_volume_avg = sum_sell_volume / count;
            }
        }
    }


    for (int k = 1; k < j; k++) {
        float displacement_sum = 0;
        float delta_sum = 0;
        for (int l = k - 1; l >= 0; l--) {
            displacement_sum += candles[l].displacement;
            delta_sum += candles[l].delta_agg;
        }
        candles[k].impact_avg = displacement_sum / delta_sum;
        candles[k].expected_movement = candles[k].delta_agg * candles[k].impact_avg;
    }

    for (int k = 1; k < j; k++) {
        float sum_delta = 0;
        float sum_delta_buy = 0;
        float sum_delta_sell = 0;
        float variance = 0;
        for (int l = k - 1; l >= 0; l--) {
            sum_delta += fabs(candles[l].delta_agg);
        }
        candles[k].delta_avg = sum_delta / k;
        for (int l = k - 1; l >= 0; l--) {
            float diff = candles[l].delta_agg - candles[k].delta_avg;
            variance += diff * diff;
        }
        variance /= k;
        candles[k].delta_std = sqrt(variance);
    }

    for (int k = 0; k < j; k++) {
        candles[k].structure = calculate_structure(candles, k, LOOKBACK);
    }

    for (int k = 0; k < j; k++) {
        float sum_buy_agg = 0;
        float sum_sell_agg = 0;
        float sum_delta = 0;
        float sum_displacement = 0;
        for (int l = k; l >= 0; l--) {
            sum_buy_agg += candles[l].total_buy_agg_volume;
            sum_sell_agg += candles[l].total_sell_agg_volume;
            sum_delta += fabs(candles[l].delta_agg);
            sum_displacement += fabs(candles[l].displacement);
        }
        candles[k].buy_agg_per_displacement_avg = sum_buy_agg / sum_displacement;
        candles[k].sell_agg_per_displacement_avg = sum_sell_agg / sum_displacement;
        candles[k].delta_agg_per_displacement_avg = sum_delta / sum_displacement;
    }

    float alpha = 2.0 / (ema_period + 1);

    for (int k = 0; k < j; k++) {
        float current_close = candles[k].close;

        if (k == 0 && *prev_ema == 0) {
            candles[k].ema = current_close;
        } else {
            candles[k].ema = current_close * alpha + *prev_ema * (1 - alpha);
        }

        *prev_ema = candles[k].ema;

        if (*prev_close_count >= sma_period) {
            memmove(*prev_closes, *prev_closes + 1, (sma_period - 1) * sizeof(float));
            (*prev_closes)[sma_period - 1] = current_close;
        } else {
            (*prev_closes)[*prev_close_count] = current_close;
            (*prev_close_count)++;
        }

        if (*prev_close_count >= sma_period) {
            float sum = 0;

            for (int i = 0; i < sma_period; i++) {
                sum += (*prev_closes)[i];
            }

            candles[k].sma = sum / sma_period;
        } else {
            candles[k].sma = 0;
        }
    }

    return candles;
}

size_t __get_candles_size(Candle *candles) {
    return candles->size;
}

size_t __get_candle_times_and_trades_size(Candle *candles, int pos) {
    return candles[pos].candle_times_and_trades_rows;
}

void __free_candles(Candle *candles) {
    for (size_t i = 0; i < candles->size; i++) {
        free(candles[i].candle_times_and_trades);
        free(candles[i].price_volumes);
    }
    free(candles);
}

void generate_triggers(Candle *candles) {
    for (size_t i = 0; i < candles->size; i++) {
        int buy = 0;
        int sell = 0;
        if (candles[i].real_movement < candles[i].expected_movement
            && candles[i].buy_agg_per_displacement > candles[i - 1].buy_agg_per_displacement_avg
            && candles[i].delta_agg_per_displacement > candles[i - 1].delta_agg_per_displacement_avg
            && candles[i].delta_agg > 0) {
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].buy_volume > candles[i].buy_volume_avg) {
                    sell = 1;
                    break;
                }
            }
        }
        if (candles[i].real_movement > candles[i].expected_movement
            && candles[i].sell_agg_per_displacement > candles[i - 1].sell_agg_per_displacement_avg
            && candles[i].delta_agg_per_displacement > candles[i - 1].delta_agg_per_displacement_avg
            && candles[i].delta_agg < 0) {
            int found = 0;
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].sell_volume > candles[i].sell_volume_avg) {
                    buy = 1;
                    break;
                }
            }
        }
        if (sell) {
            candles[i].trigger = 'V';
        } else if (buy) {
            candles[i].trigger = 'C';
        } else {
            candles[i].trigger = '-';
        }
    }
}

void __generate_footprint(Candle *candles) {
    printf("Datetime\t"
           "Total_Buy_Agg_Volume\t"
           "Total_Sell_Agg_Volume\t"
           "Delta_Agg\t"
           "Delta_Agg_Abs\t"
           "Open\t"
           "High\t"
           "Low\t"
           "Close\t"
           "Structure\t"
           "Displacement\t"
           "Displacement_Abs\t"
           "Impact\t"
           "Impact_Avg\t"
           "Expected_Movement\t"
           "Daily_High\t"
           "Daily_Low\t"
           "Real_Movement\t"
           "Buy_Agg_Per_Displacement\t"
           "Buy_Agg_Per_Displacement_Avg\t"
           "Buy_Volume_Avg\t"
           "Sell_Agg_Per_Displacement\t"
           "Sell_Agg_Per_Displacement_Avg\t"
           "Sell_Volume_Avg\t"
           "Delta_Agg_Per_Displacement\t"
           "Delta_Agg_Per_Displacement_Avg\t"
           "Op\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", candles[i].datetime.year, candles[i].datetime.month, candles[i].datetime.day, candles[i].datetime.hour, candles[i].datetime.minute, candles[i].datetime.second);
        printf("%.1f\t", candles[i].total_buy_agg_volume);
        printf("%.1f\t", candles[i].total_sell_agg_volume);
        printf("%.1f\t", candles[i].delta_agg);
        printf("%.1f\t", fabs(candles[i].delta_agg));
        printf("%.1f\t", candles[i].open);
        printf("%.1f\t", candles[i].high);
        printf("%.1f\t", candles[i].low);
        printf("%.1f\t", candles[i].close);
        printf("%d\t", candles[i].structure);
        printf("%.1f\t", candles[i].displacement);
        printf("%.1f\t", fabs(candles[i].displacement));
        printf("%f\t", candles[i].impact);
        printf("%f\t", candles[i].impact_avg);
        printf("%f\t", candles[i].expected_movement);
        printf("%f\t", candles[i].daily_high_up_to_current_candle);
        printf("%f\t", candles[i].daily_low_up_to_current_candle);
        printf("%f\t", candles[i].real_movement);
        printf("%f\t", candles[i].buy_agg_per_displacement);
        printf("%f\t", candles[i].buy_agg_per_displacement_avg);
        printf("%f\t", candles[i].buy_volume_avg);
        printf("%f\t", candles[i].sell_agg_per_displacement);
        printf("%f\t", candles[i].sell_agg_per_displacement_avg);
        printf("%f\t", candles[i].sell_volume_avg);
        printf("%f\t", candles[i].delta_agg_per_displacement);
        printf("%f\t", candles[i].delta_agg_per_displacement_avg);
        int buy = 0;
        int sell = 0;
        if (candles[i].real_movement < candles[i].expected_movement
            && candles[i].buy_agg_per_displacement > candles[i - 1].buy_agg_per_displacement_avg
            && candles[i].delta_agg_per_displacement > candles[i - 1].delta_agg_per_displacement_avg
            && candles[i].delta_agg > 0) {
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].buy_volume > candles[i].buy_volume_avg) {
                    sell = 1;
                    break;
                }
            }
        }
        if (candles[i].real_movement > candles[i].expected_movement
            && candles[i].sell_agg_per_displacement > candles[i - 1].sell_agg_per_displacement_avg
            && candles[i].delta_agg_per_displacement > candles[i - 1].delta_agg_per_displacement_avg
            && candles[i].delta_agg < 0) {
            int found = 0;
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].sell_volume > candles[i].sell_volume_avg) {
                    buy = 1;
                    break;
                }
            }
        }
        if (sell) {
            printf("V\t");
        } else if (buy) {
            printf("C\t");
        } else {
            printf("-\t");
        }
        printf("\n");
    }
}

void __print_candles(Candle *candles) {
    printf("Datetime\tOpen\tHigh\tLow\tClose\tStructure\tDailyHigh\tDailyLow\tSMA\tEMA\tTotal_Agg_Volume_Buy\tTotal_Agg_Volume_Sell\tAgg_Delta\tAvg_Delta\tStd_Delta\tAvg_Volume_Buy\tAvg_Volume_Sell\tCandle_Displacement\tCandle_Impact\tAvg_Candle_Impact\tExpected_Movement\tReal_Movement\tBuy_Agg_Per_Displacement\tSell_Agg_Per_Displacement\tDelta_Agg_Per_Displacement\tBuy_Mad2\tSell_Mad2\tDelta_Mad2\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d\t%.1f\t%.1f\t%.1f\t%.1f\t%d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%.4f\t%.4f\t%.4f\n",
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
            candles[i].structure,
            candles[i].daily_high_up_to_current_candle,
            candles[i].daily_low_up_to_current_candle,
            candles[i].sma,
            candles[i].ema,
            candles[i].total_buy_agg_volume,
            candles[i].total_sell_agg_volume,
            candles[i].delta_agg,
            candles[i].delta_avg,
            candles[i].delta_std,
            candles[i].buy_volume_avg,
            candles[i].sell_volume_avg,
            candles[i].displacement,
            candles[i].impact,
            candles[i].impact_avg,
            candles[i].expected_movement,
            candles[i].real_movement,
            candles[i].buy_agg_per_displacement,
            candles[i].sell_agg_per_displacement,
            candles[i].delta_agg_per_displacement,
            candles[i].buy_agg_per_displacement_avg,
            candles[i].sell_agg_per_displacement_avg,
            candles[i].delta_agg_per_displacement_avg
        );
        // for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
        //     printf("\t%.1f\t%.1f\t%.1f\t%c\n",
        //         candles[i].price_volumes[j].price,
        //         candles[i].price_volumes[j].buy_volume,
        //         candles[i].price_volumes[j].sell_volume,
        //         candles[i].price_volumes[j].agg
        //     );
        // }
    }
}
