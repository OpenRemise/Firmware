import random, string
from websocket import create_connection


ws = create_connection("ws://remise.local/zimo/zusi/")
message = bytearray([i % 256 for i in range(0, 100)])
# message = "".join(random.choices(string.ascii_uppercase + string.digits, k=1024))
for i in range(0, 10):
    ws.send(message, opcode=2)  # Change opcode to 2 to send binary
    response = ws.recv()
    print(response)
ws.close()
