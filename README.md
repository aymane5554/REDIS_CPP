# KV Cache Server

A Redis-inspired, in-memory key–value server written in **C++ (Linux)**.

This project is intentionally systems-focused: **non-blocking I/O (epoll)**, **custom protocol parsing**, **durability (WAL + snapshots)**, **memory-pressure behavior (LRU eviction + RLIMIT_AS)**, and **multi-client lifecycle management**.

---

## Highlights

- Event-driven, **non-blocking TCP server** using **epoll**
- **RESP-like** request parsing + command dispatch
- Data types:
  - Strings
  - Lists
  - Hashmaps
- **TTL / expiration** with periodic cleanup
- **LRU-style eviction** when memory allocation fails
- Durability:
  - **Write-Ahead Log (WAL)**
  - Periodic **snapshots**
- Crash recovery: **load snapshot → replay WAL**
- Idle client cleanup via configurable timeout

---

## Build & Run

### Requirements
- Linux
- `g++`
- `make`
- `python3` (optional, for the testing client)

### Build
```bash
make
```

### Run
```bash
./redihh
```

### Run with configuration
```bash
./redihh --port 9090 --memory-mb 64 --ttl-interval 15 --snapshot-interval 20 --client-timeout 30 --db-file ./my.db --wal-file ./my.wal
```

### CLI Options
- `--port <1-65535>`: listening TCP port
- `--memory-mb <positive-int>`: process address-space cap in MB (`RLIMIT_AS`)
- `--ttl-interval <seconds>`: expired-key cleanup interval
- `--snapshot-interval <seconds>`: full snapshot interval
- `--client-timeout <seconds>`: idle client disconnect timeout
- `--db-file <path>`: snapshot database file path
- `--wal-file <path>`: write-ahead log file path
- `--help`: print usage

### Defaults
- Port: `8080`
- Address-space limit: `16 MB` (`RLIMIT_AS`)
- TTL sweep period: `60s`
- Snapshot period: `30s`
- Client idle timeout: `10s`
- Snapshot file path: `costum.db`
- WAL file path: `wal.log`

---

## High-Level Architecture

1. **`main.cpp`**
   - Ignores `SIGPIPE`
   - Applies memory cap via `setrlimit(RLIMIT_AS, ...)`
   - Starts the server loop

2. **`Server.cpp`**
   - Creates listening socket + sets non-blocking mode
   - Registers sockets in epoll
   - Accepts clients and tracks per-client state
   - Runs the main event loop (`EPOLLIN`, `EPOLLOUT`, `EPOLLHUP`, `EPOLLERR`)
   - Spawns a background TTL cleanup thread
   - Triggers periodic snapshotting in a forked child process

3. **`Request.cpp`**
   - Parses RESP-like frames from each client receive buffer
   - Supports partial reads by accumulating buffers

4. **`Response.cpp` + `Commands.cpp`**
   - Dispatches to command handlers
   - Writes WAL entries before mutating operations
   - Sends RESP-formatted replies

5. **`Cache.cpp` + `LRU.cpp`**
   - Owns the storage map and type-specific operations
   - Tracks recency and evicts least-recently-used keys when needed

6. **`Serialize.cpp` + `WAL.cpp`**
   - Snapshot serialize/deserialize (`costum.db`)
   - Append-only WAL (`wal.log`) with replay on startup

---

## Networking Model

- Single-process, event-driven I/O loop using epoll
- Non-blocking sockets for server and clients
- One background thread for TTL expiration scans
- Cache access is guarded by a `std::mutex` during command execution and maintenance tasks

Per-client state tracks:
- Request buffer + parsed tokens
- Response buffer + partial-send offsets
- Last activity timestamp (for timeout enforcement)
- `QUIT` flag for graceful close after response flush

---

## Protocol (RESP-style)

Commands are array-encoded in a format similar to **Redis RESP2**.

Example: `SET user:1 aymane`
```text
*3\r\n
$3\r\n
SET\r\n
$6\r\n
user:1\r\n
$6\r\n
aymane\r\n
```

Reply types used:
- Simple strings: `+OK\r\n`, `+string\r\n`, `+list\r\n`, `+none\r\n`
- Integers: `:1\r\n`, `:0\r\n`, `:-2\r\n`, etc.
- Bulk strings: `$<len>\r\n<data>\r\n` and null bulk `$-1\r\n`
- Arrays (used for `LRANGE`)
- Errors: `-ERR ...\r\n`, `-WRONGTYPE ...\r\n`

---

## Commands

Supported commands:
- `SET key value`
- `GET key`
- `DEL key`
- `EXISTS key`
- `EXPIRE key seconds`
- `TTL key`
- `FLUSH`
- `TYPE key`
- `QUIT`
- `LPUSH key value`
- `RPUSH key value`
- `LPOP key`
- `RPOP key`
- `LRANGE key start stop`
- `HSET key field value`
- `HGET key field`
- `HGETALL key`
- `HDEL key field`

Notes:
- Wrong type access returns `-WRONGTYPE ...`
- Missing keys return Redis-like null/integer forms (`$-1`, `:0`, `:-2`, or `*0`)

---

## Durability & Recovery

### Write-Ahead Log (`wal.log`)
- Mutating commands are written **before** execution completes
- Each WAL record is line-oriented:
  1. Arg count (`N`)
  2. `N` lines of raw arguments
- WAL is `fsync`’d on every append

Persisted commands:
`SET`, `DEL`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `EXPIRE`, `FLUSH`, `HSET`, `HDEL`

### Snapshot (`costum.db`)
Periodically written full-state binary image.

Current layout:
- 8 bytes magic: `\x00FTREDIS`
- 4 bytes key count
- Per key:
  1. 1 byte TTL flag
  2. 8 bytes expiration timestamp (if set)
  3. 1 byte value type
  4. 4 bytes key length
  5. key bytes
  6. Type-specific payload:
     - String: 4 bytes length + bytes
     - List: 4 bytes element count, then repeated (4 bytes length + bytes)

### Startup recovery order
1. Deserialize snapshot from `costum.db` (if present)
2. Replay `wal.log` entries in order

---

## Runtime Maintenance

- TTL sweeper runs every `TTL_SLEEP_TIME` seconds
- Idle clients are closed after `TIMEOUT` seconds
- Snapshotting triggers every `SERIALIZE_TIME` seconds via `fork()`
- WAL is removed after successful snapshot (child path) and during graceful shutdown

---

## Memory Pressure Behavior

- Memory is constrained with `RLIMIT_AS`
- Allocation failures may trigger eviction of one key from the LRU queue (then retry)
- On `std::bad_alloc` during request handling, the server attempts eviction and returns:
  `-ERR memory limit reached resend request`

---

## Quick Validation (Tester Client)

```bash
python3 tester.py set user:1 aymane
python3 tester.py get user:1
python3 tester.py expire user:1 60
python3 tester.py ttl user:1
python3 tester.py lpush tasks "finish project"
python3 tester.py lrange tasks 0 10
```

---

## Source Layout

- `main.cpp`: process bootstrap + memory limit
- `Server.cpp` / `Server.hpp`: socket setup, epoll loop, command table, client lifecycle
- `Request.cpp`: RESP-style parsing pipeline
- `Response.cpp`: response state machine + dispatch
- `Commands.cpp`: command handlers
- `Cache.cpp` / `Cache.hpp`: storage engine and type operations
- `LRU.cpp`: eviction primitive
- `WAL.cpp`: append/replay log
- `Serialize.cpp`: snapshot encode/decode
- `tester.py`: protocol test client
- `Makefile`: build rules
