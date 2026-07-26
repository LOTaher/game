#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)

#include "lt.h"
#include <time.h>

u64 lt_time_now_ms(void)
{
    struct timespec ts;
    clock_getttime(CLOCK_MONOTONIC, &ts);

    return (u64)ts.tv_sec * 1000 + (u64)ts.tv_nsec / 1000000
}

void lt_time_sleep_ms(u64 ms)
{
    struct timespec ts;
    ts.tv_spec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

#endif // __linux__, __unix__, __APPLE__

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

