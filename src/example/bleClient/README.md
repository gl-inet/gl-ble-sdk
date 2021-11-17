# GL-BLE-SDK     bleClient

This example is used to connect to the BLE server and send data (it needs to be used with bleService)

This is a demonstration of the user using gl-ble-api to send out a ble broadcast and get the result.

## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> {M} libglble.......................... GL inet BLE driver library
	-> <M> gl-bleClient........................................ GL inet BLE Client
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

### install

```shell
opkg install libglble_1.0.0_mipsel_24kc.ipk 
opkg install gl-bleClient_1.0.0_mipsel_24kc.ipk 
```

### Using

```shell
bleClient [address:"88:88:88:88:88:88"]
	-> address: The MAC of the device to be connected (lower-case)
```

The MAC here must be filled in, otherwise it will connect to 88:88:88:88:88:88 by default.

```shell
root@GL-S1300:~# bleClient 88:88:88:88:88:88
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 12, "patch": 3, "build": 151, "bootloader": 17367041, "hw": 1, "ble_hash": "de9d9544" }
GAP_CB_MSG >> { "type": "connect_open", "mac": "88:88:88:88:88:88", "address_type": 0, "connect_role": 1, "bonding": 255, "advertiser": 255 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "88:88:88:88:88:88", "interval": 40, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 27 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "88:88:88:88:88:88", "interval": 40, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 251 }
{ "service_list": [ { "service_handle": 65544, "service_uuid": "1801" }, { "service_handle": 589837, "service_uuid": "1800" }, { "service_handle": 917520, "service_uuid": "00ff" }, { "service_handle": 1179647, "service_uuid": "00ee" } ] }
gl_ble_get_char_65544 failed: 0
{ "characteristic_list": [ { "characteristic_handle": 3, "properties": 32, "characteristic_uuid": "2a05" }, { "characteristic_handle": 6, "properties": 2, "characteristic_uuid": "2b2a" }, { "characteristic_handle": 8, "properties": 10, "characteristic_uuid": "2b29" } ] }
gl_ble_get_char_589837 failed: 0
{ "characteristic_list": [ { "characteristic_handle": 11, "properties": 10, "characteristic_uuid": "2a00" }, { "characteristic_handle": 13, "properties": 2, "characteristic_uuid": "2a01" } ] }
gl_ble_get_char_917520 failed: 3
gl_ble_get_char_1179647 failed: 0
{ "characteristic_list": [ { "characteristic_handle": 16, "properties": 12, "characteristic_uuid": "ff01" }, { "characteristic_handle": 19, "properties": 18, "characteristic_uuid": "ee01" } ] }
Data from the server : Hello Client .

```

You can use **CTRL+C** or **CTRL+\\** to terminate the application.