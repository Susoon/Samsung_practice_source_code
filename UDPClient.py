from socket import *

serverName = 'uni.skku.edu'
serverPort = 12001
clientSocket = socket(AF_INET, SOCK_DGRAM)
message = input('Input lowercase sentence:')
clientSocket.sendto(message.encode(), (serverName, serverPort))
modifiedMessage, serverAddress = clientSocket.recvfrom(2048)
print(serverAddress)
print(modifiedMessage.decode())
clientSocket.close()

