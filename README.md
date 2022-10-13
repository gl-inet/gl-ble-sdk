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
bleScanner   	-------------------->  A Ble scanner that can configure scan parameters
bleAdvertiser   -------------------->  A Ble advertiser with configurable parameters
bleService   	-------------------->  A Ble server with configurable parameters
bleClient   	-------------------->  A Ble client with configurable parameters
bletool   		-------------------->  A tool that integrates all BLE API functions
```

We'll keep updating the example, so let us know if you have any good ideas :D



## API References

We will continue to update the online version of the API documentation.

[C API References](https://dev.gl-inet.com/gl-ble-sdk-api/group__user)

Users can also use doxygen to generate documents locally.



## Supported devices
BLE may not be available in all standard versions.So please confirm if it comes with BLE first. 

| device name | Details                                                      |
| ----------- | ------------------------------------------------------------ |
| GL-X750     | [GL-X750 / Spitz - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-x750/) |
| GL-S1300    | [GL-S1300 / Convexa-S - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-s1300/) |
| GL-XE300    | [GL-XE300 / Puli - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-xe300/) |
| GL-X300B    | [GL-X300B / Collie - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-x300b/) |
| GL-AP1300   | [GL-AP1300 / Cirrus - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-ap1300/) |
| GL-B2200    | [GL-B2200 / Velica - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-b2200/) |
| GL-S200     | [GL-S200 - GL.iNet (gl-inet.com)](https://www.gl-inet.com/products/gl-s200/) |

