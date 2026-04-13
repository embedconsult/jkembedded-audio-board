cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(jkembedded_mikrobus_hat_mspm0_mux)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE ../../../../modules/hal/ti/mspm0/source/ti/driverlib/dl_i2c.c)
