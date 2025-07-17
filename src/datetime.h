#ifndef DATETIME_H_
#define DATETIME_H_

#include <time.h>

typedef struct {
    int month;
    int day;
    int year;
    int hour;
    int minute;
    int second;
    int millisecond;
} Datetime;

typedef struct {
    int hour;
    int minute;
    int seecond;
    int millisecond;
} Time;

time_t timestamp_to_unix(Datetime dt);
Datetime time_to_timestamp(time_t t);

void __print_datetime(Datetime dt);

#endif // DATETIME_H_
