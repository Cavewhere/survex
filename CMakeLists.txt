cmake_minimum_required(VERSION 3.10)

# set the project name
project(Survex VERSION 1.0)

# In this file we are doing all of our 'configure' checks. Things like checking
# for headers, functions, libraries, types and size of types.
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

# To check for an include file you do this:
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
check_include_file("string.h" HAVE_STRING_H)

# To check the size of a primitive type:
CHECK_TYPE_SIZE("int"   SIZEOF_INT)

check_symbol_exists("getc_unlocked" "stdio.h" HAVE_GETC_UNLOCKED)
check_symbol_exists("putc_unlocked" "stdio.h" HAVE_PUTC_UNLOCKED)

set(PACKAGE survex)
set(VERSION 1.0)
set(PRETTYPACKAGE Survex-1.0)
set(COMMAVERSION 1,0,0)
set(SOURCE_DIR src)

configure_file ("${PROJECT_SOURCE_DIR}/src/msvc/config.h.in"
                "${PROJECT_BINARY_DIR}/config.h" )

include_directories(${PROJECT_BINARY_DIR})

# CONAN download proj
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES proj/6.3.1
                BASIC_SETUP)

add_custom_target(thgeomag ALL
    COMMAND python ${PROJECT_SOURCE_DIR}/${SOURCE_DIR}/igrf2c.py ${PROJECT_SOURCE_DIR}/${SOURCE_DIR}/igrf12coeffs.txt
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/../thgeomagdata.h ${PROJECT_BINARY_DIR}/thgeomagdata.h
    )

# add the executable
add_executable(cavern
    ${SOURCE_DIR}/cavern.c
    ${SOURCE_DIR}/date.c
    ${SOURCE_DIR}/listpos.c
    ${SOURCE_DIR}/commands.c
    ${SOURCE_DIR}/datain.c
    ${SOURCE_DIR}/netskel.c
    ${SOURCE_DIR}/network.c
    ${SOURCE_DIR}/readval.c
    ${SOURCE_DIR}/matrix.c
    ${SOURCE_DIR}/img_hosted.c
    ${SOURCE_DIR}/netbits.c
    ${SOURCE_DIR}/useful.c
    ${SOURCE_DIR}/validate.c
    ${SOURCE_DIR}/netartic.c
    ${SOURCE_DIR}/thgeomag.c

    #Shared sources?
    ${SOURCE_DIR}/message.c
    ${SOURCE_DIR}/cmdline.c
    ${SOURCE_DIR}/osdepend.c
    ${SOURCE_DIR}/filename.c
    ${SOURCE_DIR}/str.c
    ${SOURCE_DIR}/useful.c
    ${SOURCE_DIR}/getopt.c
    ${SOURCE_DIR}/getopt1.c
    )
add_dependencies(cavern thgeomag)

target_include_directories(cavern PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(cavern ${CONAN_LIBS})
target_compile_definitions(cavern PUBLIC
#    ACCEPT_USE_OF_DEPRECATED_PROJ_API_H #Because we use deperecated proj api
    HAVE_CONFIG_H #Yea, cavern doesn't build without it
    )
