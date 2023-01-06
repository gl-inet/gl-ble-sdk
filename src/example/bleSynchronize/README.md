# GL-BLE-SDK     bleSynchronize

Demo of Synchronizing the surrounding ble periodic broadcast and output it in JSON format.

This is the demo for user to use gl-ble-api to Synchronizing  periodic broadcast and get results.

You can collaborate with [bleAdvertiserPeriodic](../bleAdvertiserPeriodic/README.md).



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
	-> <M> libglble...................................... GL inet BLE driver library
	-> <M> gl-bleSynchronize................................ GL inet BLE Synchronize 
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

You can use example [bleAdvertiserPeriodic](../bleAdvertiserPeriodic/README.md) to Advertising a Periodic packet first. 



### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk 
opkg install gl-bleSynchronize_2.0.0_mips_24kc.ipk
```



### Using

```shell
bleSynchronize [skip] [timeout] [address] [address_type] [adv_sid]
	-> skip: The maximum number of periodic advertising packets that can be skipped after a successful receive.
               Range: 0x0000 to 0x01F3.
	-> timeout: The maximum permitted time between successful receives.If this time is exceeded, synchronization is lost. 
				Unit: 10 ms. Range: 0x0A to 0x4000, Time Range: 100 ms to 163.84 s.
	-> address: Address of the device to synchronize to. Like “11:22:33:44:55:66”.
	-> address_type: Address type of the device to connect to.
                       0: Public address.
                       1: Random address.  
	-> adv_sid: Advertising set identifiers.
                The adv_sid of a periodic advertising packet can be obtained by GAP_BLE_EXTENDED_SCAN_RESULT_EVT whether 						periodic_interval exists. Periodic_interval equal 0 indicates no periodic advertising packet.
```

**Note:** The synchronize timeout you set should bigger than Periodic advertising interval. If not, it will breaks the established synchronization. 



### demonstrate

- customize parameters 

```shell
root@OpenWrt:~# bleSynchronize 0 100 94:de:b8:f1:35:1a 0 0
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
```

- default parameters

The default parameters is [skip:0] [timeout:100] [address:NULL] [address_type:NULL] [adv_sid:NULL]

If you don't need to customize the parameters, you can use the default parameters. It automatically synchronizes the first periodic broadcast package scanned into the environment.

```shell
root@OpenWrt:~# bleSynchronize 
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
GAP_CB_MSG >> { "type": "periodic_adv_result", "mac": "94:de:b8:f1:35:1a", "address_type": 0, "rssi": -10, "adv_sid": 0, "periodic_interval": 120 }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -11, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
GAP_CB_MSG >> { "type": "sync_result", "rssi": -12, "data": "0201060709474c5f424c45111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344" }
```

You can use **CTRL+C** or **CTRL+\\** to terminate the application.

