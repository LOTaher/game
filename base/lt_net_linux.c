#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)

#include "lt.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

b32 lt_net_init(void)
{
    // NOTE(laith): no global init step needed on POSIX, unlike WSAStartup on win32.
    return TRUE;
}

void lt_net_shutdown(void)
{
    // NOTE(laith): no global shutdown step needed on POSIX, unlike WSACleanup on win32.
}

Socket lt_net_socket_create(Net_Protocol protocol)
{
    int type = (protocol == LT_NET_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int fd = socket(AF_INET, type, 0);

    return (Socket){(u64)fd};
}

b32 lt_net_socket_is_valid(Socket sock)
{
    // NOTE(laith): a failed socket() returns -1 on POSIX, NOT all-bits-set like win32.
    return (i64)sock.handle >= 0;
}

void lt_net_socket_close(Socket sock)
{
    close((int)sock.handle);
}

b32 lt_net_socket_bind(Socket sock, u16 port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    return bind((int)sock.handle, (struct sockaddr*)&addr, sizeof(addr)) == 0;
}

b32 lt_net_tcp_listen(Socket sock, i32 backlog)
{
    return listen((int)sock.handle, backlog) == 0;
}

Socket lt_net_tcp_accept(Socket sock, Net_Addr* out_addr)
{
    struct sockaddr_in addr = {0};
    socklen_t addr_len = sizeof(addr);
    int client_fd = accept((int)sock.handle, (struct sockaddr*)&addr, &addr_len);
    if (out_addr != NULL) {
        out_addr->ip   = ntohl(addr.sin_addr.s_addr);
        out_addr->port = ntohs(addr.sin_port);
    }

    return (Socket){(u64)client_fd};
}

b32 lt_net_tcp_connect(Socket sock, Net_Addr addr)
{
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr.ip);
    sa.sin_port = htons(addr.port);

    return connect((int)sock.handle, (struct sockaddr*)&sa, sizeof(sa)) == 0;
}

i64 lt_net_tcp_send(Socket sock, const void* data, u64 size)
{
    return (i64)send((int)sock.handle, data, size, 0);
}

i64 lt_net_tcp_recv(Socket sock, void* buf, u64 size)
{
    return (i64)recv((int)sock.handle, buf, size, 0);
}

i64 lt_net_udp_sendto(Socket sock, const void* data, u64 size, Net_Addr addr)
{
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr.ip);
    sa.sin_port = htons(addr.port);

    return (i64)sendto((int)sock.handle, data, size, 0, (struct sockaddr*)&sa, sizeof(sa));
}

i64 lt_net_udp_recvfrom(Socket sock, void* buf, u64 size, Net_Addr* out_addr)
{
    struct sockaddr_in sa = {0};
    socklen_t addr_len = sizeof(sa);
    i64 n = (i64)recvfrom((int)sock.handle, buf, size, 0, (struct sockaddr*)&sa, &addr_len);
    if (out_addr != NULL) {
        out_addr->ip   = ntohl(sa.sin_addr.s_addr);
        out_addr->port = ntohs(sa.sin_port);
    }

    return n;
}

b32 lt_net_resolve(String8 host, u16 port, Net_Addr* out_addr)
{
    // NOTE(laith): getaddrinfo needs a null-terminated C string; String8 isn't
    // guaranteed null-terminated, so copy it into a fixed stack buffer first.
    char host_cstr[256];
    u64 len = MIN(host.length, sizeof(host_cstr) - 1);
    for (u64 i = 0; i < len; i += 1) {
        host_cstr[i] = (char)host.str[i];
    }
    host_cstr[len] = '\0';

    char port_cstr[8];
    snprintf(port_cstr, sizeof(port_cstr), "%u", port);

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result = NULL;
    if (getaddrinfo(host_cstr, port_cstr, &hints, &result) != 0) {
        return FALSE;
    }

    struct sockaddr_in* addr_in = (struct sockaddr_in*)result->ai_addr;
    out_addr->ip   = ntohl(addr_in->sin_addr.s_addr);
    out_addr->port = port;

    freeaddrinfo(result);

    return TRUE;
}

#endif // __linux__, __unix__, __APPLE__

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

