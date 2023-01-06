cmake_minimum_required( VERSION 3.0 )

project(gl-ble-sdk
		VERSION 2.0.0 
        LANGUAGES C )

add_definitions( -D_GNU_SOURCE )
# add_compile_options(-O -Wall -Werror --std=gnu99)
add_compile_options(-O -Wall --std=gnu99)

set(CMAKE_C_FLAGS_RELEASE "-O3") 

add_subdirectory(./lib build/lib)
add_subdirectory(./example/bleScanner build/bleScanner)
add_subdirectory(./example/bleSynchronize build/bleSynchronize)
add_subdirectory(./example/bleAdvExtended build/bleAdvExtended)
add_subdirectory(./example/bleAdvLegacy build/bleAdvLegacy)
add_subdirectory(./example/bleAdvPeriodic build/bleAdvPeriodic)
add_subdirectory(./example/bleClient build/bleClient)
add_subdirectory(./example/bleService build/bleService)
add_subdirectory(./example/bletool build/bletool)
add_subdirectory(./example/bleIbeacon build/bleIbeacon)