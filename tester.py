#!/usr/bin/python3
import socket

HOST = '127.0.0.1'
PORT = 8080 
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    message = b"*0"
    m2 = b"2\r\n$10\r\n"
    m3 = b"012345"
    m4 = b"6789\r\n$2\r\naa\r\n"
    s.send(message)
    s.send(m2)
    s.send(m3)
    s.send(m4)
    data = s.recv(1024)

print(f"Received from server: {data.decode('utf-8')}")
