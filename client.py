#!/usr/bin/python3
import socket
import sys

def Lpush(key, *value, s):
    message = f"*{len(value) + 2}\r\n$5\r\nLPUSH\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    for v in value:
        message += f"${len(v)}\r\n{v}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"LPUSH {key} {data.decode('utf-8')}")
    return data

def Rpush(key, *value, s):
    message = f"*{len(value) + 2}\r\n$5\r\nRPUSH\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    for v in value:
        message += f"${len(v)}\r\n{v}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"RPUSH {key} {data.decode('utf-8')}")
    return data

def Lrange(key, start, stop, s):
    message = f"*4\r\n$6\r\nLRANGE\r\n${len(key)}\r\n{key}\r\n${len(start)}\r\n{start}\r\n${len(stop)}\r\n{stop}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"Lrange {key} {data.decode('utf-8')}")
    return data

def Hset(key, field, value, s):
    message = f"*4\r\n$4\r\nHSET\r\n${len(key)}\r\n{key}\r\n${len(field)}\r\n{field}\r\n${len(value)}\r\n{value}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"HSET {key} {field} {data.decode('utf-8')}")
    return data

def Hget(key, field, s):
    message = f"*3\r\n$4\r\nHGET\r\n${len(key)}\r\n{key}\r\n${len(field)}\r\n{field}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"HGET {key} {field} {data.decode('utf-8')}")
    return data

def Hgetall(key, s):
    message = f"*2\r\n$7\r\nHGETALL\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"HGETALL {key} {data.decode('utf-8')}")
    return data

def Hdel(key, field, s):
    message = f"*3\r\n$4\r\nHDEL\r\n${len(key)}\r\n{key}\r\n${len(field)}\r\n{field}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"HDEL {key} {field} {data.decode('utf-8')}")
    return data

def Set(key, value, s):
    message = f"*3\r\n$3\r\nSET\r\n${len(key)}\r\n{key}\r\n${len(value)}\r\n{value}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"SET {key} {value} {data.decode('utf-8')}")
    return data

def Get(key, s):
    message = f"*2\r\n$3\r\nGET\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"GET {key} {data.decode('utf-8')}")

def Expire(key, seconds, s):
    message = f"*3\r\n$6\r\nEXPIRE\r\n${len(key)}\r\n{key}\r\n${len(str(seconds))}\r\n{seconds}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"EXPIRE {key} {seconds} {data.decode('utf-8')}")

def TTL(key, s):
    message = f"*2\r\n$3\r\nTTL\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"TTL {key} {data.decode('utf-8')}")

def Del(key, s):
    message = f"*2\r\n$3\r\nDEL\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"DEL {key} {data.decode('utf-8')}")

def Lpop(key, s):
    message = f"*2\r\n$4\r\nLPOP\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"LPOP {key} {data.decode('utf-8')}")

def Rpop(key, s):
    message = f"*2\r\n$4\r\nRPOP\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"RPOP {key} {data.decode('utf-8')}")

def Exists(key, s):
    message = f"*2\r\n$6\r\nEXISTS\r\n${len(key)}\r\n{key}\r\n".encode('utf-8')
    s.send(message)
    data = s.recv(1024)
    print(f"EXISTS {key} {data.decode('utf-8')}")

def Flush(s):
    message = b"*1\r\n$5\r\nFLUSH\r\n"
    s.send(message)
    data = s.recv(1024)
    print(f"FLUSH {data.decode('utf-8')}")

def Lru(s):
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
  lpush  <key> <values>
  rpush  <key> <values>
  lpop   <key>
  rpop   <key>
  flush
  lru
  hset   <key> <field> <value>
  hget   <key> <field>
  hgetall <key>
  hdel   <key> <field>
"""

DISPATCH = {
    "set":    (Set,    ["key", "value"]),
    "get":    (Get,    ["key"]),
    "expire": (Expire, ["key", "seconds"]),
    "ttl":    (TTL,    ["key"]),
    "del":    (Del,    ["key"]),
    "lpop":    (Lpop,    ["key"]),
    "rpop":    (Rpop,    ["key"]),
    "exists": (Exists, ["key"]),
    "flush":  (Flush,  []),
    "lpush": (Lpush, ["key", "value"]),
    "rpush": (Rpush, ["key", "value"]),
    "lrange": (Lrange, ["key", "start", "stop"]),
    "lru":  (Lru,  []),
    "hset": (Hset, ["key", "field", "value"]),
    "hget": (Hget, ["key", "field"]),
    "hgetall": (Hgetall, ["key"]),
    "hdel": (Hdel, ["key", "field"]),
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

    if cmd == "lpush":
        if len(args) < len(params):
            print(f"'{cmd}' expects {len(params)} arg(s): {' '.join(params)}")
            sys.exit(1)
    else:
        if len(args) != len(params):
            print(f"'{cmd}' expects {len(params)} arg(s): {' '.join(params)}")
            sys.exit(1)

    if cmd == "expire":
        args[1] = int(args[1])
    
    HOST = '127.0.0.1'
    PORT = 8080
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    fn(*args, s=s)
    s.close()

if __name__ == "__main__":
    main()