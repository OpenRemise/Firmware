import random, string
from websocket import create_connection


ws = create_connection("ws://wulf.local/mdu/firmware/")
packet = bytearray([i % 256 for i in range(0, 100)])
for i in range(0, 10):
    ws.send(packet, opcode=2)  # Change opcode to 2 to send binary
    response = ws.recv()
    print(response)
ws.close()
