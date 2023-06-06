# Template CMakeLists.txt configuration for STM32 MCUS
# Following ideas and concepts from Matej Blagšič
# https://youtu.be/9tWRdnyWGb8
cmake_minimum_required(VERSION 3.24)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include (gcc-arm-none-eabi)
# ##############################################################################
# Edit your MCU information up to the next border
project(LivSynth)
set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/App)

set(MCU_FAMILY STM32F4xx)
set(MCU_MODEL STM32F405xx)
set(CPU_PARAMETERS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32f405rgtx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F405RGTX_FLASH.ld)
# ##############################################################################
set(EXECUTABLE ${CMAKE_PROJECT_NAME})
enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Headers
set(CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/App
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Inc)
set(PROJECT_INCLUDE_DIRECTORIES
    #${CMAKE_CURRENT_SOURCE_DIR}
    ${APP_DIR}
    ${APP_DIR}/Utils
    ${APP_DIR}/System
    ${APP_DIR}/Middleware
    ${APP_DIR}/Model
    ${APP_DIR}/Engine
    ${APP_DIR}/UI
)

# Sources
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/App/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/USB_DEVICE/Target/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CompositeBuilder/Src/*.c)

file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
    ${APP_DIR}/*.cpp
    ${APP_DIR}/*.c)

# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES} 
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT})

# Embedded macros(defines)
target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    USE_FULL_LL_DRIVER
    #USE_HAL_DRIVER
    HSE_VALUE=16000000
    )

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${PROJECT_INCLUDE_DIRECTORIES})

# Compiler and linker options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    -Wall # enable all warnings
    -Wextra # enable extra warnings
    -Wpedantic # enable pedantic warnings
    -Wno-unused-parameter # disable unused parameter warnings
    $<$<COMPILE_LANGUAGE:CXX>: # only for CXX compilation
        -Wno-volatile # 
        #-Wold-style-cast # Deactivated, because there are lots of useless casts in LL lib
        #-Wuseless-cast # Deactivated, because there are lots of useless casts in LL lib
        -Wno-deprecated-enum-enum-conversion
        -Wsuggest-override> # 
    $<$<CONFIG:Debug>:-Og -g3 -ggdb> # optimize debugging experience, produce debugging information on level 3 for gdb
    $<$<CONFIG:Release>:-O2>) # optimize debug experience (TODO: change that!), no debug information

#--specs=nosys.specs
target_link_options(${EXECUTABLE} PRIVATE
    -T${MCU_LINKER_SCRIPT} # define linker script
    ${CPU_PARAMETERS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map # generate map file
    -Wl,--start-group
    -lc # add C libraries
    -lm # add math libraries
    -lstdc++ # add standard C++ libraries
    -lsupc++ # add support C++ libraries
    -u _printf_float # enable float formatting in printf
    -Wl,--end-group
    -Wl,--gc-sections # garbage collector for sections so the linker can choose which sections to use after compilation (omits unused stuff, optimized code size)
    # -Wl,--print-gc-sections
    -Wl,--no-warn-rwx-segments
    -Wl,--print-memory-usage) # print memory usage after linking

# Disable overflow warnings from STM32Cube generated code of LL drivers
set_source_files_properties(${STM32CUBEMX_SOURCES} PROPERTIES
    COMPILE_FLAGS -Wno-overflow)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.bin)

# OpenOCD configuration
find_program (OPEN_OCD openocd DOC "openocd executable")
set (OPENOCD_INTERFACE stlink)
set (OPENOCD_TARGET stm32f4x)

# functions
function (add_openocd_targets elf_file)
    add_custom_target(flash
        COMMAND ${OPEN_OCD}
            #-f board/${OPENOCD_BOARD}.cfg
            -f interface/${OPENOCD_INTERFACE}.cfg
            -f target/${OPENOCD_TARGET}.cfg
            -c "program ${EXECUTABLE}.elf verify reset exit"
            VERBATIM
        DEPENDS ${elf_file}
    )
endfunction (add_openocd_targets)

add_openocd_targets(${PROJECT_NAME})
