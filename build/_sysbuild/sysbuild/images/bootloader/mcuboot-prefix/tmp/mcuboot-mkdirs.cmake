# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/saket-jha/zephyrproject/bootloader/mcuboot/boot/zephyr"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/mcuboot"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/tmp"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/src/mcuboot-stamp"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/src"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/src/mcuboot-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/src/mcuboot-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/bootloader/mcuboot-prefix/src/mcuboot-stamp${cfgdir}") # cfgdir has leading slash
endif()
