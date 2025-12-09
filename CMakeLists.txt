cmake_minimum_required(VERSION 3.22.1)

project(libev++)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(libev REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
SYSTEM
INTERFACE ${libev_INCLUDE_DIRS}
          $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
          $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME} INTERFACE ${libev_LIBRARIES})

install(
    DIRECTORY include/
    DESTINATION include
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)

install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/${PROJECT_NAME}Config.cmake.in cmake/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)
