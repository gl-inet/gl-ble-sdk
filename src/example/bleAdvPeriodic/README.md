# GL-BLE-SDK     bleAdvPeriodic

Demonstrate how to Periodic broadcast

This is a demonstration of the user using gl-ble-api to send out a Periodic broadcast.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleAdvPeriodic....................... GL inet BLE Periodic Advertising 
```



## Using the example

You can use SCP or other means to upload example ipk to the device.



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleAdvPeriodic_2.0.0_mipsel_24kc.ipk 
```



### Using

**note**

> Bluetooth 5.0 introduces 40 RF channels are defined in the 2.4GHz ISM band. These RF channels are allocated into three LE physical channels: advertising, periodic, and data. These RF channels are divided into 3 RF channels(37,38,39), known as the "**primary advertising channel**", used for initial advertising and all legacy advertising activities, and 37 RF channels(0~36), known as the "**secondary advertising channel**", used for the majority of the communications involved. The periodic physical channel uses the same RF channels as the secondary advertising channel over the advertising physical channel.
>
> The mandatory symbol rate is 1 megasymbol per second (Msym/s), where 1 symbol represents 1 bit therefore supporting a bit rate of 1 megabit per second (Mb/s), which is referred to as the **LE 1M PHY**. The 1 Msym/s symbol rate may optionally support error correction coding, which is referred to as the **LE Coded PHY**. This may use either of two coding schemes: S=2, where 2 symbols represent 1 bit therefore supporting a bit rate of 500 kb/s, and S=8, where 8 symbols represent 1 bit therefore supporting a bit rate of 125 kb/s. An optional symbol rate of 2 Msym/s may be supported, with a bit rate of 2 Mb/s, which is referred to as the LE 2M PHY. The 2 Msym/s symbol rate supports uncoded data only. LE 1M and LE 2M are collectively referred to as the **LE Uncoded PHYs**.
>
> 2M PHY ensures **higher data throughput**. A new long range PHY (LE Coded PHY) with 125kbps and 500kbps coding which **gives range gains of 1.5-2x with improved sensitivity of 4 to 6dB**. The LE Coded PHY uses 1Mbit PHY but payload is coded at 125kbps or 500kbps. It also adds Forward Error Correction and Pattern Mapper.

```shell
bleAdvPeriodic [json_file_path]
	-> json_file_path : The path of configuration parameter json file.
```

**json file parameter(“-” means you must set this parameter, "/" means optional parameter.)**

|      | key               | value_type | descriptiion                                                 |
| ---- | ----------------- | ---------- | ------------------------------------------------------------ |
| -    | primary_phy       | int        | The PHY on which the advertising packets are transmitted on the primary advertising channel.<br/>(1M PHY must be used when Legacy Advertising, Legacy Advertising ignore this parameter)<br/>1: LE 1M PHY        4: LE Coded PHY |
| -    | secondary_phy     | int        | The PHY on which the advertising packets are transmitted on the secondary advertising channel.<br/>(1M PHY must be used when Legacy Advertising, Legacy Advertising ignore this parameter)<br/>1: LE 1M PHY        2: LE 2M PHY        4: LE Coded PHY |
| -    | interval_min      | int        | Minimum periodic advertising interval. Value in units of 1.25 ms<br/>Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s |
| -    | interval_max      | int        | Maximum periodic advertising interval. Value in units of 1.25 ms<br/>Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s |
| /    | periodic_adv_data | string     | Customized non-targeted advertising data. Must be hexadecimal ASCII. Like “020106”.<br/>The maximum size is 1024 bytes. |

The Periodic advertising interval you set should be smaller than synchronize timeout. If not, it will breaks the established synchronization. 



#### demonstrate 

**note**

This example will detect whether the ble module firmware version is 4_2_0. If it is not, it will upgrade to the corresponding firmware version. However, the previous devices does not support this automatic upgrade method, which needs to be upgraded in other ways.

```shell
root@OpenWrt:~# bleAdvPeriodic ./periodic_adv.json 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



- customize parameters 

```shell
#1. Create the json file and write the parameter
root@OpenWrt:~# vim periodic_adv.json 
```

```shell
#2. The template of json file
{
        "primary_phy": 1,
        "secondary_phy": 1,
        "interval_min": 80,
        "interval_max": 160,
        "periodic_adv_data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344"
}
```

```shell
#3. Run the example
root@OpenWrt:~# bleAdvPeriodic ./periodic_adv.json 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:f1:35:1a
```

You can use example [bleSynchronize](../bleSynchronize/README.md) to Synchronize it. 

```shell
root@OpenWrt:~# bleSynchronize 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "periodic_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "adv_sid": 0, "periodic_interval": 120 }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -11, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
```

- default parameters

The default parameters is [primary_phy:1] [secondary_phy:1] [interval_min:80] [interval_max:160] [data:NULL].

```shell
root@OpenWrt:~# bleAdvPeriodic 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
Broadcasting data, the mac of the current device is > 94:de:b8:f1:35:1a
```

You can use example [bleSynchronize](../bleSynchronize/README.md) to Synchronize it. 

```shell
root@OpenWrt:~# bleSynchronize 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "periodic_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "adv_sid": 0, "periodic_interval": 120 }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -11, "data": "" }
```



You can use **CTRL+C** or **CTRL+\\** to terminate the application.

