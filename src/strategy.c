#include <stdio.h>

#include "strategy.h"
#include "candles.h"
#include "datetime.h"

void strategy(Candles *candles) {
    for (size_t i = 1; i < __get_candles_size(candles); i++) {
        int cond = 1;

        float entry_price;

        if (cond) {
            entry_price = get_low(candles, i - 1);

            printf("entry_price: %f\n", entry_price);

            for (size_t j = 0; j < __get_candle_times_and_trades_size(candles, i); j++) {
                if (get_price_from_times_and_trades(candles, i, j) < entry_price) {
                    __print_datetime(get_candle_times_and_trades_datetime(candles, i, j));
                    break;
                }
            }
        }
    }
}
