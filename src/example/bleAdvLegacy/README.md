# GL-BLE-SDK     bleAdvLegacy

Demonstrate how to Legacy broadcast 

This is a demonstration of the user using gl-ble-api to send out a Legacy  broadcast.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleAdvLegacy........................... GL inet BLE Legacy Advertising
    
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleAdvLegacy_2.0.0_mipsel_24kc.ipk 
```



### Using

```shell
bleAdvLegacy [json_file_path]
	-> json_file_path : The path of configuration parameter json file.
```

**json file parameter(“-” means you must set this parameter, "/" means optional parameter.)**

|      | key             | value_type | descriptiion                                                 |
| ---- | --------------- | ---------- | ------------------------------------------------------------ |
| -    | interval_min    | int        | Minimum Legacy advertising interval.Value in units of 0.625 ms<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s |
| -    | interval_max    | int        | Maximum Legacy advertising interval. Value in units of 0.625 ms<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s |
| -    | discover        | int        | Define the discoverable mode.<br/>        0: Not discoverable<br/>        1: Discoverable by both limited and general discovery procedures<br/>        2: Discoverable by the general discovery procedure<br/>        3: Send legacy advertising and/or scan response data defined by the user.<br/>            The limited/general discoverable flags are defined by the user. |
| -    | connect         | int        | Define the Legacy Advertising connectable mode.<br/>       0: Undirected non-connectable and non-scannable legacy advertising <br/>       2: Undirected connectable and scannable legacy advertising <br/>       3: Undirected scannable and non-connectable legacy advertising |
| /    | legacy_adv_data | string     | Customized non-targeted advertising data. Must be hexadecimal ASCII. Like “020106”.<br/>The maximum size is 31 bytes. |
| /    | legacy_rsp_data | string     | Customized non-targeted scan respone data. Must be hexadecimal ASCII. Like “020106”.<br/>The maximum size is 31 bytes. |



### demonstrate 

- customize parameters

```shell
#1. Create the json file and write the parameter
root@OpenWrt:~# vim legacy_adv.json 
```

```shell
#2. The template of json file
{
        "interval_min": 160,
        "interval_max": 320,
        "discover": 3,
        "connect": 3,
        "legacy_adv_data": "0201060709474c5f424c45",
        "legacy_rsp_data": "05ff74657374"
}
```

```shell
#3. Run the example
root@OpenWrt:~# bleAdvLegacy ./legacy_adv.json 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:f1:35:1a
```

You can use example [bleScanner](../bleScanner/README.md) to specify address scan it. 

```shell
root@OpenWrt:~# bleScanner 1 16 16  1 2 94:de:b8:f1:35:1a
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "event_flags": 2, "bonding": 255, "data": "0201060709474c5f424c45" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -12, "event_flags": 2, "bonding": 255, "data": "0201060709474c5f424c45" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 8, "bonding": 255, "data": "05ff74657374" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "event_flags": 2, "bonding": 255, "data": "0201060709474c5f424c45" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -9, "event_flags": 2, "bonding": 255, "data": "0201060709474c5f424c45" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 2, "bonding": 255, "data": "0201060709474c5f424c45" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -9, "event_flags": 8, "bonding": 255, "data": "05ff74657374" }
```

- default parameters

The default parameters is [interval_min:160] [interval_max:320]  [discover:2] [connect:2] [data0:NULL] [data1:NULL].

```shell
root@OpenWrt:~# bleAdvLegacy 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:f1:35:1a
```

You can use example [bleScanner](../bleScanner/README.md) to specify address scan it. 

```shell
root@OpenWrt:~# bleScanner 94:de:b8:f1:35:1a
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "event_flags": 3, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 3, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 3, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 3, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -11, "event_flags": 3, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }

```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.
