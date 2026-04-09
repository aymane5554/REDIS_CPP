#!/usr/bin/python3
import socket
import time

HOST = '127.0.0.1'
PORT = 8080 

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

message = b"*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$3\r\nbbb\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST SET {data.decode('utf-8')}")

message = b"*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST GET {data.decode('utf-8')}")

message = b"*3\r\n$6\r\nEXPIRE\r\n$3\r\nkey\r\n$1\r\n5\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST EXPIRE {data.decode('utf-8')}")

message = b"*2\r\n$3\r\nTTL\r\n$3\r\nkey\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST TTL {data.decode('utf-8')}")

message = b"*2\r\n$3\r\nDEL\r\n$3\r\nkey\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST DEL {data.decode('utf-8')}")

message = b"*2\r\n$6\r\nEXISTS\r\n$3\r\nkey\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST EXISTS {data.decode('utf-8')}")

message = b"*1\r\n$5\r\nFLUSH\r\n"
s.send(message)
data = s.recv(1024)
print(f"TEST FLUSH {data.decode('utf-8')}")

s.close()
