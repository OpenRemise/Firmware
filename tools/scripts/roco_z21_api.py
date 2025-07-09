import random, string
from websocket import create_connection

ws = create_connection("ws://remise.local/roco/z21/")
# message = bytearray([0x07, 0x00, 0x40, 0x00, 0xF1, 0x0A, 0xFB])
message = bytearray([0x07, 0x00, 0x40, 0x00, 0x21, 0x81, 0xA0])
for i in range(0, 10):
    ws.send(message, opcode=2)  # Change opcode to 2 to send binary
    response = ws.recv()
    print(response)
ws.close()
