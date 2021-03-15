cmake_minimum_required(VERSION 3.5)

project(PLUGINS_LIB)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(ranges HAS_RANGES)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib_structures lib_structures)

add_library(${PROJECT_NAME} STATIC)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/plugins PLUGIN_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/app APP_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/base BASE_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/chat CHAT_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/play PLAY_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/stream STREAM_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/user USER_SRC)

target_link_libraries(${PROJECT_NAME} PUBLIC STRUCTURES_LIB)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(${PROJECT_NAME} PUBLIC
        ${PLUGIN_SRC}
        ${APP_SRC}
        ${BASE_SRC}
        ${CHAT_SRC}
        ${PLAY_SRC}
        ${STREAM_SRC}
        ${USER_SRC})
