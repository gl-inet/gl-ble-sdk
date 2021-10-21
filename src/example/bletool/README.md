# GL-BLE-SDK     bletool

bletool is a cmd-line tool which provides a basic and simple method for developers to operate all the BLE functions.

You can use it to quickly verify BLE functionality and API usage.



## Compile

Make sure you have the compile option enabled before compiling the project.

```
make menuconfig
-> gl-inet-ble
		-> {M} libglble...................................... GL inet BLE driver library
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
opkg install libglble_1.0.0_mipsel_24kc.ipk
opkg install gl-bletool_1.0.0_mipsel_24kc.ipk
```

### Using

You can use the help command to quickly view the currently supported commands. 

```shell
root@GL-MT300N-V2:~# bletool 
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17367041, "hw": 1, "ble_hash": "61965a4d" }
bletool >> help
quit                           Quit bletool
help                           Help
enable                         Enable or disable the module
set_power                      Set the tx power level
local_address                  Get local Bluetooth module public address
adv_data                       Set adv data
adv                            Set and Start advertising
adv_stop                       Stop advertising
send_notify                    Send notification to remote device
discovery                      Start discovery
stop_discovery                 End current GAP procedure
connect                        Open connection
disconnect                     Close connection
get_rssi                       Get rssi of an established connection
get_service                    Get supported services list
get_char                       Get supported characteristics in specified service
set_notify                     Enable or disable the notifications and indications
read_value                     Read specified characteristic value
write_value                    Write characteristic value
test                           test
bletool >>
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

| Type    | Name   | Default Value* | Description                                                  |
| ------- | ------ | -------------- | ------------------------------------------------------------ |
| int32_t | enable | 1              | 0 means disable the BLE hardware; None-zero means enable the BLE hardware. |



#### local_address

```shell
bletool >> local_address 
{ "code": 0, "mac": "80:4b:50:50:e7:72" }
```

**Description**：Get the Local Bluetooth MAC address.



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



#### adv_data

```shell
bletool >> adv_data 0 020106050974657374
{ "code": 0 }
```

**Description**：Act as BLE slave, set customized advertising data

**Parameters**:

| Type    | Name | Default Value | Description                       |
| ------- | ---- | ------------- | --------------------------------- |
| int32_t | flag | -             | Adv data flag.                    |
| string  | data | -             | Customized advertising data(hex). |

**flag**: 

- 0: Advertising packets

- 1: Scan response packets
- 2: OTA advertising packets
- 4: OTA scan response packets



#### adv

```shell
bletool >> adv
{ "code": 0 }
```

**Description**：Set the advertising parameters and start advertising act as BLE slave.

**Parameters**:

| Type    | Name         | Default Value | Description                                                  |
| ------- | ------------ | ------------- | ------------------------------------------------------------ |
| int32_t | phys         | 1             | The PHY on which the advertising packets are transmitted on. |
| int32_t | interval_min | 160 (100ms)   | Minimum advertising interval.                                |
| int32_t | interval_max | 160 (100ms)   | Maximum advertising interval.                                |
| int32_t | discover     | 2             | Discoverable mode.                                           |
| int32_t | connect      | 2             | Connectable mode.                                            |

**phys**:

- 1: LE 1M PHY

- *4:(not support now) LE Coded PHY*

**discover**: 

- 0: Not discoverable 

- 1: Discoverable using both limited and general discovery procedures

- 2: Discoverable using general discovery procedure

- 3: Device is not discoverable in either limited or generic discovery procedure, but may be discovered by using the Observation procedure

- 4: Send advertising and/or scan response data defined by the user. The limited/general discoverable flags are defined by the user.

**connect**:

- 0: Non-connectable non-scannable

- 1: Directed connectable (RESERVED, DO NOT USE)

- 2: Undirected connectable scannable (This mode can only be used in legacy advertising PDUs)

- 3: Undirected scannable (Non-connectable but responds to scan requests)

- 4: Undirected connectable non-scannable. This mode can only be used in extended advertising PDUs



#### adv_stop

```shell
bletool >> adv_stop 
{ "code": 0 }
```

**Description**：Stop advertising.



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



#### discovery

```shell
bletool >> discovery 
{ "code": 0 }
```

**Description**：Set and start the BLE discovery.

**Parameters**:

| Type    | Name     | Default Value | Description               |
| ------- | -------- | ------------- | ------------------------- |
| int32_t | phys     | 1             | The scanning PHY.         |
| int32_t | interval | 16 (10ms)     | Scan interval.            |
| int32_t | window   | 16 (10ms)     | Scan window.              |
| int32_t | type     | 0             | Scan type.                |
| int32_t | mode     | 1             | Bluetooth discovery Mode. |

**phys**:

- 1: LE 1M PHY

- 4:(not support now) LE Coded PHY

**type**:

- 0: Passive scanning
- 1: Active scanning

**mode**:

- 0: Discover only limited discoverable devices
- 1: Discover limited and generic discoverable devices
- 2: Discover all devices

**Note**: You can stop the current Ble scan by typing **q+Enter**.



#### stop_discovery

```shell
bletool >> stop_discovery
```

**Description**：Stop discovery procedure. 



#### connect

```shell
bletool >> connect 1 1 73:8c:01:1d:3f:b0
{ "code": 0 }
```

**Description**：Act as master, start connect to a remote BLE device.

**Parameters:**

| Type    | Name         | Default Value | Description                |
| ------- | ------------ | ------------- | -------------------------- |
| int32_t | phy          | _             | The initiating PHY.        |
| int32_t | address_type | -             | Advertiser address type.   |
| string  | address      | -             | Remote BLE device address. |

**phys**:

- 1: LE 1M PHY

- 4:(not support now) LE Coded PHY

**address_type**:

- 0: Public address
- 1: Random address
- 2: Public identity address resolved by stack
- 3: Random identity address resolved by stack

**Note**: If connect success, it will report a **connect_open** message, check more in CB_MSG->GAP_CB_MSG->connect_open.



#### disconnect

```
bletool >> disconnect 73:8c:01:1d:3f:b0
{ "code": 0 }
```

**Description**：Disconnect with remote device.

**Parameters**:

| Type   | Name    | Default Value | Description                          |
| ------ | ------- | ------------- | ------------------------------------ |
| string | address | -             | The MAC address of the remote device |

**Note**: If success, it will report a **connect_close** message, check more in CB_MSG->GAP_CB_MSG->connect_close.



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

| Type   | Name    | Default Value | Description                          |
| ------ | ------- | ------------- | ------------------------------------ |
| string | address | -             | The MAC address of the remote device |



#### get_char

```shell
bletool >> get_char 73:8c:01:1d:3f:b0 65539
{ "code": 0, "characteristic_list": [ { "characteristic_handle": 3, "properties": 32, "characteristic_uuid": "2a05" } ] }
```

**Description**：Act as master, Get characteristic list of a remote GATT server.

**Parameters**:

| Type    | Name           | Default Value | Description                          |
| ------- | -------------- | ------------- | ------------------------------------ |
| string  | address        | -             | The MAC address of the remote device |
| int32_t | service_handle | -             | Service handle                       |



#### set_notify

```shell
bletool >> set_notify 73:8c:01:1d:3f:b0 61 1
{ "code": 0 }
```

**Description**：Act as master, Enable or disable the notification or indication of a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                          |
| ------- | ----------- | ------------- | ------------------------------------ |
| string  | address     | -             | The MAC address of the remote device |
| int32_t | char_handle | -             | Characteristic handle                |
| int32_t | flag        | -             | Notification flag.                   |

**flag**:

- 0: disable
- 1: notification
- 2: indication



#### read_value

```shell
bletool >> read_value 73:8c:01:1d:3f:b0 61
{ "code": 0 }
```

**Description**：Act as master, Read value of specified characteristic in a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                          |
| ------- | ----------- | ------------- | ------------------------------------ |
| string  | address     | -             | The MAC address of the remote device |
| int32_t | char_handle | -             | Characteristic handle                |

**Note**: If success, it will report a **remote_characteristic_value** message, check more in CB_MSG->GATT_CB_MSG->remote_characteristic_value.



#### write_value

```shell
bletool >> write_value 73:8c:01:1d:3f:b0 67 0 0102  
{ "code": 0 }
```

**Description**：Act as master, Write value to specified characteristic in a remote gatt server.

**Parameters**:

| Type    | Name        | Default Value | Description                          |
| ------- | ----------- | ------------- | ------------------------------------ |
| string  | address     | _             | The MAC address of the remote device |
| int32_t | char_handle | _             | Characteristic handle                |
| int32_t | res         | _             | Response flag                        |
| string  | value       | _             | Value to be written                  |



#### test

**Description**：Empty function, users can add their own code in this source code to do the test.



### CB_MSG

Bletool displays any data that is actively reported by ble module.

#### MODULE_CB_MSG

##### module_start

This data is reported every time the Ble module is started.

```shell
MODULE_CB_MSG >> { "type": "module_start", "major": 2, "minor": 13, "patch": 10, "build": 423, "bootloader": 17367041, "hw": 1, "ble_hash": "61965a4d" }
```

#### GAP_CB_MSG

##### scan_result

This data is reported when scanning ble broadcast.

```shell
GAP_CB_MSG >> { "type": "scan_result", "mac": "6c:56:69:93:db:3c", "address_type": 1, "rssi": -54, "packet_type": 0, "bonding": 255, "data": "02011a020a070bff4c0010060d1abc419e9c" }
```

##### connect_update

This data is reported when connection parameters update.

```shell
GAP_CB_MSG >> { "type": "connect_update", "mac": "56:38:ac:a7:5f:96", "interval": 40, "latency": 0, "timeout": 500, "security_mode": 0, "txsize": 27 }
```

##### connect_open

This data is reported when connection open.

```shell
bletool >> GAP_CB_MSG >> { "type": "connect_open", "mac": "56:38:ac:a7:5f:96", "address_type": 1, "connect_role": 0, "bonding": 255, "advertiser": 0 }
```

##### connect_close

This data is reported when connection close.

```shell
bletool >> GAP_CB_MSG >> { "type": "connect_close", "mac": "56:38:ac:a7:5f:96", "reason": 531 }
```

#### GATT_CB_MSG

##### remote_characteristic_value

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

