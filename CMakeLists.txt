cmake_minimum_required(VERSION 3.5)

project(PLUGINS_LIB)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(ranges HAS_RANGES)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib_structures lib_structures)

add_library(${PROJECT_NAME} STATIC)

include(${CMAKE_CURRENT_SOURCE_DIR}/FindMailIO.cmake)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/plugins PLUGIN_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/app APP_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/base BASE_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/chat CHAT_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/play PLAY_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/stream STREAM_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/user USER_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/strategies/manage MANAGE_SRC)

target_link_libraries(${PROJECT_NAME} PUBLIC STRUCTURES_LIB ${MAILIO_LIB})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost 1.75 REQUIRED regex date_time)
    if(Boost_FOUND)
        target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
    endif()
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC
        ${MAILIO_INCLUDE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(${PROJECT_NAME} PUBLIC
        ${PLUGIN_SRC}
        ${APP_SRC}
        ${BASE_SRC}
        ${CHAT_SRC}
        ${PLAY_SRC}
        ${STREAM_SRC}
        ${USER_SRC}
        ${MANAGE_SRC})
