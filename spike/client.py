
from gevent import socket


for i in range(100000):
	conn = socket.create_connection(('localhost', 9980))


	print conn.recv(100)

	break

