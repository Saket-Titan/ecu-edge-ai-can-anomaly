cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(anomaly_test)

target_include_directories(app PRIVATE
	../../../firmware/ids_node/src
)

target_sources(app PRIVATE
	../../../firmware/ids_node/src/manual_inference.c
	src/main.c
)

