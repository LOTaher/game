// Server Implementation for Laith's Game
// Server on start pings the directory and gets added to the directory's available entries.

#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        perror("Usage: ./server.exe [directory ip] [server port] [server name]\n\nExample: ./server.exe 192.168.1.2 4001 us-east");
        return 0;
    }


    return 0;
}
