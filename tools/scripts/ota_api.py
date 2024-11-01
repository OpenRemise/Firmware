import random, os, string, time
from websocket import create_connection


def read_in_chunks(file, chunk_size=1024):
    while True:
        data = file.read(chunk_size)
        if not data:
            break
        yield data


start_time = time.time()
ws = create_connection("ws://remise.local/ota/")
file_path = os.path.dirname(os.path.abspath(__file__))
with open(file_path + "/WULF_0.0.1-23-ga317b94.bin", mode="rb") as file:
    for chunk in read_in_chunks(file):
        ws.send(chunk, opcode=2)  # Change opcode to 2 to send binary
        response = ws.recv()
        if response != bytes(b"\x06"):
            break
ws.close()
end_time = time.time()
print("Executed in " + str(end_time - start_time))
