# redis++

A Redis-inspired, persistent key-value server written in **C++ for Linux** — built from scratch as a systems engineering deep-dive.

Implements the full Redis operational model: **RESP2 wire protocol**, **non-blocking epoll I/O**, **Write-Ahead Logging**, **fork-based snapshotting**, and **LRU eviction under memory pressure** — without any external dependencies.

---

## Why this project exists

Most key-value store tutorials stop at a hash map behind a TCP socket. This one doesn't.

The goal was to understand what makes production systems like Redis actually work: how do you guarantee durability without destroying write throughput? How do you serve thousands of clients on a single thread? How do you take a consistent snapshot of live data without pausing the server?

Every design decision here — epoll over threads, WAL before mutation, fork() for copy-on-write snapshots — was made deliberately and mirrors how Redis itself works under the hood.

---

## What it does

- **Non-blocking TCP server** using `epoll` — single-process, event-driven, handles multiple concurrent clients without threads
- **RESP2 protocol** — wire-compatible command framing, same as Redis clients use
- **Three data types**: Strings, Lists, Hashmaps — each with type-safe access and `-WRONGTYPE` enforcement
- **TTL / expiry** — lazy expiry on access + periodic background sweeper thread
- **LRU eviction** — doubly linked list + hashmap, O(1) evict on memory pressure detected via `RLIMIT_AS` + `std::bad_alloc`
- **Write-Ahead Log** — every mutating command is `fsync`'d to disk before the client receives `+OK`
- **Fork-based snapshots** — child process serializes a consistent binary snapshot using copy-on-write semantics; parent keeps serving writes uninterrupted
- **Crash recovery** — on startup: load snapshot → replay WAL → ready to serve

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    redis++ process                   │
│                                                      │
│  ┌──────────┐    ┌────────────┐    ┌─────────────┐  │
│  │  epoll   │───▶│  Request   │───▶│  Commands   │  │
│  │  loop    │    │  parser    │    │  dispatch   │  │
│  └──────────┘    └────────────┘    └──────┬──────┘  │
│       │                                   │         │
│  accepts/reads                     ┌──────▼──────┐  │
│  client fds                        │    Cache    │  │
│                                    │  (mutex)    │  │
│  ┌──────────┐                      └──────┬──────┘  │
│  │  TTL     │ background thread           │         │
│  │  sweeper │ (mutex-guarded)      ┌──────▼──────┐  │
│  └──────────┘                      │  WAL.cpp    │  │
│                                    │  fsync()    │  │
│  ┌──────────┐                      └─────────────┘  │
│  │  fork()  │ child process                         │
│  │snapshot  │ copy-on-write                         │
│  └──────────┘                                       │
└─────────────────────────────────────────────────────┘
```

**Key design decisions:**

- **epoll over thread-per-client**: avoids context switching and stack allocation overhead for thousands of idle connections. One thread handles all I/O readiness events in O(1) per active fd.
- **WAL before mutation**: durability guarantee — if the process crashes after the client gets `+OK`, the command is already on disk. `fsync()` forces the kernel page cache to physical storage.
- **fork() for snapshots**: the child inherits the parent's entire memory space at zero cost (copy-on-write). Pages shared between parent and child are only copied when the parent writes to them. This gives the child a consistent, frozen view of the dataset while the parent serves live traffic uninterrupted.
- **RLIMIT_AS for memory pressure**: caps the process address space. When allocation fails (`std::bad_alloc`), the server evicts the least-recently-used key and retries — instead of crashing.

---

## Protocol (RESP2)

Commands are array-encoded following the Redis RESP2 specification.

Example — `SET user:1 aymane`:
```
*3\r\n$3\r\nSET\r\n$6\r\nuser:1\r\n$6\r\naymane\r\n
```

Reply types:
| Type | Example |
|------|---------|
| Simple string | `+OK\r\n` |
| Integer | `:1\r\n` |
| Bulk string | `$6\r\naymane\r\n` |
| Null bulk | `$-1\r\n` |
| Array | `*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n` |
| Error | `-ERR unknown command\r\n` |
| Wrong type | `-WRONGTYPE operation against wrong type\r\n` |

---

## Commands

### Strings
| Command | Description |
|---------|-------------|
| `SET key value` | Set a string value |
| `GET key` | Get a string value |
| `DEL key` | Delete a key |
| `EXISTS key` | Returns 1 if key exists, 0 otherwise |
| `TYPE key` | Returns the type of the value |

### TTL
| Command | Description |
|---------|-------------|
| `EXPIRE key seconds` | Set expiry in seconds |
| `TTL key` | Returns remaining TTL; -1 if no expiry, -2 if missing |

### Lists
| Command | Description |
|---------|-------------|
| `LPUSH key value` | Prepend to list |
| `RPUSH key value` | Append to list |
| `LPOP key` | Remove and return head |
| `RPOP key` | Remove and return tail |
| `LRANGE key start stop` | Return sublist (Redis-compatible indexing) |

### Hashmaps
| Command | Description |
|---------|-------------|
| `HSET key field value` | Set a hash field |
| `HGET key field` | Get a hash field |
| `HGETALL key` | Get all fields and values |
| `HDEL key field` | Delete a hash field |

### Server
| Command | Description |
|---------|-------------|
| `FLUSH` | Delete all keys |
| `QUIT` | Graceful client disconnect |

---

## Durability & Recovery

### Write-Ahead Log

Every mutating command is logged to `wal.log` **before** the in-memory operation completes. Each record stores the raw argument count followed by argument bytes. `fsync()` is called on every append.

On crash: the log is replayed in order on startup to reconstruct mutations that occurred after the last snapshot.

Persisted commands: `SET`, `DEL`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `EXPIRE`, `FLUSH`, `HSET`, `HDEL`

### Binary Snapshot Format

Snapshots are written to `custom.db` in a custom binary format:

```
[8 bytes]  Magic: \x00FTREDIS
[4 bytes]  Key count (uint32)

Per key:
  [1 byte]   TTL flag (0 = no expiry, 1 = has expiry)
  [8 bytes]  Expiration timestamp in ms (if TTL flag set)
  [1 byte]   Value type (0=String, 1=List, 2=Hashmap)
  [4 bytes]  Key length
  [N bytes]  Key bytes

  String payload:
    [4 bytes]  Value length
    [N bytes]  Value bytes

  List payload:
    [4 bytes]  Element count
    Per element:
      [4 bytes]  Element length
      [N bytes]  Element bytes

  Hashmap payload:
    [4 bytes]  Field count
    Per field:
      [4 bytes]  Field name length
      [N bytes]  Field name bytes
      [4 bytes]  Field value length
      [N bytes]  Field value bytes
```

The magic header identifies file type and rejects corrupt or misidentified files on load.

### Recovery sequence

```
startup
  └─▶ load custom.db (if exists)
        └─▶ replay wal.log (if exists)
              └─▶ ready
```

WAL is truncated after a successful snapshot. On graceful shutdown, a final snapshot is written.

---

## Memory Pressure Behavior

The process address space is hard-capped using `setrlimit(RLIMIT_AS, ...)` at startup (default: 16MB).

When any allocation fails:
1. Server catches `std::bad_alloc`
2. Evicts the least-recently-used key from the LRU queue
3. Retries the allocation
4. If still failing, returns `-ERR memory limit reached resend request`

The LRU queue is a doubly linked list + hashmap — O(1) eviction and O(1) promotion on access.

---

## Build & Run

### Requirements
- Linux
- `g++` (C++17)
- `make`
- `python3` (optional — test client)

### Build
```bash
git clone https://github.com/yourusername/redis-plus-plus
cd redis-plus-plus
make
```

### Run
```bash
./redis++
```

### Run with options
```bash
./redis++ --port 9090 --memory-mb 64 --ttl-interval 15 --snapshot-interval 20 --client-timeout 30
```

### All options
| Flag | Default | Description |
|------|---------|-------------|
| `--port` | `8080` | TCP listening port |
| `--memory-mb` | `16` | Address space cap (RLIMIT_AS) in MB |
| `--ttl-interval` | `60` | Expired-key sweep interval in seconds |
| `--snapshot-interval` | `30` | Snapshot write interval in seconds |
| `--client-timeout` | `10` | Idle client disconnect threshold in seconds |
| `--db-file` | `custom.db` | Snapshot file path |
| `--wal-file` | `wal.log` | WAL file path |

---

## Quick Test

```bash
python3 client.py set user:1 aymane
python3 client.py get user:1
python3 client.py expire user:1 60
python3 client.py ttl user:1
python3 client.py hset profile name aymane
python3 client.py hgetall profile
python3 client.py lpush tasks "finish project"
python3 client.py lrange tasks 0 10
```

---

## Source Layout

| File | Responsibility |
|------|---------------|
| `main.cpp` | Process bootstrap, memory limit, signal handling |
| `Server.cpp/hpp` | epoll loop, client lifecycle, command table |
| `Request.cpp` | RESP2 frame parser, partial-read buffering |
| `Response.cpp` | Response state machine, partial-write handling |
| `Commands.cpp` | Command handlers |
| `Cache.cpp/hpp` | Storage engine, type dispatch, mutex |
| `LRU.cpp` | Eviction primitive (doubly linked list + hashmap) |
| `WAL.cpp` | Append-only log, fsync, replay |
| `Serialize.cpp` | Binary snapshot encode/decode |
| `client.py` | RESP2 test client |
| `Makefile` | Build rules |

---