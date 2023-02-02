cmake_minimum_required(VERSION 3.22)
project(arcirk_solution)

add_subdirectory("${PROJECT_SOURCE_DIR}/ws_addin" "${PROJECT_SOURCE_DIR}/ws_addin/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/client/console" "${PROJECT_SOURCE_DIR}/client/console/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/server" "${PROJECT_SOURCE_DIR}/server/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/arcirk" "${PROJECT_SOURCE_DIR}/arcirk/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/client" "${PROJECT_SOURCE_DIR}/client/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/http_client" "${PROJECT_SOURCE_DIR}/http_client/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/http_client/console" "${PROJECT_SOURCE_DIR}/http_client/console/cmake-build-debug")

