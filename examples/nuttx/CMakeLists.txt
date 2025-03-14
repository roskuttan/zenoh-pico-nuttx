cmake_minimum_required(VERSION 3.28)
if(CONFIG_EXAMPLES_ZENOH_TEST)
  set(CMAKE_SYSTEM_NAME "Generic")
  set(WITH_NUTTX ON CACHE BOOL "Build for NuttX RTOS" FORCE)
  include_directories(${CMAKE_SOURCE_DIR}/apps/zenoh-pico-nuttx/include)
  add_subdirectory(../../../zenoh-pico-nuttx SOURCE_DIR)
  add_compile_definitions(ZENOH_NUTTX)
  nuttx_add_application(
    NAME
    ${CONFIG_EXAMPLES_ZENOH_TEST_PROGNAME}
    SRCS
    zenoh-test.c
    DEPENDS
    zenohpico_static
    STACKSIZE
    ${CONFIG_EXAMPLES_ZENOH_TEST_STACKSIZE}
    PRIORITY
    ${CONFIG_EXAMPLES_ZENOH_TEST_PRIORITY})
endif()
