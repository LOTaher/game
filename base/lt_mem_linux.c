#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)

#include "lt.h"
#include <sys/mman.h>
#include <unistd.h>

void* lt_mem_reserve(u64 size)
{
    void* ptr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (ptr == MAP_FAILED) ? NULL : ptr;
}

b32 lt_mem_commit(void* ptr, u64 size)
{
    return mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
}

void lt_mem_decommit(void* ptr, u64 size)
{
    mprotect(ptr, size, PROT_NONE);
    madvise(ptr, size, MADV_DONTNEED);
}

void lt_mem_release(void* ptr, u64 size)
{
    munmap(ptr, size);
}

u64 lt_mem_page_size(void)
{
    return (u64)sysconf(_SC_PAGESIZE);
}

#endif // __linux__, __unix__, __APPLE__

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

