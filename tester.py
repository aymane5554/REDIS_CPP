#!/usr/bin/python3
import socket
import sys
import random

HOST = '127.0.0.1'
PORT = 8080
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def Set(key, value):
    message = f"*3\r\n$3\r\nSET\r\n${len(key)}\r\n{key}\r\n${len(value)}\r\n{value}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"SET {key} {value} {data.decode('utf-8')}")
    return data

def Get(key):
    message = f"*2\r\n$3\r\nGET\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"GET {key} {data.decode('utf-8')}")

def Expire(key, seconds):
    message = f"*3\r\n$6\r\nEXPIRE\r\n${len(key)}\r\n{key}\r\n${len(str(seconds))}\r\n{seconds}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"EXPIRE {key} {seconds} {data.decode('utf-8')}")

def TTL(key):
    message = f"*2\r\n$3\r\nTTL\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"TTL {key} {data.decode('utf-8')}")

def Del(key):
    message = f"*2\r\n$3\r\nDEL\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"DEL {key} {data.decode('utf-8')}")

def Exists(key):
    message = f"*2\r\n$6\r\nEXISTS\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"EXISTS {key} {data.decode('utf-8')}")

def Flush():
    message = b"*1\r\n$5\r\nFLUSH\r\n"
    s.send(message)
    data = s.recv(1024)
    print(f"FLUSH {data.decode('utf-8')}")

def Lru():
    message = b"*1\r\n$3\r\nLRU\r\n"
    s.send(message)
    data = s.recv(1024)
    print(f"LRU {data.decode('utf-8')}")

USAGE = """
Usage: script.py <command> [args...]

Commands:
  set    <key> <value>
  get    <key>
  expire <key> <seconds>
  ttl    <key>
  del    <key>
  exists <key>
  flush
"""

DISPATCH = {
    "set":    (Set,    ["key", "value"]),
    "get":    (Get,    ["key"]),
    "expire": (Expire, ["key", "seconds"]),
    "ttl":    (TTL,    ["key"]),
    "del":    (Del,    ["key"]),
    "exists": (Exists, ["key"]),
    "flush":  (Flush,  []),
    "lru":  (Lru,  []),
}

def main():
    if len(sys.argv) < 2:
        print(USAGE)
        sys.exit(1)

    cmd = sys.argv[1].lower()

    if cmd not in DISPATCH:
        print(f"Unknown command: '{cmd}'{USAGE}")
        sys.exit(1)

    fn, params = DISPATCH[cmd]
    args = sys.argv[2:]

    if len(args) != len(params):
        print(f"'{cmd}' expects {len(params)} arg(s): {' '.join(params)}")
        sys.exit(1)

    if cmd == "expire":
        args[1] = int(args[1])

    s.connect((HOST, PORT))
    fn(*args)
    s.close()

if __name__ == "__main__":
    main()