#include "Server.hpp"

/*
    Plan:
    1. HashMap + GET/SET/DEL          DONE
    2. TTL + expiry                   DONE
    3. Serialization and Deserialization DONE
    4. LRU eviction                   → cap memory usage IN PROGRESS
        1. limit memory by size example 64mb
        2. when the limit is reached call LRU
        3. keep track of keys usage
    5. Write-Ahead Log                → survive crashes
    6. Snapshotting                   → keep the WAL from growing forever
    7. ADD LIST type
    8. add checksum to file format

    costum.db file format
    [4 bytes]  magic number          0FTREDIS
    [4 bytes]  number of keys
    [per key]
        [1 byte]   has_ttl flag      0 or 1
        [8 bytes]  expires_at        only if has_ttl == 1
        [1 byte]   type
        [4 bytes]  key length
        [N bytes]  key
        [4 bytes]  value length
        [N bytes]  value
    [8 bytes]  CRC64 checksum <- TODO
*/

std::atomic<bool> sigint = 0;

int main()
{
    Server server;

    signal(SIGPIPE, SIG_IGN);
    server.run();
    return 0;
}