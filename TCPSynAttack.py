from socket import *
import time

serverName = "127.0.0.1"
serverPort = 10090

for i in range(10000):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    try:
        time.sleep(0.01)
        clientSocket.connect((serverName,serverPort))
        data = str(i)
        message = bytes(data, 'utf-8')
        clientSocket.send(message)
    except Exception as e:
        print(str(i) + "th send Error!", e)
    clientSocket.close()

