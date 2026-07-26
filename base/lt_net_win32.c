#if defined(_WIN32)
#include "lt.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

b32 lt_net_init(void)
{
    WSADATA wsa_data;

    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

void lt_net_shutdown(void)
{
    WSACleanup();
}

Socket lt_net_socket_create(Net_Protocol protocol)
{
    int type = (protocol == LT_NET_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    SOCKET s = socket(AF_INET, type, 0);

    return (Socket){(u64)s};
}

b32 lt_net_socket_is_valid(Socket sock)
{
    // NOTE(laith): INVALID_SOCKET is all-bits-set on win32, NOT 0 like a POSIX fd error.
    return (SOCKET)sock.handle != INVALID_SOCKET;
}

void lt_net_socket_close(Socket sock)
{
    closesocket((SOCKET)sock.handle);
}

b32 lt_net_socket_bind(Socket sock, u16 port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    return bind((SOCKET)sock.handle, (struct sockaddr*)&addr, sizeof(addr)) == 0;
}

b32 lt_net_tcp_listen(Socket sock, i32 backlog)
{
    return listen((SOCKET)sock.handle, backlog) == 0;
}

Socket lt_net_tcp_accept(Socket sock, Net_Addr* out_addr)
{
    struct sockaddr_in addr = {0};
    int addr_len = sizeof(addr);
    SOCKET client = accept((SOCKET)sock.handle, (struct sockaddr*)&addr, &addr_len);
    if (out_addr != NULL) {
        out_addr->ip   = ntohl(addr.sin_addr.s_addr);
        out_addr->port = ntohs(addr.sin_port);
    }

    return (Socket){(u64)client};
}

b32 lt_net_tcp_connect(Socket sock, Net_Addr addr)
{
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr.ip);
    sa.sin_port = htons(addr.port);

    return connect((SOCKET)sock.handle, (struct sockaddr*)&sa, sizeof(sa)) == 0;
}

i64 lt_net_tcp_send(Socket sock, const void* data, u64 size)
{
    return (i64)send((SOCKET)sock.handle, (const char*)data, (int)size, 0);
}

i64 lt_net_tcp_recv(Socket sock, void* buf, u64 size)
{
    return (i64)recv((SOCKET)sock.handle, (char*)buf, (int)size, 0);
}

i64 lt_net_udp_sendto(Socket sock, const void* data, u64 size, Net_Addr addr)
{
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr.ip);
    sa.sin_port = htons(addr.port);

    return (i64)sendto((SOCKET)sock.handle, (const char*)data, (int)size, 0, (struct sockaddr*)&sa, sizeof(sa));
}

i64 lt_net_udp_recvfrom(Socket sock, void* buf, u64 size, Net_Addr* out_addr)
{
    struct sockaddr_in sa = {0};
    int addr_len = sizeof(sa);
    i64 n = (i64)recvfrom((SOCKET)sock.handle, (char*)buf, (int)size, 0,
                           (struct sockaddr*)&sa, &addr_len);
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
    wsprintfA(port_cstr, "%u", port);

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

#endif // _WIN32

// NOTE(laith): external declaration to prevent warning C4206 from MSVC (empty translation unit)
typedef int _compile;

