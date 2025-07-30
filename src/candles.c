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
    float daily_low_until_current_candle;
    float daily_high_until_current_candle;
    float simple_moving_average;
    float exponential_moving_average;
    Candle_Color color;
    size_t size;
    Candle_Times_And_Trades *candle_times_and_trades;
    size_t candle_times_and_trades_rows;
    Tape_Reading *price_volumes;
    size_t price_volumes_rows;
    float total_buy_aggression_volume;
    float total_sell_aggression_volume;
    float delta_aggression;
    float delta_average;
    float delta_std;
    float buy_volume_average;
    float sell_volume_average;
    float displacement;
    float impact;
    float impact_average;
    float expected_movement;
    float real_movement;
    float buy_aggression_per_displacement;
    float sell_aggression_per_displacement;
    float delta_aggression_per_displacement;
    float buy_aggression_per_displacement_average;
    float sell_aggression_per_displacement_average;
    float delta_aggression_per_displacement_average;
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

float get_daily_low_until_current_candle(Candle *candles, int pos) {
    return candles[pos].daily_low_until_current_candle;
}

float get_daily_high_until_current_candle(Candle *candles, int pos) {
    return candles[pos].daily_high_until_current_candle;
}

Candle_Color get_color(Candle *candles, int pos) {
    return candles[pos].color;
}

float get_simple_moving_average(Candle *candles, int pos) {
    return candles[pos].simple_moving_average;
}

float get_exponential_moving_average(Candle *candles, int pos) {
    return candles[pos].exponential_moving_average;
}

Tape_Reading *get_price_volumes(Candle *candles, int pos) {
    return candles[pos].price_volumes;
}

float get_total_buy_aggression_volume(Candle *candles, int pos) {
    return candles[pos].total_buy_aggression_volume;
}

float get_total_sell_aggression_volume(Candle *candles, int pos) {
    return candles[pos].total_sell_aggression_volume;
}

float get_delta_aggression(Candle *candles, int pos) {
    return candles[pos].delta_aggression;
}

float get_delta_average(Candle *candles, int pos) {
    return candles[pos].delta_average;
}

float get_delta_std(Candle *candles, int pos) {
    return candles[pos].delta_std;
}

float get_buy_volume_average(Candle *candles, int pos) {
    return candles[pos].buy_volume_average;
}

float get_sell_volume_average(Candle *candles, int pos) {
    return candles[pos].sell_volume_average;
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

Candle *generate_candles(Times_And_Trades *times_and_trades, int timeframe, float *prev_exponential_moving_average, float **prev_closes, int *prev_close_count, int simple_moving_average_period, int exponential_moving_average_period) {
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
    int daily_low_until_current_candle = (int)1e9;
    int daily_high_until_current_candle = (int)-1e9;

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
                candles[j].daily_high_until_current_candle = (float)daily_high_until_current_candle / 10.0;
                candles[j].daily_low_until_current_candle = (float)daily_low_until_current_candle / 10.0;
                candles[j].delta_average = 0;
                candles[j].delta_std = 0;
                candles[j].displacement = candles[j].close - candles[j].open;
                candles[j].impact_average = 0;
                candles[j].real_movement = candles[j].close - candles[j].open;
                candles[j].buy_aggression_per_displacement = 0;
                candles[j].sell_aggression_per_displacement = 0;
                candles[j].delta_aggression_per_displacement = 0;

                Tape_Reading *tr = malloc(count * sizeof(Tape_Reading));
                size_t tr_count = 0;

                float total_buy_aggression_volume = 0;
                float total_sell_aggression_volume = 0;

                for (int k = 0; k < count; k++) {
                    candles[j].candle_times_and_trades[k].datetime = get_times_and_trades_datetime(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].price = get_times_and_trades_price(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    candles[j].candle_times_and_trades[k].pos = k;

                    float price = get_times_and_trades_price(times_and_trades, i - count + k);
                    float volume = get_times_and_trades_volume(times_and_trades, i - count + k);
                    float buy_volume = 0;
                    float sell_volume = 0;
                    char aggression = get_times_and_trades_aggression(times_and_trades, i - count + k);

                    if (aggression == 'C') {
                        total_buy_aggression_volume += volume;
                        buy_volume = volume;
                    }
                    if (aggression == 'V') {
                        total_sell_aggression_volume += volume;
                        sell_volume = volume;
                    }

                    int found = 0;

                    for (int m = 0; m < tr_count; m++) {
                        if (fabs(tr[m].price - price) < 0.0001 && aggression == 'C') {
                            tr[m].buy_volume += buy_volume;
                            found = 1;
                            break;
                        }
                        if (fabs(tr[m].price - price) < 0.0001 && aggression == 'V') {
                            tr[m].sell_volume += sell_volume;
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        tr[tr_count].price = price;
                        tr[tr_count].buy_volume = buy_volume;
                        tr[tr_count].sell_volume = sell_volume;
                        tr[tr_count].aggression = aggression;
                        tr_count++;
                    }
                }

                candles[j].candle_times_and_trades_rows = count;
                candles[j].total_buy_aggression_volume = total_buy_aggression_volume;
                candles[j].total_sell_aggression_volume = total_sell_aggression_volume;
                candles[j].delta_aggression = total_buy_aggression_volume - total_sell_aggression_volume;
                candles[j].price_volumes = realloc(tr, tr_count * sizeof(Tape_Reading));
                candles[j].price_volumes_rows = tr_count;
                candles[j].impact = candles[j].displacement / candles[j].delta_aggression;

                if (candles[j].displacement != 0) {
                    candles[j].buy_aggression_per_displacement = total_buy_aggression_volume / fabs(candles[j].displacement);
                    candles[j].sell_aggression_per_displacement = total_sell_aggression_volume / fabs(candles[j].displacement);
                    candles[j].delta_aggression_per_displacement = fabs(candles[j].delta_aggression) / fabs(candles[j].displacement);
                }

                if (high > daily_high_until_current_candle) {
                    daily_high_until_current_candle = high;
                }
                if (low < daily_low_until_current_candle) {
                    daily_low_until_current_candle = low;
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
        candles[j].daily_low_until_current_candle = 0;
        candles[j].daily_high_until_current_candle = 0;
        candles[j].delta_average = 0;
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

    for (int k = 1; k < j; k++) {
        for (int x = 0; x < candles[k].price_volumes_rows; x++) {
            float buy_volume_sum = 0;
            float sell_volume_sum = 0;
            candles[k].buy_volume_average = 0;
            candles[k].sell_volume_average = 0;
            float price = candles[k].price_volumes[x].price;
            int count = 0;
            for (int l = k - 1; l >= 0; l--) {
                for (int y = 0; y < candles[l].price_volumes_rows; y++) {
                    buy_volume_sum += candles[l].price_volumes[y].buy_volume;
                    sell_volume_sum += candles[l].price_volumes[y].sell_volume;
                    // if (candles[l].price_volumes[y].aggression == 'L' && candles[l].price_volumes[y].buy_volume == 0 && candles[l].price_volumes[y].sell_volume == 0) {
                    //     continue;
                    // }
                    count++;
                }
            }
            if (count > 0) {
                candles[k].buy_volume_average = buy_volume_sum / count;
                candles[k].sell_volume_average = sell_volume_sum / count;
            }
        }
    }


    for (int k = 1; k < j; k++) {
        float displacement_sum = 0;
        float delta_sum = 0;
        for (int l = k - 1; l >= 0; l--) {
            displacement_sum += candles[l].displacement;
            delta_sum += candles[l].delta_aggression;
        }
        candles[k].impact_average = displacement_sum / delta_sum;
        candles[k].expected_movement = candles[k].delta_aggression * candles[k].impact_average;
    }

    for (int k = 1; k < j; k++) {
        float sum_delta = 0;
        float sum_delta_buy = 0;
        float sum_delta_sell = 0;
        float variance = 0;
        for (int l = k - 1; l >= 0; l--) {
            sum_delta += fabs(candles[l].delta_aggression);
        }
        candles[k].delta_average = sum_delta / k;
        for (int l = k - 1; l >= 0; l--) {
            float diff = candles[l].delta_aggression - candles[k].delta_average;
            variance += diff * diff;
        }
        variance /= k;
        candles[k].delta_std = sqrt(variance);
    }

    for (int k = 0; k < j; k++) {
        float sum_buy_aggression = 0;
        float sum_sell_aggression = 0;
        float sum_delta = 0;
        float sum_displacement = 0;
        for (int l = k; l >= 0; l--) {
            sum_buy_aggression += candles[l].total_buy_aggression_volume;
            sum_sell_aggression += candles[l].total_sell_aggression_volume;
            sum_delta += fabs(candles[l].delta_aggression);
            sum_displacement += fabs(candles[l].displacement);
        }
        candles[k].buy_aggression_per_displacement_average = sum_buy_aggression / sum_displacement;
        candles[k].sell_aggression_per_displacement_average = sum_sell_aggression / sum_displacement;
        candles[k].delta_aggression_per_displacement_average = sum_delta / sum_displacement;
    }

    float alpha = 2.0 / (exponential_moving_average_period + 1);

    for (int k = 0; k < j; k++) {
        float current_close = candles[k].close;

        if (k == 0 && *prev_exponential_moving_average == 0) {
            candles[k].exponential_moving_average = current_close;
        } else {
            candles[k].exponential_moving_average = current_close * alpha + *prev_exponential_moving_average * (1 - alpha);
        }

        *prev_exponential_moving_average = candles[k].exponential_moving_average;

        if (*prev_close_count >= simple_moving_average_period) {
            memmove(*prev_closes, *prev_closes + 1, (simple_moving_average_period - 1) * sizeof(float));
            (*prev_closes)[simple_moving_average_period - 1] = current_close;
        } else {
            (*prev_closes)[*prev_close_count] = current_close;
            (*prev_close_count)++;
        }

        if (*prev_close_count >= simple_moving_average_period) {
            float sum = 0;

            for (int i = 0; i < simple_moving_average_period; i++) {
                sum += (*prev_closes)[i];
            }

            candles[k].simple_moving_average = sum / simple_moving_average_period;
        } else {
            candles[k].simple_moving_average = 0;
        }
    }

    return candles;
}

void __print_header(FILE *filename) {
    fprintf(filename, "datetime\topen\thigh\tlow\tclose\tdaily_high_until_current_candle\tdaily_low_until_current_candle\tsma\tema\ttotal_buy_aggression_volume\ttotal_sell_aggression_volume\tdelta_aggression\tdelta_average\tdelta_std\tbuy_volume_average\tsell_volume_average\tdisplacement\timpact\timpact_average\texpected_movement\treal_movement\tbuy_aggression_per_displacement\tsell_aggression_per_displacement\tdelta_aggression_per_displacement\tbuy_mad2\tsell_mad2\tdelta_mad2\n");
}

void __print_candles(Candle *candles, char *filename) {
    FILE *out = fopen(filename, "w");

    if (!out) {
        perror("Failed to open file\n");
        return;
    }

    __print_header(out);
    for (size_t i = 0; i < candles->size; i++) {
        fprintf(out, "%02d/%02d/%02d %02d:%02d:%02d\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%.4f\t%.4f\t%.4f\n",
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
            candles[i].daily_high_until_current_candle,
            candles[i].daily_low_until_current_candle,
            candles[i].simple_moving_average,
            candles[i].exponential_moving_average,
            candles[i].total_buy_aggression_volume,
            candles[i].total_sell_aggression_volume,
            candles[i].delta_aggression,
            candles[i].delta_average,
            candles[i].delta_std,
            candles[i].buy_volume_average,
            candles[i].sell_volume_average,
            candles[i].displacement,
            candles[i].impact,
            candles[i].impact_average,
            candles[i].expected_movement,
            candles[i].real_movement,
            candles[i].buy_aggression_per_displacement,
            candles[i].sell_aggression_per_displacement,
            candles[i].delta_aggression_per_displacement,
            candles[i].buy_aggression_per_displacement_average,
            candles[i].sell_aggression_per_displacement_average,
            candles[i].delta_aggression_per_displacement_average
        );
        // for (size_t j = 0; j < candles[i].price_volumes_rows; j++) {
        //     printf("\t%.1f\t%.1f\t%.1f\t%c\n",
        //         candles[i].price_volumes[j].price,
        //         candles[i].price_volumes[j].buy_volume,
        //         candles[i].price_volumes[j].sell_volume,
        //         candles[i].price_volumes[j].aggression
        //     );
        // }
    }

    fclose(out);
}

void __free_candles(Candle *candles) {
    for (size_t i = 0; i < candles->size; i++) {
        free(candles[i].candle_times_and_trades);
        free(candles[i].price_volumes);
    }
    free(candles);
}

size_t __get_candles_size(Candle *candles) {
    return candles->size;
}

size_t __get_candle_times_and_trades_size(Candle *candles, int pos) {
    return candles[pos].candle_times_and_trades_rows;
}
