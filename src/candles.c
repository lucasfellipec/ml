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
    float total_agg_volume_buy;
    float total_agg_volume_sell;
    float agg_delta;
    float avg_delta;
    float avg_delta_per_agg;
    float std_delta;
    float avg_volume_buy;
    float avg_volume_sell;
    float candle_displacement;
    float candle_impact;
    float avg_candle_impact;
    float expected_movement;
    float real_movement;
    float buy_agg_per_displacement;
    float sell_agg_per_displacement;
    float delta_per_displacement;
    float avg_buy_agg_per_displacement;
    float avg_sell_agg_per_displacement;
    float avg_delta_per_displacement;
    float avg_volume_buy_variance;
    float avg_volume_sell_variance;
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

float get_total_agg_volume_buy(Candle *candles, int pos) {
    return candles[pos].total_agg_volume_buy;
}

float get_total_agg_volume_sell(Candle *candles, int pos) {
    return candles[pos].total_agg_volume_sell;
}

float get_agg_delta(Candle *candles, int pos) {
    return candles[pos].agg_delta;
}

float get_avg_delta(Candle *candles, int pos) {
    return candles[pos].avg_delta;
}

float get_std_delta(Candle *candles, int pos) {
    return candles[pos].std_delta;
}

float get_avg_volume_buy(Candle *candles, int pos) {
    return candles[pos].avg_volume_buy;
}

float get_avg_volume_sell(Candle *candles, int pos) {
    return candles[pos].avg_volume_sell;
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
                candles[j].avg_delta = 0;
                candles[j].avg_delta_per_agg = 0;
                candles[j].std_delta = 0;
                candles[j].candle_displacement = candles[j].close - candles[j].open;
                candles[j].avg_candle_impact = 0;
                candles[j].real_movement = candles[j].close - candles[j].open;
                candles[j].buy_agg_per_displacement = 0;
                candles[j].sell_agg_per_displacement = 0;
                candles[j].delta_per_displacement = 0;

                Tape_Reading *tr = malloc(count * sizeof(Tape_Reading));
                size_t tr_count = 0;

                float total_agg_volume_buy = 0;
                float total_agg_volume_sell = 0;

                for (int k = 0; k < count; k++) {
                    candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].pos = k;

                    float price = get_times_and_trades_price(times_and_trades, i - count + k);
                    float volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    float volume_buy = 0;
                    float volume_sell = 0;
                    char agg = get_times_and_trades_agg(times_and_trades, i - count + k);

                    if (agg == 'C') {
                        total_agg_volume_buy += volume;
                        volume_buy = volume;
                    }
                    if (agg == 'V') {
                        total_agg_volume_sell += volume;
                        volume_sell = volume;
                    }

                    int found = 0;

                    for (int m = 0; m < tr_count; m++) {
                        if (fabs(tr[m].price - price) < 0.0001 && agg == 'C') {
                            tr[m].volume_buy += volume_buy;
                            found = 1;
                            break;
                        }
                        if (fabs(tr[m].price - price) < 0.0001 && agg == 'V') {
                            tr[m].volume_sell += volume_sell;
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        tr[tr_count].price = price;
                        tr[tr_count].volume_buy = volume_buy;
                        tr[tr_count].volume_sell = volume_sell;
                        tr[tr_count].agg = agg;
                        tr_count++;
                    }
                }

                candles[j].candle_times_and_trades_rows = count;
                candles[j].total_agg_volume_buy = total_agg_volume_buy;
                candles[j].total_agg_volume_sell = total_agg_volume_sell;
                candles[j].agg_delta = total_agg_volume_buy - total_agg_volume_sell;
                candles[j].price_volumes = realloc(tr, tr_count * sizeof(Tape_Reading));
                candles[j].price_volumes_rows = tr_count;
                candles[j].candle_impact = candles[j].candle_displacement / candles[j].agg_delta;
                if (candles[j].candle_displacement != 0) {
                    candles[j].buy_agg_per_displacement = total_agg_volume_buy / fabs(candles[j].candle_displacement);
                    candles[j].sell_agg_per_displacement = total_agg_volume_sell / fabs(candles[j].candle_displacement);
                    candles[j].delta_per_displacement = fabs(candles[j].agg_delta) / fabs(candles[j].candle_displacement);
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
        candles[j].avg_delta = 0;
        candles[j].avg_delta_per_agg = 0;
        candles[j].std_delta = 0;
        candles[j].candle_displacement = (candles[j].close - candles[j].open);
        candles[j].candle_impact = 0;

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
            float sum_volume_buy = 0;
            float sum_volume_sell = 0;
            candles[k].avg_volume_buy = 0;
            candles[k].avg_volume_sell = 0;
            float price = candles[k].price_volumes[x].price;
            int count = 0;
            // float sum_sq_buy = 0;
            // float sum_sq_sell = 0;
            for (int l = k - 1; l >= 0; l--) {
                for (int y = 0; y < candles[l].price_volumes_rows; y++) {
                    sum_volume_buy += candles[l].price_volumes[y].volume_buy;
                    sum_volume_sell += candles[l].price_volumes[y].volume_sell;
                    if (candles[l].price_volumes[y].agg == 'L' && candles[l].price_volumes[y].volume_buy == 0 && candles[l].price_volumes[y].volume_sell == 0) {
                        continue;
                    }
                    count++;
                }
            }
            if (count > 0) {
                candles[k].avg_volume_buy = sum_volume_buy / count;
                candles[k].avg_volume_sell = sum_volume_sell / count;
                // for (int l = k - 1; l >= 0; l--) {
                //     for (int y = 0; y < candles[l].price_volumes_rows; y++) {
                //         if (candles[l].price_volumes[y].agg == 'L' && candles[l].price_volumes[y].volume_buy == 0 && candles[l].price_volumes[y].volume_sell == 0) {
                //             continue;
                //         }
                //         double diff_buy = candles[l].price_volumes[y].volume_buy - candles[k].avg_volume_buy;
                //         double diff_sell = candles[l].price_volumes[y].volume_sell - candles[k].avg_volume_sell;
                //         sum_sq_buy += diff_buy * diff_buy;
                //         sum_sq_sell += diff_sell * diff_sell;
                //     }
                // }
                // float variance_buy = sum_sq_buy / count;
                // float variance_sell = sum_sq_sell / count;
                // candles[k].price_volumes[x].std_volume_buy = (float)sqrt(variance_buy);
                // candles[k].price_volumes[x].std_volume_sell = (float)sqrt(variance_sell);
            }
        }
    }


    for (int k = 1; k < j; k++) {
        float displacement_sum = 0;
        float delta_sum = 0;
        for (int l = k - 1; l >= 0; l--) {
            displacement_sum += candles[l].candle_displacement;
            delta_sum += candles[l].agg_delta;
        }
        candles[k].avg_candle_impact = displacement_sum / delta_sum;
        candles[k].expected_movement = candles[k].agg_delta * candles[k].avg_candle_impact;
    }

    for (int k = 1; k < j; k++) {
        float sum_delta = 0;
        float sum_delta_buy = 0;
        float sum_delta_sell = 0;
        float variance = 0;
        for (int l = k - 1; l >= 0; l--) {
            sum_delta += fabs(candles[l].agg_delta);
        }
        candles[k].avg_delta = sum_delta / k;
        for (int l = k - 1; l >= 0; l--) {
            float diff = candles[l].agg_delta - candles[k].avg_delta;
            variance += diff * diff;
        }
        variance /= k;
        candles[k].std_delta = sqrt(variance);
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
            sum_buy_agg += candles[l].total_agg_volume_buy;
            sum_sell_agg += candles[l].total_agg_volume_sell;
            sum_delta += fabs(candles[l].agg_delta);
            sum_displacement += fabs(candles[l].candle_displacement);
        }
        candles[k].avg_buy_agg_per_displacement = sum_buy_agg / sum_displacement;
        candles[k].avg_sell_agg_per_displacement = sum_sell_agg / sum_displacement;
        candles[k].avg_delta_per_displacement = sum_delta / sum_displacement;
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

void __generate_footprint_csv(Candle *candles) {
    for (size_t i = 0; i < candles->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d\t", candles[i].datetime.year, candles[i].datetime.month, candles[i].datetime.day, candles[i].datetime.hour, candles[i].datetime.minute, candles[i].datetime.second);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].total_agg_volume_buy);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].total_agg_volume_sell);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].agg_delta);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", fabs(candles[i].agg_delta));
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].open);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].close);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", candles[i].candle_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%.1f\t", fabs(candles[i].candle_displacement));
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].candle_impact);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_candle_impact);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].expected_movement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].real_movement);
    }
    printf("\n");
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].buy_agg_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_buy_agg_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_volume_buy);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].sell_agg_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_sell_agg_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_volume_buy_variance);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_volume_sell);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].delta_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_delta_per_displacement);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        printf("%f\t", candles[i].avg_volume_sell_variance);
    }
    printf("\n");
    for (size_t i = 0; i < candles->size; i++) {
        int buy = 0;
        int sell = 0;
        if (candles[i].real_movement < candles[i].expected_movement
            && candles[i].buy_agg_per_displacement > candles[i - 1].avg_buy_agg_per_displacement
            && candles[i].delta_per_displacement > candles[i - 1].avg_delta_per_displacement
            && candles[i].agg_delta > 0) {
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].volume_buy > 3 * candles[i].avg_volume_buy) {
                    sell = 1;
                    break;
                }
            }
        }
        if (candles[i].real_movement > candles[i].expected_movement
            && candles[i].sell_agg_per_displacement > candles[i - 1].avg_sell_agg_per_displacement
            && candles[i].delta_per_displacement > candles[i - 1].avg_delta_per_displacement
            && candles[i].agg_delta < 0) {
            int found = 0;
            for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
                if (candles[i].price_volumes[j].volume_sell > 3 * candles[i].avg_volume_sell) {
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
            candles[i].total_agg_volume_buy,
            candles[i].total_agg_volume_sell,
            candles[i].agg_delta,
            candles[i].avg_delta,
            candles[i].std_delta,
            candles[i].avg_volume_buy,
            candles[i].avg_volume_sell,
            candles[i].candle_displacement,
            candles[i].candle_impact,
            candles[i].avg_candle_impact,
            candles[i].expected_movement,
            candles[i].real_movement,
            candles[i].buy_agg_per_displacement,
            candles[i].sell_agg_per_displacement,
            candles[i].delta_per_displacement,
            candles[i].avg_buy_agg_per_displacement,
            candles[i].avg_sell_agg_per_displacement,
            candles[i].avg_delta_per_displacement
        );
        for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
            printf("\t%.1f\t%.1f\t%.1f\t%c\n",
                candles[i].price_volumes[j].price,
                candles[i].price_volumes[j].volume_buy,
                candles[i].price_volumes[j].volume_sell,
                candles[i].price_volumes[j].agg
            );
        }
    }
}
