cmake_minimum_required(VERSION 3.22)
project(arcirk_solution)

add_subdirectory("${PROJECT_SOURCE_DIR}/server" "${PROJECT_SOURCE_DIR}/server/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/arcirk" "${PROJECT_SOURCE_DIR}/arcirk/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/client" "${PROJECT_SOURCE_DIR}/client/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/wsClient" "${PROJECT_SOURCE_DIR}/wsClient/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/client/console" "${PROJECT_SOURCE_DIR}/client/console/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/server/tests" "${PROJECT_SOURCE_DIR}/server/tests/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/client/tests" "${PROJECT_SOURCE_DIR}/client/tests/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/server_new" "${PROJECT_SOURCE_DIR}/server_new/cmake-build-debug")

#set(JSON_BuildTests OFF CACHE INTERNAL "")
#add_subdirectory(${PROJECT_SOURCE_DIR}/json)