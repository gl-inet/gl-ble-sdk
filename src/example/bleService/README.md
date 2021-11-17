# GL-BLE-SDK     bleService

This example is used to initiate a connectable BLE server (need to cooperate with bleClient)

This is a demonstration of the user using gl-ble-api to start the ble broadcast server and get the results.

## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> {M} libglble.......................... GL inet BLE driver library
	-> <M> gl-bleService........................................ GL inet BLE Service
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

### install

```shell
opkg install libglble_1.0.0_mipsel_24kc.ipk 
opkg install gl-bleService_1.0.0_mipsel_24kc.ipk 
```

### Using

```shell
bleService
```

There are no configured parameters for this example.

**Before being connected**

```shell
root@GL-MT300N-V2:~# bleService 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 12, "patch": 3, "build": 151, "bootloader": 17367041, "hw": 1, "ble_hash": "de9d9544" }
Waiting to connect, the mac of the current device is > 88:88:88:88:88:88
```

**After being connected**

```shell
root@GL-MT300N-V2:~# bleService 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 12, "patch": 3, "build": 151, "bootloader": 17367041, "hw": 1, "ble_hash": "de9d9544" }
Waiting to connect, the mac of the current device is > 88:88:88:88:88:88
GAP_CB_MSG >> { "type": "connect_open", "mac": "80:4b:50:51:06:bb", "address_type": 0, "connect_role": 0, "bonding": 255, "advertiser": 0 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "80:4b:50:51:06:bb", "interval": 40, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 27 }
GAP_CB_MSG >> { "type": "connect_update", "mac": "80:4b:50:51:06:bb", "interval": 40, "latency": 0, "timeout": 100, "security_mode": 0, "txsize": 251 }
Data from the client : Hello server .
```

You can use **CTRL+C** or **CTRL+\\** to terminate the application.

