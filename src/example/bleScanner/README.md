# GL-BLE-SDK     bleScanner

Demo of scanning the surrounding ble broadcast and output it in JSON format.

This is the demo for user to use gl-ble-api to scanning ble broadcast and get results.

You can collaborate with [bleAdvertiserLegacy](../bleAdvertiserLegacy/README.md) and [bleAdvertiserExtended](../bleAdvertiserExtended/README.md).



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleScanner........................................ GL inet BLE Scanner
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleScanner_2.0.0_mipsel_24kc.ipk 
```



### Using

```shell
bleScanner [phys] [interval] [window] [type] [mode] [address]

	-> phys: The PHY on which the advertising packets are transmitted on.
              1: LE 1M PHY, 4: LE Coded PHY, 5: Simultaneous LE 1M and Coded PHY alternatively
	-> interval: Scan interval. Time = Value x 0.625 ms. 
				  Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
	-> window: Scan window. Time = Value x 0.625 ms. 
				  Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
	-> type: Scan type.
				0: Passive scanning, 1: Active scanning.
	-> mode: Bluetooth discovery Mode.
				0: Discover only limited discoverable devices
				1: Discover limited and generic discoverable devices
				2: Discover all devices
	-> address: (optional)Scans the broadcast packet for the specified MAC address. Like “11:22:33:44:55:66”.
```



### demonstrate 

**note**

This example will detect whether the ble module firmware version is 4_2_0. If it is not, it will upgrade to the corresponding firmware version. However, the previous devices does not support this automatic upgrade method, which needs to be upgraded in other ways.

```shell
root@OpenWrt:~# bleScanner 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



- customize parameters 

```shell
# Not specify MAC address
root@OpenWrt:~# bleScanner 1 16 16 0 2
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "6e:8c:cc:0f:a7:fa", "address_type": 1, "rssi": -70, "event_flags": 3, 
"bonding": 255, "data": "0201061aff4c000c0e0895232231d3e902f72f3870ce4e1005491c23b634" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "e7:43:07:1e:e8:cb", "address_type": 1, "rssi": -57, "event_flags": 0, "bonding": 255, "data": "07ff4c0012020001" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "72:64:21:10:00:ba", "address_type": 1, "rssi": -63, "event_flags": 3, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782500010002d8" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "6e:be:a6:f7:0a:a7", "address_type": 1, "rssi": -82, "event_flags": 3, "bonding": 255, "data": "02011a020a070bff4c0010061f1978d89a28" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "03:b9:77:cc:9c:47", "address_type": 1, "rssi": -67, "event_flags": 0, "bonding": 255, "data": "1eff060001092002b911fbc40564078c43aba56dc86d7f5107f6f63e0541e2" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "69:df:be:90:44:9c", "address_type": 1, "rssi": -68, "event_flags": 3, "bonding": 255, "data": "02010217ff8f03271110184122343266621526080857010304830b0303aafd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "47:8d:5d:77:bd:66", "address_type": 1, "rssi": -60, "event_flags": 3, "bonding": 255, "data": "02011a020a080bff4c001006281a734eb5da" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "c2:0a:0c:41:21:74", "address_type": 1, "rssi": -79, "event_flags": 0, "bonding": 255, "data": "07ff4c0012020001" }
```

```shell
# Specify MAC address
root@OpenWrt:~# bleScanner 1 160 160 0 2 72:64:21:10:00:ba
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "72:64:21:10:00:ba", "address_type": 1, "rssi": -72, "event_flags": 3, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782500010002d8" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "72:64:21:10:00:ba", "address_type": 1, "rssi": -64, "event_flags": 3, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782500010002d8" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "72:64:21:10:00:ba", "address_type": 1, "rssi": -74, "event_flags": 3, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782500010002d8" }
```

- default parameters

The default parameters is [phys:1] [interval:16] [window:16] [type:0] [mode:2] [address:NULL]

```shell
# Not specify MAC address
root@OpenWrt:~# bleScanner 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "7f:15:94:0d:43:54", "address_type": 1, "rssi": -64, "event_flags": 3, "bonding": 255, "data": "02011a020a0c0aff4c0010050018dca06a" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "e0:9c:ed:25:42:fa", "address_type": 1, "rssi": -75, "event_flags": 0, "bonding": 255, "data": "07ff4c0012020000" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "2e:fb:c6:26:2d:c6", "address_type": 1, "rssi": -80, "event_flags": 0, "bonding": 255, "data": "1eff0600010920020b5f21cacaee774827990d1adfbcb8f04d3827ac123b94" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:f1:23:f4", "address_type": 0, "rssi": -25, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "2e:ae:02:4d:ec:2c", "address_type": 1, "rssi": -60, "event_flags": 0, "bonding": 255, "data": "1eff060001092002f4073ed89f586b7389b2bbe6958ea24135dc9c732a6390" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "67:a6:af:05:f9:a0", "address_type": 1, "rssi": -60, "event_flags": 3, "bonding": 255, "data": "02011a020a070aff4c0010055f1c67f376" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "22:46:6d:9e:00:ca", "address_type": 1, "rssi": -64, "event_flags": 0, "bonding": 255, "data": "02011a0bff4c0009060369c0a82669" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "61:40:61:ae:65:e3", "address_type": 1, "rssi": -75, "event_flags": 3, "bonding": 255, "data": "02011a020a030aff4c0010054b1cfcda09" }
```

```shell
# Specify MAC address
root@OpenWrt:~# bleScanner 94:de:b8:ee:4a:70
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -11, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -11, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.
