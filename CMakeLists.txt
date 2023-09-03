cmake_minimum_required(VERSION 3.15)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(
    stm32f469i-disco-bsp
    VERSION 0.1
    DESCRIPTION "Board support package for a STM32F469I Discovery board"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

file(GLOB_RECURSE BSP_FILES REQUIRE_DEPENDS "src/*.hpp" "src/*.cpp")

message(NOTICE "List of all sources: ${BSP_FILES}")

add_library(stm32f469i-disco-bsp STATIC ${BSP_FILES})
set_target_properties(stm32f469i-disco-bsp PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(stm32f469i-disco-bsp PRIVATE "src/include" "src")

target_compile_options(stm32f469i-disco-bsp PRIVATE
                       $<$<COMPILE_LANG_AND_ID:CXX,GNU>:
                       -fno-rtti -fno-exceptions -std=c++20>)
target_compile_options(stm32f469i-disco-bsp PRIVATE
                        $<$<COMPILE_LANG_AND_ID:C,GNU>:
                        -std=c99>)

target_compile_options(stm32f469i-disco-bsp PRIVATE -gdwarf-2 -mthumb -mcpu=cortex-m4 -ffunction-sections -fdata-sections -DNO_SYSTEMINIT -Wall -Wextra -O0)