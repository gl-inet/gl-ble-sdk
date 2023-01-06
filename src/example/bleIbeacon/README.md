# GL-BLE-SDK     bleIBeacon

This Demo is used to send or receive ibeacon packets. There are two run modes, foreground mode and background mode.

Foreground mode: 

It interacts through the command line. You can use it to send or receive ibeacon packet simply.

Background mode:

If you want to add ibeacon functionality to your code, here is a ubus interface to facilitate your implementation.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleIbeacon........................................ GL inet BLE Ibeacon
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mips_24kc.ipk 
opkg install gl-bleIbeacon_2.0.0_mips_24kc.ipk 
```



### Using

#### Foreground mode

##### send

```shell
bleIbeacon send [rsp_type] [proximity_uuid] [major] [minor] [measured_power] [measured_power] [tx_power]

	-> rsp_type: Whether to send a scan response packet.
				rsp_close: no send scan response packet
				rsp_open: send scan response packet		
	-> proximity_uuid: 16 bytes. Application developers should define a UUID specific to their app and deployment use case.
					   Must not be set to all 0s.		   
	-> major: 2 bytes. Further specifies a specific iBeacon and use case. For example, this could define a sub-region within a 				  larger region defined by the UUID.
	-> minor: 2 bytes. Allows further subdivision of region or use case, specified by the application developer.
	-> measured_power: 1 bytes. Apple devices use the Measured Power value of a Proximity Beacon to help with ranging accuracy.
	-> tx_power: This item is optional, leave it blank and leave it unset. Set the global power level（0.1 dBm steps）.
```

###### demonstrate 

```shell
#unset tx_power
root@OpenWrt:~# bleIbeacon send rsp_close 11112222333344445555777766668888 0001 0002 50
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }

foreground mode
data: 0201061aff004c1502111122223333444455557777666688880001000250
Broadcasting ibeacon data, the mac of the current device is > 94:de:b8:f1:35:1a
```

```shell
#set tx_power
root@OpenWrt:~# bleIbeacon send rsp_close 11112222333344445555777766668888 0001 0002 50 50 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }

foreground mode
data: 0201061aff004c1502111122223333444455557777666688880001000250
{ "code": 0, "current_power": 51 }
Broadcasting ibeacon data, the mac of the current device is > 94:de:b8:f1:35:1a
```

##### recieve

```shell
bleIbeacon recieve [scan_type:passive_scan]
	-> rsp_type: Scan type.
				passive_scan: Passive scanning
				active_scan: Active scanning
```

###### demonstrate 

```shell
root@OpenWrt:~# bleIbeacon recieve passive_scan
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }

foreground mode
passive scan
{ "type": "Beacon", "mac": "ac:23:3f:5e:03:1d", "address_type": 0, "rssi": -69, "packet_type": 0, "bonding": 255, "proximity_uuid": "fda50693a4e24fb1afcfc6eb07647825", "major": "2711", "minor": "2711", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:f2", "address_type": 0, "rssi": -52, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7e:02", "address_type": 0, "rssi": -55, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "d8:96:e0:8b:7d:e9", "address_type": 0, "rssi": -79, "packet_type": 0, "bonding": 255, "proximity_uuid": "17320508075688772935274463415059", "major": "0001", "minor": "0001", "measured_power": -59 }
{ "type": "Beacon", "mac": "ac:23:3f:5e:03:1d", "address_type": 0, "rssi": -73, "packet_type": 0, "bonding": 255, "proximity_uuid": "fda50693a4e24fb1afcfc6eb07647825", "major": "2711", "minor": "2711", "measured_power": -59 }
```





#### Background mode

Firstly, Run the bleIbeacon program without parameters.

```shell
root@OpenWrt:~# bleIbeacon
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }

background mode

```

And then, you can check the ubus invoke obj(ibeacon) like this.

```shell
root@OpenWrt:~# ubus -v list ibeacon
'ibeacon' @5d6b5db6
        "recieve":{"scan_type":"String","stop_scan":"String"}
        "send":{"rsp_type":"String","uuid":"String","major":"String","minor":"String","measured_power":"String","tx_power":"String","stop_send":"String"}
```



##### send

```
"send":{"rsp_type":"String","uuid":"String","major":"String","minor":"String","measured_power":"String","tx_power":"String","stop_send":"String"}

	-> rsp_type: Whether to send a scan response packet.
				rsp_close: no send scan response packet
				rsp_open: send scan response packet		
	-> uuid: 16 bytes. Application developers should define a UUID specific to their app and deployment use case.
					   Must not be set to all 0s.		   
	-> major: 2 bytes. Further specifies a specific iBeacon and use case. For example, this could define a sub-region within a 				  larger region defined by the UUID.
	-> minor: 2 bytes. Allows further subdivision of region or use case, specified by the application developer.
	-> measured_power: 1 bytes. Apple devices use the Measured Power value of a Proximity Beacon to help with ranging accuracy.
	-> tx_power: This item is optional, leave it blank and leave it unset. Set the global power level（0.1 dBm steps）.
	-> stop_send: It is use for stop to send ibeacon packet.
    			  true: stop to send ibeacon packet
```

###### demonstrate 

```shell
#unset tx_power
root@OpenWrt:~# ubus call ibeacon send '{"rsp_type":"rsp_close","uuid":"12345678123456781234567812345678","major":"0003","minor":"0005","measured_power":"50"}'
{
        "code": 0
}
```

```shell
#set tx_power
root@OpenWrt:~# ubus call ibeacon send '{"rsp_type":"rsp_close","uuid":"12345678123456781234567812345678","major":"0003","minor":"0005","measured_power":"50","tx_power":"50"}'
{
        "current_power": 51,
        "code": 0
}
```

```shell
#stop send
root@OpenWrt:~#  ubus call ibeacon send '{"stop_send":"true"}'
{
        "code": 0
}
```



##### recieve

```
"recieve":{"scan_type":"String","stop_scan":"String"}

	-> rsp_type: Scan type.
				passive_scan: Passive scanning
				active_scan: Active scanning
	-> stop_scan: It is use for stop to recieve ibeacon packet.
				  true: stop to recieve ibeacon packet
```

###### demonstrate 

```shell
# Set and start scan
root@OpenWrt:~# ubus call ibeacon recieve '{"scan_type": "passive_scan"}'
{
        "code": 0
}

# And then Receive reported ibeacon data through event ibeacon_data
root@OpenWrt:~# ubus listen ibeacon_data
{ "ibeacon_data": {"type":"Beacon","mac":"d8:96:e0:8b:7d:f2","address_type":0,"rssi":-64,"event_flags":3,"bonding":255,"proximity_uuid":"17320508075688772935274463415059","major":"0001","minor":"4ad0","measured_power":-59} }
{ "ibeacon_data": {"type":"Beacon","mac":"72:64:21:10:00:d8","address_type":1,"rssi":-66,"event_flags":3,"bonding":255,"proximity_uuid":"fda50693a4e24fb1afcfc6eb07647825","major":"0001","minor":"0002","measured_power":-40} }
{ "ibeacon_data": {"type":"Beacon","mac":"d8:96:e0:8b:76:fd","address_type":0,"rssi":-64,"event_flags":3,"bonding":255,"proximity_uuid":"17320508075688772935274463415059","major":"0001","minor":"43db","measured_power":-59} }
```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.

