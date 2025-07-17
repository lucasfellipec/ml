#include <stdio.h>

#include "datetime.h"

void __print_datetime(Datetime dt) {
    printf("%02d/%02d/%02d %02d:%02d:%02d.%d\n", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.millisecond);
}

time_t timestamp_to_unix(Datetime dt) {
    struct tm tm = {
        .tm_sec = dt.second,
        .tm_min = dt.minute,
        .tm_hour = dt.hour,
        .tm_mday = dt.day,
        .tm_mon = dt.month - 1,
        .tm_year = dt.year - 1900,
    };

    return mktime(&tm);
}

Datetime time_to_timestamp(time_t t) {
    struct tm *tm_info = localtime(&t);

    Datetime timestamp = {
        .month = tm_info->tm_mon + 1,
        .day = tm_info->tm_mday,
        .year = tm_info->tm_year + 1900,
        .hour = tm_info->tm_hour,
        .minute = tm_info->tm_min,
        .second = tm_info->tm_sec
    };

    return timestamp;
}
