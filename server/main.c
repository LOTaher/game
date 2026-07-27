// Server Implementation for Laith's Game
// Server on start pings the directory and gets added to the directory's available entries.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../base/lt.h"
#include "../shared/protocol.h"

typedef struct {
    Net_Addr directory_addr;
    u16      server_port;
    char     server_region[32];
} DirectoryHeartbeatArgs;

internal b32 send_register(DirectoryHeartbeatArgs* args)
{
    Socket directorySocket = lt_net_socket_create(LT_NET_TCP);

    if (!lt_net_tcp_connect(directorySocket, args->directory_addr))
    {
        perror("could not connect to directory");
        lt_net_socket_close(directorySocket);
        return FALSE;
    }

    DirectoryRegisterRequest req = {
        .type = DIRECTORY_REGISTER,
        .port = args->server_port,
    };
    memcpy(req.region, args->server_region, sizeof(args->server_region));

    if (!lt_net_tcp_send_exact(directorySocket, &req, sizeof(req)))
    {
        perror("could not send registry request to directory");
        lt_net_socket_close(directorySocket);
        return FALSE;
    }

    u8 ack;
    if (!lt_net_tcp_recv_exact(directorySocket, &ack, sizeof(ack), sizeof(ack)))
    {
        perror("could not recieve ack from directory");
        lt_net_socket_close(directorySocket);
        return FALSE;
    }

    lt_net_socket_close(directorySocket);

    return ack == PACKET_ACK;
}

internal void heartbeat_loop(DirectoryHeartbeatArgs* args)
{
    // TODO(laith): WHILE TRUE LOOP THIS!
    if (!send_register(args))
    {
        printf("Heartbeat failed. Sleeping then will retry.\n");
    }

    lt_time_sleep_ms(5 * 1000); // NOTE(laith): 5 seconds for now?
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        perror("Usage: ./server.exe [directory ip] [server port] [server region]\n\nExample: ./server.exe 192.168.1.2 4001 us-east");
        return 0;
    }

    String8 directoryIpStr = lt_string_cstring(argv[1]);
    DirectoryHeartbeatArgs discoveryHeartbeatArgs = {0};

    lt_net_init();

    if (!lt_net_resolve(directoryIpStr, DIRECTORY_PORT, &discoveryHeartbeatArgs.directory_addr))
    {
        fprintf(stderr, "Could not resolve directory address: %s\n", argv[1]);
        return 1;
    }

    discoveryHeartbeatArgs.server_port = (u16)atoi(argv[2]);

    // NOTE(laith): leaving some room for the null terminator
    u64 len = MIN(strlen(argv[3]), sizeof(discoveryHeartbeatArgs.server_region) - 1);
    memcpy(discoveryHeartbeatArgs.server_region, argv[3], len);

    if (!send_register(&discoveryHeartbeatArgs))
    {
        perror("unable to send initial register to directory");
        return 1;
    }



    Socket gameSocket = lt_net_socket_create(LT_NET_UDP);

    if (!lt_net_socket_bind(gameSocket, discoveryHeartbeatArgs.server_port))
    {
        perror("could not bind udp socket");
        return 1;
    }

    return 0;
}
