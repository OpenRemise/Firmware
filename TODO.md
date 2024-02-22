# TODO

## DCC
Switching between op and serv mode on the fly?

## HTTP -> Websocket
Should DCC use UDP or the Z21 protocol in general?
Maybe we should only use HTTP requests to generate and delete locos?

## RMT
There's a small delay between two consecutive packets... https://github.com/espressif/esp-idf/issues/13003
Problem is that this delay depends on compiler optimizations :/
I've measured:
- -O2 78.8us
- -Os 80us
- -Og 82us

## Settings
DCC BiDi option (in us, no longer bool?)
DCC timeout
DCC ZIMO0 option
DCC ACK current (>=60mA)
Current settings (4 steps)
MDU?

# Bugs
- Should WebSockets handle PING/PONG?
- Disable heartbeat fetch for MDU/ZUSI?