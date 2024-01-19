# TODO

# Install ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone -b v5.2-beta1 --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.fish all

# Hardware improvements
- TVS (e.g. SMF24A) for track outputs
- During production PCBWay asked me the polarity of D2, U4, U5 and U6

# KiCad
Default silkscreen size
Width 1mm
Height 1mm
Thickness 0.15mm

## RevE
RevE could place the "deglitch capacitor" on top (once I figured out its value...)

# ESP32-S3-WROOM-2-N32R8V

## Flash settings
- Octal
- 80MHz
- DDR
- 32MB

## SPIRAM settings
- 80MHz

## Fuses
With the help of the Fuse "STRAP_JTAG_SEL" you can use GPIO3 to switch between internal USB and external JTAG debugger. The PCB contains a jumper called "CONFIG" for that.

## Partition table
A custom partition table (partitions.csv) is used to specify the sizes of the various storage areas. One of those areas ("storage") is used for a SPIFFS partition. SPIFFS is a filesystem for NOR flashes and this partition will be used to store the frontend. The size of the storage is the whole flash size, minus all other partitions minus 52K from... who knows.
(32M - 8M - 4M - 4M - 8K - 4K - 52K)

## WiFi
WiFi is optimized according to [Espressif's guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/wifi.html#how-to-improve-wi-fi-performance). With a few exceptions, I took over most of them. A few buffers had to be smaller to save internal RAM.

## HTTP
A restrict pattern for SSID and password inputs would be great... Sadly all regex I found relies on a syntax containg {} braces which are also used for formatting the "ap_server.html" file.

### JSON online viewer
http://jsonviewer.stack.hu/#http://github.com

### REST API
https://stackoverflow.com/questions/60761955/rest-update-best-practice-put-collection-id-without-id-in-body-vs-put-collecti
https://stackoverflow.com/questions/43391191/can-i-put-without-an-id
https://zditect.com/blog/57373041.html
https://softwareengineering.stackexchange.com/questions/263925/to-include-a-resource-id-in-the-payload-or-to-derive-from-uri

### Status codes
A great overview about HTTP status codes
https://www.codetinkerer.com/2015/12/04/choosing-an-http-status-code.html
httpd_resp_send* functions of the ESP-IDF implicitly set content type text/html and status code 200!

### POST vs PUT
https://www.youtube.com/watch?v=41y6VwtvgwE

## mDNS
I've restricted the mDNS name to (.+\.)?wulf  
This ensures that the DNS can be found by lookup, even when using an app or desktop version instead of web.
For some reason mDNS will never be supported by "older" android versions...
https://issuetracker.google.com/issues/140786115?pli=1  
Listing mDNS services with linux: avahi-browse -a

## IRAM
Be careful when using the IRAM attribute! Even harmless lookings things like constexpr data will cause an exception when accessed inside a function placed in IRAM.

## GPTimer
GPTimer has it's limit when used for creating signals. A 50µs DCC signal could most likely be generated within it's tolerance, a 10µs MDU signal can't.

## Performance
It's a good idea to seperate Espressif system tasks and application tasks. The convention seems to be that Espressifs own stuff runs on core0 whereas the app runs on core1. Tasks with priorities >=19 on core1 can practically no longer be interrupted. ESP-IDF things which need to be pinned to specific core:
- LWIP
- TCPIP
- mDNS
- HTTP servers

## FreeRTOS
Tick is set at 100Hz. This means delays below 10ms are pointless!

## NVS
According to Espressif its smarter to store many small values in NVS than one large binary blob. So it might be a good idea to store e.g. loco data individually?
It is safe to call nvs_set_* functions even if the value to be stored hasn't actually changed. The underlying ESP-IDF code performs a "sanity check" and only writes values which really differs, see the nvs_storage.cpp source for details:
https://github.com/espressif/esp-idf/blob/c77b5752ef9e4e40c2e819d511b1dd7b83be2996/components/nvs_flash/src/nvs_storage.cpp

## Log
LOG_LOCAL_LEVEL is fucking useless  
https://github.com/espressif/esp-idf/issues/8570

# Component Manager
I don't like Espressifs CMake build system and their component manager, yet it's difficult to work against it. E.g. you can't choose your own fucking component name. If you have your sources in a "src" folder as most projects do, your component will be named __idf_src or some shit like that. Great... now try to include 2 libs with a src folder.

# Bugs
- DCC ain't working with anything lower than 21 preamble bits (presumably because of the invalid bit in between packets, see dcc_encoder_config function for details)
- DCC cutout is too long, we'd have to jump some serious loops to make this compliant. The next RMT packet would need to be issued before the end of channel 2. There is a delay of AT LEAST 50µs before a packet sent to RMT gets actually transmitted.
- Should WebSockets handle PING/PONG?
- Disable heartbeat fetch for MDU/ZUSI?