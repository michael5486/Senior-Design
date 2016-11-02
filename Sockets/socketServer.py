import socket

ip = '192.168.1.27'
port = 8089

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind((ip, port))
serversocket.listen(5) # become a server socket, maximum 5 connections

print "Listening on {} on port {}".format(ip, port)

while True:
    connection, address = serversocket.accept()
    buf = connection.recv(64)
    if len(buf) > 0:
        print buf
        break
