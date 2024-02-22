# Firmware

## Install ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone -b v5.2 --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.fish all

## Fuses
With the help of the Fuse "STRAP_JTAG_SEL" you can use GPIO3 to switch between internal USB and external JTAG debugger. The PCB contains a jumper called "CONFIG" for that.

## Partition table
A custom partition table (partitions.csv) is used to specify the sizes of the various storage areas. One of those areas ("storage") is used for a SPIFFS partition. SPIFFS is a filesystem for NOR flashes and this partition will be used to store the frontend. The size of the storage is the whole flash size, minus all other partitions minus 52K from... who knows.
(32M - 8M - 4M - 4M - 8K - 4K - 52K)

## JSON online viewer
http://jsonviewer.stack.hu/#http://github.com

## REST API
https://stackoverflow.com/questions/60761955/rest-update-best-practice-put-collection-id-without-id-in-body-vs-put-collecti
https://stackoverflow.com/questions/43391191/can-i-put-without-an-id
https://zditect.com/blog/57373041.html
https://softwareengineering.stackexchange.com/questions/263925/to-include-a-resource-id-in-the-payload-or-to-derive-from-uri

## Status codes
A great overview about HTTP status codes
https://www.codetinkerer.com/2015/12/04/choosing-an-http-status-code.html
httpd_resp_send* functions of the ESP-IDF implicitly set content type text/html and status code 200!

## POST vs PUT
https://www.youtube.com/watch?v=41y6VwtvgwE

## mDNS
I've restricted the mDNS name to (.+\.)?wulf  
This ensures that the DNS can be found by lookup, even when using an app or desktop version instead of web.
For some reason mDNS will never be supported by "older" android versions...
https://issuetracker.google.com/issues/140786115?pli=1  
Listing mDNS services with linux: avahi-browse -a

## IRAM
Be careful when using the IRAM attribute! Even harmless lookings things like constexpr data will cause an exception when accessed inside a function placed in IRAM.

## Performance
It's a good idea to seperate Espressif system tasks and application tasks. The convention seems to be that Espressifs own stuff runs on core0 whereas the app runs on core1. Tasks with priorities >=19 on core1 can practically no longer be interrupted. ESP-IDF things which need to be pinned to specific core:
- LWIP
- TCPIP
- mDNS
- HTTP servers

## NVS
According to Espressif its smarter to store many small values in NVS than one large binary blob. So it might be a good idea to store e.g. loco data individually?
It is safe to call nvs_set_* functions even if the value to be stored hasn't actually changed. The underlying ESP-IDF code performs a "sanity check" and only writes values which really differs, see the nvs_storage.cpp source for details:
https://github.com/espressif/esp-idf/blob/c77b5752ef9e4e40c2e819d511b1dd7b83be2996/components/nvs_flash/src/nvs_storage.cpp

## Log
LOG_LOCAL_LEVEL is fucking useless  
https://github.com/espressif/esp-idf/issues/8570

## Debug
openocd -f esp32s3-builtin.cfg

xtensa-esp32s3-elf-gdb -ex 'set pagination off' -ex 'target remote :3333' ./build/Firmware.elf