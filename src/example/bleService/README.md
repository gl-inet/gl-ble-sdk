# GL-BLE-SDK     bleService

This example is used to initiate a connectable BLE server (need to cooperate with [bleClient](../bleClient/README.md))

This is a demonstration of the user using gl-ble-api to start the ble broadcast server and get the results.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleService........................................ GL inet BLE Service
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleService_2.0.0_mipsel_24kc.ipk 
```



### Using

**note**

This example will detect whether the ble module firmware version is 4_2_0. If it is not, it will upgrade to the corresponding firmware version. However, the previous devices does not support this automatic upgrade method, which needs to be upgraded in other ways.

```shell
root@OpenWrt:~# bleService 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



**format**

```shell
bleService
```

There are no configured parameters for this example.



**Before being connected**

```shell
root@OpenWrt:~# bleService 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Waiting to connect, the mac of the current device is > 94:de:b8:f1:35:1a
```

**After being connected**

```shell
root@OpenWrt:~# bleService 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Waiting to connect, the mac of the current device is > 94:de:b8:f1:35:1a
GAP_CB_MSG >> { "type": "connect_open", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "connect_role": 0, "bonding": 255, "advertiser": 0 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "94:de:b8:ee:4a:70", "interval": 22, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 27 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "94:de:b8:ee:4a:70", "interval": 22, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 251 }
Data from the client : Hello server .
```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.

