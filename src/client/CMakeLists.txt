cmake_minimum_required(VERSION 3.19)
project(bclient VERSION 1.0.1 DESCRIPTION "arcirk websocket client")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
#set(CMAKE_AUTOMOC ON)

set(TARGET bclient)
if(MSVC)
    add_compile_options(
            $<$<CONFIG:>:/MT> #---------|
            $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
            $<$<CONFIG:Release>:/MT> #--|
    )
endif()

if (UNIX)
    SET(CMAKE_CXX_FLAGS -pthread)
endif()

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

if (WIN32)
    set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
    set(Boost_USE_DEBUG_LIBS        ON)  # ignore debug libs and
    set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME    ON)
    #        set (Boost_USE_MULTITHREADED ON)
    #
    foreach (flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        if (${flag_var} MATCHES "/MD")
            string (REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif (${flag_var} MATCHES "/MD")

    endforeach(flag_var)

    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
    set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRTD")
endif ()

add_definitions(-DWS_CLIENT_LIBRARY)

set(ARCIRK_LIB ../arcirk/cmake-build-debug)
set(ARCIRK_INCLUDE ../arcirk/include)
set(CHARED_INCLUDE ../chared)

list(APPEND SOURCES
        include/beast.h
        include/net.h
        include/ws_session.h
        include/ws_client.h
        src/ws_session.cpp
        src/ws_client.cpp
        src/iws_client.cpp
        include/iws_client.h
        include/stdafx.h
        ../chared/server_response.h
        ../chared/server_response.cpp
        )
if (WS_CLIENT_STATIC)
    add_library(${TARGET}
            ${SOURCES})
else()
    file(GLOB ARCIRK_SOURCES ../arcirk/src/*.cpp)
    add_library(${TARGET} SHARED
            ${SOURCES}
            ${ARCIRK_SOURCES})
    set_target_properties(ws_client PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION 1
            PUBLIC_HEADER include/iws_client.h)
endif ()

if (WIN32)
    add_definitions(/MT)
    set(CMAKE_SUPPRESS_REGENERATION 1)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    target_compile_definitions(${TARGET} PRIVATE _WINDOWS
            _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    target_compile_options(${TARGET} PRIVATE /utf-8)
endif ()

            ##SHARED
            #add_library(ws_client SHARED
            #        include/beast.h
            #        include/net.h
            #        include/ws_session.h
            #        include/ws_client.h
            #        src/ws_session.cpp
            #        src/ws_client.cpp
            #        src/iws_client.cpp
            #        include/iws_client.h
            #        ../arcirk/src/arcirk.cpp
            #        ../arcirk/src/arcirk_static.cpp
            #        ../arcirk/src/arcirk_json.cpp
            #        ../arcirk/src/arcirk_ip.cpp
            #        ../arcirk/src/arcirk_message.cpp
            #        ../arcirk/src/arcirk_settings.cpp
            #        include/stdafx.h)
            #
            #set_target_properties(ws_client PROPERTIES
            #        VERSION ${PROJECT_VERSION}
            #        SOVERSION 1
            #        PUBLIC_HEADER include/iws_client.h)
if (WIN32)
    link_directories(
            ${ARCIRK_LIB})
endif ()

if (WIN32)
    find_package(Boost REQUIRED COMPONENTS regex thread)
    target_link_libraries(${TARGET} PRIVATE
            bcrypt.lib
            arcirk
            Boost::thread)

    include_directories (
            ${Boost_INCLUDE_DIRS}
            ${ARCIRK_INCLUDE}
            ${CHARED_INCLUDE}
    )

else()
    SET(CMAKE_CXX_FLAGS -pthread)
    find_package(Boost COMPONENTS regex filesystem system thread REQUIRED)
    include_directories (
            ${Boost_INCLUDE_DIRS}
            ${ARCIRK_INCLUDE}
            ${CHARED_INCLUDE}
    )

    target_link_libraries(${TARGET} PRIVATE
            Boost::filesystem
            Boost::system
            Boost::thread
            -static-libstdc++)
endif()

#if (UNIX)
#    SET(CMAKE_CXX_FLAGS -pthread)
#endif ()
#
#if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#    set (NMOS_CPP_SYSTEM_LIBS bcrypt.winreg)
#else()
#    set (NMOS_CPP_SYSTEM_LIBS )
#endif()
#if (UNIX)
#find_package(Boost REQUIRED COMPONENTS thread regex filesystem)
#else()
#find_package(Boost REQUIRED COMPONENTS regex)
#endif()
#
#include_directories(
#        "../arcirk/include"
#        ${Boost_INCLUDE_DIRS})

#if (UNIX)
#target_link_libraries(ws_client
#        ${Boost_LIBRARIES}
#        Boost::thread
#        Boost::filesystem
#        ${NMOS_CPP_SYSTEM_LIBS}
#        )
#else()
#    target_link_libraries(ws_client  PRIVATE
#        ${Boost_LIBRARIES}
#        ${NMOS_CPP_SYSTEM_LIBS}
#        )
#endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wunreachable-code -Wpedantic)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET} PRIVATE -Wweak-vtables -Wexit-time-destructors -Wglobal-constructors -Wmissing-noreturn )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${TARGET} PRIVATE /W4 /w44265 /w44061 /w44062 )
endif()

if(WIN32)
#[[    ADD_CUSTOM_COMMAND(
            TARGET ${TARGET}
            POST_BUILD
            COMMENT "Update winreg..."
            COMMAND ${CMAKE_COMMAND} -E rm ../../ws_admin_qt/ws_admin_qt/cmake-build-debug/${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX})


    ADD_CUSTOM_COMMAND(
            TARGET ${TARGET}
            POST_BUILD
            COMMENT "Copying winreg..."
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug/${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}
            ../../ws_admin_qt/ws_admin_qt/cmake-build-debug
    )]]
endif ()

#install(TARGETS ${TARGET} EXPORT ${TARGET}_export)
#export(EXPORT ${TARGET}_export FILE cmake/${TARGET}-targets.cmake)