#ifndef LT_H
#define LT_H

/* ------ LT ------
  Utility library used by Laith Taher for all C Programming.It is meant to be imported in all C files I use and work on all systems */

#include <stdint.h>

/* --- Base Layer --- */

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i8       b8;
typedef i32      b32;
typedef float    f32;
typedef double   f64;

#define MIN(a, b) \
    ((a) < (b) ? (a) : (b))
#define MAX(a, b) \
    ((a) > (b) ? (a) : (b))

#define KiB(n) \
    ((u64)(n) << 10)
#define MiB(n) \
    ((u64)(n) << 20)
#define GiB(n) \
    ((u64)(n) << 30)

#define ARR_LENGTH(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

#define local static
#define global static
#define internal static

#define TRUE 1
#define FALSE 0

/* --- Memory Library --- */

extern void* lt_mem_reserve(u64 size);               // Reserve address space without physical commits.
extern b32   lt_mem_commit(void* ptr, u64 size);     // Commit pages and map to physical memory.
extern void  lt_mem_decommit(void* ptr, u64 size);   // Decommit pages.
extern void  lt_mem_release(void* ptr, u64 size);    // Release the reservation.
extern u64   lt_mem_page_size(void);                 // Check the page size of the system.

/* --- Arena Library --- */

typedef struct {
    u64 capacity;   // Reserved address space
    u64 committed;  // Backed by physical memory
    u64 pos;        // Current allocation offset
} Arena;

typedef struct {
    Arena* arena;
    u64 pos;
} Arena_Temp;

#define LT_ARENA_COMMIT_CHUNK MiB(1) // How much memory to commit at a time

extern Arena*     lt_arena_create(u64 capacity);                    // Create an Arena Allocator.
extern void       lt_arena_destroy(Arena* arena);                   // Destroy (free) an Arena.
extern u64        lt_arena_align_forward(u64 pos, u64 alignment);   // Align memory to the next power of 2.
extern void*      lt_arena_push(Arena* arena, u64 size);            // Push new memory onto the Arena.
extern void       lt_arena_clear(Arena* arena);                     // Clear the memory in an Arena.
extern u64        lt_arena_mark(Arena* arena);                      // Mark the current position in an Arena.
extern void       lt_arena_pop(Arena* arena, u64 mark);             // Pop to the memory mark of an Arena.

extern Arena_Temp lt_arena_temp_begin(Arena* arena);                // Create a Temp_Arena.
extern void       lt_arena_temp_end(Arena_Temp arena);              // Destory a Temp_Arena.

/* --- String Library --- */

typedef struct {
  u8* str;
  u64 length;
} String8;

#define lt_string_lit(s) \
    (String8){(u8 *)(s), sizeof(s) - 1}
#define lt_string_fmt(s) \
    (int)(s).length, (s).str

extern String8 lt_string_substring(String8 str, u64 start, u64 end);       // Get the substring of a given string.
extern String8 lt_string_cstring(char *str);                               // Convert a C string to a String8.
extern String8 lt_string_copy(String8 str, Arena *arena);                  // Copy a new string in memory.
extern String8 lt_string_concat(String8 str1, String8 str2, Arena *arena); // Concat two strings together.
extern b8      lt_string_compare(String8 str1, String8 str2);              // Check whether two strings are equal.
extern b8      lt_string_contains(String8 str, String8 substr);            // Check whether a string contains another.

/* --- Net Library --- */

typedef struct {
    u64 handle; // NOTE(laith): SOCKET on win32, int fd on linux. it'll be casted inside lt_net_*.c
} Socket;

typedef struct {
    u32 ip;
    u16 port;
} Net_Addr;

typedef enum {
    LT_NET_UDP,
    LT_NET_TCP,
} Net_Protocol;

#define LT_NET_INVALID_SOCKET ((Socket){0})

extern b32      lt_net_init(void);                                                              // Platform net init (WSAStartup on win32, nothing on linux)
extern void     lt_net_shutdown(void);                                                          // Platform net shutdown (WSACleanup on win32, nothing on linux)
extern Socket   lt_net_socket_create(Net_Protocol protocol);                                    // Create a socket of the given protocol
extern b32      lt_net_socket_is_valid(Socket sock);                                            // Check whether a socket handle is valid
extern void     lt_net_socket_close(Socket sock);                                               // Close socket
extern b32      lt_net_socket_bind(Socket sock, u16 port);                                      // Bind a socket to a local port.
extern b32      lt_net_resolve(String8 host, u16 port, Net_Addr* out_addr);                     // Resolve a hostname/IP string to a Net_Addr.
extern b32      lt_net_tcp_listen(Socket sock, i32 backlog);                                    // (TCP) Mark socket as listening.
extern Socket   lt_net_tcp_accept(Socket sock, Net_Addr* out_addr);                             // (TCP) Accept an incoming connection.
extern b32      lt_net_tcp_connect(Socket sock, Net_Addr addr);                                 // (TCP) Connect to a remote address.
extern i64      lt_net_tcp_send(Socket sock, const void* data, u64 size);                       // (TCP) Send on a connected socket.
extern b32      lt_net_tcp_send_exact(Socket sock, void* buf, u64 send_size);                   // (TCP) Send a specific number of bytes to a connected socket
extern i64      lt_net_tcp_recv(Socket sock, void* buf, u64 size);                              // (TCP) Receive on a connected socket.
extern b32      lt_net_tcp_recv_exact(Socket sock, void* buf, u64 buffer_size, u64 recv_size);  // (TCP) Recieves a specific number of bytes from a connected socket.
extern i64      lt_net_udp_sendto(Socket sock, const void* data, u64 size, Net_Addr addr);      // (UDP) Send to an address.
extern i64      lt_net_udp_recvfrom(Socket sock, void* buf, u64 size, Net_Addr* out_addr);      // (UDP) Receive, capturing sender address.

/* --- Time Library --- */

extern u64   lt_time_now_ms(void);               // Monotonic ms for measuring elapsed time.
extern void  lt_time_sleep_ms(u64 ms);           // Pause calling for a number of milliseconds

/* --- Thread Library --- */

typedef struct {
    u64 handle; // NOTE(laith): HANDLE (cast from void*) on wind32, pthread_t on linux
} Thread;

typedef void (*Thread_Func)(void* arg);

extern Thread lt_thread_create(Thread_Func func, void* arg);   // Create and start a new thread that is running func(arg)
extern void   lt_thread_join(Thread thread);                   // Block until the thread finishes

#endif // LT_H
