#include "Server.hpp"

/*
    Plan:
    1. HashMap + GET/SET/DEL          → works in memory, no persistence yet
    2. TTL + expiry                   → background cleanup thread
    3. Serialization and Deserialization of the hashmap
    4. LRU eviction                   → cap memory usage
    5. Write-Ahead Log                → survive crashes
    6. Snapshotting                   → keep the WAL from growing forever

    costum.db file format
    [4 bytes]  magic number          0FTREDIS
    [4 bytes]  number of keys
    [per key]
        [1 byte]   has_ttl flag      0 or 1
        [8 bytes]  expires_at        only if has_ttl == 1
        [4 bytes]  key length
        [N bytes]  key
        [4 bytes]  value length
        [N bytes]  value
    [8 bytes]  CRC64 checksum
*/

std::atomic<bool> sigint = 0;

int main()
{
    Server server;

    signal(SIGPIPE, SIG_IGN);
    server.run();
    return 0;
}