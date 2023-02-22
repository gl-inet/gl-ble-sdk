# GL-Ble-SDK User Guide




## What’s GL-Ble-SDK

GL-Ble-SDK is developed and provided by GL-iNet Technology as a part of the iot solution. SDK encapsulates the operation interface of bluetooth modules from different manufacturers and provides users with bluetooth operation functions after abstraction. Users don't need to care what Bluetooth module they are using, just call the **gl-ble-api** in their application to control Bluetooth.



## How to test easily

bletool is a cmd line tool for debug gl-ble module which bases on gl-ble-api. You can use it for quick controning and managing ble.

For information on how to use bletool, you can refer to the [Bletool User Guide](src/example/bletool/README.md).



## Build develop environment

Before using the SDK, you need to install the Openwrt SDK environment.

Please set up the environment according to the following project documents:

https://github.com/gl-inet/sdk



## Download&Compile

```shell
$ git clone https://github.com/gl-inet/gl-ble-sdk.git
```

Place your package in `openwrt-sdk/<version>/<target>/package/`, then run:

```shell
$ cd sdk/<version>/<target>/
$ make package/<package_name>/compile V=s
```

For example, to compile the ble SDK for the S1300

```shell
$ cd sdk/qsdk53/ipq806x/
$ make package/bletool/{clean,compile} V=s
```

If there is a problem with missing dependent libraries during compilation, follow these steps

```shell
$ ./scripts/feeds update -f
$ ./scripts/feeds install <dependency_name>
```



## Directory structure of gl-ble-sdk source code

```
gl-ble-sdk
	|
	|- Doxyfile
	|- LICENSE
	|- Makefile
	|- docs
	|- src
	|	|
	|	|- bledriver           # ble module driver
	|	|- components          # sdk component
	|	|- example             # example folder
	|	|- lib                 # gl-ble-api lib source code
	|	|- CMakeLists.txt
```



## Example introduce

We provide a series of examples(src/example) to help users better understand and use the API. 

```
bleAdvertiserLegacy    -------------------->  A Ble Legacy advertiser with configurable parameters
bleAdvertiserExtended  -------------------->  A Ble Extended advertiser with configurable parameters 
bleAdvertiserPeriodic  -------------------->  A Ble Periodic advertiser with configurable parameters 
bleScanner             -------------------->  A Ble scanner that can configure scan parameters
bleSynchronize         -------------------->  A Ble Synchronize that can configure scan parameters
bleIbeacon             -------------------->  A Ble Ibeacon that can configure send or receive.
bleService             -------------------->  A Ble server wait for connected
bleClient              -------------------->  A Ble client with configurable parameters
bletool                -------------------->  A tool that integrates all BLE API functions
```

We'll keep updating the example, so let us know if you have any good ideas :D

**Note: The above example will check the BLE module firmware version, if the version of BLE module firmware is not v4.2.0, it will exit. Please see [DFU DOCS](./dfu).**



## API References

We will continue to update the online version of the API documentation.

[C API References](https://dev.gl-inet.com/gl-ble-sdk-api/group__user)

Users can also use doxygen to generate documents locally.



## Supported devices

| device name | Details                                                      |
| ----------- | ------------------------------------------------------------ |
| GL-B2200    | [GL-B2200 / Velica - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-b2200/) |
| GL-S200     | [GL-S200 - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-s200/) |

We'll updating the Supported devices.
