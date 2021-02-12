cmake_minimum_required(VERSION 3.12)

# set the project name
project(Survex VERSION 1.0)

# In this file we are doing all of our 'configure' checks. Things like checking
# for headers, functions, libraries, types and size of types.
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
include (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
include (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
include (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

# To check for an include file you do this:
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("string.h" HAVE_STRING_H)

# To check the size of a primitive type:
check_type_size("int"   SIZEOF_INT)

check_symbol_exists("getc_unlocked" "stdio.h" HAVE_GETC_UNLOCKED)
check_symbol_exists("putc_unlocked" "stdio.h" HAVE_PUTC_UNLOCKED)
check_function_exists("strcasecmp" HAVE_STRCASECMP)
check_function_exists("_stricmp" HAVE_STRICMP)

set(PACKAGE survex)
set(PACKAGE_STRING survex)
set(VERSION 1.0)
set(PRETTYPACKAGE Survex-1.0)
set(COMMAVERSION 1,0,0)
set(SRC_DIR src)
set(SRC_LIB_DIR lib)
set(COPYRIGHT_MSG copyright-undefined)
set(AVEN_COPYRIGHT_MSG copyright-message-undefined)
set(COPYRIGHT_MSG_UTF8 copyright-message_undefined)
set(AVEN_COPYRIGHT_MSG_UTF8 copyright-message_undefined)

include_directories(${PROJECT_BINARY_DIR})

# CONAN download proj
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_add_remote(NAME bincrafters
                 INDEX 1
                 URL https://api.bintray.com/conan/bincrafters/public-conan
                 VERIFY_SSL True)

conan_cmake_run(REQUIRES proj/6.3.1
                REQUIRES wxwidgets/3.1.4@bincrafters/stable
                REQUIRES glew/2.1.0@bincrafters/stable
                OPTIONS wxwidgets:webview=False
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

find_package (Python3 COMPONENTS Interpreter)
find_package (Perl)

SET(CMAKE_REQUIRED_INCLUDES ${CONAN_INCLUDE_DIRS})
check_include_file("proj.h" HAVE_PROJ_H)

SET(CMAKE_EXTRA_INCLUDE_FILES "wx/chartype.h")
check_type_size("wxChar" SIZEOF_WXCHAR)
SET(CMAKE_EXTRA_INCLUDE_FILES)

configure_file ("${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h" )

if(NOT ${Python3_FOUND})
    message( FATAL_ERROR "Missing python3 please install it." )
endif()

if(NOT ${PERL_FOUND})
    message( FATAL_ERROR "Missing perl please install it.")
endif()

execute_process(COMMAND ${PERL_EXECUTABLE} -MLocale::PO -e 1
    ERROR_QUIET RESULT_VARIABLE missing_perl_localPO)

if(${missing_perl_localPO})
    message( FATAL_ERROR "Missing perl package \"Locale::PO\". Please run \"cpan Locale::PO\" to install it" )
endif()

set(MESSAGE_OUTPUTS
    ${PROJECT_BINARY_DIR}/bin/en.msg
    ${PROJECT_BINARY_DIR}/bin/en_US.msg
)

include(messages.cmake)
svx_add_messages(messageFiles
    ${SRC_LIB_DIR}/bg.po
    ${SRC_LIB_DIR}/de_CH.po
    ${SRC_LIB_DIR}/el.po
    ${SRC_LIB_DIR}/es.po
    ${SRC_LIB_DIR}/hu.po
    ${SRC_LIB_DIR}/it.po
    ${SRC_LIB_DIR}/pt_BR.po
    ${SRC_LIB_DIR}/ro.po
    ${SRC_LIB_DIR}/sk.po
    ${SRC_LIB_DIR}/ca.po
    ${SRC_LIB_DIR}/de.po
    ${SRC_LIB_DIR}/en_US.po
    ${SRC_LIB_DIR}/fr.po
    ${SRC_LIB_DIR}/id.po
    ${SRC_LIB_DIR}/pl.po
    ${SRC_LIB_DIR}/pt.po
    ${SRC_LIB_DIR}/ru.po
    ${SRC_LIB_DIR}/zh_CN.po
    )

add_custom_command(
    OUTPUT
    ${PROJECT_BINARY_DIR}/thgeomagdata.h

    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_DIR}/igrf2c.py ${PROJECT_SOURCE_DIR}/${SRC_DIR}/igrf13coeffs.txt
    COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/../thgeomagdata.h ${PROJECT_BINARY_DIR}/thgeomagdata.h

    DEPENDS
    ${PROJECT_SOURCE_DIR}/${SRC_DIR}/igrf2c.py
    ${PROJECT_SOURCE_DIR}/${SRC_DIR}/igrf13coeffs.txt
    )
add_custom_target(thgeomagdata DEPENDS ${PROJECT_BINARY_DIR}/thgeomagdata.h)

add_library(survex_utils STATIC
    ${SRC_DIR}/message.c
    ${SRC_DIR}/cmdline.c
    ${SRC_DIR}/osdepend.c
    ${SRC_DIR}/filename.c
    ${SRC_DIR}/str.c
    ${SRC_DIR}/useful.c
    ${SRC_DIR}/getopt.c
    ${SRC_DIR}/getopt1.c
    ${SRC_DIR}/img_hosted.c
    ${SRC_DIR}/hash.c
)

target_compile_definitions(survex_utils PUBLIC
    HAVE_CONFIG_H
    )
target_link_libraries(survex_utils CONAN_PKG::proj)

# add the executable
add_executable(cavern
    ${SRC_DIR}/cavern.c
    ${SRC_DIR}/date.c
    ${SRC_DIR}/listpos.c
    ${SRC_DIR}/commands.c
    ${SRC_DIR}/datain.c
    ${SRC_DIR}/netskel.c
    ${SRC_DIR}/network.c
    ${SRC_DIR}/readval.c
    ${SRC_DIR}/matrix.c
    ${SRC_DIR}/netbits.c
    ${SRC_DIR}/useful.c
    ${SRC_DIR}/validate.c
    ${SRC_DIR}/netartic.c
    ${SRC_DIR}/thgeomag.c
)

add_dependencies(cavern
    messageFiles
    thgeomagdata
    )

target_include_directories(cavern PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(cavern CONAN_PKG::proj survex_utils)
target_compile_definitions(cavern PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )

add_executable(dump3d
    ${SRC_DIR}/dump3d.c
    ${SRC_DIR}/date.c
    ${SRC_DIR}/useful.c
    )
target_include_directories(dump3d PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(dump3d survex_utils)
target_compile_definitions(dump3d PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )

add_executable(diffpos
    ${SRC_DIR}/diffpos.c
    ${SRC_DIR}/namecmp.c
    ${SRC_DIR}/hash.c
    )
target_include_directories(diffpos PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(diffpos survex_utils)
target_compile_definitions(diffpos PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )

add_executable(sorterr
    ${SRC_DIR}/sorterr.c
    )
target_include_directories(sorterr PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(sorterr survex_utils)
target_compile_definitions(sorterr PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )

add_executable(extend
    ${SRC_DIR}/extend.c
    )
target_include_directories(extend PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(extend survex_utils)
target_compile_definitions(extend PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )


add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/img2aven.h
    COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gen_img2aven > img2aven.h
    DEPENDS ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gen_img2aven
    )
add_custom_target(img2aven DEPENDS ${PROJECT_BINARY_DIR}/img2aven.h)

add_executable(survexport
    ${SRC_DIR}/survexport.cc
    ${SRC_DIR}/model.cc
    ${SRC_DIR}/export.cc
    ${SRC_DIR}/namecompare.cc
    ${SRC_DIR}/gpx.cc
    ${SRC_DIR}/hpgl.cc
    ${SRC_DIR}/json.cc
    ${SRC_DIR}/kml.cc
    ${SRC_DIR}/pos.cc
    ${SRC_DIR}/vector3.cc
    )
target_include_directories(survexport PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(survexport
    CONAN_PKG::wxwidgets
    CONAN_PKG::glew
    survex_utils)
target_compile_definitions(survexport PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )
add_dependencies(survexport img2aven)

set(FONT_FILES
    ${PROJECT_BINARY_DIR}/unifont.hex
    ${PROJECT_BINARY_DIR}/unifont.pixelfont
    ${PROJECT_BINARY_DIR}/preload_font.h
    )

add_custom_command(
    OUTPUT ${FONT_FILES}

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/unifont.hex ${PROJECT_BINARY_DIR}/unifont.hex
    COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/make-pixel-font

    DEPENDS
    ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/make-pixel-font ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/unifont.hex
    )
add_custom_target(make-pixel-font DEPENDS ${FONT_FILES})

add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/avenpal.h

    COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gdtconvert < ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gradient.pov > avenpal.h

    DEPENDS
    ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gradient.pov
    ${PROJECT_SOURCE_DIR}/${SRC_DIR}/gdtconvert
    )
add_custom_target(gdtconvert DEPENDS ${PROJECT_BINARY_DIR}/avenpal.h)

add_executable(aven WIN32
    ${SRC_DIR}/aven.cc
    ${SRC_DIR}/gfxcore.cc
    ${SRC_DIR}/mainfrm.cc
    ${SRC_DIR}/model.cc
    ${SRC_DIR}/vector3.cc
    ${SRC_DIR}/aboutdlg.cc
    ${SRC_DIR}/namecompare.cc
    ${SRC_DIR}/aventreectrl.cc
    ${SRC_DIR}/export.cc
    ${SRC_DIR}/guicontrol.cc
    ${SRC_DIR}/gla-gl.cc
    ${SRC_DIR}/glbitmapfont.cc
    ${SRC_DIR}/gpx.cc
    ${SRC_DIR}/json.cc
    ${SRC_DIR}/kml.cc
    ${SRC_DIR}/log.cc
    ${SRC_DIR}/moviemaker.cc
    ${SRC_DIR}/hpgl.cc
    ${SRC_DIR}/cavernlog.cc
    ${SRC_DIR}/avenprcore.cc
    ${SRC_DIR}/printing.cc
    ${SRC_DIR}/buttontaghandler.cc
    ${SRC_DIR}/pos.cc
    ${SRC_DIR}/date.c
    ${SRC_DIR}/img_hosted.c
    ${SRC_DIR}/useful.c
    ${SRC_DIR}/hash.c
)

add_dependencies(aven
    make-pixel-font
    gdtconvert
    messageFiles
    img2aven
    )

target_include_directories(aven PUBLIC ${PROJECT_SOURCE_DIR}/${SRC_DIR})
target_link_libraries(aven
    CONAN_PKG::wxwidgets
    CONAN_PKG::glew
    CONAN_PKG::proj
    survex_utils)
target_compile_definitions(aven PUBLIC
    HAVE_CONFIG_H
    _CRT_SECURE_NO_WARNINGS
    )
