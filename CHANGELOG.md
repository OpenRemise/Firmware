# Changelog

## 0.5.2
- Add DNS server in AP mode ([#100](https://github.com/OpenRemise/Firmware/issues/100))
- Add CSS to captive portal ([#103](https://github.com/OpenRemise/Firmware/pull/103))
- Disable ZIMO MX decoder updates and uploads ([#105](https://github.com/OpenRemise/Firmware/issues/105))

## 0.5.1
- Increase ADC sample rate to 80kHz
- Bugfix service mode packets are interleaved ([#97](https://github.com/OpenRemise/Firmware/issues/97))
- Bugfix increase minimum number of service mode programming packets to 5

## 0.5.0
- Add support for basic accessory decoders
- Add `/dcc/turnouts/` endpoint
- Add `dcc_accy_swtime` setting
- Add `dcc_accy_pc` setting

## 0.4.1
- Add `/dcc/` endpoint ([#84](https://github.com/OpenRemise/Firmware/pull/84))
- Implement `z21::server::intf::System::systemState` ([#90](https://github.com/OpenRemise/Firmware/issues/90))
- Bugfix task names inside `intf::usb::any_service_task_active` ([#86](https://github.com/OpenRemise/Firmware/pull/86))
- Bugfix `intf::http::Endpoints` key comparison uses prefix-based matching ([#87](https://github.com/OpenRemise/Firmware/pull/87))

## 0.4.0
- Add `http_exit_msg` setting ([#83](https://github.com/OpenRemise/Firmware/pull/83))
- Workaround for Windows 11 USB issues ([#83](https://github.com/OpenRemise/Firmware/pull/83))

## 0.3.3
- Bugfix POM read with CV value 0 ([#81](https://github.com/OpenRemise/Firmware/issues/81))

## 0.3.2
- Bugfix some MX decoders require 2nd stop bit during DECUP update ([#68](https://github.com/OpenRemise/Firmware/issues/68))

## 0.3.1
- Bugfix POM ([#67](https://github.com/OpenRemise/Firmware/issues/67))
- Bugfix don't add DCC address 0 to locos ([#71](https://github.com/OpenRemise/Firmware/issues/71))

## 0.3.0
- Bugfix short circuit detection does not trigger ([#63](https://github.com/OpenRemise/Firmware/issues/63))

## 0.2.1
- Add RailCom data to Loco JSON ([#53](https://github.com/OpenRemise/Firmware/issues/53))
- Bugfix MN180N18 (bootloader 2.10.8) update not working (`ulTaskNotifyTakeIndexed` without time to wait not reliable) ([#52](https://github.com/OpenRemise/Firmware/issues/52))
- Bugfix turn on bug LED during DCC service mode ([#54](https://github.com/OpenRemise/Firmware/issues/54))
- Bugfix BiDi datagram containing ACK discarded ([#58](https://github.com/OpenRemise/Firmware/issues/58))

## 0.2.0
- Support static IP ([#49](https://github.com/OpenRemise/Firmware/issues/49))

## 0.1.0
- Remove current limit during updates setting ([#46](https://github.com/OpenRemise/Firmware/issues/46))

## 0.0.9
- Update to Frontend [0.0.9](https://github.com/OpenRemise/Frontend/releases/tag/v0.0.9)
- LED duty cycle settings ([#42](https://github.com/OpenRemise/Firmware/issues/42))
- Current limit during updates setting ([#46](https://github.com/OpenRemise/Firmware/issues/46))
- Bugfixes WebSocket ([#45](https://github.com/OpenRemise/Firmware/issues/45))
  - Send messages on `WIFI_TASK_CORE_ID` using `httpd_queue_work`
  - Set `CONFIG_HTTPD_QUEUE_WORK_BLOCKING=y` because otherwise work in the queue got lost
  - Set `CONFIG_LWIP_TCPIP_RECVMBOX_SIZE=64` to allow for bigger batch sizes when receiving WebSocket messages
  - Set `CONFIG_LWIP_TCP_RECVMBOX_SIZE=64` (see above)
  - Set `CONFIG_LWIP_UDP_RECVMBOX_SIZE=64` (see above)

## 0.0.8
- Bugfix WebSocket frame parsing errors ([#9](https://github.com/OpenRemise/Firmware/issues/9))
- Bugfix mDNS does not support subdomains ([#39](https://github.com/OpenRemise/Firmware/issues/39))

## 0.0.7
- Add short loco addresses and repeat higher functions DCC flags ([#29](https://github.com/OpenRemise/Firmware/issues/29))
- Add `_z21` mDNS service ([#32](https://github.com/OpenRemise/Firmware/issues/32))
- Bugfix wrong loco address in NVS when changing address ([#31](https://github.com/OpenRemise/Firmware/issues/31))
- Bugfix increase brown-out level to 2.98V ([#30](https://github.com/OpenRemise/Firmware/issues/30))

## 0.0.6
- Update to Frontend [0.0.7](https://github.com/OpenRemise/Frontend/releases/tag/v0.0.7)
- Add RSSI to `/sys/` GET request ([#27](https://github.com/OpenRemise/Firmware/issues/27))

## 0.0.5
- Update to Frontend [0.0.6](https://github.com/OpenRemise/Frontend/releases/tag/v0.0.6)
- Add `LAN_X_SET_LOCO_FUNCTION_GROUP`
- Allow changing loco address ([#25](https://github.com/OpenRemise/Firmware/issues/25))

## 0.0.4
- Update to Frontend [0.0.3](https://github.com/OpenRemise/Frontend/releases/tag/v0.0.3)
- Add alternative SSID/password ([#16](https://github.com/OpenRemise/Firmware/issues/16))
- Bugfix DECUP_EIN timeout locks firmware in `Suspend` state ([#21](https://github.com/OpenRemise/Firmware/issues/21))

## 0.0.3
- Update to Frontend [0.0.2](https://github.com/OpenRemise/Frontend/releases/tag/v0.0.2)

## 0.0.2
- Bugfix ZCS does not establish a Z21 protocol connection ([#17](https://github.com/OpenRemise/Firmware/issues/17))

## 0.0.1
- Bugfix MX decoders don't support ZUSI exit ([#11](https://github.com/OpenRemise/Firmware/issues/11))

## 0.0.0
- Test release for [Flasher](https://github.com/OpenRemise/Flasher)