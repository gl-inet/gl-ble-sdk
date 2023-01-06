# GL-BLE-SDK     bleAdvExtended

Demonstrate how to Extended broadcast

This is a demonstration of the user using gl-ble-api to send out a Extended broadcast.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleAdvExtended....................... GL inet BLE Extended Advertising 
```



## Using the examplema

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleAdvExtended_2.0.0_mipsel_24kc.ipk 
```



### Using

**note**

> Bluetooth 5.0 introduces 40 RF channels are defined in the 2.4GHz ISM band. These RF channels are allocated into three LE physical channels: advertising, periodic, and data. These RF channels are divided into 3 RF channels(37,38,39), known as the "**primary advertising channel**", used for initial advertising and all legacy advertising activities, and 37 RF channels(0~36), known as the "**secondary advertising channel**", used for the majority of the communications involved. The periodic physical channel uses the same RF channels as the secondary advertising channel over the advertising physical channel.
>
> The mandatory symbol rate is 1 megasymbol per second (Msym/s), where 1 symbol represents 1 bit therefore supporting a bit rate of 1 megabit per second (Mb/s), which is referred to as the **LE 1M PHY**. The 1 Msym/s symbol rate may optionally support error correction coding, which is referred to as the **LE Coded PHY**. This may use either of two coding schemes: S=2, where 2 symbols represent 1 bit therefore supporting a bit rate of 500 kb/s, and S=8, where 8 symbols represent 1 bit therefore supporting a bit rate of 125 kb/s. An optional symbol rate of 2 Msym/s may be supported, with a bit rate of 2 Mb/s, which is referred to as the LE 2M PHY. The 2 Msym/s symbol rate supports uncoded data only. LE 1M and LE 2M are collectively referred to as the **LE Uncoded PHYs**.
>
> 2M PHY ensures **higher data throughput**. A new long range PHY (LE Coded PHY) with 125kbps and 500kbps coding which **gives range gains of 1.5-2x with improved sensitivity of 4 to 6dB**. The LE Coded PHY uses 1Mbit PHY but payload is coded at 125kbps or 500kbps. It also adds Forward Error Correction and Pattern Mapper.

```shell
bleAdvExtended [json_file_path]
	-> json_file_path : The path of configuration parameter json file.
```

**json file parameter(“-” means you must set this parameter, "/" means optional parameter.)**

|      | key               | value_type | descriptiion                                                 |
| ---- | ----------------- | ---------- | ------------------------------------------------------------ |
| -    | primary_phy       | int        | The PHY on which the advertising packets are transmitted on the primary advertising channel.<br/>(1M PHY must be used when Legacy Advertising, Legacy Advertising ignore this parameter)<br/>1: LE 1M PHY        4: LE Coded PHY(125k, S=8) |
| -    | secondary_phy     | int        | The PHY on which the advertising packets are transmitted on the secondary advertising channel.<br/>(1M PHY must be used when Legacy Advertising, Legacy Advertising ignore this parameter)<br/>1: LE 1M PHY        2: LE 2M PHY        4: LE Coded PHY(125k, S=8) |
| -    | interval_min      | int        | Minimum Extended advertising interval.Value in units of 0.625 ms<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s |
| -    | interval_max      | int        | Maximum Extended advertising interval. Value in units of 0.625 ms<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s |
| -    | discover          | int        | Define the discoverable mode.<br/>        0: Not discoverable,<br/>        1: Discoverable by both limited and general discovery procedures<br/>        2: Discoverable by the general discovery procedure<br/>        3：Send extended advertising data defined by the user.<br/>              The limited/general discoverable flags are defined by the user. |
| -    | connect           | int        | Define the Extended Advertising connectable mode.<br/>        0: Non-connectable and non-scannable extended advertising <br/>        3: Scannable extended advertising <br/>        4: Connectable extended advertising |
| /    | extended_adv_data | string     | Customized non-targeted extended advertising data. Must be hexadecimal ASCII. Like “020106”.<br/>Maximum 1024 bytes of data can be set for unconnectable extended advertising.<br/>Maximum 191 bytes of data can be set for connectable extended advertising. |

When Extended advertising packet is more than 254 bytes and short advertising interval you set, it will cause lose packet problem. Here's recommend advertising interval of no less than 200ms.



### demonstrate 

**note**

Make sure your ble module firmware version is 4_2_0. If not, the example is not working.

```shell
root@OpenWrt:~# bleAdvExtended 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



- **customize parameters** 

```shell
#1. Create the json file and write the parameter
root@OpenWrt:~# vim extended_adv.json
```

```shell
#2. The template of json file
{
        "primary_phy": 1,
        "secondary_phy": 1,
        "interval_min": 480,
        "interval_max": 480,
        "discover": 3,
        "connect": 0,
        "extended_adv_data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012"
}
```

```shell
#3. Run the example
root@OpenWrt:~# bleAdvExtended ./extended_adv.json 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:f1:35:1a
```

You can use example [bleScanner](../bleScanner/README.md) to specify address scan it. 

```shell
root@OpenWrt:~# bleScanner 94:de:b8:ee:4a:70
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -11, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -11, "event_flags": 0, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012" }
```

- **default parameters**

The default parameters is [primary_phy:1] [secondary_phy:1] [interval_min:160] [interval_max:320] [discover:2] [connect:4] [data:NULL]

```shell
root@OpenWrt:~# bleAdvExtended 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:ee:4a:70
```

You can use example [bleScanner](../bleScanner/README.md) to specify address scan it. 

```shell
root@OpenWrt:~# bleScanner 94:de:b8:ee:4a:70
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 1, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 1, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 1, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -10, "event_flags": 1, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:ee:4a:70", "address_type": 0, "rssi": -11, "event_flags": 1, "bonding": 255, "data": "0201060f0953696c616273204578616d706c65" }
```





You can use **CTRL+C** or **CTRL+\\** to terminate the application.

