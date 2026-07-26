// Directory implementation for Laith's Game
// All servers heartbeat with the directory.
// Clients ping the directory once, and the directory responds with where to connect.
// Client connects to that game server


// https://learn.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock

#include <stdio.h>
#include <string.h>

#include "../base/lt.h"
#include "../shared/protocol.h"

global b32 DIRECTORY_SHOULD_CLOSE = TRUE;

typedef struct {
    Net_Addr addr;
    char     region[32];
    u64      last_seen;
    b32      in_use;
} Server;

#define SERVER_EXPIRE_MS (15 * 1000) // NOTE(laith): 15 seconds for now?
#define NUM_SERVERS  0
global Server SERVERS[DIRECTORY_MAX_SERVERS];

int main(void)
{
    lt_net_init();

    Socket socket = lt_net_socket_create(LT_NET_TCP);
    b32 binded = lt_net_socket_bind(socket, DIRECTORY_PORT);
    if (binded == FALSE)
    {
        perror("could not bind directory server to socket");
        return 0;
    }

    b32 listening = lt_net_tcp_listen(socket, DIRECTORY_BACKLOG_SIZE);
    if (listening == FALSE)
    {
        perror("could not set socket to listen");
        return 0;
    }

    while (!DIRECTORY_SHOULD_CLOSE)
    {
        Net_Addr clientAddr;
        Socket client = lt_net_tcp_accept(socket, &clientAddr);

        u8 connTypeByte;

        // NOTE(laith): check the type of directory packet it is
        if (!lt_net_tcp_recv_exact(client, &connTypeByte, sizeof(connTypeByte), sizeof(connTypeByte)))
        {
            lt_net_socket_close(client);
            continue;
        }

        switch (connTypeByte)
        {
            case DIRECTORY_LIST:
            {
                DirectoryListResponse response = {0};
                response.type = DIRECTORY_LIST_RESPONSE;
                response.count = 0;

                for (u8 i = 0; i < DIRECTORY_MAX_SERVERS; ++i)
                {
                    u64 now = lt_os_time_now_ms();
                    Server server = SERVERS[i];

                    if (!server.in_use) continue;

                    if ((now - server.last_seen) > SERVER_EXPIRE_MS) continue;

                    DirectoryListEntry* entry = &response.entries[response.count];
                    entry->ip = server.addr.ip;
                    entry->port = server.addr.port;
                    memcpy(entry->region, server.region, sizeof(server.region));

                    response.count++;
                }

                // NOTE(laith): can just send the address until the size of it because the bits are packed and not spaced out
                if (!lt_net_tcp_send_exact(client, &response, sizeof(response)))
                {
                    perror("could not send all of packet, connection is in error state. closing socket");
                    lt_net_socket_close(client);
                    continue;
                }

                lt_net_socket_close(client);

            } break;

            case DIRECTORY_REGISTER:
            {
                DirectoryRegisterRequest RegisterRequest = {0};
                RegisterRequest.type = connTypeByte;

                u64 remainingRegisterPacketSize = sizeof(DirectoryRegisterRequest) - sizeof(RegisterRequest.type);
                u8* pointerToNextStructField = (u8*)&RegisterRequest + sizeof(u8);

                if (!lt_net_tcp_recv_exact(client, pointerToNextStructField, remainingRegisterPacketSize, remainingRegisterPacketSize))
                {
                    lt_net_socket_close(client);
                    perror("invalid register request packet");
                    continue;
                }

                Server* target = NULL;

                // NOTE(laith): check to see if there is an existing server with the same IP and port
                for (u8 i = 0; i < DIRECTORY_MAX_SERVERS; ++i)
                {
                    // NOTE(laith): we are checking for expired servers by replacing them even if their in_use is true but they're not last seen
                    u64 now = lt_os_time_now_ms();
                    if ((SERVERS[i].in_use && SERVERS[i].addr.ip == clientAddr.ip && SERVERS[i].addr.port == clientAddr.port) || (now - SERVERS[i].last_seen > SERVER_EXPIRE_MS))
                    {
                        target = &SERVERS[i];
                        break;
                    }
                }

                // NOTE(laith): if no existing match, take the first free slot left
                if (target == NULL)
                {
                    for (u8 i = 0; i < DIRECTORY_MAX_SERVERS; ++i)
                    {
                        if (!SERVERS[i].in_use)
                        {
                            target = &SERVERS[i];
                            break;
                        }
                    }
                }

                if (target != NULL)
                {
                    target->addr.ip = clientAddr.ip;
                    // NOTE(laith): the port of the client request was given by the operating system, the real port would be the specified UDP port the server would specify in this packet
                    target->addr.port = RegisterRequest.port;
                    memcpy(target->region, RegisterRequest.region, sizeof(target->region));
                    target->in_use = TRUE;
                    target->last_seen = lt_os_time_now_ms();
                }
                else {
                    // NOTE(laith): list full since the target didn't take a new slot. just log and drop.
                    lt_net_socket_close(client);
                    fprintf(stderr, "Server list full, rejecting IP: %d\n", clientAddr.ip);
                    continue;
                }

                u8 response = PACKET_ACK;
                lt_net_tcp_send(client, &response, sizeof(response));
                lt_net_socket_close(client);

            } break;

            default:
            lt_net_socket_close(client);
            continue;
        }
    }

    perror("directory no longer running. ending program.");
    return 0;
}
