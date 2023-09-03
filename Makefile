
FLAGS := -gdwarf-2 -mthumb -mcpu=cortex-m4 -ffunction-sections -fdata-sections -DNO_SYSTEMINIT -Wall -Wextra -O0
CFLAGS := ${FLAGS} -std=c99
CXXFLAGS := ${FLAGS} -std=c++20 -fno-rtti -fno-exceptions
LDFLAGS := -Wl,--gc-sections --specs=nosys.specs

.PHONY: all

all:
	arm-none-eabi-g++.exe ${CXXFLAGS} -Isrc/bsp/include -Isrc -c src/bsp/implementation/bsp_init.cpp -o build/bsp_init.o
	arm-none-eabi-g++.exe ${CXXFLAGS} -Isrc/bsp/include -Isrc -c src/bsp/implementation/bsp_leds.cpp -o build/bsp_leds.o
	arm-none-eabi-gcc.exe ${CFLAGS} -c startup.c -o build/startup.o
	arm-none-eabi-g++.exe ${CXXFLAGS} -c src\stm32f469-hello-world.cpp -o build/stm32f469-hello-world.o
	arm-none-eabi-g++.exe ${LDFLAGS} --static -nostartfiles -Tstm32-biggest.ld build/startup.o build/bsp_init.o build/bsp_leds.o build/stm32f469-hello-world.o -o build/hello-world.elf
