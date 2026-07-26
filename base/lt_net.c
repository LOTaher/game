#include "lt.h"

b32 lt_net_tcp_recv_exact(Socket sock, void* buf, u64 buffer_size, u64 recv_size)
{
    if (recv_size > buffer_size) {
        return FALSE;
    }

    u64 total_read = 0;
    u8* bytes = (u8*)buf;

    while (total_read < recv_size) {
        i64 n = lt_net_tcp_recv(sock, bytes + total_read, recv_size - total_read);
        if (n <= 0) {
            return FALSE;
        }
        total_read += (u64)n;
    }

    return TRUE;
}

b32 lt_net_tcp_send_exact(Socket sock, void* buf, u64 send_size)
{
    u64 total_sent = 0;
    u8* bytes = (u8*)buf;

    while (total_sent < send_size) {
        i64 n = lt_net_tcp_send(sock, bytes + total_sent, send_size - total_sent);
        if (n <= 0) {
            return FALSE;
        }
        total_sent += (u64)n;
    }

    return TRUE;
}
