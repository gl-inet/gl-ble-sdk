# GL-BLE-SDK     bletool

bletool is a cmd-line tool which provides a basic and simple method for developers to operate all the BLE functions.

You can use it to quickly verify BLE functionality and API usage.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
		-> <M> libglble...................................... GL inet BLE driver library
		-> <M> gl-bletool................................ GL inet BLE Debug Cmdline Tool
```

Make

```shell
make ./package/gl-ble-sdk/{clean,compile} V=s
```



## Using the example

You can use SCP or other means to upload example ipk to the device.

### install

```shell
opkg install libglble_2.0.0_mipsel_24kc.ipk
opkg install gl-bletool_2.0.0_mipsel_24kc.ipk
```

### Using

**note**

This example will detect whether the ble module firmware version is 4_2_0. If it is not, it will upgrade to the corresponding firmware version. However, the previous devices does not support this automatic upgrade method, which needs to be upgraded in other ways.

```shell
root@OpenWrt:~# bletool 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17563648, "hw": 1, "ble_hash": "4d5a9661" }
The ble module firmware version is not 4_2_0, please switch it.
```



You can use the help command to quickly view the currently supported commands. 

```shell
root@OpenWrt:~# bletool 
bletool >> MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
bletool >> help 
quit                           Quit bletool
help                           Help
enable                         Enable or disable the module
set_power                      Set the tx power level
local_address                  Get local Bluetooth module public address
show_adv_handle_list           Show adv handle list
create_adv_handle              Create adv handle
delete_adv_handle              Delete adv handle
set_legacy_adv_data            Set legacy adv data
set_extended_adv_data          Set extended adv data
set_periodic_adv_data          Set periodic adv data
start_legacy_adv               Set and Start legacy advertising
start_extended_adv             Set and Start extended advertising
start_periodic_adv             Set and Start periodic advertising
adv_stop                       Stop advertising
send_notify                    Send notification to remote device
set_gattdb                     Set local gatt database
discovery                      Start discovery
stop_discovery                 End current GAP procedure
synchronize                    synchronize to periodic advertising
stop_synchronize               stop synchronize to periodic advertising
connect                        Open connection
disconnect                     Close connection
get_rssi                       Get rssi of an established connection
get_service                    Get supported services list
get_char                       Get supported characteristics in specified service
set_notify                     Enable or disable the notifications and indications
read_value                     Read specified characteristic value
write_value                    Write characteristic value
```

You can also use the TAB completion command and the up and down key to view the history commands.



### CLI Command Instruction

***A default value means you may not set this parameter. “-” means you must set this parameter.***

#### enable

```shell
bletool >> enable 
{ "code": 0 }
```

**Description**：Enable or disable the BLE hardware.

**Parameters**：

| Type    | Name   | Default Value | Description                                                  |
| ------- | ------ | ------------- | ------------------------------------------------------------ |
| int32_t | enable | 1             | 0 means disable the BLE hardware; None-zero means enable the BLE hardware. |



#### set_power

```shell
bletool >> set_power 80
{ "code": 0, "current_power": 74 }
```

**Description**：Set the global power level.

**Parameters**：

| Type    | Name  | Default Value | Description                   |
| ------- | ----- | ------------- | ----------------------------- |
| int32_t | power | -             | Power level （0.1 dBm steps） |

**Note**: *current_power means the selected maximum output power level after applying RF path compensation.* *Due to hardware differences, there may be some deviation from the set parameters*



#### local_address

```shell
bletool >> local_address 
{ "code": 0, "mac": "94:de:b8:f1:35:1a" }
```

**Description**：Get the Local Bluetooth MAC address.



#### set_identity_address

```shell
bletool >> set_identity_address 0 94:de:b8:f1:35:10
{ "code": 0 }
```

**Description**：Set the device's Bluetooth identity address.

**Parameters**：

| Type    | Name     | Default Value | Description                                                  |
| ------- | -------- | ------------- | ------------------------------------------------------------ |
| int32_t | mac_type | -             | **0:** Public device address <br />**1:** Static device address |
| string  | mac      | _             | The MAC address of the device                                |

**Note**: The new address will be effective in the next system reboot. Please use `enable 0` and `enable 1`  to reboot the module.  To 00:00:00:00:00:00 and ff:ff:ff:ff:ff:ff as invalid addresses, if the address is set to these two will use the default address in the next system reboot.



#### get_identity_address

```shell
bletool >> get_identity_address
{ "code": 0, "mac": "d4:de:b8:f1:2c:03", "mac_type": 1 }
```

**Description**：Get the Bluetooth identity address used by the device, which can be a public or random static device address.

**Parameters**：

| Type    | Name     | Default Value | Description                                                  |
| ------- | -------- | ------------- | ------------------------------------------------------------ |
| string  | mac      | _             | The MAC address of the device                                |
| int32_t | mac_type | -             | **0:** Public device address <br />**1:** Static device address |



#### <span id="create_adv_handle">create_adv_handle</span>

```shell
bletool >> create_adv_handle 
{ "code": 0, "new_adv_handle": 0, "adv_handle_list": [ 0 ] }
```

**Description**：Create advertising set handle. It be used to operation of advertising.



#### delete_adv_handle

```shell
bletool >> delete_adv_handle 0
{ "code": 0, "adv_handle_list": [ ] }
```

**Description**：Delete advertising set handle.

**Parameters**：

| Type    | Name       | Default Value | Description                                                  |
| ------- | ---------- | ------------- | ------------------------------------------------------------ |
| int32_t | adv_handle | -             | The Advertising set handle which you want to delete.<br/>You can view the currently created adv_handle by [show_adv_handle_list](#show_adv_handle_list). |

**Note**: If the Advertising set handle that it is broadcasting, and then call this function to delete it, the broadcast is stopped.



#### <span id="show_adv_handle_list">show_adv_handle_list</span>

```shell
bletool >> show_adv_handle_list 
{ "adv_handle_list": [ 0, 2, 3 ] }
```

**Description**：Show the list of advertising set handle that is created.



#### set_legacy_adv_data

```shell
bletool >> set_legacy_adv_data ./legacy_adv.json 
{ "code": 0 }
```

**Description**：Act as BLE slave, set customized legacy advertising  data

**Parameters**:

| Type   | Name           | Default Value | Description                                    |
| ------ | -------------- | ------------- | ---------------------------------------------- |
| string | json_file_path | -             | The path of configuration parameter json file. |

**json file parameter**

| Key             | Value_type | Descriptiion                                                 |
| --------------- | ---------- | ------------------------------------------------------------ |
| adv_handle      | int32_t    | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| flag            | int32_t    | Adv data flag. <br/>0: Legacy advertising packets, the maximum size is 31 bytes  <br/>1: Scan response packets, the maximum size is 31 bytes |
| legacy_adv_data | string     | Customized legacy advertising data(hex). Like “020106”.      |

```shell
# The template of json file
{
        "adv_handle": 0,
        "flag": 0,
        "legacy_adv_data": "0201060709474c5f424c45"
}
```



#### set_extended_adv_data

```shell
bletool >> set_extended_adv_data ./extended_adv.json 
{ "code": 0 }
```

**Description**：Act as BLE slave, set customized extended advertising  data

**Parameters**:

| Type   | Name           | Default Value | Description                                    |
| ------ | -------------- | ------------- | ---------------------------------------------- |
| string | json_file_path | -             | The path of configuration parameter json file. |

**json file parameter**

| Key               | Value_type | Descriptiion                                                 |
| ----------------- | ---------- | ------------------------------------------------------------ |
| adv_handle        | int32_t    | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| extended_adv_data | string     | Customized extended advertising data(hex).<br/>Maximum 1024 bytes of data can be set for unconnectable extended advertising.<br/>Maximum 191 bytes of data can be set for connectable extended advertising. |

```shell
# The template of json file
{
        "adv_handle": 0,
        "extended_adv_data": "020106111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344"
}
```



#### set_periodic_adv_data

```shell
bletool >> set_periodic_adv_data ./periodic_adv.json 
{ "code": 0 }
```

**Description**：Act as BLE slave, set customized periodic advertising  data

**Parameters**:

| Type   | Name           | Default Value | Description                                    |
| ------ | -------------- | ------------- | ---------------------------------------------- |
| string | json_file_path | -             | The path of configuration parameter json file. |

**json file parameter**

| Key               | Value_type | Descriptiion                                                 |
| ----------------- | ---------- | ------------------------------------------------------------ |
| adv_handle        | int32_t    | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| periodic_adv_data | string     | Customized periodic advertising data(hex).<br/>Maximum 1024 bytes of data can be set for periodic advertising. |

```shell
# The template of json file
{
        "adv_handle": 0,
        "periodic_adv_data": "020106111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999000000000011111111112222222222333333333344"
}  
```



#### start_legacy_adv

```shell
bletool >> start_legacy_adv 0
{ "code": 0, "adv_handle": 0 }
```

**Description**：Set the advertising parameters and start advertising act as BLE slave.

**Parameters**:

| Type    | Name         | Default Value | Description                                                  |
| ------- | ------------ | ------------- | ------------------------------------------------------------ |
| uint8_t | adv_handle   | -             | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| int32_t | interval_min | 160 (100ms)   | Minimum advertising interval. Value in units of 0.625 ms. <br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s. |
| int32_t | interval_max | 160 (100ms)   | Maximum advertising interval. Value in units of 0.625 ms.<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s. |
| uint8_t | discover     | 2             | Discoverable mode.<br/>0: Not discoverable<br/>1: Discoverable by both limited and general discovery procedures<br/>2: Discoverable by the general discovery procedure <br/>3: Send legacy advertising and/or scan response data defined by the user.<br/>    The limited/general discoverable flags are defined by the user. |
| uint8_t | connect      | 2             | Connectable mode.<br/>0: Undirected non-connectable and non-scannable legacy advertising<br/>2: Undirected connectable and scannable legacy advertising<br/>3: Undirected scannable and non-connectable legacy advertising |

**Note:**

Interval_max should be bigger than interval_min.

Legacy Avertising is transmitted on LE 1M PHY by default.

If you want multiple Avertising to run at the same time, make sure their advertising interval are different.



#### start_extended_adv

```shell
bletool >> start_extended_adv 0
{ "code": 0, "adv_handle": 0 }
```

**Description**：Set the advertising parameters and start advertising act as BLE slave.

**Parameters**:

| Type    | Name          | Default Value | Description                                                  |
| ------- | ------------- | ------------- | ------------------------------------------------------------ |
| uint8_t | adv_handle    | -             | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| uint8_t | primary_phy   | 1             | The PHY on which the advertising packets are transmitted on the primary advertising channel.<br/>1: LE 1M PHY, 4: LE Coded PHY(125k, S=8) |
| uint8_t | secondary_phy | 1             | The PHY on which the advertising packets are transmitted on the secondary advertising channel.            1: LE 1M PHY, 2: LE 2M PHY, 4: LE Coded PHY(125k, S=8) |
| int32_t | interval_min  | 320(200ms)    | Minimum advertising interval. Value in units of 0.625 ms. <br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s. |
| int32_t | interval_max  | 320(200ms)    | Maximum advertising interval. Value in units of 0.625 ms.<br/>Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s. |
| uint8_t | discover      | 2             | Discoverable mode.<br/>0: Not discoverable<br/>1: Discoverable by both limited and general discovery procedures<br/>2: Discoverable by the general discovery procedure <br/>3: Send extended advertising data defined by the user.<br/>    The limited/general discoverable flags are defined by the user. |
| uint8_t | connect       | 4             | Connectable mode.<br/>0: Non-connectable and non-scannable extended advertising<br/>3: Scannable extended advertising<br/>4: Connectable extended advertising |

**Note:**

Interval_max should be bigger than interval_min.

Maximum 191 bytes of data can be set for connectable extended advertising.

Maximum 1024 bytes of data can be set for unconnectable extended advertising.

When Extended advertising packet is more than 254 bytes and short advertising interval you set, it will cause lose packet problem. Here's recommend advertising interval of no less than 200ms.



#### start_periodic_adv

```shell
bletool >> start_periodic_adv 0
{ "code": 0, "adv_handle": 0 }
```

**Description**：Set the advertising parameters and start advertising act as BLE slave.

**Parameters**:

| Type    | Name          | Default Value | Description                                                  |
| ------- | ------------- | ------------- | ------------------------------------------------------------ |
| uint8_t | adv_handle    | -             | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle). |
| uint8_t | primary_phy   | 1             | The PHY on which the advertising packets are transmitted on the primary advertising channel.<br/>1: LE 1M PHY, 4: LE Coded PHY(125k, S=8) |
| uint8_t | secondary_phy | 1             | The PHY on which the advertising packets are transmitted on the secondary advertising channel.            1: LE 1M PHY, 2: LE 2M PHY, 4: LE Coded PHY(125k, S=8) |
| int32_t | interval_min  | 80(100ms)     | Minimum periodic advertising interval. Value in units of 1.25 ms <br/>Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s |
| int32_t | interval_max  | 160(200ms)    | Maximum periodic advertising interval. Value in units of 1.25 ms<br/>Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s |

**Note:**

Interval_max should be bigger than interval_min.

The Periodic advertising interval you set should be smaller than synchronize timeout. If not, it will breaks the established synchronization.



#### stop_adv

```shell
bletool >> stop_adv 0
{ "code": 0 }
```

**Description**：Stop advertising.

**Parameters**:

| Type    | Name       | Default Value | Description                                                  |
| ------- | ---------- | ------------- | ------------------------------------------------------------ |
| uint8_t | adv_handle | -             | Advertising set handle. It be created by [create_adv_handle](#create_adv_handle).<br/>You can view the currently created adv_handle by [show_adv_handle_list](#show_adv_handle_list). |



#### send_notify

```shell
bletool >> send_notify 56:38:ac:a7:5f:96 16 010203
{ "code": 0 }
```

**Description**：Send notification to remote device.

**Parameters**:

| Type    | Name                         | Default Value | Description                                                  |
| ------- | ---------------------------- | ------------- | ------------------------------------------------------------ |
| string  | remote_mac                   | _             | The MAC address of the remote device                         |
| int32_t | notify_characteristic_handle | _             | GATT characteristic handle                                   |
| string  | data                         | _             | Data value to be sent.(Must be hexadecimal ASCII. Like “020106”) |

**Note**: *If BLE master don't enable notify, BLE slaver can't send notification. You can get notify_characteristic_handle in GATT_CB_MSG, check more in CB_MSG->GATT_CB_MSG->remote_set*



#### set_gattdb

```shell
bletool >> set_gattdb ./gl_gattdb_cfg.json
It will take a while, please waiting...
{ "code": 0 }
```

**Description**：Set the local Gatt database dynamically. 

**Parameters**:

| Type   | Name           | Default Value | Description         |
| ------ | -------------- | ------------- | ------------------- |
| string | json_file_name | _             | The json file name. |

**json file parameter**

| Key              | Value_type | Descriptiion                                                 |
| ---------------- | ---------- | ------------------------------------------------------------ |
| service          | json array | Each array element is a service instance                     |
| service_property | int32_t    | 0: The service should not be advertised<br/>1: The service should be advertised |
| service_uuid_len | int32_t    | 2:  16 bits uuid by SIG<br/>4:  32 bits uuid by SIG<br/>16: 128 bits uuid by custom generate |
| service_uuid     | string     | Service uuid                                                 |
| characteristic   | json array | mean the characteristics contained in a service instance, can be empty |



| Key             | Value_type | Descriptiion                                                 |
| --------------- | ---------- | ------------------------------------------------------------ |
| char_property   | int32_t    | A Characteristic Extended Properties descriptor is automatically added if the reliable write property is set.<br/>2:  GATTDB_CHARACTERISTIC_READ<br/>4:  GATTDB_CHARACTERISTIC_WRITE_NO_RESPONSE<br/>8:  GATTDB_CHARACTERISTIC_WRITE <br/>16: GATTDB_CHARACTERISTIC_NOTIFY <br/>32: GATTDB_CHARACTERISTIC_INDICATE <br/>128:GATTDB_CHARACTERISTIC_EXTENDED_PROPS <br/>257:GATTDB_CHARACTERISTIC_RELIABLE_WRITE |
| char_flag       | int32_t    | 0: Automatically create a Client Characteristic Configuration descriptor when adding a characteristic that has the notify or indicate property.<br/>1: Do not automatically create a Client Characteristic Configuration descriptor when adding a characteristic that has the notify or indicate property. |
| char_uuid_len   | int32_t    | 2:  16 bits uuid by SIG<br/>16: 128 bits uuid by custom generate |
| char_uuid       | string     | Characteristic uuid                                          |
| char_value_type | int32_t    | 1: fixed_length_value<br/>2: variable_length_value<br/>3: user_managed_value |
| char_maxlen     | int32_t    | The maximum length of the characteristic value. Ignored if value_type is user_managed_value. |
| char_value_len  | int32_t    | At most 240 bytes when uuid_len is 2 bytes, At most 226 bytes when uuid_len is 16 bytes |
| char_value      | string     | it should be hex format                                      |
| descriptor      | json array | mean the descriptors contained in a characteristic instance, can be empty |



| Key              | Value_type | Descriptiion                                                 |
| ---------------- | ---------- | ------------------------------------------------------------ |
| despt_property   | int32_t    | 1:  GATTDB_DESCRIPTOR_READ<br/> 2:  GATTDB_DESCRIPTOR_WRITE<br/>512: GATTDB_DESCRIPTOR_LOCAL_ONLY |
| despt_uuid_len   | int32_t    | 2:  16 bits uuid by SIG<br/>16: 128 bits uuid by custom generate |
| despt_uuid       | string     | Descriptor uuid                                              |
| despt_value_type | int32_t    | 1: fixed_length_value<br/>2: variable_length_value<br/>3: user_managed_value |
| despt_maxlen     | int32_t    | The maximum length of the descriptor value. Ignored if value_type is user_managed_value, or if this is a Client Characteristic Configuration descriptor. |
| despt_value_len  | int32_t    | At most 241 bytes when uuid_len is 2 bytes, At most 227 bytes when uuid_len is 16 bytes |
| despt_value      | string     | it should be hex format                                      |



```shell
# The template of json file
{
    "service": [
        {
            "service_property": 0,
            "service_uuid_len": 2,
            "service_uuid": "180a",
            "characteristic": [
                {
                    "char_property": 34,
                    "char_flag": 0,
                    "char_uuid_len": 16,
                    "char_uuid": "0D77CC114AC149F2BFA9CD96AC7A92F8",
                    "char_value_type": 1,
                    "char_maxlen": 5,
                    "char_value_len": 5,
                    "char_value": "342E322E30",
                    "descriptor": []
                }
            ]
        },
        {
            "service_property": 1,
            "service_uuid_len": 2,
            "service_uuid": "1800",
            "characteristic": [
                {
                    "char_property": 2,
                    "char_flag": 1,
                    "char_uuid_len": 2,
                    "char_uuid": "2A00",
                    "char_value_type": 1,
                    "char_maxlen": 6,
                    "char_value_len": 6,
                    "char_value": "474C5F424C45",
                    "descriptor": [
                        {
                            "despt_property": 1,
                            "despt_uuid_len": 2,
                            "despt_uuid": "2902",
                            "despt_value_type": 1,
                            "despt_maxlen": 2,
                            "despt_value_len": 2,
                            "despt_value": "3030"
                        }
                    ]
                },
                {
                    "char_property": 10,
                    "char_flag": 1,
                    "char_uuid_len": 2,
                    "char_uuid": "2A23",
                    "char_value_type": 1,
                    "char_maxlen": 3,
                    "char_value_len": 3,
                    "char_value": "333231",
                    "descriptor": []
                }
            ]
        }
    ]
}
```

**Note:** Please config JSON file(like /etc/ble/gl_gattdb_cfg.json) before execute this command. It will remove default static gatt database first before set new gatt database. **When you exit bletool, it will recovery to the default static gatt database.**



#### <span id="discovery">discovery</span>

```shell
# Default parameters, not specify MAC address
bletool >> discovery
{ "code": 0 }

# Default parameters, specify MAC address
bletool >> discovery 72:64:21:10:00:ba
{ "code": 0 }

# Customize parameters, not specify MAC address
bletool >> discovery 1 16 16 0 2
{ "code": 0 }

# Customize parameters, specify MAC address
bletool >> discovery 1 160 160 0 2 72:64:21:10:00:ba
{ "code": 0 }
```

**Description**：Set and start the BLE discovery.

**Parameters**:

| Type    | Name     | Default Value | Description                                                  |
| ------- | -------- | ------------- | ------------------------------------------------------------ |
| int32_t | phys     | 1             | The scanning PHY.<br/>1: LE 1M PHY<br/>4: LE Coded PHY<br/>5: Simultaneous LE 1M and Coded PHY alternatively |
| int32_t | interval | 16 (10ms)     | Scan interval. Time = Value x 0.625 ms<br/>Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s |
| int32_t | window   | 16 (10ms)     | Scan window. Time = Value x 0.625 ms<br/>Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s. |
| int32_t | type     | 0             | Scan type. <br/>0: Passive scanning<br/>1: Active scanning   |
| int32_t | mode     | 2             | Bluetooth discovery Mode.<br/>0: Discover only limited discoverable devices<br/>1: Discover limited and generic discoverable devices<br/>2: Discover all devices |
| string  | address: |               | (**optional**)Scans the broadcast packet for the specified MAC address. Like “11:22:33:44:55:66”(**low case**). |

**Note**: You can stop the current Ble scan by typing **q+Enter**.



#### stop_discovery

```shell
bletool >> stop_discovery
{ "code": 0 }
```

**Description**：Stop discovery procedure. 



#### synchronize

```shell
# Default parameters
bletool >> synchronize
{ "code": 0 }

# Customize parameters 
bletool >> synchronize 0 100 94:de:b8:f1:23:f4 0 0
{ "code": 0 }
```

**Description**：Set and start the BLE synchronize.

**Parameters**:

| Type    | Name         | Default Value | Description                                                  |
| ------- | ------------ | ------------- | ------------------------------------------------------------ |
| int32_t | skip         | 0             | The maximum number of periodic advertising packets that can be skipped after a successful receive. Range: 0x0000 to 0x01F3. |
| int32_t | timeout      | 100(1000ms)   | The maximum permitted time between successful receives. If this time is exceeded, synchronization is lost. Unit: 10 ms.<br/>Range: 0x0A to 0x4000, Time Range: 100 ms to 163.84 s. |
| string  | address      | NULL          | Address of the device to synchronize to. Like “11:22:33:44:55:66”(**low case**). |
| int32_t | address_type | NULL          | Address type of the device to connect to.<br/>0: public address<br/>1: random address |
| int32_t | adv_sid      | NULL          | Advertising set identifiers. You can get adv_sid in GAP_CB_MSG, check more in CB_MSG->GAP_CB_MSG->[scan_result](#scan_result). |

**Note:**  **Please use [discovery](#discover) to enable ble scanner firstly, after that you can stop it or keep it enable.** If you don't need to customize the parameters, you can use the default parameters. It automatically synchronizes the first periodic broadcast package scanned into the environment.



#### stop_synchronize

```shell
bletool >> stop_synchronize 
{ "code": 0 }
```

**Description**：Stop synchronizeprocedure. 



#### connect

```shell
bletool >> connect 1 1 73:8c:01:1d:3f:b0
{ "code": 0 }
```

**Description**：Act as master, start connect to a remote BLE device.

**Parameters:**

| Type    | Name         | Default Value | Description                                                  |
| ------- | ------------ | ------------- | ------------------------------------------------------------ |
| int32_t | phy          | _             | The initiating PHY.<br/>1: LE 1M PHY<br/>4: LE Coded PHY     |
| int32_t | address_type | -             | Advertiser address type.<br/>0: Public device address<br/>1: Static device address <br/>2: Resolvable private random address <br/>3: Non-resolvable private random address |
| string  | address      | -             | Remote BLE device address.(**low case**)                     |

**Note**: If connect success, it will report a **connect_open** message, check more in CB_MSG->GAP_CB_MSG->[connect_open](#connect_open).



#### disconnect

```
bletool >> disconnect 73:8c:01:1d:3f:b0
{ "code": 0 }
```

**Description**：Disconnect with remote device.

**Parameters**:

| Type   | Name    | Default Value | Description                                        |
| ------ | ------- | ------------- | -------------------------------------------------- |
| string | address | -             | The MAC address of the remote device(**low case**) |

**Note**: If success, it will report a **connect_close** message, check more in CB_MSG->GAP_CB_MSG->[connect_close](#connect_close).



#### get_rssi

```shell
bletool >> get_rssi 73:8c:01:1d:3f:b0
{ "code": 0, "rssi": -35 }
```

**Description**：Get rssi of connection with remote device.

**Parameters**:

| Type   | Name    | Default Value | Description                          |
| ------ | ------- | ------------- | ------------------------------------ |
| string | address | -             | The MAC address of the remote device |



#### get_service

```shell
bletool >> get_service 73:8c:01:1d:3f:b0
{ "code": 0, "service_list": [ { "service_handle": 65539, "service_uuid": "1801" }, { "service_handle": 1310746, "service_uuid": "1800" }, { "service_handle": 2621490, "service_uuid": "fe35" }, { "service_handle": 3342389, "service_uuid": "046a" }, { "service_handle": 3539002, "service_uuid": "11c8b31080e44276afc0f81590b2177f" }, { "service_handle": 3866691, "service_uuid": "0000aaa0000010008000aabbccddeeff" }, { "service_handle": 4521983, "service_uuid": "181c" } ] }
```

**Description**：Act as master, get service list of a remote GATT server.

**Parameters**:

| Type   | Name    | Default Value | Description                                        |
| ------ | ------- | ------------- | -------------------------------------------------- |
| string | address | -             | The MAC address of the remote device(**low case**) |



#### get_char

```shell
bletool >> get_char 73:8c:01:1d:3f:b0 65539
{ "code": 0, "characteristic_list": [ { "characteristic_handle": 3, "properties": 32, "characteristic_uuid": "2a05" } ] }
```

**Description**：Act as master, Get characteristic list of a remote GATT server.

**Parameters**:

| Type    | Name           | Default Value | Description                                        |
| ------- | -------------- | ------------- | -------------------------------------------------- |
| string  | address        | -             | The MAC address of the remote device(**low case**) |
| int32_t | service_handle | -             | Service handle                                     |



#### set_notify

```shell
bletool >> set_notify 73:8c:01:1d:3f:b0 61 1
{ "code": 0 }
```

**Description**：Act as master, Enable or disable the notification or indication of a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                                                  |
| ------- | ----------- | ------------- | ------------------------------------------------------------ |
| string  | address     | -             | The MAC address of the remote device(**low case**)           |
| int32_t | char_handle | -             | Characteristic handle                                        |
| int32_t | flag        | -             | Notification flag.<br/>0: disable<br/>1: notification<br/>2: indication |



#### read_value

```shell
bletool >> read_value 73:8c:01:1d:3f:b0 61
{ "code": 0 }
```

**Description**：Act as master, Read value of specified characteristic in a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                                        |
| ------- | ----------- | ------------- | -------------------------------------------------- |
| string  | address     | -             | The MAC address of the remote device(**low case**) |
| int32_t | char_handle | -             | Characteristic handle                              |

**Note**: If success, it will report a **remote_characteristic_value** message, check more in CB_MSG->GATT_CB_MSG->[remote_characteristic_value](#remote_characteristic_value).



#### write_value

```shell
bletool >> write_value 73:8c:01:1d:3f:b0 67 0 0102  
{ "code": 0 }
```

**Description**：Act as master, Write value to specified characteristic in a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                                                 |
| ------- | ----------- | ------------- | ----------------------------------------------------------- |
| string  | address     | _             | The MAC address of the remote device(**low case**)          |
| int32_t | char_handle | _             | Characteristic handle                                       |
| int32_t | res         | _             | Response flag<br/>0: without response <br/>1: with response |
| string  | value       | _             | Value to be written(hex)                                    |



#### uart_test

**Description**：Test the stability of serial communication.



### CB_MSG

Bletool displays any data that is actively reported by ble module.

#### MODULE_CB_MSG

##### module_start

This data is reported every time the Ble module is started.

```shell
MODULE_CB_MSG >> { "type": "module_start", "major": 4, "minor": 2, "patch": 0, "build": 321, "bootloader": 0, "hw": 257, "ble_hash": "1b0e33cd" }
```

#### GAP_CB_MSG

##### <span id="scan_result">scan_result</span>

This data is reported when scanning ble Legacy broadcast.

```shell
GAP_CB_MSG >> { "type": "legacy_adv_result", "mac": "6c:56:69:93:db:3c", "address_type": 1, "rssi": -54, "packet_type": 0, "bonding": 255, "data": "02011a020a070bff4c0010060d1abc419e9c" }
```

This data is reported when scanning ble Extended broadcast.

```shell
GAP_CB_MSG >> { "type": "extended_adv_result", "mac": "94:de:b8:f1:13:75", "address_type": 0, "rssi": -27, "tx_power": 127, "event_flags": 0, "adv_sid": 0, "periodic_interval": 120, "bonding": 255, "data": "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" }
```

This data is reported when sync ble Periodic broadcast.

```shell
GAP_CB_MSG >> { "type": "sync_result", "tx_power": 127, "rssi": -29, "data": "123455" }
```

##### connect_update

This data is reported when connection parameters update.

```shell
GAP_CB_MSG >> { "type": "connect_update", "mac": "56:38:ac:a7:5f:96", "interval": 40, "latency": 0, "timeout": 500, "security_mode": 0, "txsize": 27 }
```

##### <span id="connect_open">connect_open</span>

This data is reported when connection open.

```shell
bletool >> GAP_CB_MSG >> { "type": "connect_open", "mac": "56:38:ac:a7:5f:96", "address_type": 1, "connect_role": 0, "bonding": 255, "advertiser": 0 }
```

##### <span id="connect_close">connect_close</span>

This data is reported when connection close.

```shell
bletool >> GAP_CB_MSG >> { "type": "connect_close", "mac": "56:38:ac:a7:5f:96", "reason": 531 }
```

#### GATT_CB_MSG

##### <span id="remote_characteristic_value">remote_characteristic_value</span>

This data is reported when get remote device characteristic value.

```shell
GATT_CB_MSG >> { "type": "remote_characteristic_value", "mac": "73:8c:01:1d:3f:b0", "characteristic": 61, "att_opcode": 11, "offset": 0, "value": "01" }
```

##### local_gatt_attribute

This data is reported when local gatt attribute change, such as remote device writing data.

```shell
GATT_CB_MSG >> { "type": "local_gatt_attribute", "mac": "5a:51:f9:7d:fb:11", "attribute": 14, "att_opcode": 18, "offset": 0, "value": "12" }
```

##### remote_set

This data is reported when master setting characteristic flag, such as setting client_config_flags.

```shell
GATT_CB_MSG >> { "type": "remote_set", "mac": "56:38:ac:a7:5f:96", "characteristic": 16, "status_flags": 1, "client_config_flags": 1 }
```
