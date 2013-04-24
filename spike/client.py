
from gevent import socket

conn = socket.create_connection(('localhost', 9995))

print conn.recv(100)

