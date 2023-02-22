# GL-BLE-SDK     bleClient

This example is used to connect to the BLE server and send data (it needs to be used with [bleService](../bleService/README.md))

This is a demonstration of the user using gl-ble-api to send out a ble broadcast and get the result.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleClient.......................................... GL inet BLE Client
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleClient_2.0.0_mipsel_24kc.ipk 
```



### Using

**note**

This example will detect whether the ble module firmware version is 4_2_0. If it is not, it will upgrade to the corresponding firmware version. However, the previous devices does not support this automatic upgrade method, which needs to be upgraded in other ways.

```shell
root@OpenWrt:~# bleClient 94:de:b8:f1:35:1a 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



**format**

```shell
bleClient [address:"88:88:88:88:88:88"]
	-> address: The MAC of the device to be connected (lower-case)
```

```shell
root@OpenWrt:~# bleClient 94:de:b8:f1:35:1a
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "connect_open", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "connect_role": 1, "bonding": 255, "advertiser": 255 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "94:de:b8:f1:35:1a", "interval": 22, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 27 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "94:de:b8:f1:35:1a", "interval": 22, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 251 }
{ "service_list": [ { "service_handle": 65544, "service_uuid": "1801" }, { "service_handle": 589837, "service_uuid": "1800" }, { "service_handle": 983039, "service_uuid": "180a" } ] }
{ "characteristic_list": [ { "characteristic_handle": 3, "properties": 32, "characteristic_uuid": "2a05" }, { "characteristic_handle": 6, "properties": 2, "characteristic_uuid": "2b2a" }, { "characteristic_handle": 8, "properties": 10, "characteristic_uuid": "2b29" } ] }
{ "characteristic_list": [ { "characteristic_handle": 11, "properties": 10, "characteristic_uuid": "2a00" }, { "characteristic_handle": 13, "properties": 2, "characteristic_uuid": "2a01" } ] }
{ "characteristic_list": [ { "characteristic_handle": 16, "properties": 2, "characteristic_uuid": "2a29" }, { "characteristic_handle": 18, "properties": 2, "characteristic_uuid": "2a23" } ] }
Data from the server : Hello Client .
```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.