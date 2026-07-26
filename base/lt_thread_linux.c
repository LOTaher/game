#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include "lt.h"
#include <pthread.h>
#include <stdlib.h>

// NOTE(laith): pthreads requires a start routine shaped void*(*)(void*),
// but Thread_Func is void(*)(void*) with no return value. Same trampoline
// pattern as the win32 side bridges the two signatures.
typedef struct {
    Thread_Func func;
    void* arg;
} ThreadStartData;

static void* thread_trampoline(void* param)
{
    ThreadStartData* data = (ThreadStartData*)param;
    Thread_Func func = data->func;
    void* arg = data->arg;
    free(data);

    func(arg);

    return NULL;
}

Thread lt_thread_create(Thread_Func func, void* arg)
{
    ThreadStartData* data = (ThreadStartData*)malloc(sizeof(ThreadStartData));
    if (data == NULL) {
        return (Thread){0};
    }
    data->func = func;
    data->arg = arg;

    pthread_t tid;
    if (pthread_create(&tid, NULL, thread_trampoline, data) != 0) {
        free(data);
        return (Thread){0};
    }

    // NOTE(laith): pthread_t is an unsigned long on Linux/glibc, so this cast
    // is safe here. Not guaranteed portable to every POSIX platform (some
    // implementations use an opaque struct for pthread_t), but fine for our
    // Linux-only target.
    return (Thread){(u64)tid};
}

void lt_thread_join(Thread thread)
{
    pthread_t tid = (pthread_t)thread.handle;
    pthread_join(tid, NULL);
}

#endif // __linux__, __unix__, __APPLE__

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;
