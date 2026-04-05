#!/usr/bin/python3
import socket

HOST = '127.0.0.1'
PORT = 8080 
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    message = b"*2\r\n$1\r\na\r\n$2\r\naa\r\n"
    s.send(message)
    data = s.recv(1024)

print(f"Received from server: {data.decode('utf-8')}")
