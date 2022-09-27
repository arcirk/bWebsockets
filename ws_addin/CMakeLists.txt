cmake_minimum_required(VERSION 3.21)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
include(C:/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(ws_addin)

set(CMAKE_CXX_STANDARD 17)
set(TARGET ws_addin)

if(MSVC)
    add_compile_options(
            $<$<CONFIG:>:/MT> #---------|
            $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
            $<$<CONFIG:Release>:/MT> #--|
    )
endif()

option(CASE_INSENSITIVE "Case insensitive method names" OFF)
if (WIN32)
    option(STATIC_CRT "Static CRT linkage" ON)
else()
    option(STATIC_CRT "Static CRT linkage" OFF)
endif ()

option(OUT_PARAMS "Support output parameters" OFF)


if (WIN32)
    set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
    set(Boost_USE_DEBUG_LIBS        ON)  # ignore debug libs and
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

add_definitions (-DBOOST_ALL_STATIC_LINK=1)

if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()
if (MSVC)
    set (CMAKE_VERBOSE_MAKEFILE FALSE)

    add_definitions (
            -D_WIN32_WINNT=0x0601
            -D_SCL_SECURE_NO_WARNINGS=1
            -D_CRT_SECURE_NO_WARNINGS=1
            -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
            -D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
    )

    add_compile_options(
            /bigobj       # large object file format
            /permissive-  # strict C++
            #/wd4503      # decorated name length exceeded, name was truncated
            /W4           # enable all warnings
            /MP           # Multi-processor compilation
    )

    set (Boost_USE_STATIC_RUNTIME ON)

    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob2 /Oi /Ot /GL /MT")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi /Ot /MT")

    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

    # for RelWithDebInfo builds, disable incremental linking
    # since CMake sets it ON by default for that build type and it
    # causes warnings
    #
    string (REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" replacement_flags
            ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${replacement_flags})

else()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    find_package (Threads)
    link_libraries(Threads::Threads)


    set( CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wno-unused-parameter")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis")
    endif ()
endif()

set(ARCIRK_INCLUDE ../arcirk)
set(ARCIRK_LIB ../arcirk/cmake-build-debug)
set(BCLIENT_INCLUDE ../client/include)
set(BCLIENT_LIB ../client/cmake-build-debug)

file(GLOB CPPS "src/*.cpp"
        CPPH "include/*h")

add_library(${TARGET} SHARED
        ${CPPS}
        ${CPPH}
        ../../arcirk/beast.hpp
        ../../arcirk/net.hpp
        )

find_package(Boost COMPONENTS thread regex filesystem system locale REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(${TARGET} PRIVATE
        Boost::filesystem
        Boost::system
        Boost::thread
        Boost::locale
        OpenSSL::SSL
        OpenSSL::Crypto
        arcirk
        bclient
        )


target_compile_definitions(${TARGET} PRIVATE UNICODE _UNICODE)

target_include_directories(${TARGET} PRIVATE
        include
        ${Boost_INCLUDE_DIRS}
        ${ARCIRK_INCLUDE}
        ${BCLIENT_INCLUDE}
        )

set (LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (EXECUTABLE_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
set (CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})

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
    if (TARGET_PLATFORM_32)
        set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
    else(TARGET_PLATFORM_32)
        set(CMAKE_C_FLAGS "-m64 ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")
    endif ()
    target_link_libraries(${PROJECT_NAME} -static-libstdc++)
else(UNIX)
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

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE
            bcrypt.lib)
endif()