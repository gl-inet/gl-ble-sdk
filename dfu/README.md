# BLE Module DFU

## 1. Build and compile

- Before using the DFU tool, you need to install the Openwrt SDK environment. Please set up the environment according to the following project documents: https://github.com/gl-inet/sdk.

- Secondly, you should clone the [gl-silabs-dfu](https://github.com/gl-inet/gl-silabs-dfu) repository into the package directory mentioned above. The version v1.3.1 is used when writing this document.

- The following is a general compile process.

```shell
# 1.Place your gl-silabs-dfu repository in sdk/<version>/<target>/package/, then run:
cd sdk/<version>/<target>/

# 2.compile
make package/gl-silabs-dfu/compile V=s
```



## 2. Select device BLE firmware

Currently, only supported for the GL-B2200 and GL-S200 to upgrade the BLE module firmware to v4.2.0. The following is the firmware corresponding to the device.

| Device   | BLE module firmware                                          |
| -------- | ------------------------------------------------------------ |
| GL-B2200 | [gl-efr32mg21f512-ble-ncp-v4_2_0.gbl](./gl-efr32mg21f512-ble-ncp-v4_2_0.gbl) |
| GL-S200  | [gl-efr32mg21f768-ble-ncp-v4_2_0.gbl](gl-efr32mg21f768-ble-ncp-v4_2_0.gbl) |



## 3. Upgrade

**Warning：This upgrade is risky and may cause the BLE module of the device to fail to work properly. Please confirm this before upgrading.**



- About GL-B2200, please use the bgaip soft mode to upgrade.

```shell
root@GL-B2200:~# gl-silabs-dfu bgapi soft ./gl-efr32mg21f512-ble-ncp-v4_2_0.gbl /dev/ttyMSM1 42 -v
sh: write error: Resource busy
Get upgrade firmware size: 217248
* System boot event!
* Module firmware version: 2.12.3
* Build number:            151
process:[====================================================================================================>]99% *** DFU END! ***
* System boot event!
* Module firmware version: 4.2.0
* Build number:            321
Module reset finish, please check firmware version.
```



- About GL-S200, please use the xmodem mode to upgrade.

```shell
root@GL-S200:~# gl-silabs-dfu xmodem ./gl-efr32mg21f768-ble-ncp-v4_2_0.gbl /dev/ttyCH343USB1 1 11 -v
file size:216216
start upload...
process:[====================================================================================================>]100% 
upload ok. total size:216320
reset chip now...
dfu success!
```
