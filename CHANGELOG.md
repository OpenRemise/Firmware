# Changelog

##
- Update to Frontend ?
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
- Update to Frontend 0.0.7
- Add RSSI to `/sys/` GET request ([#27](https://github.com/OpenRemise/Firmware/issues/27))

## 0.0.5
- Update to Frontend 0.0.6
- Add `LAN_X_SET_LOCO_FUNCTION_GROUP`
- Allow changing loco address ([#25](https://github.com/OpenRemise/Firmware/issues/25))

## 0.0.4
- Add alternative SSID/password ([#16](https://github.com/OpenRemise/Firmware/issues/16))
- Update to Frontend 0.0.3
- Bugfix DECUP_EIN timeout locks firmware in `Suspend` state ([#21](https://github.com/OpenRemise/Firmware/issues/21))

## 0.0.3
- Update to Frontend 0.0.2

## 0.0.2
- Bugfix ZCS does not establish a Z21 protocol connection ([#17](https://github.com/OpenRemise/Firmware/issues/17))

## 0.0.1
- Bugfix MX decoders don't support ZUSI exit ([#11](https://github.com/OpenRemise/Firmware/issues/11))

## 0.0.0
- Test release for [Flasher](https://github.com/OpenRemise/Flasher)