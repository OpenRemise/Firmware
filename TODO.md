# TODO

## DCC Service Mode
Do we need to change the sampling frequency? It's currently only 1kHz.

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
DCC BiDi option (in us, no longer bool)
ZIMO0 option
Current settings

# Bugs
- Should WebSockets handle PING/PONG?
- Disable heartbeat fetch for MDU/ZUSI?
- MDU update for old bootloaders (2.2.0) doesn't seem to work?
  - Looks like the old revision has a higher inrush current, maybe use 4.1A for entry and then switch back to 0.5A?
  - MS990 has same problem
  - Update: ok, i have no clue what causes this issue... maybe it's timing related? maybe spamming busy isn't such a good idea?