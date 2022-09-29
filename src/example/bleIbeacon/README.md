# GL-BLE-SDK     bleIBeacon

This Demois used to send and receive ibeacon packets. There are two run modes, foreground mode and background mode.

Foreground mode interacts through the command line.

Background mode interacts through the ubusd.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> {M} libglble.......................... GL inet BLE driver library
	-> <M> gl-bleIbeacon........................................ GL inet BLE Ibeacon
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

### install

```shell
opkg install libglble_1.4.0_mips_24kc.ipk 
opkg install gl-bleIbeacon_1.4.0_mips_24kc.ipk 
```

### Using

#### Foreground mode

##### send

```shell
bleIbeacon send [rsp_type:rsp_close] [proximity_uuid:1] [major:1] [minor:1] [measured_power:1] [measured_power:50] [tx_power:50]
	-> rsp_type: Whether to send a scan response package.
				rsp_close: no send scan response package
				rsp_open: send scan response package		
	-> proximity_uuid: 16 bytes. Application developers should define a UUID specific to their app and deployment use case.
					   Must not be set to all 0s.		   
	-> major: 2 bytes. Further specifies a specific iBeacon and use case. For example, this could define a sub-region within a 				  larger region defined by the UUID.
	-> minor: 2 bytes. Allows further subdivision of region or use case, specified by the application developer.
	-> measured_power: 1 bytes. Apple devices use the Measured Power value of a Proximity Beacon to help with ranging accuracy.
	-> tx_power: This item is optional, leave it blank and leave it unset. Set the global power level（0.1 dBm steps）.
```

```shell
#unset tx_power
root@OpenWrt:~# bleIbeacon send rsp_close 11112222333344445555777766668888 0001 0002 50
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }

foreground mode
data: 0201061aff004c1502111122223333444455557777666688880001000250
Broadcasting ibeacon data, the mac of the current device is > 94:DE:B8:F1:35:1A


#set tx_power
root@OpenWrt:~# bleIbeacon send rsp_close 11112222333344445555777766668888 0001 0002 50 50 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }

foreground mode
data: 0201061aff004c1502111122223333444455557777666688880001000250
{ "code": 0, "current_power": 51 }
Broadcasting ibeacon data, the mac of the current device is > 94:DE:B8:F1:35:1A
```

##### recieve

```shell
bleIbeacon recieve [scan_type:passive_scan]
	-> rsp_type: Scan type.
				passive_scan: Passive scanning
				active_scan: Active scanning
```

```shell
root@OpenWrt:~# bleIbeacon recieve passive_scan
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }

foreground mode
passive scan
{ "type": "Beacon", "mac": "ac:23:3f:5e:03:1d", "address_type": 0, "rssi": -69, "packet_type": 0, "bonding": 255, "proximity_uuid": "fda50693a4e24fb1afcfc6eb07647825", "major": "2711", "minor": "2711", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f2", "address_type": 0, "rssi": -52, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7e:02", "address_type": 0, "rssi": -55, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:e9", "address_type": 0, "rssi": -79, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "ac:23:3f:5e:03:1d", "address_type": 0, "rssi": -73, "packet_type": 0, "bonding": 255, "proximity_uuid": "fda50693a4e24fb1afcfc6eb07647825", "major": "2711", "minor": "2711", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f7", "address_type": 0, "rssi": -74, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f7", "address_type": 0, "rssi": -74, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "72:64:21:10:00:dc", "address_type": 1, "rssi": -72, "packet_type": 0, "bonding": 255, "proximity_uuid": "11112222333344445555666677778888", "major": "0003", "minor": "0003", "measured_power": -89 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f2", "address_type": 0, "rssi": -52, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f7", "address_type": 0, "rssi": -74, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
```

#### Background mode

```shell
root@OpenWrt:~# bleIbeacon
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }

background mode

```

```shell
root@OpenWrt:~# ubus -v list ibeacon
'ibeacon' @45a15f71
        "recieve":{"scan_type":"String"}
        "send":{"rsp_type":"String","uuid":"String","major":"String","minor":"String","measured_power":"String","tx_power":"String"}
```

##### send

```shell
#unset tx_power
root@OpenWrt:~# ubus call ibeacon send '{"rsp_type":"rsp_close","uuid":"12345678123456781234567812345678","major":"0003","minor":"0005","measured_power":"50"}'
{
        "code": 0
}


#set tx_power
root@OpenWrt:~# ubus call ibeacon send '{"rsp_type":"rsp_close","uuid":"12345678123456781234567812345678","major":"0003","minor":"0005","measured_power":"50","tx_power":"50"}'
{
        "current_power": 51,
        "code": 0
}
```

##### recieve

```shell
root@OpenWrt:~# ubus call ibeacon recieve '{"scan_type": "passive_scan"}'
{
        "type": "Beacon",
        "mac": "d8:96:e0:8b:7d:f2",
        "address_type": 0,
        "rssi": -54,
        "packet_type": 0,
        "bonding": 255,
        "proximity_uuid": "17320508075688772935274463415059",
        "major": "0001",
        "minor": "0001",
        "measured_power": -59,
        "code": 0
}
```

You can use **CTRL+C** or **CTRL+\\** to terminate the application.

