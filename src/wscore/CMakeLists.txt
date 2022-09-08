cmake_minimum_required(VERSION 3.21)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
include(C:/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(ws_drv)

set(CMAKE_CXX_STANDARD 17)
set(TARGET webcore)

option(CASE_INSENSITIVE "Case insensitive method names" OFF)
if (WIN32)
    option(STATIC_CRT "Static CRT linkage" ON)
else()
    option(STATIC_CRT "Static CRT linkage" OFF)
endif ()

option(OUT_PARAMS "Support output parameters" OFF)


if (WIN32)
    #add_definitions(-DBOOST_ALL_NO_LIB)
    set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
    set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
    set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
    set(Boost_USE_MULTITHREADED      ON)
    set(Boost_USE_STATIC_RUNTIME    ON)


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

endif()

if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

list(APPEND SOURCES
        src/addin.def
        include/stdafx.h
        src/dllmain.cpp
        src/exports.cpp
        src/Component.cpp
        include/Component.h
        src/webcore.cpp
        include/webcore.h
        ../chared/server_response.h
        ../chared/server_response.cpp)



if (WIN32)
    add_library(${TARGET} SHARED
            ${SOURCES})
else()
    file(GLOB ARCIRK_SOURCES ../arcirk/src/*.cpp)
    file(GLOB WDC_SOURCES ../wdc/src/*.cpp ../wdc/src/*.hpp)
    file(GLOB PUGI_SOURCES ../pugixml/*.cpp)
    file(GLOB WS_CLIENT_SOURCES ../ws_client/src/*.cpp)
    add_library(${TARGET} SHARED
            ${SOURCES}
            ${ARCIRK_SOURCES}
            ${WDC_SOURCES}
            ${PUGI_SOURCES}
            ${WS_CLIENT_SOURCES})
endif ()


set(ARCIRK_INCLUDE ../arcirk/include)
set(ARCIRK_LIB ../arcirk/cmake-build-debug)
set(WDC_LIB C:/src/ws/ws_solution/wdc-Release)
set(WDC_INCLUDE ../wdc/include)
set(PUGI_INCLUDE ../pugixml)
set(WS_CLIENT_INCLUDE ../ws_client/include)
set(WS_CLIENT_LIB ../ws_client/cmake-build-debug)
set(CHARED_INCLUDE ../chared)

target_compile_definitions(${TARGET} PRIVATE UNICODE _UNICODE)
#add_definitions(-DWS_CLIENT_STATIC)

target_include_directories(${TARGET} PRIVATE
        include
        ${ARCIRK_INCLUDE}
        ${PUGI_INCLUDE}
        ${WDC_INCLUDE}
        ${WS_CLIENT_INCLUDE}
        ${CHARED_INCLUDE})

set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (EXECUTABLE_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
set (CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
#if (NOT TARGET_PLATFORM_32)
#    set(TARGET_SUFFIX "_x64")
#else()
#    set(TARGET_SUFFIX "")
#endif()
if (${CMAKE_SIZEOF_VOID_P} STREQUAL 4)
    message("It's 32 bit build")
    set(TARGET_SUFFIX "")
elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
    message("It's 64 bit build")
    set(TARGET_SUFFIX "_x64")
endif ()
set_target_properties( ${TARGET} PROPERTIES
        OUTPUT_NAME ${TARGET}${TARGET_SUFFIX}
        )

if (UNIX)
#    if (TARGET_PLATFORM_32)
#        set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
#        set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
#    else(TARGET_PLATFORM_32)
#        set(CMAKE_C_FLAGS "-m64 ${CMAKE_C_FLAGS}")
#        set(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")
#    endif ()
#    target_link_libraries(${PROJECT_NAME} -static-libstdc++)
else(UNIX)
    add_definitions(/MT)
    set(CMAKE_SUPPRESS_REGENERATION 1)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
    target_compile_definitions(${TARGET} PRIVATE _WINDOWS
            _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    target_compile_options(${TARGET} PRIVATE /utf-8)
endif(UNIX)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
    link_directories(
            ${ARCIRK_LIB}
            ${WDC_LIB}
            ${WS_CLIENT_LIB})
endif ()

#if (WIN32 AND __COMPILER_GNU)
    # mingw-gcc fails to link boost::thread
#    add_definitions(-DBOOST_THREAD_USE_LIB)
#endif (WIN32 AND __COMPILER_GNU)
#find_package(Threads REQUIRED)

if (WIN32)
    find_package(Boost REQUIRED COMPONENTS regex thread filesystem system locale)
    target_link_libraries(${TARGET} PRIVATE
            bcrypt.lib
            libwdc
            arcirk
            ws_client
            Boost::thread
            Boost::locale
            )

    include_directories (
            ${Boost_INCLUDE_DIRS}
            ${ARCIRK_INCLUDE}
            ${WDC_INCLUDE}
            ${WS_CLIENT_INCLUDE}
            ${CHARED_INCLUDE}
    )
    ADD_CUSTOM_COMMAND(
            TARGET ${TARGET}
            POST_BUILD
            COMMENT "Copying to share..."
            COMMAND ${CMAKE_COMMAND} -E copy ${LIBRARY_OUTPUT_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}
            /script/lib)

else()
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    find_package(Boost COMPONENTS regex filesystem system thread REQUIRED)
    include_directories (
            ${Boost_INCLUDE_DIRS}
            ${ARCIRK_INCLUDE}
            ${PUGI_INCLUDE}
            ${WDC_INCLUDE}
            ${WS_CLIENT_INCLUDE}
            ${CHARED_INCLUDE}
    )

    target_link_libraries(${TARGET} PRIVATE
            Boost::filesystem
            Boost::system
            Boost::thread
            OpenSSL::SSL
            OpenSSL::Crypto
            ${CURL_LIBRARIES}
            -static-libstdc++)
endif()

