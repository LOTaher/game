#if defined(_WIN32)
#include "lt.h"
#include <windows.h>
#include <stdlib.h>

// NOTE(laith): Windows requires a thread proc shaped DWORD WINAPI(*)(LPVOID),
// but Thread_Func is a simple void(*)(void*). This small heap-allocated struct
// plus trampoline function bridges the two signatures.
typedef struct {
    Thread_Func func;
    void* arg;
} ThreadStartData;

static DWORD WINAPI thread_trampoline(LPVOID param)
{
    ThreadStartData* data = (ThreadStartData*)param;
    Thread_Func func = data->func;
    void* arg = data->arg;
    free(data);

    func(arg);

    return 0;
}

Thread lt_thread_create(Thread_Func func, void* arg)
{
    ThreadStartData* data = (ThreadStartData*)malloc(sizeof(ThreadStartData));
    if (data == NULL) {
        return (Thread){0};
    }
    data->func = func;
    data->arg = arg;

    HANDLE h = CreateThread(NULL, 0, thread_trampoline, data, 0, NULL);
    if (h == NULL) {
        free(data);
        return (Thread){0};
    }

    return (Thread){(u64)h};
}

void lt_thread_join(Thread thread)
{
    HANDLE h = (HANDLE)thread.handle;
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
}

#endif // _WIN32

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;
