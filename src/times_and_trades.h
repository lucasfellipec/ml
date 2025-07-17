#ifndef TIMES_AND_TRADES_H_
#define TIMES_AND_TRADES_H_

#include "datetime.h"

typedef struct Times_And_Trades Times_And_Trades;

Times_And_Trades* read_times_and_trades(const char* filename);

float get_price(Times_And_Trades *times_and_trades, int pos);
Datetime get_datetime(Times_And_Trades* times_and_trades, int pos);

size_t __get_times_and_trades_size(Times_And_Trades* times_and_trades);
void __print_times_and_trades(Times_And_Trades* times_and_trades);

#endif // TIMES_AND_TRADES_H_
