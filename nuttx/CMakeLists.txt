# CMakeLists.txt for Zenoh-Pico NuttX Port
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0

if(CONFIG_ZENOH_PICO_NUTTX)
  # Set up the ARM toolchain for NuttX (adjust as necessary for your target)
#   set(NUTTX_TOOLCHAIN_PREFIX "arm-none-eabi")
#   set(CMAKE_C_COMPILER   ${NUTTX_TOOLCHAIN_PREFIX}-gcc)
#   set(CMAKE_CXX_COMPILER ${NUTTX_TOOLCHAIN_PREFIX}-g++)
#   set(CMAKE_ASM_COMPILER ${NUTTX_TOOLCHAIN_PREFIX}-as)
#   set(CMAKE_AR           ${NUTTX_TOOLCHAIN_PREFIX}-ar)
#   set(CMAKE_OBJCOPY      ${NUTTX_TOOLCHAIN_PREFIX}-objcopy)
#   set(CMAKE_OBJDUMP      ${NUTTX_TOOLCHAIN_PREFIX}-objdump)
#   set(CMAKE_RANLIB       ${NUTTX_TOOLCHAIN_PREFIX}-ranlib)

  # Linker flags: remove unused sections
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

  # Ensure proper cross-compilation behavior
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

  # Collect NuttX-specific Zenoh-Pico system sources.
  file(GLOB_RECURSE SOURCES_NUTTX "../src/system/nuttx/*.c")

  # Create a static library for the NuttX port.
  add_library(zenohpico_nuttx STATIC ${SOURCES_NUTTX})

  # Include Zenoh-Pico headers (assumed to be in ../include).
  target_include_directories(zenohpico_nuttx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

  # Force feature definitions based on our Kconfig settings:
  # Enable Serial transport; disable TCP, UDP, Bluetooth, and Raw Ethernet.
  target_compile_definitions(zenohpico_nuttx PRIVATE
    Z_FEATURE_LINK_SERIAL=1
    Z_FEATURE_LINK_TCP=0
    Z_FEATURE_LINK_UDP_UNICAST=0
    Z_FEATURE_LINK_UDP_MULTICAST=0
    Z_FEATURE_LINK_BLUETOOTH=0
    Z_FEATURE_RAWETH_TRANSPORT=0
  )

  # Optionally link with pthreads if multithreading is enabled in your configuration.
  find_package(Threads REQUIRED)
  target_link_libraries(zenohpico_nuttx PRIVATE Threads::Threads)
endif()
