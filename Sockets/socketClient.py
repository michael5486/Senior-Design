import socket, time

ip = '192.168.1.27'
port = 8089

print "Sending data to {} on port {}".format(ip,port)

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect((ip,port))

for x in (0,10)
	clientsocket.send('hello')
	clientsocket.send(x)
	time.sleep(1)
