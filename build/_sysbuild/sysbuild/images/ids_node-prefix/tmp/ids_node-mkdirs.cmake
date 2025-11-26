# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/firmware/ids_node"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/ids_node"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/tmp"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/src/ids_node-stamp"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/src"
  "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/src/ids_node-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/src/ids_node-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/saket-jha/zephyrproject/ecu-edge-ai-can-anomaly/build/_sysbuild/sysbuild/images/ids_node-prefix/src/ids_node-stamp${cfgdir}") # cfgdir has leading slash
endif()
