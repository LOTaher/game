// Laith's Game Protocol for Client to Server communication

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../base/lt.h"

// NOTE(laith): no specific reason, just seems like a consistent size
#define MAX_PAYLOAD_SIZE    1024
#define PROTCOL_MAGIC       0x47414D45u

typedef enum
{
    PACKET_PING = 1,
    PACKET_PONG = 2,
    PACKET_JOIN = 3,
    PACKET_ACK = 4,
    PACKET_DISCONNECT = 5,
    // TODO(laith): add game related packets like PACKET_STATE (for world state) and PACKET_INPUT (player input)
} PacketType;

// NOTE(laith): struct is sent/received as raw bytes over UDP (client <-> server
// must agree on exact byte layout). #pragma pack(1) removes compiler-inserted
// alignment padding so sizeof(GamePacket) always equals the sum of its fields.
// Currently safe without this on our matched Windows/MSVC/x64 builds, but this
// guarantees it explicitly instead of relying on both sides staying in sync by luck.'
// #pragma pack(push, 1)
// typedef struct
// {
//     u32                  magic;
//     PacketType  type;
//     u32                  sequence;
//     u16                  payload_len;
//     u8                   paylod[MAX_PAYLOAD_SIZE];
// } Protocol_GamePacket;
// #pragma pack(pop)

#define DIRECTORY_PORT                      9300
#define DIRECTORY_HEARTBEAT_INTERVAL        5
#define DIRECTORY_SERVER_EXPIRE_INTERVAL    15
#define DIRECTORY_BACKLOG_SIZE              10
#define DIRECTORY_MAX_SERVERS               64

typedef enum
{
    DIRECTORY_LIST = 1,
    DIRECTORY_REGISTER = 2,
    DIRECTORY_LIST_RESPONSE = 3
} DirectoryPacketType;

// NOTE(laith): going to be taking the performance hit by packing bytes, this is ok since the directory is not required to be very compute heavy
#pragma pack(push, 1)
typedef struct {
    u8                       type;
    u16                      port;
    char                     region[32];
} DirectoryRegisterRequest;

typedef struct {
    DirectoryPacketType type;
} DirectoryListRequest;

typedef struct {
    u32  ip;
    u16  port;
    char region[32];
} DirectoryListEntry;

typedef struct {
    u8                      type;
    u8                      count;
    DirectoryListEntry      entries[DIRECTORY_MAX_SERVERS];
} DirectoryListResponse;
#pragma pack(pop)

#endif // PROTOCOL_H
