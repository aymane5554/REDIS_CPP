# KV Cache Server

Redis-inspired in-memory key-value server implemented in C++ for Linux.

This project focuses on systems-level backend concerns: non-blocking I/O, custom protocol parsing, data durability, memory pressure handling, and multi-client lifecycle management.

## Core Capabilities

- Non-blocking TCP server using epoll
- RESP-like request parsing and command dispatch
- In-memory string and list value types
- Key expiration (TTL) with periodic cleanup
- LRU-style eviction when allocation fails
- Durability via Write-Ahead Log + periodic snapshots
- Crash recovery by loading snapshot then replaying WAL
- Idle connection timeout cleanup

## Build And Run

Requirements:
- Linux
- g++
- make
- python3 (optional, for tester script)

Build:

```bash
make
```

Run:

```bash
./redihh
```

Defaults:
- Port: `8080`
- Address-space limit: `16 MB` (`RLIMIT_AS`)
- TTL sweep period: `60s`
- Snapshot period: `30s`
- Client idle timeout: `10s`

## High-Level Architecture

1. `main.cpp`
- Installs `SIGPIPE` ignore.
- Applies process memory cap with `setrlimit(RLIMIT_AS, ...)`.
- Starts the server event loop.

2. `Server.cpp`
- Creates listening socket, configures non-blocking mode.
- Registers server socket in epoll.
- Accepts clients and tracks per-client state.
- Runs the main event loop (`EPOLLIN`, `EPOLLOUT`, `EPOLLHUP`, `EPOLLERR`).
- Spawns a background TTL cleanup thread.
- Triggers periodic snapshotting in a forked child process.

3. `Request.cpp`
- Parses RESP-like frames from each client receive buffer.
- Supports partial TCP reads by accumulating client buffers.

4. `Response.cpp` + `Commands.cpp`
- Dispatches to command handlers.
- Writes WAL entries before mutating operations.
- Sends RESP-formatted replies.

5. `Cache.cpp` + `LRU.cpp`
- Owns key/value map and type-specific operations.
- Tracks recency and evicts least-recently-used keys when needed.

6. `Serialize.cpp` + `WAL.cpp`
- Snapshot serialize/deserialize (`costum.db`).
- Append-only WAL (`wal.log`) with replay on startup.

## Networking Model

- Single process, event-driven I/O loop with epoll.
- Non-blocking sockets for server and clients.
- One background thread for periodic TTL expiration scans.
- Shared cache access guarded by a `std::mutex` during command execution and maintenance tasks.

Per-client state tracks:
- Request buffer and parsed command tokens.
- Response buffer and partial-send offsets.
- Last activity timestamp for timeout enforcement.
- QUIT flag for graceful close after response flush.

## Protocol Details (RESP-Style)

The server expects array-encoded commands similar to Redis RESP2.

Example (`SET user:1 aymane`):

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
- Arrays for `LRANGE`
- Errors with `-ERR ...\r\n` and `-WRONGTYPE ...\r\n`

## Command Surface

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

Important implementation notes:
- Wrong type access returns `-WRONGTYPE ...`.
- Missing keys commonly return Redis-like null/integer forms (`$-1`, `:0`, `:-2`, or `*0`).

## Durability And Recovery

### WAL (`wal.log`)

- Mutating commands are written before execution response completion.
- Each WAL record is line-oriented:
1. Arg count line (`N`)
2. `N` lines of raw arguments
- File is `fsync`'d on every append.

Mutating commands persisted:
- `SET`, `DEL`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `EXPIRE`, `FLUSH`

### Snapshot (`costum.db`)

Periodically written full-state binary image.

Current file layout:
- 8 bytes magic: `\x00FTREDIS`
- 4 bytes key count
- Per key:
1. 1 byte TTL flag
2. 8 bytes expiration timestamp (if TTL flag set)
3. 1 byte value type
4. 4 bytes key length
5. key bytes
6. Type-specific payload:
   - String: 4 bytes length + bytes
   - List: 4 bytes element count, then repeated (4 bytes length + bytes)

### Startup Recovery Order

1. Deserialize snapshot from `costum.db` (if present).
2. Replay `wal.log` entries in order.

This design gives lower restart latency than WAL-only recovery while preserving post-snapshot writes.

## Runtime Maintenance

- TTL sweeper thread runs every `TTL_SLEEP_TIME` seconds and removes expired keys.
- Idle clients are closed when inactive for `TIMEOUT` seconds.
- Snapshotting is triggered every `SERIALIZE_TIME` seconds using `fork()`.
- WAL file is removed after successful snapshot (child path) and during graceful shutdown.

## Memory Pressure Behavior

- Process memory is constrained with `RLIMIT_AS`.
- Allocation failures are handled by evicting one key from the LRU queue and retrying in selected paths.
- On `std::bad_alloc` during request handling, the server attempts eviction and returns:
  `-ERR memory limit reached resend request`

## Quick Validation

Use the included client:

```bash
python3 tester.py set user:1 aymane
python3 tester.py get user:1
python3 tester.py expire user:1 60
python3 tester.py ttl user:1
python3 tester.py lpush tasks "finish project"
python3 tester.py lrange tasks 0 10
```

## Source Layout

- `main.cpp`: process bootstrap and memory limit
- `Server.cpp` / `Server.hpp`: socket setup, epoll loop, command table, client lifecycle
- `Request.cpp`: RESP-style parse pipeline
- `Response.cpp`: response state machine and dispatch
- `Commands.cpp`: command handlers
- `Cache.cpp` / `Cache.hpp`: storage engine and type operations
- `LRU.cpp`: eviction primitive
- `WAL.cpp`: append/replay log
- `Serialize.cpp`: snapshot encode/decode
- `tester.py`: protocol test client
- `Makefile`: build rules

## Known Gaps / Next Engineering Steps

- Add snapshot checksum/CRCn validation and corruption handlig.
- Harden parser and command validation with fuzz/property tests.
- Add integration tests for restart/crash-recovery scenarios.
- Expose runtime configuration (port, memory, intervals) via CLI/env.
- Add metrics (latency, ops/sec, evictions, active clients).