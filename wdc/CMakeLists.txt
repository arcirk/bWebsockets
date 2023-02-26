cmake_minimum_required(VERSION 3.21)
set(WDC_VERSION_MAJOR 1)
set(WDC_VERSION_MINOR 1)
set(WDC_VERSION_PATCH 5)
set(WDC_VERSION_TWEAK 1)
set(WDC_VERSION ${WDC_VERSION_MAJOR}.${WDC_VERSION_MINOR}.${WDC_VERSION_PATCH})

project(WDC VERSION ${WDC_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
        add_definitions(/MT)

        add_definitions("-DCURL_STATICLIB")
        set(CURL_LIBRARY "-lcurl")

        set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
        set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
        set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
        set(Boost_USE_MULTITHREADED      ON)
        set(Boost_USE_STATIC_RUNTIME    ON)
endif()

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
if (WIN32)
        find_package(CURL CONFIG REQUIRED)
else()
        find_package(CURL REQUIRED)
endif ()


include_directories(
        ../pugixml
        include
        )

file(GLOB WDC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(libwdc
        ${${PROJECT_NAME}_SOURCES}
        ../pugixml/pugixml.cpp
        ../pugixml/pugiconfig.hpp
        ../pugixml/pugixml.hpp
        )
set_target_properties(libwdc PROPERTIES PREFIX "")
set_target_properties(libwdc PROPERTIES IMPORT_PREFIX "")

target_link_libraries(libwdc PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
        ${Boost_LIBRARIES}
        )
