#!/usr/bin/python3
import socket
import time

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

def main():
    s.connect((HOST, PORT))
    for i in range(1000000000):
        data = Set(f"key{i}", f"value{i}")
        if data.decode() == "-ERR Value Not Set\r\n":
            s.close()
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((HOST, PORT))
    s.close()

if __name__ == "__main__":
    main()
