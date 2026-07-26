#if defined(_WIN32)

#include "lt.h"
#include <windows.h>

u64 lt_time_now_ms(void)
{
    return (u64)GetTickCount64();
}

void lt_time_sleep_ms(u64 ms)
{
    Sleep((DWORD)ms);
}

#endif // _WIN32

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

