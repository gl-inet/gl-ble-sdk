# GL-BLE-SDK     bleScanner

Demo of scanning the surrounding ble broadcast and output it in JSON format.

This is the demo for user to use gl-ble-api to scanning ble broadcast and get results.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> {M} libglble.......................... GL inet BLE driver library
	-> <M> gl-bleScanner........................................ GL inet BLE Scanner
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

### install

```shell
opkg install libglble_1.0.0_mipsel_24kc.ipk 
opkg install gl-bleScanner_1.0.0_mipsel_24kc.ipk 
```

### Using

```shell
bleScanner [phys:1] [interval:1] [window:1] [type:1] [mode:1]
	-> phys: The PHY on which the advertising packets are transmitted on. Now only support LE 1M PHY.
				1: LE 1M PHY, 4: LE Coded PHY.
	-> interval: Scan interval. Time = Value x 0.625 ms. Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
	-> window: Scan window. Time = Value x 0.625 ms. Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
	-> type: Scan type.
				0: Passive scanning, 1: Active scanning.
	-> mode: Bluetooth discovery Mode.
				0: Discover only limited discoverable devices
				1: Discover limited and generic discoverable devices
				2: Discover all devices
```

If you don't need to customize the parameters, you can use the default parameters.

```shell
root@GL-MT300N-V2:~# bleScanner 
{ "type": "scan_result", "mac": "d8:96:e0:8b:7d:f2", "address_type": 0, "rssi": -60, "packet_type": 0, "bonding": 255, "data": "0201061aff4c0002151732050807568877293527446341505900014ad0c5" }
{ "type": "scan_result", "mac": "a4:c1:38:14:f5:16", "address_type": 0, "rssi": -47, "packet_type": 0, "bonding": 255, "data": "05095432303102010614ff55aa0101a4c13814f51601070aa21501640001" }
{ "type": "scan_result", "mac": "00:e0:4c:b0:f8:b8", "address_type": 0, "rssi": -77, "packet_type": 0, "bonding": 255, "data": "02010a030366660319d00709ffee03b8f8b04ce000" }
{ "type": "scan_result", "mac": "98:f4:ab:0a:40:b2", "address_type": 0, "rssi": -65, "packet_type": 0, "bonding": 255, "data": "0201060709474c2d533130020a030503ee00ff00051206001000" }
{ "type": "scan_result", "mac": "ac:23:3f:5e:03:1d", "address_type": 0, "rssi": -63, "packet_type": 0, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782527114cb9c5" }
{ "type": "scan_result", "mac": "47:ca:9a:3b:aa:04", "address_type": 1, "rssi": -70, "packet_type": 0, "bonding": 255, "data": "02011a020a080cff4c0010072f1ffde7906a18" }
{ "type": "scan_result", "mac": "62:c7:24:14:03:06", "address_type": 1, "rssi": -77, "packet_type": 0, "bonding": 255, "data": "02011a020a070bff4c0010062c1a088dcadf" }
{ "type": "scan_result", "mac": "ac:23:3f:5e:6a:f4", "address_type": 0, "rssi": -73, "packet_type": 0, "bonding": 255, "data": "0201061aff4c000215fda50693a4e24fb1afcfc6eb0764782527114cb9c5" }
```

You can use **CTRL+C** or **CTRL+\\** to terminate the application.

