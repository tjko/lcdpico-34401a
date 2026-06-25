# CMakeLists.txt for lcd-pico
#

cmake_minimum_required(VERSION 3.18)

# Include Pico-SDK
if (DEFINED ENV{PICO_SDK_PATH})
   include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
else ()
   include(cmake/pico_sdk_import.cmake)
endif ()
message("Pico SDK version: ${PICO_SDK_VERSION_STRING}")

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
   message(FATAL_ERROR "Pico-SDK version 2.0.0 or later required.")
endif()


project(firmware
  VERSION 0.1.0
  DESCRIPTION "LCD Driver for HP34401A"
  HOMEPAGE_URL https://kokkonen.net/lcd-pico-34401a/
  LANGUAGES C CXX ASM
  )
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


set(LCDPICO_BUILD beta CACHE STRING "LCD-Pico Build Tag")
set(LCDPICO_BOARD 34401A CACHE STRING "LCD-Pico Board Model")
set(LCDPICO_LCD er-tft371-1 CACHE STRING "LCD-Pico LCD Panel Model")
set(LCDPICO_THEME nixie CACHE STRING "LCD-Pico Theme")

set(TLS_SUPPORT 1 CACHE STRING "TLS Support")
# Generate some "random" data for mbedtls (better than nothing...)
set(EXTRA_ENTROPY_LEN 64)
string(RANDOM LENGTH ${EXTRA_ENTROPY_LEN} EXTRA_ENTROPY)

#set(CMAKE_BUILD_TYPE Debug)

# extract panel resolution

file(STRINGS "src/lcd/${LCDPICO_LCD}.h" LCD_RESOLUTION_LINE LIMIT_COUNT 1 REGEX "PANEL_RESOLUTION")
if (LCD_RESOLUTION_LINE MATCHES "PANEL_RESOLUTION[ \t]+\"(.*)\"")
  set(LCD_RESOLUTION ${CMAKE_MATCH_1})
  message("LCD resolution: ${LCD_RESOLUTION}")
else()
  message(FATAL_ERROR "Failed to detect LCD resolution.")
endif()


pico_sdk_init()

message("---------------------------------")
message("       LCDPICO_BOARD: ${LCDPICO_BOARD}")
message("         LCDPICO_LCD: ${LCDPICO_LCD}")
message("          PICO_BOARD: ${PICO_BOARD}")
message("       PICO_PLATFORM: ${PICO_PLATFORM}")
message("         TLS_SUPPORT: ${TLS_SUPPORT}")
message("    CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("---------------------------------")



# External libaries


# pico-lfs
add_subdirectory(libs/pico-lfs)
# prevent warnings from littlefs...
set_source_files_properties(
  libs/pico-lfs/libs/littlefs/lfs.c
  PROPERTIES
  COMPILE_OPTIONS "-Wno-unused-function"
  )

# cJSON
add_library(cJSON INTERFACE)
target_include_directories(cJSON INTERFACE libs/cJSON)
target_sources(cJSON INTERFACE
  libs/cJSON/cJSON.h
  libs/cJSON/cJSON.c
  )

# libb64
add_library(libb64 INTERFACE)
target_include_directories(libb64 INTERFACE libs/libb64/include)
target_sources(libb64 INTERFACE
  libs/libb64/src/cdecode.c
  libs/libb64/src/cencode.c
  )

# PNGdec
add_library(PNGdec INTERFACE)
target_include_directories(PNGdec INTERFACE libs/PNGdec2/src)
target_sources(PNGdec INTERFACE
  libs/PNGdec2/src/PNGdec.c
  libs/PNGdec2/src/adler32.c
  libs/PNGdec2/src/crc32.c
  libs/PNGdec2/src/infback.c
  libs/PNGdec2/src/inffast.c
  libs/PNGdec2/src/inflate.c
  libs/PNGdec2/src/inftrees.c
  libs/PNGdec2/src/zutil.c
  )

# pico-telnetd
add_subdirectory(libs/pico-telnetd)

# pico-sensor-lib
add_subdirectory(libs/pico-sensor-lib)

# pico-sshd
add_subdirectory(libs/pico-sshd)
target_compile_definitions(pico-sshd-lib INTERFACE PICOSSHD_EXTERNAL_SHA512CRYPT)

# wolfSSL / wolfCrypt
if (DEFINED ENV{WOLFSSL_ROOT})
  set(WOLFSSL_ROOT $ENV{WOLFSSL_ROOT})
else ()
  set(WOLFSSL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/wolfssl)
endif ()
message("wolfSSL available at ${WOLFSSL_ROOT}")
include(libs/pico-sshd/cmake/wolfssl_pico.cmake)

# wolfSSH
if (DEFINED ENV{WOLFSSH_ROOT})
  set(WOLFSSH_ROOT $ENV{WOLFSSH_ROOT})
else ()
  set(WOLFSSH_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/wolfssh)
endif ()
message("wolfSSH available at ${WOLFSSH_ROOT}")
include(libs/pico-sshd/cmake/wolfssh_pico.cmake)

include_directories(src/wolfssl)



# FanPico Firmware

add_executable(firmware
  src/lcd-pico.c
  src/bi_decl.c
  src/command.c
  src/command_util.c
  src/flash.c
  src/config.c
  src/display.c
  src/network.c
  src/tls.c
  src/sensors.c
  src/i2c.c
  src/psram.c
  src/memtest.c
  src/util.c
  src/util_rp2.c
  src/log.c
  src/crc32.c
  src/lt7680.c
  src/st7701.c
#  src/st7701_lcd.c
#  src/lt7680_lcd.c
  src/lcd/er-tft371-1.c
  src/lcd/er-tft458-1.c
  src/default_config.S
  src/credits.s
  src/img/logo.S
  src/img/display.S
  )


set_property(SOURCE src/*.s APPEND PROPERTY COMPILE_OPTIONS -I${CMAKE_CURRENT_LIST_DIR}/src)
set_property(SOURCE src/credits.s APPEND PROPERTY COMPILE_OPTIONS -I${CMAKE_CURRENT_LIST_DIR})


pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)
pico_add_extra_outputs(firmware)

target_include_directories(firmware PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})

configure_file(src/config.h.in config.h)
configure_file(src/lcd-pico-compile.h.in lcd-pico-compile.h)

target_link_libraries(firmware PRIVATE
  pico_stdlib
  pico_stdio_uart
  pico_multicore
  pico_rand
  pico_aon_timer
  hardware_pwm
  hardware_pio
  hardware_adc
  hardware_i2c
  hardware_spi
  pico-lfs
  cJSON
  libb64
  pico_sensor_lib
  PNGdec
  )

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
  message("Enabling WiFi stuff...")
  target_sources(firmware PRIVATE
    src/dhcp.c
    src/syslog.c
    src/httpd.c
#    src/mqtt.c
    src/telnetd.c
#    src/snmp.c
    src/sshd.c
    src/ssh_util.c
#    ${WOLFSSL_ROOT}/wolfcrypt/test/test.c
    src/util_net.c
    )
  target_link_libraries(firmware PRIVATE
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_sntp
    pico_lwip_http
    pico_lwip_mqtt
    pico_lwip_snmp
    pico-telnetd-lib
    pico-sshd-lib
    wolfssl
    wolfssh
    )
  if (TLS_SUPPORT)
    target_link_libraries(firmware PRIVATE
      pico_lwip_mbedtls
      pico_mbedtls
      )
    # prevent warnings from mbedtls...
    set_source_files_properties(
      ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
      PROPERTIES
      COMPILE_OPTIONS "-Wno-unused-result"
      )
  endif()
endif()

target_compile_options(firmware PRIVATE -Wall)
#target_compile_options(firmware PRIVATE -fno-strict-aliasing -fwrapv -fno-aggressive-loop-optimizations)
#target_compile_options(firmware PRIVATE -fstack-protector-all)
target_compile_options(firmware PRIVATE -fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=)

target_compile_definitions(firmware PRIVATE USBD_MANUFACTURER="TJKO Industries")
target_compile_definitions(firmware PRIVATE USBD_PRODUCT="LCD-PICO-34401A Display Controller")
target_compile_definitions(firmware PRIVATE USBD_DESC_STR_MAX=40)
target_compile_definitions(firmware PRIVATE PARAM_ASSERTIONS_ENABLE_ALL=1)
target_compile_definitions(firmware PRIVATE PICO_MALLOC_PANIC=0)
target_compile_definitions(firmware PRIVATE PICO_DEBUG_MALLOC=0)
#target_compile_definitions(firmware PRIVATE PICO_DEOPTIMIZED_DEBUG=1)
target_compile_definitions(firmware PRIVATE PICO_STACK_SIZE=0x2000)
target_compile_definitions(firmware PRIVATE PICO_CORE1_STACK_SIZE=0x1000)
target_compile_definitions(firmware PRIVATE PICO_USE_STACK_GUARDS=1)
target_compile_definitions(firmware PRIVATE PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)
target_compile_definitions(firmware PRIVATE PICO_USE_FASTEST_SUPPORTED_CLOCK=1)
target_compile_definitions(firmware PRIVATE PICO_EMBED_XIP_SETUP=1)
#target_compile_definitions(bs2_default PRIVATE PICO_FLASH_SPI_CLKDIV=2)

target_compile_definitions(firmware PRIVATE
  LFS_THREADSAFE=1
  LFS_NO_DEBUG=1
#  LFS_YES_TRACE=1
#  I2C_DEBUG=1
   PNG_MAX_BUFFERED_PIXELS=8194
  )


if (${PICO_PLATFORM} MATCHES rp2350)
  pico_set_linker_script(firmware ${CMAKE_SOURCE_DIR}/src/memmap_custom_rp2350.ld)
else()
  pico_set_linker_script(firmware ${CMAKE_SOURCE_DIR}/src/memmap_custom_rp2040.ld)
endif()

# eof :-)
