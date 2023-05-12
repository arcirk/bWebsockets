cmake_minimum_required(VERSION 3.20)
project(arcirk)

#if (MSVC)
#    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
#    include(C:/lib/vcpkg/scripts/buildsystems/vcpkg.cmake)
#else()
#    set(CMAKE_TOOLCHAIN_FILE "/home/arcady/lib/vcpkg/scripts/buildsystems/vcpkg.cmake"
#            CACHE STRING "Vcpkg toolchain file")
#endif()

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

if(MSVC)
    add_compile_options(
            $<$<CONFIG:>:/MT> #---------|
            $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
            $<$<CONFIG:Release>:/MT> #--|
    )
endif()

if (WIN32)
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

add_definitions (-DBOOST_ALL_STATIC_LINK=1)

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

file(GLOB CPPS "src/*.cpp")

add_library(arcirk
        arcirk.hpp
        ${CPPS}
        )

find_package(Boost REQUIRED COMPONENTS regex filesystem)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(arcirk PRIVATE
        ${Boost_LIBRARIES}
        )