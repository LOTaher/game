#if defined(_WIN32)

#include "lt.h"
#include <windows.h>

void* lt_os_reserve(u64 size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}

b32 lt_os_commit(void* ptr, u64 size)
{
    return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != NULL;
}

void lt_os_decommit(void* ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

void lt_os_release(void* ptr, u64 size)
{
    (void)size; // NOTE(laith): unused on Windows but would like to keep signatures the same
    VirtualFree(ptr, 0, MEM_RELEASE);
}

u64 lt_os_page_size(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

u64 lt_os_time_now_ms(void)
{
    return (u64)GetTickCount64();
}

#endif // _WIN32

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

