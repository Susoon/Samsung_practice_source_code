from socket import *

serverName = "uni.skku.edu"
serverPort = 12000

for i in range(10000):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName,serverPort))
    clientSocket.close()

