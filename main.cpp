#include "Server.hpp"

/*
    Plan:
    1. HashMap + GET/SET/DEL                DONE
    2. TTL + expiry                         DONE
    3. Serialization and Deserialization    DONE
    4. LRU eviction                         DONE
    5. ttl for clients                      DONE
    6. ADD LIST type                        DONE
    7. Write-Ahead Log
    8. Snapshotting→ stops the WAL from growing forever
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
            type == LIST
                [4 bytes]  number of values
                [4 bytes]  value length
                [N bytes]  value
            type == STR
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