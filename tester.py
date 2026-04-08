#!/usr/bin/python3
import socket
import time

HOST = '127.0.0.1'
PORT = 8080 

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    message = b"*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$3\r\nbbb\r\n"
    s.send(message)
    data = s.recv(1024)
    s.close()
print(f"Received from server:{data.decode('utf-8')}")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    message = b"*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n"
    s.send(message)
    data = s.recv(1024)
    s.close()
print(f"Received from server:{data.decode('utf-8')}")