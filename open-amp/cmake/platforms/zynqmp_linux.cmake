set (CMAKE_SYSTEM_PROCESSOR "arm64")
set (CROSS_PREFIX           "aarch64-linux-gnu-")
set (MACHINE                "zynqmp" CACHE STRING "")

include (cross_linux_gcc)
