# Firmware

[![build](https://github.com/OpenRemise/Firmware/actions/workflows/build.yml/badge.svg)](https://github.com/OpenRemise/Firmware/actions/workflows/build.yml) [![tests](https://github.com/OpenRemise/Firmware/actions/workflows/tests.yml/badge.svg)](https://github.com/OpenRemise/Firmware/actions/workflows/tests.yml) [![license](https://img.shields.io/github/license/OpenRemise/Firmware)](https://github.com/OpenRemise/Firmware/raw/master/LICENSE)

<a href="https://openremise.at">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://github.com/OpenRemise/.github/raw/master/data/icons/icon_dark.svg">
  <img src="https://github.com/OpenRemise/.github/raw/master/data/icons/icon_light.svg" width="20%" align="right">
</picture>
</a>

## RMT
There's a small delay between two consecutive packets... https://github.com/espressif/esp-idf/issues/13003
Problem is that this delay depends on compiler optimizations :/
I've measured:
- -O2 78.8us
- -Os 80us
- -Og 82us

## Documentation
https://github.com/wireviz/WireViz
https://hackaday.com/2024/03/11/share-your-projects-kicad-automations-and-pretty-renders/
https://github.com/openscopeproject/InteractiveHtmlBom
https://github.com/yaqwsx/PcbDraw
https://yaqwsx.github.io/Pinion/

# Notes
## JSON online viewer
https://jsonviewer.stack.hu

## REST API
https://stackoverflow.com/questions/60761955/rest-update-best-practice-put-collection-id-without-id-in-body-vs-put-collecti
https://stackoverflow.com/questions/43391191/can-i-put-without-an-id
https://zditect.com/blog/57373041.html
https://softwareengineering.stackexchange.com/questions/263925/to-include-a-resource-id-in-the-payload-or-to-derive-from-uri

## Status codes
A great overview about HTTP status codes
https://www.codetinkerer.com/2015/12/04/choosing-an-http-status-code.html
httpd_resp_send* functions of the ESP-IDF implicitly set content type text/html and status code 200!

## mDNS
I've restricted the mDNS name to [^\.]*remise  
This ensures that the DNS can be found by lookup, even when using an app or desktop version instead of web.
For some reason mDNS will never be supported by "older" android versions...
https://issuetracker.google.com/issues/140786115?pli=1  
Listing mDNS services with linux: avahi-browse -a

## IRAM
Be careful when using the IRAM attribute! Even harmless looking things like constexpr data will cause an exception when accessed inside a function placed in IRAM.

## Performance
It's a good idea to separate Espressif system tasks and application tasks. The convention seems to be that Espressifs own stuff runs on core0 whereas the app runs on core1. Tasks with priorities >=19 on core1 can practically no longer be interrupted. ESP-IDF things which need to be pinned to specific core:
- LWIP
- TCPIP
- mDNS
- HTTP servers

FreeRTOS stats can be enabled in with menuconfig:
(Top) → Component config → FreeRTOS → Kernel -> config
vTaskGetRunTimeStats

If that's not sufficient there's also this: https://github.com/espressif/esp-idf/tree/master/examples/system/freertos/real_time_stats

## NVS
According to Espressif its smarter to store many small values in NVS than one large binary blob. So it might be a good idea to store e.g. loco data individually?
It is safe to call nvs_set_* functions even if the value to be stored hasn't actually changed. The underlying ESP-IDF code performs a "sanity check" and only writes values which really differs, see the nvs_storage.cpp source for details:
https://github.com/espressif/esp-idf/blob/c77b5752ef9e4e40c2e819d511b1dd7b83be2996/components/nvs_flash/src/nvs_storage.cpp#L305
```cpp
// Do a sanity check that the item in question is actually being modified.
// If it isn't, it is cheaper to purposefully not write out new data.
// since it may invoke an erasure of flash.
```

## WebSocket
https://esp32.com/viewtopic.php?t=31224  