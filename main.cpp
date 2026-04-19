#include "Server.hpp"

/*
    Plan:
    1. HashMap + GET/SET/DEL          DONE
    2. TTL + expiry                   DONE
    3. Serialization and Deserialization DONE
    4. LRU eviction                   → cap memory usage IN PROGRESS
        1. limit memory by size example 16mb
        2. when the limit is reached call LRU
        3. keep track of keys usage
    5. ttl for clients
    6. ADD LIST type  <-- here
        LPUSH
        RPUSH
        LPOP
        RPOP
        LRANGE
        QUIT
        TYPE
    7. Write-Ahead Log                → survive crashes
    8. Snapshotting                   → keep the WAL from growing forever
    9. add checksum to file format

    costum.db file format
    [8 bytes]  magic number          0FTREDIS
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

std::atomic<bool> sigint;

static bool set_as_limit_bytes(rlim_t bytes)
{
    rlimit rl;

    rl.rlim_cur = bytes;
    rl.rlim_max = bytes;
    if (setrlimit(RLIMIT_AS, &rl) != 0) {
        return false;
    }
    return true;
}

int main()
{
    Server server;

    signal(SIGPIPE, SIG_IGN);
    set_as_limit_bytes(MEMORY_LIMIT_MB * 1024 * 1024);
    server.run();
    return 0;
}