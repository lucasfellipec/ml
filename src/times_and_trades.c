#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "times_and_trades.h"
#include "datetime.h"

struct Times_And_Trades {
    Datetime datetime;
    float price;
    double real_volume;
    float volume;
    char agg;
    size_t size;
};

Times_And_Trades *read_times_and_trades(const char *filename) {
    FILE *file = fopen(filename, "r");

    int count = 0;

    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char line_buffer[1024];

    if (!fgets(line_buffer, sizeof(line_buffer), file)) {
        fprintf(stderr, "Error reading file\n");
        fclose(file);
        return NULL;
    }

    const int initial_capacity = 1024;
    Times_And_Trades *times_and_trades = malloc(initial_capacity * sizeof(Times_And_Trades));
    int rows = 0;

    if (!times_and_trades) {
        fprintf(stderr, "Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    int capacity = initial_capacity;

    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        char *cols[5];
        int col = 0;
        char *start = line_buffer;
        cols[0] = start;

        for (char *p = line_buffer; *p && col < 7; p++) {
            if (*p == '\t') {
                *p = '\0';
                col++;
                if (col < 7) {
                    cols[col] = p + 1;
                }
            } else if (*p == '\n' || *p == '\r') {
                *p = '\0';
            }
        }

        Datetime dt;
        if (sscanf(cols[0], "%d.%d.%d", &dt.year, &dt.month, &dt.day) != 3) {
            continue;
        }
        if (sscanf(cols[1], "%d:%d:%d.%d", &dt.hour, &dt.minute, &dt.second, &dt.millisecond) != 4) {
            continue;
        }

        float last = atof(cols[2]);
        float volume = atof(cols[3]);
        char *flag = cols[4];

        if (rows == capacity) {
            capacity *= 2;
            Times_And_Trades *temp = realloc(times_and_trades, capacity * sizeof(Times_And_Trades));
            if (!temp) {
                fprintf(stderr, "Error allocating memory\n");
                free(times_and_trades);
                fclose(file);
                return NULL;
            }
            times_and_trades = temp;
        }

        times_and_trades[rows].datetime = dt;
        times_and_trades[rows].price = last;
        times_and_trades[rows].volume = volume;
        times_and_trades[rows].real_volume = (double)volume * last;
        times_and_trades[rows].agg = flag[0];

        ++rows;
    }

    fclose(file);

    times_and_trades->size = rows;

    return times_and_trades;
}

float get_times_and_trades_price(Times_And_Trades *times_and_trades, int pos) {
    return times_and_trades[pos].price;
}

float get_times_and_trades_volume(Times_And_Trades *times_and_trades, int pos) {
    return times_and_trades[pos].volume;
}

float get_times_and_trades_agg(Times_And_Trades *times_and_trades, int pos) {
    return times_and_trades[pos].agg;
}

Datetime get_times_and_trades_datetime(Times_And_Trades *times_and_trades, int pos) {
    return times_and_trades[pos].datetime;
}

size_t __get_times_and_trades_size(Times_And_Trades *times_and_trades) {
    return times_and_trades->size;
}

void __print_times_and_trades(Times_And_Trades *times_and_trades) {
    printf("Datetime\tPrice\tVolume\tRealVolume\tAgg\n");
    for (size_t i = 0; i < times_and_trades->size; i++) {
        printf("%02d/%02d/%02d %02d:%02d:%02d.%d\t%f\t%f\t%lf\t%c\n",
               times_and_trades[i].datetime.year,
               times_and_trades[i].datetime.month,
               times_and_trades[i].datetime.day,
               times_and_trades[i].datetime.hour,
               times_and_trades[i].datetime.minute,
               times_and_trades[i].datetime.second,
               times_and_trades[i].datetime.millisecond,
               times_and_trades[i].price,
               times_and_trades[i].volume,
               times_and_trades[i].real_volume,
               times_and_trades[i].agg
        );

    }
}
